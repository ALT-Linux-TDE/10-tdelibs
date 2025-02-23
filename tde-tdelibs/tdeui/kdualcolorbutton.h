/* This file is part of the KDE libraries
   Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

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
#ifndef __KDUALCOLORBTN_H
#define __KDUALCOLORBTN_H

class TQBitmap;
#include <tqbrush.h>
#include <tqwidget.h>

#include <tdelibs_export.h>

/**
 * @short A widget for selecting two related colors.
 *
 * KDualColorButton allows the user to select two cascaded colors (usually a
 * foreground and background color). Other features include drag and drop
 * from other KDE color widgets, a reset to black and white control, and a
 * swap colors control.
 *
 * When the user clicks on the foreground or background rectangle the
 * rectangle is first sunken and the currentChanged() signal is emitted.
 * Further clicks will present a color dialog and emit either the fgChanged()
 * or bgChanged() if a new color is selected.
 *
 * Note: With drag and drop when dropping a color the current selected color
 * will be set, while when dragging a color it will use whatever color
 * rectangle the mouse was pressed inside.
 *
 * \image html kdualcolorbutton.png "KDE Dual Color Button"
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class TDEUI_EXPORT KDualColorButton : public TQWidget
{
    TQ_OBJECT
    TQ_ENUMS( DualColor )
    TQ_PROPERTY( TQColor foreground READ foreground WRITE setForeground )
    TQ_PROPERTY( TQColor background READ background WRITE setBackground )
    TQ_PROPERTY( TQColor currentColor READ currentColor WRITE setCurrentColor STORED false DESIGNABLE false )
    TQ_PROPERTY( DualColor current READ current WRITE setCurrent )

public:

    enum DualColor { Foreground, Background };
    /**
     * Constructs a new KDualColorButton using the default black and white
     * colors.
     *
     * As of KDE 3.5.1, sets the dialog parent to the same as "parent" if that
     * argument is non-null and the dialogParent argument is null.
     */
    KDualColorButton(TQWidget *parent=0, const char *name=0, TQWidget* dialogParent=0);

    /**
     * Constructs a new KDualColorButton with the supplied foreground and
     * background colors.
     */
    KDualColorButton(const TQColor &fgColor, const TQColor &bgColor,
                     TQWidget *parent=0, const char *name=0, TQWidget* dialogParent=0);

    ~KDualColorButton();
    /**
     * Returns the current foreground color.
     */
    TQColor foreground() const;
    /**
     * Returns the current background color.
     */
    TQColor background() const;
    /**
     * Returns the current color item selected by the user.
     */
    DualColor current() const;
    /**
     * Returns the color of the selected item.
     */
    TQColor currentColor() const;
    /**
     * Returns the minimum size needed to display the widget and all its
     * controls.
     */
    virtual TQSize sizeHint() const;
public slots:
    /**
     * Sets the foreground color.
     */
    void setForeground(const TQColor &c);
    /**
     * Sets the background color.
     */
    void setBackground(const TQColor &c);
    /**
     * Sets the current selected color item.
     */
    void setCurrent(DualColor s);
    /**
     * Sets the color of the selected item.
     */
    void setCurrentColor(const TQColor &c);
signals:
    /**
     * Emitted when the foreground color is changed.
     */
    void fgChanged(const TQColor &c);
    /**
     * Emitted when the background color is changed.
     */
    void bgChanged(const TQColor &c);
    /**
     * Emitted when the user changes the current color selection.
     */
    void currentChanged(KDualColorButton::DualColor s);
protected:
    /**
     * Sets the supplied rectangles to the proper size and position for the
     * current widget size. You can reimplement this to change the layout
     * of the widget. Restrictions are that the swap control will always
     * be at the top right, the reset control will always be at the bottom
     * left, and you must leave at least a 14x14 space in those corners.
     */
    virtual void metrics(TQRect &fgRect, TQRect &bgRect);
    virtual void paintEvent(TQPaintEvent *ev);
    virtual void mousePressEvent(TQMouseEvent *ev);
    virtual void mouseMoveEvent(TQMouseEvent *ev);
    virtual void mouseReleaseEvent(TQMouseEvent *ev);
    // Dnd
    virtual void dragEnterEvent(TQDragEnterEvent *ev);
    virtual void dropEvent(TQDropEvent *ev);
private:
    TQBitmap *arrowBitmap;
    TQPixmap *resetPixmap;
    TQBrush fg, bg;
    TQPoint mPos;
    bool dragFlag, miniCtlFlag;
    DualColor curColor, tmpColor;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KDualColorPrivate;
    KDualColorPrivate *d;
};

#endif
