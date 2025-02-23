/*
    This file is part of libtdeabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_VCARDTOOL_H
#define KABC_VCARDTOOL_H

#include "addressee.h"
#include "vcardparser.h"

class TQDateTime;

namespace TDEABC {

class Agent;
class Key;
class Picture;
class Secrecy;
class Sound;

class KABC_EXPORT VCardTool
{
  public:
    VCardTool();
    ~VCardTool();

    /**
      Creates a string that contains the addressees from the list in
      the vCard format.
     */
    TQString createVCards( Addressee::List list, VCard::Version version = VCard::v3_0 );

    /**
      Parses the string and returns a list of addressee objects.
     */
    Addressee::List parseVCards( const TQString& vcard );

  private:
    /**
      Split a string and replaces escaped separators on the fly with
      unescaped ones.
     */
    TQStringList splitString( const TQChar &sep, const TQString &value );
    
    TQDateTime parseDateTime( const TQString &str );
    TQString createDateTime( const TQDateTime &dateTime );

    Picture parsePicture( const VCardLine &line );
    VCardLine createPicture( const TQString &identifier, const Picture &pic );

    Sound parseSound( const VCardLine &line );
    VCardLine createSound( const Sound &snd );

    Key parseKey( const VCardLine &line );
    VCardLine createKey( const Key &key );

    Secrecy parseSecrecy( const VCardLine &line );
    VCardLine createSecrecy( const Secrecy &secrecy );

    Agent parseAgent( const VCardLine &line );
    VCardLine createAgent( VCard::Version version, const Agent &agent );

    TQMap<TQString, int> mAddressTypeMap;
    TQMap<TQString, int> mPhoneTypeMap;

    class VCardToolPrivate;
    VCardToolPrivate *d;
};

}

#endif
