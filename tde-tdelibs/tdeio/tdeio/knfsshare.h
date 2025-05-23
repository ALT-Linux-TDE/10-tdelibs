/* This file is part of the KDE project
   Copyright (c) 2004 Jan Schaefer <j_schaef@informatik.uni-kl.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef knfsshare_h
#define knfsshare_h

#include <tqobject.h>

#include <tdelibs_export.h>

class KNFSSharePrivate;

/**
 * Similar functionality like KFileShare, 
 * but works only for NFS and do not need 
 * any suid script.
 * It parses the /etc/exports file to get its information.
 * Singleton class, call instance() to get an instance.
 */
class TDEIO_EXPORT KNFSShare : public TQObject 
{
TQ_OBJECT
public:
  /**
   * Returns the one and only instance of KNFSShare
   */
  static KNFSShare* instance();

  /**
   * Wether or not the given path is shared by NFS.
   * @param path the path to check if it is shared by NFS.
   * @return wether the given path is shared by NFS.
   */
  bool isDirectoryShared( const TQString & path ) const;
  
  /**
   * Returns a list of all directories shared by NFS.
   * The resulting list is not sorted.
   * @return a list of all directories shared by NFS.
   */
  TQStringList sharedDirectories() const;
  
  /**
   * KNFSShare destructor. 
   * Do not call!
   * The instance is destroyed automatically!
   */ 
  virtual ~KNFSShare();
  
  /**
   * Returns the path to the used exports file,
   * or null if no exports file was found
   */
  TQString exportsPath() const;
  
signals:
  /**
   * Emitted when the /etc/exports file has changed
   */
  void changed();  
  
private:
  KNFSShare();
  static KNFSShare* _instance;
  KNFSSharePrivate* d;
  
private slots:
  void slotFileChange(const TQString&);  
};

#endif
