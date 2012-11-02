/* mutil.h
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

#ifndef MBUS_UTIL_H
#define MBUS_UTIL_H

#include <glib.h>

#include "gmbus/mmessage.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
\brief codes of possible errors
*/
typedef enum {
	MERR_NOERR, MERR_MULTICAST_JOIN, MERR_UNICAST, MERR_CONFIG, MERR_READ,
	MERR_CODER_SETUP, MERR_DECODE, MERR_PARSE, MERR_WRITE, MERR_ENCODE,
	MERR_SEND, MERR_SEND_RELIABLE
} MErrorType;

/**
\brief describes an error that has occured with an unique identifier and
  an error message
*/
typedef struct {
	MErrorType	type;
	gchar *		message;
} MError;

/**
\brief is the generic callback function prototype with two gpointer
    arguments no return type. All callback function types used in gmbus
    can be casted to this type as a generic argument to functions.
*/
typedef void ( *MCallbackFunc )( gconstpointer * arg1, gpointer arg2 );

/**
\brief is a callback function used to notify an application of a new
    incoming message
\sa MMessage
*/
typedef void ( *MMessageFunc )( const MMessage * m, gpointer data );

/**
\brief is a callback function used to notify an application of a new or
    lost entity in its Mbus session
\sa MAddress
*/
typedef void ( *MAddressFunc )( const MAddress * a, gpointer data );

/**
\brief is a callback function used to notify an application that an error
    has occured
\sa MError
*/
typedef void ( *MErrorFunc )( const MError * error, gpointer data );

/**
\brief callback function with two arguments. The first one is a pointer to
  an MMessage and the second is a gpointer.
*/
typedef struct {
	MMessageFunc	function;
	gpointer		data;
} MMessageCallback;

/**
\brief callback function with two arguments. The first on is a pointer to
  an MAddress and the second is a gpointer.
*/
typedef struct {
	MAddressFunc	function;
	gpointer		data;
} MAddressCallback;

/**
\brief callback function with two arguments. The first on is a pointer to
  an MError and the second is a gpointer.
*/
typedef struct {
	MErrorFunc	function;
	gpointer	data;
} MErrorCallback;

/**
\brief initialize the callback object
\param[out] self any type of known callback Object, e.g. MMessageCallback,
    MAddressCallback, MErrorCallback
*/
#define mbus_callback_init( self ) 			\
	self.function = NULL;		   			\
	self.data = NULL;

/**
\brief set the callback object to a specific function and defines the second
    argument
\param[out] self the callback object
\param func a pointer to the function this object shoudl point to
\param arg the value of the second generic argument
*/
#define mbus_callback_set( self, func, arg )		\
	self.function = func;							\
	self.data = arg;

/**
\brief invoke the function of the callback object with the given parameter as
    first argument
\param self the callback object
\param object the value of the first argument for the function
*/
#define mbus_callback_invoke( self, object )		\
	self.function( object, self.data )

/**
\brief checks if function and second argument are valid
\param self the callback object to check
*/
#define mbus_callback_ok( self )			\
	( self.function )

/**
\brief initializes an MError object
\param[out] self the error object
*/
#define mbus_error_init( self )				\
	self.type = MERR_NOERR;					\
	self.message = NULL;

/**
\brief set the error objects id and the message text
\param[out] self the error object
\param id the unique error identifer choosen from MErrorType
\param text the error message
*/
#define mbus_error_set(self,id,text)		\
	self.type = id;									\
	if ( self.message ) g_free( self.message );		\
	if ( text ) self.message = g_strdup( text );	\
	else self.message = NULL;

/**
\brief reset the error identifier and the messsage
\param[out] self the error object
*/
#define mbus_error_reset( self )					\
	self.type = MERR_NOERR;							\
	if ( self.message ) g_free( self.message );		\
	self.message = NULL;

void mbus_error_printf( MError * self, MErrorType id,
		const gchar * format, ... );

void mbus_util_cut_whitespaces( GString * buf );
guchar * mbus_util_find_char( const guchar * start, guchar c );
gboolean mbus_util_str_to_uint64( const gchar * buf, guint64 * v );
void mbus_util_int64_to_str( GString * buf, guint64 number, gboolean sign );

gboolean mbus_argument_check( const gchar * pattern, const MObject * arguments,
			      GString * error );

#ifdef __cplusplus
}
#endif

#endif /* MBUS_UTIL_H */
