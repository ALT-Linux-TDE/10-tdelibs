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

#ifndef KTABWIDGET_H
#define KTABWIDGET_H

#include <tqtabwidget.h>
#include <tqstringlist.h>

#include <tdelibs_export.h>

class KTabWidgetPrivate;

/**
 * \brief A widget containing multiple tabs
 *
 * @since 3.2
 */
class TDEUI_EXPORT KTabWidget : public TQTabWidget
{
    TQ_OBJECT
    TQ_PROPERTY( bool tabReorderingEnabled READ isTabReorderingEnabled WRITE setTabReorderingEnabled )
    TQ_PROPERTY( bool hoverCloseButton READ hoverCloseButton WRITE setHoverCloseButton )
    TQ_PROPERTY( bool hoverCloseButtonDelayed READ hoverCloseButtonDelayed WRITE setHoverCloseButtonDelayed )
    TQ_PROPERTY( bool tabCloseActivatePrevious READ tabCloseActivatePrevious WRITE setTabCloseActivatePrevious )
    TQ_PROPERTY( bool automaticResizeTabs READ automaticResizeTabs WRITE setAutomaticResizeTabs )

public:
    KTabWidget( TQWidget *parent = 0, const char *name = 0, WFlags f = 0 );

    /**
     * Destructor.
     */
    virtual ~KTabWidget();

    /*!
      Set the tab of the given widget to \a color.
    */
    void setTabColor( TQWidget *, const TQColor& color );

    /*!
      Reset the color of the tab of the given widget.
    */
    void resetTabColor( TQWidget * );

    /*!
      Returns the tab color for the given widget.
    */
    TQColor tabColor( TQWidget * ) const;

    /*!
      Returns true if tab ordering with the middle mouse button
      has been enabled.
    */
    bool isTabReorderingEnabled() const;

    /*!
      Returns true if the close button is shown on tabs
      when mouse is hovering over them.
    */
    bool hoverCloseButton() const;

    /*!
      Returns true if the close button is shown on tabs
      after a delay.
    */
    bool hoverCloseButtonDelayed() const;

    /*!
      Returns true if closing the current tab activates the previous
      actice tab instead of the one to the right.
      @since 3.3
    */
    bool tabCloseActivatePrevious() const;

    /*!
      Returns true if calling setTitle() will resize tabs
      to the width of the tab bar.
      @since 3.4
    */
    bool automaticResizeTabs() const;

    /*!
      If \a hide is true, the tabbar is hidden along with any corner
      widgets.
      @since 3.4
    */
    void setTabBarHidden( bool hide );

    /*!
      Returns true if the tabbar is hidden.
      @since 3.4
    */
    bool isTabBarHidden() const;

    /*!
      Enable/disable "scroll tabs on mouse wheel event" functionality
      \a mouseWheelScroll true -> scroll enabled, false -> scroll disabled
      @since 14.0
    */
    void setMouseWheelScroll(bool mouseWheelScroll);

    /*!
      Reimplemented for internal reasons.
     */
    virtual void insertTab( TQWidget *, const TQString &, int index = -1 );

    /*!
      Reimplemented for internal reasons.
     */
    virtual void insertTab( TQWidget *child, const TQIconSet& iconset,
                            const TQString &label, int index = -1 );
    /*!
      Reimplemented for internal reasons.
    */
    virtual void insertTab( TQWidget *, TQTab *, int index = -1 );

    /*!
      Reimplemented for internal reasons.
    */
    void changeTab( TQWidget *, const TQString & );

    /*!
      Reimplemented for internal reasons.
    */
    void changeTab( TQWidget *child, const TQIconSet& iconset, const TQString &label );

    /*!
      Reimplemented for internal reasons.
    */
    TQString label( int ) const;

    /*!
      Reimplemented for internal reasons.
    */
    TQString tabLabel( TQWidget * ) const;

    /*!
      Reimplemented for internal reasons.
    */
    void setTabLabel( TQWidget *, const TQString & );

public slots:
    /*!
      Move a widget's tab from first to second specified index and emit
      signal movedTab( int, int ) afterwards.
    */
    virtual void moveTab( int, int );

