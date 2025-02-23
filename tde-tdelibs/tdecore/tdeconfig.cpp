/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (C) 1997-1999 Matthias Kalle Dalheimer (kalle@kde.org)

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

// $Id$

#include <config.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <stdlib.h>
#include <unistd.h>

#include <tqfileinfo.h>

#include <tdeapplication.h>
#include "tdeconfigbackend.h"

#include "tdeconfig.h"
#include "tdeglobal.h"
#include "kstandarddirs.h"
#include "kstaticdeleter.h"
#include <tqtimer.h>

TDEConfig::TDEConfig( const TQString& fileName,
                 bool bReadOnly, bool bUseKderc, const char *resType )
  : TDEConfigBase(), bGroupImmutable(false), bFileImmutable(false),
    bForceGlobal(false)
{
  // set the object's read-only status.
  setReadOnly(bReadOnly);

  // for right now we will hardcode that we are using the INI
  // back end driver.  In the future this should be converted over to
  // a object factory of some sorts.
  TDEConfigINIBackEnd *aBackEnd = new TDEConfigINIBackEnd(this,
						      fileName,
                                                      resType,
						      bUseKderc);

  // set the object's back end pointer to this new backend
  backEnd = aBackEnd;

  // read initial information off disk
  reparseConfiguration();

  // we let TDEStandardDirs add custom user config files. It will do
  // this only once. So only the first call ever to this constructor
  // will anything else than return here We have to reparse here as
  // configuration files may appear after customized directories have
  // been added. and the info they contain needs to be inserted into the
  // config object.
  // Since this makes only sense for config directories, addCustomized
  // returns true only if new config directories appeared.
  if (TDEGlobal::dirs()->addCustomized(this))
      reparseConfiguration();
}

TDEConfig::TDEConfig(TDEConfigBackEnd *aBackEnd, bool bReadOnly)
    : bGroupImmutable(false), bFileImmutable(false),
    bForceGlobal(false)
{
  setReadOnly(bReadOnly);
  backEnd = aBackEnd;
  reparseConfiguration();
}

TDEConfig::~TDEConfig()
{
  sync();

  delete backEnd;
}

void TDEConfig::rollback(bool bDeep)
{
  TDEConfigBase::rollback(bDeep);

  if (!bDeep)
    return; // object's bDeep flag is set in TDEConfigBase method

  // clear any dirty flags that entries might have set
  for (KEntryMapIterator aIt = aEntryMap.begin();
       aIt != aEntryMap.end(); ++aIt)
    (*aIt).bDirty = false;
}

TQStringList TDEConfig::groupList() const
{
  TQStringList retList;

  KEntryMapConstIterator aIt = aEntryMap.begin();
  KEntryMapConstIterator aEnd = aEntryMap.end();
  for (; aIt != aEnd; ++aIt)
  {
    while(aIt.key().mKey.isEmpty())
    {
      TQCString group = aIt.key().mGroup;
      ++aIt;
      while (true)
      {
         if (aIt == aEnd)
            return retList; // done

         if (aIt.key().mKey.isEmpty())
            break; // Group is empty, next group

         if (!aIt.key().bDefault && !(*aIt).bDeleted)
         {
            if (group != "$Version") // Special case!
               retList.append(TQString::fromUtf8(group));
            break; // Group is non-empty, added, next group
         }
         ++aIt;
      }
    }
  }

  return retList;
}

TQMap<TQString, TQString> TDEConfig::entryMap(const TQString &pGroup) const
{
  TQCString pGroup_utf = pGroup.utf8();
  KEntryKey groupKey( pGroup_utf, 0 );
  TQMap<TQString, TQString> tmpMap;

  KEntryMapConstIterator aIt = aEntryMap.find(groupKey);
  if (aIt == aEntryMap.end())
     return tmpMap;
  ++aIt; // advance past special group entry marker
  for (; aIt.key().mGroup == pGroup_utf && aIt != aEntryMap.end(); ++aIt)
  {
    // Leave the default values out && leave deleted entries out
    if (!aIt.key().bDefault && !(*aIt).bDeleted)
      tmpMap.insert(TQString::fromUtf8(aIt.key().mKey), TQString::fromUtf8((*aIt).mValue.data(), (*aIt).mValue.length()));
  }

  return tmpMap;
}

void TDEConfig::reparseConfiguration()
{
  // Don't lose pending changes
  if (!isReadOnly() && backEnd && bDirty)
    backEnd->sync();

  aEntryMap.clear();

  // add the "default group" marker to the map
  KEntryKey groupKey("<default>", 0);
  aEntryMap.insert(groupKey, KEntry());

  bFileImmutable = false;
  parseConfigFiles();
  bFileImmutable = bReadOnly;
}

KEntryMap TDEConfig::internalEntryMap(const TQString &pGroup) const
{
  TQCString pGroup_utf = pGroup.utf8();
  KEntry aEntry;
  KEntryMapConstIterator aIt;
  KEntryKey aKey(pGroup_utf, 0);
  KEntryMap tmpEntryMap;

  aIt = aEntryMap.find(aKey);
  if (aIt == aEntryMap.end()) {
    // the special group key is not in the map,
    // so it must be an invalid group.  Return
    // an empty map.
    return tmpEntryMap;
  }
  // we now have a pointer to the nodes we want to copy.
  for (; aIt.key().mGroup == pGroup_utf && aIt != aEntryMap.end(); ++aIt)
  {
    tmpEntryMap.insert(aIt.key(), *aIt);
  }

  return tmpEntryMap;
}

