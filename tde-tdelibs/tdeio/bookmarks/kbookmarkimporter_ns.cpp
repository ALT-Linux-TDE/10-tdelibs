/* This file is part of the KDE libraries
   Copyright (C) 1996-1998 Martin R. Jones <mjones@kde.org>
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

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

#include "kbookmarkimporter.h"
#include "kbookmarkexporter.h"
#include "kbookmarkmanager.h"
#include <tdefiledialog.h>
#include <kstringhandler.h>
#include <tdelocale.h>
#include <kdebug.h>
#include <kcharsets.h>
#include <tqtextcodec.h>
#include <tqstylesheet.h>

#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <assert.h>

void KNSBookmarkImporterImpl::parse()
{
    TQFile f(m_fileName);
    TQTextCodec * codec = m_utf8 ? TQTextCodec::codecForName("UTF-8") : TQTextCodec::codecForLocale();
    Q_ASSERT(codec);
    if (!codec)
        return;

    if(f.open(IO_ReadOnly)) {

        static const int g_lineLimit = 16*1024;
        TQCString s(g_lineLimit);
        // skip header
        while(f.readLine(s.data(), g_lineLimit) >= 0 && !s.contains("<DL>"));

        while(f.readLine(s.data(), g_lineLimit)>=0) {
            if ( s[s.length()-1] != '\n' ) // Gosh, this line is longer than g_lineLimit. Skipping.
            {
               kdWarning() << "Netscape bookmarks contain a line longer than " << g_lineLimit << ". Skipping." << endl;
               continue;
            }
            TQCString t = s.stripWhiteSpace();
            if(t.left(12).upper() == "<DT><A HREF=" ||
               t.left(16).upper() == "<DT><H3><A HREF=") {
              int firstQuotes = t.find('"')+1;
              int secondQuotes = t.find('"', firstQuotes);
              if (firstQuotes != -1 && secondQuotes != -1)
              {
                TQCString link = t.mid(firstQuotes, secondQuotes-firstQuotes);
                int endTag = t.find('>', secondQuotes+1);
                TQCString name = t.mid(endTag+1);
                name = name.left(name.findRev('<'));
                if ( name.right(4) == "</A>" )
                    name = name.left( name.length() - 4 );
                TQString qname = KCharsets::resolveEntities( codec->toUnicode( name ) );
                TQCString additionalInfo = t.mid( secondQuotes+1, endTag-secondQuotes-1 );

                emit newBookmark( qname,
                                  link, codec->toUnicode(additionalInfo) );
              }
            }
            else if(t.left(7).upper() == "<DT><H3") {
                int endTag = t.find('>', 7);
                TQCString name = t.mid(endTag+1);
                name = name.left(name.findRev('<'));
                TQString qname = KCharsets::resolveEntities( codec->toUnicode( name ) );
                TQCString additionalInfo = t.mid( 8, endTag-8 );
                bool folded = (additionalInfo.left(6) == "FOLDED");
                if (folded) additionalInfo.remove(0,7);

                emit newFolder( qname,
                                !folded,
                                codec->toUnicode(additionalInfo) );
            }
            else if(t.left(4).upper() == "<HR>")
                emit newSeparator();
            else if(t.left(8).upper() == "</DL><P>")
                emit endFolder();
        }

        f.close();
    }
}

TQString KNSBookmarkImporterImpl::findDefaultLocation(bool forSaving) const
{
    if (m_utf8) 
    {
       if ( forSaving )
           return KFileDialog::getSaveFileName( TQDir::homeDirPath() + "/.mozilla",
                                                i18n("*.html|HTML Files (*.html)") );
       else
           return KFileDialog::getOpenFileName( TQDir::homeDirPath() + "/.mozilla",
                                                i18n("*.html|HTML Files (*.html)") );
    } 
    else 
    {
       return TQDir::homeDirPath() + "/.netscape/bookmarks.html";
    }
}

////////////////////////////////////////////////////////////////


void KNSBookmarkImporter::parseNSBookmarks( bool utf8 )
{
    KNSBookmarkImporterImpl importer;
    importer.setFilename(m_fileName);
    importer.setUtf8(utf8);
    importer.setupSignalForwards(&importer, this);
    importer.parse();
}

TQString KNSBookmarkImporter::netscapeBookmarksFile( bool forSaving )
{
    static KNSBookmarkImporterImpl *p = 0;
    if (!p)
    {
        p = new KNSBookmarkImporterImpl;
        p->setUtf8(false);
    }
    return p->findDefaultLocation(forSaving);
}

TQString KNSBookmarkImporter::mozillaBookmarksFile( bool forSaving )
{
    static KNSBookmarkImporterImpl *p = 0;
    if (!p)
    {
        p = new KNSBookmarkImporterImpl;
        p->setUtf8(true);
    }
    return p->findDefaultLocation(forSaving);
}


////////////////////////////////////////////////////////////////
//                   compat only
////////////////////////////////////////////////////////////////

void KNSBookmarkExporter::write(bool utf8) {
   KNSBookmarkExporterImpl exporter(m_pManager, m_fileName);
   exporter.setUtf8(utf8);
   exporter.write(m_pManager->root());
}

void KNSBookmarkExporter::writeFolder(TQTextStream &/*stream*/, KBookmarkGroup /*gp*/) {
   // TODO - requires a d pointer workaround hack?
}

