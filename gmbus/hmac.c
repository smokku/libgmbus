/*
 * FILE:    hmac.c
 * AUTHORS: Colin Perkins
 *
 * HMAC message authentication (RFC2104)
 *
 * Copyright (c) 1998-2000 University College London
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the Computer Science
 *      Department at University College London
 * 4. Neither the name of the University nor of the Department may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "gmbus/hmac.h"
#include "gmbus/md5.h"

#include <string.h>

/**
\brief Calulates a HMAC/MD5 digest for a given byte array using a given \a key.
\param data pointer to the data array for which a digest should be calculated
\param data_len length of the data in bytes
\param key pointer to the key
\param key_len length of the key in bytes
\param[out] digest a byte array that will contain the digest
*/
void hmac_md5( const guchar * data,	/* pointer to data stream        */
	       gint data_len,		/* length of data stream         */
	       const guchar * key,	/* pointer to authentication key */
	       gint key_len,		/* length of authentication key  */
	       guchar digest[ 16 ] ) /* caller digest to be filled in */
{
  MD5_CTX	context;
  guchar	k_ipad[ 65 ];    /* inner padding - key XORd with ipad */
  guchar	k_opad[ 65 ];    /* outer padding - key XORd with opad */
  guchar	tk[ 16 ];
  gint	i;

  /* If key is longer than 64 bytes reset it to key=MD5(key) */
  if ( key_len > 64 ) {
    MD5_CTX tctx;

    MD5Init( &tctx );
    MD5Update( &tctx, key, key_len );
    MD5Final( tk, &tctx );

    key = tk;
    key_len = 16;
  }

  /*
   * The HMAC_MD5 transform looks like:
   *
   * MD5(K XOR opad, MD5(K XOR ipad, data))
   *
   * where K is an n byte key
   * ipad is the byte 0x36 repeated 64 times
   * opad is the byte 0x5c repeated 64 times
   * and text is the data being protected
   */

  /* Start out by storing key in pads */
  memset( k_ipad, 0, sizeof( k_ipad ) );
  memset( k_opad, 0, sizeof( k_opad ) );
  memcpy( k_ipad, key, key_len );
  memcpy( k_opad, key, key_len );

  /* XOR key with ipad and opad values */
  for ( i = 0; i < 64; i++ ) {
    k_ipad[ i ] ^= 0x36;
    k_opad[ i ] ^= 0x5c;
  }

  /*
   * perform inner MD5
   */
  MD5Init( &context );                   /* init context for 1st pass */
  MD5Update( &context, k_ipad, 64 );     /* start with inner pad      */
  MD5Update( &context, data, data_len ); /* then text of datagram     */
  MD5Final( digest, &context );          /* finish up 1st pass        */
  /*
   * perform outer MD5
   */
  MD5Init( &context );                   /* init context for 2nd pass */
  MD5Update( &context, k_opad, 64 );     /* start with outer pad      */
  MD5Update( &context, digest, 16 );     /* then results of 1st hash  */
  MD5Final( digest, &context );          /* finish up 2nd pass        */
}

/*
 * Test Vectors (Trailing '\0' of a character string not included in test):
 *
 * key =         0x0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b
 * key_len =     16 bytes
 * data =        "Hi There"
 * data_len =    8  bytes
 * digest =      0x9294727a3638bb1c13f48ef8158bfc9d
 *
 * key =         "Jefe"
 * data =        "what do ya want for nothing?"
 * data_len =    28 bytes
 * digest =      0x750c783e6ab0b503eaa86e310a5db738
 *
 * key =         0xAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 * key_len       16 bytes
 * data =        0xDDDDDDDDDDDDDDDDDDDD...
 *               ..DDDDDDDDDDDDDDDDDDDD...
 *               ..DDDDDDDDDDDDDDDDDDDD...
 *               ..DDDDDDDDDDDDDDDDDDDD...
 *               ..DDDDDDDDDDDDDDDDDDDD
 * data_len =    50 bytes
 * digest =      0x56be34521d144c88dbb8c733f0e8b3f6
 */

