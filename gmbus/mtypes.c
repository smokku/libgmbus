/* mtypes.c
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

#include "gmbus/base64.h"
#include "gmbus/mtypes.h"
#include "gmbus/mutil.h"

/* Integer */
MObject *
mbus_integer_new( gint64 number )
{
  M_OBJECT_ALLOC( MInteger, MINTEGER );
  me->number = number;

  return obj;
}

MObject *
mbus_integer_copy( const MObject * mint )
{
  return mbus_integer_new( M_INTEGER( mint )->number );
}

void
mbus_integer_set( MObject * mint, gint64 number )
{
  M_OBJECT_ASSERT( mint, MINTEGER );
  M_INTEGER( mint )->number = number;
}

void
mbus_integer_free( MObject * mint )
{
  M_OBJECT_ASSERT( mint, MINTEGER );
  M_OBJECT_FREE( mint );
}

void
mbus_integer_as_string( MObject * mint, GString * buf )
{
  M_OBJECT_ASSERT( mint, MINTEGER );
  mbus_util_int64_to_str( buf, M_INTEGER( mint )->number, TRUE );
}


/* Float */
MObject *
mbus_float_new( gdouble number )
{
  M_OBJECT_ALLOC( MFloat, MFLOAT );
  me->number = number;

  return obj;
}

MObject *
mbus_float_copy( const MObject * mfloat )
{
  return mbus_float_new( M_FLOAT( mfloat )->number );
}

void
mbus_float_set( MObject * mfloat, gdouble number )
{
  M_OBJECT_ASSERT( mfloat, MFLOAT );
  M_FLOAT( mfloat )->number = number;
}

void
mbus_float_free( MObject * mfloat )
{
  M_OBJECT_ASSERT( mfloat, MFLOAT );
  M_OBJECT_FREE( mfloat );
}

void
mbus_float_as_string( MObject * mfloat, GString * buf )
{
  M_OBJECT_ASSERT( mfloat, MFLOAT );
  g_string_append_printf( buf, "%f", M_FLOAT( mfloat )->number );
}

/* String */
MObject *
mbus_string_new( const gchar * str )
{
  M_OBJECT_ALLOC( MString, MSTRING );
  me->str = g_string_new( str );

  return obj;
}

MObject *
mbus_string_copy( const MObject * mstring )
{
  return mbus_string_new( M_STRING( mstring )->str->str );
}

void
mbus_string_set( MObject * mstring, const gchar * str )
{
  M_OBJECT_ASSERT( mstring, MSTRING );
  g_string_assign( M_STRING( mstring )->str, str );
}

void
mbus_string_free( MObject * mstring )
{
  M_OBJECT_ASSERT( mstring, MSTRING );
  g_string_free( M_STRING( mstring )->str, TRUE );
  M_OBJECT_FREE( mstring );
}

void
mbus_string_as_string( MObject * mstring, GString * buf )
{
  M_OBJECT_ASSERT( mstring, MSTRING );
  g_string_append_printf( buf, "\"%s\"", M_STRING( mstring )->str->str );
}

/* Symbol */

static gboolean
__is_valid_symbol( const gchar * sym )
{
  while ( *sym != '\0' &&
	  ( g_ascii_isalpha( *sym ) || g_ascii_isdigit( *sym ) ||
	    *sym == '-' || *sym == '_' || *sym == '.' ) ) sym ++;

  return ( *sym == '\0' );
}

MObject *
mbus_symbol_new( const gchar * str )
{
  if ( !str || __is_valid_symbol( str ) ) {
    M_OBJECT_ALLOC( MString, MSYMBOL );
    me->str = g_string_new( str );

    return obj;
  }

  return NULL;
}

MObject *
mbus_symbol_copy( const MObject * msymbol )
{
  return mbus_symbol_new( M_SYMBOL( msymbol )->str->str );
}

