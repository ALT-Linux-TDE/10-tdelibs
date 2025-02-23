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

#ifndef __kbookmarkimporter_crash_h
#define __kbookmarkimporter_crash_h

#include <tqdom.h>
#include <tqcstring.h>
#include <tqstringlist.h>
#include <tqmap.h>
#include <ksimpleconfig.h>
#include <tdemacros.h>

#include "kbookmarkimporter.h"

/**
 * A class for importing all crash sessions as bookmarks
 * @deprecated
 */
class TDEIO_EXPORT_DEPRECATED TDECrashBookmarkImporter : public TQObject
{
    TQ_OBJECT
public:
    TDECrashBookmarkImporter( const TQString & fileName ) : m_fileName(fileName) {}
    ~TDECrashBookmarkImporter() {}
    void parseCrashBookmarks( bool del = true );
    static TQString crashBookmarksDir( );
    static TQStringList getCrashLogs(); // EMPTY!
signals:
    void newBookmark( const TQString & text, const TQCString & url, const TQString & additionalInfo );
    void newFolder( const TQString & text, bool open, const TQString & additionalInfo );
    void newSeparator();
    void endFolder();
protected:
    TQString m_fileName;
    void parseCrashLog( TQString filename, bool del ); // EMPTY!
};

/**
 * A class for importing all crash sessions as bookmarks
 * @since 3.2
 */
class TDEIO_EXPORT TDECrashBookmarkImporterImpl : public KBookmarkImporterBase
{
public:
    TDECrashBookmarkImporterImpl() : m_shouldDelete(false) { }
    void setShouldDelete(bool);
    virtual void parse();
    virtual TQString findDefaultLocation(bool forSaving = false) const;
    static TQStringList getCrashLogs();
private:
    bool m_shouldDelete;
    TQMap<TQString, TQString> parseCrashLog_noemit( const TQString & filename, bool del );
    class TDECrashBookmarkImporterImplPrivate *d;
};

#endif
