/*
   Copyright (C) 2000-2002 Stephan Kulow <coolo@kde.org>
   Copyright (C) 2000-2002 David Faure <faure@kde.org>
   Copyright (C) 2000-2002 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __file_h__
#define __file_h__ "$Id$"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <unistd.h>

#include <tqobject.h>
#include <tqintdict.h>
#include <tqstring.h>
#include <tqvaluelist.h>

#include <tdeio/global.h>
#include <tdeio/slavebase.h>

// Note that this header file is installed, so think twice
// before breaking binary compatibility (read: it is forbidden :)

class FileProtocol : public TQObject, public TDEIO::SlaveBase
{
  TQ_OBJECT
public:
  FileProtocol( const TQCString &pool, const TQCString &app);
  virtual ~FileProtocol() { }

  virtual void get( const KURL& url );
  virtual void put( const KURL& url, int permissions,
		    bool overwrite, bool resume );
  virtual void copy( const KURL &src, const KURL &dest,
                     int permissions, bool overwrite );
  virtual void rename( const KURL &src, const KURL &dest,
                       bool overwrite );
  virtual void symlink( const TQString &target, const KURL &dest,
                        bool overwrite );

  virtual void stat( const KURL& url );
  virtual void listDir( const KURL& url );
  virtual void mkdir( const KURL& url, int permissions );
  virtual void chmod( const KURL& url, int permissions );
  virtual void del( const KURL& url, bool isfile);

  /**
   * Special commands supported by this slave:
   * 1 - mount
   * 2 - unmount
   * 3 - shred
   */
  virtual void special( const TQByteArray &data);
  void unmount( const TQString& point );
  void mount( bool _ro, const char *_fstype, const TQString& dev, const TQString& point );
  bool pumount( const TQString &point );
  bool pmount( const TQString &dev );

protected slots:
  void slotProcessedSize( TDEIO::filesize_t _bytes );
  void slotInfoMessage( const TQString & msg );

protected:

  bool createUDSEntry( const TQString & filename, const TQCString & path, TDEIO::UDSEntry & entry, 
                       short int details, bool withACL );
  int setACL( const char *path, mode_t perm, bool _directoryDefault );
  
  TQString getUserName( uid_t uid );
  TQString getGroupName( gid_t gid );

  TQIntDict<TQString> usercache;      // maps long ==> TQString *
  TQIntDict<TQString> groupcache;

  class FileProtocolPrivate;
  FileProtocolPrivate *d;
};

#endif
