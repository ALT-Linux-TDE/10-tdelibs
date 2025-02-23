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

#ifndef KABC_KEY_H
#define KABC_KEY_H

#include <tqvaluelist.h>

#include <tdelibs_export.h>

namespace TDEABC {

/**
 * @short A class to store an encryption key.
 */
class KABC_EXPORT Key
{
  friend KABC_EXPORT TQDataStream &operator<<( TQDataStream &, const Key & );
  friend KABC_EXPORT TQDataStream &operator>>( TQDataStream &, Key & );

public:
  typedef TQValueList<Key> List;
  typedef TQValueList<int> TypeList;  

  /**
   * Key types
   *
   * @li X509   - X509 key
   * @li PGP    - Pretty Good Privacy key
   * @li Custom - Custom or IANA conform key
   */
  enum Types {
    X509,
    PGP,
    Custom
  };

  /**
   * Constructor.
   *
   * @param text  The text data.
   * @param type  The key type, see Types.
   */
  Key( const TQString &text = TQString::null, int type = PGP );

  /**
   * Destructor.
   */
  ~Key();
    
  bool operator==( const Key & ) const;
  bool operator!=( const Key & ) const;

  /**
   * Sets the unique identifier.
   */
  void setId( const TQString &id );

  /**
   * Returns the unique identifier.
   */
  TQString id() const;

  /**
   * Sets binary data.
   */
  void setBinaryData( const TQByteArray &binary );

  /**
   * Returns the binary data.
   */
  TQByteArray binaryData() const;

  /**
   * Sets text data.
   */
  void setTextData( const TQString &text );

  /**
   * Returns the text data.
   */
  TQString textData() const;

  /**
   * Returns whether the key contains binary or text data.
   */
  bool isBinary() const;

  /**
   * Sets the type, see Type.
   */
  void setType( int type );

  /**
   * Sets custom type string.
   */
  void setCustomTypeString( const TQString &custom );

  /**
   * Returns the type, see Type.
   */
  int type() const;

  /**
   * Returns the custom type string.
   */
  TQString customTypeString() const;

  /**
   * Returns a list of all available key types.
   */
  static TypeList typeList();
  
  /**
   * Returns a translated label for a given key type.
   */
  static TQString typeLabel( int type );

private:
  TQByteArray mBinaryData;
  TQString mId;
  TQString mTextData;
  TQString mCustomTypeString;

  int mIsBinary;
  int mType;
};

KABC_EXPORT TQDataStream &operator<<( TQDataStream &, const Key & );
KABC_EXPORT TQDataStream &operator>>( TQDataStream &, Key & );

}
#endif
