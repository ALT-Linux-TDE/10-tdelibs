/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>
  Copyright (c) 2003 Matthias Kretz <kretz@kde.org>

  This file is part of the KDE project
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2, as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include <tqvariant.h>

#include <kdesktopfile.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <kstandarddirs.h>
#include <tdelocale.h>

#include "tdecmoduleinfo.h"

class TDECModuleInfo::TDECModuleInfoPrivate
{
  public:
    TDECModuleInfoPrivate() :
      testModule( false )
    {}
    ~TDECModuleInfoPrivate()
    { }

    TQString factoryName;
    bool testModule;

};

TDECModuleInfo::TDECModuleInfo()
{
  _allLoaded = false;
  d = new TDECModuleInfoPrivate;
}

TDECModuleInfo::TDECModuleInfo(const TQString& desktopFile)
{
  KService::Ptr service = KService::serviceByStorageId(desktopFile);
  if(!service) setName(desktopFile);
  init(service);
}

TDECModuleInfo::TDECModuleInfo( KService::Ptr moduleInfo )
{
  init(moduleInfo);
}

TDECModuleInfo::TDECModuleInfo( const TDECModuleInfo &rhs )
{
    d = new TDECModuleInfoPrivate;
    ( *this ) = rhs;
}

// this re-implementation exists to ensure that other code always calls
// our re-implementation, so in case we add data to the d pointer in the future 
// we can be sure that we get called when we are copied.
TDECModuleInfo &TDECModuleInfo::operator=( const TDECModuleInfo &rhs )
{
    _keywords = rhs._keywords;
    _name = rhs._name;
    _icon = rhs._icon;
    _lib = rhs._lib;
    _handle = rhs._handle;
    _fileName = rhs._fileName;
    _doc = rhs._doc;
    _comment = rhs._comment;
    _needsRootPrivileges = rhs._needsRootPrivileges;
    _isHiddenByDefault = rhs._isHiddenByDefault;
    _allLoaded = rhs._allLoaded;
    _service = rhs._service;

    *d = *(rhs.d);

    return *this;
}

TQString TDECModuleInfo::factoryName() const
{
  if( d->factoryName.isEmpty() )
  {
    d->factoryName = _service->property("X-TDE-FactoryName", TQVariant::String).toString();
    if ( d->factoryName.isEmpty() )
      d->factoryName = library();
  }

  return d->factoryName;
}

bool TDECModuleInfo::operator==( const TDECModuleInfo & rhs ) const
{
  return ( ( _name == rhs._name ) && ( _lib == rhs._lib ) && ( _fileName == rhs._fileName ) );
}

bool TDECModuleInfo::operator!=( const TDECModuleInfo & rhs ) const
{
  return ! operator==( rhs );
}

TDECModuleInfo::~TDECModuleInfo()
{ 
  delete d;
}

void TDECModuleInfo::init(KService::Ptr s)
{
  _allLoaded = false;
  d = new TDECModuleInfoPrivate;

  if ( s )
    _service = s;
  else
  {
    kdDebug(712) << "Could not find the service." << endl;
    return;
  }

  // set the modules simple attributes
  setName(_service->name());
  setComment(_service->comment());
  setIcon(_service->icon());

  _fileName = ( _service->desktopEntryPath() );

  // library and factory
  setLibrary(_service->library());

  // get the keyword list
  setKeywords(_service->keywords());
}

void
TDECModuleInfo::loadAll() 
{
  if( !_service ) /* We have a bogus service. All get functions will return empty/zero values */
    return;

  _allLoaded = true;

  // library and factory
  setHandle(_service->property("X-TDE-FactoryName", TQVariant::String).toString());

  TQVariant tmp;

  // read weight
  tmp = _service->property( "X-TDE-Weight", TQVariant::Int );
  setWeight( tmp.isValid() ? tmp.toInt() : 100 );

  // does the module need super user privileges?
  tmp = _service->property( "X-TDE-RootOnly", TQVariant::Bool );
  setNeedsRootPrivileges( tmp.isValid() ? tmp.toBool() : false );

  // does the module need to be shown to root only?
  // Deprecated ! KDE 4
  tmp = _service->property( "X-TDE-IsHiddenByDefault", TQVariant::Bool );
  setIsHiddenByDefault( tmp.isValid() ? tmp.toBool() : false );

  // get the documentation path
  setDocPath( _service->property( "X-DocPath", TQVariant::String ).toString() );

  tmp = _service->property( "X-TDE-Test-Module", TQVariant::Bool );
  setNeedsTest( tmp.isValid() ? tmp.asBool() : false );
}

TQString
TDECModuleInfo::docPath() const
{
  if (!_allLoaded) 
    const_cast<TDECModuleInfo*>(this)->loadAll();

  return _doc;
}

TQString
TDECModuleInfo::handle() const
{
  if (!_allLoaded) 
    const_cast<TDECModuleInfo*>(this)->loadAll();

  if (_handle.isEmpty())
     return _lib;

  return _handle;
}

int
TDECModuleInfo::weight() const
{
  if (!_allLoaded) 
    const_cast<TDECModuleInfo*>(this)->loadAll();

  return _weight;
}

bool
TDECModuleInfo::needsRootPrivileges() const
{
  if (!_allLoaded) 
    const_cast<TDECModuleInfo*>(this)->loadAll();

  return _needsRootPrivileges;
}

bool
TDECModuleInfo::isHiddenByDefault() const
{
  if (!_allLoaded)
    const_cast<TDECModuleInfo*>(this)->loadAll();

  return _isHiddenByDefault;
}

bool TDECModuleInfo::needsTest() const
{
  if (!_allLoaded)
    const_cast<TDECModuleInfo*>(this)->loadAll();

  return d->testModule;
}

void TDECModuleInfo::setNeedsTest( bool val )
{
  d->testModule = val;
}
