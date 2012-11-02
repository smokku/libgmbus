/* mbus.h
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

#ifndef MBUS_MBUS_H
#define MBUS_MBUS_H

/* command prefixes (must be terminated with '.') */
#define MBUS_PREFIX_STATUS      "status."
#define MBUS_PREFIX_MANAGEMENT  "mbus."

/* RPC-related symbols */
/* suffix for RPC responses */
#define MBUS_SUFFIX_RPC_RETURN  ".return"
#define MBUS_RPC_ID             "ID"
#define MBUS_RPC_STATUS         "RPC-STATUS"
#define MBUS_RPC_TYPE           "RPC-TYPE"

/* status symbols for RPC responses */
#define MBUS_RPC_OK             "OK"
/* #define MBUS_RPC_ERROR          "ERROR" */
#define MBUS_RPC_FAILED          "FAILED"
#define MBUS_RPC_UNKNOWN        "UNKNOWN"

/* properties */
#define MBUS_SUFFIX_SET         "set"
#define MBUS_SUFFIX_GET         "get"
#define MBUS_SUFFIX_WATCH       "watch"
#define MBUS_SUFFIX_UNWATCH     "unwatch"
#define MBUS_CMD_PROPERTIES     "properties"

/* "no control" service model */
#define MBUS_CMD_REGISTER       "register"
#define MBUS_CMD_DEREGISTER     "deregister"
#define MBUS_CMD_REGISTERED     "registered"

/* commands */
/* ...beginning with MBUS_PREFIX_MANAGEMENT */
#define MBUS_CMD_HELLO          "hello"
#define MBUS_CMD_BYE            "bye"
#define MBUS_CMD_PING           "ping"
#define MBUS_CMD_WAITING        "waiting"
#define MBUS_CMD_GO             "go"
#define MBUS_CMD_QUERY          "query"
#define MBUS_CMD_INFO           "info"
#define MBUS_CMD_POLL           "poll"
#define MBUS_CMD_VOTE           "vote"

#define MBUS_HELLO MBUS_PREFIX_MANAGEMENT MBUS_CMD_HELLO
#define MBUS_BYE MBUS_PREFIX_MANAGEMENT MBUS_CMD_BYE


/* commands */
/* ...beginning with MBUS_PREFIX_STATUS */
#define MBUS_CMD_ERROR           "error"
#define MBUS_CMD_WARN            "warn"
#define MBUS_CMD_INFO            "info"

/* wildcard */
#define MBUS_DONTCARE          "*"

/* role address key */
#define MBUS_ROLE              "role"

#endif /* MBUS_MBUS_H */
