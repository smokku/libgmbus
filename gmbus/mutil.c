/* mutil.c
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

#include "gmbus/mutil.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

/**
\brief cuts all trailing whitespaces from the buffer
\param[out] buf the string
*/
void
mbus_util_cut_whitespaces( GString * buf )
{
	gchar * pos = buf->str;

	while( g_ascii_isspace( *pos ) || *pos == '\0' ) pos++;

	g_string_erase( buf, 0, pos - buf->str );
}

/**
\brief walks through the string starting at \a start and searches for the
    character \a c, when found its position is returned.
\param start pointer to the string
\param c character to search for
\return The first position of character \a c if found, otherwise NULL is
    returned
*/
guchar *
mbus_util_find_char( const guchar * start, guchar c )
{
	while ( *start != '\0' && *start != c )  start++;

	return ( guchar * ) ( *start != '\0' ? start : NULL );
}

/**
\brief converts the string \a buf to a long integer and stores it in \a v.
\param buf string to convert
\param[out] v pointer to a long, used to store the result
\return TRUE if the conversion was successful, FALSE otherwise
*/
gboolean
mbus_util_str_to_uint64( const gchar * buf, guint64 * v)
{
	errno = 0;
	*v = ( guint64 ) strtoull( buf, NULL, 10 );

	return ( errno != EINVAL ) && ( errno != ERANGE );
}

/**
\brief sets the error identifer and creates an error message by using printf
\param[out] self the error object
\param id the new error identifier
\param format the format string for the error message
\param ... optional arguments passed to printf
*/
void
mbus_error_printf( MError * self, MErrorType id, const gchar * format, ... )
{
	va_list       args;

	g_assert( self );
	va_start( args, format );

	self->type = id;
	self->message = g_strdup_vprintf( format, args );
	va_end( args );
}

/**
\brief converts a 64bit integer number into a string and appends it to
the GString
\param[out] buf the result will be appended to this string buffer
\param number the integer number to convert
\param sign if TRUE the number is treated as a signed 64 bit integer
otherwise as a unsigned one
*/
void
mbus_util_int64_to_str( GString * buf, guint64 number, gboolean sign )
{
	gchar		buffer[ 128 ];
	gchar *		bufferend;
	gchar *		pointer;
	gboolean	negative = FALSE;
	gint		i;

	static const gchar *	digits = "0123456789ABCDEFX";

	if ( sign && ( gint64 ) number < 0 ) negative = TRUE;
	if ( negative ) number = - ( ( gint64 ) number );

	/* Build number */
	pointer = bufferend = &buffer[ sizeof( buffer ) - 1 ];
	*pointer-- = '\0';

	for ( i = 1; i < ( gint ) sizeof( buffer ); i++ ) {
		*pointer-- = digits[ number % 10 ];
		number /= 10;
		if ( number == 0 ) break;
	}

	if ( negative )
		g_string_append_c( buf, '-' );
	g_string_append( buf, ++pointer );
}

/**
\brief Verifies that an argument list matches a given pattern.

The string pattern describes all acceptable variations of Mbus basic
types by representing each type by one single letter. An exception is
the Mbus list object that is is represented by two angle brackets
enclosing the the list elements representations. The following table
shows a list of possible types and the representation:

\li \c i MInteger
\li \c f MFloat
\li \c s MString
\li \c y MSymbol
\li \c d MData
\li \c <...> MList

For example, the pattern \c "<iify>" describes a list with four
elements. The first two elements are of type MInteger, the third
should be a MFloat and the fourth a MSymbol. If the given parameter
arguments matches this specification the function returns
TRUE. Otherwise FALSE is returned and if the third parameter is valid
GString object a description of the failure is append to it.

\param pattern describes the specification
\param arguments points to the arguments to check
\param[out] error if the match fails this parameter contains an error
description
\return TRUE if the pattern matches otherwise FALSE
*/
gboolean
mbus_argument_check( const gchar * pattern, const MObject * arguments,
		GString * error )
{
	GString *	args = g_string_new( NULL );
	guint		i;

	M_OBJECT_ASSERT( arguments, MLIST );
	g_string_set_size( args, 10 );

	for ( i = 0; i < mbus_list_length( ( MObject * ) arguments ); i++ ) {
		MObject * elem = mbus_list_index( ( MObject * ) arguments, i );

		switch ( elem->type ) {
		case MINTEGER:
		case MFLOAT:
		case MSTRING:
		case MSYMBOL:
		case MDATA:
		case MLIST:
			break;

		}
	}

	return TRUE;
}
