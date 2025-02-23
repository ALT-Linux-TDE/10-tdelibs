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


#include <cstdlib>
#include <cstring>

#include <tqtextcodec.h>

#include <kcharsets.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <tdeio/job.h>

#include "downloader.moc"

namespace KPAC
{
    Downloader::Downloader( TQObject* parent )
        : TQObject( parent )
    {
    }

    void Downloader::download( const KURL& url )
    {
        m_data.resize( 0 );
        m_script = TQString::null;
        m_scriptURL = url;

        TDEIO::TransferJob* job = TDEIO::get( url, false, false );
        connect( job, TQ_SIGNAL( data( TDEIO::Job*, const TQByteArray& ) ),
                 TQ_SLOT( data( TDEIO::Job*, const TQByteArray& ) ) );
        connect( job, TQ_SIGNAL( result( TDEIO::Job* ) ), TQ_SLOT( result( TDEIO::Job* ) ) );
    }

    void Downloader::failed()
    {
        emit result( false );
    }

    void Downloader::setError( const TQString& error )
    {
        m_error = error;
    }

    void Downloader::data( TDEIO::Job*, const TQByteArray& data )
    {
        unsigned offset = m_data.size();
        m_data.resize( offset + data.size() );
        std::memcpy( m_data.data() + offset, data.data(), data.size() );
    }

    void Downloader::result( TDEIO::Job* job )
    {
        if ( !job->error() && !static_cast< TDEIO::TransferJob* >( job )->isErrorPage() )
        {
            bool dummy;
            m_script = TDEGlobal::charsets()->codecForName(
                job->queryMetaData( "charset" ), dummy )->toUnicode( m_data );
            emit result( true );
        }
        else
        {
            if ( job->error() )
                setError( i18n( "Could not download the proxy configuration script:\n%1" )
                              .arg( job->errorString() ) );
            else setError( i18n( "Could not download the proxy configuration script" ) ); // error page
            failed();
        }
    }
}
