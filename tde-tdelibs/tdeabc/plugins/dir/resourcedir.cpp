/*
    This file is part of libtdeabc.
    Copyright (c) 2002 - 2003 Tobias Koenig <tokoe@kde.org>

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

#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <tqregexp.h>
#include <tqtimer.h>
#include <tqwidget.h>

#include <tdeapplication.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include "addressbook.h"
#include "formatfactory.h"
#include "resourcedirconfig.h"
#include "stdaddressbook.h"
#include "lock.h"

#include "resourcedir.h"

using namespace TDEABC;

extern "C"
{
  void *init_tdeabc_dir()
  {
    return new KRES::PluginFactory<ResourceDir,ResourceDirConfig>();
  }
}


ResourceDir::ResourceDir( const TDEConfig *config )
  : Resource( config ), mAsynchronous( false )
{
  if ( config ) {
    init( config->readPathEntry( "FilePath", StdAddressBook::directoryName() ),
          config->readEntry( "FileFormat", "vcard" ) );
  } else {
    init( StdAddressBook::directoryName(), "vcard" );
  }
}

ResourceDir::ResourceDir( const TQString &path, const TQString &format )
  : Resource( 0 ), mAsynchronous( false )
{
  init( path, format );
}

void ResourceDir::init( const TQString &path, const TQString &format )
{
  mFormatName = format;

  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( mFormatName );

  if ( !mFormat ) {
    mFormatName = "vcard";
    mFormat = factory->format( mFormatName );
  }

  mLock = 0;

  connect( &mDirWatch, TQ_SIGNAL( dirty(const TQString&) ), TQ_SLOT( pathChanged() ) );
  connect( &mDirWatch, TQ_SIGNAL( created(const TQString&) ), TQ_SLOT( pathChanged() ) );
  connect( &mDirWatch, TQ_SIGNAL( deleted(const TQString&) ), TQ_SLOT( pathChanged() ) );

  setPath( path );
}

ResourceDir::~ResourceDir()
{
  delete mFormat;
  mFormat = 0;
}

void ResourceDir::writeConfig( TDEConfig *config )
{
  Resource::writeConfig( config );

  if ( mPath == StdAddressBook::directoryName() )
    config->deleteEntry( "FilePath" );
  else
    config->writePathEntry( "FilePath", mPath );

  config->writeEntry( "FileFormat", mFormatName );
}

Ticket *ResourceDir::requestSaveTicket()
{
  kdDebug(5700) << "ResourceDir::requestSaveTicket()" << endl;

  if ( !addressBook() ) return 0;

  delete mLock;
  mLock = new Lock( mPath );

  if ( mLock->lock() ) {
    addressBook()->emitAddressBookLocked();
  } else {
    addressBook()->error( mLock->error() );
    kdDebug(5700) << "ResourceFile::requestSaveTicket(): Unable to lock path '"
                  << mPath << "': " << mLock->error() << endl;
    return 0;
  }

  return createTicket( this );
}

void ResourceDir::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
  
  delete mLock;
  mLock = 0;
}

bool ResourceDir::doOpen()
{
  TQDir dir( mPath );
  if ( !dir.exists() ) { // no directory available
    return dir.mkdir( dir.path() );
  } else {
    TQString testName = dir.entryList( TQDir::Files )[0];
    if ( testName.isNull() || testName.isEmpty() ) // no file in directory
      return true;

    TQFile file( mPath + "/" + testName );
    if ( file.open( IO_ReadOnly ) )
      return true;

    if ( file.size() == 0 )
      return true;

    bool ok = mFormat->checkFormat( &file );
    file.close();
    return ok;
  }
}

void ResourceDir::doClose()
{
}

bool ResourceDir::load()
{
  kdDebug(5700) << "ResourceDir::load(): '" << mPath << "'" << endl;

  mAsynchronous = false;

  TQDir dir( mPath );
  TQStringList files = dir.entryList( TQDir::Files );

  TQStringList::Iterator it;
  bool ok = true;
  for ( it = files.begin(); it != files.end(); ++it ) {
    TQFile file( mPath + "/" + (*it) );

    if ( !file.open( IO_ReadOnly ) ) {
      addressBook()->error( i18n( "Unable to open file '%1' for reading" ).arg( file.name() ) );
      ok = false;
      continue;
    }

    if ( !mFormat->loadAll( addressBook(), this, &file ) )
      ok = false;

    file.close();
  }

  return ok;
}

bool ResourceDir::asyncLoad()
{
  mAsynchronous = true;

  bool ok = load();
  if ( !ok )
    emit loadingError( this, i18n( "Loading resource '%1' failed!" )
                       .arg( resourceName() ) );
  else
    emit loadingFinished( this );

  return ok;
}

bool ResourceDir::save( Ticket * )
{
  kdDebug(5700) << "ResourceDir::save(): '" << mPath << "'" << endl;

  Addressee::Map::Iterator it;
  bool ok = true;

  mDirWatch.stopScan();

  for ( it = mAddrMap.begin(); it != mAddrMap.end(); ++it ) {
    if ( !it.data().changed() )
      continue;

    TQFile file( mPath + "/" + (*it).uid() );
    if ( !file.open( IO_WriteOnly ) ) {
      addressBook()->error( i18n( "Unable to open file '%1' for writing" ).arg( file.name() ) );
      continue;
    }

    mFormat->save( *it, &file );

    // mark as unchanged
    (*it).setChanged( false );

    file.close();
  }

  mDirWatch.startScan();

  return ok;
}

bool ResourceDir::asyncSave( Ticket *ticket )
{
  bool ok = save( ticket );
  if ( !ok )
    emit savingError( this, i18n( "Saving resource '%1' failed!" )
                      .arg( resourceName() ) );
  else
    emit savingFinished( this );

  return ok;
}

void ResourceDir::setPath( const TQString &path )
{
  mDirWatch.stopScan();
  if ( mDirWatch.contains( mPath ) )
    mDirWatch.removeDir( mPath );

  mPath = path;
  mDirWatch.addDir( mPath, true );
  mDirWatch.startScan();
}

TQString ResourceDir::path() const
{
  return mPath;
}

void ResourceDir::setFormat( const TQString &format )
{
  mFormatName = format;

  if ( mFormat )
    delete mFormat;

  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( mFormatName );
}

TQString ResourceDir::format() const
{
  return mFormatName;
}

void ResourceDir::pathChanged()
{
  if ( !addressBook() )
    return;

  clear();
  if ( mAsynchronous )
    asyncLoad();
  else {
    load();
    addressBook()->emitAddressBookChanged();
  }
}

void ResourceDir::removeAddressee( const Addressee& addr )
{
  TQFile::remove( mPath + "/" + addr.uid() );
  mAddrMap.erase( addr.uid() );
}

#include "resourcedir.moc"
