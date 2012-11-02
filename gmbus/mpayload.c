/* mpayload.c
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

#include "gmbus/mpayload.h"

#include <string.h>
#include <stdio.h>

static void
__mbus_payload_init( MPayload * self )
{
  self->cmds = g_ptr_array_sized_new( 1 );
  self->ok = TRUE;
}

/**
\brief creates a new MPayload object
\return a pointer to the new MPayload object or NULL on failure
*/
MPayload *
mbus_payload_new( void )
{
  MPayload * self = g_new( MPayload, 1 );

  g_assert( self );
  __mbus_payload_init( self );

  return self;
}

static void
__mbus_payload_finalize( MPayload * self )
{
  gint i;

  for ( i = 0; i < self->cmds->len; i++ )
    mbus_command_free( self->cmds->pdata[ i ] );

  g_ptr_array_free( self->cmds, TRUE );
}

/**
\brief frees a MPayload object and all MCommand objects in it.
\param self the MPayload object to free
*/
void
mbus_payload_free( MPayload * self )
{
  __mbus_payload_finalize( self );

  g_free( self );
}

/**
\brief parses the body of an Mbus message by using the MCommand parse for
    each line in the body.
\param self the payload object used to store the parsed data
\param buf the buffer to parse
\return TRUE if the message body could be parsed successfully, otherwise FALSE
\sa mbus_command_read
*/
gboolean
mbus_payload_read( MPayload * self, GString * buf )
{
  MCommand *	cmd;
  gboolean	finished = FALSE;

  self->ok = TRUE;			/* payload may be empty */

  while ( !finished && ( buf->str[ 0 ] != '\n' ) && ( buf->len > 1 ) ) {
    cmd = mbus_command_new( NULL );
    mbus_command_read( cmd, buf );
    if ( mbus_command_ok( cmd ) ) {
      g_ptr_array_add( self->cmds, cmd );
      /* look for end of line */
      while ( buf->str[ 0 ] != '\n' ) {
	if ( buf->str[ 0 ] == '\0' ) return FALSE;
	g_string_erase( buf, 0, 1 );
      }
      g_string_erase( buf, 0, 1 );
      finished = FALSE;
    } else {
      mbus_command_free( cmd );
      finished = TRUE;
    }
  } /* while(!finished) */

  return TRUE;
}

/**
\brief creates a text representation of the payloiad object used for Mbus
    transport
\param self the payload object to transform
\param buf the buffer used to store the text representation
\return TRUE of the creation of the text represention was successful,
    otherwise FALSE
\sa mbus_command_as_string
*/
gboolean
mbus_payload_as_string( MPayload * self, GString * buf )
{
  gint i;

  for ( i = 0; i < self->cmds->len; i++ ) {
    MCommand * cmd  = g_ptr_array_index( self->cmds, i );

    mbus_command_as_string( cmd, buf );
    g_string_append_c( buf, '\n' );
  }

  self->ok = TRUE;

  return TRUE;
}

/**
\brief append a new MCommand object to the payload
\param self the payload object that the MCommand object will be append to
\param cmd the MCommand to append
\return TRUE if the MCommand object could be appended, otherwise FALSE
*/
gboolean
mbus_payload_add( MPayload * self, MCommand * cmd )
{
  g_ptr_array_add( self->cmds, cmd );

  return TRUE;
}

/**
\brief assigns the MPayload object self a new list of MCommand objects copied
    from the MPayload object other
\param self the payload object to change
\param other the payload object to copy from
\return the modified payload object
*/
MPayload *
mbus_payload_assign( MPayload * self, const MPayload * other )
{
  if( self != other ) {
    gint i;

    g_ptr_array_set_size( self->cmds, self->cmds->len );
    for ( i = 0; i < other->cmds->len; i++ )
      g_ptr_array_add( self->cmds,
		       mbus_command_copy( other->cmds->pdata[ i ] ) );
    self->ok = other->ok;
  }

  return self;
}
