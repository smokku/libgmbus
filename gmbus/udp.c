/* udp.c
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

#include "gmbus/udp.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

static void __mbus_socket_multicast_ipv4( gint fd, MEndpoint * ep );
static void __mbus_socket_multicast_ipv6( gint fd, MEndpoint * ep );

#ifdef __APPLE__
#define IPV6_ADD_MEMBERSHIP IP_ADD_MEMBERSHIP
#endif

/**
\addtogroup mendpoint MEndpoint
\{
*/

/**
   \brief Creates a UDP socket and binds to the given transport address. If
   the argument \a multicast is set to TRUE the multicast group is joined.
   \param ep the remote trnasport address
   \param multicast if \a addr is a multicast address and this parameter is TRUE
   the multicast group is joined.
   \return A pointer to a GIOChannel object or NULL on failure.
*/
GIOChannel *
mbus_socket_new( MEndpoint * ep, gboolean multicast )
{
  gint			fd;
  gint			yes = 1;

  fd = socket( mbus_endpoint_get_family( ep ), SOCK_DGRAM, 0 );
  if ( fd == -1 ) perror( "socket" );
  if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
	  ( gchar * ) &yes, sizeof yes ) )
    perror( "setsockopt( SO_REUSEADDR )" );

  if ( bind( fd, ( struct sockaddr * ) ep,
	  mbus_endpoint_get_size( ep ) ) == -1 ) {
    mbus_endpoint_print( ep );
    perror( "bind" );
  }
  if ( multicast ) {
    if ( mbus_endpoint_get_family( ep ) == AF_INET )
      __mbus_socket_multicast_ipv4( fd, ep );
    else
      __mbus_socket_multicast_ipv6( fd, ep );
  }

  return g_io_channel_unix_new( fd );
}

/**
   \brief Retrieves the local IP address of the local interface used for
   this socket.

   \param channel the channel to retrieve the local transport address from.
   \param[out] ep contains the local transport address used by this socket.
*/
void
mbus_socket_get_name( GIOChannel * channel, MEndpoint * ep )
{
  gint			fd = g_io_channel_unix_get_fd( channel );
  socklen_t		len = sizeof( MEndpoint );

  getsockname( fd, ( struct sockaddr * ) ep, &len );
}

static void
__mbus_socket_multicast_ipv4( gint fd, MEndpoint * ep )
{
  gint			yes = 1;

#ifdef HAVE_IP_MREQN
  struct ip_mreqn	imr;
#else
  struct ip_mreq	imr;
#endif
  if ( setsockopt( fd, IPPROTO_IP, IP_MULTICAST_TTL, ( gchar * ) &yes,
	  sizeof yes ) == -1 )
    perror( "setsockopt: multicast_ttl" );

  if ( setsockopt( fd, IPPROTO_IP, IP_MULTICAST_LOOP,
	  ( gchar * ) &yes, sizeof yes ) == -1 )
    perror( "setsockopt: multicast_loop" );

  imr.imr_multiaddr = ep->addr.in.sin_addr;
#ifdef HAVE_IP_MREQN
  imr.imr_address.s_addr = INADDR_ANY;
  imr.imr_ifindex = 0;
#else
  imr.imr_interface.s_addr = INADDR_ANY;
#endif
  if ( setsockopt( fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, ( gchar * ) &imr,
	  sizeof imr) == -1 )
    perror( "setsockopt: add_membership" );
}

static void
__mbus_socket_multicast_ipv6( gint fd, MEndpoint * ep )
{
  gint			yes = 1;
  struct ipv6_mreq	imr;

  if ( setsockopt( fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, ( gchar * ) &yes,
	  sizeof yes ) == -1 )
    perror( "setsockopt: multicast_ttl" );

  if ( setsockopt( fd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
	  ( gchar * ) &yes, sizeof yes ) == -1 )
    perror( "setsockopt: multicast_loop" );

  imr.ipv6mr_multiaddr = ep->addr.in6.sin6_addr;
  imr.ipv6mr_interface = 0;

  if ( setsockopt( fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, ( gchar * ) &imr,
	  sizeof imr) == -1 )
    perror( "setsockopt: add_membership" );
}

/**
   \brief Creates a new endpoint initialized for the given address
   family
   \param family the address family; currently only AF_INET and
   AF_INET6 are supported
   \return a newly created MEndpoint object or NULL if an error has occured
*/
MEndpoint *
mbus_endpoint_new( sa_family_t family )
{
  MEndpoint * ep = g_new0( MEndpoint, 1 );

  ep->af_family = family;

  return ep;
}

