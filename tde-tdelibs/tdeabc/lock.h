/*
    This file is part of libtdeabc.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KABC_LOCK_H
#define KABC_LOCK_H

#include <tqstring.h>
#include <tqobject.h>

#include <tdelibs_export.h>

namespace TDEABC {

/**
  This class provides locking functionality for a file, directory or an
  arbitrary string-represented resource.
*/
class KABC_EXPORT Lock : public TQObject
{
    TQ_OBJECT
  public:
    /**
      Constructor.

      @param identifier An identifier for the resource to be locked, e.g. a file
                        name.
     */
    Lock( const TQString &identifier );

    /**
      Destruct lock object. This also removes the lock on the resource.
    */
    ~Lock();

    /**
      Lock resource.
    */
    virtual bool lock();
    
    /**
      Unlock resource.
    */
    virtual bool unlock();

    virtual TQString error() const;

    TQString lockFileName() const;

    static bool readLockFile( const TQString &filename, int &pid, TQString &app );
    static bool writeLockFile( const TQString &filename );

    static TQString locksDir();

  signals:
    void locked();
    void unlocked();

  private:
    TQString mIdentifier;
    
    TQString mLockUniqueName;

    TQString mError;

    class Private;
    Private *d;
};

}

#endif
