/* mclient.h
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

#ifndef MBUS_CLIENT_H
#define MBUS_CLIENT_H

#include "gmbus/maddress.h"
#include "gmbus/mtrans.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
\addtogroup mclient MClient
\{
\brief manages the Mbus interface of a single Mbus entity. Applications may
    instantiate more than one MClient.
*/
typedef struct {
	/** The Mbus transport layer */
	MTransport *		trans;
	/** Status information */
	gboolean		ok;
	/** The configuration for the Mbus session */
	MConfig *		config;

	/** A single linked list of callback functions for registered
	 * commands */
	GSList *		callbacks;

	/** Callback function invoked on recpetion of unknown messages */
	MMessageCallback	unknown_message;
} MClient;

/**
\enum MEventType
\brief An MClient object may notify the application of specific events that
    occured on the Mbus. To receive such notifications an application needs
    to attach it self to the event handler of the MClient
*/
typedef enum {
	/** Mbus commands that are not registered with a specific callback
	 * function */
	MEVENT_UNKNOWN_MESSAGE,
	/** If an known entity leaves the Mbus session */
	MEVENT_LOST_ENTITY,
	/** If unknown entity appears in an Mbus session */
	MEVENT_NEW_ENTITY,
	/** If any error occures an the transport layer */
	MEVENT_ERROR
} MEventType;

typedef struct {
	gchar *				command;
	MMessageCallback	callback;
} MCommandCallback;

MClient * mbus_client_new( const gchar * address, MConfig * config );
void mbus_client_free( MClient * self );
void mbus_client_subscribe( MClient * self, MEventType type,
		MCallbackFunc cb, gpointer data );
void mbus_client_unsubscribe( MClient * self, MEventType type );
void mbus_client_register( MClient * self, const gchar * cmd,
		MMessageFunc func, gpointer data );
void mbus_client_unregister( MClient * self, const gchar * cmd );
gboolean mbus_client_send( MClient * self, MMessage * msg );
gboolean mbus_client_send_reliable( MClient * self, MMessage * msg,
		const MAddress * addr );

/**
\brief General status predicate.
\param self the current object
\return TRUE if there has been no error occured since last call, otherwise FALSE
*/
#define mbus_client_ok( self ) ( self->ok )

/**
\}
*/
#ifdef __cplusplus
}
#endif

#endif
