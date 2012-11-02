/* maddress.c
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

#include "gmbus/maddress.h"
#include "gmbus/mutil.h"
#include "gmbus/udp.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

/**
\addtogroup maddresselement MAddressElement
\{
*/

/**
\brief Parses an Mbus address element.
\param[out] self The Mbus address element for storing the parsed data
\param buf The string buffer containing the Mbus address element
\return TRUE if the address element could be parsed, otherwise FALSE
*/
gboolean
mbus_address_element_read( MAddressElement * self, GString * buf )
{
	static gchar	tmp[ M_ADDRESS_ELEMENT_SIZE ];

	gint		el = 0;
	gchar *	pos;

	self->ok = FALSE;

	tmp[ 0 ] = '\0';
	self->key = self->value = NULL;

	/* skip white space */
	mbus_util_cut_whitespaces( buf );
	pos = buf->str;

	/* read key */
	while ( g_ascii_isalnum( *pos ) && ( el < M_ADDRESS_ELEMENT_SIZE ) ) {
		tmp[ el++ ] = *pos; pos++;
	}
	tmp[ el ] = '\0';
	if ( *pos != ':' ) return FALSE;
	self->key = g_strdup( tmp );
	g_string_erase( buf, 0, el + 1 );

	/* read value */
	pos = buf->str;
	el = 0;
	while ( g_ascii_iscntrl( *pos ) == FALSE && ( *pos != ' ' ) &&
			( *pos != ')' ) && ( el < M_ADDRESS_ELEMENT_SIZE ) ) {
		tmp[ el++ ] = *pos;
		pos++;
	}
	tmp[ el ] = '\0';
	self->value = g_strdup( tmp );
	g_string_erase( buf, 0, el );

	self->ok = TRUE;

	return TRUE;
}

/**
\brief Creates a string representation of an Mbus address element
\param self the Mbus address element
\param[out] buf the buffer to store the string
*/
gboolean
mbus_address_element_as_string( MAddressElement * self, GString * buf )
{
	if ( self->key && self->value )
		g_string_append_printf( buf, "%s:%s", self->key, self->value );

	return TRUE;
}

/**
\brief Compares two Mbus address elements
\param self one Mbus address element
\param other the other Mbus address element
\return TRUE if both address elements are equal, otherwise FALSE
*/
gboolean
mbus_address_element_is_equal( const MAddressElement * self,
		const MAddressElement * other )
{
	if ( self == other ) return TRUE;
	if ( strcmp( self->key, other->key ) ) return FALSE;
	if ( self->value[ 0 ] == '*' || other->value[ 0 ] == '*' ) return TRUE;
	if ( strcmp( self->value, other->value ) ) return FALSE;

	return TRUE;
}

/**
\}
*/

/**
\addtogroup MAddress
\{
An MAddress object consists of a list of MAddressElement objects.
*/

static void
__mbus_address_init( MAddress * self )
{
	self->elements = g_ptr_array_sized_new( 3 );
	self->ok = TRUE;
}

/**
\brief Creates a new Mbus address object
\return A new MAddress object or NULL if allocation failed
*/
MAddress *
mbus_address_new( void )
{
	MAddress * self = g_new( MAddress, 1 );

	g_assert( self );
	__mbus_address_init( self );

	return self;
}

static void
__mbus_address_finalize( MAddress * self )
{
	gint i;

	for ( i = 0; i < self->elements->len; i++ ) {
		MAddressElement * elem = self->elements->pdata[ i ];

		g_free( elem->key );
		g_free( elem->value );
		g_free( elem );
	}

	g_ptr_array_free( self->elements, TRUE );
}

/**
\brief Frees an Mbus address object
\param self the Mbus address object to free
*/
void
mbus_address_free( MAddress * self )
{
	__mbus_address_finalize( self );
	g_free( self );
}

/**
\brief Copies an Mbus address
\param self the Mbus address object to copy
\return the newly created Mbus address object being a copy of \a self
*/
MAddress *
mbus_address_copy( MAddress * self )
{
	MAddress * copy = mbus_address_new();

	mbus_address_assign( copy, self );

	return copy;
}

