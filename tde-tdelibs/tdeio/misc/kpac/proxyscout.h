/* 
   Copyright (c) 2003 Malte Starostik <malte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#ifndef KPAC_PROXYSCOUT_H
#define KPAC_PROXYSCOUT_H

#include <tqmap.h>

#include <kdedmodule.h>
#include <kurl.h>

#include <time.h>

class DCOPClientTransaction;
class TDEInstance;

namespace KPAC
{
    class Downloader;
    class Script;

    class ProxyScout : public KDEDModule
    {
        TQ_OBJECT
        K_DCOP
    public:
        ProxyScout( const TQCString& );
        virtual ~ProxyScout();

    k_dcop:
        TQString proxyForURL( const KURL& url );
        ASYNC blackListProxy( const TQString& proxy );
        ASYNC reset();

    private slots:
        void downloadResult( bool );

    private:
        bool startDownload();
        TQString handleRequest( const KURL& url );

        TDEInstance* m_instance;
        Downloader* m_downloader;
        Script* m_script;

        struct QueuedRequest
        {
            QueuedRequest() : transaction( 0 ) {}
            QueuedRequest( const KURL& );

            DCOPClientTransaction* transaction;
            KURL url;
        };
        typedef TQValueList< QueuedRequest > RequestQueue;
        RequestQueue m_requestQueue;

        typedef TQMap< TQString, time_t > BlackList;
        BlackList m_blackList;
        time_t m_suspendTime;
    };
}

#endif // KPAC_PROXYSCOUT_H
