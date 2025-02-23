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

#include "sound.h"

#include <tqdatastream.h>

using namespace TDEABC;

Sound::Sound()
  : mIntern( false )
{
}

Sound::Sound( const TQString &url )
  : mUrl( url ), mIntern( false )
{
}

Sound::Sound( const TQByteArray &data )
  : mData( data ), mIntern( true )
{
}

Sound::~Sound()
{
}

bool Sound::operator==( const Sound &s ) const
{
  if ( mIntern != s.mIntern ) return false;

  if ( mIntern ) {
    if ( mData != s.mData )
      return false;
  } else {
    if ( mUrl != s.mUrl )
      return false;
  }

  return true;
}

bool Sound::operator!=( const Sound &s ) const
{
  return !( s == *this );
}

void Sound::setUrl( const TQString &url )
{
  mUrl = url;
  mIntern = false;
}

void Sound::setData( const TQByteArray &data )
{
  mData = data;
  mIntern = true;
}

bool Sound::isIntern() const
{
  return mIntern;
}

bool Sound::isEmpty() const
{
  return (!mIntern) && mUrl.isEmpty();

}

TQString Sound::url() const
{
  return mUrl;
}

TQByteArray Sound::data() const
{
  return mData;
}

TQString Sound::asString() const
{
  if ( mIntern )
    return "intern sound";
  else
    return mUrl;
}

TQDataStream &TDEABC::operator<<( TQDataStream &s, const Sound &sound )
{
  return s << sound.mIntern << sound.mUrl;
//  return s << sound.mIntern << sound.mUrl << sound.mData;
}

TQDataStream &TDEABC::operator>>( TQDataStream &s, Sound &sound )
{
  s >> sound.mIntern >> sound.mUrl;
//  s >> sound.mIntern >> sound.mUrl >> sound.mData;
  return s;
}
