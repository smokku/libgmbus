/* mtest.c
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

#include "gmbus/init.h"
#include "gmbus/mcoder.h"
#include "gmbus/mmessage.h"

void parse_test(void)
{
  GString *	input = g_string_new( "mbus/1.0 0 952624915 U () () ()\n" );
  MMessage *	msg = mbus_message_new();

  g_string_append( input, "mbus.test(\"blub\")" );

  mbus_message_read( msg, input );
  if( mbus_message_ok( msg ) ) {
    g_string_set_size( input, 0 );
    printf( "OK!\n" );
    mbus_message_as_string( msg, input );
    printf( "%s\n", input->str );
  } else {
    printf( "NOT OK!\n" );
  }
  g_string_free( input, TRUE );
  mbus_message_free( msg );
}

void
coder_test( void )
{
  GString *	buf = g_string_new( NULL );
  GByteArray *	bytearray = g_byte_array_new();
  MConfig *	config = mbus_config_new( TRUE );
  MCoder *	coder = mbus_coder_new( config );
  MMessage *	msg = mbus_message_new();
  MCommand *	cmd = mbus_command_new( "mbus.test(bla)" );
  gint k;

  mbus_payload_add( msg->payload, cmd );
  mbus_message_as_string( msg, buf );

  mbus_coder_encode( coder, buf, bytearray );
  g_message( "coder_test: encoded buffer: " );
  for ( k = 0; k < bytearray->len; k++ )
    printf( "%c", bytearray->data[ k ] );
  printf( "\n" );
  g_string_set_size( buf, 0 );
  mbus_coder_decode( coder, bytearray, buf );
  g_message( "decoded message: %s\n", buf->str );

  g_byte_array_free( bytearray, TRUE );
  g_string_free( buf, TRUE );
  mbus_config_free( config );
  mbus_coder_free( coder );
  mbus_message_free( msg );
}

void
write_test( void )
{
  GString *	buf = g_string_new( NULL );
  MMessage *	msg = mbus_message_new();
  MCommand *	cmd = mbus_command_new( "mbus.test(bla)" );

  mbus_payload_add( msg->payload, cmd );

  mbus_message_as_string( msg, buf );

  printf( "%s\n", buf->str );
}

void
types_test( void )
{
  MObject *	list = mbus_list_new();
  MObject *	result = mbus_list_new();
  GString *	str = g_string_new( NULL );

  mbus_list_append( list, mbus_integer_new( 5 ) );
  mbus_list_append( list, mbus_float_new( 7.8 ) );
  mbus_list_append( list, mbus_string_new( "hello world" ) );
  mbus_list_append( list, mbus_symbol_new( "TEST_SYMBOL" ) );
  mbus_list_append( list, mbus_data_new( ( guchar * ) "data", 4, FALSE ) );

  mbus_list_as_string( list, str );

  printf( "test as_string: %s\n", str->str );

  g_string_erase( str, 0, -1 );
  mbus_list_as_string( result, str );
  printf( "test read: %s\n", str->str );
}

int
main( int argc, char * argv[] )
{
  mbus_init( NULL );

  g_message( ">>>>>> parse test >>>>>>" );
  parse_test();
  g_message( ">>>>>> write test >>>>>>" );
  write_test();
  g_message( ">>>>>> coder test >>>>>>" );
  coder_test();
  g_message( ">>>>>> types test >>>>>>" );
  types_test();

  return 0;
}
