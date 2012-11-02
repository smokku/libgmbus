/* mheader.c
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

#include "gmbus/mheader.h"
#include "gmbus/mutil.h"

#include <time.h>
#include <string.h>
#include <ctype.h>

static const char * pvers = "mbus/1.0";

static void
__mbus_header_init( MHeader * self )
{
	self->type = UNRELIABLE;
	self->sequence_no = 0UL;
	self->timestamp = 0UL;
	self->source = mbus_address_new();
	self->destination = mbus_address_new();
	self->acks= g_array_new( FALSE, TRUE, sizeof( guint64 ) );
	self->ok = TRUE;
}

MHeader *
mbus_header_new( void )
{
	MHeader * self = g_new( MHeader, 1 );

	g_assert( self );
	__mbus_header_init( self );

	return self;
}

static void
__mbus_header_finalize( MHeader * self )
{
	mbus_address_free( self->source );
	mbus_address_free( self->destination );

	g_array_free( self->acks, TRUE );
}

void
mbus_header_free( MHeader * self )
{
	__mbus_header_finalize( self );
	g_free( self );
}

gboolean
mbus_header_read( MHeader * self, GString * buf )
{
	gint		pvlen = strlen( pvers );
	gchar		numbuf[ 64 ];
	gchar *		pos;
	gint		nc = 0;

	self->ok = FALSE;

	mbus_util_cut_whitespaces( buf );

	if ( strncmp( buf->str, pvers, pvlen ) != 0 ) return FALSE;
	g_string_erase( buf, 0, pvlen );
	mbus_util_cut_whitespaces( buf );

	/* copy version number */
	numbuf[ 0 ] = '\0';
	pos = buf->str;
	while( isdigit( *pos ) && ( nc < 64 ) ) {
		numbuf[ nc++ ] = *pos; ++pos;
	}
	numbuf[ nc ] = '\0';

	if ( !mbus_util_str_to_uint64( numbuf, &self->sequence_no ) ||
			!isspace( *pos ) )
		return FALSE;

	g_string_erase( buf, 0, nc + 1 );
	mbus_util_cut_whitespaces( buf );
	pos = buf->str;
	nc = 0;
	while ( isdigit( *pos ) && ( nc < 64 ) ) {
		numbuf[ nc++ ] = *pos; ++pos;
	}
	numbuf[ nc++ ] = '\0';
	if ( !mbus_util_str_to_uint64( numbuf, &self->timestamp ) ||
			!isspace( *pos ) )
		return FALSE;

	/* cut behind terminating zero (after timestamp) */
	g_string_erase( buf, 0, nc );
	/* message type */
	switch( buf->str[ 0 ] ) {
	case 'U':
		self->type = UNRELIABLE;
		break;
	case 'R':
		self->type = RELIABLE;
		break;
	default:
		return FALSE;			/* error */
	}
	++pos;

	g_string_erase( buf, 0, 1 );
	mbus_util_cut_whitespaces( buf );

	mbus_address_read( self->source, buf );
	if ( !mbus_address_ok( self->source ) ) return FALSE;

	mbus_address_read( self->destination, buf );
	if ( !mbus_address_ok( self->destination ) ) return FALSE;

	mbus_util_cut_whitespaces( buf );
	/* reading acknowledgement list */
	if ( buf->str[ 0 ] != '(' ) {
		return FALSE;
	} else {
		gboolean finished = FALSE;

		g_string_erase( buf, 0, 1 );

		while ( !finished ) {
			guint64 v;

			mbus_util_cut_whitespaces( buf );
			pos = buf->str;
			nc = 0;
			if ( *pos == ')' ) {
				finished = TRUE;
			} else {
				while ( isdigit( *pos ) && ( nc < 64 ) ) {
					numbuf[ nc++ ] = *pos; ++pos;
				}
				numbuf[ nc ] = '\0';
				if ( ( !isspace( *pos ) || *pos == ')' ) ||
						! mbus_util_str_to_uint64( numbuf, &v ) )
					return FALSE;

				g_array_append_val( self->acks, v );
				g_string_erase( buf, 0, nc );
			}
		} /* while(!finished) */
	}

	g_string_erase( buf, 0, ( gsize ) ( pos - buf->str ) );

	self->ok = TRUE;

	return TRUE;
}

gboolean
mbus_header_as_string( MHeader * self, GString * buf )
{
	gchar		type;
	GTimeVal	now;
	guint64		timestamp;
	guint		i;

	g_get_current_time( &now );
	self->ok = FALSE;
	/* calc type */
	switch ( self->type ) {
	case RELIABLE:
		type = 'R';
		break;
	case UNRELIABLE:
	default:
		type = 'U';
		break;
	}

	/* <protocol version> <sequence no> <timestamp> <type> */
	timestamp = ( guint64 ) now.tv_sec * 1000 + ( guint64 ) now.tv_usec / 1000;
	g_string_append( buf, pvers );
	g_string_append_c( buf, ' ' );
	mbus_util_int64_to_str( buf, self->sequence_no, FALSE );
	g_string_append_c( buf, ' ' );
	mbus_util_int64_to_str( buf, timestamp, FALSE );
	g_string_append_c( buf, ' ' );
	g_string_append_c( buf, type );
	g_string_append_c( buf, ' ' );

	/* source address */
	mbus_address_as_string( self->source, buf );
	g_string_append_c( buf, ' ' );

	/* destination address */
	mbus_address_as_string( self->destination, buf );
	g_string_append( buf, " (" );

	/* acknowledgements */
	for ( i = 0; i < self->acks->len; i++ )
		g_string_append_printf( buf, "%ld ",
			    g_array_index( self->acks, glong, i ) );

	g_string_append_c( buf, ')' );

	self->ok = TRUE;

	return TRUE;
}

MHeader *
mbus_header_assign( MHeader * self, MHeader * rhs )
{
	if ( self != rhs ) {
		self->type = rhs->type;
		self->pvers = rhs->pvers; /* pvers isn't strduped */
		self->sequence_no = rhs->sequence_no;
		self->timestamp = rhs->timestamp;
		mbus_address_assign( self->source, rhs->source );
		mbus_address_assign( self->destination, rhs->destination );

		/* copy array of long's */
		g_array_free( self->acks, TRUE );
		self->acks = g_array_sized_new( FALSE, TRUE, sizeof( glong ),
				rhs->acks->len );
		g_array_append_vals( self->acks, rhs->acks->data, self->acks->len );

		self->ok = rhs->ok;
	}

	return self;
}
