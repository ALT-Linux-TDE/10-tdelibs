/**
 * broker.h
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#ifndef TDESPELL_BROKER_H
#define TDESPELL_BROKER_H

#include <ksharedptr.h>

#include <tqobject.h>
#include <tqstringlist.h>
#include <tqstring.h>

class TDESharedConfig;
template <typename T>
class TQPtrDict;

namespace KSpell2
{
    class Settings;
    class Dictionary;
    class DefaultDictionary;

    /**
     * @short Class used to deal with dictionaries
     *
     * This class manages all dictionaries. It's the top level
     * KSpell2 class, you can think of it as the kernel or manager
     * of the KSpell2 architecture.
     */
    class TDE_EXPORT Broker : public TQObject,
                   public TDEShared
    {
        TQ_OBJECT
    public:
        typedef TDESharedPtr<Broker> Ptr;
        /**
         * Constructs the broker.
         *
         * It's very important that you assign it to Broker::Ptr
         * as soon as possible. Broker is reference counted so
         * if you don't want to have it deleted under you simply
         * have to hold it in a Broker::Ptr for as long as you're
         * using it.
         *
         * @param config is the name of config file which
         *        broker should use to read default language
         *        and default client values. If no value will
         *        be passed Broker will use global tdespellrc file.
         */
        static Broker *openBroker( TDESharedConfig *config = 0 );

    public:
        ~Broker();

        /**
         * Function returns the so-called DefaultDictionary. It's a
         * special form a dictionary which automatically mutates
         * according to changes tot the KSpell2::Settings object.
         * You also can't delete it like other dictionaries since
         * it's owned by the Broker and it will take care of it.
         */
        DefaultDictionary *defaultDictionary() const;

        /**
         * Returns dictionary for the given language and preferred client.
         *
         * @param language specifies the language of the dictionary. If an
         *        empty string will be passed the default language will
         *        be used. Has to be one of the values returned by
         *        \ref languages()
         * @param client specifies the preferred client. If no client is
         *               specified a client which supports the given
         *               language is picked. If a few clients supports
         *               the same language the one with the biggest
         *               reliability value is returned.
         *
         */
        Dictionary *dictionary(
            const TQString& language = TQString::null,
            const TQString& client = TQString::null ) const;

        /**
         * Returns names of all supported clients (e.g. ISpell, ASpell)
         */
        TQStringList clients() const;

        /**
         * Returns a list of supported languages.
         */
        TQStringList languages() const;

        /**
         * Returns the Settings object used by the broker.
         */
        Settings *settings() const;
    signals:
        /**
         * Signal is emitted whenever the Settings object
         * associated with this Broker changes.
         */
        void configurationChanged();

    protected:
        friend class Settings;
        void changed();
    private:
        Broker( TDESharedConfig *config );
        void loadPlugins();
        void loadPlugin( const TQString& );
    private:
        class Private;
        Private *d;
    private:
        static TQPtrDict<Broker> *s_brokers;
    };
}

#endif
