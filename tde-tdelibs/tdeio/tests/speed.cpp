/*
 *  Copyright (C) 2002, 2003 Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2003       David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#include <kdebug.h>
#include <tdeapplication.h>
#include <time.h>
#include "speed.h"
#include <tdeio/job.h>
#include <tdecmdlineargs.h>
#include <tqdir.h>
#include <tdeio/global.h>

using namespace TDEIO;

SpeedTest::SpeedTest( const KURL & url )
    : TQObject(0, "speed")
{
    Job *job = listRecursive( url );
    //Job *job = del( KURL("file:" + TQDir::currentDirPath()) ); DANGEROUS !
    connect(job, TQ_SIGNAL( result( TDEIO::Job*)),
	    TQ_SLOT( finished( TDEIO::Job* ) ));
    /*connect(job, TQ_SIGNAL( entries( TDEIO::Job*, const TDEIO::UDSEntryList&)),
	    TQ_SLOT( entries( TDEIO::Job*, const TDEIO::UDSEntryList&)));
    */
}

void SpeedTest::entries(TDEIO::Job*, const UDSEntryList& list) {

    UDSEntryListConstIterator it=list.begin();
    for (; it != list.end(); ++it) {
      UDSEntry::ConstIterator it2 = (*it).begin();
        for( ; it2 != (*it).end(); it2++ ) {
            if ((*it2).m_uds == UDS_NAME)
              kdDebug() << ( *it2 ).m_str << endl;
        }
    }
}


void SpeedTest::finished(Job*) {
    kdDebug() << "job finished" << endl;
    kapp->quit();
}

static TDECmdLineOptions options[] =
{
  { "+[URL]", "the URL to list", 0 },
  TDECmdLineLastOption
};

int main(int argc, char **argv) {

    TDECmdLineArgs::init( argc, argv, "speedapp", "A TDEIO::listRecursive testing tool", "0.0" );

    TDECmdLineArgs::addCmdLineOptions( options );

    TDEApplication app;

    TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

    KURL url;
    if ( args->count() == 1 )
      url = args->url(0);
    else
      url = "file:" + TQDir::currentDirPath();

    kdDebug() << url.url() << " is probably " << (TDEIO::probably_slow_mounted(url.path()) ? "slow" : "normal") << " mounted\n";
    kdDebug() << url.url() << " is " << (TDEIO::manually_mounted(url.path()) ? "manually" : "system") << " mounted\n";
    TQString mp = TDEIO::findDeviceMountPoint(url.path());
    if (mp.isEmpty()) {
        kdDebug() << "no mount point for device " << url.url() << " found\n";
    } else
        kdDebug() << mp << " is the mount point for device " << url.url() << endl;

    mp = TDEIO::findPathMountPoint(url.path());
    if (mp.isEmpty()) {
        kdDebug() << "no mount point for path " << url.url() << " found\n";
    } else
        kdDebug() << mp << " is the mount point for path " << url.url() << endl;
    // SpeedTest test( url );
    // app.exec();

    mp = TDEIO::findPathMountPoint(url.path());
    if (mp.isEmpty()) {
        kdDebug() << "no mount point for path " << url.url() << " found\n";
    } else
        kdDebug() << mp << " is the mount point for path " << url.url() << endl;
    // SpeedTest test( url );
    // app.exec();

    url.setPath(TQDir::homeDirPath());

    mp = TDEIO::findPathMountPoint(url.path());
    if (mp.isEmpty()) {
        kdDebug() << "no mount point for path " << url.url() << " found\n";
    } else
        kdDebug() << mp << " is the mount point for path " << url.url() << endl;
    // SpeedTest test( url );
    // app.exec();

    mp = TDEIO::findPathMountPoint(url.path());
    if (mp.isEmpty()) {
        kdDebug() << "no mount point for path " << url.url() << " found\n";
    } else
        kdDebug() << mp << " is the mount point for path " << url.url() << endl;
    // SpeedTest test( url );
    // app.exec();

    if ( args->count() == 1 )
      url = args->url(0);
    else
      url = "file:" + TQDir::currentDirPath();

    mp = TDEIO::findPathMountPoint(url.path());
    if (mp.isEmpty()) {
        kdDebug() << "no mount point for path " << url.url() << " found\n";
    } else
        kdDebug() << mp << " is the mount point for path " << url.url() << endl;
    // SpeedTest test( url );
    // app.exec();

}

#include "speed.moc"
