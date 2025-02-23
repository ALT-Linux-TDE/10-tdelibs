/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>

#include <kbookmarkimporter.h>
#include <kbookmarkdombuilder.h>
#include <tdepopupmenu.h>
#include <kstandarddirs.h>

#include "tdefiledialog.h"
#include "tdefilebookmarkhandler.h"

KFileBookmarkHandler::KFileBookmarkHandler( KFileDialog *dialog )
    : TQObject( dialog, "KFileBookmarkHandler" ),
      KBookmarkOwner(),
      m_dialog( dialog )
{
    m_menu = new TDEPopupMenu( dialog, "bookmark menu" );

    TQString file = locate( "data", "tdefile/bookmarks.xml" );
    if ( file.isEmpty() )
        file = locateLocal( "data", "tdefile/bookmarks.xml" );

    KBookmarkManager *manager = KBookmarkManager::managerForFile( file, false);

    // import old bookmarks
    if ( !TDEStandardDirs::exists( file ) ) {
        TQString oldFile = locate( "data", "tdefile/bookmarks.html" );
        if ( !oldFile.isEmpty() )
            importOldBookmarks( oldFile, manager );
    }

    manager->setUpdate( true );
    manager->setShowNSBookmarks( false );

    m_bookmarkMenu = new KBookmarkMenu( manager, this, m_menu,
                                        dialog->actionCollection(), true );
}

KFileBookmarkHandler::~KFileBookmarkHandler()
{
    delete m_bookmarkMenu;
}

TQString KFileBookmarkHandler::currentURL() const
{
    return m_dialog->baseURL().url();
}

void KFileBookmarkHandler::importOldBookmarks( const TQString& path,
                                               KBookmarkManager *manager )
{
    KBookmarkDomBuilder *builder = new KBookmarkDomBuilder( manager->root(), manager );
    KNSBookmarkImporter importer( path );
    builder->connectImporter( &importer );
    importer.parseNSBookmarks();
    delete builder;
    manager->save();
}

void KFileBookmarkHandler::virtual_hook( int id, void* data )
{ KBookmarkOwner::virtual_hook( id, data ); }

#include "tdefilebookmarkhandler.moc"
