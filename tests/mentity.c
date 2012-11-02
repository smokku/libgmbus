/* mentity.c
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
#include "gmbus/mclient.h"

typedef struct {
  MAddress *	address;
  MClient *	client;
} MDestination;

void
unknown_message( const MMessage * msg, MClient * client )
{
  GString *	buf = g_string_new( NULL );

  mbus_message_as_string( ( MMessage * ) msg, buf );
  g_message( "mentity: incoming message: %s", buf->str );

  g_string_free( buf, TRUE );
}

gboolean
send_reliable( MDestination * dest )
{
  MMessage * msg = mbus_message_new();

  g_message( "sending reliable message" );
  mbus_message_add( msg, mbus_command_new( "test.reliable ()" ) );
  if ( !mbus_client_send_reliable( dest->client, msg, dest->address ) )
    g_warning( "sending message reliable failed" );

  return TRUE;
}

void
new_entity( const MAddress * addr, MClient * client )
{
  GString *		tmp = g_string_new( NULL );
  MDestination * 	dest = g_new( MDestination, 1 );

  mbus_address_as_string( ( MAddress * ) addr, tmp );

  g_message( "new entity: %s", tmp->str );
  dest->address = mbus_address_copy( ( MAddress * ) addr );
  dest->client = client;

  g_timeout_add( 1000, ( GSourceFunc ) send_reliable, dest );

  g_string_free( tmp, TRUE );
}

void
lost_entity( const MAddress * addr, MClient * client )
{
  GString * tmp = g_string_new( NULL );
  mbus_address_as_string( ( MAddress * ) addr, tmp );

  g_message( "lost entity: %s", tmp->str );

  g_string_free( tmp, TRUE );
}

void
transport_error( const MError * error, MClient * client )
{
  g_warning( "error [%d]: %s\n", error->type, error->message );
}

int
main( int argc, char * argv[] )
{
  MClient * client;

  mbus_init( NULL );

  client = mbus_client_new( "(app:mentity module:libmbusc type:test)", NULL );

  mbus_client_subscribe( client, MEVENT_UNKNOWN_MESSAGE,
      ( MCallbackFunc ) unknown_message, client );
  mbus_client_subscribe( client, MEVENT_LOST_ENTITY,
      ( MCallbackFunc ) lost_entity, client );
  mbus_client_subscribe( client, MEVENT_NEW_ENTITY,
      ( MCallbackFunc ) new_entity, client );
  mbus_client_subscribe( client, MEVENT_ERROR,
      ( MCallbackFunc ) transport_error, client );

  mbus_loop();

  return 0;
}


/* end of mentity.c */
