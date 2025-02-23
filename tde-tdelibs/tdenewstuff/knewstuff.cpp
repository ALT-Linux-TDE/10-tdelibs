/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <tdeaction.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <kstandarddirs.h>

#include "engine.h"

#include "knewstuff.h"

using namespace KNS;

TDEAction* KNS::standardAction(const TQString& what,
                             const TQObject *recvr,
                             const char *slot, TDEActionCollection* parent,
                             const char *name)
{
    return new TDEAction(i18n("Download New %1").arg(what), "knewstuff",
                       0, recvr, slot, parent, name);
}

TDENewStuff::TDENewStuff( const TQString &type, TQWidget *parentWidget )
{
    mEngine = new Engine( this, type, parentWidget );
}

TDENewStuff::TDENewStuff( const TQString &type, const TQString &providerList, TQWidget *parentWidget )
{
  mEngine = new Engine( this, type, providerList, parentWidget );
}

TQString TDENewStuff::type() const
{
  return mEngine->type();
}

TQWidget *TDENewStuff::parentWidget() const
{
  return mEngine->parentWidget();
}

TDENewStuff::~TDENewStuff()
{
  delete mEngine;
}

void TDENewStuff::download()
{
  mEngine->download();
}

TQString TDENewStuff::downloadDestination( Entry *entry )
{
  // Respect downloaded file's extension
  TQString ext = entry->payload().fileName().section('.', 1);
  if ( ! ext.isEmpty() ) ext = "." + ext;

  return TDEGlobal::dirs()->saveLocation( "tmp" ) +
         TDEApplication::randomString( 10 ) + ext;
}

void TDENewStuff::upload()
{
  mEngine->upload();
}

void TDENewStuff::upload( const TQString &fileName, const TQString previewName )
{
  mEngine->upload(fileName, previewName);
}
