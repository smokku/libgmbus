/* rpcentity.c
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

#include "gmbus/rpcentity.h"
#include "gmbus/mbus.h"

#include <string.h>

static void
__rpc_entity_incoming_message( MMessage * msg, RPCEntity * entity )
{
	static const gchar * ret_suffix = ".return";

	if ( mbus_payload_size( msg->payload ) ) {
		MCommand *	cmd;
		gchar *		ret;

		cmd = mbus_payload_index( msg->payload, 0 );
		ret = cmd->cmd->str + ( cmd->cmd->len - 7 );

		/* is a RPC command */
		if ( strcmp( ret, ret_suffix ) ) {
			RPCCommandCallback * callback;

			callback = g_hash_table_lookup( entity->commands, cmd->cmd->str );
			if ( callback ) {
				RPCCommand *	rpccmd = rpc_command_new_from_command( cmd );
				RPCReturn *		ret;

				if ( !rpccmd ) {
					if ( mbus_callback_ok( entity->unknown_message ) )
						mbus_callback_invoke( entity->unknown_message, msg );
					return;
				}
				mbus_address_assign( rpccmd->source, msg->header->source );
				ret = mbus_callback_invoke( ( *callback ), rpccmd );
				rpc_command_free( rpccmd );
				rpc_entity_send_return( entity, ret, msg->header->source );
			} else {
				if ( mbus_callback_ok( entity->unknown_message ) )
					mbus_callback_invoke( entity->unknown_message, msg );
			}
		} else {
			RPCReturnCallback *	callback;
			RPCReturn *			ret = rpc_return_new_from_command( cmd );
			gpointer			key;

			g_hash_table_lookup_extended( entity->returns,
					M_STRING( ret->id )->str->str, ( gpointer * ) &key,
					( gpointer * ) &callback );
			if ( callback ) {
				g_hash_table_remove( entity->returns,
						M_STRING( ret->id )->str->str );
				g_free( key );
				mbus_callback_invoke( ( *callback ), ret );
				g_free( callback );
			} else {
				if ( mbus_callback_ok( entity->unknown_message ) )
					mbus_callback_invoke( entity->unknown_message, msg );
			}
			rpc_return_free( ret );
		}
	}
}

/**
\brief Creates a new RPC entity.
\param address The Mbus address for this entity.
\param config The Mbus session configuration.
*/
RPCEntity *
rpc_entity_new( const gchar * address, MConfig * config )
{
	RPCEntity * self = g_new( RPCEntity, 1 );

	self->client = mbus_client_new( address, config );
	self->commands = g_hash_table_new( g_str_hash, g_str_equal );
	self->returns = g_hash_table_new( g_str_hash, g_str_equal );

	mbus_callback_init( self->unknown_message );
	mbus_callback_set( self->client->unknown_message,
			( MMessageFunc ) __rpc_entity_incoming_message, self );

	return self;
}

/**
\brief Frees an RPC entity
\param self The RPC entity to free.
*/
void
rpc_entity_free( RPCEntity * self )
{
	g_assert( self );

	mbus_client_free( self->client );
}

/**
\brief Register an RPC command with an RPC entity
\param self The RPC entity to register with
\param command The RPC command name to register
\param func The callback function to invoke when the command is received
\param data An additional argument given to the callback function
*/
void
rpc_entity_register( RPCEntity * self, const gchar * command,
		RPCCommandFunc func, gpointer data )
{
	RPCCommandCallback * callback = g_new( RPCCommandCallback, 1 );

	mbus_callback_set( ( *callback ), func, data );
	g_hash_table_insert( self->commands, g_strdup( command ), callback );
}

/* void
rpc_entity_register_return( RPCEntity * self, const gchar * command,
    RPCReturnFunc func, gpointer data )
{
  RPCReturnCallback * callback = g_new( RPCReturnCallback, 1 );

  mbus_callback_set( ( *callback ), func, data );
  g_hash_table_insert( self->returns, g_strdup( command ), callback );
} */

/**
\brief Subscribes for the event \a type.
\param self the entity that should notify the application when the event occures
\param type The event type to subscribe for.
\param func the callback function to invoke when the event occures
\param data An addtional argument given to the callback function
*/
void
rpc_entity_subscribe( RPCEntity * self, MEventType type,
		MCallbackFunc func, gpointer data )
{
	switch ( type ) {
	case MEVENT_UNKNOWN_MESSAGE:
		mbus_callback_set( self->unknown_message, ( MMessageFunc ) func, data );
		break;
	case MEVENT_LOST_ENTITY:
		mbus_callback_set( self->client->trans->lost_entity,
				( MAddressFunc ) func, data );
		break;
	case MEVENT_NEW_ENTITY:
		mbus_callback_set( self->client->trans->new_entity,
				( MAddressFunc ) func, data );
		break;
	case MEVENT_ERROR:
		mbus_callback_set( self->client->trans->transport_error,
				( MErrorFunc ) func, data );
	}
}

/**
\brief Unsubscribes for the given event type
\param self The RPC entity to unsubscribe from
\param type The event type
*/
void
rpc_entity_unsubscribe( RPCEntity * self, MEventType type )
{
	switch ( type ) {
	case MEVENT_UNKNOWN_MESSAGE:
		mbus_callback_init( self->unknown_message );
		break;
	case MEVENT_LOST_ENTITY:
		mbus_callback_init( self->client->trans->lost_entity );
		break;
	case MEVENT_NEW_ENTITY:
		mbus_callback_init( self->client->trans->new_entity );
		break;
	case MEVENT_ERROR:
		mbus_callback_init( self->client->trans->transport_error );
	}
}

