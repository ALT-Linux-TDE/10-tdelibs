/*
 * kdiskfreesp.h
 *
 * Copyright (c) 1999 Michael Kropfberger <michael.kropfberger@gmx.net>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */


#ifndef __KDISKFREESP_H__
#define __KDISKFREESP_H__

#include <tqobject.h>
#include <tqstring.h>

#include <tdelibs_export.h>

class TDEProcess;

/**
 * This class parses the output of "df" to find the disk usage
 * information for a given partition (mount point).
 */
class TDEIO_EXPORT KDiskFreeSp : public TQObject
{  TQ_OBJECT
public:
   KDiskFreeSp( TQObject *parent=0, const char *name=0 );
   /**
    * Destructor - this object autodeletes itself when it's done
    */
   ~KDiskFreeSp();
   /**
    * Call this to fire a search on the disk usage information
    * for @p mountPoint. foundMountPoint will be emitted
    * if this mount point is found, with the info requested.
    * done is emitted in any case.
    */
   int readDF( const TQString & mountPoint );

   /**
    * Call this to fire a search on the disk usage information
    * for the mount point containing @p path.
    * foundMountPoint will be emitted
    * if this mount point is found, with the info requested.
    * done is emitted in any case.
    */
   static KDiskFreeSp * findUsageInfo( const TQString & path );

signals:
   void foundMountPoint( const TQString & mountPoint, unsigned long kBSize, unsigned long kBUsed, unsigned long kBAvail );

   // This one is a hack around a weird (compiler?) bug. In the former signal,
   // the slot in KPropsDlg would get 0L, 0L as the last two parameters.
   // When using const ulong& instead, all is ok.
   void foundMountPoint( const unsigned long&, const unsigned long&, const unsigned long&, const TQString& );
   void done();

private slots:
   void receivedDFStdErrOut(TDEProcess *, char *data, int len);
   void dfDone();

private:
  TDEProcess         *dfProc;
  TQCString          dfStringErrOut;
  TQString           m_mountPoint;
  bool              readingDFStdErrOut;
  class KDiskFreeSpPrivate;
  KDiskFreeSpPrivate * d;
};
/***************************************************************************/


#endif
