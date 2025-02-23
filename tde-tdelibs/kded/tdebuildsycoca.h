/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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
 **/
#ifndef __tdebuildsycoca_h__
#define __tdebuildsycoca_h__ 

#include <sys/stat.h>

#include <tqobject.h>
#include <tqstring.h>
#include <tqdict.h>

#include <kservice.h>
#include <tdesycoca.h>
#include <tdesycocatype.h>
#include <tdesycocaentry.h>
#include <kservicegroup.h>

#include "vfolder_menu.h"

class TQDataStream;

// No need for this in libtdeio - apps only get readonly access
class KBuildSycoca : public KSycoca
{
   TQ_OBJECT
public:
   KBuildSycoca();
   virtual ~KBuildSycoca();

   /**
    * Recreate the database file
    */
   bool recreate();

   static bool checkTimestamps( TQ_UINT32 timestamp, const TQStringList &dirs );

   static TQStringList existingResourceDirs();
   
   void setTrackId(const TQString &id) { m_trackId = id; }

protected slots:
   void slotCreateEntry(const TQString &file, KService **entry);
       
protected:

   /**
    * Look up gnome mimetypes.
    */
   void processGnomeVfs();

   /**
    * Add single entry to the sycoca database.
    * Either from a previous database or regenerated from file.
    */
   KSycocaEntry *createEntry(const TQString &file, bool addToFactory);

   /**
    * Convert a VFolderMenu::SubMenu to KServiceGroups.
    */
   void createMenu(TQString caption, TQString name, VFolderMenu::SubMenu *menu);

   /**
    * Build the whole system cache, from .desktop files
    */
   bool build();
   
   /**
    * Save the tdesycoca file
    */
   void save();

   /**
    * Clear the factories
    */
   void clear();
   
   static bool checkDirTimestamps( const TQString& dir, const TQDateTime& stamp, bool top );
   
   /**
    * @internal
    * @return true if building (i.e. if a KBuildSycoca);
    */
   virtual bool isBuilding() { return true; }

   TQStringList m_allResourceDirs;
   TQString m_trackId;
};

#endif