    /*!
      Removes the widget, reimplemented for
      internal reasons (keeping labels in sync).
    */
    virtual void removePage ( TQWidget * w );

    /*!
      If \a enable is true, tab reordering with middle button will be enabled.

      Note that once enabled you shouldn't rely on previously queried
      currentPageIndex() or indexOf( TQWidget * ) values anymore.

      You can connect to signal movedTab(int, int) which will notify
      you from which index to which index a tab has been moved.
    */
    void setTabReorderingEnabled( bool enable );

    /*!
      If \a enable is true, a close button will be shown on mouse hover
      over tab icons which will emit signal closeRequest( TQWidget * )
      when pressed.
    */
    void setHoverCloseButton( bool enable );

    /*!
      If \a delayed is true, a close button will be shown on mouse hover
      over tab icons after mouse double click delay else immediately.
    */
    void setHoverCloseButtonDelayed( bool delayed );

    /*!
      If \a previous is true, closing the current tab activates the
      previous active tab instead of the one to the right.
      @since 3.3
    */
    void setTabCloseActivatePrevious( bool previous );

    /*!
      If \a enable is true, tabs will be resized to the width of the tab bar.

      Does not work reliable with "TQTabWidget* foo=new KTabWidget()" and if
      you change tabs via the tabbar or by accessing tabs directly.
      @since 3.4
    */
    void setAutomaticResizeTabs( bool enable );

signals:
    /*!
      Connect to this and set accept to true if you can and want to decode the event.
    */
    void testCanDecode(const TQDragMoveEvent *e, bool &accept /* result */);

    /*!
      Received an event in the empty space beside tabbar. Usually creates a new tab.
      This signal is only possible after testCanDecode and positive accept result.
    */
    void receivedDropEvent( TQDropEvent * );

    /*!
      Received an drop event on given widget's tab.
      This signal is only possible after testCanDecode and positive accept result.
    */
    void receivedDropEvent( TQWidget *, TQDropEvent * );

    /*!
      Request to start a drag operation on the given tab.
    */
    void initiateDrag( TQWidget * );

    /*!
      The right mouse button was pressed over empty space besides tabbar.
    */
    void contextMenu( const TQPoint & );

    /*!
      The right mouse button was pressed over a widget.
    */
    void contextMenu( TQWidget *, const TQPoint & );

    /*!
      A tab was moved from first to second index. This signal is only
      possible after you have called setTabReorderingEnabled( true ).
    */
    void movedTab( int, int );

    /*!
      A double left mouse button click was performed over empty space besides tabbar.
      @since 3.3
    */
    void mouseDoubleClick();

    /*!
      A double left mouse button click was performed over the widget.
    */
    void mouseDoubleClick( TQWidget * );

    /*!
      A middle mouse button click was performed over empty space besides tabbar.
    */
    void mouseMiddleClick();

    /*!
      A middle mouse button click was performed over the widget.
    */
    void mouseMiddleClick( TQWidget * );

    /*!
      The close button of a widget's tab was clicked. This signal is
      only possible after you have called setHoverCloseButton( true ).
    */
    void closeRequest( TQWidget * );

protected:
    virtual void mouseDoubleClickEvent( TQMouseEvent *e );
    virtual void mousePressEvent( TQMouseEvent * );
    virtual void dragMoveEvent( TQDragMoveEvent * );
    virtual void dropEvent( TQDropEvent * );
    unsigned int tabBarWidthForMaxChars( uint maxLength );
#ifndef TQT_NO_WHEELEVENT
    virtual void wheelEvent( TQWheelEvent *e );
#endif
    virtual void resizeEvent( TQResizeEvent * );

protected slots:
    virtual void receivedDropEvent( int, TQDropEvent * );
    virtual void initiateDrag( int );
    virtual void contextMenu( int, const TQPoint & );
    virtual void mouseDoubleClick( int );
    virtual void mouseMiddleClick( int );
    virtual void closeRequest( int );
#ifndef TQT_NO_WHEELEVENT
    virtual void wheelDelta( int );
#endif

private:
    bool isEmptyTabbarSpace( const TQPoint & )  const;
    void resizeTabs( int changedTabIndex = -1 );
    void updateTab( int index );

    KTabWidgetPrivate *d;
};

#endif
