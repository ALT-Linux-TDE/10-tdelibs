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

#include <tdelocale.h>

#include "secrecy.h"

using namespace TDEABC;

Secrecy::Secrecy( int type )
  : mType( type )
{
}

bool Secrecy::operator==( const Secrecy &s ) const
{
  return ( mType == s.mType );
}

bool Secrecy::operator!=( const Secrecy &s ) const
{
  return !( *this == s );
}

bool Secrecy::isValid() const
{
  return mType != Invalid;
}

void Secrecy::setType( int type )
{
  mType = type;
}

int Secrecy::type() const
{
  return mType;
}

Secrecy::TypeList Secrecy::typeList()
{
  static TypeList list;

  if ( list.isEmpty() )
    list << Public << Private << Confidential;

  return list;
}

TQString Secrecy::typeLabel( int type )
{
  switch ( type ) {
    case Public:
      return i18n( "Public" );
      break;
    case Private:
      return i18n( "Private" );
      break;
    case Confidential:
      return i18n( "Confidential" );
      break;
    default:
      return i18n( "Unknown type" );
      break;
  }
}

TQString Secrecy::asString() const
{
  return typeLabel( mType );
}

TQDataStream &TDEABC::operator<<( TQDataStream &s, const Secrecy &secrecy )
{
    return s << secrecy.mType;
}

TQDataStream &TDEABC::operator>>( TQDataStream &s, Secrecy &secrecy )
{
    s >> secrecy.mType;

    return s;
}