gboolean
mbus_symbol_is_valid_char( gchar c )
{
  return ( g_ascii_isalpha( c ) || g_ascii_isdigit( c ) ||
      c == '-' || c == '_' || c == '.' );
}

gboolean
mbus_symbol_is_valid( const MObject * msymbol )
{
  gchar * walk;

  M_OBJECT_ASSERT( msymbol, MSYMBOL );
  walk = M_SYMBOL( msymbol )->str->str;
  while ( *walk ) {
    if ( ! mbus_symbol_is_valid_char( *walk ) )
      return FALSE;
    walk++;
  }

  return TRUE;
}

void
mbus_symbol_set( MObject * msymbol, const gchar * str )
{
  M_OBJECT_ASSERT( msymbol, MSYMBOL );

  if ( mbus_symbol_is_valid( msymbol ) )
    g_string_assign( M_SYMBOL( msymbol )->str, str );
  else
    g_error( "invalid characters in symbol (not set)" );
}

void
mbus_symbol_free( MObject * msymbol )
{
  M_OBJECT_ASSERT( msymbol, MSYMBOL );
  g_string_free( M_SYMBOL( msymbol )->str, TRUE );
  M_OBJECT_FREE( msymbol );
}

void
mbus_symbol_as_string( MObject * msymbol, GString * buf )
{
  M_OBJECT_ASSERT( msymbol, MSYMBOL );
  g_string_append( buf, M_SYMBOL( msymbol )->str->str );
}

/* Data */
MObject *
mbus_data_new( const guint8 * str, guint len, gboolean decode )
{
  M_OBJECT_ALLOC( MData, MDATA );

  if ( str ) {
    if ( decode ) {
      GByteArray * tmp = g_byte_array_sized_new( len );

      me->array = g_byte_array_new();
      g_byte_array_append( tmp, str, len );
      base64decode( tmp, me->array );

      g_byte_array_free( tmp, TRUE );
    } else {
      me->array = g_byte_array_sized_new( len );
      g_byte_array_append( me->array, str, len );
    }
  } else
    me->array = g_byte_array_new();

  return obj;
}

MObject *
mbus_data_copy( const MObject * mdata )
{
  return mbus_data_new( M_DATA( mdata )->array->data,
			M_DATA( mdata )->array->len, FALSE );
}

void
mbus_data_set( MObject * mdata, const guint8 * str, guint len )
{
  GByteArray * array;

  M_OBJECT_ASSERT( mdata, MDATA );
  array = M_DATA( mdata )->array;
  g_byte_array_remove_range( array, 0, array->len );
  g_byte_array_append( array, str, len );
}

void
mbus_data_free( MObject * mdata )
{
  M_OBJECT_ASSERT( mdata, MDATA );
  g_byte_array_free( M_DATA( mdata )->array, TRUE );
  M_OBJECT_FREE( mdata );
}

void
mbus_data_as_string( MObject * mdata, GString * buf )
{
  GByteArray * tmp = g_byte_array_new();

  M_OBJECT_ASSERT( mdata, MDATA );
  base64encode( M_DATA( mdata )->array, tmp );
  g_byte_array_prepend( tmp, ( guchar * ) "<", 1 );
  g_byte_array_append( tmp, ( guchar * ) ">", 1 );
  g_string_append_len( buf, ( gchar * ) tmp->data, tmp->len );

  g_byte_array_free( tmp, TRUE );
}

/* List */
MObject *
mbus_list_new( void )
{
  M_OBJECT_ALLOC( MList, MLIST );
  me->list = NULL;

  return obj;
}

