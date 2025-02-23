/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Wynn Wilkes <wynnw@caldera.com>
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


#ifndef KJAVADOWNLOADER_H
#define KJAVADOWNLOADER_H

#include <tqobject.h>

/**
 * @short A class for handling downloads from TDEIO
 *
 * This class handles a TDEIO::get job and passes the data
 * back to the AppletServer.
 *
 * @author Wynn Wilkes, wynnw@calderasystems.com
 */

namespace TDEIO {
    class Job;
}

class KJavaDownloaderPrivate;
class KJavaUploaderPrivate;

class KJavaTDEIOJob : public TQObject
{
TQ_OBJECT
public:
    virtual ~KJavaTDEIOJob();
    virtual void jobCommand( int cmd ) = 0;
    virtual void data( const TQByteArray& qb );
};

class KJavaDownloader : public KJavaTDEIOJob
{
TQ_OBJECT

public:
    KJavaDownloader( int ID, const TQString& url );
    ~KJavaDownloader();

    virtual void jobCommand( int cmd );
protected slots:
    void slotData( TDEIO::Job*, const TQByteArray& );
    void slotConnected( TDEIO::Job* );
    void slotMimetype( TDEIO::Job*, const TQString& );
    void slotResult( TDEIO::Job* );

private:
    KJavaDownloaderPrivate* d;

};

class KJavaUploader : public KJavaTDEIOJob
{
TQ_OBJECT

public:
    KJavaUploader( int ID, const TQString& url );
    ~KJavaUploader();

    virtual void jobCommand( int cmd );
    virtual void data( const TQByteArray& qb );
    void start();
protected slots:
    void slotDataRequest( TDEIO::Job*, TQByteArray& );
    void slotResult( TDEIO::Job* );
private:
    KJavaUploaderPrivate* d;

};
#endif
