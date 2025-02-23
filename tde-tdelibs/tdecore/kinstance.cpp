/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "kinstance.h"

#include <stdlib.h>
#include <unistd.h>

#include "tdeconfig.h"
#include "tdelocale.h"
#include "kcharsets.h"
#include "kiconloader.h"
#ifdef __TDE_HAVE_TDEHWLIB
#include "klibloader.h"
#include "tdeversion.h"
#include "tdehardwaredevices.h"
#include "tdenetworkconnections.h"
#endif
#include "tdeaboutdata.h"
#include "kstandarddirs.h"
#include "kdebug.h"
#include "tdeglobal.h"
#include "kmimesourcefactory.h"

#include <tqfont.h>

#include "config.h"
#ifndef NDEBUG
  #include <assert.h>
  #include <tqptrdict.h>
  static TQPtrList<TDEInstance> *allInstances = 0;
  static TQPtrDict<TQCString> *allOldInstances = 0;
  #define DEBUG_ADD do { if (!allInstances) { allInstances = new TQPtrList<TDEInstance>(); allOldInstances = new TQPtrDict<TQCString>(); } allInstances->append(this); allOldInstances->insert( this, new TQCString( _name)); } while (false);
  #define DEBUG_REMOVE do { allInstances->removeRef(this); } while (false);
  #define DEBUG_CHECK_ALIVE do { if (!allInstances->contains((TDEInstance*)this)) { TQCString *old = allOldInstances->find((TDEInstance*)this); tqWarning("ACCESSING DELETED KINSTANCE! (%s)", old ? old->data() : "<unknown>"); assert(false); } } while (false);
#else
  #define DEBUG_ADD
  #define DEBUG_REMOVE
  #define DEBUG_CHECK_ALIVE
#endif

class TDEInstancePrivate
{
public:
    TDEInstancePrivate ()
    {
        mimeSourceFactory = 0L;
    }

    ~TDEInstancePrivate ()
    {
        delete mimeSourceFactory;
    }

    KMimeSourceFactory* mimeSourceFactory;
    TQString configName;
    bool ownAboutdata;
    TDESharedConfig::Ptr sharedConfig;
};

TDEInstance::TDEInstance( const TQCString& name)
  : _dirs (0L),
    _config (0L),
    _iconLoader (0L),
#ifdef __TDE_HAVE_TDEHWLIB
    _hardwaredevices (0L),
    _networkmanager (0L),
#endif
    _name( name ), _aboutData( new TDEAboutData( name, "", 0 ) ), m_configReadOnly(false)
{
    DEBUG_ADD
    Q_ASSERT(!name.isEmpty());
    if (!TDEGlobal::_instance)
    {
      TDEGlobal::_instance = this;
      TDEGlobal::setActiveInstance(this);
    }

    d = new TDEInstancePrivate ();
    d->ownAboutdata = true;
}

TDEInstance::TDEInstance( const TDEAboutData * aboutData )
  : _dirs (0L),
    _config (0L),
    _iconLoader (0L),
#ifdef __TDE_HAVE_TDEHWLIB
    _hardwaredevices (0L),
    _networkmanager (0L),
#endif
    _name( aboutData->appName() ), _aboutData( aboutData ), m_configReadOnly(false)
{
    DEBUG_ADD
    Q_ASSERT(!_name.isEmpty());

    if (!TDEGlobal::_instance)
    {
      TDEGlobal::_instance = this;
      TDEGlobal::setActiveInstance(this);
    }

    d = new TDEInstancePrivate ();
    d->ownAboutdata = false;
}

TDEInstance::TDEInstance( TDEInstance* src )
  : _dirs ( src->_dirs ),
    _config ( src->_config ),
    _iconLoader ( src->_iconLoader ),
#ifdef __TDE_HAVE_TDEHWLIB
    _hardwaredevices ( src->_hardwaredevices ),
    _networkmanager ( src->_networkmanager ),
#endif
    _name( src->_name ), _aboutData( src->_aboutData ), m_configReadOnly(false)
{
    DEBUG_ADD
    Q_ASSERT(!_name.isEmpty());

    if (!TDEGlobal::_instance || TDEGlobal::_instance == src )
    {
      TDEGlobal::_instance = this;
      TDEGlobal::setActiveInstance(this);
    }

    d = new TDEInstancePrivate ();
    d->ownAboutdata = src->d->ownAboutdata;
    d->sharedConfig = src->d->sharedConfig;

    src->_dirs = 0L;
    src->_config = 0L;
    src->_iconLoader = 0L;
#ifdef __TDE_HAVE_TDEHWLIB
    src->_hardwaredevices = 0L;
    src->_networkmanager = 0L;
#endif
    src->_aboutData = 0L;
    delete src;
}

TDEInstance::~TDEInstance()
{
    DEBUG_CHECK_ALIVE

    if (d->ownAboutdata)
        delete _aboutData;
    _aboutData = 0;

    delete d;
    d = 0;

    delete _iconLoader;
    _iconLoader = 0;

#ifdef __TDE_HAVE_TDEHWLIB
    delete _hardwaredevices;
    _hardwaredevices = 0;

    delete _networkmanager;
    _networkmanager = 0;
#endif

    // delete _config; // Do not delete, stored in d->sharedConfig
    _config = 0;
    delete _dirs;
    _dirs = 0;

    if (TDEGlobal::_instance == this)
        TDEGlobal::_instance = 0;
    if (TDEGlobal::activeInstance() == this)
        TDEGlobal::setActiveInstance(0);
    DEBUG_REMOVE
}


