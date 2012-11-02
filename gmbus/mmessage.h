/* mmessage.h
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

#ifndef MBUS_MESSAGE_H
#define MBUS_MESSAGE_H

#include "gmbus/mheader.h"
#include "gmbus/mpayload.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
\addtogroup mmessage MMessage
\{
*/

/**
\brief describes an Mbus message with header and payload
\sa MHeader, MPayload
*/
typedef struct {
  MHeader *	header;
  MPayload *	payload;
  gboolean	ok;

  guint         _index;
} MMessage;

MMessage * mbus_message_new( void );
MMessage * mbus_message_assign( MMessage * self, MMessage * rhs );
void mbus_message_free( MMessage * me );
gboolean mbus_message_read( MMessage * me, GString * buf );
gboolean mbus_message_as_string( MMessage * me, GString * buf );

gboolean mbus_message_contains( const MMessage * msg, const gchar * command );
gboolean mbus_message_add( MMessage * me, MCommand * c );
gboolean mbus_message_add_command( MMessage * me, const gchar * cmd );

MCommand * mbus_message_get_current_command( MMessage * self );

/**
\def mbus_message_ok
\brief checks the status of the given MMessage object
\return TRUE if no error has occured since last call, otherwise FALSE
*/
#define mbus_message_ok(m) ( m->ok )

/**
\}
*/

#ifdef __cplusplus
}
#endif

#endif
