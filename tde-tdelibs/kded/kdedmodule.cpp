/*
   This file is part of the KDE libraries

   Copyright (c) 2001 Waldo Bastian <bastian@kde.org>

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

#include <tqtimer.h>

#include "kded.h"
#include "kdedmodule.h"
#include "tdeconfigdata.h"

typedef TQMap<KEntryKey, TDESharedPtr<TDEShared> > KDEDObjectMap;

class KDEDModulePrivate
{
public:
  KDEDObjectMap *objMap;
  int timeout;
  TQTimer timer;
};

KDEDModule::KDEDModule(const TQCString &name) : TQObject(), DCOPObject(name)
{
   d = new KDEDModulePrivate;
   d->objMap = 0;
   d->timeout = 0;
   connect(&(d->timer), TQ_SIGNAL(timeout()), this, TQ_SLOT(idle()));
}
  
KDEDModule::~KDEDModule()
{
   emit moduleDeleted(this);
   delete d; d = 0;
}
  
void KDEDModule::setIdleTimeout(int secs)
{
   d->timeout = secs*1000;
}

void KDEDModule::resetIdle()
{
   d->timer.stop();
   if (!d->objMap || d->objMap->isEmpty())
      d->timer.start(d->timeout, true);
}

void KDEDModule::insert(const TQCString &app, const TQCString &key, TDEShared *obj)
{
   if (!d->objMap)
      d->objMap = new KDEDObjectMap;

   // appKey acts as a placeholder
   KEntryKey appKey(app, 0);
   d->objMap->replace(appKey, 0);

   KEntryKey indexKey(app, key);

   // Prevent deletion in case the same object is inserted again.
   TDESharedPtr<TDEShared> _obj = obj; 

   d->objMap->replace(indexKey, _obj);
   resetIdle();
}

TDEShared * KDEDModule::find(const TQCString &app, const TQCString &key)
{
   if (!d->objMap)
      return 0;
   KEntryKey indexKey(app, key);

   KDEDObjectMap::Iterator it = d->objMap->find(indexKey);
   if (it == d->objMap->end())
      return 0;

   return it.data().data();
}
  
void KDEDModule::remove(const TQCString &app, const TQCString &key)
{
   if (!d->objMap)
      return;
   KEntryKey indexKey(app, key);

   d->objMap->remove(indexKey);
   resetIdle();
}

void KDEDModule::removeAll(const TQCString &app)
{
   if (!d->objMap)
      return;

   KEntryKey indexKey(app, 0);
   // Search for placeholder.

   KDEDObjectMap::Iterator it = d->objMap->find(indexKey);
   while (it != d->objMap->end())
   {
      KDEDObjectMap::Iterator it2 = it++;
      if (it2.key().mGroup != app)
         break; // All keys for this app have been removed.
      d->objMap->remove(it2);  
   }
   resetIdle();
}

bool KDEDModule::isWindowRegistered(long windowId)
{
   return Kded::self()->isWindowRegistered(windowId);
}
#include "kdedmodule.moc"
