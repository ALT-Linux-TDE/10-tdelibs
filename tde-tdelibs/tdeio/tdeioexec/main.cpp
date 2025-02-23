/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C)       2000 David Faure <faure@kde.org>
   Copyright (C)       2001 Waldo Bastian <bastian@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <tqfile.h>

#include <tdeapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <tdemessagebox.h>
#include <tdeio/job.h>
#include <krun.h>
#include <tdeio/netaccess.h>
#include <kprocess.h>
#include <kservice.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>
#include <tdestartupinfo.h>
#include <kshell.h>
#include <kde_file.h>


#include "main.h"


static const char description[] =
        I18N_NOOP("TDEIO Exec - Opens remote files, watches modifications, asks for upload");

static TDECmdLineOptions options[] =
{
   { "tempfiles", I18N_NOOP("Treat URLs as local files and delete them afterwards"), 0 },
   { "suggestedfilename <file name>", I18N_NOOP("Suggested file name for the downloaded file"), 0 },
   { "+command", I18N_NOOP("Command to execute"), 0 },
   { "+[URLs]", I18N_NOOP("URL(s) or local file(s) used for 'command'"), 0 },
   TDECmdLineLastOption
};


int jobCounter = 0;

TQPtrList<TDEIO::Job>* jobList = 0L;

KIOExec::KIOExec()
{
    jobList = new TQPtrList<TDEIO::Job>;
    jobList->setAutoDelete( false ); // jobs autodelete themselves

    TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
    if (args->count() < 1)
        TDECmdLineArgs::usage(i18n("'command' expected.\n"));

    tempfiles = args->isSet("tempfiles");
    if ( args->isSet( "suggestedfilename" ) )
        suggestedFileName = TQString::fromLocal8Bit( args->getOption( "suggestedfilename" ) );
    expectedCounter = 0;
    command = args->arg(0);
    kdDebug() << "command=" << command << endl;

    for ( int i = 1; i < args->count(); i++ )
    {
        KURL url = args->url(i);
        // we need to map system:/ etc to make sure we get this right
        url = TDEIO::NetAccess::mostLocalURL( url, 0 );

        //kdDebug() << "url=" << url.url() << " filename=" << url.fileName() << endl;
        // A local file, not an URL ?
        // => It is not encoded and not shell escaped, too.
        if ( url.isLocalFile() )
        {
            fileInfo file;
            file.path = url.path();
            file.url = url;
            fileList.append(file);
        }
        // It is an URL
        else
        {
            if ( !url.isValid() )
                KMessageBox::error( 0L, i18n( "The URL %1\nis malformed" ).arg( url.url() ) );
            else if ( tempfiles )
                KMessageBox::error( 0L, i18n( "Remote URL %1\nnot allowed with --tempfiles switch" ).arg( url.url() ) );
            else
            // We must fetch the file
            {
                TQString fileName = TDEIO::encodeFileName( url.fileName() );
                if ( !suggestedFileName.isEmpty() )
                    fileName = suggestedFileName;
                // Build the destination filename, in ~/.trinity/cache-*/krun/
                // Unlike KDE-1.1, we put the filename at the end so that the extension is kept
                // (Some programs rely on it)
                TQString tmp = TDEGlobal::dirs()->saveLocation( "cache", "krun/" ) +
                              TQString("%1.%2.%3").arg(getpid()).arg(jobCounter++).arg(fileName);
                fileInfo file;
                file.path = tmp;
                file.url = url;
                fileList.append(file);

                expectedCounter++;
                KURL dest;
                dest.setPath( tmp );
                kdDebug() << "Copying " << url.prettyURL() << " to " << dest << endl;
                TDEIO::Job *job = TDEIO::file_copy( url, dest );
                jobList->append( job );

                connect( job, TQ_SIGNAL( result( TDEIO::Job * ) ), TQ_SLOT( slotResult( TDEIO::Job * ) ) );
            }
        }
    }
    args->clear();

    if ( tempfiles ) {
        // #113991
        TQTimer::singleShot( 0, this, TQ_SLOT( slotRunApp() ) );
        //slotRunApp(); // does not return
        return;
    }

    counter = 0;
    if ( counter == expectedCounter )
        slotResult( 0L );
}

void KIOExec::slotResult( TDEIO::Job * job )
{
    if (job && job->error())
    {
        // That error dialog would be queued, i.e. not immediate...
        //job->showErrorDialog();
        if ( (job->error() != TDEIO::ERR_USER_CANCELED) )
            KMessageBox::error( 0L, job->errorString() );

        TQString path = static_cast<TDEIO::FileCopyJob*>(job)->destURL().path();

        TQValueList<fileInfo>::Iterator it = fileList.begin();
        for(;it != fileList.end(); ++it)
        {
           if ((*it).path == path)
              break;
        }

        if ( it != fileList.end() )
           fileList.remove( it );
        else
           kdDebug() <<  static_cast<TDEIO::FileCopyJob*>(job)->destURL().path() << " not found in list" << endl;
    }

    counter++;

    if ( counter < expectedCounter )
        return;

    kdDebug() << "All files downloaded, will call slotRunApp shortly" << endl;
    // We know we can run the app now - but let's finish the job properly first.
    TQTimer::singleShot( 0, this, TQ_SLOT( slotRunApp() ) );

    jobList->clear();
}

