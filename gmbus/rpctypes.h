/* rpctypes.h
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

#ifndef RPC_TYPES_HH
#define RPC_TYPES_HH

#include "gmbus/maddress.h"
#include "gmbus/mcommand.h"
#include "gmbus/mtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
\addtogroup rpctypes RPC Types
\{
*/

/**
Defining all possible states an RPC return may have.
*/
typedef enum { RPC_OK, RPC_FAILED, RPC_UNKNOWN } RPCStatus;

/**
Defining all possible types of an RPC command.
*/
typedef enum { RPC_UNICAST, RPC_ANYCAST } RPCType;

/**
\brief Represents on Mbus RPC command.
*/
typedef struct {
	/** The source of the RPC command */
	MAddress *    source;
	/** The command name */
	gchar *	command;
	/** The unique RPC id */
	MObject *	id;
	/** The RPC type of this command */
	RPCType	type;
	/** The arguments of this command */
	MObject *	arguments;
} RPCCommand;

/**
\brief Describes an RPC return command.
*/
typedef struct {
	/** The name of the RPC command is return is for. */
	gchar *	command;
	/** The unique of the RPC command is RPC return belongs to */
	MObject *	id;
	/** The RPC status */
	RPCStatus	status;
	/** The application return status */
	RPCStatus	app_status;
	/** A symbol describing the return status */
	gchar *       app_result;
	/** A short description of the return status */
	gchar *       app_description;
	/** The optional return arguments */
	MObject *	arguments;
} RPCReturn;

/**
\brief This type of function is used for RPC command callback functions
*/
typedef RPCReturn * ( *RPCCommandFunc )( const RPCCommand * cmd,
		gpointer data );

/**
\brief This type of function is used for RPC return callback functions
*/
typedef void ( *RPCReturnFunc )( const RPCReturn * ret, gpointer data );

/**
Encapsulate a callback function for RPC commands and an optional argument
passed to this function as a second argument
*/
typedef struct {
	RPCCommandFunc	function;
	gpointer		data;
} RPCCommandCallback;

/**
Encapsulate a callback function for RPC return commands and an optional
argument passed to this function as a second argument
*/
typedef struct {
	RPCReturnFunc	function;
	gpointer		data;
} RPCReturnCallback;

RPCCommand * rpc_command_new( const gchar * name );
RPCCommand * rpc_command_new_from_command( MCommand * command );
void rpc_command_free( RPCCommand * self );

RPCReturn * rpc_return_new_from_command( MCommand * command );
RPCReturn * rpc_return_new( const RPCCommand * cmd, RPCStatus status );
void rpc_return_free( RPCReturn * self );

void rpc_return_set_arguments( RPCReturn * self, MObject * args );

/**
\}
*/

#ifdef __cplusplus
}
#endif

#endif

/* end of rpctypes.h */
