/* mtypes.h
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

#ifndef MBUS_TYPES_H
#define MBUS_TYPES_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
\addtogroup mbustypes Mbus Types
\{
*/

/**
\enum MType
\brief MObject base type
*/
typedef enum { MINTEGER, MFLOAT, MSTRING, MSYMBOL, MDATA, MLIST } MType;

/**
\struct MObject
\brief MObject is the base class for all Mbus types.
*/
typedef struct {
  MType		type;
  gpointer	data;
  gboolean	ok;
} MObject;

/**
\struct MInteger
\brief MInteger represents a whole-numbered value
*/
typedef struct {
  MObject	_object;
  gint64	number;
} MInteger ;

MObject * mbus_integer_new( gint64 number );
MObject * mbus_integer_copy( const MObject * mint );
void mbus_integer_free( MObject * mint );
void mbus_integer_set( MObject * mint, gint64 number );
void mbus_integer_free( MObject * mint );
void mbus_integer_as_string( MObject * mint, GString * buf );

/**
\struct MFloat
\brief MFloat represents a floating-point number
*/
typedef struct {
  MObject	_object;
  gdouble	number;
} MFloat ;

MObject * mbus_float_new( gdouble number );
MObject * mbus_float_copy( const MObject * mfloat );
void mbus_float_free( MObject * mfloat );
void mbus_float_set( MObject * mfloat, gdouble number );
void mbus_float_free( MObject * mfloat );

/**
\struct MString
\brief MString represents a character string
*/
typedef struct {
  MObject	_object;
  GString *	str;
} MString;

MObject * mbus_string_new( const gchar * str );
MObject * mbus_string_copy( const MObject * mstring );
void mbus_string_free( MObject * mstring );
void mbus_string_set( MObject * mstring, const gchar * str );
void mbus_string_free( MObject * mstring );
void mbus_string_as_string( MObject * mstring, GString * buf );

/**
\struct MSymbol
\brief MSymbol represents a character string with a restricted set of
  available characters
*/
typedef struct {
  MObject	_object;
  GString *	str;
} MSymbol;

MObject * mbus_symbol_new( const gchar * str );
MObject * mbus_symbol_copy( const MObject * msymbol );
void mbus_symbol_free( MObject * msymbol );
gboolean mbus_symbol_is_valid_char( gchar c );
gboolean mbus_symbol_is_valid( const MObject * msymbol );
void mbus_symbol_set( MObject * msymbol, const gchar * str );
void mbus_symbol_free( MObject * msymbol );
void mbus_symbol_as_string( MObject * msymbol, GString * buf );

/**
\struct MData
\brief MData represents binary data that is base64 encoded for transport
  via Mbus
*/
typedef struct {
  MObject	_object;
  GByteArray *	array;
} MData;

MObject * mbus_data_new( const guint8 * str, guint len, gboolean decode );
MObject * mbus_data_copy( const MObject * mdata );
void mbus_data_free( MObject * mdata );
void mbus_data_set( MObject * mdata, const guint8 * str, guint len );
void mbus_data_free( MObject * mdata );

/**
\struct MList
\brief MList represents a list of MObject instances.
*/
typedef struct {
  MObject	_object;
  GSList *	list;
} MList;

MObject * mbus_list_new( void );
MObject * mbus_list_copy( const MObject * mlist );
void mbus_list_free( MObject * mlist );
void mbus_list_append( MObject * mlist, MObject * element );
gint mbus_list_read( MObject * mlist, const guchar * buf );
void mbus_list_as_string( MObject * mlist, GString * buf );
guint mbus_list_length( MObject * mlist );
MObject * mbus_list_index( MObject * mlist, guint idx );
gboolean mbus_list_remove( MObject * mlist, guint idx, gboolean remove_data );

/* MACROS */
#define M_OBJECT_IS(Object,Type) ( Object && ( Object->type == Type ) )
#define M_OBJECT_ASSERT(Object,Type) g_assert( Object->type == Type )
#define M_OBJECT_OK(Object) ( Object->ok == TRUE )
#define M_OBJECT_ALLOC(Object,Type)		\
  MObject * obj = g_new( MObject, 1 );	\
  Object *  me = g_new( Object, 1 );	\
  obj->type = Type;			\
  obj->data = me;                       \
  obj->ok = TRUE;
#define M_OBJECT_FREE(Object)	\
  g_free( Object->data );		\
  g_free( Object );
#define M_INTEGER(Object) ( ( MInteger * ) Object->data )
#define M_FLOAT(Object) ( ( MFloat * ) Object->data )
#define M_STRING(Object) ( ( MString * ) Object->data )
#define M_SYMBOL(Object) ( ( MSymbol * ) Object->data )
#define M_DATA(Object) ( ( MData * ) Object->data )
#define M_LIST(Object) ( ( MList * ) Object->data )

/**
\}
*/
#ifdef __cplusplus
}
#endif

#endif

/* end of mtypes.h */
