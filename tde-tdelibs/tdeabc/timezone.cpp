/*
    This file is part of libtdeabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <tqdatastream.h>

#include "timezone.h"

using namespace TDEABC;

TimeZone::TimeZone() :
  mOffset( 0 ), mValid( false )
{
}

TimeZone::TimeZone( int offset ) :
  mOffset( offset ), mValid( true )
{
}

void TimeZone::setOffset( int offset )
{
  mOffset = offset;
  mValid = true;
}

int TimeZone::offset() const
{
  return mOffset;
}

bool TimeZone::isValid() const
{
  return mValid;
}

bool TimeZone::operator==( const TimeZone &t ) const
{
  if ( !t.isValid() && !isValid() ) return true;
  if ( !t.isValid() || !isValid() ) return false;
  if ( t.mOffset == mOffset ) return true;
  return false;
}

bool TimeZone::operator!=( const TimeZone &t ) const
{
  if ( !t.isValid() && !isValid() ) return false;
  if ( !t.isValid() || !isValid() ) return true;
  if ( t.mOffset != mOffset ) return true;
  return false;
}

TQString TimeZone::asString() const
{
  return TQString::number( mOffset );
}

TQDataStream &TDEABC::operator<<( TQDataStream &s, const TimeZone &zone )
{
    return s << zone.mOffset;
}

TQDataStream &TDEABC::operator>>( TQDataStream &s, TimeZone &zone )
{
    s >> zone.mOffset;
    zone.mValid = true;

    return s;
}
