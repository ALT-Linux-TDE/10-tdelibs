/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KPIXMAPSPLITTER_H
#define KPIXMAPSPLITTER_H

#include <tqpixmap.h>
#include <tqrect.h>
#include <tqsize.h>
#include <tqstring.h>

#include <tdelibs_export.h>

class KPixmapSplitterPrivate;
/**
 * @short A class to split a pixmap into several items.
 *
 * If you have a pixmap containing several items (icons), you can use this
 * class to get the coordinates of each item.
 *
 * For example, if you have a pixmap with 25 items and you want to get the
 * 4th item as a pixmap (every item being 20x10 pixels):
 * \code
 * KPixmapSplitter splitter;
 * splitter.setPixmap( somePixmap );
 * splitter.setItemSize( TQSize( 20, 10 ));
 *
 * TQPixmap item( 20, 10 );
 * item.fill( TQt::white );
 * TQRect rect = splitter.coordinates( 4 );
 * if ( !rect.isEmpty() )
 *     bitBlt( &item, TQPoint(0,0), &somePixmap, rect, CopyROP );
 * \endcode
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class TDEFX_EXPORT KPixmapSplitter
{
public:
    /**
     * Constructor, does nothing but initialize some default-values.
     */
    KPixmapSplitter();
    ~KPixmapSplitter();

    /**
     * Sets the pixmap to be split.
     */
    void setPixmap( const TQPixmap& pixmap );

    /**
     * @returns the pixmap that has been set via setPixmap().
     */
    const TQPixmap& pixmap() const { return m_pixmap; }

    /**
     * Sets the size of the items you want to get out of the given pixmap.
     * The TQRect of #coordinates(int) will have the width and height of exactly
     * this @p size.
     */
    void setItemSize( const TQSize& size );

    /**
     * @returns the set size of the items (coordinates) you want to get
     * out of the given pixmap.
     */
    TQSize itemSize() const { return m_itemSize; }

    /**
     * If there is space between rows in the given pixmap, you have to specify
     * how many pixels there are.
     */
    void setVSpacing( int spacing );

    /**
     * If there is space between columns in the given pixmap, you have to
     * specify how many pixels there are.
     */
    void setHSpacing( int spacing );

    /**
     * @returns the coordinates of the item at position pos in the given
     * pixmap.
     */
    TQRect coordinates( int pos );

    /**
     * Overloaded for convenience. Returns the item at the position of the
     * given character (when using a latin1 font-pixmap)
     */
    TQRect coordinates( const TQChar& ch );

private:
    TQPixmap m_pixmap;
    TQSize m_itemSize;

    int m_vSpacing;
    int m_hSpacing;

    int m_numCols;
    int m_numRows;

    bool m_dirty;
    KPixmapSplitterPrivate* d;
};

#endif // KPIXMAPSPLITTER_H
