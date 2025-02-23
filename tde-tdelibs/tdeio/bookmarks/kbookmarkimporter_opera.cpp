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
#include "kbookmarkimporter_opera.h"

void KOperaBookmarkImporter::parseOperaBookmarks( )
{
   TQFile file(m_fileName);
   if(!file.open(IO_ReadOnly)) {
      return;
   }

   TQTextCodec * codec = TQTextCodec::codecForName("UTF-8");
   Q_ASSERT(codec);
   if (!codec)
      return;

   int lineno = 0;
   TQString url, name, type;
   static const int g_lineLimit = 16*1024; 
   TQCString line(g_lineLimit);

   while ( file.readLine(line.data(), g_lineLimit) >=0 ) {
      lineno++;
    
      // skip lines that didn't fit in buffer and first two headers lines 
      if ( line[line.length()-1] != '\n' || lineno <= 2 )
          continue;
    
      TQString currentLine = codec->toUnicode(line).stripWhiteSpace();
    
      if (currentLine.isEmpty()) {
         // end of data block
         if (type.isNull())
            continue;
         else if ( type == "URL")
            emit newBookmark( name, url.latin1(), "" );
         else if (type == "FOLDER" )
            emit newFolder( name, false, "" ); 

         type = TQString::null;
         name = TQString::null;
         url = TQString::null;
         
      } else if (currentLine == "-") {
         // end of folder
         emit endFolder();
    
      } else {
         // data block line
         TQString tag;
         if ( tag = "#", currentLine.startsWith( tag ) )
            type = currentLine.remove( 0, tag.length() );
         else if ( tag = "NAME=", currentLine.startsWith( tag ) )
            name = currentLine.remove(0, tag.length());
         else if ( tag = "URL=", currentLine.startsWith( tag ) )
            url = currentLine.remove(0, tag.length());
      }
   }

}

TQString KOperaBookmarkImporter::operaBookmarksFile()
{
   static KOperaBookmarkImporterImpl *p = 0;
   if (!p) 
       p = new KOperaBookmarkImporterImpl; 
   return p->findDefaultLocation();
}

void KOperaBookmarkImporterImpl::parse() {
   KOperaBookmarkImporter importer(m_fileName);
   setupSignalForwards(&importer, this);
   importer.parseOperaBookmarks();
}

TQString KOperaBookmarkImporterImpl::findDefaultLocation(bool saving) const
{
   return saving ? KFileDialog::getSaveFileName( 
                       TQDir::homeDirPath() + "/.opera", 
                       i18n("*.adr|Opera Bookmark Files (*.adr)") )
                 : KFileDialog::getOpenFileName( 
                       TQDir::homeDirPath() + "/.opera", 
                       i18n("*.adr|Opera Bookmark Files (*.adr)") );
}

/////////////////////////////////////////////////

class OperaExporter : private KBookmarkGroupTraverser {
public:
    OperaExporter();
    TQString generate( const KBookmarkGroup &grp ) { traverse(grp); return m_string; };
private:
    virtual void visit( const KBookmark & );
    virtual void visitEnter( const KBookmarkGroup & );
    virtual void visitLeave( const KBookmarkGroup & );
private:
    TQString m_string;
    TQTextStream m_out;
};

OperaExporter::OperaExporter() : m_out(&m_string, IO_WriteOnly) {
    m_out << "Opera Hotlist version 2.0" << endl;
    m_out << "Options: encoding = utf8, version=3" << endl;
}

void OperaExporter::visit( const KBookmark &bk ) {
    // kdDebug() << "visit(" << bk.text() << ")" << endl;
    m_out << "#URL" << endl;
    m_out << "\tNAME=" << bk.fullText() << endl;
    m_out << "\tURL=" << bk.url().url().utf8() << endl;
    m_out << endl;
}

void OperaExporter::visitEnter( const KBookmarkGroup &grp ) {
    // kdDebug() << "visitEnter(" << grp.text() << ")" << endl;
    m_out << "#FOLDER" << endl;
    m_out << "\tNAME="<< grp.fullText() << endl;
    m_out << endl;
}

void OperaExporter::visitLeave( const KBookmarkGroup & ) {
    // kdDebug() << "visitLeave()" << endl;
    m_out << "-" << endl;
    m_out << endl;
}

void KOperaBookmarkExporterImpl::write(KBookmarkGroup parent) {
    OperaExporter exporter;
    TQString content = exporter.generate( parent );
    TQFile file(m_fileName);
    if (!file.open(IO_WriteOnly)) {
       kdError(7043) << "Can't write to file " << m_fileName << endl;
       return;
    }
    TQTextStream fstream(&file);
    fstream.setEncoding(TQTextStream::UnicodeUTF8);
    fstream << content;
}

#include "kbookmarkimporter_opera.moc"
