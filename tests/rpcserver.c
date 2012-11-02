/* rpcserver.c
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

#include <glib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "gmbus/init.h"
#include "gmbus/rpcentity.h"

RPCReturn *
__rpc_math_sum( const RPCCommand * command, gpointer data )
{
	RPCReturn *	ret = rpc_return_new( command, RPC_OK );
	MObject *	op1 = mbus_list_index( command->arguments, 0 );
	MObject *	op2 = mbus_list_index( command->arguments, 1 );
	GString *	tmp = g_string_new( NULL );

	mbus_list_as_string( command->arguments, tmp );
	g_message( "received RPC %s with %d arguments: %s",
			command->command, mbus_list_length( command->arguments ),
			tmp->str );
	g_string_free( tmp, TRUE );

	M_OBJECT_ASSERT( op1, MINTEGER );
	M_OBJECT_ASSERT( op2, MINTEGER );

	g_message( " -> %lld + %lld", M_INTEGER( op1 )->number,
			M_INTEGER( op2 )->number );
	if ( op1 && op2 ) {
		mbus_list_append( ret->arguments,
				mbus_integer_new( M_INTEGER( op1 )->number +
						M_INTEGER( op2 )->number ) );
	}

	return ret;
}

RPCReturn *
__rpc_math_sub( const RPCCommand * command, gpointer data )
{
	RPCReturn *	ret = rpc_return_new( command, RPC_OK );
	MObject *	op1 = mbus_list_index( command->arguments, 0 );
	MObject *	op2 = mbus_list_index( command->arguments, 1 );

	g_message( "received RPC %s with %d arguments",
			command->command, mbus_list_length( command->arguments ) );

	M_OBJECT_ASSERT( op1, MINTEGER );
	M_OBJECT_ASSERT( op2, MINTEGER );

	printf( " -> %lld + %lld", M_INTEGER( op1 )->number,
			M_INTEGER( op2 )->number );
	if ( op1 && op2 )
		mbus_list_append( ret->arguments,
				mbus_integer_new( M_INTEGER( op1 )->number -
						M_INTEGER( op2 )->number ) );

	return ret;
}

void
new_entity( const MAddress * addr, RPCEntity * entity )
{
	GString *		tmp = g_string_new( NULL );

	mbus_address_as_string( ( MAddress * ) addr, tmp );
	g_message( "new entity: %s", tmp->str );
	g_string_free( tmp, TRUE );
}

void
lost_entity( const MAddress * addr, RPCEntity * entity )
{
	GString *		tmp = g_string_new( NULL );

	mbus_address_as_string( ( MAddress * ) addr, tmp );
	g_message( "lost entity: %s", tmp->str );
	g_string_free( tmp, TRUE );
}

RPCEntity * entity;

void
bye( int sig )
{
	g_message( "cleaning up" );
	rpc_entity_free( entity );
	exit( 0 );
}

int
main( int argc, const char * argv[] )
{
	entity = rpc_entity_new( "(module:server app:test)", NULL );

	signal( SIGTERM, bye );

	mbus_init( NULL );

	rpc_entity_register( entity, "math.sum", __rpc_math_sum, entity );
	rpc_entity_register( entity, "math.sub", __rpc_math_sub, entity );

	rpc_entity_subscribe( entity, MEVENT_NEW_ENTITY,
			( MCallbackFunc ) new_entity, entity );
	rpc_entity_subscribe( entity, MEVENT_LOST_ENTITY,
			( MCallbackFunc ) lost_entity, entity );

	mbus_loop();

	return 0;
}


/* end of rpcserver.c */
