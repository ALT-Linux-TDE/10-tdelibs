/* This file is part of the KDE libraries
   Copyright (C) 2002 Alexander Kellett <lypanov@kde.org>

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

#ifndef __kbookmarkimporter_opera_h
#define __kbookmarkimporter_opera_h

#include <tqdom.h>
#include <tqcstring.h>
#include <tqstringlist.h>
#include <ksimpleconfig.h>

#include <kbookmarkimporter.h>

/**
 * A class for importing Opera bookmarks
 * @deprecated
 */
class TDEIO_EXPORT_DEPRECATED KOperaBookmarkImporter : public TQObject
{
    TQ_OBJECT
public:
    KOperaBookmarkImporter( const TQString & fileName ) : m_fileName(fileName) {}
    ~KOperaBookmarkImporter() {}

    void parseOperaBookmarks();

    // Usual place for Opera bookmarks
    static TQString operaBookmarksFile();

signals:
    void newBookmark( const TQString & text, const TQCString & url, const TQString & additionalInfo );
    void newFolder( const TQString & text, bool open, const TQString & additionalInfo );
    void newSeparator();
    void endFolder();

protected:
    TQString m_fileName;
};

/**
 * A class for importing Opera bookmarks
 * @since 3.2
 */
class TDEIO_EXPORT KOperaBookmarkImporterImpl : public KBookmarkImporterBase
{
public:
    KOperaBookmarkImporterImpl() { }
    virtual void parse();
    virtual TQString findDefaultLocation(bool forSaving = false) const;
private:
    class KOperaBookmarkImporterImplPrivate *d;
};

/**
 * @since 3.2
 */
class TDEIO_EXPORT KOperaBookmarkExporterImpl : public KBookmarkExporterBase
{
public:
    KOperaBookmarkExporterImpl(KBookmarkManager* mgr, const TQString & filename)
      : KBookmarkExporterBase(mgr, filename) 
    { ; }
    virtual ~KOperaBookmarkExporterImpl() {}
    virtual void write(KBookmarkGroup);
private:
    class KOperaBookmarkExporterImplPrivate *d;
};

#endif
