/*
 *
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
 *
 * $Id$
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
 **/

#include "autostart.h"

#include <tdeconfig.h>
#include <kdesktopfile.h>
#include <tdeglobal.h>
#include <kstandarddirs.h>

#include <stdlib.h>

class AutoStartItem
{
public:
   TQString name;
   TQString service;
   TQString startAfter;
   int     phase;
};

class AutoStartList: public TQPtrList<AutoStartItem>
{
public:
   AutoStartList() { }
};

AutoStart::AutoStart( bool new_startup )
  : m_newStartup( new_startup ), m_phase( new_startup ? -1 : 0), m_phasedone(false)
{
  m_startList = new AutoStartList;
  m_startList->setAutoDelete(true);
  TDEGlobal::dirs()->addResourceType("autostart", "share/autostart");
  TQString xdgdirs = getenv("XDG_CONFIG_DIRS");
  if (xdgdirs.isEmpty())
        xdgdirs = "/etc/xdg";

  TQStringList xdgdirslist = TQStringList::split( ':', xdgdirs );
  for ( TQStringList::Iterator itr = xdgdirslist.begin(); itr != xdgdirslist.end(); ++itr ) {
	TDEGlobal::dirs()->addResourceDir("autostart", (*itr) +"/autostart");
  }
}

AutoStart::~AutoStart()
{
	delete m_startList;
}

void
AutoStart::setPhase(int phase)
{
   if (phase > m_phase)
   {
      m_phase = phase;
      m_phasedone = false;
   }
}

void AutoStart::setPhaseDone()
{
   m_phasedone = true;
}

static TQString extractName(TQString path)
{
  int i = path.findRev('/');
  if (i >= 0)
     path = path.mid(i+1);
  i = path.findRev('.');
  if (i >= 0)
     path = path.left(i);
  return path;
}

static bool startCondition(const TQString &condition)
{
  if (condition.isEmpty())
     return true;

  TQStringList list = TQStringList::split(':', condition, true);
  if (list.count() < 4) 
     return true;
  if (list[0].isEmpty() || list[2].isEmpty()) 
     return true;

  TDEConfig config(list[0], true, false);
  if (!list[1].isEmpty())
     config.setGroup(list[1]);

  bool defaultValue = (list[3].lower() == "true");

  return config.readBoolEntry(list[2], defaultValue);
}

