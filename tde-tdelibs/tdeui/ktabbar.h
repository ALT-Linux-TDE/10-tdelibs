/* This file is part of the KDE libraries
    Copyright (C) 2003 Stephan Binner <binner@kde.org>
    Copyright (C) 2003 Zack Rusin <zack@kde.org>

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

#ifndef KTABBAR_H
#define KTABBAR_H

#include <tqtabbar.h>

#include <tdelibs_export.h>

class TQTimer;
class TQPushButton;
class KTabBarPrivate;

/**
 * @since 3.2
 */
class TDEUI_EXPORT KTabBar: public TQTabBar
{
    TQ_OBJECT

public:
    KTabBar( TQWidget* parent=0, const char* name=0 );
    virtual ~KTabBar();

    virtual void setTabEnabled( int, bool );

    const TQColor &tabColor( int ) const;
    void setTabColor( int, const TQColor& );
    void resetTabColor( int );

    virtual int insertTab( TQTab *, int index = -1 );
    virtual void removeTab( TQTab * );

    void setTabReorderingEnabled( bool enable );
    bool isTabReorderingEnabled() const;

    void setHoverCloseButton( bool );
    bool hoverCloseButton() const;

    void setHoverCloseButtonDelayed( bool );
    bool hoverCloseButtonDelayed() const;

    void setTabCloseActivatePrevious( bool );
    bool tabCloseActivatePrevious() const;

signals:
    void contextMenu( int, const TQPoint & );
    void mouseDoubleClick( int );
    void mouseMiddleClick( int );
    void initiateDrag( int );
    void testCanDecode(const TQDragMoveEvent *e, bool &accept /* result */);
    void receivedDropEvent( int, TQDropEvent * );
    void moveTab( int, int );
    void closeRequest( int );
#ifndef TQT_NO_WHEELEVENT
    void wheelDelta( int );
#endif

protected:
    virtual void mouseDoubleClickEvent( TQMouseEvent *e );
    virtual void mousePressEvent( TQMouseEvent *e );
    virtual void mouseMoveEvent( TQMouseEvent *e );
    virtual void mouseReleaseEvent( TQMouseEvent *e );
#ifndef TQT_NO_WHEELEVENT
    virtual void wheelEvent( TQWheelEvent *e );
#endif

    virtual void dragMoveEvent( TQDragMoveEvent *e );
    virtual void dropEvent( TQDropEvent *e );

    virtual void paintLabel( TQPainter*, const TQRect&, TQTab*, bool ) const;

protected slots:
    virtual void closeButtonClicked();
    virtual void onLayoutChange();
    virtual void enableCloseButton();
    virtual void activateDragSwitchTab();

private:
    TQPoint mDragStart;
    int mReorderStartTab;
    int mReorderPreviousTab;
    TQMap<int, TQColor> mTabColors;
    TQTab *mHoverCloseButtonTab, *mDragSwitchTab;
    TQPushButton *mHoverCloseButton;
    TQTimer *mEnableCloseButtonTimer, *mActivateDragSwitchTabTimer;

    bool mHoverCloseButtonEnabled;
    bool mHoverCloseButtonDelayed;
    bool mTabReorderingEnabled;
    bool mTabCloseActivatePrevious;

    KTabBarPrivate * d;
};

#endif