MObject *
mbus_list_copy( const MObject * mlist )
{
  MObject * copy = mbus_list_new();
  GSList *	list;
  MObject *	object;

  M_OBJECT_ASSERT( mlist, MLIST );
  list = M_LIST( mlist )->list;

  while ( list ) {
    object = ( MObject * ) list->data;

    if ( M_OBJECT_IS( object, MINTEGER ) ) {
      mbus_list_append( copy, mbus_integer_copy( object ) );
    } else if ( M_OBJECT_IS( object, MFLOAT ) ) {
      mbus_list_append( copy, mbus_float_copy( object ) );
    } else if ( M_OBJECT_IS( object, MSTRING ) ) {
      mbus_list_append( copy, mbus_string_copy( object ) );
    } else if ( M_OBJECT_IS( object, MSYMBOL ) ) {
      mbus_list_append( copy, mbus_symbol_copy( object ) );
    } else if ( M_OBJECT_IS( object, MDATA ) ) {
      mbus_list_append( copy, mbus_data_copy( object ) );
    } else if ( M_OBJECT_IS( object, MLIST ) ) {
      mbus_list_append( copy, mbus_list_copy( object ) );
    }
    list = g_slist_next( list );
  }

  return copy;
}

void
mbus_list_free( MObject * mlist )
{
  GSList * list;

  M_OBJECT_ASSERT( mlist, MLIST );
  list = M_LIST( mlist )->list;

  while ( list ) {
    MObject * object = list->data;

    if ( M_OBJECT_IS( object, MINTEGER ) ) {
      mbus_integer_free( object );
    } else if ( M_OBJECT_IS( object, MFLOAT ) ) {
      mbus_float_free( object );
    } else if ( M_OBJECT_IS( object, MSTRING ) ) {
      mbus_string_free( object );
    } else if ( M_OBJECT_IS( object, MSYMBOL ) ) {
      mbus_symbol_free( object );
    } else if ( M_OBJECT_IS( object, MDATA ) ) {
      mbus_data_free( object );
    } else if ( M_OBJECT_IS( object, MLIST ) ) {
      mbus_list_free( object );
    }
    list = g_slist_next( list );
  }

  g_slist_free( M_LIST( mlist )->list );
  M_OBJECT_FREE( mlist );
}

void
mbus_list_append( MObject * mlist, MObject * element )
{
  M_OBJECT_ASSERT( mlist, MLIST );

  M_LIST( mlist )->list = g_slist_append( M_LIST( mlist )->list, element );
}

static gint
__first_non_whitespace( const gchar * buf )
{
  gint len = 0;
  while ( (buf[ len ] == ' ' ) || ( buf[ len ] == '\t' ) ) len++;

  return len;
}

gint
mbus_list_read( MObject * mlist, const guchar * buf )
{
  MList *	list;
  gint		len = 0;

  M_OBJECT_ASSERT( mlist, MLIST );
  list = M_LIST( mlist );
  len += __first_non_whitespace( ( gchar * ) buf );
  /* beginning of list */

  if ( buf[ len++ ] == '(' ) {
    const guchar *	start = &( buf[ len - 1 ] );
    guchar *		pos = ( guchar * ) ( start + 1 );
    guchar *		end = NULL;
    MObject *		obj = NULL;

    while ( *pos != '\0' && *pos != ')' ) {
      pos += __first_non_whitespace( ( gchar * ) pos );
      if ( *pos == '\0' || *pos == ')' ) break;
      if ( *pos == '"' ) {
	/* string */
	end = mbus_util_find_char( ++pos, '"' );
	if ( end ) {
	  *end = '\0';
	  obj = mbus_string_new( ( gchar * ) pos );
	} else
	  goto failed;
      } else if ( *pos == '<' ) {
	/* data */
	end = mbus_util_find_char( ++pos, '>' );
	if ( end ) {
	  *end = '\0';
	  obj = mbus_data_new( pos, ( gint ) ( end - pos ), TRUE );
	} else
	  goto failed;
      } else if ( *pos == '(' ) {
	/* list */
	gint len;

	obj = mbus_list_new();
	len = mbus_list_read( obj, pos );
        end = pos + len - 1;
      } else if ( g_ascii_isdigit( *pos ) || *pos == '-' ) {
	/* number */
	end = pos + 1;
	while ( *end != ' ' && *end != '\n' && *end != '\0' ) ++end;
	*end = '\0';
        if ( mbus_util_find_char( pos + 1, '.' ) ) {
          gdouble d = g_ascii_strtod( ( gchar * ) pos, NULL );
          obj = mbus_float_new( d );
        } else {
          gint64 i = ( gint64 ) g_ascii_strtoull( ( gchar * ) pos,
						  ( char ** ) NULL, 10 );
          obj = mbus_integer_new( i );
        }
      } else if ( g_ascii_isalpha( *pos ) && g_ascii_isupper( *pos ) ) {
	end = mbus_util_find_char( pos, ' ' );
	if ( end ) {
	  *end = '\0';
	  obj = mbus_symbol_new( ( gchar * ) pos );
          if ( !obj ) goto failed;
	} else
	  goto failed;
      }

      pos = ++end;

      list->list = g_slist_append( list->list, obj );
    }
    mlist->ok = TRUE;
    return ( pos - start + 1 );
  }

 failed:
  mlist->ok = FALSE;
  return -1;
}

