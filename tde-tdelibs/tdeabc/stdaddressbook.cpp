/*
    This file is part of libtdeabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <stdlib.h>

#include <tdeapplication.h>
#include <kcrash.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tderesources/manager.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>

#include "resource.h"

#include "stdaddressbook.h"

using namespace TDEABC;

StdAddressBook *StdAddressBook::mSelf = 0;
bool StdAddressBook::mAutomaticSave = true;

static KStaticDeleter<StdAddressBook> addressBookDeleter;

TQString StdAddressBook::fileName()
{
  return locateLocal( "data", "tdeabc/std.vcf" );
}

TQString StdAddressBook::directoryName()
{
  return locateLocal( "data", "tdeabc/stdvcf" );
}

void StdAddressBook::handleCrash()
{
}

StdAddressBook *StdAddressBook::self()
{
  if ( !mSelf )
    addressBookDeleter.setObject( mSelf, new StdAddressBook );

  return mSelf;
}

StdAddressBook *StdAddressBook::self( bool asynchronous )
{
  if ( !mSelf )
    addressBookDeleter.setObject( mSelf, new StdAddressBook( asynchronous ) );

  return mSelf;
}

StdAddressBook::StdAddressBook()
  : AddressBook( "" )
{
  kdDebug(5700) << "StdAddressBook::StdAddressBook()" << endl;

  init( false );
}

StdAddressBook::StdAddressBook( bool asynchronous )
  : AddressBook( "" )
{
  kdDebug(5700) << "StdAddressBook::StdAddressBook( bool )" << endl;

  init( asynchronous );
}

StdAddressBook::~StdAddressBook()
{
  if ( mAutomaticSave )
    saveAll();
}

void StdAddressBook::init( bool asynchronous )
{
  KRES::Manager<Resource> *manager = resourceManager();

  KRES::Manager<Resource>::ActiveIterator it;
  for ( it = manager->activeBegin(); it != manager->activeEnd(); ++it ) {
    (*it)->setAddressBook( this );
    if ( !(*it)->open() ) {
      error( TQString( "Unable to open resource '%1'!" ).arg( (*it)->resourceName() ) );
      continue;
    }
    connect( *it, TQ_SIGNAL( loadingFinished( Resource* ) ),
             this, TQ_SLOT( resourceLoadingFinished( Resource* ) ) );
    connect( *it, TQ_SIGNAL( savingFinished( Resource* ) ),
             this, TQ_SLOT( resourceSavingFinished( Resource* ) ) );

    connect( *it, TQ_SIGNAL( loadingError( Resource*, const TQString& ) ),
             this, TQ_SLOT( resourceLoadingError( Resource*, const TQString& ) ) );
    connect( *it, TQ_SIGNAL( savingError( Resource*, const TQString& ) ),
             this, TQ_SLOT( resourceSavingError( Resource*, const TQString& ) ) );
  }

  Resource *res = standardResource();
  if ( !res ) {
    res = manager->createResource( "file" );
    if ( res )
      addResource( res );
    else
      kdDebug(5700) << "No resource available!!!" << endl;
  }

  setStandardResource( res );
  manager->writeConfig();

  if ( asynchronous )
    asyncLoad();
  else
    load();
}

bool StdAddressBook::saveAll()
{
  kdDebug(5700) << "StdAddressBook::saveAll()" << endl;
  bool ok = true;

  deleteRemovedAddressees();

  KRES::Manager<Resource>::ActiveIterator it;
  KRES::Manager<Resource> *manager = resourceManager();
  for ( it = manager->activeBegin(); it != manager->activeEnd(); ++it ) {
    if ( !(*it)->readOnly() && (*it)->isOpen() ) {
      Ticket *ticket = requestSaveTicket( *it );
      if ( !ticket ) {
        error( i18n( "Unable to save to resource '%1'. It is locked." )
                   .arg( (*it)->resourceName() ) );
        return false;
      }

      if ( !AddressBook::save( ticket ) ) {
        ok = false;
        releaseSaveTicket( ticket );
      }
    }
  }

  return ok;
}

bool StdAddressBook::save()
{
  kdDebug(5700) << "StdAddressBook::save()" << endl;

  if ( mSelf ) 
    return mSelf->saveAll();
  else
    return true;  
}

void StdAddressBook::close()
{
  addressBookDeleter.destructObject();
}

void StdAddressBook::setAutomaticSave( bool enable )
{
  mAutomaticSave = enable;
}

bool StdAddressBook::automaticSave()
{
  return mAutomaticSave;
}

// should get const for 4.X
Addressee StdAddressBook::whoAmI()
{
  TDEConfig config( "tdeabcrc" );
  config.setGroup( "General" );

  return findByUid( config.readEntry( "WhoAmI" ) );
}

void StdAddressBook::setWhoAmI( const Addressee &addr )
{
  TDEConfig config( "tdeabcrc" );
  config.setGroup( "General" );

  config.writeEntry( "WhoAmI", addr.uid() );
}