TDEStandardDirs *TDEInstance::dirs() const
{
    DEBUG_CHECK_ALIVE
    if( _dirs == 0 ) {
	_dirs = new TDEStandardDirs( );
        if (_config) {
            if (_dirs->addCustomized(_config))
                _config->reparseConfiguration();
	} else
            config(); // trigger adding of possible customized dirs
    }

    return _dirs;
}

extern bool kde_kiosk_exception;
extern bool kde_kiosk_admin;

void TDEInstance::setConfigReadOnly(bool ro)
{
    m_configReadOnly = ro;
}

TDEConfig	*TDEInstance::config() const
{
    DEBUG_CHECK_ALIVE
    if( _config == 0 ) {
        if ( !d->configName.isEmpty() )
        {
            d->sharedConfig = TDESharedConfig::openConfig( d->configName );

            // Check whether custom config files are allowed.
            d->sharedConfig->setGroup( "KDE Action Restrictions" );
            TQString kioskException = d->sharedConfig->readEntry("kiosk_exception");
            if (d->sharedConfig->readBoolEntry( "custom_config", true))
            {
               d->sharedConfig->setGroup(TQString::null);
            }
            else
            {
               d->sharedConfig = 0;
            }

        }

        if ( d->sharedConfig == 0 )
        {
	    if ( !_name.isEmpty() ) {
	        d->sharedConfig = TDESharedConfig::openConfig( _name + "rc", m_configReadOnly );
	    }
	    else {
	        d->sharedConfig = TDESharedConfig::openConfig( TQString::null );
	    }
	}

	// Check if we are excempt from kiosk restrictions
	if (kde_kiosk_admin && !kde_kiosk_exception && !TQCString(getenv("TDE_KIOSK_NO_RESTRICTIONS")).isEmpty())
	{
            kde_kiosk_exception = true;
            d->sharedConfig = 0;
            return config(); // Reread...
        }

	_config = d->sharedConfig;
        if (_dirs)
            if (_dirs->addCustomized(_config))
                _config->reparseConfiguration();
    }

    return _config;
}

TDESharedConfig *TDEInstance::sharedConfig() const
{
    DEBUG_CHECK_ALIVE
    if (_config == 0)
       (void) config(); // Initialize config

    return d->sharedConfig;
}

void TDEInstance::setConfigName(const TQString &configName)
{
    DEBUG_CHECK_ALIVE
    d->configName = configName;
}

TDEIconLoader *TDEInstance::iconLoader() const
{
    DEBUG_CHECK_ALIVE
    if( _iconLoader == 0 ) {
	_iconLoader = new TDEIconLoader( _name, dirs() );
    	_iconLoader->enableDelayedIconSetLoading( true );
    }

    return _iconLoader;
}

#ifdef __TDE_HAVE_TDEHWLIB
TDEHardwareDevices *TDEInstance::hardwareDevices() const
{
	DEBUG_CHECK_ALIVE
	if (_hardwaredevices == 0)
	{
		TQString libName = TQString("libtdehw.so.%1").arg(TDE_VERSION_MAJOR);
		KLibrary *lib = KLibLoader::self()->library(libName.latin1());
		if (lib)
		{
			TDEHardwareDevices* (*create)() = (TDEHardwareDevices* (*)())lib->symbol("create_tdeHardwareDevices");
			if (create)
			{
				_hardwaredevices = create();
			}
		}
	}

	return _hardwaredevices;
}

TDEGlobalNetworkManager *TDEInstance::networkManager() const
{
	DEBUG_CHECK_ALIVE
	if (_networkmanager == 0)
	{
		TQString libName = TQString("libtdehw.so.%1").arg(TDE_VERSION_MAJOR);
		KLibrary *lib = KLibLoader::self()->library(libName.latin1());
		if (lib)
		{
			TDEGlobalNetworkManager* (*create)() = (TDEGlobalNetworkManager* (*)())lib->symbol("create_tdeGlobalNetworkManager");
			if (create)
			{
				_networkmanager = create();
			}
		}
	}

	return _networkmanager;
}
#endif

void TDEInstance::newIconLoader() const
{
    DEBUG_CHECK_ALIVE
    TDEIconTheme::reconfigure();
    _iconLoader->reconfigure( _name, dirs() );
}

const TDEAboutData * TDEInstance::aboutData() const
{
    DEBUG_CHECK_ALIVE
    return _aboutData;
}

TQCString TDEInstance::instanceName() const
{
    DEBUG_CHECK_ALIVE
    return _name;
}

KMimeSourceFactory* TDEInstance::mimeSourceFactory () const
{
  DEBUG_CHECK_ALIVE
  if (!d->mimeSourceFactory)
  {
    d->mimeSourceFactory = new KMimeSourceFactory(_iconLoader);
    d->mimeSourceFactory->setInstance(const_cast<TDEInstance *>(this));
  }

  return d->mimeSourceFactory;
}

void TDEInstance::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

