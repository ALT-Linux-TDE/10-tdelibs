/*
    This file is part of libtdeabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KABC_VCARDCONVERTER_H
#define KABC_VCARDCONVERTER_H

#include <tqstring.h>

#include "addressee.h"

namespace TDEABC {

/**
  @short Class to converting contact objects into vCard format and vice versa.
  
  This class implements reading and writing of contact using from/to the
  vCard format. Currently vCard version 2.1 and 3.0 is supported.

  Example:

  \code

  TQFile file( "myfile.vcf" );
  file.open( IO_ReadOnly );
  
  TQString data = file.readAll();

  VCardConverter converter;
  Addressee::List list = converter.parseVCards( data );

  // print formatted name of first contact
  tqDebug( "name=%s", list[ 0 ].formattedName().latin1() );

  \endcode
*/
class KABC_EXPORT VCardConverter
{
  public:

  /**
    @li v2_1 - VCard format version 2.1
    @li v3_0 - VCard format version 3.0
   */
   enum Version
    {
      v2_1,
      v3_0
    };

    /**
      Constructor.
     */
    VCardConverter();

    /**
      Destructor.
     */
    ~VCardConverter();
  
    /**
      Creates a string in vCard format which contains the given
      contact.

      @param addr The contact object
      @param version The version of the generated vCard format
     */
    TQString createVCard( const Addressee &addr, Version version = v3_0 );

    /**
      Creates a string in vCard format which contains the given
      list of contact.

      @param list The list of contact objects
      @param version The version of the generated vCard format
     */
    // FIXME: Add error handling
    TQString createVCards( Addressee::List list, Version version = v3_0 );

    // FIXME: Add "createVCards( AddressBook * )"

    /**
      Parses a string in vCard format and returns the first contact.
     */
    Addressee parseVCard( const TQString& vcard );

    /**
      Parses a string in vCard format and returns a list of contact objects.
     */
    // FIXME: Add error handling
    Addressee::List parseVCards( const TQString& vcard );

    // FIXME: Add "bool parseVCards( AddressBook *, const TQString &vcard )"

    /**
      @deprecated
     */
    bool vCardToAddressee( const TQString&, Addressee &, Version version = v3_0 ) TDE_DEPRECATED;

    /**
      @deprecated
     */
    bool addresseeToVCard( const Addressee&, TQString&, Version version = v3_0 ) TDE_DEPRECATED;

  private:
    /**
      Split a string and replaces escaped separators on the fly with
      unescaped ones.
     */
    TQStringList splitString( const TQChar &sep, const TQString &value );

    struct VCardConverterData;
    VCardConverterData *d;
};


/**
    Helper functions
  */

/**
  * Converts a TQDateTime to a date string as it is used in VCard and LDIF files.
  * The return value is in the form "yyyyMMddThhmmssZ" (e.g. "20031201T120000Z")
  * @param dateTime date and time to be converted 
  * @since 3.2
  */
KABC_EXPORT TQString dateToVCardString( const TQDateTime &dateTime );

/**
  * Converts a TQDate to a short date string as it is used in VCard and LDIF files.
  * The return value is in the form "yyyyMMdd" (e.g. "20031201")
  * @param date date to be converted 
  * @since 3.2
  */
KABC_EXPORT TQString dateToVCardString( const TQDate &date );

/**
  * Converts a date string as it is used in VCard and LDIF files to a TQDateTime value.
  * If the date string does not contain a time value, it will be returned as 00:00:00.
  * (e.g. "20031201T120000" will return a TQDateTime for 2003-12-01 at 12:00)
  * @param dateString string representing the date and time.
  * @since 3.2
  */
KABC_EXPORT TQDateTime VCardStringToDate( const TQString &dateString );

}
#endif
