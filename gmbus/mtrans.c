/* mtrans.c
 * -*- coding: utf-8 -*-
 *
 * Author: Andreas Büsching  <crunchy@bitkipper.net>
 *
 * Copyright (C) 2004, 2005, 2006, 2007
 *		Andreas Büsching <crunchy@bitkipper.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "gmbus/maddress.h"
#include "gmbus/mtrans.h"
#include "gmbus/mpayload.h"
#include "gmbus/mbus.h"
#include "gmbus/mmessage.h"
#include "gmbus/mutil.h"

static const gdouble	c_hello_factor = 200.0;
static const gdouble	c_hello_min = 1000.0;
static const gdouble	c_hello_dither_min = 0.9;
static const gdouble	c_hello_dither_max = 1.1;
static const gdouble	c_hello_dead = 5.0;
static const gdouble	T_r = 100.0;
static const gdouble	N_r = 3;
static const gdouble	T_c = 70.0;
/* static const gdouble	T_k = ( N_r * ( N_r + 1 ) / 2 ) * T_r; */

static GTimeVal			__now;

/**
\struct MLostEntity
\brief for internal use only.
*/
typedef struct {
  glong				is_dead;
  MAddressCallback	callback;
} MLostEntity;

/**
\struct MReliableMessage
\brief for internal use only.
*/
typedef struct {
  guint			n;
  guint			timer;
  MMessage *	message;
  MTransport *	transport;
} MReliableMessage;

/**
\struct MAcknowledgement
\brief for internal use only.
*/
typedef struct {
  MAddress *	source;
  GArray *		acks;
  guint			timer;
  MTransport *	transport;
} MAcknowledgement;

/* manage entities */
static gboolean
__find_dead_entities( MAddress * address, MEntity * entity,
		MLostEntity * lost_entity )
{
	GTimeVal tmp = entity->last_hello;

	g_time_val_add( &tmp, lost_entity->is_dead );

	if ( ( tmp.tv_sec < __now.tv_sec ) ||
			( ( tmp.tv_sec == __now.tv_sec ) &&
					( tmp.tv_usec < __now.tv_usec ) ) ) {
		/* inform registered client */
		if ( mbus_callback_ok( lost_entity->callback ) )
			mbus_callback_invoke( lost_entity->callback, address );

		/* clean up */
		mbus_address_free( address );
		g_free( entity );

		/* remove the entity */
		return TRUE;
	}

	return FALSE;
}

static gboolean
__mbus_transport_check_lost_entities( MTransport * self )
{
	static gdouble	hello_d;
	MLostEntity		lost_entity;

	g_get_current_time( &__now );

	hello_d = ( g_hash_table_size( self->entities ) + 1 ) * c_hello_factor;
	if ( hello_d < c_hello_min )
		hello_d = c_hello_min;
	lost_entity.is_dead =
			( glong ) ( c_hello_dead * hello_d * c_hello_dither_max );
	/* is_dead in microseconds */
	lost_entity.is_dead *= 1000;
	lost_entity.callback = self->lost_entity;
	g_hash_table_foreach_remove( self->entities,
			( GHRFunc ) __find_dead_entities, &lost_entity );

	return TRUE;
}

/* hello timer */
static guint
__calculate_hello_interval( MTransport * self )
{
	static gdouble	hello_d;
	static gdouble	hello_e;
	static gdouble	dither;

	hello_d = g_hash_table_size( self->entities ) * c_hello_factor;
	if ( hello_d < c_hello_min ) hello_d = c_hello_min;
	dither = c_hello_dither_min +
			( g_random_double() * ( c_hello_dither_max - c_hello_dither_min ) );
	hello_e = hello_d * dither;

	return ( guint ) hello_e;
}

static gboolean
__mbus_transport_when_hello( MTransport * self )
{
	mbus_transport_send( self, self->hello_message );

	self->hello_timer = g_timeout_add( __calculate_hello_interval( self ),
			( GSourceFunc ) __mbus_transport_when_hello, self );

	return FALSE; /* remove old hello timer */
}

