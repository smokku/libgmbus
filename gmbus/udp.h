/* udp.h
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

#ifndef MBUS_UDP_H
#define MBUS_UDP_H

#include <glib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

  /**
     \addtogroup mendpoint MEndpoint
     \{
     \brief describes the transport address of an MClient. Currently
     only IPv4 is support (IPv6 support is partly implemented, but
     multicast is still missing).
     \sa MClient
  */

  /**
     \brief Alias for the sockaddr structure that is used as a generic
     container for endpoint addresses (IPv4 and IPv6)
  */
  typedef struct {
    union {
      struct sockaddr		inc;
      struct sockaddr_in	in;
      struct sockaddr_in6	in6;
    } addr;
#define af_family addr.inc.sa_family
  } MEndpoint ;

  MEndpoint * mbus_endpoint_new( sa_family_t family );
  MEndpoint * mbus_endpoint_new_from_string( const gchar * address );
  MEndpoint * mbus_endpoint_new4( struct in_addr addr, in_port_t port );
  MEndpoint * mbus_endpoint_new6( struct in6_addr addr, in_port_t port );
  sa_family_t mbus_endpoint_get_family( MEndpoint * self );
  socklen_t mbus_endpoint_get_size( MEndpoint * self );
  in_port_t mbus_endpoint_get_port( MEndpoint * self );
  void mbus_endpoint_set_port( MEndpoint * self, in_port_t );
  void mbus_endpoint_set_address4( MEndpoint * self, in_addr_t addr );
  void mbus_endpoint_set_address6( MEndpoint * self, struct in6_addr addr );
  gboolean mbus_endpoint_set_address_from_string( MEndpoint * self,
      const gchar * address );
  gboolean mbus_endpoint_is_equal( const MEndpoint * e1,
				   const MEndpoint * e2 );
  void mbus_endpoint_print( MEndpoint * self );

  GIOChannel * mbus_socket_new( MEndpoint * ep, gboolean multicast );
  void mbus_socket_get_name( GIOChannel * channel, MEndpoint * ep );

  /**
     \}
  */
#ifdef __cplusplus
}
#endif

#endif
