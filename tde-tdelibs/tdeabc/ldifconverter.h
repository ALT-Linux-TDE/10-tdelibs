/*
    This file is part of libtdeabc.
    Copyright (c) 2003 Helge Deller <deller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 only as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KABC_LDIFCONVERTER_H
#define KABC_LDIFCONVERTER_H

#include <tqstring.h>
#include <tqdatetime.h>

#include "addressee.h"
#include "addresseelist.h"

namespace TDEABC {

  /**
   * A set of functions to convert a string with LDIF information to addressees 
   * and vice versa. It is useful for addressbook import- and exportfilters 
   * and might be used to read and write Mozilla and Netscape addresssbooks.
   */

  namespace LDIFConverter {

  /**
   * Converts a LDIF string to a list of addressees.
   *
   * @param str         The vcard string.
   * @param addrList    The addresseelist.
   * @param dt          The date & time value of the last modification (e.g. file modification time).
   * @since 3.2
   */
  KABC_EXPORT bool LDIFToAddressee( const TQString &str, AddresseeList &addrList, TQDateTime dt = TQDateTime::currentDateTime() );

  /**
   * Converts a list of addressees to a LDIF string.
   *
   * @param addrList    The addresseelist.
   * @param str         The LDIF string.
   * @since 3.2
   */
  KABC_EXPORT bool addresseeToLDIF( const AddresseeList &addrList, TQString &str );

  /**
   * Converts an addressee to a LDIF string.
   *
   * @param addr    The addressee.
   * @param str     The LDIF string.
   * @since 3.2
   */
  KABC_EXPORT bool addresseeToLDIF( const Addressee &addr, TQString &str );

  /**
   * @deprecated
   * Obsoleted - please use LDIF::assembleLine()
   * Returns a LDIF compatible string representing a given field/value pair.
   * If necessary, the value parameter will be base64encoded and split into multiple.
   * This function will return an empty string if the given value is empty.
   *
   * @param field   The LDAP field name or a complete LDIF field string (e.g. "cn" or "cn = %1\n").
   * @param value   The value for this field.
   * @param allowEncode Set to false if you wish no encoding of the value.
   * @since 3.2
   */
  KABC_EXPORT TQString makeLDIFfieldString( TQString field, TQString value, bool allowEncode = true ) TDE_DEPRECATED;



  /* internal functions - do not use !! */
  
  /** No need for this function anymore - use LDIF::splitLine() + evaluatePair() */
  KABC_EXPORT bool parseSingleLine( Addressee &a,
		Address &homeAddr, Address &workAddr, TQString &line );

  /** No need for this function anymore - use LDIF::splitLine() */
  KABC_EXPORT bool splitLine( TQString &line, TQString &fieldname, TQString &value);


  KABC_EXPORT bool evaluatePair( Addressee &a, Address &homeAddr, Address &workAddr,
	TQString &fieldname, TQString &value );

  }

}
#endif

