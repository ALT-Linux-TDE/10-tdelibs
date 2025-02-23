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

#ifndef KABC_RESOURCESQL_H
#define KABC_RESOURCESQL_H

#include <tdeconfig.h>

#include "addressbook.h"
#include "resource.h"

class TQSqlDatabase;

namespace TDEABC {

class KABC_EXPORT ResourceSql : public Resource
{
public:
  ResourceSql( AddressBook *ab, const TQString &user, const TQString &password,
    const TQString &db, const TQString &host );
  ResourceSql( AddressBook *ab, const TDEConfig * );
  
  virtual bool open();
  virtual void close();
  
  virtual Ticket *requestSaveTicket();
  virtual void releaseSaveTicket( Ticket* );

  virtual bool load();
  virtual bool save( Ticket * ticket );

  virtual TQString identifier() const;

private:
  void init(const TQString &user, const TQString &password,
      const TQString &db, const TQString &host );

  TQString mUser;
  TQString mPassword;
  TQString mDbName;
  TQString mHost;

  TQSqlDatabase *mDb;
};

}
#endif
