/*
    This file is part of libtdeabc.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_SORTMODE_H
#define KABC_SORTMODE_H

#include <tdelibs_export.h>

#include <tdeabc/addressee.h>

namespace TDEABC {

/**
  @short Sort method for sorting an addressee list.

  This interface should be reimplemented by classes which shall act as
  SortModes for TDEABC::AddresseeList.
*/
class KABC_EXPORT SortMode
{
  public:
    /**
      Reimplement this method and return whether the first contact is 'smaller'
      than the second.
     */
    virtual bool lesser( const TDEABC::Addressee &first, const TDEABC::Addressee &second ) const = 0;
};

class KABC_EXPORT NameSortMode : public SortMode
{
  public:
    enum NameType
    {
      FormattedName,
      FamilyName,
      GivenName
    };

    /**
      Constructor.

      Creates a NameSortMethod with FormattedName as name type set.
     */
    NameSortMode();

    /**
      Constructor.

      Creates a NameSortMethod with the specified name type.

      @param type The name type.
      @param ascending true for ascending sort, false for descending.
     */
    NameSortMode( NameType type, bool ascending = true );

    /**
      Returns whether the first contact is 'smaller' then the second.
     */
    virtual bool lesser( const TDEABC::Addressee&, const TDEABC::Addressee& ) const;

  private:
    NameType mNameType;
    bool mAscendingOrder;

    class NameSortModePrivate;
    NameSortModePrivate *d;
};

class KABC_EXPORT FieldSortMode : public SortMode
{
  public:
    /**
      Constructor.

      Creates a FieldSortMethod with the specified field.

      @param field The field.
      @param ascending true for ascending sort, false for descending.
     */
    FieldSortMode( TDEABC::Field *field, bool ascending = true );

    /**
      Returns whether the first contact is 'smaller' then the second.
     */
    virtual bool lesser( const TDEABC::Addressee&, const TDEABC::Addressee& ) const;

  private:
    TDEABC::Field *mField;
    bool mAscendingOrder;

    class FieldSortModePrivate;
    FieldSortModePrivate *d;
};

}

#endif
