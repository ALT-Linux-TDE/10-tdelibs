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

#ifndef __kbookmarkimporter_kde1_h
#define __kbookmarkimporter_kde1_h

#include <tqdom.h>
#include <tqcstring.h>
#include <tqstringlist.h>
#include <ksimpleconfig.h>

/**
 * A class for importing the previous bookmarks (desktop files)
 * Separated from KBookmarkManager to save memory (we throw this one
 * out once the import is done)
 */
class TDEIO_EXPORT KBookmarkImporter
{
public:
    KBookmarkImporter( TQDomDocument * doc ) : m_pDoc(doc) {}

    void import( const TQString & path );

private:
    void scanIntern( TQDomElement & parentElem, const TQString & _path );
    void parseBookmark( TQDomElement & parentElem, TQCString _text,
                        KSimpleConfig& _cfg, const TQString &_group );
    TQDomDocument * m_pDoc;
    TQStringList m_lstParsedDirs;
};

#endif
