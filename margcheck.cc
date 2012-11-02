/** 
 * @file margcheck.cc
 *
 * @brief utility functions for argument checking
 *
 * @author Eilert Brinkmann \<eilert@tzi.org\>
 * $Revision: 1.4 $
 * $Date: 2004/03/11 11:59:17 $
 *
 * Copyright (C) 2002, 2003, 2004 Eilert Brinkmann
 *
 * @license
 * This file is part of the TZI SIP stack.
 *
 * The TZI SIP stack is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The TZI SIP stack is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the TZI SIP stack; if not, write to 
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307 USA 
 *
 * @endlicense
 */

/* $Id: margcheck.cc,v 1.4 2004/03/11 11:59:17 bergmann Exp $ */

#include <libgensrv/debug.h>
#include <libmbus/mbus.hh>
#include <string>

// Don't use /usr/include/local/regex.h on Solaris!
#include </usr/include/regex.h>

#include "margcheck.hh"

using namespace libmbus;

std::string MArgCheck::toString(const MList& args)
{
  std::string p("");
  
  for (unsigned int i = 0; i < args.size(); i++) {
    if (args[i] == NULL) {
      p += '0';
    } else {
      switch (args[i]->type()) {
      case MObject::Char:           p += 'c'; break;
      case MObject::Bool:           p += 'b'; break; // obsolete
      case MObject::Int:            p += 'i'; break;
      case MObject::Float:          p += 'f'; break;
      case MObject::String:         p += 's'; break;
      case MObject::Symbol:         p += 'y'; break;
      case MObject::Data:           p += 'd'; break;
      case MObject::Sequence:       p += 'S'; break;
      case MObject::Command:        p += 'C'; break;
      case MObject::Payload:        p += 'P'; break;
      case MObject::AddressElement: p += 'E'; break;
      case MObject::Address:        p += 'A'; break;
      case MObject::Header:         p += 'H'; break;
      case MObject::Message:        p += 'M'; break;
      case MObject::List:
	p += '<' + toString(*dynamic_cast<MList*>(args[i])) + '>';
	break;
      default: // includes MObject::Unknown
	p += 'U';
	break;
      }
    }
  }
  return p;
}

bool MArgCheck::check(const MList& args, const char* regex)
{
  MArgCheck mac(regex);
  return mac.matches(args);
}

MArgCheck::MArgCheck(const char* regex)
  : pattern(regex)
{
  error = regcomp(&_regex, regex, REG_EXTENDED);
  if (error != 0) {
    char temp[64];
    if (regerror(error, &_regex, temp, 64) == 0) {
      sprintf (temp, "unknown error");
    }
    std::cerr << "Invalid regular expression '" << regex << "': "
	      << temp << std::endl;
  }
}

MArgCheck::~MArgCheck()
{
  // FIXME:
//    if (error == 0)
//      regfree(&_regex);
}

MArgCheck::operator bool() const
{
  return error == 0;
}

bool MArgCheck::matches(const MList& args) const
{
  std::string s(toString(args));
  bool ok = *this && regexec(&_regex, s.c_str(), 0, NULL, 0) == 0;
  if (!ok) {
    DEBUG_MSG("Argument format is " << s << ", expected " << pattern);
  }
  return ok;
}



namespace libmbus {

  MList* makeMList(MObject* o1)
  {
    MList* l = new MList;
    if (o1 != NULL)
      *l << o1;
    return l;
  }

  MList* makeMList(MObject* o1, MObject* o2)
  {
    MList* l = makeMList(o1);
    if (o2 != NULL)
      *l << o2;
    return l;
  }

  MList* makeMList(MObject* o1, MObject* o2, MObject* o3)
  {
    MList* l = makeMList(o1, o2);
    if (o3 != NULL)
      *l << o3;
    return l;
  }

  MList* makeMList(MObject* o1, MObject* o2, MObject* o3, MObject* o4)
  {
    MList* l = makeMList(o1, o2, o3);
    if (o4 != NULL)
      *l << o4;
    return l;
  }

  MList* makeMList(MObject* o1, MObject* o2, MObject* o3,
		   MObject* o4, MObject* o5)
  {
    MList* l = makeMList(o1, o2, o3, o4);
    if (o5 != NULL)
      *l << o5;
    return l;
  }

  MList* makeMList(MObject* o1, MObject* o2, MObject* o3,
		   MObject* o4, MObject* o5, MObject* o6)
  {
    MList* l = makeMList(o1, o2, o3, o4, o5);
    if (o6 != NULL)
      *l << o6;
    return l;
  }

}
