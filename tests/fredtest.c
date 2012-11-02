/* fredtest.c
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

void
__rpc_devices_list( const RPCReturn * ret, gpointer data )
{
  MObject *	res = mbus_list_index( ret->arguments, 0 );
  GString *     tmp = g_string_new( NULL );

  g_message( "received RPC return for RPC ID %s with %u arguments",
      M_STRING( ret->id )->str->str, mbus_list_length( ret->arguments ) );

  if ( res ) {
    mbus_list_as_string( ret->arguments, tmp );
    g_message( "result is %s", tmp->str );
    g_string_free( tmp, TRUE );
  }
}

void
new_entity( const MAddress * addr, RPCEntity * entity )
{
  GString *		tmp = g_string_new( NULL );
  RPCCommand *		cmd = rpc_command_new( "home-theatre.devices.list" );

  mbus_address_as_string( ( MAddress * ) addr, tmp );
  g_message( "new entity: %s", tmp->str );

  {
    GString * tmp = g_string_new( NULL );
    mbus_address_as_string( ( MAddress * ) addr, tmp );
    g_message( "%s", tmp->str );
    g_string_free( tmp, TRUE );
  }
  rpc_entity_send_command( entity, cmd, ( MAddress * ) addr,
      __rpc_devices_list, entity );

  g_string_free( tmp, TRUE );
}

static void
__log_handler( const gchar * log_domain, GLogLevelFlags log_level,
    const gchar * message, gpointer user_data)
{
  g_log_default_handler (log_domain, log_level, message, user_data);

  g_message( ">>> my log handler" );
  g_on_error_query( log_domain );
}

int
main( int argc, const char * argv[] )
{
  RPCEntity * entity = rpc_entity_new( "(module:fred app:test)", NULL );

  mbus_init( NULL );
  g_log_set_always_fatal( G_LOG_LEVEL_CRITICAL );
  g_log_set_handler( "Glib",
      G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
      __log_handler, NULL );

  rpc_entity_subscribe( entity, MEVENT_NEW_ENTITY,
      ( MCallbackFunc ) new_entity, entity );

  mbus_loop();

  return 0;
}


/* end of rpcclient.c */
