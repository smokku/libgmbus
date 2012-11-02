/* printll.c
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

#include "gmbus/mutil.h"

#include <stdio.h>

int
main( int argc, char * argv [] )
{
  guint64 	positive =  (guint64 ) ~1;
  gint64 	negative = (gint64) ~1;
  GString *	tmp = g_string_new( NULL );

  mbus_util_int64_to_str( tmp, positive, FALSE );
  g_string_append_c( tmp, ' ' );
  mbus_util_int64_to_str( tmp, negative, TRUE );

  g_message( "printf                   : %llu %lld", positive, negative );
  g_message( "mbus_util_print_long_long: %s ", tmp->str );
  g_string_free(tmp, TRUE );

  return 0;
}




/* end of printll.c */
