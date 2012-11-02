/* mclient.c
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "gmbus/mclient.h"
#include "gmbus/mtrans.h"

#include <stdio.h>
#include <string.h>

static void
__mbus_client_incoming_message( MMessage * msg, MClient * client )
{
	GSList *			walk;
	MCommandCallback *	cb;
	MCommand *			command;
	guint				i;
	GString *			tmp = g_string_new( NULL );

	mbus_message_as_string( msg, tmp );
	g_string_free( tmp, TRUE );
	/* if there are no commands in this message abort */
	if ( !mbus_payload_size( msg->payload ) ) return;

	for ( i = 0; i < mbus_payload_size( msg->payload ); i++ ) {
		command = mbus_payload_index( msg->payload, i );
		walk = client->callbacks;

		while ( walk ) {
			cb = walk->data;
			if ( !strcmp( cb->command, command->cmd->str ) ) break;
			walk = g_slist_next( walk );
		}

		if ( walk && mbus_callback_ok( cb->callback ) ) {
			msg->_index = i;
			mbus_callback_invoke( cb->callback, msg );
		} else if ( mbus_callback_ok( client->unknown_message ) )
			mbus_callback_invoke( client->unknown_message, msg );
	}
}

static void
__mbus_client_init( MClient * self, const gchar * address )
{
	self->trans = mbus_transport_new( self->config, address );

	if( mbus_transport_ok( self->trans ) )
		self->ok = TRUE;
	else
		self->ok = FALSE;

	mbus_callback_init( self->unknown_message );
	mbus_callback_set( self->trans->incoming_message,
			( MMessageFunc ) __mbus_client_incoming_message, self )
			self->callbacks = NULL;
}

/**
\brief Creates and initializes a new MClient object. The object must be
    finalized and delete by calling \a mbus_client_free.
\param address the Mbus address for this entity as a string
\param config the configuration for the Mbus session this client will attach to
\return the newly created MClient object
*/
MClient *
mbus_client_new( const gchar * address, MConfig * config )
{
	MClient * self = g_new( MClient, 1 );

	g_assert( self );

	self->ok = TRUE;

	if ( config )
		self->config = config;
	else
		self->config = mbus_config_new( TRUE );

	__mbus_client_init( self, ( const gchar * ) address );

	return self;
}

static void
__mbus_client_finalize( MClient * self )
{
	mbus_transport_free( self->trans );
	mbus_config_free( self->config );
}

/**
\brief Finalizes and deletes the specified MClient object.
\param self the MClient object to delete
*/
void
mbus_client_free( MClient * self )
{
	__mbus_client_finalize( self );
	g_free( self );
}

/**
\brief An MClient object may notify an application of specific events that
    occur an the Mbus. To be notified on such events the application has to
    subscribe for it by providing the event type and a callback function.
\param self the current object
\param type the event
\param cb the callback function that will be invoked if the event occures
\param data an additional parameter which be passwd to the callback function
*/
void
mbus_client_subscribe( MClient * self, MEventType type, MCallbackFunc cb,
		gpointer data )
{
	switch ( type ) {
	case MEVENT_UNKNOWN_MESSAGE:
		mbus_callback_set( self->unknown_message, ( MMessageFunc ) cb, data );
		break;
	case MEVENT_LOST_ENTITY:
		mbus_callback_set( self->trans->lost_entity, ( MAddressFunc ) cb, data );
		break;
	case MEVENT_NEW_ENTITY:
		mbus_callback_set( self->trans->new_entity, ( MAddressFunc ) cb, data );
		break;
	case MEVENT_ERROR:
		mbus_callback_set( self->trans->transport_error, ( MErrorFunc ) cb, data );
	}
}

/**
   \brief Unsubscribes for the event \a type. After that the application is not
   notified anymore if an event of type \a type occures.
   \param self the current object
   \param type type of event to unsubscribe
   \sa mbus_client_subscribe.
*/
void
mbus_client_unsubscribe( MClient * self, MEventType type )
{
	switch ( type ) {
	case MEVENT_UNKNOWN_MESSAGE:
		mbus_callback_init( self->unknown_message );
		break;
	case MEVENT_LOST_ENTITY:
		mbus_callback_init( self->trans->lost_entity );
		break;
	case MEVENT_NEW_ENTITY:
		mbus_callback_init( self->trans->new_entity );
		break;
	case MEVENT_ERROR:
		mbus_callback_init( self->trans->transport_error );
	}
}

/**
\brief Used to send a message to the Mbus. \a mbus_client_send sets the header
    fields address and sequence number.
\param self the current object
\param msg the message to send
\return TRUE means the operations was successfully completed, otherwise FALSE
*/
gboolean
mbus_client_send( MClient * self, MMessage * msg )
{
	gboolean ret = mbus_transport_send( self->trans, msg );
	mbus_message_free( msg );

	return ret;
}

/**
\brief To become informed when a specific mbus command arrives the
    application may call this function to register for it. In the case the
    given command is received the function \a func is invoked with the second
    argument of \a data
\param self the client to register with
\param cmd the command to register
\param func the function to invoke
\param data the additonal argument to pass to the callback function
*/
void
mbus_client_register( MClient * self, const gchar * cmd, MMessageFunc func,
		gpointer data )
{
	MCommandCallback * cb = g_new( MCommandCallback, 1 );

	cb->command = g_strdup( cmd );
	mbus_callback_set( cb->callback, func, data );

	self->callbacks = g_slist_append( self->callbacks, cb );
}

/**
\brief This function removes the registration for a given mbus command
from the MCLient obect
\param self the client object
\param cmd the Mbus command to unregister
*/
void
mbus_client_unregister( MClient * self, const gchar * cmd )
{
	GSList *			walk = self->callbacks;
	MCommandCallback *	cb;

	while ( walk ) {
		cb = walk->data;
		if ( !strcmp( cb->command, cmd ) ) break;
		walk = g_slist_next( walk );
	}

	if ( walk ) {
		g_free( cb->command );
		g_free( cb );
		self->callbacks = g_slist_remove( self->callbacks, walk );
	}
}

/**
\brief Sends an Mbus message as a reliable message and internally handles the
    reception of acknowledgements or necessary retransmissions
\param self the client object to use for the transmission
\param msg the message to send reliable
\param addr the destination
\return TRUE if the transmission of the message was successful, otherwise FALSE. It does not mean that the message is already acknowledged, but that was send to the Mbus session.
*/
gboolean
mbus_client_send_reliable( MClient * self, MMessage * msg,
		const MAddress * addr )
{
	if ( mbus_address_is_unique( addr ) &&
			mbus_transport_is_entity_available( self->trans, addr ) ) {
		gboolean ret;

		msg->header->type = RELIABLE;
		mbus_address_assign( msg->header->destination, addr );
		ret = mbus_transport_send( self->trans, msg );
		mbus_message_free( msg );

		return ret;
	}

	return FALSE;
}
