/* mcoder.h
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

#ifndef MBUS_CODER_H
#define MBUS_CODER_H

#include "gmbus/mconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
\addtogroup mcoder MCoder
\{
*/

#define DIGEST_LEN_64 16		/* FIXME */
#define DIGEST_LEN_BIN 12		/* FIXME */

/**
\brief API for digest calculators.
*/
typedef struct {
	gint ( *calculate )( const GByteArray * key, const GString * buf,
			GByteArray * digest );
} MDigestor;


/**
\brief API for encryptors/decryptors
*/
typedef struct {
	gint ( *encrypt )( const GByteArray * k, const GString * buf,
			GByteArray * result );
	gint ( *decrypt )( const GByteArray * k, const GByteArray * buf,
			GString * result );
} MEncryptor ;

extern MDigestor *	digestors[];
extern MEncryptor *	encryptors[];

/**
\brief A Coder can generate and verify hash keys and de- and encrypt Mbus
    messages
*/
typedef struct {
	/** algorithm to create a digest */
	MDigestor		digest;
	/** encrypt and decrypt functions */
	MEncryptor		crypto;
	/** hash key */
	GByteArray *	hash_key;
	/** encryption key */
	GByteArray *	encr_key;
	/** status information */
	gboolean		ok;
} MCoder ;

MCoder * mbus_coder_new( const MConfig * config );
void mbus_coder_free( MCoder * me );
gboolean mbus_coder_encode( MCoder * me, GString * buf, GByteArray * result );
gboolean mbus_coder_decode( MCoder * me, const GByteArray * buf,
		GString * result );

/**
\brief Retrieves status information
*/
#define mbus_coder_ok(me) ( me->ok )

/**
\}
*/

#ifdef __cplusplus
}
#endif

#endif
