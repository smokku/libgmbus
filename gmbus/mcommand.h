/* mcommand.h
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

#ifndef MBUS_COMMAND_H
#define MBUS_COMMAND_H

#include "gmbus/mtypes.h"

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
\addtogroup mcommand MCommand
\{
*/

/**
\brief Describes an Mbus command.
*/
typedef struct {
	/** The command name */
	GString *	cmd;
	/** The arguments */
	MObject *	arguments;
	/** status information */
	gboolean	ok;
} MCommand;

MCommand * mbus_command_new( const gchar * cmd );
MCommand * mbus_command_new_with_name( const gchar * cmd );
MCommand * mbus_command_assign( MCommand * self, const MCommand * other );
MCommand * mbus_command_copy( MCommand * self );
void mbus_command_free( MCommand * self );
gboolean mbus_command_read( MCommand * self, GString * buf );
gboolean mbus_command_as_string( MCommand * self, GString * buf );
gboolean mbus_command_is_command( const MCommand * self, const gchar * s );

/**
\brief Returns the status of the Mbus command object
*/
#define mbus_command_ok(self) ( self->ok )

/**
\}
*/

#ifdef __cplusplus
}
#endif

#endif
