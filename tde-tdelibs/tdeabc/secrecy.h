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

#ifndef KABC_SECRECY_H
#define KABC_SECRECY_H

#include <tqvaluelist.h>

#include <tdelibs_export.h>

namespace TDEABC {

class KABC_EXPORT Secrecy
{
    friend KABC_EXPORT TQDataStream &operator<<( TQDataStream &, const Secrecy & );
    friend KABC_EXPORT TQDataStream &operator>>( TQDataStream &, Secrecy & );

public:
  typedef TQValueList<int> TypeList;  

  /**
   * Secrecy types
   *
   * @li Public       - for public access
   * @li Private      - only private access
   * @li Confidential - access for confidential persons
   */
  enum Types {
    Public,
    Private,
    Confidential,
    Invalid
  };

  /**
   * Constructor.
   *
   * @param type  The secrecy type, see Types.
   */
  Secrecy( int type = Invalid );

  bool operator==( const Secrecy & ) const;
  bool operator!=( const Secrecy & ) const;

  /**
    Returns if the Secrecy object has a valid value.
  */
  bool isValid() const;

  /**
   * Sets the type, see Types.
   */
  void setType( int type );

  /**
   * Returns the type, see Types.
   */
  int type() const;

  /**
   * Returns a list of all available secrecy types.
   */
  static TypeList typeList();
  
  /**
   * Returns a translated label for a given secrecy type.
   */
  static TQString typeLabel( int type );

  /**
   * For debug.
   */
  TQString asString() const;

private:
  int mType;
};

KABC_EXPORT TQDataStream& operator<<( TQDataStream &s, const Secrecy &secrecy );
KABC_EXPORT TQDataStream& operator>>( TQDataStream &s, Secrecy &secrecy );

}
#endif
