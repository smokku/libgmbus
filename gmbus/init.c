/* init.c
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

#include "gmbus/init.h"

static GMainLoop * __loop;

/**
\brief Initialize the main loop
\param loop If loop is not NULL the given main loop is used, otherwise this
    function will create a new one.
*/
void
mbus_init( GMainLoop * loop )
{
	if ( loop == NULL ) {
		__loop = g_main_loop_new( NULL, FALSE );
	} else {
		g_main_loop_ref( loop );
		__loop = loop;
	}
}

/**
\brief Runs the main loop
*/
void
mbus_loop( void )
{
	g_main_loop_run( __loop );
}

/**
\brief Runs one step in the main loop
*/
gboolean
mbus_loop_step( gboolean may_block )
{
	return g_main_context_iteration( NULL, may_block );
}

/**
\brief Quits and frees the main loop
*/
void
mbus_quit( void )
{
	g_main_loop_quit( __loop );
	g_main_loop_unref( __loop );
}

/* end of init.c */
