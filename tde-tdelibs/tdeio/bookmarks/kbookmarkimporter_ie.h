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

#ifndef __kbookmarkimporter_ie_h
#define __kbookmarkimporter_ie_h

#include <tqdom.h>
#include <tqcstring.h>
#include <tqstringlist.h>
#include <ksimpleconfig.h>
#include <tdemacros.h>

#include <kbookmarkimporter.h>

/**
 * A class for importing IE bookmarks
 * @deprecated
 */
class TDEIO_EXPORT_DEPRECATED KIEBookmarkImporter : public TQObject
{
    TQ_OBJECT
public:
    KIEBookmarkImporter( const TQString & fileName ) : m_fileName(fileName) {}
    ~KIEBookmarkImporter() {}

    void parseIEBookmarks();

    // Usual place for IE bookmarks
    static TQString IEBookmarksDir();

signals:
    void newBookmark( const TQString & text, const TQCString & url, const TQString & additionalInfo );
    void newFolder( const TQString & text, bool open, const TQString & additionalInfo );
    void newSeparator();
    void endFolder();

protected:
    void parseIEBookmarks_dir( TQString dirname, TQString name = TQString::null );
    void parseIEBookmarks_url_file( TQString filename, TQString name );

    TQString m_fileName;
};

/**
 * A class for importing IE bookmarks
 * @since 3.2
 */
class TDEIO_EXPORT KIEBookmarkImporterImpl : public KBookmarkImporterBase
{
public:
    KIEBookmarkImporterImpl() { }
    virtual void parse();
    virtual TQString findDefaultLocation(bool forSaving = false) const;
private:
    class KIEBookmarkImporterImplPrivate *d;
};

/*
 * @since 3.2
 */
class TDEIO_EXPORT KIEBookmarkExporterImpl : public KBookmarkExporterBase
{
public:
    KIEBookmarkExporterImpl(KBookmarkManager* mgr, const TQString & path)
      : KBookmarkExporterBase(mgr, path) 
    { ; }
    virtual ~KIEBookmarkExporterImpl() {}
    virtual void write(KBookmarkGroup);
private:
    class KIEBookmarkExporterImplPrivate *d;
};

#endif
