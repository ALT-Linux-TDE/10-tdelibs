/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Richard Moore <rich@kde.org>
 *               2000 Wynn Wilkes <wynnw@caldera.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kjavadownloader.h"
#include "kjavaappletserver.h"

#include <kurl.h>
#include <tdeio/job.h>
#include <tdeio/jobclasses.h>
#include <kdebug.h>
#include <tqfile.h>

static const int DATA = 0;
static const int FINISHED = 1;
static const int ERRORCODE = 2;
static const int HEADERS = 3;
static const int REDIRECT = 4;
static const int MIMETYPE = 5;
static const int CONNECTED = 6;
static const int REQUESTDATA = 7;

static const int KJAS_STOP = 0;
static const int KJAS_HOLD = 1;
static const int KJAS_RESUME = 2;

KJavaTDEIOJob::~KJavaTDEIOJob() {}

void KJavaTDEIOJob::data( const TQByteArray& )
{
    kdError(6100) << "Job id mixup" << endl;
}

//-----------------------------------------------------------------------------

class KJavaDownloaderPrivate
{
friend class KJavaDownloader;
public:
    KJavaDownloaderPrivate() : responseCode(0), isfirstdata(true) {}
    ~KJavaDownloaderPrivate()
    {
        delete url;
        if (job) job->kill(); // TDEIO::Job::kill deletes itself
    }
private:
    int               loaderID;
    KURL*             url;
    TQByteArray        file;
    TDEIO::TransferJob* job;
    int               responseCode;
    bool              isfirstdata;
};


KJavaDownloader::KJavaDownloader( int ID, const TQString& url )
{
    kdDebug(6100) << "KJavaDownloader(" << ID << ") = " << url << endl;

    d = new KJavaDownloaderPrivate;

    d->loaderID = ID;
    d->url = new KURL( url );

    d->job = TDEIO::get( *d->url, false, false );
    d->job->addMetaData("PropagateHttpHeader", "true");
    connect( d->job,  TQ_SIGNAL(data( TDEIO::Job*, const TQByteArray& )),
             this,    TQ_SLOT(slotData( TDEIO::Job*, const TQByteArray& )) );
    connect( d->job, TQ_SIGNAL(connected(TDEIO::Job*)),
             this, TQ_SLOT(slotConnected(TDEIO::Job*)));
    connect( d->job, TQ_SIGNAL(mimetype(TDEIO::Job*, const TQString&)),
             this, TQ_SLOT(slotMimetype(TDEIO::Job*, const TQString&)));
    connect( d->job, TQ_SIGNAL(result(TDEIO::Job*)),
             this,   TQ_SLOT(slotResult(TDEIO::Job*)) );
}

KJavaDownloader::~KJavaDownloader()
{
    delete d;
}

void KJavaDownloader::slotData( TDEIO::Job*, const TQByteArray& qb )
{
    //kdDebug(6100) << "slotData(" << d->loaderID << ")" << endl;

    KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
    if (d->isfirstdata) {
        TQString headers = d->job->queryMetaData("HTTP-Headers");
        if (!headers.isEmpty()) {
            d->file.resize( headers.length() );
            memcpy( d->file.data(), headers.ascii(), headers.length() );
            server->sendURLData( d->loaderID, HEADERS, d->file );
            d->file.resize( 0 );
        }
        d->isfirstdata = false;
    }
    if ( qb.size() )
        server->sendURLData( d->loaderID, DATA, qb );
    KJavaAppletServer::freeJavaServer();
}

void KJavaDownloader::slotConnected(TDEIO::Job*)
{
    kdDebug(6100) << "slave connected" << endl;
    d->responseCode = d->job->error();
}

void KJavaDownloader::slotMimetype(TDEIO::Job*, const TQString & type) {
    kdDebug(6100) << "slave mimetype " << type << endl;
}

void KJavaDownloader::slotResult( TDEIO::Job* )
{
    kdDebug(6100) << "slotResult(" << d->loaderID << ")" << endl;

    KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
    if( d->job->error())
    {
        kdDebug(6100) << "slave had an error = " << d->job->errorString() << endl;
        int code = d->job->error();
        if (!code)
            code = 404;
        TQString codestr = TQString::number(code);
        d->file.resize(codestr.length());
        memcpy( d->file.data(), codestr.ascii(), codestr.length() );
        kdDebug(6100) << "slave had an error = " << code << endl;

        server->sendURLData( d->loaderID, ERRORCODE, d->file );
        d->file.resize( 0 );
    }
    else
    {
        server->sendURLData( d->loaderID, FINISHED, d->file );
    }
    d->job = 0L; // signal TDEIO::Job::result deletes itself
    server->removeDataJob( d->loaderID ); // will delete this
    KJavaAppletServer::freeJavaServer();
}

