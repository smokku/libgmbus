/* rpcentity.h
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

#ifndef RPC_ENTITY_HH
#define RPC_ENTITY_HH

#include "gmbus/mclient.h"
#include "gmbus/mconfig.h"

#include "gmbus/rpctypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
\addtogroup rpcentity RPCEntity
\{
*/

/**
\brief An RPC entity is an Mbus entity providing additional functionality
    defined int he Mbus guidelines
*/
typedef struct {
	/** The Mbus client providing the basic functionality for an Mbus
	 * entity */
	MClient *			client;

	/** A hash table of registered RPC commands */
	GHashTable *		commands;
	/** A hash table of registered RPC return commands */
	GHashTable *		returns;

	/** A callback function invoked for unknown messages */
	MMessageCallback	unknown_message;
} RPCEntity;

RPCEntity * rpc_entity_new( const gchar * address, MConfig * config );
void rpc_entity_free( RPCEntity * self );

void rpc_entity_register( RPCEntity * self, const gchar * command,
		RPCCommandFunc func, gpointer data );
/* void rpc_entity_register_return( RPCEntity * self, const gchar * command,
    RPCReturnFunc func, gpointer data ); */

void rpc_entity_subscribe( RPCEntity * self, MEventType type,
		MCallbackFunc func, gpointer data );
void rpc_entity_unsubscribe( RPCEntity * self, MEventType type );

gboolean rpc_entity_send_return( RPCEntity * self, RPCReturn * ret,
		const MAddress * dest );
gboolean rpc_entity_send_command( RPCEntity * self, RPCCommand * ret,
		const MAddress * addr, RPCReturnFunc func, gpointer data );

#define rpc_entity_ok( self ) ( self->client->ok )
#define rpc_entity_get_client( self ) ( self->client )

/**
\}
*/

#ifdef __cplusplus
}
#endif
#endif

/* end of rpcentity.h */
