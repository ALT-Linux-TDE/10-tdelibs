/*
    This file is part of libtderesources.
    
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
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

#include <dcopclient.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdeconfig.h>
#include <kstandarddirs.h>

#include "resource.h"
#include "factory.h"
#include "manager.h"
#include "managerimpl.h"
#include "manageriface_stub.h"

using namespace KRES;

ManagerImpl::ManagerImpl( ManagerNotifier *notifier, const TQString &family )
  : DCOPObject( "ManagerIface_" + family.utf8() ),
    mNotifier( notifier ),
    mFamily( family ), mConfig( 0 ), mStdConfig( 0 ), mStandard( 0 ),
    mFactory( 0 ), mConfigRead( false )
{
  kdDebug(5650) << "ManagerImpl::ManagerImpl()" << endl;

  mId = TDEApplication::randomString( 8 );

  // Register with DCOP
  if ( !kapp->dcopClient()->isRegistered() ) {
    kapp->dcopClient()->registerAs( "TDEResourcesManager" );
    kapp->dcopClient()->setDefaultObject( objId() );
  }

  kdDebug(5650) << "Connecting DCOP signals..." << endl;
  if ( !connectDCOPSignal( 0, "ManagerIface_" + family.utf8(),
                           "signalKResourceAdded( TQString, TQString )",
                           "dcopKResourceAdded( TQString, TQString )", false ) )
    kdWarning(5650) << "Could not connect ResourceAdded signal!" << endl;

  if ( !connectDCOPSignal( 0, "ManagerIface_" + family.utf8(),
                           "signalKResourceModified( TQString, TQString )",
                           "dcopKResourceModified( TQString, TQString )", false ) )
    kdWarning(5650) << "Could not connect ResourceModified signal!" << endl;

  if ( !connectDCOPSignal( 0, "ManagerIface_" + family.utf8(),
                           "signalKResourceDeleted( TQString, TQString )",
                           "dcopKResourceDeleted( TQString, TQString )", false ) )
    kdWarning(5650) << "Could not connect ResourceDeleted signal!" << endl;

  kapp->dcopClient()->setNotifications( true );
}

ManagerImpl::~ManagerImpl()
{
  kdDebug(5650) << "ManagerImpl::~ManagerImpl()" << endl;

  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    delete *it;
  }
 
  delete mStdConfig;
}

void ManagerImpl::createStandardConfig()
{
  if ( !mStdConfig ) {
    TQString file = defaultConfigFile( mFamily );
    mStdConfig = new TDEConfig( file );
  }
  
  mConfig = mStdConfig;
}

void ManagerImpl::readConfig( TDEConfig *cfg )
{
  kdDebug(5650) << "ManagerImpl::readConfig()" << endl;

  delete mFactory;
  mFactory = Factory::self( mFamily );

  if ( !cfg ) {
    createStandardConfig();
  } else {
    mConfig = cfg;
  }

  mStandard = 0;

  mConfig->setGroup( "General" );

  TQStringList keys = mConfig->readListEntry( "ResourceKeys" );
  keys += mConfig->readListEntry( "PassiveResourceKeys" );

  TQString standardKey = mConfig->readEntry( "Standard" );

  for ( TQStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
    readResourceConfig( *it, false );
  }

  mConfigRead = true;
}

void ManagerImpl::writeConfig( TDEConfig *cfg )
{
  kdDebug(5650) << "ManagerImpl::writeConfig()" << endl;

  if ( !cfg ) {
    createStandardConfig();
  } else {
    mConfig = cfg;
  }

  TQStringList activeKeys;
  TQStringList passiveKeys;

  // First write all keys, collect active and passive keys on the way
  Resource::List::Iterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    writeResourceConfig( *it, false );

    TQString key = (*it)->identifier();
    if( (*it)->isActive() )
      activeKeys.append( key );
    else
      passiveKeys.append( key );
  }

  // And then the general group

  kdDebug(5650) << "Saving general info" << endl;
  mConfig->setGroup( "General" );
  mConfig->writeEntry( "ResourceKeys", activeKeys );
  mConfig->writeEntry( "PassiveResourceKeys", passiveKeys );
  if ( mStandard ) 
    mConfig->writeEntry( "Standard", mStandard->identifier() );
  else
    mConfig->writeEntry( "Standard", "" );

  mConfig->sync();
  kdDebug(5650) << "ManagerImpl::save() finished" << endl;
}

void ManagerImpl::add( Resource *resource )
{
  resource->setActive( true );

  if ( mResources.isEmpty() ) {
    mStandard = resource;
  }

  mResources.append( resource );

  if ( mConfigRead )
    writeResourceConfig( resource, true );

  signalKResourceAdded( mId, resource->identifier() );
}

void ManagerImpl::remove( Resource *resource )
{
  if ( mStandard == resource ) mStandard = 0;
  removeResource( resource );

  mResources.remove( resource );

  signalKResourceDeleted( mId, resource->identifier() );

  delete resource;

  kdDebug(5650) << "Finished ManagerImpl::remove()" << endl;
}

void ManagerImpl::change( Resource *resource )
{
  writeResourceConfig( resource, true );

  signalKResourceModified( mId, resource->identifier() );
}

void ManagerImpl::setActive( Resource *resource, bool active )
{
  if ( resource && resource->isActive() != active ) {
    resource->setActive( active );
  }
}

Resource *ManagerImpl::standardResource() 
{
  return mStandard;
}

void ManagerImpl::setStandardResource( Resource *resource ) 
{
  mStandard = resource;
}

// DCOP asynchronous functions

void ManagerImpl::dcopKResourceAdded( TQString managerId, TQString resourceId )
{
  if ( managerId == mId ) {
    kdDebug(5650) << "Ignore DCOP notification to myself" << endl;
    return;
  }
  kdDebug(5650) << "Receive DCOP call: added resource " << resourceId << endl;

  if ( getResource( resourceId ) ) {
    kdDebug(5650) << "This resource is already known to me." << endl;
  }

  if ( !mConfig ) createStandardConfig();

  mConfig->reparseConfiguration();
  Resource *resource = readResourceConfig( resourceId, true );

  if ( resource ) {
    mNotifier->notifyResourceAdded( resource );
  } else 
    kdError() << "Received DCOP: resource added for unknown resource "
              << resourceId << endl;
}

void ManagerImpl::dcopKResourceModified( TQString managerId, TQString resourceId )
{
  if ( managerId == mId ) {
    kdDebug(5650) << "Ignore DCOP notification to myself" << endl;
    return;
  }
  kdDebug(5650) << "Receive DCOP call: modified resource " << resourceId << endl;

  Resource *resource = getResource( resourceId );
  if ( resource ) {
    mNotifier->notifyResourceModified( resource );
  } else 
    kdError() << "Received DCOP: resource modified for unknown resource "
              << resourceId << endl;
}

void ManagerImpl::dcopKResourceDeleted( TQString managerId, TQString resourceId )
{
  if ( managerId == mId ) {
    kdDebug(5650) << "Ignore DCOP notification to myself" << endl;
    return;
  }
  kdDebug(5650) << "Receive DCOP call: deleted resource " << resourceId << endl;

  Resource *resource = getResource( resourceId );
  if ( resource ) {
    mNotifier->notifyResourceDeleted( resource );

    kdDebug(5650) << "Removing item from mResources" << endl;
    // Now delete item
    if ( mStandard == resource )
      mStandard = 0;
    mResources.remove( resource );
  } else
    kdError() << "Received DCOP: resource deleted for unknown resource "
              << resourceId << endl;
}

TQStringList ManagerImpl::resourceNames()
{
  TQStringList result;

  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    result.append( (*it)->resourceName() );
  }
  return result;
}

Resource::List *ManagerImpl::resourceList()
{
  return &mResources;
}

TQPtrList<Resource> ManagerImpl::resources()
{
  TQPtrList<Resource> result;

  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    result.append( *it );
  }
  return result;
}

TQPtrList<Resource> ManagerImpl::resources( bool active )
{
  TQPtrList<Resource> result;

  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    if ( (*it)->isActive() == active ) {
      result.append( *it );
    }
  }
  return result;
}

Resource *ManagerImpl::readResourceConfig( const TQString &identifier,
                                           bool checkActive )
{
  kdDebug(5650) << "ManagerImpl::readResourceConfig() " << identifier << endl;

  if ( !mFactory ) {
    kdError(5650) << "ManagerImpl::readResourceConfig: mFactory is 0. Did the app forget to call readConfig?" << endl;
    return 0;
  }

  mConfig->setGroup( "Resource_" + identifier );

  TQString type = mConfig->readEntry( "ResourceType" );
  TQString name = mConfig->readEntry( "ResourceName" );
  Resource *resource = mFactory->resource( type, mConfig );
  if ( !resource ) {
    kdDebug(5650) << "Failed to create resource with id " << identifier << endl;
    return 0;
  }

  if ( resource->identifier().isEmpty() )
    resource->setIdentifier( identifier );

  mConfig->setGroup( "General" );

  TQString standardKey = mConfig->readEntry( "Standard" );
  if ( standardKey == identifier ) {
    mStandard = resource;
  }

  if ( checkActive ) {
    TQStringList activeKeys = mConfig->readListEntry( "ResourceKeys" );
    resource->setActive( activeKeys.contains( identifier ) );
  }
  mResources.append( resource );

  return resource;
}

void ManagerImpl::writeResourceConfig( Resource *resource, bool checkActive )
{
  TQString key = resource->identifier();

  kdDebug(5650) << "Saving resource " << key << endl;

  if ( !mConfig ) createStandardConfig();

  mConfig->setGroup( "Resource_" + key );
  resource->writeConfig( mConfig );

  mConfig->setGroup( "General" );
  TQString standardKey = mConfig->readEntry( "Standard" );

  if ( resource == mStandard  && standardKey != key )
    mConfig->writeEntry( "Standard", resource->identifier() );
  else if ( resource != mStandard && standardKey == key )
    mConfig->writeEntry( "Standard", "" );
  
  if ( checkActive ) {
    TQStringList activeKeys = mConfig->readListEntry( "ResourceKeys" );
    TQStringList passiveKeys = mConfig->readListEntry( "PassiveResourceKeys" );
    if ( resource->isActive() ) {
      if ( passiveKeys.contains( key ) ) { // remove it from passive list
        passiveKeys.remove( key );
        mConfig->writeEntry( "PassiveResourceKeys", passiveKeys );
      }
      if ( !activeKeys.contains( key ) ) { // add it to active list
        activeKeys.append( key );
        mConfig->writeEntry( "ResourceKeys", activeKeys );
      }
    } else if ( !resource->isActive() ) {
      if ( activeKeys.contains( key ) ) { // remove it from active list
        activeKeys.remove( key );
        mConfig->writeEntry( "ResourceKeys", activeKeys );
      }
      if ( !passiveKeys.contains( key ) ) { // add it to passive list
        passiveKeys.append( key );
        mConfig->writeEntry( "PassiveResourceKeys", passiveKeys );
      }
    }
  }

  mConfig->sync();
}

void ManagerImpl::removeResource( Resource *resource )
{
  TQString key = resource->identifier();

  if ( !mConfig ) createStandardConfig();
  
  mConfig->setGroup( "General" );
  TQStringList activeKeys = mConfig->readListEntry( "ResourceKeys" );
  if ( activeKeys.contains( key ) ) {
    activeKeys.remove( key );
    mConfig->writeEntry( "ResourceKeys", activeKeys );
  } else {
    TQStringList passiveKeys = mConfig->readListEntry( "PassiveResourceKeys" );
    passiveKeys.remove( key );
    mConfig->writeEntry( "PassiveResourceKeys", passiveKeys );
  }

  TQString standardKey = mConfig->readEntry( "Standard" );
  if ( standardKey == key ) {
    mConfig->writeEntry( "Standard", "" );
  }

  mConfig->deleteGroup( "Resource_" + resource->identifier() );
  mConfig->sync();
}

Resource *ManagerImpl::getResource( const TQString &identifier )
{
  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    if ( (*it)->identifier() == identifier )
      return *it;
  }
  return 0;
}

TQString ManagerImpl::defaultConfigFile( const TQString &family )
{
  return TQString( "tderesources/%1/stdrc" ).arg( family );
}
