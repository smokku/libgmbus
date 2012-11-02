/* rpcclient.c
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

#include "gmbus/init.h"
#include "gmbus/rpcentity.h"

#include <signal.h>
#include <stdlib.h>

guint       timer = 0;
RPCEntity * entity;

void
__rpc_math_sum( const RPCReturn * ret, gpointer data )
{
  MObject *	res = mbus_list_index( ret->arguments, 0 );

  g_message( "received RPC return for RPC ID %s with %u arguments",
      M_STRING( ret->id )->str->str, mbus_list_length( ret->arguments ) );

  if ( res ) {
    GString * tmp = g_string_new( NULL );
    mbus_util_int64_to_str( tmp, M_INTEGER( res )->number, TRUE );
    g_message( "result is %s", tmp->str );
    g_string_free( tmp, TRUE );
  }
}

void
__rpc_math_sub( const RPCReturn * ret, gpointer data )
{
  MObject *	res = mbus_list_index( ret->arguments, 1 );

  g_message( "received RPC return for RPC ID %s with %d arguments",
      M_STRING( ret->id )->str->str, mbus_list_length( ret->arguments ) );

  if ( res ) {
    GString * tmp = g_string_new( NULL );
    mbus_util_int64_to_str( tmp,
	M_INTEGER( mbus_list_index( ret->arguments, 1 ) )->number, TRUE );
    g_message( "result is %s", tmp->str );
    g_string_free( tmp, TRUE );
  }
}

gboolean
__send_rpc_command( MAddress * addr )
{
  RPCCommand *		cmd = rpc_command_new( "math.sum" );

  mbus_list_append( cmd->arguments, mbus_integer_new( 5 ) );
  mbus_list_append( cmd->arguments, mbus_integer_new( 3 ) );

  rpc_entity_send_command( entity, cmd, addr, __rpc_math_sum, entity );

  return TRUE;
}

void
new_entity( const MAddress * addr, RPCEntity * entity )
{
  GString *		tmp = g_string_new( NULL );

  mbus_address_as_string( ( MAddress * ) addr, tmp );
  g_message( "new entity: %s", tmp->str );
  g_string_free( tmp, TRUE );

  __send_rpc_command( ( MAddress * ) addr );

  /* the MAddress exists as long as the entity is available */
  timer = g_timeout_add( 100, ( GSourceFunc ) __send_rpc_command,
      ( gpointer ) addr );
}

void
lost_entity( const MAddress * addr, RPCEntity * entity )
{
  GString *		tmp = g_string_new( NULL );

  mbus_address_as_string( ( MAddress * ) addr, tmp );
  g_message( "lost entity: %s", tmp->str );
  g_string_free( tmp, TRUE );

  if ( timer ) {
    g_source_remove( timer );
    timer = 0;
  }
}

static void
__log_handler( const gchar * log_domain, GLogLevelFlags log_level,
    const gchar * message, gpointer user_data )
{
  g_log_default_handler( log_domain, log_level, message, user_data );

  g_message( ">>> my log handler" );
  g_on_error_query( log_domain );
}

void
bye( int sig )
{
  g_message( "cleaning up" );
  rpc_entity_free( entity );
  exit( 0 );
}

int
main( int argc, char * argv[] )
{
  entity = rpc_entity_new( "(module:client app:test)", NULL );

  signal( SIGTERM, bye );

  mbus_init( NULL );
  g_log_set_always_fatal( G_LOG_LEVEL_CRITICAL );
  g_log_set_handler( "Glib",
      G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
      __log_handler, NULL );

  rpc_entity_subscribe( entity, MEVENT_NEW_ENTITY,
      ( MCallbackFunc ) new_entity, entity );
  rpc_entity_subscribe( entity, MEVENT_LOST_ENTITY,
      ( MCallbackFunc ) lost_entity, entity );

  mbus_loop();

  return 0;
}


/* end of rpcclient.c */
