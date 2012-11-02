/* maddress.h
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

#ifndef MBUS_ADDRESS_H
#define MBUS_ADDRESS_H

#define M_ADDRESS_ELEMENT_SIZE 256

#include <glib.h>

typedef struct {
	gchar *		key;
	gchar *		value;
	gboolean	ok;
} MAddressElement;

typedef struct {
	GPtrArray *	elements;
	gboolean	ok;
} MAddress;

gboolean mbus_address_element_read( MAddressElement * self, GString * buf );
gboolean mbus_address_element_as_string( MAddressElement * self,
		GString * buf );
gboolean mbus_address_element_is_equal( const MAddressElement * self,
		const MAddressElement * other );

MAddress * mbus_address_new( void );
void mbus_address_free( MAddress * self );
MAddress * mbus_address_copy( MAddress * self );
MAddressElement * mbus_address_find( MAddress * self, const gchar * key );
gboolean mbus_address_read( MAddress * self, GString * buf );
gboolean mbus_address_as_string( MAddress * self, GString * buf );
guint mbus_address_hash( const MAddress * self );
MAddress * mbus_address_assign( MAddress * self, const MAddress * other );
gboolean mbus_address_is_equal( const MAddress * self, const MAddress * other );
gboolean mbus_address_is_unique( const MAddress * self );
gboolean mbus_address_is_subset_of( const MAddress * self,
    const MAddress * other );
void mbus_address_disambiguate( MAddress * self );

#define mbus_address_ok(self) ( self->ok )

#endif
