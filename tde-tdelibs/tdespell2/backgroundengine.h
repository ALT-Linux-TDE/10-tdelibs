/*
 * backgroundengine.h
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
#ifndef TDESPELL_BACKGROUNDENGINE_H
#define TDESPELL_BACKGROUNDENGINE_H

#include "broker.h"

#include <tqobject.h>
#include <tqstringlist.h>

namespace KSpell2
{
    class Filter;
    class Broker;
    class Dictionary;
    class BackgroundEngine : public TQObject
    {
        TQ_OBJECT
    public:
        BackgroundEngine( TQObject *parent );
        ~BackgroundEngine();

        void setBroker( const Broker::Ptr& broker );
        Broker *broker() const { return m_broker; }

        void setText( const TQString& );
        TQString text() const;

        void changeLanguage( const TQString& );
        TQString language() const;

        void setFilter( Filter *filter );
        Filter *filter() const { return m_filter; }

        void start();
        void continueChecking();
        void stop();

        bool        checkWord( const TQString& word );
        TQStringList suggest( const TQString& word );
        bool        addWord( const TQString& word );
    signals:
        void misspelling( const TQString&, int );
        void done();
    protected slots:
        void checkNext();
    private:
        Filter            *m_filter;
        Broker::Ptr        m_broker;
        Dictionary        *m_dict;
        DefaultDictionary *m_defaultDict;
    };
}

#endif
