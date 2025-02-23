/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include "kbookmarkimporter_kde1.h"
#include <tdefiledialog.h>
#include <kstringhandler.h>
#include <tdelocale.h>
#include <kdebug.h>
#include <kcharsets.h>
#include <tqtextcodec.h>

#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <assert.h>

////////////////////

void KBookmarkImporter::import( const TQString & path )
{
    TQDomElement elem = m_pDoc->documentElement();
    Q_ASSERT(!elem.isNull());
    scanIntern( elem, path );
}

void KBookmarkImporter::scanIntern( TQDomElement & parentElem, const TQString & _path )
{
    kdDebug(7043) << "KBookmarkImporter::scanIntern " << _path << endl;
    // Substitute all symbolic links in the path
    TQDir dir( _path );
    TQString canonical = dir.canonicalPath();

    if ( m_lstParsedDirs.contains(canonical) )
    {
        kdWarning() << "Directory " << canonical << " already parsed" << endl;
        return;
    }

    m_lstParsedDirs.append( canonical );

    DIR *dp;
    struct dirent *ep;
    dp = opendir( TQFile::encodeName(_path) );
    if ( dp == 0L )
        return;

    // Loop thru all directory entries
    while ( ( ep = readdir( dp ) ) != 0L )
    {
        if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
        {
            KURL file;
            file.setPath( TQString( _path ) + '/' + TQFile::decodeName(ep->d_name) );

            KMimeType::Ptr res = KMimeType::findByURL( file, 0, true );
            //kdDebug(7043) << " - " << file.url() << "  ->  " << res->name() << endl;

            if ( res->name() == "inode/directory" )
            {
                // We could use KBookmarkGroup::createNewFolder, but then it
                // would notify about the change, so we'd need a flag, etc.
                TQDomElement groupElem = m_pDoc->createElement( "folder" );
                parentElem.appendChild( groupElem );
                TQDomElement textElem = m_pDoc->createElement( "title" );
                groupElem.appendChild( textElem );
                textElem.appendChild( m_pDoc->createTextNode( TDEIO::decodeFileName( ep->d_name ) ) );
                if ( TDEIO::decodeFileName( ep->d_name ) == "Toolbar" )
                    groupElem.setAttribute("toolbar","yes");
                scanIntern( groupElem, file.path() );
            }
            else if ( (res->name() == "application/x-desktop")
                      || (res->name() == "media/builtin-mydocuments")
                      || (res->name() == "media/builtin-mycomputer")
                      || (res->name() == "media/builtin-mynetworkplaces")
                      || (res->name() == "media/builtin-printers")
                      || (res->name() == "media/builtin-trash")
                      || (res->name() == "media/builtin-webbrowser") )
            {
                KSimpleConfig cfg( file.path(), true );
                cfg.setDesktopGroup();
                TQString type = cfg.readEntry( "Type" );
                // Is it really a bookmark file ?
                if ( type == "Link" )
                    parseBookmark( parentElem, ep->d_name, cfg, 0 /* desktop group */ );
                else
                    kdWarning(7043) << "  Not a link ? Type=" << type << endl;
            }
            else if ( res->name() == "text/plain")
            {
                // maybe its an IE Favourite..
                KSimpleConfig cfg( file.path(), true );
                TQStringList grp = cfg.groupList().grep( "internetshortcut", false );
                if ( grp.count() == 0 )
                    continue;
                cfg.setGroup( *grp.begin() );

                TQString url = cfg.readPathEntry("URL");
                if (!url.isEmpty() )
                    parseBookmark( parentElem, ep->d_name, cfg, *grp.begin() );
            } else
                kdWarning(7043) << "Invalid bookmark : found mimetype='" << res->name() << "' for file='" << file.path() << "'!" << endl;
        }
    }

    closedir( dp );
}

void KBookmarkImporter::parseBookmark( TQDomElement & parentElem, TQCString _text,
                                       KSimpleConfig& _cfg, const TQString &_group )
{
    if ( !_group.isEmpty() )
        _cfg.setGroup( _group );
    else
        _cfg.setDesktopGroup();

    TQString url = _cfg.readPathEntry( "URL" );
    TQString icon = _cfg.readEntry( "Icon" );
    if (icon.right( 4 ) == ".xpm" ) // prevent warnings
        icon.truncate( icon.length() - 4 );

    TQString text = TDEIO::decodeFileName( TQString::fromLocal8Bit(_text) );
    if ( text.length() > 8 && text.right( 8 ) == ".desktop" )
        text.truncate( text.length() - 8 );
    if ( text.length() > 7 && text.right( 7 ) == ".kdelnk" )
        text.truncate( text.length() - 7 );

    TQDomElement elem = m_pDoc->createElement( "bookmark" );
    parentElem.appendChild( elem );
    elem.setAttribute( "href", url );
    //if ( icon != "www" ) // No need to save the default
    // Hmm, after all, it makes KBookmark::pixmapFile faster,
    // and it shows a nice feature to those reading the file
    elem.setAttribute( "icon", icon );
    TQDomElement textElem = m_pDoc->createElement( "title" );
    elem.appendChild( textElem );
    textElem.appendChild( m_pDoc->createTextNode( text ) );
    kdDebug(7043) << "KBookmarkImporter::parseBookmark text=" << text << endl;
}
