/**
 * defaultdictionary.cpp
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
#include "defaultdictionary.h"

#include "broker.h"

using namespace KSpell2;


class DefaultDictionary::Private
{
public:
    Dictionary *dict;
    Broker     *broker; //not a Ptr because Broker holds DefaultDictionary
                        //we need it only to switch the dics
};

DefaultDictionary::DefaultDictionary( const TQString& lang, Broker *broker )
    : TQObject( broker ), Dictionary( lang, true )
{
    d = new Private;
    d->dict = broker->dictionary();
    d->broker = broker;
    connect( broker, TQ_SIGNAL(configurationChanged()),
             TQ_SLOT(defaultConfigurationChanged()) );
}

DefaultDictionary::~DefaultDictionary()
{
    delete d; d = 0;
}

bool DefaultDictionary::isValid() const
{
    //if d->dict exists then we're valid
    return d->dict;
}

bool DefaultDictionary::check( const TQString& word )
{
    if ( d->dict )
        return d->dict->check( word );
    else
        return true;
}

TQStringList DefaultDictionary::suggest( const TQString& word )
{
    if ( d->dict )
        return d->dict->suggest( word );
    else
        return TQStringList();

}

bool DefaultDictionary::checkAndSuggest( const TQString& word,
                                         TQStringList& suggestions )
{
    if ( d->dict )
        return d->dict->checkAndSuggest( word, suggestions );
    else
        return true;
}

bool DefaultDictionary::storeReplacement( const TQString& bad,
                                          const TQString& good )
{
    if ( d->dict )
        return d->dict->storeReplacement( bad, good );
    else
        return false;
}

bool DefaultDictionary::addToPersonal( const TQString& word )
{
    if ( d->dict )
        return d->dict->addToPersonal( word );
    else
        return false;
}

bool DefaultDictionary::addToSession( const TQString& word )
{
    if ( d->dict )
        return d->dict->addToSession( word );
    else
        return false;
}

void DefaultDictionary::defaultConfigurationChanged()
{
    delete d->dict;
    d->dict = d->broker->dictionary();
    if ( d->dict )
        m_language = d->dict->language();
    else
        m_language = TQString::null;
}

#include "defaultdictionary.moc"