/* handle default message */
static void
__mbus_transport_recv_hello( MTransport * self, const MMessage * msg )
{
	MEntity * entity =
			g_hash_table_lookup( self->entities, msg->header->source );
	g_get_current_time( &entity->last_hello );
}

static void
__mbus_transport_recv_bye( MTransport * self, const MMessage * msg )
{
}

/* check for new entities */
static void
__mbus_transport_check_for_new_entity( MTransport * self,
					   const MLinkMessage * lmsg )
{
	/* not found (new entity) */
	if ( !g_hash_table_lookup( self->entities,
					lmsg->message->header->source ) ) {
		/* add new entity to list */
		MEntity *	ep = g_new( MEntity, 1 );
		MAddress *	addr = mbus_address_copy( lmsg->message->header->source );

		ep->endpoint = lmsg->sender;
		g_get_current_time( &ep->last_hello );
		g_hash_table_insert( self->entities, addr, ep );

		if ( mbus_callback_ok( self->new_entity ) )
			mbus_callback_invoke( self->new_entity, addr );
	}
}

/* analyse incoming message */
static void
__mbus_transport_check_for_acks( MTransport * self, const MMessage * msg )
{
	gint i;

	for ( i = 0; i < msg->header->acks->len; i++ ) {
		gpointer	rel;
		gpointer	key = &( g_array_index( msg->header->acks, glong, i ) );

		rel = g_hash_table_lookup( self->reliable_messages, key );
		/* received */
		if ( rel ) {
			MReliableMessage *	rmsg = ( MReliableMessage * ) rel;

			if ( rmsg->timer ) g_source_remove( rmsg->timer );
			mbus_message_free( rmsg->message );
			g_hash_table_lookup_extended( self->reliable_messages, key,
					&key, &rel );
			g_hash_table_remove( self->reliable_messages, key );
			g_free( rel );
			g_free( key );
		}
	}
}

static void
__mbus_transport_handle_message( MTransport * self, const MLinkMessage * lmsg )
{
	/* check for acknowledgements of reliable messages */
	if ( g_hash_table_size( self->reliable_messages ) )
		__mbus_transport_check_for_acks( self, lmsg->message );

	/* is this a new entity */
	__mbus_transport_check_for_new_entity( self, lmsg );
	/* check for known messages like hello and bye */
	if ( mbus_message_contains( lmsg->message, MBUS_HELLO ) ) {
		__mbus_transport_recv_hello( self, lmsg->message );
	} else if ( mbus_message_contains( lmsg->message, MBUS_BYE ) )
		__mbus_transport_recv_bye( self, lmsg->message );
	else {
		/* is addressed to this entity */
		if ( !mbus_address_is_subset_of( self->address,
						lmsg->message->header->destination ) ) return;

		if ( mbus_callback_ok( self->incoming_message ) &&
				mbus_payload_size( lmsg->message->payload ) ) {
			mbus_callback_invoke( self->incoming_message, lmsg->message );
		}
	}
}

static gboolean
__mbus_transport_is_my_message( MTransport * self, const MLinkMessage * lmsg )
{
	return mbus_address_is_equal( self->address,
			lmsg->message->header->source );
}

/* callback messages for the link layer */
static void
__mbus_transport_when_link_error( const MError * error, gpointer data )
{
	MTransport * self = data;

	g_assert( self );
	if ( mbus_callback_ok( self->transport_error ) )
		mbus_callback_invoke( self->transport_error, error );
}

static gboolean
__mbus_transport_send_acks( MAcknowledgement * ack )
{
	/* need to send acknowledgement without piggy bagging */
	MMessage * msg = mbus_message_new();

	g_array_append_vals( msg->header->acks, ack->acks->data, ack->acks->len );
	msg->header->sequence_no = ack->transport->sequence_no++;
	mbus_address_assign( msg->header->destination, ack->source );
	mbus_address_assign( msg->header->source, ack->transport->address );
	mbus_link_send( ack->transport->link, msg );

	g_hash_table_remove( ack->transport->acknowledgements, ack->source );

	mbus_address_free( ack->source );
	g_array_free( ack->acks, TRUE );
	g_free( ack );

	mbus_message_free( msg );

	return FALSE;
}

