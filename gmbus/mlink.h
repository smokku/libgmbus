/* mlink.h
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

#ifndef MBUS_LINK_H
#define MBUS_LINK_H

#include "gmbus/udp.h"
#include "gmbus/mmessage.h"
#include "gmbus/mcoder.h"
#include "gmbus/mutil.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
\addtogroup mlink MLink
\{
*/

/* The API for higher layer clients */
typedef struct {
	MEndpoint	sender;
	MMessage *	message;
} MLinkMessage;

typedef struct {
	/** associated transport object */
	gpointer *	data;

	/** error callback */
	void ( *when_error )( const MError * c, gpointer object );
	/** for received messages */
	void ( *when_message )( const MLinkMessage * c, gpointer object );
} MLinkCallback;

#define MBUS_MESSAGE_BUF_SIZE 32768

typedef struct {
	/** Represents the mulicast channel to the Mbus session */
	GIOChannel *	multicast;
	/** Represents the unicast channel to the Mbus session */
	GIOChannel *	unicast;
	/** unique identifer for the event source on the multicast socket */
	guint		source_multicast;
	/** unique identifer for the event source on the unicast socket */
	guint		source_unicast;
	/** Transport address of the local multicast endpoint */
	MEndpoint	ep_multicast;
	/** Transport address of the local unicast endpoint */
	MEndpoint	ep_unicast;

	/** Last error that occured */
	MError	error;
	/** dynamic transmission buffer */
	GByteArray *	buf;
	/** List of MLinkCallback objects. Each MTransport object attaching
		to an Mbus session does this by subscribing to corresponding
		MLink object. These subscriptions are stores in this list.*/
	GArray *	callbacks;
	/** The messages coder instance */
	MCoder *	coder;

	/** The transport address of the Mbus session */
	MEndpoint	default_dest;

	/** Counts the number of MTransport objects attached to this MLink
	 * object */
	guint		ref_count;
} MLink;

MLink * mbus_link_new( MConfig * config );
void mbus_link_free( MLink * self );
gboolean mbus_link_ok( const MLink * self );
const MError *mbus_link_error( const MLink * self );
gboolean mbus_link_send( MLink * self, MMessage * msg );
gboolean mbus_link_send_unicast( MLink * self, MMessage * msg,
		const MEndpoint * ep );
void mbus_link_attach( MLink * self, MLinkCallback c );
gboolean mbus_link_detach( MLink * self, MLinkCallback c );

/**
\}
*/

#ifdef __cplusplus
}
#endif

#endif
