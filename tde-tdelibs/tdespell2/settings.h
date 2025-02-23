/**
 * settings.h
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
#ifndef TDESPELL_SETTINGS_H
#define TDESPELL_SETTINGS_H

#include <tqstringlist.h>
#include <tqstring.h>
#include <tdelibs_export.h>

class TDESharedConfig;

namespace KSpell2
{
    class Broker;

    class TDE_EXPORT Settings
    {
    public:
        ~Settings();

        void setDefaultLanguage( const TQString& lang );
        TQString defaultLanguage() const;

        void setDefaultClient( const TQString& client );
        TQString defaultClient() const;

        void setCheckUppercase( bool );
        bool checkUppercase() const;

        void setSkipRunTogether( bool );
        bool skipRunTogether() const;

        void setBackgroundCheckerEnabled( bool );
        bool backgroundCheckerEnabled() const;

        void setCurrentIgnoreList( const TQStringList& ignores );
        void addWordToIgnore( const TQString& word );
        TQStringList currentIgnoreList() const;
        bool ignore( const TQString& word );

        void save();

        TDESharedConfig *sharedConfig() const;

    private:
        void loadConfig();
        void readIgnoreList();
        void setQuietIgnoreList( const TQStringList& ignores );

    private:
        friend class Broker;
        Settings( Broker *broker, TDESharedConfig *config );
    private:
        class Private;
        Private *d;
    };
}

#endif