static void
__mbus_transport_when_link_message( const MLinkMessage * lmsg, gpointer data )
{
	MTransport * transport = data;

	g_assert( lmsg );

	if ( __mbus_transport_is_my_message( transport, lmsg ) ) return;

	/* if reliable message extract sequence no and prepare for sending ack */
	if ( lmsg->message->header->type == RELIABLE ) {
		MAcknowledgement * ack =
				g_hash_table_lookup( transport->acknowledgements,
						lmsg->message->header->source );

		if ( ack ) {
			g_array_append_val( ack->acks, lmsg->message->header->sequence_no );
		} else {
			MAcknowledgement *	ack = g_new( MAcknowledgement, 1 );
			MAddress *			addr =
					mbus_address_copy( lmsg->message->header->source );

			ack->acks = g_array_new( FALSE, FALSE, sizeof( gulong ) );
			g_array_append_val( ack->acks, lmsg->message->header->sequence_no );
			ack->source = addr;
			ack->transport = transport;
			ack->timer = g_timeout_add( T_c,
					( GSourceFunc ) __mbus_transport_send_acks, ack );
			g_hash_table_insert( transport->acknowledgements, addr, ack );
		}
	}

  __mbus_transport_handle_message( transport, lmsg );
}

static void
__mbus_transport_init( MTransport * self, MConfig * config )
{
	MLinkCallback cl = { ( gpointer ) self,
						 __mbus_transport_when_link_error,
						 __mbus_transport_when_link_message };

	/* init members */
	self->sequence_no = 0;
	self->entities = g_hash_table_new( ( GHashFunc ) mbus_address_hash,
			( GEqualFunc ) mbus_address_is_equal );
	self->reliable_messages = g_hash_table_new( g_int_hash, g_int_equal );
	self->acknowledgements =
			g_hash_table_new( ( GHashFunc ) mbus_address_hash,
					( GEqualFunc ) mbus_address_is_equal );
	self->config = config;

	/* setup network link */
	self->link = mbus_link_new( config );
	if ( mbus_link_ok( self->link ) ) {
		mbus_link_attach( self->link, cl );
		self->ok = TRUE;
	} else {
		self->ok = FALSE;
	}

	/* init callback object */
	mbus_callback_init( self->incoming_message );
	mbus_callback_init( self->lost_entity );
	mbus_callback_init( self->new_entity );
	mbus_callback_init( self->transport_error );

	/* prepare hello message */
	self->hello_timer = 0;
	self->hello_message = mbus_message_new();
	mbus_message_add_command( self->hello_message, "mbus.hello ()" );
	/* do NOT send hello message directly from here as it causes mysterious
	   problems with the sockets */
	self->hello_timer = g_timeout_add( 0,
			( GSourceFunc ) __mbus_transport_when_hello, self );

	self->lost_timer = g_timeout_add( 1000, ( GSourceFunc )
			__mbus_transport_check_lost_entities,
			self );
}

/* the Mbus transport object */
/**
\brief creates a new transport object
\param config the configuration of the Mbus session this transport
	object should be used for
\param address a string containing the Mbus address for the entity that
	will be represented by this transport object
\return pointer to the newly created transport object
*/
MTransport *
mbus_transport_new( MConfig * config, const gchar * address )
{
	MTransport *	self = g_new( MTransport, 1 );
	GString *		tmp = g_string_new( address );

	g_assert( self );

	self->address = mbus_address_new();
	mbus_address_read( self->address, tmp );
	mbus_address_disambiguate( self->address );
	self->ok = mbus_address_ok( self->address );

	g_string_erase( tmp, 0, -1 );
	mbus_address_as_string( self->address, tmp );

	g_string_free( tmp, TRUE );

	__mbus_transport_init( self, config );

	return self;
}

