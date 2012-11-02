/* mmessage.c
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

#include "gmbus/mmessage.h"

#include <string.h>

static void
__mbus_message_init( MMessage * self )
{
  self->header = mbus_header_new();
  self->payload = mbus_payload_new();
  self->ok = TRUE;
  self->_index = 0;
}

/**
\brief creates a new MMessage object
\return newly created MMessage object
*/
MMessage *
mbus_message_new( void )
{
  MMessage * self = g_new( MMessage, 1 );

  g_assert( self );
  __mbus_message_init( self );

  return self;
}

/**
\brief copies the header and payload of \a rhs to \a self
*/
MMessage *
mbus_message_assign( MMessage * self, MMessage * rhs )
{
  mbus_header_assign( self->header, rhs->header );
  mbus_payload_assign( self->payload, rhs->payload );

  return self;
}


static void
__mbus_message_finalize( MMessage * self )
{
  mbus_header_free( self->header );
  mbus_payload_free( self->payload );
}

/**
\brief Frees an Mbus message object
\param self the MMessage object to destroy
*/
void
mbus_message_free( MMessage * self )
{
  __mbus_message_finalize( self );
  g_free( self );
}

/**
\brief Parses an Mbus message
\param self the MMessage object to store the parsed message
\param buf the string containing the text to parse
\return TRUE if the message parsing was successful, otherwise FALSE
*/
gboolean
mbus_message_read( MMessage * self, GString * buf )
{
  gchar * pos;

  self->ok = FALSE;

  mbus_header_read( self->header, buf );
  if ( !mbus_header_ok( self->header ) ) return FALSE;
  pos = strchr( buf->str, '\n' );
  if ( ! pos ) return FALSE;
  else pos++;

  g_string_erase( buf, 0, ( gsize ) ( pos - buf->str ) );

  if ( buf->len ) mbus_payload_read( self->payload, buf );

  self->ok = TRUE;

  return TRUE;
}

/**
\brief Converts an Mbus message object into a string representation usable for
    transport over Mbus
\param self The MMessage object to represent as a string
\param[out] buf the GString storing the string
\return TRUE if the conversion to string ws successful, otherwise FALSE
*/
gboolean
mbus_message_as_string( MMessage * self, GString * buf )
{
  self->ok = FALSE;

  mbus_header_as_string( self->header, buf );
  if ( !mbus_header_ok( self->header ) ) return FALSE;

  g_string_append_c( buf, '\n' );

  mbus_payload_as_string( self->payload, buf );
  if ( !mbus_payload_ok( self->payload ) ) return FALSE;

  self->ok = TRUE;

  return TRUE;
}

/**
\brief Checks if the given Mbus command is part of the Mbus message
\param self the MMessage object to search
\param command the Mbus command to find as a string
\return TRUE if the command was found in the message, otherwise FALSE
*/
gboolean
mbus_message_contains( const MMessage * self, const gchar * command )
{
  if ( !mbus_payload_size( self->payload ) ) return FALSE;

  return mbus_command_is_command( mbus_payload_index( self->payload, 0 ),
				  command );
}

/**
\brief Ads an Mbus command to the Mbus message object
\param self the MMessage object
\param c the MCommand object to add to the MMessage object
\return TRUE if the operation was successful, otherwise FALSE
\sa mbus_message_add_command
*/
gboolean
mbus_message_add( MMessage * self, MCommand * c )
{
  if ( mbus_command_ok( c ) ) {
    mbus_payload_add( self->payload, c );
    return TRUE;
  }

  return FALSE;
}

/**
\brief Adds an command to the Mbus message object. This function is only usful for
    commands that do not have any arguments.
\param self the MMessage object
\param cmd the Mbus command name as a string
\return TRUE if the operation was successful, otherwise FALSE
\sa mbus_message_add
*/
gboolean
mbus_message_add_command( MMessage * self, const gchar * cmd )
{
  return mbus_message_add( self, mbus_command_new( cmd ) );
}

/**
\brief When a MMessage object is passed to an callback function because an
    application registered for an command in the message the application may
    want to have access to the corresponding MCommand object. As an MMessage
    object may contain more than one MCommand objects this function provides an
    easy way to access the relevant MCommand object.
\param self the MMessage object
\return The relevant MCommand object if found, otherwise NULL
*/
MCommand *
mbus_message_get_current_command( MMessage * self )
{
  if ( self->_index >= mbus_payload_size( self->payload ) )
    return NULL;

  return mbus_payload_index( self->payload, self->_index );
}
