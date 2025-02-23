/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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
//-----------------------------------------------------------------------------
// KDE color selection combo box

// layout management added Oct 1997 by Mario Weilguni
// <mweilguni@sime.com>


#ifndef _KCOLORCOMBO_H__
#define _KCOLORCOMBO_H__

#include <tqcombobox.h>
#include <kcolordialog.h>
#include "tdeselect.h"


class KColorComboInternal;

/**
 * Combobox for colors.
 */
class TDEUI_EXPORT KColorCombo : public TQComboBox
{
    TQ_OBJECT
    TQ_PROPERTY( TQColor color READ color WRITE setColor )

public:
    /**
     * Constructs a color combo box.
     */
    KColorCombo( TQWidget *parent, const char *name = 0L );
    ~KColorCombo();

    /**
     * Selects the color @p col.
     */
    void setColor( const TQColor &col );
    /**
     * Returns the currently selected color.
     **/
    TQColor color() const;


    /**
     * Clear the color list and don't show it, till the next setColor() call
     **/
     void showEmptyList();

signals:
    /**
     * Emitted when a new color box has been selected.
     */
    void activated( const TQColor &col );
    /**
     * Emitted when a new item has been highlighted.
     */
    void highlighted( const TQColor &col );

protected:
	virtual void resizeEvent( TQResizeEvent *re );

private slots:
	void slotActivated( int index );
	void slotHighlighted( int index );

private:
	void addColors();
	TQColor customColor;
	TQColor internalcolor;

protected:
	virtual void virtual_hook( int id, void* data );
private:
	class KColorComboPrivate;
	KColorComboPrivate *d;
};

#endif	// __KCOLORCOMBO_H__
