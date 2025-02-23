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


#ifndef KPAC_DOWNLOADER_H
#define KPAC_DOWNLOADER_H

#include <tqobject.h>

#include <kurl.h>

namespace TDEIO { class Job; }

namespace KPAC
{
    class Downloader : public TQObject
    {
        TQ_OBJECT
    public:
        Downloader( TQObject* );

        void download( const KURL& );
        const KURL& scriptURL() { return m_scriptURL; }
        const TQString& script() { return m_script; }
        const TQString& error() { return m_error; }

    signals:
        void result( bool );

    protected:
        virtual void failed();
        void setError( const TQString& );

    private slots:
        void data( TDEIO::Job*, const TQByteArray& );
        void result( TDEIO::Job* );

    private:
        TQByteArray m_data;
        KURL m_scriptURL;
        TQString m_script;
        TQString m_error;
    };
}

#endif // KPAC_DOWNLOADER_H
