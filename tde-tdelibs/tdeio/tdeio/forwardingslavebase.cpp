/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>

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

#include <kdebug.h>
#include <tdeio/job.h>
#include <kmimetype.h>
#include <kprotocolinfo.h>

#include <tqapplication.h>
#include <tqeventloop.h>

#include "forwardingslavebase.h"

namespace TDEIO
{

class ForwardingSlaveBasePrivate
{
};

ForwardingSlaveBase::ForwardingSlaveBase(const TQCString &protocol,
                                         const TQCString &poolSocket,
                                         const TQCString &appSocket)
    : TQObject(), SlaveBase(protocol, poolSocket, appSocket)
{
}

ForwardingSlaveBase::~ForwardingSlaveBase()
{
}

bool ForwardingSlaveBase::internalRewriteURL(const KURL &url, KURL &newURL)
{
    bool result = true;

    if ( url.protocol().ascii()==mProtocol )
    {
        result = rewriteURL(url, newURL);
    }
    else
    {
        newURL = url;
    }

    m_processedURL = newURL;
    m_requestedURL = url;
    return result;
}

void ForwardingSlaveBase::prepareUDSEntry(TDEIO::UDSEntry &entry,
                                          bool listing) const
{
    kdDebug() << "ForwardingSlaveBase::prepareUDSEntry: listing=="
              << listing << endl;

    TQString name;
    KURL url;

    TDEIO::UDSEntry::iterator it = entry.begin();
    TDEIO::UDSEntry::iterator end = entry.end();

    for(; it!=end; ++it)
    {
        KURL new_url = m_requestedURL;

        switch( (*it).m_uds )
        {
        case TDEIO::UDS_NAME:
            name = (*it).m_str;
            kdDebug() << "Name = " << name << endl;
	    break;
        case TDEIO::UDS_URL:
            url = (*it).m_str;
	    if (listing)
            {
                new_url.addPath(url.fileName());
            }
            (*it).m_str = new_url.url();
            kdDebug() << "URL = " << url << endl;
            kdDebug() << "New URL = " << (*it).m_str << endl;
            break;
        }
    }

    if ( m_processedURL.isLocalFile() )
    {
        KURL new_url = m_processedURL;
        if (listing)
        {
            new_url.addPath( name );
        }

        TDEIO::UDSAtom atom;
        atom.m_uds = TDEIO::UDS_LOCAL_PATH;
        atom.m_long = 0;
        atom.m_str = new_url.path();
        entry.append(atom);
    }
}

void ForwardingSlaveBase::get(const KURL &url)
{
    kdDebug() << "ForwardingSlaveBase::get: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        TDEIO::TransferJob *job = TDEIO::get(new_url, false, false);
        connectTransferJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::put(const KURL &url, int permissions,
                              bool overwrite, bool resume )
{
    kdDebug() << "ForwardingSlaveBase::put: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        TDEIO::TransferJob *job = TDEIO::put(new_url, permissions, overwrite,
                                         resume, false);
        connectTransferJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::stat(const KURL &url)
{
    kdDebug() << "ForwardingSlaveBase::stat: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        TDEIO::SimpleJob *job = TDEIO::stat(new_url, false);
        connectSimpleJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::mimetype(const KURL &url)
{
    kdDebug() << "ForwardingSlaveBase::mimetype: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        TDEIO::TransferJob *job = TDEIO::mimetype(new_url, false);
        connectTransferJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::listDir(const KURL &url)
{
    kdDebug() << "ForwardingSlaveBase::listDir: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        TDEIO::ListJob *job = TDEIO::listDir(new_url, false);
        connectListJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::mkdir(const KURL &url, int permissions)
{
    kdDebug() << "ForwardingSlaveBase::mkdir: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        TDEIO::SimpleJob *job = TDEIO::mkdir(new_url, permissions);
        connectSimpleJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::rename(const KURL &src, const KURL &dest,
                                 bool overwrite)
{
    kdDebug() << "ForwardingSlaveBase::rename: " << src << ", " << dest << endl;

    KURL new_src, new_dest;
    if ( internalRewriteURL(src, new_src) && internalRewriteURL(dest, new_dest) )
    {
        TDEIO::Job *job = TDEIO::rename(new_src, new_dest, overwrite);
        connectJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::symlink(const TQString &target, const KURL &dest,
                                  bool overwrite)
{
    kdDebug() << "ForwardingSlaveBase::symlink: " << target << ", " << dest << endl;

    KURL new_dest;
    if ( internalRewriteURL(dest, new_dest) )
    {
        TDEIO::SimpleJob *job = TDEIO::symlink(target, new_dest, overwrite, false);
        connectSimpleJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::chmod(const KURL &url, int permissions)
{
    kdDebug() << "ForwardingSlaveBase::chmod: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        TDEIO::SimpleJob *job = TDEIO::chmod(new_url, permissions);
        connectSimpleJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::copy(const KURL &src, const KURL &dest,
                               int permissions, bool overwrite)
{
    kdDebug() << "ForwardingSlaveBase::copy: " << src << ", " << dest << endl;

    KURL new_src, new_dest;
    if ( internalRewriteURL(src, new_src) && internalRewriteURL(dest, new_dest) )
    {
        TDEIO::Job *job = TDEIO::file_copy(new_src, new_dest, permissions,
                                       overwrite, false);
        connectJob(job);

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::del(const KURL &url, bool isfile)
{
    kdDebug() << "ForwardingSlaveBase::del: " << url << endl;

    KURL new_url;
    if ( internalRewriteURL(url, new_url) )
    {
        if (isfile)
        {
            TDEIO::DeleteJob *job = TDEIO::del(new_url, false, false);
            connectJob(job);
        }
        else
        {
            TDEIO::SimpleJob *job = TDEIO::rmdir(new_url);
            connectSimpleJob(job);
        }

        tqApp->eventLoop()->enterLoop();
    }
}

void ForwardingSlaveBase::localURL(const KURL& remoteURL)
{
    kdDebug() << "ForwardingSlaveBase::localURL: " << remoteURL << endl;

    KURL new_url;
    if ( internalRewriteURL(remoteURL, new_url) )
    {
        TDEIO::LocalURLJob *job = TDEIO::localURL(new_url);
        connectLocalURLJob(job);

        tqApp->eventLoop()->enterLoop();
    }
    else
    {
        // Let the slave base emit the required signals
        SlaveBase::localURL(remoteURL);
    }
}

//////////////////////////////////////////////////////////////////////////////

void ForwardingSlaveBase::connectJob(TDEIO::Job *job)
{
    // We will forward the warning message, no need to let the job
    // display it itself
    job->setInteractive(false);

    // Forward metadata (e.g. modification time for put())
    job->setMetaData( allMetaData() );
#if 0 // debug code
    kdDebug() << k_funcinfo << "transferring metadata:" << endl;
    const MetaData md = allMetaData();
    for ( MetaData::const_iterator it = md.begin(); it != md.end(); ++it )
        kdDebug() << it.key() << " = " << it.data() << endl;
#endif

    connect( job, TQ_SIGNAL( result(TDEIO::Job *) ),
             this, TQ_SLOT( slotResult(TDEIO::Job *) ) );
    connect( job, TQ_SIGNAL( warning(TDEIO::Job *, const TQString &) ),
             this, TQ_SLOT( slotWarning(TDEIO::Job *, const TQString &) ) );
    connect( job, TQ_SIGNAL( infoMessage(TDEIO::Job *, const TQString &) ),
             this, TQ_SLOT( slotInfoMessage(TDEIO::Job *, const TQString &) ) );
    connect( job, TQ_SIGNAL( totalSize(TDEIO::Job *, TDEIO::filesize_t) ),
             this, TQ_SLOT( slotTotalSize(TDEIO::Job *, TDEIO::filesize_t) ) );
    connect( job, TQ_SIGNAL( processedSize(TDEIO::Job *, TDEIO::filesize_t) ),
             this, TQ_SLOT( slotProcessedSize(TDEIO::Job *, TDEIO::filesize_t) ) );
    connect( job, TQ_SIGNAL( speed(TDEIO::Job *, unsigned long) ),
             this, TQ_SLOT( slotSpeed(TDEIO::Job *, unsigned long) ) );
}

void ForwardingSlaveBase::connectSimpleJob(TDEIO::SimpleJob *job)
{
    connectJob(job);
    connect( job, TQ_SIGNAL( redirection(TDEIO::Job *, const KURL &) ),
             this, TQ_SLOT( slotRedirection(TDEIO::Job *, const KURL &) ) );
}

void ForwardingSlaveBase::connectListJob(TDEIO::ListJob *job)
{
    connectSimpleJob(job);
    connect( job, TQ_SIGNAL( entries(TDEIO::Job *, const TDEIO::UDSEntryList &) ),
             this, TQ_SLOT( slotEntries(TDEIO::Job *, const TDEIO::UDSEntryList &) ) );
}

void ForwardingSlaveBase::connectTransferJob(TDEIO::TransferJob *job)
{
    connectSimpleJob(job);
    connect( job, TQ_SIGNAL( data(TDEIO::Job *, const TQByteArray &) ),
             this, TQ_SLOT( slotData(TDEIO::Job *, const TQByteArray &) ) );
    connect( job, TQ_SIGNAL( dataReq(TDEIO::Job *, TQByteArray &) ),
             this, TQ_SLOT( slotDataReq(TDEIO::Job *, TQByteArray &) ) );
    connect( job, TQ_SIGNAL( mimetype(TDEIO::Job *, const TQString &) ),
             this, TQ_SLOT( slotMimetype(TDEIO::Job *, const TQString &) ) );
    connect( job, TQ_SIGNAL( canResume(TDEIO::Job *, TDEIO::filesize_t) ),
             this, TQ_SLOT( slotCanResume(TDEIO::Job *, TDEIO::filesize_t) ) );
}

void ForwardingSlaveBase::connectLocalURLJob(TDEIO::LocalURLJob *job)
{
    connectJob(job);
    connect( job, TQ_SIGNAL( localURL(TDEIO::LocalURLJob *, const KURL&, bool) ),
             this, TQ_SLOT( slotLocalURL(TDEIO::LocalURLJob *, const KURL&, bool) ) );
}

//////////////////////////////////////////////////////////////////////////////

void ForwardingSlaveBase::slotResult(TDEIO::Job *job)
{
    if ( job->error() != 0)
    {
        error( job->error(), job->errorText() );
    }
    else
    {
        TDEIO::StatJob *stat_job = dynamic_cast<TDEIO::StatJob *>(job);
        if ( stat_job!=0L )
        {
            TDEIO::UDSEntry entry = stat_job->statResult();
	    prepareUDSEntry(entry);
            statEntry( entry );
        }
        finished();
    }

    tqApp->eventLoop()->exitLoop();
}

void ForwardingSlaveBase::slotWarning(TDEIO::Job* /*job*/, const TQString &msg)
{
    warning(msg);
}

void ForwardingSlaveBase::slotInfoMessage(TDEIO::Job* /*job*/, const TQString &msg)
{
    infoMessage(msg);
}

void ForwardingSlaveBase::slotTotalSize(TDEIO::Job* /*job*/, TDEIO::filesize_t size)
{
    totalSize(size);
}

void ForwardingSlaveBase::slotProcessedSize(TDEIO::Job* /*job*/, TDEIO::filesize_t size)
{
    processedSize(size);
}

void ForwardingSlaveBase::slotSpeed(TDEIO::Job* /*job*/, unsigned long bytesPerSecond)
{
    speed(bytesPerSecond);
}

void ForwardingSlaveBase::slotRedirection(TDEIO::Job *job, const KURL &url)
{
    redirection(url);

    // We've been redirected stop everything.
    job->kill( true );
    finished();

    tqApp->eventLoop()->exitLoop();
}

void ForwardingSlaveBase::slotEntries(TDEIO::Job* /*job*/,
                                      const TDEIO::UDSEntryList &entries)
{
    TDEIO::UDSEntryList final_entries = entries;

    TDEIO::UDSEntryList::iterator it = final_entries.begin();
    TDEIO::UDSEntryList::iterator end = final_entries.end();

    for(; it!=end; ++it)
    {
        prepareUDSEntry(*it, true);
    }

    listEntries( final_entries );
}

void ForwardingSlaveBase::slotData(TDEIO::Job* /*job*/, const TQByteArray &d)
{
    data(d);
}

void ForwardingSlaveBase::slotDataReq(TDEIO::Job* /*job*/, TQByteArray &data)
{
    dataReq();
    readData(data);
}

void ForwardingSlaveBase::slotMimetype (TDEIO::Job* /*job*/, const TQString &type)
{
    mimeType(type);
}

void ForwardingSlaveBase::slotCanResume (TDEIO::Job* /*job*/, TDEIO::filesize_t offset)
{
    canResume(offset);
}

void ForwardingSlaveBase::slotLocalURL(TDEIO::LocalURLJob *, const KURL& url, bool)
{
    SlaveBase::localURL(url);
}

}

#include "forwardingslavebase.moc"

