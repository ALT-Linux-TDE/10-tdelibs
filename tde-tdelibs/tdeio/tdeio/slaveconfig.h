/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef TDEIO_SLAVE_CONFIG_H
#define TDEIO_SLAVE_CONFIG_H

#include <tqobject.h>
#include <tdeio/global.h>

namespace TDEIO {

    class SlaveConfigPrivate;
    /**
     * SlaveConfig
     *
     * This class manages the configuration for io-slaves based on protocol
     * and host. The Scheduler makes use of this class to configure the slave
     * whenever it has to connect to a new host.
     *
     * You only need to use this class if you want to override specific
     * configuration items of an io-slave when the io-slave is used by
     * your application. 
     *
     * Normally io-slaves are being configured by "tdeio_<protocol>rc" 
     * configuration files. Groups defined in such files are treated as host 
     * or domain specification. Configuration items defined in a group are 
     * only applied when the slave is connecting with a host that matches with 
     * the host and/or domain specified by the group.
     */
    class TDEIO_EXPORT SlaveConfig : public TQObject
    {
	TQ_OBJECT
    public:
        static SlaveConfig *self();
        ~SlaveConfig();
        /**
         * Configure slaves of type @p protocol by setting @p key to @p value.
         * If @p host is specified the configuration only applies when dealing
         * with @p host.
         *
         * Changes made to the slave configuration only apply to slaves
         * used by the current process.
         */
        void setConfigData(const TQString &protocol, const TQString &host, const TQString &key, const TQString &value );
        
        /**
         * Configure slaves of type @p protocol with @p config.
         * If @p host is specified the configuration only applies when dealing
         * with @p host.
         *
         * Changes made to the slave configuration only apply to slaves
         * used by the current process.
         */
        void setConfigData(const TQString &protocol, const TQString &host, const MetaData &config );
                
        /**
         * Query slave configuration for slaves of type @p protocol when
         * dealing with @p host.
         */
        MetaData configData(const TQString &protocol, const TQString &host);

        /**
         * Query a specific configuration key for slaves of type @p protocol when
         * dealing with @p host.
         */
        TQString configData(const TQString &protocol, const TQString &host, const TQString &key);

        /**
         * Undo any changes made by calls to setConfigData.
         */
        void reset();
    signals:
        /**
         * This signal is raised when a slave of type @p protocol deals
         * with @p host for the first time.
         *
         * Your application can use this signal to make some last minute
         * configuration changes with setConfigData based on the
         * host.
         */
        void configNeeded(const TQString &protocol, const TQString &host);
    protected:
        SlaveConfig();
        static SlaveConfig *_self;
        SlaveConfigPrivate *d;
    };
}

#endif