void
mbus_list_as_string( MObject * mlist, GString * buf )
{
  GSList *	list;
  MObject *	object;

  M_OBJECT_ASSERT( mlist, MLIST );
  list = M_LIST( mlist )->list;

  g_string_append_c( buf, '(' );
  while ( list ) {
    object = ( MObject * ) list->data;

    if ( M_OBJECT_IS( object, MINTEGER ) ) {
      mbus_integer_as_string( object, buf );
    } else if ( M_OBJECT_IS( object, MFLOAT ) ) {
      mbus_float_as_string( object, buf );
    } else if ( M_OBJECT_IS( object, MSTRING ) ) {
      mbus_string_as_string( object, buf );
    } else if ( M_OBJECT_IS( object, MSYMBOL ) ) {
      mbus_symbol_as_string( object, buf );
    } else if ( M_OBJECT_IS( object, MDATA ) ) {
      mbus_data_as_string( object, buf );
    } else if ( M_OBJECT_IS( object, MLIST ) ) {
      mbus_list_as_string( object, buf );
    }
    list = g_slist_next( list );
    g_string_append_c( buf, ' ' );
  }
  g_string_append( buf, ")" );
}

guint
mbus_list_length( MObject * mlist )
{
  M_OBJECT_ASSERT( mlist, MLIST );

  return g_slist_length( M_LIST( mlist )->list );
}

MObject *
mbus_list_index( MObject * mlist, guint idx )
{
  M_OBJECT_ASSERT( mlist, MLIST );

  return g_slist_nth_data( M_LIST( mlist )->list, idx );
}

gboolean
mbus_list_remove( MObject * mlist, guint idx, gboolean remove_data )
{
  GSList * list;

  M_OBJECT_ASSERT( mlist, MLIST );

  list = g_slist_nth( M_LIST( mlist )->list, idx );
  if ( !list ) return FALSE;

  list = g_slist_remove( M_LIST( mlist )->list, list );
  if ( remove_data ) {
    MObject * object = ( MObject * ) list->data;

    if ( M_OBJECT_IS( object, MINTEGER ) ) {
      mbus_integer_free( object );
    } else if ( M_OBJECT_IS( object, MFLOAT ) ) {
      mbus_float_free( object );
    } else if ( M_OBJECT_IS( object, MSTRING ) ) {
      mbus_string_free( object );
    } else if ( M_OBJECT_IS( object, MSYMBOL ) ) {
      mbus_symbol_free( object );
    } else if ( M_OBJECT_IS( object, MDATA ) ) {
      mbus_data_free( object );
    } else if ( M_OBJECT_IS( object, MLIST ) ) {
      mbus_list_free( object );
    }
  }

  return TRUE;
}

/* end of mtypes.c */
