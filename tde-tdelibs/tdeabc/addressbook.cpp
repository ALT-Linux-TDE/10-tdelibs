/*
    This file is part of libtdeabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General  Public
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
#include <tqregexp.h>
#include <tqtimer.h>

#include <tdeapplication.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <kinstance.h>
#include <tdelocale.h>
#include <kstandarddirs.h>

#include "errorhandler.h"
#include "resource.h"

#include "addressbook.h"
#include "addressbook.moc"

using namespace TDEABC;

struct AddressBook::AddressBookData
{
  Field::List mAllFields;
  ErrorHandler *mErrorHandler;
  TDEConfig *mConfig;
  KRES::Manager<Resource> *mManager;
  TQPtrList<Resource> mPendingLoadResources;
  TQPtrList<Resource> mPendingSaveResources;
  Iterator end;
};

struct AddressBook::Iterator::IteratorData
{
  Resource::Iterator mIt;
  TQValueList<Resource*> mResources;
  int mCurrRes;
};

struct AddressBook::ConstIterator::ConstIteratorData
{
  Resource::ConstIterator mIt;
  TQValueList<Resource*> mResources;
  int mCurrRes;
};

AddressBook::Iterator::Iterator()
  : d( new IteratorData )
{
}

AddressBook::Iterator::Iterator( const AddressBook::Iterator &i )
  : d( new IteratorData )
{
  d->mIt = i.d->mIt;
  d->mResources = i.d->mResources;
  d->mCurrRes = i.d->mCurrRes;
}

AddressBook::Iterator &AddressBook::Iterator::operator=( const AddressBook::Iterator &i )
{
  if ( this == &i )
    return *this; // guard against self assignment

  delete d; // delete the old data the Iterator was completely constructed before
  d = new IteratorData;
  d->mIt = i.d->mIt;
  d->mResources = i.d->mResources;
  d->mCurrRes = i.d->mCurrRes;

  return *this;
}

AddressBook::Iterator::~Iterator()
{
  delete d;
  d = 0;
}

const Addressee &AddressBook::Iterator::operator*() const
{
  return *(d->mIt);
}

Addressee &AddressBook::Iterator::operator*()
{
  return *(d->mIt);
}

Addressee *AddressBook::Iterator::operator->()
{
  return &(*(d->mIt));
}

AddressBook::Iterator &AddressBook::Iterator::operator++()
{
  do {
    bool jumped = false;
    while ( d->mIt == (d->mResources[ d->mCurrRes ])->end() ) { // at end of addressee list of resource
      if ( (uint)d->mCurrRes == d->mResources.count() - 1 ) {
        return *this;
      }

      d->mCurrRes++; // jump to next resource

      jumped = true;
      d->mIt = (d->mResources[ d->mCurrRes ])->begin();
    }

    if ( !jumped )
      (d->mIt)++;

  } while ( d->mIt == (d->mResources[ d->mCurrRes ])->end() );

  return *this;
}

AddressBook::Iterator &AddressBook::Iterator::operator++( int )
{
  do {
    bool jumped = false;
    while ( d->mIt == (d->mResources[ d->mCurrRes ])->end() ) { // at end of addressee list of resource
      if ( (uint)d->mCurrRes == d->mResources.count() - 1 ) {
        return *this;
      }

      d->mCurrRes++; // jump to next resource

      jumped = true;
        d->mIt = (d->mResources[ d->mCurrRes ])->begin();
    }

    if ( !jumped )
      (d->mIt)++;

  } while ( d->mIt == (d->mResources[ d->mCurrRes ])->end() );

  return *this;
}

AddressBook::Iterator &AddressBook::Iterator::operator--()
{
  (d->mIt)--;

  return *this;
}

AddressBook::Iterator &AddressBook::Iterator::operator--( int )
{
  (d->mIt)--;

  return *this;
}

bool AddressBook::Iterator::operator==( const Iterator &it )
{
  return ( d->mIt == it.d->mIt );
}

bool AddressBook::Iterator::operator!=( const Iterator &it )
{
  return ( d->mIt != it.d->mIt );
}


AddressBook::ConstIterator::ConstIterator()
  : d( new ConstIteratorData )
{
}

AddressBook::ConstIterator::ConstIterator( const AddressBook::ConstIterator &i )
  : d( new ConstIteratorData )
{
  d->mIt = i.d->mIt;
  d->mResources = i.d->mResources;
  d->mCurrRes = i.d->mCurrRes;
}

AddressBook::ConstIterator::ConstIterator( const AddressBook::Iterator &i )
{
  d = new ConstIteratorData;
  d->mIt = i.d->mIt;
  d->mResources = i.d->mResources;
  d->mCurrRes = i.d->mCurrRes;
}

AddressBook::ConstIterator &AddressBook::ConstIterator::operator=( const AddressBook::ConstIterator &i )
{
  if ( this  == &i )
    return *this; // guard for self assignment

  delete d; // delete the old data because the Iterator was really constructed before
  d = new ConstIteratorData;
  d->mIt = i.d->mIt;
  d->mResources = i.d->mResources;
  d->mCurrRes = i.d->mCurrRes;

  return *this;
}

AddressBook::ConstIterator::~ConstIterator()
{
  delete d;
  d = 0;
}

const Addressee &AddressBook::ConstIterator::operator*() const
{
  return *(d->mIt);
}

const Addressee* AddressBook::ConstIterator::operator->() const
{
  return &(*(d->mIt));
}

AddressBook::ConstIterator &AddressBook::ConstIterator::operator++()
{
  do {
    bool jumped = false;
    while ( d->mIt == (d->mResources[ d->mCurrRes ])->end() ) { // at end of addressee list of resource
      if ( (uint)d->mCurrRes == d->mResources.count() - 1 ) {
        return *this;
      }

      d->mCurrRes++; // jump to next resource

      jumped = true;
      d->mIt = (d->mResources[ d->mCurrRes ])->begin();
    }

    if ( !jumped )
      (d->mIt)++;

  } while ( d->mIt == (d->mResources[ d->mCurrRes ])->end() );

  return *this;
}

AddressBook::ConstIterator &AddressBook::ConstIterator::operator++(int)
{
  do {
    bool jumped = false;
    while ( d->mIt == (d->mResources[ d->mCurrRes ])->end() ) { // at end of addressee list of resource
      if ( (uint)d->mCurrRes == d->mResources.count() - 1 ) {
        return *this;
      }

      d->mCurrRes++; // jump to next resource

      jumped = true;
      d->mIt = (d->mResources[ d->mCurrRes ])->begin();
    }

    if ( !jumped )
      (d->mIt)++;

  } while ( d->mIt == (d->mResources[ d->mCurrRes ])->end() );

  return *this;
}

AddressBook::ConstIterator &AddressBook::ConstIterator::operator--()
{
  (d->mIt)--;
  return *this;
}

AddressBook::ConstIterator &AddressBook::ConstIterator::operator--(int)
{
  (d->mIt)--;
  return *this;
}

bool AddressBook::ConstIterator::operator==( const ConstIterator &it )
{
  return ( d->mIt == it.d->mIt );
}

bool AddressBook::ConstIterator::operator!=( const ConstIterator &it )
{
  return ( d->mIt != it.d->mIt );
}


AddressBook::AddressBook()
  : d( new AddressBookData )
{
  d->mErrorHandler = 0;
  d->mConfig = 0;
  d->mManager = new KRES::Manager<Resource>( "contact" );
  d->end.d->mResources = TQValueList<Resource*>();
  d->end.d->mCurrRes = -1;
}

AddressBook::AddressBook( const TQString &config )
  : d( new AddressBookData )
{
  d->mErrorHandler = 0;
  if ( config.isEmpty() )
    d->mConfig = 0;
  else
    d->mConfig = new TDEConfig( config );
  d->mManager = new KRES::Manager<Resource>( "contact" );
  d->mManager->readConfig( d->mConfig );
  d->end.d->mResources = TQValueList<Resource*>();
  d->end.d->mCurrRes = -1;
}

AddressBook::~AddressBook()
{
  delete d->mManager; d->mManager = 0;
  delete d->mConfig; d->mConfig = 0;
  delete d->mErrorHandler; d->mErrorHandler = 0;
  delete d; d = 0;
}

bool AddressBook::load()
{
  kdDebug(5700) << "AddressBook::load()" << endl;

  clear();

  KRES::Manager<Resource>::ActiveIterator it;
  bool ok = true;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    if ( !(*it)->load() ) {
      error( i18n("Unable to load resource '%1'").arg( (*it)->resourceName() ) );
      ok = false;
    }
  }

  return ok;
}

bool AddressBook::asyncLoad()
{
  kdDebug(5700) << "AddressBook::asyncLoad()" << endl;

  clear();

  KRES::Manager<Resource>::ActiveIterator it;
  bool ok = true;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    d->mPendingLoadResources.append( *it );
    if ( !(*it)->asyncLoad() ) {
      error( i18n("Unable to load resource '%1'").arg( (*it)->resourceName() ) );
      ok = false;
    }
  }

  return ok;
}

bool AddressBook::save( Ticket *ticket )
{
  kdDebug(5700) << "AddressBook::save()"<< endl;

  if ( ticket->resource() ) {
    deleteRemovedAddressees();
    bool ok = ticket->resource()->save( ticket );
    if ( ok ) ticket->resource()->releaseSaveTicket( ticket );
    return ok;
  }

  return false;
}

bool AddressBook::asyncSave( Ticket *ticket )
{
  kdDebug(5700) << "AddressBook::asyncSave()"<< endl;

  if ( ticket->resource() ) {
    d->mPendingSaveResources.append( ticket->resource() );
    bool ok = ticket->resource()->asyncSave( ticket );
    if ( ok ) ticket->resource()->releaseSaveTicket( ticket );
    return ok;
  }

  return false;
}

AddressBook::Iterator AddressBook::begin()
{
  TQValueList<Resource*> list;
  KRES::Manager<Resource>::ActiveIterator resIt;
  for ( resIt = d->mManager->activeBegin(); resIt != d->mManager->activeEnd(); ++resIt )
    list.append( *resIt );

  if ( list.count() == 0 )
    return end();

  Iterator it = Iterator();
  it.d->mResources = list;
  it.d->mCurrRes = 0;
  it.d->mIt = (it.d->mResources[ it.d->mCurrRes ])->begin();

  while ( it.d->mIt == (it.d->mResources[ it.d->mCurrRes ])->end() ) {
    if ( (uint)it.d->mCurrRes == it.d->mResources.count() - 1 )
      return end();

    it.d->mCurrRes++;

    it.d->mIt = (it.d->mResources[ it.d->mCurrRes ])->begin();
  }

  return it;
}

AddressBook::ConstIterator AddressBook::begin() const
{
  TQValueList<Resource*> list;
  KRES::Manager<Resource>::ActiveIterator resIt;
  for ( resIt = d->mManager->activeBegin(); resIt != d->mManager->activeEnd(); ++resIt )
    list.append( *resIt );

  if ( list.count() == 0 )
    return end();

  Iterator it = Iterator();
  it.d->mResources = list;
  it.d->mCurrRes = 0;
  it.d->mIt = (it.d->mResources[ it.d->mCurrRes ])->begin();

  while ( it.d->mIt == (it.d->mResources[ it.d->mCurrRes ])->end() ) {
    if ( (uint)it.d->mCurrRes == it.d->mResources.count() - 1 )
      return end();

    it.d->mCurrRes++;

    it.d->mIt = (it.d->mResources[ it.d->mCurrRes ])->begin();
  }

  return it;
}

AddressBook::Iterator AddressBook::end()
{
  KRES::Manager<Resource>::ActiveIterator resIt = d->mManager->activeEnd();

  if ( resIt == d->mManager->activeBegin() || ! *(--resIt) ) { // no resource available
    d->end.d->mIt = Resource::Iterator();
  } else {
    d->end.d->mIt = (*resIt)->end();
  }

  return d->end;
}

AddressBook::ConstIterator AddressBook::end() const
{
  KRES::Manager<Resource>::ActiveIterator resIt = d->mManager->activeEnd();

  if ( resIt == d->mManager->activeBegin() || ! *(--resIt) ) { // no resource available
    d->end.d->mIt = Resource::Iterator();
  } else {
    d->end.d->mIt = (*resIt)->end();
  }

  return d->end;
}

void AddressBook::clear()
{
  KRES::Manager<Resource>::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it )
    (*it)->clear();
}

Ticket *AddressBook::requestSaveTicket( Resource *resource )
{
  kdDebug(5700) << "AddressBook::requestSaveTicket()" << endl;

  if ( !resource )
    resource = standardResource();

  KRES::Manager<Resource>::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    if ( (*it) == resource ) {
      if ( (*it)->readOnly() || !(*it)->isOpen() )
        return 0;
      else
        return (*it)->requestSaveTicket();
    }
  }

  return 0;
}

void AddressBook::releaseSaveTicket( Ticket *ticket )
{
  if ( !ticket )
    return;

  if ( ticket->resource() ) {
    ticket->resource()->releaseSaveTicket( ticket );
  }
}

void AddressBook::insertAddressee( const Addressee &a )
{
  Resource *resource = a.resource();
  if ( resource == 0 )
    resource = standardResource();

  Resource::Iterator it;
  Addressee fAddr = resource->findByUid( a.uid() );

  Addressee addr( a );
  if ( !fAddr.isEmpty() ) {
    if ( fAddr != a )
      addr.setRevision( TQDateTime::currentDateTime() );
    else {
      if ( fAddr.resource() == 0 ) {
        fAddr.setResource( resource );
        //NOTE: Should we have setChanged( true ) here?
        resource->insertAddressee( fAddr );
      }
      return;
    }
  }

  addr.setResource( resource );
  addr.setChanged( true );
  resource->insertAddressee( addr );
}

void AddressBook::removeAddressee( const Addressee &a )
{
  if ( a.resource() )
    a.resource()->removeAddressee( a );
}

void AddressBook::removeAddressee( const Iterator &it )
{
  if ( (*it).resource() )
    (*it).resource()->removeAddressee( *it );
}

AddressBook::Iterator AddressBook::find( const Addressee &a )
{
  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( a.uid() == (*it).uid() )
      return it;
  }

  return end();
}

Addressee AddressBook::findByUid( const TQString &uid )
{
  KRES::Manager<Resource>::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    Addressee addr = (*it)->findByUid( uid );
    if ( !addr.isEmpty() )
      return addr;
  }

  return Addressee();
}

Addressee::List AddressBook::allAddressees()
{
  Addressee::List list;

  ConstIterator it;
  for ( it = begin(); it != end(); ++it )
    list.append( *it );

  return list;
}

Addressee::List AddressBook::findByName( const TQString &name )
{
  Addressee::List results;

  KRES::Manager<Resource>::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it )
    results += (*it)->findByName( name );

  return results;
}

Addressee::List AddressBook::findByEmail( const TQString &email )
{
  Addressee::List results;

  KRES::Manager<Resource>::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it )
    results += (*it)->findByEmail( email );

  return results;
}

Addressee::List AddressBook::findByCategory( const TQString &category )
{
  Addressee::List results;

  KRES::Manager<Resource>::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it )
    results += (*it)->findByCategory( category );

  return results;
}

void AddressBook::dump() const
{
  kdDebug(5700) << "AddressBook::dump() --- begin ---" << endl;

  ConstIterator it;
  for( it = begin(); it != end(); ++it ) {
    (*it).dump();
  }

  kdDebug(5700) << "AddressBook::dump() ---  end  ---" << endl;
}

TQString AddressBook::identifier()
{
  TQStringList identifier;


  KRES::Manager<Resource>::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    if ( !(*it)->identifier().isEmpty() )
      identifier.append( (*it)->identifier() );
  }

  return identifier.join( ":" );
}

Field::List AddressBook::fields( int category )
{
  if ( d->mAllFields.isEmpty() ) {
    d->mAllFields = Field::allFields();
  }

  if ( category == Field::All ) return d->mAllFields;

  Field::List result;
  Field::List::ConstIterator it;
  for ( it = d->mAllFields.constBegin(); it != d->mAllFields.constEnd(); ++it ) {
    if ( (*it)->category() & category )
      result.append( *it );
  }

  return result;
}

bool AddressBook::addCustomField( const TQString &label, int category,
                                  const TQString &key, const TQString &app )
{
  if ( d->mAllFields.isEmpty() ) {
    d->mAllFields = Field::allFields();
  }

  TQString a = app.isNull() ? TDEGlobal::instance()->instanceName() : app;
  TQString k = key.isNull() ? label : key;

  Field *field = Field::createCustomField( label, category, k, a );

  if ( !field ) return false;

  d->mAllFields.append( field );

  return true;
}

TQDataStream &TDEABC::operator<<( TQDataStream &s, const AddressBook &ab )
{
  if (!ab.d) return s;

  return s;// << ab.d->mAddressees;
}

TQDataStream &TDEABC::operator>>( TQDataStream &s, AddressBook &ab )
{
  if (!ab.d) return s;

//  s >> ab.d->mAddressees;

  return s;
}

bool AddressBook::addResource( Resource *resource )
{
  if ( !resource->open() ) {
    kdDebug(5700) << "AddressBook::addResource(): can't add resource" << endl;
    return false;
  }

  d->mManager->add( resource );
  resource->setAddressBook( this );

  connect( resource, TQ_SIGNAL( loadingFinished( Resource* ) ),
           this, TQ_SLOT( resourceLoadingFinished( Resource* ) ) );
  connect( resource, TQ_SIGNAL( savingFinished( Resource* ) ),
           this, TQ_SLOT( resourceSavingFinished( Resource* ) ) );

  connect( resource, TQ_SIGNAL( loadingError( Resource*, const TQString& ) ),
           this, TQ_SLOT( resourceLoadingError( Resource*, const TQString& ) ) );
  connect( resource, TQ_SIGNAL( savingError( Resource*, const TQString& ) ),
           this, TQ_SLOT( resourceSavingError( Resource*, const TQString& ) ) );

  return true;
}

bool AddressBook::removeResource( Resource *resource )
{
  resource->close();

  if ( resource == standardResource() )
    d->mManager->setStandardResource( 0 );

  resource->setAddressBook( 0 );

  disconnect( resource, TQ_SIGNAL( loadingFinished( Resource* ) ),
              this, TQ_SLOT( resourceLoadingFinished( Resource* ) ) );
  disconnect( resource, TQ_SIGNAL( savingFinished( Resource* ) ),
              this, TQ_SLOT( resourceSavingFinished( Resource* ) ) );

  disconnect( resource, TQ_SIGNAL( loadingError( Resource*, const TQString& ) ),
              this, TQ_SLOT( resourceLoadingError( Resource*, const TQString& ) ) );
  disconnect( resource, TQ_SIGNAL( savingError( Resource*, const TQString& ) ),
              this, TQ_SLOT( resourceLoadingError( Resource*, const TQString& ) ) );

  d->mManager->remove( resource );

  return true;
}

TQPtrList<Resource> AddressBook::resources()
{
  TQPtrList<Resource> list;

  KRES::Manager<Resource>::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    if ( d->mManager->standardResource() == (*it) )
      list.prepend( *it );
    else
      list.append( *it );
  }

  return list;
}

void AddressBook::setErrorHandler( ErrorHandler *handler )
{
  delete d->mErrorHandler;
  d->mErrorHandler = handler;
}

void AddressBook::error( const TQString& msg )
{
  if ( !d->mErrorHandler ) // create default error handler
    d->mErrorHandler = new ConsoleErrorHandler;

  if ( d->mErrorHandler )
    d->mErrorHandler->error( msg );
  else
    kdError(5700) << "no error handler defined" << endl;
}

void AddressBook::deleteRemovedAddressees()
{
  // no any longer needed
}

void AddressBook::setStandardResource( Resource *resource )
{
  d->mManager->setStandardResource( resource );
}

Resource *AddressBook::standardResource()
{
  return d->mManager->standardResource();
}

KRES::Manager<Resource> *AddressBook::resourceManager()
{
  return d->mManager;
}

void AddressBook::cleanUp()
{
}

bool AddressBook::loadingHasFinished() const
{
  return d->mPendingLoadResources.isEmpty();
}

void AddressBook::resourceLoadingFinished( Resource *res )
{
  d->mPendingLoadResources.remove( res );
  emit loadingFinished( res );

  if ( d->mPendingLoadResources.count() == 0 )
    emit addressBookChanged( this );
}

void AddressBook::resourceSavingFinished( Resource *res )
{
  d->mPendingSaveResources.remove( res );

  emit savingFinished( res );
}

void AddressBook::resourceLoadingError( Resource *res, const TQString &errMsg )
{
  error( errMsg );

  d->mPendingLoadResources.remove( res );
  if ( d->mPendingLoadResources.count() == 0 )
    emit addressBookChanged( this );
}

void AddressBook::resourceSavingError( Resource *res, const TQString &errMsg )
{
  error( errMsg );

  d->mPendingSaveResources.remove( res );
}