static gint
__mbus_transport_finalize( MTransport * self )
{
	mbus_link_free( self->link );
	g_hash_table_destroy( self->entities );
	g_hash_table_destroy( self->reliable_messages );
	g_hash_table_destroy( self->acknowledgements );

	g_source_remove( self->hello_timer );
	g_source_remove( self->lost_timer );

	mbus_address_free( self->address );
	mbus_message_free( self->hello_message );

	return 0;
}

/**
\brief frees the memory allocated by the transport object
\param self the transport object to free
*/
void
mbus_transport_free( MTransport * self )
{
	__mbus_transport_finalize( self );
	g_free( self );
}

static gboolean
__retransmit_reliable( MReliableMessage * message )
{
	message->n++;

	if ( message->n < N_r ) {
		message->timer = g_timeout_add( message->n * T_r,
				( GSourceFunc ) __retransmit_reliable, message );
	} else {
		gpointer	key = NULL;
		gpointer	value = message;
		GString *	seq_str = g_string_new( NULL );

		mbus_util_int64_to_str( seq_str,
				message->message->header->sequence_no, FALSE );
		g_warning(
				"sending message reliable failed: never received an ack (%s)",
				seq_str->str );
		g_string_free( seq_str, TRUE );

		if ( mbus_callback_ok( message->transport->transport_error ) ) {
			MError error;

			mbus_error_init( error );
			mbus_error_set( error, MERR_SEND_RELIABLE,
					"sending message reliable failed" );

			mbus_callback_invoke( message->transport->transport_error, &error );
			mbus_error_reset( error );
		}

		if ( g_hash_table_lookup_extended(
				message->transport->reliable_messages,
				& message->message->header->sequence_no,
				&key, &value ) ) {
		  g_hash_table_remove( message->transport->reliable_messages, key );
		  mbus_message_free( message->message );
		  g_free( message );
		  g_free( key );
		}
	}

	return FALSE;
}

/**
\brief sends a given Mbus message into the Mbus session used by the
	given transport objct
\param self the transport object
\param msg the mMbus message to send
\return TRUE if the sending of the message was successful, otherwise FALSE
*/
gboolean
mbus_transport_send( MTransport * self, MMessage * msg )
{
	MEntity * entity;

	msg->header->sequence_no = self->sequence_no++;
	mbus_address_assign( msg->header->source, self->address );

	if ( msg->header->type == RELIABLE ) {
		gulong *			key = g_new( gulong, 1 );
		MReliableMessage *	rel_message = g_new( MReliableMessage, 1 );

		*key = msg->header->sequence_no;
		rel_message->n = 1;
		rel_message->transport = self;
		rel_message->message = mbus_message_new();
		mbus_message_assign( rel_message->message, msg );
		rel_message->timer = g_timeout_add( rel_message->n * T_r,
				( GSourceFunc ) __retransmit_reliable, rel_message );
		g_hash_table_insert( self->reliable_messages, key, rel_message );
	}

	/* see if we could pass some acks with it */
	if ( mbus_address_is_unique( msg->header->destination ) ) {
		MAcknowledgement * ack =
				g_hash_table_lookup( self->acknowledgements,
						msg->header->destination );
		if ( ack ) {
			/* stop timer */
			g_source_remove( ack->timer );
			g_array_append_vals( msg->header->acks, ack->acks->data,
					ack->acks->len );
			g_hash_table_remove( self->acknowledgements, ack->source );
			mbus_address_free( ack->source );
			g_array_free( ack->acks, TRUE );
			g_free( ack );
		}
	}

	entity = g_hash_table_lookup( self->entities, msg->header->destination );

	if ( !entity || ( self->config->send_unicast == FALSE ) )
		return mbus_link_send( self->link, msg );
	else
		return mbus_link_send_unicast( self->link, msg, &( entity->endpoint ) );
}

/**
\brief checks if a given Mbus entity exists in the current Mbus session
	managed by the transport object
\param self the transport object
\param addr the Mbus address identifying the Mbus entity to search for.
\sa MAddress
*/
gboolean
mbus_transport_is_entity_available( MTransport * self, const MAddress * addr )
{
	return ( g_hash_table_lookup( self->entities, addr ) != NULL );
}