////////////////////////////////////////////////////////////////

void KNSBookmarkExporterImpl::setUtf8(bool utf8) {
   m_utf8 = utf8;
}

void KNSBookmarkExporterImpl::write(KBookmarkGroup parent) {
   if (TQFile::exists(m_fileName)) {
      ::rename(
         TQFile::encodeName(m_fileName), 
         TQFile::encodeName(m_fileName + ".beforekde"));
   }

   TQFile file(m_fileName);

   if (!file.open(IO_WriteOnly)) {
      kdError(7043) << "Can't write to file " << m_fileName << endl;
      return;
   }

   TQTextStream fstream(&file);
   fstream.setEncoding(m_utf8 ? TQTextStream::UnicodeUTF8 : TQTextStream::Locale);

   TQString charset 
      = m_utf8 ? "UTF-8" : TQString::fromLatin1(TQTextCodec::codecForLocale()->name()).upper();

   fstream << "<!DOCTYPE NETSCAPE-Bookmark-file-1>" << endl
           << i18n("<!-- This file was generated by Konqueror -->") << endl
           << "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=" 
              << charset << "\">" << endl
           << "<TITLE>" << i18n("Bookmarks") << "</TITLE>" << endl
           << "<H1>" << i18n("Bookmarks") << "</H1>" << endl
           << "<DL><p>" << endl
           << folderAsString(parent)
           << "</DL><P>" << endl;
}

TQString KNSBookmarkExporterImpl::folderAsString(KBookmarkGroup parent) const {
   TQString str;
   TQTextStream fstream(&str, IO_WriteOnly);

   for (KBookmark bk = parent.first(); !bk.isNull(); bk = parent.next(bk)) {
      if (bk.isSeparator()) {
         fstream << "<HR>" << endl;
         continue;
      }

      TQString text = TQStyleSheet::escape(bk.fullText());

      if (bk.isGroup() ) {
         fstream << "<DT><H3 " 
                    << (!bk.toGroup().isOpen() ? "FOLDED " : "")
                    << bk.internalElement().attribute("netscapeinfo") << ">" 
                 << text << "</H3>" << endl
                 << "<DL><P>" << endl
                 << folderAsString(bk.toGroup())
                 << "</DL><P>" << endl;
         continue;

      } else {
         // note - netscape seems to use local8bit for url...
         fstream << "<DT><A HREF=\"" << bk.url().url() << "\""
                    << bk.internalElement().attribute("netscapeinfo") << ">" 
                 << text << "</A>" << endl;
         continue;
      }
   }

   return str;
}

////

#include "kbookmarkimporter_ns.moc"
