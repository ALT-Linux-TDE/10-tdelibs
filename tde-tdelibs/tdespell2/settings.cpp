/*
 * settings.cpp
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
#include "settings.h"

#include "broker.h"

#include <tdeglobal.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <kdebug.h>

#include <tqmap.h>
#include <tqstringlist.h>

namespace KSpell2
{
class Settings::Private
{
public:
    Broker*  broker; //can't be a Ptr since we don't want to hold a ref on it
    TDESharedConfig::Ptr config;
    bool     modified;

    TQString defaultLanguage;
    TQString defaultClient;

    bool checkUppercase;
    bool skipRunTogether;
    bool backgroundCheckerEnabled;

    TQMap<TQString, bool> ignore;
};

Settings::Settings( Broker *broker, TDESharedConfig *config )
{
    d = new Private;
    d->broker = broker;

    Q_ASSERT( config );
    d->config = config;

    d->modified = false;
    loadConfig();
}

Settings::~Settings()
{
    delete d; d = 0;
}

TDESharedConfig *Settings::sharedConfig() const
{
    return d->config;
}

void Settings::setDefaultLanguage( const TQString& lang )
{
    TQStringList cs = d->broker->languages();
    if ( cs.find( lang ) != cs.end() &&
         d->defaultLanguage != lang ) {
        d->defaultLanguage = lang;
        readIgnoreList();
        d->modified = true;
        d->broker->changed();
    }
}

TQString Settings::defaultLanguage() const
{
    return d->defaultLanguage;
}

void Settings::setDefaultClient( const TQString& client )
{
    //Different from setDefaultLanguage because
    //the number of clients can't be even close
    //as big as the number of languages
    if ( d->broker->clients().contains( client ) ) {
        d->defaultClient = client;
        d->modified = true;
        d->broker->changed();
    }
}

TQString Settings::defaultClient() const
{
    return d->defaultClient;
}

void Settings::setCheckUppercase( bool check )
{
    if ( d->checkUppercase != check ) {
        d->modified = true;
        d->checkUppercase = check;
    }
}

bool Settings::checkUppercase() const
{
    return d->checkUppercase;
}

void Settings::setSkipRunTogether( bool skip )
{
    if ( d->skipRunTogether != skip ) {
        d->modified = true;
        d->skipRunTogether = skip;
    }
}

bool Settings::skipRunTogether() const
{
    return d->skipRunTogether;
}

void Settings::setBackgroundCheckerEnabled( bool enable )
{
    if ( d->backgroundCheckerEnabled != enable ) {
        d->modified = true;
        d->backgroundCheckerEnabled = enable;
    }
}

bool Settings::backgroundCheckerEnabled() const
{
    return d->backgroundCheckerEnabled;
}

void Settings::setCurrentIgnoreList( const TQStringList& ignores )
{
    setQuietIgnoreList( ignores );
    d->modified = true;
}

void Settings::setQuietIgnoreList( const TQStringList& ignores )
{
    d->ignore = TQMap<TQString, bool>();//clear out
    for ( TQStringList::const_iterator itr = ignores.begin();
          itr != ignores.end(); ++itr ) {
        d->ignore.insert( *itr, true );
    }
}

TQStringList Settings::currentIgnoreList() const
{
    return d->ignore.keys();
}

void Settings::addWordToIgnore( const TQString& word )
{
    if ( !d->ignore.contains( word ) ) {
        d->modified = true;
        d->ignore.insert( word, true );
    }
}

bool Settings::ignore( const TQString& word )
{
    return d->ignore.contains( word );
}

void Settings::readIgnoreList()
{
    TDEConfigGroup conf( d->config, "Spelling" );
    TQString ignoreEntry = TQString( "ignore_%1" ).arg( d->defaultLanguage );
    TQStringList ignores = conf.readListEntry( ignoreEntry );
    setQuietIgnoreList( ignores );
}

void Settings::save()
{
    if ( d->modified ) {
        TDEConfigGroup conf( d->config, "Spelling" );
        conf.writeEntry( "defaultClient", d->defaultClient );
        conf.writeEntry( "defaultLanguage", d->defaultLanguage );
        conf.writeEntry( "checkUppercase", d->checkUppercase );
        conf.writeEntry( "skipRunTogether", d->skipRunTogether );
        conf.writeEntry( "backgroundCheckerEnabled", d->backgroundCheckerEnabled );
        conf.writeEntry( TQString( "ignore_%1" ).arg( d->defaultLanguage ),
                         d->ignore.keys() );
        conf.sync();
    }
}

void Settings::loadConfig()
{
    TDEConfigGroup conf( d->config, "Spelling" );
    d->defaultClient = conf.readEntry( "defaultClient",
                                        TQString::null );
    d->defaultLanguage = conf.readEntry(
        "defaultLanguage", TDEGlobal::locale()->language() );

    //same defaults are in the default filter (filter.cpp)
    d->checkUppercase = conf.readBoolEntry(
        "checkUppercase", true );

    d->skipRunTogether = conf.readBoolEntry(
        "skipRunTogether", true );

    d->backgroundCheckerEnabled = conf.readBoolEntry(
        "backgroundCheckerEnabled", true );

    readIgnoreList();
}


}