void KIOExec::slotRunApp()
{
    if ( fileList.isEmpty() ) {
        kdDebug() << k_funcinfo << "No files downloaded -> exiting" << endl;
        exit(1);
    }

    KService service("dummy", command, TQString::null);

    KURL::List list;
    // Store modification times
    TQValueList<fileInfo>::Iterator it = fileList.begin();
    for ( ; it != fileList.end() ; ++it )
    {
        KDE_struct_stat buff;
        (*it).time = KDE_stat( TQFile::encodeName((*it).path), &buff ) ? 0 : buff.st_mtime;
        KURL url;
        url.setPath((*it).path);
        list << url;
    }

    TQStringList params = KRun::processDesktopExec(service, list, false /*no shell*/);

    kdDebug() << "EXEC " << KShell::joinArgs( params ) << endl;

#ifdef TQ_WS_X11
    // propagate the startup indentification to the started process
    TDEStartupInfoId id;
    id.initId( kapp->startupId());
    id.setupStartupEnv();
#endif

    TDEProcess proc;
    proc << params;
    proc.start( TDEProcess::Block );

#ifdef TQ_WS_X11
    TDEStartupInfo::resetStartupEnv();
#endif

    kdDebug() << "EXEC done" << endl;

    // Test whether one of the files changed
    it = fileList.begin();
    for( ;it != fileList.end(); ++it )
    {
        KDE_struct_stat buff;
        TQString src = (*it).path;
        KURL dest = (*it).url;
        if ( (KDE_stat( TQFile::encodeName(src), &buff ) == 0) &&
             ((*it).time != buff.st_mtime) )
        {
            if ( tempfiles )
            {
                if ( KMessageBox::questionYesNo( 0L,
                                                 i18n( "The supposedly temporary file\n%1\nhas been modified.\nDo you still want to delete it?" ).arg(dest.prettyURL()),
                                                 i18n( "File Changed" ), KStdGuiItem::del(), i18n("Do Not Delete") ) != KMessageBox::Yes )
                    continue; // don't delete the temp file
            }
            else if ( ! dest.isLocalFile() )  // no upload when it's already a local file
            {
                if ( KMessageBox::questionYesNo( 0L,
                                                 i18n( "The file\n%1\nhas been modified.\nDo you want to upload the changes?" ).arg(dest.prettyURL()),
                                                 i18n( "File Changed" ), i18n("Upload"), i18n("Do Not Upload") ) == KMessageBox::Yes )
                {
                    kdDebug() << TQString(TQString("src='%1'  dest='%2'").arg(src).arg(dest.url())).ascii() << endl;
                    // Do it the synchronous way.
                    if ( !TDEIO::NetAccess::upload( src, dest, 0 ) )
                    {
                        KMessageBox::error( 0L, TDEIO::NetAccess::lastErrorString() );
                        continue; // don't delete the temp file
                    }
                }
            }
        }

        if ( !dest.isLocalFile() || tempfiles ) {
            // Wait for a reasonable time so that even if the application forks on startup (like OOo or amarok)
            // it will have time to start up and read the file before it gets deleted. #130709.
            kdDebug() << "sleeping..." << endl;
            sleep(180); // 3 mn
            kdDebug() << "about to delete " << src << endl;
            unlink( TQFile::encodeName(src) );
        }
    }

    //kapp->quit(); not efficient enough
    exit(0);
}

int main( int argc, char **argv )
{
    TDEAboutData aboutData( "tdeioexec", I18N_NOOP("KIOExec"),
        VERSION, description, TDEAboutData::License_GPL,
        "(c) 1998-2000,2003 The KFM/Konqueror Developers");
    aboutData.addAuthor("David Faure",0, "faure@kde.org");
    aboutData.addAuthor("Stephan Kulow",0, "coolo@kde.org");
    aboutData.addAuthor("Bernhard Rosenkraenzer",0, "bero@arklinux.org");
    aboutData.addAuthor("Waldo Bastian",0, "bastian@kde.org");
    aboutData.addAuthor("Oswald Buddenhagen",0, "ossi@kde.org");

    TDECmdLineArgs::init( argc, argv, &aboutData );
    TDECmdLineArgs::addCmdLineOptions( options );

    TDEApplication app;

    KIOExec exec;

    kdDebug() << "Constructor returned..." << endl;
    return app.exec();
}

#include "main.moc"
