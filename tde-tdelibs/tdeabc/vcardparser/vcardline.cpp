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

#include "vcardline.h"

using namespace TDEABC;

class VCardLine::VCardLinePrivate
{
  public:
    TQString mGroup;
};

VCardLine::VCardLine()
  : d( 0 )
{
}

VCardLine::VCardLine( const TQString &identifier )
  : d( 0 )
{
  mIdentifier = identifier;
}

VCardLine::VCardLine( const TQString &identifier, const TQVariant &value )
  : d( 0 )
{
  mIdentifier = identifier;
  mValue = value;
}

VCardLine::VCardLine( const VCardLine& line )
  : d( 0 )
{
  mParamMap = line.mParamMap;
  mValue = line.mValue;
  mIdentifier = line.mIdentifier;
}

VCardLine::~VCardLine()
{
  delete d;
  d = 0;
}

VCardLine& VCardLine::operator=( const VCardLine& line )
{
  if ( &line == this )
    return *this;

  mParamMap = line.mParamMap;
  mValue = line.mValue;
  mIdentifier = line.mIdentifier;

  return *this;
}

void VCardLine::setIdentifier( const TQString& identifier )
{
  mIdentifier = identifier;
}

TQString VCardLine::identifier() const
{
  return mIdentifier;
}

void VCardLine::setValue( const TQVariant& value )
{
  mValue = value;
}

TQVariant VCardLine::value() const
{
  return mValue;
}

void VCardLine::setGroup( const TQString& group )
{
  if ( !d )
    d = new VCardLinePrivate();

  d->mGroup = group;
}

TQString VCardLine::group() const
{
  if ( d )
    return d->mGroup;
  else
    return TQString();
}

bool VCardLine::hasGroup() const
{
  if ( !d )
    return false;
  else
    return d->mGroup.isEmpty();
}

TQStringList VCardLine::parameterList() const
{
  return mParamMap.keys();
}

void VCardLine::addParameter( const TQString& param, const TQString& value )
{
  TQStringList &list = mParamMap[ param ];
  if ( list.findIndex( value ) == -1 ) // not included yet
    list.append( value );
}

TQStringList VCardLine::parameters( const TQString& param ) const
{
  ParamMap::ConstIterator it = mParamMap.find( param );
  if ( it == mParamMap.end() )
    return TQStringList();
  else
    return *it;
}

TQString VCardLine::parameter( const TQString& param ) const
{
  ParamMap::ConstIterator it = mParamMap.find( param );
  if ( it == mParamMap.end() )
    return TQString::null;
  else {
    if ( (*it).isEmpty() )
      return TQString::null;
    else
      return (*it).first();
  }
}
