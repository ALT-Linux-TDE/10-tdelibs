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
#ifndef KABC_BINARYFORMAT_H
#define KABC_BINARYFORMAT_H

#include "formatplugin.h"

namespace TDEABC {

class AddressBook;
class Addressee;

/**
  @short binary file format for addressbook entries.
*/
class BinaryFormat : public FormatPlugin
{
public:
  /**
   * Load single addressee from file.
   */
  bool load( Addressee &, TQFile *file );

  /**
   * Load whole addressee from file.
   */
  bool loadAll( AddressBook *, Resource *, TQFile *file );

  /**
   * Save single addressee to file.
   */
  void save( const Addressee &, TQFile *file );

  /**
   * Save all addressees to file.
   */
  void saveAll( AddressBook *, Resource *, TQFile *file );

  /**
   * Check for valid format of a file.
   */
  bool checkFormat( TQFile *file ) const;

private:
  void loadAddressee( Addressee &, TQDataStream & );
  void saveAddressee( const Addressee &, TQDataStream & );
  bool checkHeader( TQDataStream & ) const;
  void writeHeader( TQDataStream & );
};

}
#endif