gint
__elements_sort( const MAddressElement ** a, const MAddressElement ** b )
{
	return strcmp( ( *a )->key, ( *b )->key );
}

static gchar *
__generate_id( void )
{
	static guint		counter = 0;

	gint			fd = socket( PF_INET, SOCK_DGRAM, 0 );
	MEndpoint *           ep = mbus_endpoint_new( AF_INET );
	guint                 size;
	gchar *               result;

	mbus_endpoint_set_address4( ep, 0xe0e0e0e0 );
	mbus_endpoint_set_port( ep, 44444 );
	connect( fd, ( struct sockaddr * ) ep, mbus_endpoint_get_size( ep ) );
	size = mbus_endpoint_get_size( ep );
	getsockname( fd, ( struct sockaddr * ) ep, &size );
	close( fd );

	result =  g_strdup_printf( "%d-%d@%s", getpid(), counter++,
			inet_ntoa( ep->addr.in.sin_addr ) );
	g_free( ep );

	return result;
}

/**
\brief Parses an Mbus address
\param self the Mbus address object storing the parsed data
\param buf containing the string to parse
\return TRUE of the Mbus address could be parsed, otherwise FALSE
*/
gboolean
mbus_address_read( MAddress * self, GString * buf )
{
	gboolean finished = FALSE;

	self->ok = FALSE;

	mbus_util_cut_whitespaces( buf );
	if ( buf->str[ 0 ] != '(' ) return FALSE;

	g_string_erase( buf, 0, 1 );

	while ( !finished ) {
		MAddressElement * e = g_new( MAddressElement, 1 );

		mbus_address_element_read( e, buf );

		if ( e->ok ) {
			g_ptr_array_add( self->elements, e );
			if ( buf->str[ 0 ] == ')' ) finished = TRUE;
		} else {
			g_free( e );
			finished = TRUE;
		}
	} /* while(!finished) */

	mbus_util_cut_whitespaces( buf );
	if ( buf->str[ 0 ] != ')' ) return FALSE;
	g_string_erase( buf, 0, 1 );

	g_ptr_array_sort( self->elements, ( GCompareFunc ) __elements_sort );

	self->ok = TRUE;

	return TRUE;
}

/**
\brief Creates an unique Mbus address
\param[out] self the Mbus address to disambiguate
*/
void
mbus_address_disambiguate( MAddress * self )
{
	gint		i;
	gboolean	finished = FALSE;

	finished = FALSE;
	for ( i = 0; i < self->elements->len; i++ ) {
		MAddressElement * elem = g_ptr_array_index( self->elements, i );

		if ( ! strcmp( elem->key, "id" ) ) {
			finished = TRUE;
			break;
		}
	}
	if ( finished == FALSE ) {
		MAddressElement * e = g_new( MAddressElement, 1 );

		e->key = g_strdup( "id" );
		e->value = __generate_id();

		g_ptr_array_add( self->elements, e );
	}

	g_ptr_array_sort( self->elements, ( GCompareFunc ) __elements_sort );
}

/**
\brief Searches for a given Mbus address element
\param self the Mbus address searched for the address element
\param key the key of the Mbus address element to find
\return A pointer to the Mbus address element with the given key. If the
    element could not be found NULL is returned.
*/
MAddressElement *
mbus_address_find( MAddress * self, const gchar * key )
{
	gint i;

	for ( i = 0; i < self->elements->len; i++ ) {
		MAddressElement * element = g_ptr_array_index( self->elements, i );

		if ( !strcmp( element->key, key ) ) return element;
	}

	return NULL;
}

/**
\brief Creates a string representation of an Mbus address
\param self the Mbus address to create a string from
\param[out] buf string buffer for the string representation
\return TRUE if the creation of the strin representation was successful,
    otherwise FALSE
*/
gboolean
mbus_address_as_string( MAddress * self, GString * buf )
{
	gint i;

	/* format string */
	g_string_append_c( buf, '(' );

	for ( i = 0; i < self->elements->len; i++ ) {
		MAddressElement * elem = g_ptr_array_index( self->elements, i );

		if ( !mbus_address_element_as_string( elem, buf ) )
			return FALSE;
		g_string_append_c( buf, ' ' );
	}
	g_string_append_c( buf, ')' );

	self->ok = TRUE;

	return TRUE;
}

