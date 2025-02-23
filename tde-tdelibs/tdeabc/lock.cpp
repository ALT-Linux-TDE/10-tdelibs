/*
    This file is part of libtdeabc.
    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "lock.h"

#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <kstandarddirs.h>
#include <tdetempfile.h>

#include <tqfile.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace TDEABC;

Lock::Lock( const TQString &identifier )
  : mIdentifier( identifier )
{
  mIdentifier.replace( "/", "_" );
}

Lock::~Lock()
{
  unlock();
}

TQString Lock::locksDir()
{
  return locateLocal( "data", "tdeabc/lock/" );
}

bool Lock::readLockFile( const TQString &filename, int &pid, TQString &app )
{
  TQFile file( filename );
  if ( !file.open( IO_ReadOnly ) ) return false;

  TQTextStream t( &file );
  pid = t.readLine().toInt();
  app = t.readLine();

  return true;
}

bool Lock::writeLockFile( const TQString &filename )
{
  TQFile file( filename );
  if ( !file.open( IO_WriteOnly ) ) return false;
  TQTextStream t( &file );
  t << ::getpid() << endl << TQString( TDEGlobal::instance()->instanceName() );

  return true;
}

TQString Lock::lockFileName() const
{
  return locksDir() + mIdentifier + ".lock";
}

bool Lock::lock()
{
  kdDebug(5700) << "Lock::lock()" << endl;

  TQString lockName = lockFileName();
  kdDebug(5700) << "-- lock name: " << lockName << endl;

  if ( TQFile::exists( lockName ) ) {  // check if it is a stale lock file
    int pid;
    TQString app;

    if ( !readLockFile( lockFileName(), pid, app ) ) {
      mError = i18n("Unable to open lock file.");
      return false;
    }

    int retval = ::kill( pid, 0 );
    if ( retval == -1 && errno == ESRCH ) { // process doesn't exists anymore
      TQFile::remove( lockName );
      kdWarning(5700) << "Removed stale lock file from process '" << app << "'"
                      << endl;
    } else {
      TQString identifier( mIdentifier );
      identifier.replace( '_', '/' );

      mError = i18n("The address book '%1' is locked by application '%2'.\nIf you believe this is incorrect, just remove the lock file from '%3'")
               .arg( identifier ).arg( app ).arg( locateLocal( "data", "tdeabc/lock/*.lock" ) );
      return false;
    }
  }

  TQString lockUniqueName;
  lockUniqueName = mIdentifier + kapp->randomString( 8 );
  mLockUniqueName = locateLocal( "data", "tdeabc/lock/" + lockUniqueName );
  kdDebug(5700) << "-- lock unique name: " << mLockUniqueName << endl;

  // Create unique file
  writeLockFile( mLockUniqueName );

  // Create lock file
  int result = ::link( TQFile::encodeName( mLockUniqueName ),
                       TQFile::encodeName( lockName ) );

  if ( result == 0 ) {
    mError = "";
    emit locked();
    return true;
  }

  // TODO: check stat

  mError = i18n("Error");
  return false;
}

bool Lock::unlock()
{
  int pid;
  TQString app;
  if ( readLockFile( lockFileName(), pid, app ) ) {
    if ( pid == getpid() ) {
      TQFile::remove( lockFileName() );
      TQFile::remove( mLockUniqueName );
      emit unlocked();
    } else {
      mError = i18n("Unlock failed. Lock file is owned by other process: %1 (%2)")
               .arg( app ).arg( TQString::number( pid ) );
      kdDebug() << "Lock::unlock(): " << mError << endl;
      return false;
    }
  }

  mError = "";
  return true;
}

TQString Lock::error() const
{
  return mError;
}

#include "lock.moc"
