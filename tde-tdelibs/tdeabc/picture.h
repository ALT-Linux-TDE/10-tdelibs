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

#ifndef KABC_PICTURE_H
#define KABC_PICTURE_H

#include <tqimage.h>

#include <tdelibs_export.h>

namespace TDEABC {

class KABC_EXPORT Picture
{
  friend KABC_EXPORT TQDataStream &operator<<( TQDataStream &, const Picture & );
  friend KABC_EXPORT TQDataStream &operator>>( TQDataStream &, Picture & );

public:

  /**
   * Consturctor. Creates an empty object.
   */
  Picture();

  /**
   * Consturctor.
   *
   * @param url  A URL that describes the position of the picture file.
   */
  Picture( const TQString &url );

  /**
   * Consturctor.
   *
   * @param data  The raw data of the picture.
   */
  Picture( const TQImage &data );

  /**
   * Destructor.
   */
  ~Picture();


  bool operator==( const Picture & ) const;
  bool operator!=( const Picture & ) const;

  /**
   * Sets a URL for the location of the picture file. When using this
   * function, isIntern() will return 'false' until you use
   * setData().
   *
   * @param url  The location URL of the picture file.
   */
  void setUrl( const TQString &url );

  /**
   * Sets the raw data of the picture. When using this function,
   * isIntern() will return 'true' until you use setUrl().
   *
   * @param data  The raw data of the picture.
   */
  void setData( const TQImage &data );

  /**
   * Sets the type of the picture.
   */
  void setType( const TQString &type );

  /**
   * Returns whether the picture is described by a URL (extern) or
   * by the raw data (intern).
   * When this method returns 'true' you can use data() to
   * get the raw data. Otherwise you can request the URL of this
   * picture by url() and load the raw data from that location.
   */
  bool isIntern() const;

  /**
   * Returns the location URL of this picture.
   */
  TQString url() const;

  /**
   * Returns the raw data of this picture.
   */
  TQImage data() const;

  /**
   * Returns the type of this picture.
   */
  TQString type() const;

  /**
   * Returns string representation of the picture.
   */
  TQString asString() const;

private:
  TQString mUrl;
  TQString mType;
  TQImage mData;

  int mIntern;
};

KABC_EXPORT TQDataStream &operator<<( TQDataStream &, const Picture & );
KABC_EXPORT TQDataStream &operator>>( TQDataStream &, Picture & );

}
#endif
