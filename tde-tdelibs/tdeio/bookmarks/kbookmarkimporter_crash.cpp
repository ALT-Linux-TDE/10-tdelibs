/* This file is part of the KDE libraries
   Copyright (C) 2002-2003 Alexander Kellett <lypanov@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbookmarkimporter_crash.h"

#include <tdefiledialog.h>
#include <kstringhandler.h>
#include <tdelocale.h>
#include <kdebug.h>
#include <tdeapplication.h>
#include <kstandarddirs.h>
#include <tqfile.h>
#include <tqdir.h>
#include <tqstring.h>
#include <tqtextcodec.h>
#include <dcopclient.h>

#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>

typedef TQMap<TQString, TQString> ViewMap;

// KDE 4.0: remove this BC keeping stub
void TDECrashBookmarkImporter::parseCrashLog( TQString /*filename*/, bool /*del*/ ) 
{
    ;
}

ViewMap TDECrashBookmarkImporterImpl::parseCrashLog_noemit( const TQString & filename, bool del ) 
{
    static const int g_lineLimit = 16*1024;

    TQFile f( filename );
    ViewMap views;

    if ( !f.open( IO_ReadOnly ) )
        return views;

    TQCString s( g_lineLimit );

    TQTextCodec * codec = TQTextCodec::codecForName( "UTF-8" );
    Q_ASSERT( codec );
    if ( !codec ) 
        return views;

    while ( f.readLine( s.data(), g_lineLimit ) >=0 ) 
    {
        if ( s[s.length()-1] != '\n' )
        {
            kdWarning() << "Crash bookmarks contain a line longer than " << g_lineLimit << ". Skipping." << endl;
            continue;
        }
        TQString t = codec->toUnicode( s.stripWhiteSpace() );
        TQRegExp rx( "(.*)\\((.*)\\):(.*)$" );
        rx.setMinimal( true );
        if ( !rx.exactMatch( t ) ) 
            continue;
        if ( rx.cap(1) == "opened" )
            views[rx.cap(2)] = rx.cap(3);
        else if ( rx.cap(1) == "close" )
            views.remove( rx.cap(2) );
    }

    f.close();

    if ( del ) 
        f.remove();

    return views;
}

TQStringList TDECrashBookmarkImporter::getCrashLogs() 
{
    return TDECrashBookmarkImporterImpl::getCrashLogs();
}

TQStringList TDECrashBookmarkImporterImpl::getCrashLogs() 
{
    TQMap<TQString, bool> activeLogs;

    DCOPClient* dcop = kapp->dcopClient();

    QCStringList apps = dcop->registeredApplications();
    for ( QCStringList::Iterator it = apps.begin(); it != apps.end(); ++it ) 
    {
        TQCString &clientId = *it;

        if ( tqstrncmp(clientId, "konqueror", 9) != 0 ) 
            continue;

        TQByteArray data, replyData;
        TQCString replyType;
        TQDataStream arg( data, IO_WriteOnly );

        if ( !dcop->call( clientId.data(), "KonquerorIface", 
                          "crashLogFile()", data, replyType, replyData) ) 
        {
            kdWarning() << "can't find dcop function KonquerorIface::crashLogFile()" << endl;
            continue;
        }

        if ( replyType != "TQString" ) 
            continue;

        TQDataStream reply( replyData, IO_ReadOnly );
        TQString ret;
        reply >> ret;
        activeLogs[ret] = true;
    }

    TQDir d( TDECrashBookmarkImporterImpl().findDefaultLocation() );
    d.setSorting( TQDir::Time );
    d.setFilter( TQDir::Files );
    d.setNameFilter( "konqueror-crash-*.log" );

    const TQFileInfoList *list = d.entryInfoList();
    TQFileInfoListIterator it( *list );

    TQFileInfo *fi;
    TQStringList crashFiles;

    int count = 0;
    for ( ; (( fi = it.current() ) != 0) && (count < 20); ++it, ++count ) 
    {
        bool stillAlive = activeLogs.contains( fi->absFilePath() );
        if ( !stillAlive )
            crashFiles << fi->absFilePath();
    }
    // Delete remaining ones
    for ( ; ( fi = it.current() ) != 0; ++it ) 
    {
        TQFile::remove( fi->absFilePath() );
    }

    return crashFiles;
}

void TDECrashBookmarkImporterImpl::parse() 
{
    TQDict<bool> signatureMap;
    TQStringList crashFiles = TDECrashBookmarkImporterImpl::getCrashLogs();
    int count = 1;
    for ( TQStringList::Iterator it = crashFiles.begin(); it != crashFiles.end(); ++it ) 
    {
        ViewMap views;
        views = parseCrashLog_noemit( *it, m_shouldDelete );
        TQString signature;
        for ( ViewMap::Iterator vit = views.begin(); vit != views.end(); ++vit ) 
            signature += "|"+vit.data();
        if (signatureMap[signature])
        {
            // Duplicate... throw away and skip
            TQFile::remove(*it);
            continue;
        }
            
        signatureMap.insert(signature, (bool *) true); // hack

        int outerFolder = ( crashFiles.count() > 1 ) && (views.count() > 0);
        if ( outerFolder )
            emit newFolder( TQString("Konqueror Window %1").arg(count++), false, "" );
        for ( ViewMap::Iterator vit = views.begin(); vit != views.end(); ++vit ) 
            emit newBookmark( vit.data(), vit.data().latin1(), TQString("") );
        if ( outerFolder )
            emit endFolder();
    }
}

TQString TDECrashBookmarkImporter::crashBookmarksDir() 
{
    static TDECrashBookmarkImporterImpl *p = 0;
    if (!p)
        p = new TDECrashBookmarkImporterImpl;
    return p->findDefaultLocation();
}

void TDECrashBookmarkImporterImpl::setShouldDelete( bool shouldDelete ) 
{
    m_shouldDelete = shouldDelete;
}

void TDECrashBookmarkImporter::parseCrashBookmarks( bool del ) 
{
    TDECrashBookmarkImporterImpl importer;
    importer.setFilename( m_fileName );
    importer.setShouldDelete( del );
    importer.setupSignalForwards( &importer, this );
    importer.parse();
}

TQString TDECrashBookmarkImporterImpl::findDefaultLocation( bool ) const 
{
    return locateLocal( "tmp", "" );
}

#include "kbookmarkimporter_crash.moc"
