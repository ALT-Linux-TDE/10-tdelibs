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

#ifndef __kebdrag_h
#define __kebdrag_h

#include <tqdragobject.h>
#include <kbookmark.h>

// Clipboard/dnd data : URLs + XML for bookmarks
class TDEIO_EXPORT KBookmarkDrag : public TQUriDrag
{
public:
    static KBookmarkDrag * newDrag( const TQValueList<KBookmark> & bookmarks,
                                    TQWidget * dragSource = 0,
                                    const char * name = 0 );
    static KBookmarkDrag * newDrag( const KBookmark & bookmark,
                                    TQWidget * dragSource = 0,
                                    const char * name = 0 );
protected:
    KBookmarkDrag( const TQValueList<KBookmark> & bookmarks,
                   const TQStrList & urls,
                   TQWidget * dragSource,
                   const char * name );
public:
    virtual ~KBookmarkDrag() {}

    virtual const char* format( int i ) const;
    virtual TQByteArray encodedData( const char* mime ) const;

    static bool canDecode( const TQMimeSource * e );
    static TQValueList<KBookmark> decode( const TQMimeSource * e );

protected:
    TQValueList<KBookmark> m_bookmarks;
    TQDomDocument m_doc;
    class KBookmarkDragPrivate;
    KBookmarkDragPrivate * d;
};
#endif
