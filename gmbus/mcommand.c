/* mcommand.c
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

#include "gmbus/mcommand.h"

#include <stdio.h>
#include <string.h>

static void
__mbus_command_init( MCommand * self, const gchar * cmd )
{
	GString * tmp = g_string_new( cmd );

	self->cmd = g_string_new( NULL );
	self->arguments = mbus_list_new();
	if ( cmd ) mbus_command_read( self, tmp );

	g_string_free( tmp, TRUE );
	self->ok = TRUE;
}

/**
\brief Creates a new Mbus command object
\param cmd A string containing a Mbus command that will be parsed
\return A newly created Mbus command object
*/
MCommand *
mbus_command_new( const gchar * cmd )
{
	MCommand * self = g_new( MCommand, 1 );

	g_assert( self );
	__mbus_command_init( self, cmd );

	return self;
}

/**
\brief Creates a new Mbus command object
\param cmd The command name
\return A newly created Mbus command object
*/
MCommand *
mbus_command_new_with_name( const gchar * cmd )
{
	MCommand * self = g_new( MCommand, 1 );

	g_assert( self );

	self->cmd = g_string_new( cmd );
	self->arguments = mbus_list_new();

	self->ok = TRUE;

	return self;
}

static void
__mbus_command_finalize( MCommand * self )
{
	g_string_free( self->cmd, TRUE );
	mbus_list_free( self->arguments );
}

/**
\brief Frees an Mbus command object
\param self the Mbus command object to free
*/
void
mbus_command_free( MCommand * self )
{
	__mbus_command_finalize( self );
	g_free( self );
}

/**
\brief Parses a string representation of an Mbus command
\param[out] self Mbus command object that will store the parsed information
\param buf The string buffer containing the command
*/
gboolean
mbus_command_read( MCommand * self, GString * buf )
{
	/* FIXME: we cannot handle escaped newlines in strings! */
	gchar *	cmd = buf->str;
	gchar         tmp;
	gint          c;

	/* each command character MUST be a valid MSymbol character */
	while ( mbus_symbol_is_valid_char( *cmd ) ) cmd++;

	if ( cmd ) {
		tmp = *cmd;
		*cmd = '\0';
	}
	g_string_erase( self->cmd, 0, -1 );
	g_string_append( self->cmd, buf->str );
	*cmd = tmp;
	g_string_erase( buf, 0, ( gsize ) ( cmd - buf->str ) );

	c = mbus_list_read( self->arguments, ( const guchar * ) buf->str );
	if ( c > 0 ) g_string_erase( buf, 0, c );
	self->ok = TRUE;

	return TRUE;
}

/**
\brief Creates a string representation of an Mbus command
\param self The Mbus command object to create a string of
\param buf the string buffer that will store the string
\return TRUE if the string representation could be created successfully,
    otherwise FALSE
*/
gboolean
mbus_command_as_string( MCommand * self, GString * buf )
{
	g_string_append( buf, self->cmd->str );
	mbus_list_as_string( self->arguments, buf );
	self->ok = TRUE;

	return TRUE;
}

/**
\brief Copies the information of one Mbus command to another
\param self the Mbus command to copy to
\param other the Mbus command to copy from
\return A pointer to the modified Mbus command object
*/
MCommand *
mbus_command_assign( MCommand * self, const MCommand * other )
{
	if ( self != other ) {
		g_string_assign( self->cmd, other->cmd->str );

		mbus_list_free( self->arguments );
		self->arguments = mbus_list_copy( other->arguments );
		self->ok = other->ok;
	}

	return self;
}

/**
\brief Copies one Mbus Command object
\param self The Mbus command to copy
\return A newly created Mbus command object being a copy of \a self
*/
MCommand *
mbus_command_copy( MCommand * self )
{
	MCommand * copy = mbus_command_new( NULL );

	mbus_command_assign( copy, self );

	return copy;
}

/**
\brief Checks if the command name is equal to the given one
\param self The Mbus Command to check for
\param s The command name to compare with
\return TRUE if the Mbus command object has the requested command name,
    otherwise FALSE
*/
gboolean
mbus_command_is_command( const MCommand * self, const gchar * s )
{
	return ( strcmp( s, self->cmd->str ) == 0 );
}
