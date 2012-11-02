/* mlink.c
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

#include "gmbus/mlink.h"

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static GSList * __mbus_sessions = NULL;

typedef struct {
	MLink *		link;
	MConfig *	config;
} MSession;

static void
__mbus_link_when_message( MLink * self, MLinkMessage * msg )
{
	gint i;

	for ( i = 0; i < self->callbacks->len; i++ ) {
		MLinkCallback * cb = &( g_array_index( self->callbacks,
						MLinkCallback, i ) );

		cb->when_message( msg, cb->data );
	}
}

static void
__mbus_link_when_error( MLink * self, MError * error )
{
	gint i;

	for ( i = 0; i < self->callbacks->len; i++ ) {
		MLinkCallback * cb = &( g_array_index( self->callbacks,
						MLinkCallback, i ) );

		cb->when_error( error, cb->data );
	}
}

static gboolean
__mbus_link_udp_event( GIOChannel * source, GIOCondition cond, MLink * link )
{
	gsize			len;
	MMessage *		msg = NULL;
	MEndpoint		ep;
	guint			slen = sizeof( MEndpoint );
	gint			fd = g_io_channel_unix_get_fd( source );

	mbus_error_reset( link->error );

	if ( ( len = recvfrom( fd, link->buf->data, MBUS_MESSAGE_BUF_SIZE, 0,
							( struct sockaddr * ) &ep, &slen ) ) == -1 ) {
		mbus_error_printf( &link->error, MERR_READ,
				"failed to read data from socket %d", fd );
	} else {
		GString *	mbuf = g_string_new( NULL );

		g_byte_array_set_size( link->buf,  len );

		if ( ! mbus_coder_decode( link->coder, link->buf, mbuf ) ) {
			mbus_error_set( link->error, MERR_DECODE,
					"could not decode message" );
			g_string_free( mbuf, TRUE );

			return TRUE;
		} else {
			msg = mbus_message_new();

			mbus_message_read( msg, mbuf );

			if ( !mbus_message_ok( msg ) ) {
				mbus_error_set( link->error, MERR_PARSE,
						"error parsing message" );
				g_string_free( mbuf, TRUE );
				mbus_message_free( msg );

				return TRUE;
			}
		}

		g_string_free( mbuf, TRUE );
	}

	if ( mbus_link_ok( link ) ) {
		MLinkMessage lmsg;

		lmsg.sender = ep;
		lmsg.message = msg;
		__mbus_link_when_message( link, &lmsg );
	} else {
		__mbus_link_when_error( link, &link->error );
	}

	if ( msg ) mbus_message_free( msg );

	return TRUE;
}

static MLink *
__mbus_session_find_link( MConfig * config )
{
	GSList * pos = __mbus_sessions;

	while ( pos ) {
		MSession * session = pos->data;

		if ( mbus_config_is_equal( session->config, config ) ) {
			session->link->ref_count++;
			return session->link;
		}

		pos = g_slist_next( pos );
	}

	return NULL;
}

MLink *
mbus_link_new( MConfig * config )
{
	MLink * self;

	/* check for an existing instance for the requested configuration */
	self = __mbus_session_find_link( config );

	if ( ! self ) {
		self = g_new( MLink, 1 );
		g_assert( self );

		mbus_error_init( self->error );
		self->multicast = NULL;
		self->unicast = NULL;
		self->ref_count = 1;
		self->buf = g_byte_array_sized_new( MBUS_MESSAGE_BUF_SIZE );
		self->callbacks = g_array_new( FALSE, TRUE, sizeof( MLinkCallback ) );
		self->default_dest = *( config->group_addr );
		self->coder = mbus_coder_new( config );

		if( !mbus_coder_ok( self->coder ) ) {
			mbus_error_set( self->error, MERR_CODER_SETUP,
					"failed to setup message coder" );
			goto done;
		}

		/* set up multicast socket */
		self->multicast = mbus_socket_new( config->group_addr, TRUE );
		if( !self->multicast ) {
			mbus_error_set( self->error, MERR_MULTICAST_JOIN,
					"failed to setup multicast socket" );
			goto done;
		}

		/* retrieve local socket address */
		mbus_socket_get_name( self->multicast, &( self->ep_multicast ) );
		self->source_multicast = g_io_add_watch( self->multicast, G_IO_IN,
				( GIOFunc ) __mbus_link_udp_event, self );

		/* create unicast socket for sending */
		self->unicast = mbus_socket_new( config->unicast_addr, FALSE );
		if( !self->unicast ) {
			mbus_error_set( self->error, MERR_UNICAST,
					"failed to setup unicast socket" );
			goto done;
		}

		mbus_socket_get_name( self->unicast, &( self->ep_unicast ) );
		self->source_unicast = g_io_add_watch( self->unicast, G_IO_IN,
				( GIOFunc ) __mbus_link_udp_event, self );
	}

	return self;

  done:
	g_warning( self->error.message );
	if ( self->multicast ) g_io_channel_unref( self->multicast );
	if ( self->unicast ) g_io_channel_unref( self->unicast );

	return self;
}


