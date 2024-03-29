/*
 * FILE:   base64.c
 * AUTHOR: Colin Perkins
 *
 * MIME base64 encoder/decoder described in rfc1521. This code is derived
 * from version 2.7 of the Bellcore metamail package.
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
 *
 * Copyright (c) 1991 Bell Communications Research, Inc. (Bellcore)
 *
 * Permission to use, copy, modify, and distribute this material
 * for any purpose and without fee is hereby granted, provided
 * that the above copyright notice and this permission notice
 * appear in all copies, and that the name of Bellcore not be
 * used in advertising or publicity pertaining to this
 * material without the specific, prior written permission
 * of an authorized representative of Bellcore.  BELLCORE
 * MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
 * OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
 * WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
 *
 */

#include "gmbus/base64.h"

static guchar b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static gsize
__alloc_array_encode( const GByteArray * input )
{
  return ( ( input->len / 4 ) + 1 ) * 4;
}

static gsize
__alloc_array_decode( const GByteArray * input )
{
  gint		len = input->len;
  guint8 *	pos = &( input->data[ len - 1 ] );

  while ( *pos == '=' ) --pos, --len;

  return ( len * 3 / 4 );
}

/**
\brief Encodes a number of bytes using Base64
\param input The byte array containing the data to encode
\param[out] output A byte array that will contain the Base64 encoded data
\return the number of bytes of the \a output array
*/
gint
base64encode( const GByteArray * input, GByteArray * output )
{
  gint	i = 0, j = 0;
  gint	pad;

  g_byte_array_set_size( output, __alloc_array_encode( input ) );
  while ( i < input->len ) {
    pad = 3 - ( input->len - i );
    if ( pad == 2 ) {
      output->data[ j ] = b64[ input->data[ i ] >> 2 ];
      output->data[ j + 1 ] = b64[ ( input->data[ i ] & 0x03 ) << 4 ];
      output->data[ j + 2 ] = '=';
      output->data[ j + 3 ] = '=';
    } else if ( pad == 1 ) {
      output->data[ j ] = b64[ input->data[ i ] >> 2 ];
      output->data[ j + 1 ] = b64[ ( ( input->data[ i ] & 0x03 ) << 4 ) |
				   ( ( input->data[ i + 1 ] & 0xf0 ) >> 4 ) ];
      output->data[ j + 2 ] = b64[ ( input->data[ i + 1 ] & 0x0f ) << 2 ];
      output->data[ j + 3 ] = '=';
    } else {
      output->data[ j ] = b64[ input->data[ i ] >> 2 ];
      output->data[ j + 1 ] = b64[ ( ( input->data[ i ] & 0x03 ) << 4 ) |
				   ( ( input->data[ i + 1 ] & 0xf0 ) >> 4 ) ];
      output->data[ j + 2 ] = b64[ ( ( input->data[ i + 1 ] & 0x0f ) << 2 ) |
				   ( ( input->data[ i + 2 ] & 0xc0 ) >> 6 ) ];
      output->data[ j + 3 ] = b64[ input->data[ i + 2 ] & 0x3f ];
    }
    i += 3;
    j += 4;
  }

  return j;
}

/* This assumes that an unsigned char is exactly 8 bits. Not portable code! :-) */
static guchar index_64[ 128 ] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff,   62, 0xff, 0xff, 0xff,   63,
    52,   53,   54,   55,   56,   57,   58,   59,
    60,   61, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff,    0,    1,    2,    3,    4,    5,    6,
     7,    8,    9,   10,   11,   12,   13,   14,
    15,   16,   17,   18,   19,   20,   21,   22,
    23,   24,   25, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff,   26,   27,   28,   29,   30,   31,   32,
    33,   34,   35,   36,   37,   38,   39,   40,
    41,   42,   43,   44,   45,   46,   47,   48,
    49,   50,   51, 0xff, 0xff, 0xff, 0xff, 0xff
};

#define char64(c) ( ( c > 127 ) ? 0xff : index_64[ ( c ) ] )

/**
\brief Decodes a number of Base64 encoded bytes
\param input The Base64 encoded byte array that is to be decoded
\param[out] output the byte array that will contain the decoded bytes
\return the number of bytes written to the \a output array
*/
gint
base64decode( const GByteArray * input, GByteArray * output )
{
  gint		i = 0, j = 0, pad;
  guchar	c[ 4 ];

  g_byte_array_set_size( output, __alloc_array_decode( input ) );
  while ( ( i + 3 ) < input->len ) {
    pad = 0;
    c[ 0 ] = char64( input->data[ i  ] ); pad += ( c[ 0 ] == 0xff );
    c[ 1 ] = char64( input->data[ i + 1 ] ); pad += ( c[ 1 ] == 0xff );
    c[ 2 ] = char64( input->data[ i + 2 ] ); pad += ( c[ 2 ] == 0xff );
    c[ 3 ] = char64( input->data[ i + 3 ] ); pad += ( c[ 3 ] == 0xff );
    if ( pad == 2 ) {
      output->data[ j++ ] = ( c[ 0 ] << 2 ) | ( ( c[ 1 ] & 0x30 ) >> 4 );
      output->data[ j ] = ( c[ 1 ] & 0x0f ) << 4;
    } else if ( pad == 1 ) {
      output->data[ j++ ] = ( c[ 0 ] << 2 ) | ( ( c[ 1 ] & 0x30 ) >> 4 );
      output->data[ j++ ] = ( ( c[ 1 ] & 0x0f ) << 4 ) |
	( ( c[ 2 ] & 0x3c ) >> 2 );
      output->data[ j ] = ( c[ 2 ] & 0x03 ) << 6;
    } else {
      output->data[ j++ ] = ( c[ 0 ] << 2 ) | ( ( c[ 1 ] & 0x30 ) >> 4 );
      output->data[ j++ ] = ( ( c[ 1 ] & 0x0f ) << 4 ) |
	( ( c[ 2 ] & 0x3c ) >> 2 );
      output->data[ j++ ] = ( ( c[ 2 ] & 0x03 ) << 6 ) | ( c[ 3 ] & 0x3f );
    }
    i += 4;
  }

  return j;
}
