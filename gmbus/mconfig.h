/* mconfig.h
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

#ifndef MBUS_CONFIG_H
#define MBUS_CONFIG_H

#include "gmbus/udp.h"

#define MAX_KEY_LEN 64		/* FIXME */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { M_NOENCR, M_DES, M_IDEA, M_ENCR_MAX } MbusEncrAlgo;

typedef enum { M_HMAC_MD5_96, M_HMAC_SHA1_96, M_DIGEST_MAX } MbusDigestAlgo;

typedef enum { HOSTLOCAL, LINKLOCAL } MbusScope;

typedef struct {
  MbusDigestAlgo	hash_algo;
  GByteArray *		hash_key;

  MbusEncrAlgo		encr_algo;
  GByteArray *		encr_key;

  MEndpoint *           group_addr;
  MEndpoint *    	unicast_addr;
  gboolean		send_unicast;

  MbusScope		scope;
} MConfig;

MConfig * mbus_config_new( gboolean read );
void mbus_config_free( MConfig * config );
gboolean mbus_config_read( MConfig * c );
gboolean mbus_config_is_equal( const MConfig * lhs, const MConfig * rhs );

#ifdef __cplusplus
}
#endif

#endif