void KJavaDownloader::jobCommand( int cmd )
{
    if (!d->job) return;
    switch (cmd) {
        case KJAS_STOP: {
            kdDebug(6100) << "jobCommand(" << d->loaderID << ") stop" << endl;
            d->job->kill();
            d->job = 0L; // TDEIO::Job::kill deletes itself
            KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
            server->removeDataJob( d->loaderID ); // will delete this
            KJavaAppletServer::freeJavaServer();
            break;
        }
        case KJAS_HOLD:
            kdDebug(6100) << "jobCommand(" << d->loaderID << ") hold" << endl;
            d->job->suspend();
            break;
        case KJAS_RESUME:
            kdDebug(6100) << "jobCommand(" << d->loaderID << ") resume" << endl;
            d->job->resume();
            break;
    }
}

//-----------------------------------------------------------------------------

class KJavaUploaderPrivate
{
public:
    KJavaUploaderPrivate() {}
    ~KJavaUploaderPrivate()
    {
        delete url;
        if (job) job->kill(); // TDEIO::Job::kill deletes itself
    }
    int               loaderID;
    KURL*             url;
    TQByteArray        file;
    TDEIO::TransferJob* job;
    bool              finished;
};

KJavaUploader::KJavaUploader( int ID, const TQString& url )
{
    kdDebug(6100) << "KJavaUploader(" << ID << ") = " << url << endl;

    d = new KJavaUploaderPrivate;

    d->loaderID = ID;
    d->url = new KURL( url );
    d->job = 0L;
    d->finished = false;
}

void KJavaUploader::start()
{
    kdDebug(6100) << "KJavaUploader::start(" << d->loaderID << ")" << endl;
    KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
    // create a suspended job
    d->job = TDEIO::put( *d->url, -1, false, false, false );
    d->job->suspend();
    connect( d->job, TQ_SIGNAL(dataReq( TDEIO::Job*, TQByteArray& )),
            this,   TQ_SLOT(slotDataRequest( TDEIO::Job*, TQByteArray& )) );
    connect( d->job, TQ_SIGNAL(result(TDEIO::Job*)),
            this,   TQ_SLOT(slotResult(TDEIO::Job*)) );
    server->sendURLData( d->loaderID, CONNECTED, d->file );
    KJavaAppletServer::freeJavaServer();
}

KJavaUploader::~KJavaUploader()
{
    delete d;
}

void KJavaUploader::slotDataRequest( TDEIO::Job*, TQByteArray& qb )
{
    // send our data and suspend
    kdDebug(6100) << "slotDataRequest(" << d->loaderID << ") finished:" << d->finished << endl;
    qb.resize( d->file.size() );
    KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
    if (d->file.size() == 0) {
        d->job = 0L; // eof, job deletes itself
        server->removeDataJob( d->loaderID ); // will delete this
    } else {
        memcpy( qb.data(), d->file.data(), d->file.size() );
        d->file.resize( 0 );
	if (!d->finished) {
            server->sendURLData( d->loaderID, REQUESTDATA, d->file );
            d->job->suspend();
        }
    }
    KJavaAppletServer::freeJavaServer();
}

void KJavaUploader::data( const TQByteArray& qb )
{
    kdDebug(6100) << "KJavaUploader::data(" << d->loaderID << ")" << endl;
    d->file.resize( qb.size() );
    memcpy( d->file.data(), qb.data(), qb.size() );
    d->job->resume();
}

void KJavaUploader::slotResult( TDEIO::Job* )
{
    kdDebug(6100) << "slotResult(" << d->loaderID << ") job:" << d->job << endl;

    if (!d->job)
        return;
    KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
    if (d->job->error())
    {
        int code = d->job->error();
        TQString codestr = TQString::number(code);
        d->file.resize(codestr.length());
        memcpy( d->file.data(), codestr.ascii(), codestr.length() );
        kdDebug(6100) << "slave had an error " << code <<  ": " << d->job->errorString() << endl;

        server->sendURLData( d->loaderID, ERRORCODE, d->file );
        d->file.resize( 0 );
    }
    else // shouldn't come here
        kdError(6100) << "slotResult(" << d->loaderID << ") job:" << d->job << endl;
    d->job = 0L; // signal TDEIO::Job::result deletes itself
    server->removeDataJob( d->loaderID ); // will delete this
    KJavaAppletServer::freeJavaServer();
}

void KJavaUploader::jobCommand( int cmd )
{
    if (!d->job) return;
    switch (cmd) {
        case KJAS_STOP: {
            kdDebug(6100) << "jobCommand(" << d->loaderID << ") stop" << endl;
	    d->finished = true;
            if (d->job->isSuspended())
                d->job->resume();
            break;
        }
    }
}

#include "kjavadownloader.moc"
