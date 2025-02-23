/*
 * defaultdictionary.h
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
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
#ifndef TDESPELL_DEFAULTDICTIONARY_H
#define TDESPELL_DEFAULTDICTIONARY_H

#include "dictionary.h"

#include <tqobject.h>
#include <tqstringlist.h>

namespace KSpell2
{
    class Broker;
    class DefaultDictionary : public TQObject,
                              public Dictionary
    {
        TQ_OBJECT
    public:
        DefaultDictionary( const TQString& lang, Broker *broker );
        ~DefaultDictionary();
    public:
        /**
         * Returns true whether the default dictionary object is
         * valid.
         * It might not be if there's no dictionary object
         * for the default language.
         */
        bool isValid() const;

        //Dictionary interface
        virtual bool check( const TQString& word );
        virtual TQStringList suggest( const TQString& word );
        virtual bool checkAndSuggest( const TQString& word,
                                      TQStringList& suggestions );
        virtual bool storeReplacement( const TQString& bad,
                                       const TQString& good );
        virtual bool addToPersonal( const TQString& word );
        virtual bool addToSession( const TQString& word );
        //end of Dictionary interfaces

    signals:
        void changed();

    protected slots:
        void defaultConfigurationChanged();

    private:
        class Private;
        Private *d;
    };
}

#endif
