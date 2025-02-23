/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>

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

#include "kdirsize.h"
#include <kdebug.h>
#include <tdeglobal.h>
#include <tqapplication.h>
#include <tqtimer.h>
#include <config-tdefile.h>

using namespace TDEIO;

KDirSize::KDirSize( const KURL & directory )
    : TDEIO::Job(false /*No GUI*/), m_bAsync(true), m_totalSize(0L), m_totalFiles(0L), m_totalSubdirs(0L)
{
    startNextJob( directory );
}

KDirSize::KDirSize( const KFileItemList & lstItems )
    : TDEIO::Job(false /*No GUI*/), m_bAsync(true), m_totalSize(0L), m_totalFiles(0L), m_totalSubdirs(0L), m_lstItems(lstItems)
{
    TQTimer::singleShot( 0, this, TQ_SLOT(processList()) );
}

void KDirSize::processList()
{
    while (!m_lstItems.isEmpty())
    {
        KFileItem * item = m_lstItems.first();
        m_lstItems.removeFirst();
	if ( !item->isLink() )
	{
            if ( item->isDir() )
            {
                kdDebug(tdefile_area) << "KDirSize::processList dir -> listing" << endl;
                KURL url = item->url();
                startNextJob( url );
                return; // we'll come back later, when this one's finished
            }
            else
            {
                m_totalSize += item->size();
// no long long with kdDebug()
//            kdDebug(tdefile_area) << "KDirSize::processList file -> " << m_totalSize << endl;
            }
	}
    }
    kdDebug(tdefile_area) << "KDirSize::processList finished" << endl;
    if ( !m_bAsync )
        tqApp->exit_loop();
    emitResult();
}

void KDirSize::startNextJob( const KURL & url )
{
    TDEIO::ListJob * listJob = TDEIO::listRecursive( url, false /* no GUI */ );
    connect( listJob, TQ_SIGNAL(entries( TDEIO::Job *,
                                      const TDEIO::UDSEntryList& )),
             TQ_SLOT( slotEntries( TDEIO::Job*,
                                const TDEIO::UDSEntryList& )));
    addSubjob( listJob );
}

void KDirSize::slotEntries( TDEIO::Job*, const TDEIO::UDSEntryList & list )
{
    static const TQString& dot = TDEGlobal::staticQString( "." );
    static const TQString& dotdot = TDEGlobal::staticQString( ".." );
    TDEIO::UDSEntryListConstIterator it = list.begin();
    TDEIO::UDSEntryListConstIterator end = list.end();
    for (; it != end; ++it) {
        TDEIO::UDSEntry::ConstIterator it2 = (*it).begin();
        TDEIO::filesize_t size = 0;
        bool isLink = false;
        bool isDir = false;
        TQString name;
        for( ; it2 != (*it).end(); it2++ ) {
          switch( (*it2).m_uds ) {
            case TDEIO::UDS_NAME:
              name = (*it2).m_str;
              break;
            case TDEIO::UDS_LINK_DEST:
              isLink = !(*it2).m_str.isEmpty();
              break;
            case TDEIO::UDS_SIZE:
              size = ((*it2).m_long);
              break;
            case TDEIO::UDS_FILE_TYPE:
              isDir = S_ISDIR((*it2).m_long);
              break;
            default:
              break;
          }
        }
        if ( name == dot )
            m_totalSize += size;
        else if ( name != dotdot )
        {
            if (!isLink)
              m_totalSize += size;
            if (!isDir)
              m_totalFiles++;
            else
              m_totalSubdirs++;
            //kdDebug(tdefile_area) << name << ":" << size << endl;
        }
    }
}

//static
KDirSize * KDirSize::dirSizeJob( const KURL & directory )
{
    return new KDirSize( directory ); // useless - but consistent with other jobs
}

//static
KDirSize * KDirSize::dirSizeJob( const KFileItemList & lstItems )
{
    return new KDirSize( lstItems );
}

//static
TDEIO::filesize_t KDirSize::dirSize( const KURL & directory )
{
    KDirSize * dirSize = dirSizeJob( directory );
    dirSize->setSync();
    tqApp->enter_loop();
    return dirSize->totalSize();
}


void KDirSize::slotResult( TDEIO::Job * job )
{
    kdDebug(tdefile_area) << " KDirSize::slotResult( TDEIO::Job * job ) m_lstItems:" << m_lstItems.count() << endl;
    if ( !m_lstItems.isEmpty() )
    {
        subjobs.remove(job); // Remove job, but don't kill this job.
        processList();
    }
    else
    {
        if ( !m_bAsync )
            tqApp->exit_loop();
        TDEIO::Job::slotResult( job );
    }
}

void KDirSize::virtual_hook( int id, void* data )
{ TDEIO::Job::virtual_hook( id, data ); }

#include "kdirsize.moc"
