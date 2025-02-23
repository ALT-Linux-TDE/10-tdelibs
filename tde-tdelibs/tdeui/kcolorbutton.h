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

#ifndef __COLBTN_H__
#define __COLBTN_H__

#include <tqpushbutton.h>

#include <tdelibs_export.h>

class KColorButtonPrivate;
/**
* @short A pushbutton to display or allow user selection of a color.
*
* This widget can be used to display or allow user selection of a color.
*
* @see KColorDialog
*
* \image html kcolorbutton.png "KDE Color Button"
*/
class TDEUI_EXPORT KColorButton : public TQPushButton
{
    TQ_OBJECT
    TQ_PROPERTY( TQColor color READ color WRITE setColor )
    TQ_PROPERTY( TQColor defaultColor READ defaultColor WRITE setDefaultColor )

public:
    /**
     * Creates a color button.
     */
    KColorButton( TQWidget *parent, const char *name = 0L );

    /**
     * Creates a color button with an initial color @p c.
     */
    KColorButton( const TQColor &c, TQWidget *parent, const char *name = 0L );
    /// @since 3.1
    KColorButton( const TQColor &c, const TQColor &defaultColor, TQWidget *parent,
                  const char *name=0L );

    virtual ~KColorButton();

    /**
     * Returns the currently chosen color.
     */
    TQColor color() const
        { return col; }

    /**
     * Sets the current color to @p c.
     */
     void setColor( const TQColor &c );

    /**
     * Returns the default color or an invalid color
     * if no default color is set.
     * @since 3.4
     */
    TQColor defaultColor() const;

    /**
     * Sets the default color to @p c.
     * @since 3.4
     */
    void setDefaultColor( const TQColor &c );

    TQSize sizeHint() const;

signals:
    /**
     * Emitted when the color of the widget
     * is changed, either with setColor() or via user selection.
     */
    void changed( const TQColor &newColor );

protected slots:
    void chooseColor();

protected:
    virtual void drawButtonLabel( TQPainter *p );
    virtual void dragEnterEvent( TQDragEnterEvent *);
    virtual void dropEvent( TQDropEvent *);
    virtual void mousePressEvent( TQMouseEvent *e );
    virtual void mouseMoveEvent( TQMouseEvent *e);
    virtual void keyPressEvent( TQKeyEvent *e );
private:
    TQColor col;
    TQPoint mPos;
    bool dragFlag;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KColorButtonPrivate;
    KColorButtonPrivate *d;
};

#endif

