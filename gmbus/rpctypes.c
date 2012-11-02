/* rpctypes.c
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

#include "gmbus/mbus.h"
#include "gmbus/mtypes.h"
#include "gmbus/mutil.h"
#include "gmbus/rpctypes.h"

#include <string.h>
#include <time.h>

static guint64 __rpc_id = 0;

/**
\addtogroup rpccommand RPCCommand
\{
*/

/**
\brief Creates a new RPC command object
\param name The command name
\return A pointer to the newly created RPC command object or NULL if an error
    has occured
*/
RPCCommand *
rpc_command_new( const gchar * name )
{
	GString *		tmp = g_string_new( NULL );
	RPCCommand *	self = g_new( RPCCommand, 1 );

	self->command = g_strdup( name );
	mbus_util_int64_to_str( tmp, __rpc_id++, FALSE );
	self->id = mbus_string_new( tmp->str );
	self->arguments = mbus_list_new();
	self->type = RPC_UNICAST;
	self->source = mbus_address_new();
	g_string_free( tmp, TRUE );

	return self;
}

RPCCommand *
rpc_command_new_from_command( MCommand * command )
{
	RPCCommand *	self = g_new( RPCCommand, 1 );
	MObject *		rpc_args = mbus_list_index( command->arguments, 0 );
	gint			i;

	self->command = g_strdup( command->cmd->str );
	self->id = mbus_string_new( NULL );
	self->type = RPC_UNICAST;
	self->source = mbus_address_new();

	if ( ! M_OBJECT_IS( mbus_list_index( rpc_args, 0 ), MLIST ) )
		goto failure;

	for ( i = 0; i < mbus_list_length( rpc_args ); i++ ) {
		MObject * pair = mbus_list_index( rpc_args, i );

		if ( M_OBJECT_IS( pair, MLIST ) ) {
			MObject *	key = mbus_list_index( pair, 0 );
			MObject *	value = mbus_list_index( pair, 1 );

			g_return_val_if_fail( key != NULL, NULL );
			g_return_val_if_fail( value != NULL, NULL );

			if ( !strcmp( M_STRING( key )->str->str, MBUS_RPC_ID ) )
				mbus_string_set( self->id, M_STRING( value )->str->str );
			else if ( strcmp( M_STRING( key )->str->str, MBUS_RPC_TYPE ) ) {
				if ( !strcmp( M_STRING( value )->str->str, "UNICAST" ) )
					self->type = RPC_UNICAST;
				else if ( !strcmp( M_STRING( value )->str->str, "ANYCAST" ) )
					self->type = RPC_ANYCAST;
			}
		} else
			goto failure;
	}
	self->arguments = mbus_list_copy(
			mbus_list_index( command->arguments , 1 ) );

	return self;

  failure:
	g_free( self->command );
	mbus_string_free( self->id );
	g_free( self );

	return NULL;
}

void
rpc_command_free( RPCCommand * self )
{
	M_OBJECT_ASSERT( self->arguments, MLIST );
	M_OBJECT_ASSERT( self->id, MSTRING );

	mbus_list_free( self->arguments );
	mbus_string_free( self->id );
	mbus_address_free( self->source );
	g_free( self->command );
	g_free( self );
}

/**
\}
*/

/**
\addtogroup rpcreturn RPCReturn
\{
*/

