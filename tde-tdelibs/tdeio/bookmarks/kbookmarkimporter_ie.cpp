/* This file is part of the KDE libraries
   Copyright (C) 2002-2003  Alexander Kellett <lypanov@kde.org>

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

#include <tdefiledialog.h>
#include <kstringhandler.h>
#include <tdelocale.h>
#include <kdebug.h>
#include <tqtextcodec.h>

#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>

#include "kbookmarkimporter.h"
#include "kbookmarkimporter_ie.h"

/* antlarr: KDE 4: Make them const TQString & */
void KIEBookmarkImporter::parseIEBookmarks_url_file( TQString filename, TQString name ) {
    static const int g_lineLimit = 16*1024;

    TQFile f(filename);

    if(f.open(IO_ReadOnly)) {

        TQCString s(g_lineLimit);

        while(f.readLine(s.data(), g_lineLimit)>=0) {
            if ( s[s.length()-1] != '\n' ) // Gosh, this line is longer than g_lineLimit. Skipping.
            {
               kdWarning() << "IE bookmarks contain a line longer than " << g_lineLimit << ". Skipping." << endl;
               continue;
            }
            TQCString t = s.stripWhiteSpace();
            TQRegExp rx( "URL=(.*)" );
            if (rx.exactMatch(t)) {
               emit newBookmark( name, TQString(rx.cap(1)).latin1(), TQString("") );
            }
        }

        f.close();
    }
}

/* antlarr: KDE 4: Make them const TQString & */
void KIEBookmarkImporter::parseIEBookmarks_dir( TQString dirname, TQString foldername )
{

   TQDir dir(dirname);
   dir.setFilter( TQDir::Files | TQDir::Dirs );
   dir.setSorting( TQDir::Name | TQDir::DirsFirst );
   dir.setNameFilter("*.url"); // AK - possibly add ";index.ini" ?
   dir.setMatchAllDirs(true);

   const TQFileInfoList *list = dir.entryInfoList();
   if (!list) return;

   if (dirname != m_fileName) 
      emit newFolder( foldername, false, "" );

   TQFileInfoListIterator it( *list );
   TQFileInfo *fi;

   while ( (fi = it.current()) != 0 ) {
      ++it;

      if (fi->fileName() == "." || fi->fileName() == "..") continue;

      if (fi->isDir()) {
         parseIEBookmarks_dir(fi->absFilePath(), fi->fileName());

      } else if (fi->isFile()) {
         if (fi->fileName().endsWith(".url")) {
            TQString name = fi->fileName();
            name.truncate(name.length() - 4); // .url
            parseIEBookmarks_url_file(fi->absFilePath(), name);
         }
         // AK - add index.ini
      }
   }

   if (dirname != m_fileName) 
      emit endFolder();
}


void KIEBookmarkImporter::parseIEBookmarks( )
{
    parseIEBookmarks_dir( m_fileName );
}

TQString KIEBookmarkImporter::IEBookmarksDir()
{
   static KIEBookmarkImporterImpl* p = 0;
   if (!p) 
       p = new KIEBookmarkImporterImpl;
   return p->findDefaultLocation();
}

void KIEBookmarkImporterImpl::parse() {
   KIEBookmarkImporter importer(m_fileName);
   setupSignalForwards(&importer, this);
   importer.parseIEBookmarks();
}

TQString KIEBookmarkImporterImpl::findDefaultLocation(bool) const
{
    // notify user that they must give a new dir such 
    // as "Favourites" as otherwise it'll just place
    // lots of .url files in the given dir and gui
    // stuff in the exporter is ugly so that exclues
    // the possibility of just writing to Favourites
    // and checking if overwriting...
    return KFileDialog::getExistingDirectory();
}

/////////////////////////////////////////////////

class IEExporter : private KBookmarkGroupTraverser {
public:
    IEExporter( const TQString & );
    void write( const KBookmarkGroup &grp ) { traverse(grp); };
private:
    virtual void visit( const KBookmark & );
    virtual void visitEnter( const KBookmarkGroup & );
    virtual void visitLeave( const KBookmarkGroup & );
private:
    TQDir m_currentDir;
};

static TQString ieStyleQuote( const TQString &str ) {
    TQString s(str);
    s.replace(TQRegExp("[/\\:*?\"<>|]"), "_");
    return s;
}

IEExporter::IEExporter( const TQString & dname ) {
    m_currentDir.setPath( dname );
}

void IEExporter::visit( const KBookmark &bk ) {
    TQString fname = m_currentDir.path() + "/" + ieStyleQuote( bk.fullText() ) + ".url";
    // kdDebug() << "visit(" << bk.text() << "), fname == " << fname << endl;
    TQFile file( fname );
    file.open( IO_WriteOnly );
    TQTextStream ts( &file );
    ts << "[InternetShortcut]\r\n";
    ts << "URL=" << bk.url().url().utf8() << "\r\n";
}

void IEExporter::visitEnter( const KBookmarkGroup &grp ) {
    TQString dname = m_currentDir.path() + "/" + ieStyleQuote( grp.fullText() );
    // kdDebug() << "visitEnter(" << grp.text() << "), dname == " << dname << endl;
    m_currentDir.mkdir( dname );
    m_currentDir.cd( dname );
}

void IEExporter::visitLeave( const KBookmarkGroup & ) {
    // kdDebug() << "visitLeave()" << endl;
    m_currentDir.cdUp();
}

void KIEBookmarkExporterImpl::write(KBookmarkGroup parent) {
    IEExporter exporter( m_fileName );
    exporter.write( parent );
}

#include "kbookmarkimporter_ie.moc"