void TDEConfig::putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup)
{
  if (bFileImmutable && !_key.bDefault)
    return;

  // check to see if the special group key is present,
  // and if not, put it in.
  if (_checkGroup)
  {
    KEntryKey groupKey( _key.mGroup, 0);
    KEntry &entry = aEntryMap[groupKey];
    bGroupImmutable = entry.bImmutable;
  }
  if (bGroupImmutable && !_key.bDefault)
    return;

  // now either add or replace the data
  KEntry &entry = aEntryMap[_key];
  bool immutable = entry.bImmutable;
  if (immutable && !_key.bDefault)
    return;

  entry = _data;
  entry.bImmutable |= immutable;
  entry.bGlobal |= bForceGlobal; // force to kdeglobals

  if (_key.bDefault)
  {
     // We have added the data as default value,
     // add it as normal value as well.
     KEntryKey key(_key);
     key.bDefault = false;
     aEntryMap[key] = _data;
  }
}

KEntry TDEConfig::lookupData(const KEntryKey &_key) const
{
  KEntryMapConstIterator aIt = aEntryMap.find(_key);
  if (aIt != aEntryMap.end())
  {
    const KEntry &entry = *aIt;
    if (entry.bDeleted)
       return KEntry();
    else
       return entry;
  }
  else {
    return KEntry();
  }
}

bool TDEConfig::internalHasGroup(const TQCString &group) const
{
  KEntryKey groupKey( group, 0);

  KEntryMapConstIterator aIt = aEntryMap.find(groupKey);
  KEntryMapConstIterator aEnd = aEntryMap.end();

  if (aIt == aEnd)
     return false;
  ++aIt;
  for(; (aIt != aEnd); ++aIt)
  {
     if (aIt.key().mKey.isEmpty())
        break;

     if (!aIt.key().bDefault && !(*aIt).bDeleted)
        return true;
  }
  return false;
}

void TDEConfig::setFileWriteMode(int mode)
{
  backEnd->setFileWriteMode(mode);
}

TDELockFile::Ptr TDEConfig::lockFile(bool bGlobal)
{
  TDEConfigINIBackEnd *aBackEnd = dynamic_cast<TDEConfigINIBackEnd*>(backEnd);
  if (!aBackEnd) return 0;
  return aBackEnd->lockFile(bGlobal);
}

void TDEConfig::checkUpdate(const TQString &id, const TQString &updateFile)
{
  TQString oldGroup = group();
  setGroup("$Version");
  TQString cfg_id = updateFile+":"+id;
  TQStringList ids = readListEntry("update_info");
  if (!ids.contains(cfg_id))
  {
     TQStringList args;
     args << "--check" << updateFile;
     TDEApplication::tdeinitExecWait("tdeconf_update", args);
     reparseConfiguration();
  }
  setGroup(oldGroup);
}

TDEConfig* TDEConfig::copyTo(const TQString &file, TDEConfig *config) const
{
  if (!config)
     config = new TDEConfig(TQString::null, false, false);
  config->backEnd->changeFileName(file, "config", false);
  config->setReadOnly(false);
  config->bFileImmutable = false;
  config->backEnd->mConfigState = ReadWrite;

  TQStringList groups = groupList();
  for(TQStringList::ConstIterator it = groups.begin();
      it != groups.end(); ++it)
  {
     TQMap<TQString, TQString> map = entryMap(*it);
     config->setGroup(*it);
     for (TQMap<TQString,TQString>::Iterator it2  = map.begin();
          it2 != map.end(); ++it2)
     {
        config->writeEntry(it2.key(), it2.data());
     }

  }
  return config;
}

void TDEConfig::virtual_hook( int id, void* data )
{ TDEConfigBase::virtual_hook( id, data ); }

static KStaticDeleter< TQValueList<TDESharedConfig*> > sd;
TQValueList<TDESharedConfig*> *TDESharedConfig::s_list = 0;

TDESharedConfig::Ptr TDESharedConfig::openConfig(const TQString& fileName, bool readOnly, bool useKDEGlobals )
{
  if (s_list)
  {
     for(TQValueList<TDESharedConfig*>::ConstIterator it = s_list->begin();
         it != s_list->end(); ++it)
     {
        if ((*it)->backEnd->fileName() == fileName &&
                (*it)->bReadOnly == readOnly &&
                (*it)->backEnd->useKDEGlobals == useKDEGlobals )
           return (*it);
     }
  }
  return new TDESharedConfig(fileName, readOnly, useKDEGlobals);
}

TDESharedConfig::TDESharedConfig( const TQString& fileName, bool readonly, bool usekdeglobals)
 : TDEConfig(fileName, readonly, usekdeglobals)
{
  if (!s_list)
  {
    sd.setObject(s_list, new TQValueList<TDESharedConfig*>);
  }

  s_list->append(this);
}

TDESharedConfig::~TDESharedConfig()
{
  if ( s_list )
    s_list->remove(this);
}

#include "tdeconfig.moc"