/**
\brief Creates a new RPCReturn object based on the information from an
    MCommand object
\param command the MCommand object
\return newly created RPCReturn object
*/
RPCReturn *
rpc_return_new_from_command( MCommand * command )
{
	RPCReturn *	ret = g_new( RPCReturn, 1 );
	MObject *	rpc_args = mbus_list_index( command->arguments, 0 );
	MObject *	app_args = mbus_list_index( command->arguments, 1 );
	gint		i;

	ret->command = g_strdup( command->cmd->str );
	ret->id = mbus_string_new( NULL );
	ret->status = RPC_OK;

	if ( ! M_OBJECT_IS( mbus_list_index( rpc_args, 0 ), MLIST ) )
		goto failure;

	for ( i = 0; i < mbus_list_length( rpc_args ); i++ ) {
		MObject * pair = mbus_list_index( rpc_args, i );

		if ( M_OBJECT_IS( pair, MLIST ) ) {
			MObject *	key = mbus_list_index( pair, 0 );
			MObject *	value = mbus_list_index( pair, 1 );

			g_return_val_if_fail( key != NULL, NULL );
			g_return_val_if_fail( value != NULL, NULL );

			if ( !strcmp( M_STRING( key )->str->str, MBUS_RPC_ID ) )
				mbus_string_set( ret->id, M_STRING( value )->str->str );
			else if ( strcmp( M_STRING( key )->str->str, MBUS_RPC_STATUS ) ) {
				if ( !strcmp( M_STRING( value )->str->str, MBUS_RPC_OK ) )
					ret->status = RPC_OK;
				else if ( !strcmp( M_STRING( value )->str->str,
								MBUS_RPC_UNKNOWN ) )
					ret->status = RPC_UNKNOWN;
				else
					ret->status = RPC_FAILED;
			}
		} else
			goto failure;
	}

	if ( app_args ) {
		MObject *	app_status = mbus_list_index( app_args, 0 );
		MObject *	return_args = mbus_list_index( app_args, 1 );

		if ( app_status && M_OBJECT_IS( app_status, MLIST ) &&
				return_args && M_OBJECT_IS( return_args, MLIST ) ) {
			MObject *	arg1 = mbus_list_index( app_status, 0 );
			MObject *	arg2 = mbus_list_index( app_status, 1 );
			MObject *	arg3 = mbus_list_index( app_status, 2 );

			if ( M_OBJECT_IS( arg1, MSYMBOL ) && M_OBJECT_IS( arg2, MSYMBOL ) &&
					M_OBJECT_IS( arg3, MSTRING ) ) {
				if ( !strcmp( M_SYMBOL( arg1 )->str->str, MBUS_RPC_OK ) )
					ret->app_status = RPC_OK;
				else
					ret->app_status = RPC_FAILED;

				ret->app_result = g_strdup( M_STRING( arg2 )->str->str );
				ret->app_description = g_strdup( M_STRING( arg3 )->str->str );
			} else {
				goto failure;
			}
			ret->arguments = mbus_list_copy( return_args );
		}
	}

	return ret;

  failure:
	g_free( ret->command );
	mbus_string_free( ret->id );
	g_free( ret );

	return NULL;
}

/**
\brief Creates a new RPCReturn object based on the information from an
    RPCCommand.
\param cmd the RPCCommand for that the RPCReturn object is created
\param status the application status send with the RPCReturn
\return a newly created RPCReturn object
*/
RPCReturn *
rpc_return_new( const RPCCommand * cmd, RPCStatus status )
{
	RPCReturn * ret = g_new( RPCReturn, 1 );

	ret->command = g_strdup( cmd->command );
	ret->id = mbus_string_copy( cmd->id );
	ret->status = status;
	ret->app_status = RPC_OK;
	ret->app_result = ret->app_description = NULL;
	ret->arguments = mbus_list_new();

	return ret;
}

/**
\brief Frees a RPCReturn object
\param self the RPCReturn object to free
*/
void
rpc_return_free( RPCReturn * self )
{
	g_free( self->command );
	mbus_string_free( self->id );
	if ( self->app_result ) g_free( self->app_result );
	if ( self->app_description ) g_free( self->app_description );
	mbus_list_free( self->arguments );
	g_free( self );
}

/**
\brief sets the arguments of the RPCReturn object
\param self The RPCReturn object to modify.
\param args The arguments for the RPCReturn. \a args must be of type MLIST.
*/
void
rpc_return_set_arguments( RPCReturn * self, MObject * args )
{
	M_OBJECT_ASSERT( args, MLIST );

	mbus_list_free( self->arguments );
	self->arguments = args;
}

/**
\}
*/

/* end of rpctypes.c */
