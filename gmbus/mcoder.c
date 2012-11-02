/* mcoder.c
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

#include "gmbus/mcoder.h"
#include "gmbus/hmac.h"
#include "gmbus/base64.h"

#include <string.h>

static gint
__ident_encode( const GByteArray * key, const GString * buf,
		GByteArray * result )
{
	/* padding bytes should be added in here if required */
	g_byte_array_append( result, ( const guchar * ) buf->str, buf->len );

	return 0;
}

static gint
__ident_decode( const GByteArray * key, const GByteArray * buf,
		GString * result )
{
	/* trailing padding bytes should ber removed in here if necessary */
	g_string_append_len( result, ( const gchar * ) buf->data,
			( gint ) buf->len );

	return 0;
}

static gint
__hmac_md5( const GByteArray * key, const GString * buf, GByteArray * digest )
{
	hmac_md5( ( const guchar * ) buf->str, buf->len, key->data, key->len,
			digest->data );

	return 0;
}

static MEncryptor	noencr	= { __ident_encode, __ident_decode };
static MDigestor	hmacMD5	= { __hmac_md5 };

/* provide the default algorithms: */
MDigestor *		digestors[]		= { &hmacMD5, 0, 0, 0 };
MEncryptor *	encryptors[]	= { &noencr, 0, 0, 0 };

static gint
__mbus_coder_init( MCoder * me, const MConfig * config )
{
	me->ok = TRUE;

	if ( digestors[ config->hash_algo ] ) {	/* is implemented */
		me->digest = *digestors[ config->hash_algo ];
	} else {
		g_warning( "unknown hash algorithmn" );
		me->ok = FALSE;
	}

	if ( me->ok && encryptors[ config->encr_algo ] ) { /* is implemented */
		me->crypto = *encryptors[ config->encr_algo ];
	} else {
		me->ok = FALSE;
		g_warning( "unknown encryption algorithmn" );
	}

	if ( me->ok ) {
		me->hash_key = config->hash_key;
		me->encr_key = config->encr_key;
	}

	return 0;
}

MCoder *
mbus_coder_new( const MConfig * config )
{
	MCoder * self = g_new( MCoder, 1 );

	g_assert( self );
	__mbus_coder_init( self, config );

	return self;
}

static void
__mbus_coder_finalize( MCoder * self )
{
}

void
mbus_coder_free( MCoder * self )
{
	__mbus_coder_finalize( self );
	g_free( self );
}

gboolean
mbus_coder_encode( MCoder * self, GString * buf, GByteArray * result )
{
	GByteArray *	digest_bin = g_byte_array_sized_new( DIGEST_LEN_BIN );

	memset( digest_bin->data, 0, DIGEST_LEN_BIN );
	/* the next line MUST be there */
	g_byte_array_set_size( digest_bin, DIGEST_LEN_BIN );
	self->digest.calculate( self->hash_key, buf, digest_bin );
	/* base64 encode of digest */
	base64encode( digest_bin, result );
	g_byte_array_append( result, ( const guchar * ) "\n", 1 );
	g_string_prepend_len( buf, ( const gchar * ) result->data,
			DIGEST_LEN_64 + 1 );

	g_byte_array_set_size( result, 0 );
	/* encrypt message buffer including the digest */
	self->crypto.encrypt( self->encr_key, buf, result );

	g_byte_array_free( digest_bin, TRUE );

	return TRUE;
}

gboolean
mbus_coder_decode( MCoder * self, const GByteArray * buf, GString * result )
{
	GByteArray *	digest64 = g_byte_array_sized_new( DIGEST_LEN_64 );
	GByteArray *	my_digest = g_byte_array_sized_new( DIGEST_LEN_BIN );
	GByteArray *	my_digest64 = g_byte_array_sized_new( DIGEST_LEN_64 );
	gchar *	pos;
	gboolean	ret = TRUE;

	/* decrypt data */
	self->crypto.decrypt( self->encr_key, buf, result );

	/* find digest and extract it */
	pos = result->str;
	while ( *pos != '\0' && *pos != '\n' ) pos++;
	if ( *pos == '\0' ) {
		g_warning( "could not find digest!" );
		ret = FALSE;
		goto done;
	}

	g_byte_array_append( digest64, ( const guchar * ) result->str,
			( gsize ) ( pos - result->str ) );
	g_string_erase( result, 0, ( gsize ) ( pos - result->str + 1 ) );

	/* calculate digest and check it */
	g_byte_array_set_size( my_digest, DIGEST_LEN_BIN );
	g_byte_array_set_size( my_digest64, DIGEST_LEN_64 );
	self->digest.calculate( self->hash_key, result, my_digest );
	base64encode( my_digest, my_digest64 );

	if ( memcmp( my_digest64->data, digest64->data, DIGEST_LEN_64 ) ) {
		g_warning( "digest mis-match: %.16s vs. %.16s",
				my_digest64->data, digest64->data );
		ret = FALSE;
	}

  done:
	g_byte_array_free( digest64, TRUE );
	g_byte_array_free( my_digest64, TRUE );
	g_byte_array_free( my_digest, TRUE );

	return ret;
}