void
mbus_link_free( MLink * self )
{
	self->ref_count--;

	/* just delete the object if there are no more references to it */
	if ( self->ref_count ) return;

	if( self->multicast ) {
		g_source_remove( self->source_multicast );
		g_io_channel_unref( self->multicast );
		self->multicast = NULL;
	}
	if(self->unicast) {
		g_source_remove( self->source_unicast );
		g_io_channel_unref( self->unicast );
		self->unicast = NULL;
	}
	g_byte_array_free( self->buf, TRUE );

	g_free( self );
}

gboolean
mbus_link_ok( const MLink * self )
{
	return ( self->error.type == MERR_NOERR );
}

const MError *
mbus_link_error( const MLink * self )
{
	return &self->error;
}

static gboolean
__mbus_link_do_send( MLink * self, MMessage * msg, const MEndpoint * ep )
{
	GString *	str = g_string_new( NULL );
	GByteArray *	buf = g_byte_array_new();

	/* reset error variable */
	mbus_error_reset( self->error );

	/* serialize msg */
	mbus_message_as_string( msg, str );

	if ( !mbus_message_ok( msg ) ) {
		mbus_error_set( self->error, MERR_WRITE,
				"could not create message text" );
	} else {			/* encode */
		/* encode into my msg buffer */
		if ( ! mbus_coder_encode( self->coder, str, buf ) ) {
			mbus_error_set( self->error, MERR_ENCODE,
					"failed to encode message" );
		} else {			/* send now */
			gint	fd;
			gint	ep_size = mbus_endpoint_get_size( ( MEndpoint * ) ep );

			if ( self->unicast )
				fd = g_io_channel_unix_get_fd( self->unicast );
			else
				fd = g_io_channel_unix_get_fd( self->multicast );


			if ( sendto( fd, buf->data, buf->len, MSG_DONTWAIT,
							( struct sockaddr * ) ep, ep_size ) == -1 ) {
				perror( "sendto" );
				mbus_error_set( self->error, MERR_SEND,
						"sending to Mbus session failed" );
				g_error( self->error.message );
			}
		}
	}

	g_byte_array_free( buf, TRUE );
	g_string_free( str, TRUE );

	return TRUE;
}

gboolean
mbus_link_send( MLink * self, MMessage * msg )
{
	return __mbus_link_do_send( self, msg, &self->default_dest );
}

gboolean
mbus_link_send_unicast( MLink * self, MMessage * msg, const MEndpoint * ep )
{
	return __mbus_link_do_send( self, msg, ep );
}

void
mbus_link_attach( MLink * self, MLinkCallback c )
{
	g_array_append_val( self->callbacks, c );
}

gboolean
mbus_link_detach( MLink * self, MLinkCallback c )
{
	gint i;

	for ( i = 0; i < self->callbacks->len; i++ ) {
		MLinkCallback * cb = &( g_array_index( self->callbacks,
						MLinkCallback, i ) );

		if ( ( cb->data == c.data ) && ( cb->when_message == c.when_message ) &&
				( cb->when_error == c.when_error ) ) {
			g_array_remove_index( self->callbacks, i );
			return TRUE;
		}
	}

	return FALSE;
}