void
AutoStart::loadAutoStartList()
{
   TQStringList files = TDEGlobal::dirs()->findAllResources("xdgconf-autostart", "*.desktop", false, true);
   TQStringList kdefiles = TDEGlobal::dirs()->findAllResources("autostart", "*.desktop", false, true);
   files += kdefiles;
   
   for(TQStringList::ConstIterator it = files.begin(); it != files.end(); ++it)
   {
       // Explicitly skip autostart files from KDE
       if ((*it).contains("org.kde") || (*it).startsWith("/etc/kde/xdg/autostart"))
       {
           continue;
       }
       KDesktopFile config(*it, true);
       if (config.hasKey("X-TDE-autostart-condition")) {
           if (!startCondition(config.readEntry("X-TDE-autostart-condition")))
              continue;
       }
       else if (config.hasKey("X-KDE-autostart-condition")) {
           if (!startCondition(config.readEntry("X-KDE-autostart-condition")))
              continue;
       }
       if (!config.tryExec())
          continue;
       if (config.readBoolEntry("Hidden", false))
          continue;

       // Check to see if the most important ( usually ~/.config/autostart or ~/.trinity/Autostart) XDG directory
       // has overridden the Hidden directive and honor it if set to True
       bool autostartOverriddenAndDisabled = false;
       for(TQStringList::ConstIterator localit = files.begin();
           localit != files.end();
           ++localit)
       {
           if (((*localit).startsWith(TDEGlobal::dirs()->localxdgconfdir()) == true) || ((*localit).startsWith(TDEGlobal::dirs()->localtdedir()) == true)) {
               // Same local file name?
               TQString localOuter;
               TQString localInner;
               int slashPos = (*it).findRev( '/', -1, TRUE );
               if (slashPos == -1) {
                   localOuter = (*it);
               }
               else {
                   localOuter = (*it).mid(slashPos+1);
               }
               slashPos = (*localit).findRev( '/', -1, TRUE );
               if (slashPos == -1) {
                   localInner = (*localit);
               }
               else {
                   localInner = (*localit).mid(slashPos+1);
               }
               if (localOuter == localInner) {
                   // Overridden!
                   // But is Hidden == True?
                   KDesktopFile innerConfig(*localit, true);
                   if (innerConfig.readBoolEntry("Hidden", false)) {
                       // Override confirmed; exit speedily without autostarting
                       autostartOverriddenAndDisabled = true;
                   }
               }
           }
       }

       if (autostartOverriddenAndDisabled == true)
           continue;

       if (config.hasKey("OnlyShowIn"))
       {
          if (!config.readListEntry("OnlyShowIn", ';').contains("TDE"))
              continue;
       }
       if (config.hasKey("NotShowIn"))
       {
           if (config.readListEntry("NotShowIn", ';').contains("TDE") ||
               config.readListEntry("NotShowIn", ';').contains("KDE"))
               continue;
       }

       AutoStartItem *item = new AutoStartItem;
       item->name = extractName(*it);
       item->service = *it;
       if (config.hasKey("X-TDE-autostart-after"))
           item->startAfter = config.readEntry("X-TDE-autostart-after");
       else
           item->startAfter = config.readEntry("X-KDE-autostart-after");
       if( m_newStartup )
       {
          if (config.hasKey("X-TDE-autostart-phase"))
              item->phase = config.readNumEntry("X-TDE-autostart-phase", 2);
          else
              item->phase = config.readNumEntry("X-KDE-autostart-phase", 2);
          if (item->phase < 0)
             item->phase = 0;
       }
       else
       {
          if (config.hasKey("X-TDE-autostart-phase"))
              item->phase = config.readNumEntry("X-TDE-autostart-phase", 1);
          else
              item->phase = config.readNumEntry("X-KDE-autostart-phase", 1);
          if (item->phase < 1)
             item->phase = 1;
       }
       m_startList->append(item);
   }

   // Check for duplicate entries and remove if found
   TQPtrListIterator<AutoStartItem> it1(*m_startList);
   TQPtrListIterator<AutoStartItem> it2(*m_startList);
   AutoStartItem *item1;
   AutoStartItem *item2;
   while ((item1 = it1.current()) != 0) {
       bool dupfound1 = false;
       it2.toFirst();
       while ((item2 = it2.current()) != 0) {
           bool dupfound2 = false;
           if (item2 != item1) {
               if (item1->service == item2->service) {
                   m_startList->removeRef(item2);
                   dupfound1 = true;
                   dupfound2 = true;
               }
           }
           if (!dupfound2) {
               ++it2;
           }
       }
       if (!dupfound1) {
           ++it1;
       }
   }
}

TQString
AutoStart::startService()
{
   if (m_startList->isEmpty())
      return 0;

   while(!m_started.isEmpty())
   {

     // Check for items that depend on previously started items
     TQString lastItem = m_started[0];
     for(AutoStartItem *item = m_startList->first(); 
         item; item = m_startList->next())
     {
        if (item->phase == m_phase
        &&  item->startAfter == lastItem)
        {
           m_started.prepend(item->name);
           TQString service = item->service;
           m_startList->remove();
           return service;
        }
     }
     m_started.remove(m_started.begin());
   }

   // Check for items that don't depend on anything
   AutoStartItem *item;
   for(item = m_startList->first();
       item; item = m_startList->next())
   {
      if (item->phase == m_phase
      &&  item->startAfter.isEmpty())
      {
         m_started.prepend(item->name);
         TQString service = item->service;
         m_startList->remove();
         return service;
      }
   }

   // Just start something in this phase
   for(item = m_startList->first();
       item; item = m_startList->next())
   {
      if (item->phase == m_phase)
      {
         m_started.prepend(item->name);
         TQString service = item->service;
         m_startList->remove();
         return service;
      }
   }

   return 0;
}