/**
\brief Copies the Mbus address \a other
\param self the Mbus address storing the copy of \a other
\param other the Mbus address that is copied to \a self
\return pointer to the modified Mbus address object \a self
*/
MAddress *
mbus_address_assign( MAddress * self, const MAddress * other )
{
	if ( self != other ) {
		gint i;

		for ( i = 0; i < self->elements->len; i++ ) {
			MAddressElement * elem = self->elements->pdata[ i ];

			g_free( elem->key );
			g_free( elem->value );
			g_free( elem );
		}

		g_ptr_array_free( self->elements, TRUE );
		self->elements = g_ptr_array_sized_new( other->elements->len );
		for ( i = 0; i < other->elements->len; i++ ) {
			MAddressElement * elem = g_ptr_array_index( other->elements, i );
			MAddressElement * nelem = g_new( MAddressElement, 1 );

			nelem->key = g_strdup( elem->key );
			nelem->value = g_strdup( elem->value );

			g_ptr_array_add( self->elements, nelem );
		}

		self->ok = other->ok;
	}

	return self;
}

/**
\brief Compares to Mbus address objects
\param self first of the Mbus addresses to compare
\param other second of the Mbus addresses to compare
\return TRUE if the Mbus addresses are equal, otherwise FALSE
*/
gboolean
mbus_address_is_equal( const MAddress * self, const MAddress * other )
{
	GPtrArray *	my = self->elements;
	GPtrArray *	your = other->elements;
	gint		i;

	if ( my == your ) return TRUE;
	if ( my->len != your->len ) return FALSE;

	for ( i = 0; i < my->len; i++ ) {
		MAddressElement * my_elem = g_ptr_array_index( my, i );
		MAddressElement * your_elem = g_ptr_array_index( your, i );

		if ( !mbus_address_element_is_equal( my_elem, your_elem ) )
			return FALSE;
	}

	return TRUE;
}

/**
\brief Tests if the Mbus address is unique
\param self the Mbus address to test
\return TRUE if the Mbus address is unique, FALSE otherwise
*/
gboolean
mbus_address_is_unique( const MAddress * self )
{
	return ( mbus_address_find( ( MAddress * ) self, "id" ) != NULL );
}

/**
\brief Generates a hash value of an Mbus address that can can be used as a
    key for GHashTable objects
\param self the Mbus address to create a hash for
\return the hash value
*/
guint
mbus_address_hash( const MAddress * self )
{
	GString *			addr_str;
	guint				hash;
	MAddressElement *	element;

	element = mbus_address_find( ( MAddress * ) self, "id" );

	if ( !element ) return 0;

	addr_str = g_string_new( NULL );
	g_string_append( addr_str, element->value );
	hash = g_string_hash( addr_str );
	g_string_free( addr_str, TRUE );

	return hash;
}

/**
\brief Checks if an Mbus address is a subset of another
\param self the Mbus address that should be a subset of the other
\param other an Mbus address just defining a few Mbus address elements
\return TRUE if the Mbus address \a self is a subset of \a other
*/
gboolean
mbus_address_is_subset_of( const MAddress * self, const MAddress * other )
{
	GPtrArray *	my = self->elements;
	GPtrArray *	your = other->elements;
	gint		i, j;

	if ( my == your ) return TRUE;

	for ( j = 0; j < your->len; j++ ) {
		MAddressElement * your_elem = g_ptr_array_index( your, j );
		gboolean found = FALSE;

		for ( i = 0; i < my->len; i++ ) {
			MAddressElement * my_elem = g_ptr_array_index( my, i );

			if ( mbus_address_element_is_equal( my_elem, your_elem ) ) {
				found = TRUE;
				break;
			}
		}
		if ( !found ) return FALSE;
	}

	return TRUE;
}

/**
\}
*/
