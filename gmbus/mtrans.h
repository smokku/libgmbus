/* mtrans.h
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

#ifndef MBUS_TRANSPORT_H
#define MBUS_TRANSPORT_H

#include "gmbus/mlink.h"
#include "gmbus/mutil.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
\addtogroup mtransport MTransport
\{
\brief It manages the transport of Mbus messages for a single MClient and
    implementsthe Mbus behaviour for an Mbus entity, like sending hello
    messages checking for new or lost entities and notifying the application of
    pre-defined events.
*/

/**
\struct MEntity
\brief is for internal use only.
*/
typedef struct {
	MEndpoint	endpoint;
	GTimeVal	last_hello;
} MEntity;

/**
\brief Implements the basic protocol behaviour
\sa MLink, MAddress, MMessage
\sa MMessageCallback, MAddressCallback, MErrorCallback
*/
typedef struct {
	/** The socket interface for the Mbus session */
	MLink *			link;
	/** The Mbus address of this entity */
	MAddress *		address;

	/** List of known entites */
	GHashTable *	entities;
	/** List of Mbus message object that needs to be acknowledged by the
		receiver as they are send in reliable mode. The acknowledgement
		for these messages is still missing.*/
	GHashTable *	reliable_messages;
	/** List of sequnece numbers of messages that needs to be
		acknowledged as they were received in reliable mode*/
	GHashTable *	acknowledgements;

	/** Mbus hello message object*/
	MMessage *		hello_message;

	/** ID for the hello timer */
	guint			hello_timer;
	/** ID for the lost entity timer. When this timer expires the
		transport object will check for dead entities. */
	guint			lost_timer;
	/** Last used sequence number */
	guint			sequence_no;

	/** This callback function is invoked on reception of an
		unregistered Mbus command */
	MMessageCallback	incoming_message;
	/** This callback function is invoked when an entity is lost */
	MAddressCallback	lost_entity;
	/** Ths callback function is invoked when a new entity appears in
		the Mbus session */
	MAddressCallback	new_entity;
	/** This callback function is invoked when an error has occured */
	MErrorCallback		transport_error;

	/** Current status predicate */
	gboolean		ok;

	/** A link to the Mbus configuration object relevant for this
		transport object */
	MConfig *		config;
} MTransport;

MTransport * mbus_transport_new( MConfig * config, const gchar * address );
void mbus_transport_free( MTransport * self );
gboolean mbus_transport_send( MTransport * self, MMessage * msg );
gboolean mbus_transport_is_entity_available( MTransport * self,
		const MAddress * addr );

/**
\def mbus_transport_ok
\brief checks the status of a transport object
\param self the transport object to check
\return TRUE if there has no error occured since the last call,
	otherwise FALSE
*/
#define mbus_transport_ok(self) ( self->ok )

/**
\}
*/
#ifdef __cplusplus
}
#endif

#endif
