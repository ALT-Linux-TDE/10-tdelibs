/* This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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
#include <tqdir.h>

#include "tdeglobal.h"
#include "kstandarddirs.h"
#include "tdeconfigbackend.h"

#include "ksimpleconfig.h"

KSimpleConfig::KSimpleConfig(const TQString &fileName, bool bReadOnly)
  : TDEConfig(TQString::fromLatin1(""), bReadOnly, false)
{
  // the difference between TDEConfig and KSimpleConfig is just that
  // for KSimpleConfig an absolute filename is guaranteed
  if (!fileName.isNull() && TQDir::isRelativePath(fileName)) {
     backEnd->changeFileName( TDEGlobal::dirs()->
	saveLocation("config", TQString::null, !bReadOnly)+fileName, "config", false);
  } else {
     backEnd->changeFileName(fileName, "config", false);
  }
  setReadOnly( bReadOnly );
  reparseConfiguration();
}

KSimpleConfig::KSimpleConfig(TDEConfigBackEnd *backEnd, bool bReadOnly)
  : TDEConfig(backEnd, bReadOnly)
{}

KSimpleConfig::~KSimpleConfig()
{
  // we need to call the KSimpleConfig version of sync.  Relying on the
  // regular TDEConfig sync is bad, because the KSimpleConfig sync has
  // different behavior.  Syncing here will insure that the sync() call
  // in the TDEConfig destructor doesn't actually do anything.
  sync();
}

void KSimpleConfig::sync()
{
   if (isReadOnly())
       return;
   backEnd->sync(false);

   if (isDirty())
     rollback();
}

void KSimpleConfig::virtual_hook( int id, void* data )
{ TDEConfig::virtual_hook( id, data ); }

#include "ksimpleconfig.moc"