/**
   \brief Reads an endpoint address from a string
   \param address the string containing the endpoint address
   \return an MEndpoint object with the endpoint address
   \sa mbus_endpoint_set_address_from_string
 */
MEndpoint *
mbus_endpoint_new_from_string( const gchar * address )
{
  MEndpoint * self = g_new0( MEndpoint, 1 );

  mbus_endpoint_set_address_from_string( self, address );

  return self;
}

/**
   \brief Retrieves the address family of the given MEndpoint object
   \param self the MEndpoint object
   \return the address family
 */
sa_family_t
mbus_endpoint_get_family( MEndpoint * self )
{
  return self->af_family;
}

socklen_t
mbus_endpoint_get_size( MEndpoint * self )
{
  if ( self->af_family == AF_INET )
    return sizeof( struct sockaddr_in );
  else
    return sizeof( struct sockaddr_in6 );
}

/**
   \brief Retrieve the port of the given MEndpoint object
   \param self the MEndpoint object
   \return the transport layer port
 */
in_port_t
mbus_endpoint_get_port( MEndpoint * self )
{
  if ( self->af_family == AF_INET )
    return ( ( struct sockaddr_in * ) self )->sin_port;
  else
    return ( ( struct sockaddr_in6 * ) self )->sin6_port;
}

/**
   \brief Set the port of the given MEndpoint object
   \param self the MEndpoint object
   \param port the new port
 */
void
mbus_endpoint_set_port( MEndpoint * self, in_port_t port )
{
  if ( self->af_family == AF_INET )
    ( ( struct sockaddr_in * ) self )->sin_port = port;
  else
    ( ( struct sockaddr_in6 * ) self )->sin6_port = port;
}

/**
   \brief Set the IPv4 address of the given MEndpoint object. If this
   object has not been used as IPv4 endpoint address before than the
   object may contain invalid data after calling this function!
   \param self the MEndpoint object
   \param addr the new IPv4 address
 */
void
mbus_endpoint_set_address4( MEndpoint * self, in_addr_t addr )
{
  self->af_family = AF_INET;
  self->addr.in.sin_addr.s_addr = addr;
}

/**
   \brief Set the IPv6 address of the given MEndpoint object If this
   object has not been used as IPv6 endpoint address before than the
   object may contain invalid data after calling this function!
   \param self the MEndpoint object
   \param addr the new IPv6 address
 */
void
mbus_endpoint_set_address6( MEndpoint * self, struct in6_addr addr )
{
  self->af_family = AF_INET6;
  self->addr.in6.sin6_addr = addr;
}

/**
   \brief Reads an IP address from a string and stores it in the given
   MEndpoint object.
   \param self the MEndpoint object
   \param address the string containg the IP address
 */
gboolean
mbus_endpoint_set_address_from_string( MEndpoint * self, const gchar * address )
{
  gint		ret;
  void *	buffer;

  if ( strchr( address, '.' ) ) { /* IPv4 */
    buffer = &( self->addr.in.sin_addr );
    ret = inet_pton( AF_INET, address, buffer );
    self->af_family = AF_INET;
  } else { /* IPv6 */
    buffer = &( self->addr.in6.sin6_addr );
    ret = inet_pton( AF_INET6, address, buffer );
    self->af_family = AF_INET6;
  }

  return ( ret > 0 );
}

/**
\brief Compares to MEndpoint objects
\param e1
\param e2
\return TRUE if both MEndpoint objects are equal, otherwise FALSE
*/
gboolean
mbus_endpoint_is_equal( const MEndpoint * e1, const MEndpoint * e2 )
{
  return !memcmp( e1, e2, sizeof( MEndpoint ) );
}

/**
   \brief This function is primary meant to be used for debugging
   purposes. It prints out the endpoint address (including the port)
   as a g_message.
   \param self the MEndpoint object to print
 */
void
mbus_endpoint_print( MEndpoint * self )
{
  gchar buffer[ INET6_ADDRSTRLEN + 1 ];
  void * src;

  if ( self->af_family == AF_INET ) {
    src = &( self->addr.in.sin_addr );
    g_message( "MEndpoint: %s:%u",
	inet_ntop( self->af_family, src, buffer, 256 ),
	ntohs( mbus_endpoint_get_port( self ) ) );
  } else {
    src = &( self->addr.in6.sin6_addr );
    g_message( "MEndpoint: [%s]:%u",
	inet_ntop( self->af_family, src, buffer, 256 ),
	ntohs( mbus_endpoint_get_port( self ) ) );
  }
}

/**
\}
*/

/* end of udp.c */
