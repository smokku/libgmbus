/* mconfig.c
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

#include "gmbus/base64.h"
#include "gmbus/mconfig.h"
#include "gmbus/mutil.h"

#include <netdb.h>
#include <string.h>
#include <netinet/in.h>

#include <glib.h>

static gchar *
__mbus_filename( void )
{
  const gchar * file = g_getenv( "MBUS" );

  if ( file && g_file_test( file, G_FILE_TEST_EXISTS ) )
    return g_strdup( file );

  return g_build_filename( g_get_home_dir(), ".mbus", NULL );
}

/**
\brief Creates a new Mbus configuration object
\param read If TRUE the Mbus configuration file read, otherwise not
\return A newly created Mbus configurayion object
*/
MConfig *
mbus_config_new( gboolean read )
{
  MConfig * config = g_new0( MConfig, 1 );

  config->encr_key = g_byte_array_new();
  config->hash_key = g_byte_array_new();
  config->unicast_addr = mbus_endpoint_new( AF_INET );
  mbus_endpoint_set_address4( config->unicast_addr, INADDR_ANY );
  config->group_addr = mbus_endpoint_new( AF_INET );

  if ( read ) mbus_config_read( config );

  return config;
}

/**
\brief Frees an Mbus configuration object
\param config The Mbus configuration object to free
*/
void
mbus_config_free( MConfig * config )
{
  g_byte_array_free( config->encr_key, TRUE );
  g_byte_array_free( config->hash_key, TRUE );
  if ( config->group_addr ) g_free( config->group_addr );
  if ( config->unicast_addr ) g_free( config->unicast_addr );

  g_free( config );
}

/**
\brief Reads a configuration for an Mbus session.
\param[out] c The configuration read from the file is stored in this object
\return TRUE If the configuration file could be found and read, otherwise FALSE
*/
gboolean
mbus_config_read( MConfig * c )
{
  static const gchar *	grp = "MBUS";
  gchar *		filename = __mbus_filename();
  GKeyFile *		file = g_key_file_new();
  GByteArray *		hash = g_byte_array_new();

  g_key_file_set_list_separator( file, ',' );
  if ( g_key_file_load_from_file( file, filename, G_KEY_FILE_NONE, NULL ) ) {
    gsize	length;
    gchar **	list;
    gchar *	tmp;

    /* HASHKEY */
    list = g_key_file_get_string_list( file, grp, "HASHKEY", &length, NULL );
    tmp = &( list[ 0 ][ 1 ] );

    if ( ! g_ascii_strcasecmp( tmp, "HMAC-MD5-96" ) )
      c->hash_algo = M_HMAC_MD5_96;
    else
      c->hash_algo = M_HMAC_MD5_96;

    tmp = list[ 1 ]; tmp[ strlen( tmp ) - 1 ] ='\0';
    while ( *tmp == ' ' ) tmp++;

    g_byte_array_append( hash, ( guchar * ) tmp, strlen( tmp ) );
    base64decode( hash, c->hash_key );
    g_strfreev( list );
    g_byte_array_set_size( hash, 0 );

    /* ADDRESS */
    tmp = g_key_file_get_string( file, grp, "ADDRESS", NULL );
    if ( tmp ) {
      if ( ! mbus_endpoint_set_address_from_string( c->group_addr, tmp ) )
	g_error( "failed to read multicast address from configuration file" );
      g_free( tmp );
    } else
      mbus_endpoint_set_address_from_string( c->group_addr, "224.255.222.239" );

    /* PORT */
    mbus_endpoint_set_port( c->group_addr,
	htons( g_key_file_get_integer( file, grp, "PORT", NULL ) ) );

    /* SCOPE */
    tmp = g_key_file_get_string( file, grp, "SCOPE", NULL );
    if ( !tmp || !strcmp( tmp, "HOSTLOCAL" ) )
      c->scope = HOSTLOCAL;
    else if ( !strcmp( tmp, "LINKLOCAL" ) )
      c->scope = LINKLOCAL;
    else
      c->scope = HOSTLOCAL;
    if ( tmp ) g_free( tmp );

    if ( c->scope == HOSTLOCAL ) {
      if ( mbus_endpoint_get_family( c->group_addr ) == AF_INET )
	mbus_endpoint_set_address4( c->unicast_addr, htonl( 0x7F000001 ) );
      else
	mbus_endpoint_set_address6( c->unicast_addr, in6addr_loopback );
    } else {
      if ( mbus_endpoint_get_family( c->group_addr ) == AF_INET )
	mbus_endpoint_set_address4( c->unicast_addr, INADDR_ANY );
      else
	mbus_endpoint_set_address6( c->unicast_addr, in6addr_any );
    }

    /* INTERFACE */
    tmp = g_key_file_get_string( file, grp, "INTERFACE", NULL );
    if ( tmp ) {
      mbus_endpoint_set_address_from_string( c->unicast_addr, tmp );
      g_free( tmp );
    }

    /* SEND_UNICAST */
    tmp = g_key_file_get_string( file, grp, "SEND_UNICAST", NULL );
    if ( tmp ) {
      c->send_unicast = ( !strcasecmp( tmp, "true" )
	  || !strcmp( tmp, "yes" ) || ! strcasecmp( tmp, "1" ) );
    } else {
      c->send_unicast = TRUE;
    }

    /* ENCRYPTION */
    c->encr_algo = M_NOENCR;
  } else {
    g_error( "filename not found\n" );
    g_free( filename );
    g_key_file_free( file );
    g_byte_array_free( hash, TRUE );

    return FALSE;
  }

  g_free( filename );
  g_key_file_free( file );
  g_byte_array_free( hash, TRUE );

  return TRUE;
}

static gboolean
__g_byte_array_is_equal( const GByteArray * lhs, const GByteArray * rhs )
{
  if ( lhs->len == rhs->len ) {
    gint i;

    for ( i = 0; i < lhs->len; i++ )
      if ( lhs->data[ i ] != rhs->data[ i ] ) return FALSE;

    return TRUE;
  }

  return FALSE;
}

/**
\brief Compares to Mbus configuration objects
\param lhs one of the Mbus configuration objects
\param rhs the other one
\return TRUE if both Mbus configuration objects are equal, otherwise FALSE
*/
gboolean
mbus_config_is_equal( const MConfig * lhs, const MConfig * rhs )
{
  if ( lhs->hash_algo == rhs->hash_algo ) {
    if ( ! __g_byte_array_is_equal( lhs->hash_key, rhs->hash_key ) )
      return FALSE;
  } else
    return FALSE;

  if ( lhs->encr_algo == rhs->encr_algo ) {
    if ( ! __g_byte_array_is_equal( lhs->encr_key, rhs->encr_key ) )
      return FALSE;
  } else
    return FALSE;

  if ( memcmp( lhs->group_addr, rhs->group_addr, sizeof( MEndpoint ) ) )
    return FALSE;

  return TRUE;
}

/* end of mbusconf.c */
