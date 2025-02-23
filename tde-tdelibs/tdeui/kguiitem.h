/* This file is part of the KDE libraries
    Copyright (C) 2001 Holger Freyther (freyher@yahoo.com)
                  based on ideas from Martijn and Simon

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

    Many thanks to Simon tronical Hausmann
*/

#ifndef __kguiitem_h__
#define __kguiitem_h__

#include <tqstring.h>
#include <tqiconset.h>
#include <tqpixmap.h>
#include <tqvaluelist.h>
#include <kicontheme.h>
#include <tdeglobal.h>

/**
 * @short An abstract class for GUI data such as ToolTip and Icon.
 *
 * @author Holger Freyther <freyher@yahoo.com>
 * @see KStdGuiItem
 */
class TDEUI_EXPORT KGuiItem
{
public:
    KGuiItem();

    // ### This should probably be explicit in KDE 4; it's easy to get
    // subtle bugs otherwise - the icon name, tooltip and whatsthis text
    // get changed behind your back if you do 'setButtonFoo( "Bar" );'
    // It gives the wrong impression that you just change the text.
    KGuiItem( const TQString &text, 
              const TQString &iconName  = TQString::null,
              const TQString &toolTip   = TQString::null,
              const TQString &whatsThis = TQString::null );

    KGuiItem( const TQString &text, const TQIconSet &iconSet, 
              const TQString &toolTip   = TQString::null, 
              const TQString &whatsThis = TQString::null );

    KGuiItem( const KGuiItem &rhs );
    KGuiItem &operator=( const KGuiItem &rhs );

    ~KGuiItem();

    TQString text() const;
    TQString plainText() const;
#ifndef KDE_NO_COMPAT
    TQIconSet iconSet( TDEIcon::Group, int size = 0, TDEInstance* instance = TDEGlobal::instance()) const;
    TQIconSet iconSet() const { return iconSet( TDEIcon::Small ); }
#else
    TQIconSet iconSet( TDEIcon::Group=TDEIcon::Small, int size = 0, TDEInstance* instance = TDEGlobal::instance()) const;
#endif

    TQString iconName() const;
    TQString toolTip() const;
    TQString whatsThis() const;
    bool isEnabled() const;
    /**
     * returns whether an icon is defined, doesn't tell if it really exists
     */
    bool hasIcon() const;
#ifndef KDE_NO_COMPAT
    bool hasIconSet() const { return hasIcon(); }
#endif

    void setText( const TQString &text );
    void setIconSet( const TQIconSet &iconset );
    void setIconName( const TQString &iconName );
    void setToolTip( const TQString &tooltip );
    void setWhatsThis( const TQString &whatsThis );
    void setEnabled( bool enable );

private:
    class KGuiItemPrivate;
    KGuiItemPrivate *d;
};
#endif
