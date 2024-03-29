/* init.h
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

#ifndef MBUS_INIT_H
#define MBUS_INIT_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

void mbus_init( GMainLoop * loop );
void mbus_loop( void );
gboolean mbus_loop_step( gboolean may_block );
void mbus_quit( void );

#ifdef __cplusplus
}
#endif

#endif /* MBUS_INIT_H */

/* end of init.h */