/**
\brief Sends an RPC return command
\param self The RPC entity sending the command
\param ret The RPC return information
\param dest the destination Mbus address for this RPC return
\return TRUE if the message could be send, FALSE otherwise
*/
gboolean
rpc_entity_send_return( RPCEntity * self, RPCReturn * ret,
		const MAddress * dest )
{
	MMessage *	msg = mbus_message_new();
	MObject *	rpc_args = mbus_list_new();
	MObject *	app_args = mbus_list_new();
	MObject *	rpc_id = mbus_list_new();
	MObject *	rpc_status = mbus_list_new();
	MObject *	app_status = mbus_list_new();
	gchar *		tmp = g_strdup_printf( "%s.return", ret->command );
	MCommand *	cmd = mbus_command_new( tmp );

	g_free( tmp );
	/* set destination address */
	mbus_address_assign( msg->header->destination, dest );
	mbus_list_append( rpc_id, mbus_string_new( "ID" ) );
	mbus_list_append( rpc_id,
			mbus_string_new( M_STRING( ret->id )->str->str ) );

	mbus_list_append( rpc_status, mbus_string_new( "RPC-STATUS" ) );

	switch ( ret->status ) {
	case RPC_OK:
		mbus_list_append( rpc_status, mbus_string_new( "OK" ) );
		break;
	case RPC_FAILED:
		mbus_list_append( rpc_status, mbus_string_new( "FAILED" ) );
		break;
	case RPC_UNKNOWN:
		mbus_list_append( rpc_status, mbus_string_new( "UNKNOWN" ) );
	}

	mbus_list_append( rpc_args, rpc_id );
	mbus_list_append( rpc_args, rpc_status );

	switch( ret->app_status ) {
    case RPC_OK:
		mbus_list_append( app_status, mbus_symbol_new( "OK" ) );
		break;
	case RPC_UNKNOWN:
	case RPC_FAILED:
		mbus_list_append( app_status, mbus_symbol_new( "FAILED" ) );
	}

	if ( ret->app_result )
		mbus_list_append( app_status, mbus_symbol_new( ret->app_result ) );
	else {
		if ( ret->status == RPC_OK )
			mbus_list_append( app_status, mbus_symbol_new( MBUS_RPC_OK ) );
		else
			mbus_list_append( app_status, mbus_symbol_new( MBUS_RPC_FAILED ) );
	}

	mbus_list_append( app_status, mbus_string_new( ret->app_description ) );

	mbus_list_append( app_args, app_status );
	mbus_list_append( app_args, mbus_list_copy( ret->arguments ) );

	mbus_list_append( cmd->arguments, rpc_args );
	mbus_list_append( cmd->arguments, app_args );

	mbus_message_add( msg, cmd );

	/* cleanup */
	rpc_return_free( ret );

	return mbus_client_send( self->client, msg );
}

/**
\brief Sends an RPC command to the given destination
\param self The RPC entity sending the RPC command
\param cmd The RPC command information
\param addr The destination address
\param func THe callback function to invoke, when the RPC return is received
\param data An additional parameter passed to the callback function
\return TRUE if the RPC command could be send, FALSE otherwise
*/
gboolean
rpc_entity_send_command( RPCEntity * self, RPCCommand * cmd,
		const MAddress * addr, RPCReturnFunc func, gpointer data )
{
	MMessage *			msg = mbus_message_new();
	MObject *			rpc_args = mbus_list_new();
	MObject *			rpc_id = mbus_list_new();
	MObject *			rpc_type= mbus_list_new();
	MCommand *			command = mbus_command_new_with_name( cmd->command );
	gboolean			ret;
	RPCReturnCallback *	callback = g_new( RPCReturnCallback, 1 );

	/* add callback for RPC return command */
	mbus_callback_set( ( *callback ), func, data );
	g_hash_table_insert( self->returns,
			g_strdup( M_STRING( cmd->id )->str->str ), callback );

	/* prepare RPC command message */
	mbus_list_append( rpc_id, mbus_string_new( "ID" ) );
	mbus_list_append( rpc_id,
			mbus_string_new( M_STRING( cmd->id )->str->str ) );

	mbus_list_append( rpc_type, mbus_string_new( "RPC-TYPE" ) );

	switch ( cmd->type ) {
	case RPC_UNICAST:
		mbus_list_append( rpc_type, mbus_string_new( "UNICAST" ) );
		break;
	case RPC_ANYCAST:
		mbus_list_append( rpc_type, mbus_string_new( "ANYCAST" ) );
		break;
	}

	mbus_list_append( rpc_args, rpc_id );
	mbus_list_append( rpc_args, rpc_type );

	mbus_list_append( command->arguments, rpc_args );
	mbus_list_append( command->arguments, mbus_list_copy( cmd->arguments ) );

	mbus_message_add( msg, command );

	if ( cmd->type == RPC_UNICAST ) {
		g_assert( addr );
		ret = mbus_client_send_reliable( self->client, msg, addr );
	} else
		ret = mbus_client_send( self->client, msg );

	rpc_command_free( cmd );

	return ret;
}

/* end of rpcentity.c */
