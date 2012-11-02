/* mheader.h
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

#ifndef MBUS_HEADER_H
#define MBUS_HEADER_H

#include <glib.h>

#include "gmbus/maddress.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
\addtogroup mheader MHeader
\{
*/

/**
\brief Defines all possible Mbus message types
*/
typedef enum { UNRELIABLE, RELIABLE } MMessageType;

/**
\brief Each Mbus message starts with a line, the message header, containing
    some protocol special information. The MHeader object holds these
    information like protocol version, timestamp, sequence number, source and
    destination address and an acknowledgement list for reliable messages.
*/
typedef struct {
	/** reliable/unreliable */
	MMessageType	type;
	/** protocol version */
	gchar *	pvers;
	/** sequence number */
	guint64	sequence_no;
	/** timestamp */
	guint64	timestamp;
	/** Mbus source address*/
	MAddress *	source;
	/** Mbus destination address */
	MAddress *	destination;
	/** list of acknowledgements */
	GArray *	acks;
	/** status information */
	gboolean	ok;
} MHeader;

MHeader * mbus_header_new( void );
MHeader * mbus_header_assign( MHeader * self, MHeader * rhs );
void mbus_header_free( MHeader * me );
gboolean mbus_header_read( MHeader * me, GString * buf );
gboolean mbus_header_as_string( MHeader * me, GString * buf );
#define mbus_header_ok(m) ( m->ok )

/**
\}
*/
#ifdef __cplusplus
}
#endif

#endif
