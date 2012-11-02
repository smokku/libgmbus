/* mpayload.h
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

#ifndef MBUS_PAYLOAD_H
#define MBUS_PAYLOAD_H

#include <glib.h>

#include "gmbus/mcommand.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
\addtogroup mpayload MPayload
\{
*/

/**
\brief describes the payload of an Mbus message and contains a list of
    MCommand objects
*/
typedef struct {
  GPtrArray *	cmds;
  gboolean	ok;
} MPayload;

MPayload * mbus_payload_new( void );
void mbus_payload_free( MPayload * self );
MPayload * mbus_payload_assign( MPayload * self, const MPayload * other );
gboolean mbus_payload_read( MPayload * self, GString * buf );
gboolean mbus_payload_as_string( MPayload * self, GString * buf );
gboolean mbus_payload_add( MPayload * self, MCommand * cmd );

/**
\def mbus_payload_size
\brief returns the number of MCommand objects in this payload
\param self the pyload object
*/
#define mbus_payload_size( self ) ( self->cmds->len )

/**
\def mbus_payload_index
\brief retrieves an MCommand object at index \a i
\param self the payload object
\param i the index of the MCommand object to retrieve
\return a pointer to the MCommand object if found, otherwise NULL
*/
#define mbus_payload_index( self, i ) ( g_ptr_array_index( self->cmds, i ) )

/**
\def mbus_payload_ok
\brief returns the status of the payload object
\param self the payload object to check
\return TRUE if there has been no error occured since last call, otherwise FALSE
*/
#define mbus_payload_ok( self ) ( self->ok )

/**
\def mbus_payload_clear
\brief removes all MCommand objects of the specified payload object
\param self the payload object to clean
*/
#define mbus_payload_clear( self ) \
  g_ptr_array_remove_range( self->cmds, 0, self->cmds->len )

/**
\}
*/

#ifdef __cplusplus
}
#endif

#endif
