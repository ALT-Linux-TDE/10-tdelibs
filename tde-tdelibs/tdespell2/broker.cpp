/**
 * broker.cpp
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
#include "broker.h"
#include "settings.h"
#include "client.h"
#include "defaultdictionary.h"
#include "dictionary.h"

#include <tdeparts/plugin.h>
#include <tdeparts/componentfactory.h>

#include <kplugininfo.h>
#include <ktrader.h>
#include <tdeconfig.h>

#include <kdebug.h>

#include <tqptrdict.h>
#include <tqmap.h>

#define DEFAULT_CONFIG_FILE   "tdespellrc"

namespace KSpell2
{

class Broker::Private
{
public:
    KPluginInfo::List plugins;
    Settings *settings;

    // <language, Clients with that language >
    TQMap<TQString, TQPtrList<Client> > languageClients;
    TQStringList clients;
    DefaultDictionary *defaultDictionary;
};

TQPtrDict<Broker> *Broker::s_brokers = 0;

Broker *Broker::openBroker( TDESharedConfig *config )
{
    TDESharedConfig::Ptr preventDeletion;
    if ( !config ) {
        preventDeletion = TDESharedConfig::openConfig( DEFAULT_CONFIG_FILE );
    } else
        preventDeletion = config;

    if ( s_brokers ) {
        Broker *broker = s_brokers->find( preventDeletion );
        if ( broker )
            return broker;
    }

    Broker *broker = new Broker( preventDeletion );
    return broker;
}

Broker::Broker( TDESharedConfig *config )
{
    TDESharedConfig::Ptr preventDeletion( config );
    Q_UNUSED( preventDeletion );

    if ( !s_brokers )
        s_brokers = new TQPtrDict<Broker>;
    s_brokers->insert( config, this );

    d = new Private;
    d->settings = new Settings( this, config );
    loadPlugins();

    d->defaultDictionary = new DefaultDictionary( d->settings->defaultLanguage(),
                                                  this );
}

Broker::~Broker()
{
    kdDebug()<<"Removing broker : "<< this << endl;
    s_brokers->remove( d->settings->sharedConfig() );
    KPluginInfo::List::iterator it = d->plugins.begin();
    while ( it != d->plugins.end() ) {
        KPluginInfo *pluginInfo = *it;
        it = d->plugins.remove( it );
        delete pluginInfo;
    }

    delete d->settings; d->settings = 0;
    delete d; d = 0;
}

DefaultDictionary* Broker::defaultDictionary() const
{
    return d->defaultDictionary;
}

Dictionary* Broker::dictionary( const TQString& language, const TQString& clientName ) const
{
    TQString pclient = clientName;
    TQString plang   = language;
    bool ddefault = false;

    if ( plang.isEmpty() ) {
        plang = d->settings->defaultLanguage();
    }
    if ( clientName == d->settings->defaultClient() &&
        plang == d->settings->defaultLanguage() ) {
        ddefault = true;
    }

    TQPtrList<Client> lClients = d->languageClients[ plang ];

    if ( lClients.isEmpty() ) {
        kdError()<<"No language dictionaries for the language : "<< plang <<endl;
        return 0;
    }

    TQPtrListIterator<Client> itr( lClients );
    while ( itr.current() ) {
        if ( !pclient.isEmpty() ) {
            if ( pclient == itr.current()->name() ) {
                Dictionary *dict = itr.current()->dictionary( plang );
                if ( dict ) //remove the if if the assert proves ok
                    dict->m_default = ddefault;
                return dict;
            }
        } else {
            //the first one is the one with the highest
            //reliability
            Dictionary *dict = itr.current()->dictionary( plang );
            Q_ASSERT( dict );
            if ( dict ) //remove the if if the assert proves ok
                dict->m_default = ddefault;
            return dict;
        }
        ++itr;
    }

    return 0;
}

TQStringList Broker::clients() const
{
    return d->clients;
}

TQStringList Broker::languages() const
{
    return d->languageClients.keys();
}

Settings* Broker::settings() const
{
    return d->settings;
}

void Broker::loadPlugins()
{
    d->plugins = KPluginInfo::fromServices(
        TDETrader::self()->query( "KSpell/Client" ) );

    for ( KPluginInfo::List::Iterator itr = d->plugins.begin();
          itr != d->plugins.end(); ++itr ) {
        loadPlugin( ( *itr )->pluginName() );
    }
}

void Broker::loadPlugin( const TQString& pluginId )
{
    int error = 0;

    kdDebug()<<"Loading plugin " << pluginId << endl;

    Client *client = KParts::ComponentFactory::createInstanceFromQuery<Client>(
        TQString::fromLatin1( "KSpell/Client" ),
        TQString::fromLatin1( "[X-TDE-PluginInfo-Name]=='%1'" ).arg( pluginId ),
        this, 0, TQStringList(), &error );

    if ( client )
    {
        TQStringList languages = client->languages();
        d->clients.append( client->name() );

        for ( TQStringList::Iterator itr = languages.begin();
              itr != languages.end(); ++itr ) {
            if ( !d->languageClients[ *itr ].isEmpty() &&
                 client->reliability() < d->languageClients[ *itr ].first()->reliability() )
                d->languageClients[ *itr ].append( client );
            else
                d->languageClients[ *itr ].prepend( client );
        }

        kdDebug() << k_funcinfo << "Successfully loaded plugin '"
                  << pluginId << "'" << endl;
    }
    else
    {
        switch( error )
        {
        case KParts::ComponentFactory::ErrNoServiceFound:
            kdDebug() << k_funcinfo << "No service implementing the given mimetype "
                      << "and fullfilling the given constraint expression can be found."
                      << endl;
            break;
        case KParts::ComponentFactory::ErrServiceProvidesNoLibrary:
            kdDebug() << "the specified service provides no shared library." << endl;
            break;
        case KParts::ComponentFactory::ErrNoLibrary:
            kdDebug() << "the specified library could not be loaded." << endl;
            break;
        case KParts::ComponentFactory::ErrNoFactory:
            kdDebug() << "the library does not export a factory for creating components."
                      << endl;
            break;
        case KParts::ComponentFactory::ErrNoComponent:
            kdDebug() << "the factory does not support creating "
                      << "components of the specified type."
                      << endl;
            break;
        }

        kdDebug() << k_funcinfo << "Loading plugin '" << pluginId
                  << "' failed, KLibLoader reported error: '" << endl
                  << KLibLoader::self()->lastErrorMessage() << "'" << endl;
    }
}

void Broker::changed()
{
    emit configurationChanged();
}

}

#include "broker.moc"
