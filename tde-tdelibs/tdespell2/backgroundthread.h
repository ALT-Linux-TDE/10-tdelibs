/**
 * backgroundthread.h
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
#ifndef TDESPELL_BACKGROUNDTHREAD_H
#define TDESPELL_BACKGROUNDTHREAD_H

#include "broker.h"

#include <tqthread.h>
#include <tqmutex.h>

class TQObject;

namespace KSpell2
{
    class Filter;
    class Broker;
    class Dictionary;
    class BackgroundThread : public TQThread
    {
    public:
        BackgroundThread();
        void setReceiver( TQObject *parent );
        TQObject *receiver() const { return m_recv; }

        void setBroker( const Broker::Ptr& broker );
        Broker *broker() const { return m_broker; }

        void setText( const TQString& );
        TQString text() const;

        void changeLanguage( const TQString& );
        TQString language() const;

        void setFilter( Filter *filter );
        Filter *filter() const { return m_filter; }

        TQStringList suggest( const TQString& ) const;

        virtual void run();
        void stop();
    private:
        TQObject    *m_recv;
        TQMutex      m_mutex;
        Filter     *m_filter;
        Broker::Ptr m_broker;
        Dictionary *m_dict;
        bool        m_done;
    };
}

#endif
