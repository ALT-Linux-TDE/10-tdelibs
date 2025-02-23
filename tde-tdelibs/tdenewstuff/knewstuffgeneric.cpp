/*
    This file is part of KDE.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <tqfile.h>
#include <tqtextstream.h>
#include <tqdir.h>

#include <kdebug.h>
#include <tdelocale.h>
#include <kprocess.h>
#include <tdeconfig.h>
#include <kstandarddirs.h>
#include <tdemessagebox.h>
#include <kmimemagic.h>
#include <ktar.h>

#include "entry.h"

#include "knewstuffgeneric.h"

using namespace std;

TDENewStuffGeneric::TDENewStuffGeneric( const TQString &type, TQWidget *parent )
  : TDENewStuff( type, parent )
{
  mConfig = TDEGlobal::config();
}

TDENewStuffGeneric::~TDENewStuffGeneric()
{
}

bool TDENewStuffGeneric::install( const TQString &fileName )
{
  // Try to detect the most common cases where (usually adware) Web pages are downloaded
  // instead of the desired file and abort
  KMimeMagicResult *res = KMimeMagic::self()->findFileType( fileName );
  if ( res->isValid() && res->accuracy() > 40 ) {
    if (type().lower().contains("wallpaper")) {
      if (!res->mimeType().startsWith("image/")) {
        TQFile::remove(fileName);
        return false;
      }
    }
}

  kdDebug() << "TDENewStuffGeneric::install(): " << fileName << endl;
  TQStringList list, list2;

  mConfig->setGroup("TDENewStuff");

  TQString uncompress = mConfig->readEntry( "Uncompress" );
  if ( !uncompress.isEmpty() ) {
    kdDebug() << "Uncompression method: " << uncompress << endl;
    KTar tar(fileName, uncompress);
    tar.open(IO_ReadOnly);
    const KArchiveDirectory *dir = tar.directory();
    dir->copyTo(destinationPath(0));
    tar.close();
    TQFile::remove(fileName);
  }

  TQString cmd = mConfig->readEntry( "InstallationCommand" );
  if ( !cmd.isEmpty() ) {
    kdDebug() << "InstallationCommand: " << cmd << endl;
    list = TQStringList::split( " ", cmd );
    for ( TQStringList::iterator it = list.begin(); it != list.end(); ++it ) {
        list2 << (*it).replace("%f", fileName);
    }
    TDEProcess proc;
    proc << list2;
    proc.start( TDEProcess::Block );
  }

  return true;
}

bool TDENewStuffGeneric::createUploadFile( const TQString & /*fileName*/ )
{
  return false;
}

TQString TDENewStuffGeneric::destinationPath( KNS::Entry *entry )
{
  TQString path, file, target, ext;

  mConfig->setGroup("TDENewStuff");

  if ( entry )
  {
    ext = entry->payload().fileName().section('.', 1);
    if ( ! ext.isEmpty() ) ext = "." + ext;

    target = entry->fullName() + ext;
  }
  else target = "/";
  TQString res = mConfig->readEntry( "StandardResource" );
  if ( res.isEmpty() )
  {
    target = mConfig->readEntry("TargetDir");
    if ( !target.isEmpty())
    {
      res = "data";
      if ( entry ) target.append("/" + entry->fullName() + ext);
      else target.append("/");
    }
  }
  if ( res.isEmpty() )
  {
    path = mConfig->readEntry( "InstallPath" );
  }
  if ( res.isEmpty() && path.isEmpty() )
  {
    if ( !entry ) return TQString::null;
    else return TDENewStuff::downloadDestination( entry );
  }

  if ( !path.isEmpty() )
  {
    file = TQDir::home().path() + "/" + path + "/";
    if ( entry ) file += entry->fullName() + ext;
  }
  else file = locateLocal( res.utf8() , target );

  return file;
}

TQString TDENewStuffGeneric::downloadDestination( KNS::Entry *entry )
{
  TQString file = destinationPath(entry);

  if ( TDEStandardDirs::exists( file ) ) {
    int result = KMessageBox::warningContinueCancel( parentWidget(),
        i18n("The file '%1' already exists. Do you want to overwrite it?")
        .arg( file ),
        TQString::null, i18n("Overwrite") );
    if ( result == KMessageBox::Cancel ) return TQString::null;
  }

  return file;
}
