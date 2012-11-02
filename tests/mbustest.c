/* mbustest.c
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

#include "gmbus/mclient.h"
#include "gmbus/init.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

void
whenNewMsg( const MMessage * msg, gpointer data )
{
  GString *	buf = g_string_new( NULL );

  mbus_message_as_string( ( MMessage * ) msg, buf );

  printf( "new message:\n%s", buf->str );

  g_string_free( buf, TRUE );
}

void
whenNewEntity( const MAddress * addr, MClient * client )
{
  GString * tmp = g_string_new( NULL );
  MMessage * msg = mbus_message_new();
  MCommand * cmd = mbus_command_new( "bla" );

  mbus_address_as_string( ( MAddress * ) addr, tmp );
  g_message( "new entity: %s", tmp->str );
  g_string_free( tmp, TRUE );

  mbus_message_add( msg, cmd );
  cmd = mbus_command_new( "fasel" );
  mbus_message_add( msg, cmd );

  mbus_client_send( client, msg );
}

void
whenLostEntity( const MAddress * addr, gpointer data )
{
}

void
whenError( const MError * error, gpointer data )
{
  printf( "Error [%d]: %s\n", error->type, error->message );
}

void
__bla( MMessage * msg, gpointer data )
{
  printf( "received: bla command\n" );
}

void
__fasel( MMessage * msg, gpointer data )
{
  printf( "received: fasel command\n" );
}

MClient *		client;
void
bye( int sig )
{
  g_message( "cleaning up" );
  mbus_client_free( client );
  exit( 0 );
}

int
main( int argc, char * argv[] )
{
  mbus_init( NULL );

  signal( SIGTERM, bye );
  client = mbus_client_new( "(app:mbustest)", NULL );

  mbus_client_subscribe( client, MEVENT_NEW_ENTITY,
      ( MCallbackFunc ) whenNewEntity, client );
  mbus_client_subscribe( client, MEVENT_LOST_ENTITY,
      ( MCallbackFunc ) whenLostEntity, client );
  mbus_client_subscribe( client, MEVENT_ERROR,
      ( MCallbackFunc ) whenError, client );
  mbus_client_subscribe( client, MEVENT_UNKNOWN_MESSAGE,
      ( MCallbackFunc ) whenNewMsg, client );

  mbus_client_register( client, "bla", ( MMessageFunc ) __bla, NULL );
  mbus_client_register( client, "fasel", ( MMessageFunc ) __fasel, NULL );
  if ( !mbus_client_ok( client ) ) {
    printf( "Error: MClient %s\n", strerror( errno ) );

    return -1;
  }

  mbus_loop();

  return 0;
}
