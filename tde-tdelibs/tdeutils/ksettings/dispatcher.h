/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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

#ifndef KSETTINGS_DISPATCHER_H
#define KSETTINGS_DISPATCHER_H

#include <tqobject.h>
#include <tqmap.h>
#include <tdelibs_export.h>

class TQCString;
class TQSignal;
class TQStrList;
template<class T> class KStaticDeleter;
class TDEInstance;
class TDEConfig;

namespace KSettings
{

/**
 * @ingroup settings
 * @short Dispatch change notifications from the KCMs to the program.
 *
 * Since your program does not have direct control over the KCMs that get loaded
 * into the TDEConfigureDialog you need a way to get notified. This is what you
 * do:
 * \code
 * Dispatcher::self()->registerInstance( instance(), this, TQ_SLOT( loadSettings() ) );
 * \endcode
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @since 3.2
 */
class TDEUTILS_EXPORT Dispatcher : public TQObject
{
    friend class KStaticDeleter<Dispatcher>;

    TQ_OBJECT
    public:
        /**
         * Get a reference the the Dispatcher object.
         */
        static Dispatcher * self();

        /**
         * Register a slot to be called when the configuration for the instance
         * has changed. @p instance is the TDEInstance object
         * that is passed to KGenericFactory (if it is used). You can query
         * it with KGenericFactory<YourClassName>::instance().
         * instance->instanceName() is also the same name that is put into the
         * .desktop file of the KCMs for the X-TDE-ParentComponents.
         *
         * @param instance     The TDEInstance object
         * @param recv         The object that should receive the signal
         * @param slot         The slot to be called: TQ_SLOT( slotName() )
         */
        void registerInstance( TDEInstance * instance, TQObject * recv, const char * slot );

        /**
         * @return the TDEConfig object that belongs to the instanceName
         */
        TDEConfig * configForInstanceName( const TQCString & instanceName );

        /**
         * @return a list of all the instance names that are currently
         * registered
         */
        TQStrList instanceNames() const;

//X         /**
//X          * @return The TDEInstance object belonging to the instance name you pass
//X          * (only works for registered instances of course).
//X          */
//X         TDEInstance * instanceForName( const TQCString & instanceName );

    public slots:
        /**
         * Call this slot when the configuration belonging to the associated
         * instance name has changed. The registered slot will be called.
         *
         * @param instanceName The value of X-TDE-ParentComponents.
         */
        void reparseConfiguration( const TQCString & instanceName );

        /**
         * When this slot is called the TDEConfig objects of all the registered
         * instances are sync()ed. This is usefull when some other TDEConfig
         * objects will read/write from/to the same config file, so that you
         * can first write out the current state of the TDEConfig objects.
         */
        void syncConfiguration();

    private slots:
        void unregisterInstance( TQObject * );

    private:
        Dispatcher( TQObject * parent = 0, const char * name = 0 );
        ~Dispatcher();
        static Dispatcher * m_self;

        struct InstanceInfo {
            TDEInstance * instance;
            TQSignal * signal;
            int count;
        };
        TQMap<TQCString, InstanceInfo> m_instanceInfo;
        TQMap<TQObject *, TQCString> m_instanceName;

        class DispatcherPrivate;
        DispatcherPrivate * d;
};

}
#endif // KSETTINGS_DISPATCHER_H
