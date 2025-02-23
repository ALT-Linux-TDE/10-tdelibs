/* This file is part of the KDE libraries
   Copyright (C) 1999 Steffen Hansen (hansen@kde.org)

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
/*
 * $Id$
 */
#ifndef _KCOLORDRAG_H
#define _KCOLORDRAG_H

#include <tqdragobject.h>
#include <tqcolor.h>
#include <tdelibs_export.h>

class KColorDragPrivate;
/**
 * A drag-and-drop object for colors. The according MIME type
 * is set to application/x-color.
 *
 * See the Qt drag'n'drop documentation.
 */
class TDEUI_EXPORT KColorDrag : public TQStoredDrag {
    TQ_OBJECT

public:
    /**
     * Constructs a color drag with a white color.
     */
    KColorDrag( TQWidget *dragsource = 0, const char *name = 0 );
    /**
     * Constructs a color drag with the color @p col.
     */
    KColorDrag( const TQColor &col, TQWidget *dragsource = 0, const char *name = 0 );
    virtual ~KColorDrag() {}

    virtual const char *format(int i) const;
    virtual TQByteArray encodedData ( const char * m ) const;

    /**
     * Sets the color of the drag to @p col.
     */
    void setColor(const TQColor &col);
    /**
     * Returns true if the MIME source @p e contains a color object.
     */
    static bool canDecode(TQMimeSource *e);
    /**
     * Decodes the MIME source @p e and puts the resulting color into @p col.
     */
    static bool decode(TQMimeSource *e, TQColor &col);
    /**
     * @deprecated This is equivalent with "new KColorDrag(color, dragsource)".
     */
    static KColorDrag* makeDrag( const TQColor&,TQWidget *dragsource) TDE_DEPRECATED;

private:
     TQColor m_color; // unused
protected:
     virtual void virtual_hook( int id, void* data );
private:
     KColorDragPrivate *d;
};


#endif // _KCOLORDRAG_H
