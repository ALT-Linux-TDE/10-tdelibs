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
#ifndef _KINSTANCE_H
#define _KINSTANCE_H

#include <tqstring.h>
#include "tdelibs_export.h"

class TDEStandardDirs;
class TDEAboutData;
class TDEConfig;
class TDEIconLoader;
class KCharsets;
class TQFont;
class TDEInstancePrivate;
class KMimeSourceFactory;
class TDESharedConfig;
#ifdef __TDE_HAVE_TDEHWLIB
class TDEHardwareDevices;
class TDEGlobalNetworkManager;
#endif


/**
 * Access to KDE global objects for use in shared libraries.  In
 * practical terms, this class is used in KDE components.  This allows
 * components to store things that normally would be accessed by
 * TDEGlobal.
 *
 * @author Torben Weis
 */
class TDECORE_EXPORT TDEInstance
{
    friend class TDEStandardDirs;

 public:
    /**
     *  Constructor.
     *  @param instanceName the name of the instance
     */
    TDEInstance( const TQCString& instanceName) ;

    /**
     *  Constructor.
     *  When building a TDEInstance that is not your TDEApplication,
     *  make sure that the TDEAboutData and the TDEInstance have the same life time.
     *  You have to destroy both, since the instance doesn't own the about data.
     *  Don't build a TDEAboutData on the stack in this case !
     *  Building a TDEAboutData on the stack is only ok for usage with
     *  TDECmdLineArgs and TDEApplication (not destroyed until the app exits).
     *  @param aboutData data about this instance (see TDEAboutData)
     */
    TDEInstance( const TDEAboutData * aboutData );

    /*
     * @internal
     * Only for K(Unique)Application
     * Initialize from src and delete it.
     */

    TDEInstance( TDEInstance* src );

    /**
     * Destructor.
     */
    virtual ~TDEInstance();

    /**
     * Returns the application standard dirs object.
     * @return The TDEStandardDirs of the application.
     */
    TDEStandardDirs	*dirs() const;

    /**
     * Returns the general config object ("appnamerc").
     * @return the TDEConfig object for the instance.
     */
    TDEConfig            *config() const;

    /**
     * Returns the general config object ("appnamerc").
     * @return the TDEConfig object for the instance.
     */
    TDESharedConfig      *sharedConfig() const;

    /**
     * Set a read-only flag on the configuration files
     * This must be called before config() or dirs() to have any effect
     * Defaults to FALSE
     * @param ro read only if TRUE
     */
    void                setConfigReadOnly(bool ro);

    /**
     *  Returns an iconloader object.
     * @return the iconloader object.
     */
    TDEIconLoader	       *iconLoader() const;

#ifdef __TDE_HAVE_TDEHWLIB
    /**
     *  Returns a TDEHardwareDevices object.
     * @return the hardwaredevices object.
     */
    TDEHardwareDevices	*hardwareDevices() const;

    /**
     *  Returns a TDEGlobalNetworkManager object.
     * @return the networkmanager object.
     */
    TDEGlobalNetworkManager  *networkManager() const;
#endif

    /**
     * Re-allocate the global iconloader.
     */
    void newIconLoader() const;

    /**
     *  Returns the about data of this instance
     *  Warning, can be 0L
     * @return the about data of the instance, or 0 if it has 
     *         not been set yet
     */
    const TDEAboutData *aboutData() const;

    /**
     * Returns the name of the instance
     * @return the instance name, can be null if the TDEInstance has been 
     *         created with a null name
     */
    TQCString          instanceName() const;

    /**
     * Returns the KMimeSourceFactory of the instance.
     * Mainly added for API completeness and future extensibility.
     * @return the KMimeSourceFactory set as default for this application.
     */
    KMimeSourceFactory* mimeSourceFactory () const;

protected:
    /**
     *  Copy Constructor is not allowed
     */
    TDEInstance( const TDEInstance& );

    /**
     * Set name of default config file.
     * @param name the name of the default config file
     * @since 3.1
     */
    void setConfigName(const TQString &name);

private:
    mutable TDEStandardDirs       *_dirs;

    mutable TDEConfig             *_config;
    mutable TDEIconLoader         *_iconLoader;

#ifdef __TDE_HAVE_TDEHWLIB
    mutable TDEHardwareDevices  *_hardwaredevices;
    mutable TDEGlobalNetworkManager  *_networkmanager;
#endif
    mutable void                *_placeholder;

    TQCString                     _name;
    const TDEAboutData            *_aboutData;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    TDEInstancePrivate *d;
    bool m_configReadOnly;
};

#endif

