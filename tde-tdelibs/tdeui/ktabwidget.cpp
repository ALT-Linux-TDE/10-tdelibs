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

#include <tqapplication.h>
#include <tqstyle.h>
#include <tqstylesheet.h>

#include <tdeconfig.h>
#include <kiconloader.h>
#include <kstringhandler.h>

#include "ktabwidget.h"
#include "ktabbar.h"

class KTabWidgetPrivate {
public:
    bool m_automaticResizeTabs;
    int  m_maxLength;
    int  m_minLength;
    unsigned int m_CurrentMaxLength;
    bool m_mouseWheelScroll;

    // Holds the full names of the tab, otherwise all we know about is the shortened name
    TQStringList m_tabNames;

    KTabWidgetPrivate() : m_automaticResizeTabs(false), m_mouseWheelScroll(true)
    {
        TDEConfigGroupSaver groupsaver(TDEGlobal::config(), "General");
        m_maxLength = TDEGlobal::config()->readNumEntry("MaximumTabLength", 30);
        m_minLength = TDEGlobal::config()->readNumEntry("MinimumTabLength", 3);
        m_CurrentMaxLength = m_minLength;
    }
};

KTabWidget::KTabWidget( TQWidget *parent, const char *name, WFlags f )
    : TQTabWidget( parent, name, f )
{
    d = new KTabWidgetPrivate;
    setTabBar( new KTabBar(this, "tabbar") );
    setAcceptDrops( true );

    setHoverCloseButtonDelayed(false);

    connect(tabBar(), TQ_SIGNAL(contextMenu( int, const TQPoint & )), TQ_SLOT(contextMenu( int, const TQPoint & )));
    connect(tabBar(), TQ_SIGNAL(mouseDoubleClick( int )), TQ_SLOT(mouseDoubleClick( int )));
    connect(tabBar(), TQ_SIGNAL(mouseMiddleClick( int )), TQ_SLOT(mouseMiddleClick( int )));
    connect(tabBar(), TQ_SIGNAL(initiateDrag( int )), TQ_SLOT(initiateDrag( int )));
    connect(tabBar(), TQ_SIGNAL(testCanDecode(const TQDragMoveEvent *, bool & )), TQ_SIGNAL(testCanDecode(const TQDragMoveEvent *, bool & )));
    connect(tabBar(), TQ_SIGNAL(receivedDropEvent( int, TQDropEvent * )), TQ_SLOT(receivedDropEvent( int, TQDropEvent * )));
    connect(tabBar(), TQ_SIGNAL(moveTab( int, int )), TQ_SLOT(moveTab( int, int )));
    connect(tabBar(), TQ_SIGNAL(closeRequest( int )), TQ_SLOT(closeRequest( int )));
#ifndef TQT_NO_WHEELEVENT
    connect(tabBar(), TQ_SIGNAL(wheelDelta( int )), TQ_SLOT(wheelDelta( int )));
#endif
}

KTabWidget::~KTabWidget()
{
    delete d;
}

void KTabWidget::insertTab( TQWidget *child, const TQString &label, int index )
{
    TQTabWidget::insertTab( child, label, index );
}

void KTabWidget::insertTab( TQWidget *child, const TQIconSet& iconset, const TQString &label, int index )
{
    TQTabWidget::insertTab( child, iconset, label, index );
}

void KTabWidget::insertTab( TQWidget *child, TQTab *tab, int index )
{
    TQTabWidget::insertTab( child, tab, index);
    if ( d->m_automaticResizeTabs ) {
        if ( index < 0 || index >= count() ) {
            d->m_tabNames.append( tab->text() );
            resizeTabs( d->m_tabNames.count()-1 );
        }
        else {
            d->m_tabNames.insert( d->m_tabNames.at( index ), tab->text() );
            resizeTabs( index );
        }
    }
}

void KTabWidget::setTabBarHidden( bool hide )
{
    TQWidget *rightcorner = this->cornerWidget( TopRight );
    TQWidget *leftcorner = this->cornerWidget( TopLeft );

    if ( hide ) {
        if ( leftcorner ) leftcorner->hide();
        if ( rightcorner ) rightcorner->hide();
        tabBar()->hide();
    } else {
        tabBar()->show();
        if ( leftcorner ) leftcorner->show();
        if ( rightcorner ) rightcorner->show();
    }
}

bool KTabWidget::isTabBarHidden() const
{
    return !( tabBar()->isVisible() );
}

void KTabWidget::setMouseWheelScroll(bool mouseWheelScroll)
{
  d->m_mouseWheelScroll = mouseWheelScroll;
}

void KTabWidget::setTabColor( TQWidget *w, const TQColor& color )
{
    TQTab *t = tabBar()->tabAt( indexOf( w ) );
    if (t) {
        static_cast<KTabBar*>(tabBar())->setTabColor( t->identifier(), color );
    }
}

void KTabWidget::resetTabColor( TQWidget *w )
{
    TQTab *t = tabBar()->tabAt( indexOf( w ) );
    if (t) {
        static_cast<KTabBar*>(tabBar())->resetTabColor( t->identifier() );
    }
}

TQColor KTabWidget::tabColor( TQWidget *w ) const
{
    TQTab *t = tabBar()->tabAt( indexOf( w ) );
    if (t) {
        return static_cast<KTabBar*>(tabBar())->tabColor( t->identifier() );
    } else {
        return TQColor();
    }
}

void KTabWidget::setTabReorderingEnabled( bool on)
{
    static_cast<KTabBar*>(tabBar())->setTabReorderingEnabled( on );
}

bool KTabWidget::isTabReorderingEnabled() const
{
    return static_cast<KTabBar*>(tabBar())->isTabReorderingEnabled();
}

void KTabWidget::setTabCloseActivatePrevious( bool previous)
{
    static_cast<KTabBar*>(tabBar())->setTabCloseActivatePrevious( previous );
}

bool KTabWidget::tabCloseActivatePrevious() const
{
    return static_cast<KTabBar*>(tabBar())->tabCloseActivatePrevious();
}

unsigned int KTabWidget::tabBarWidthForMaxChars( uint maxLength )
{
    int hframe, overlap;
    hframe  = tabBar()->style().pixelMetric( TQStyle::PM_TabBarTabHSpace, tabBar() );
    overlap = tabBar()->style().pixelMetric( TQStyle::PM_TabBarTabOverlap, tabBar() );

    TQFontMetrics fm = tabBar()->fontMetrics();
    int x = 0;
    for( int i=0; i < count(); ++i ) {
        TQString newTitle = d->m_tabNames[ i ];
        newTitle = KStringHandler::rsqueeze( newTitle, maxLength ).leftJustify( d->m_minLength, ' ' );

        TQTab* tab = tabBar()->tabAt( i );
        int lw = fm.width( newTitle );
        int iw = 0;
        if ( tab->iconSet() )
          iw = tab->iconSet()->pixmap( TQIconSet::Small, TQIconSet::Normal ).width() + 4;
        x += ( tabBar()->style().sizeFromContents( TQStyle::CT_TabBarTab, this,
                   TQSize( TQMAX( lw + hframe + iw, TQApplication::globalStrut().width() ), 0 ),
                   TQStyleOption( tab ) ) ).width();
    }
    return x;
}

void KTabWidget::changeTab( TQWidget *w, const TQString &label )
{
    TQTabWidget::changeTab( w, label );
    if ( d->m_automaticResizeTabs ) {
        int index = indexOf( w );
        if ( index != -1 ) {
            d->m_tabNames[ index ] = label;
            resizeTabs( index );
        }
    }
}

void KTabWidget::changeTab( TQWidget *w, const TQIconSet &iconset, const TQString &label )
{
    TQTabWidget::changeTab( w, iconset, label );
    if ( d->m_automaticResizeTabs ) {
        int index = indexOf( w );
        if ( index != -1 ) {
            d->m_tabNames[ index ] = label;
            resizeTabs( index );
        }
    }
}

TQString KTabWidget::label( int index ) const
{
    if ( d->m_automaticResizeTabs ) {
        if ( index >= 0 && index < count() )
            return d->m_tabNames[ index ];
        else
            return TQString::null;
    }
    else
        return TQTabWidget::label( index );
}

TQString KTabWidget::tabLabel( TQWidget * w ) const
{
    if ( d->m_automaticResizeTabs ) {
        int index = indexOf( w );
        if ( index == -1 )
            return TQString::null;
        else
            return d->m_tabNames[ index ];
    }
    else
        return TQTabWidget::tabLabel( w );
}

void KTabWidget::setTabLabel( TQWidget *w, const TQString &l )
{
    TQTabWidget::setTabLabel( w, l );
    if ( d->m_automaticResizeTabs ) {
        int index = indexOf( w );
        if ( index != -1 ) {
            d->m_tabNames[ index ] = l;
            resizeTabs( index );
        }
  }
}

void KTabWidget::resizeTabs( int changeTabIndex )
{
    uint newMaxLength;
    if ( d->m_automaticResizeTabs ) {
        // Calculate new max length
        newMaxLength=d->m_maxLength;
        uint lcw=0, rcw=0;

        int tabBarHeight = tabBar()->sizeHint().height();
        if ( cornerWidget( TopLeft ) && cornerWidget( TopLeft )->isVisible() )
            lcw = TQMAX( cornerWidget( TopLeft )->width(), tabBarHeight );
        if ( cornerWidget( TopRight ) && cornerWidget( TopRight )->isVisible() )
            rcw = TQMAX( cornerWidget( TopRight )->width(), tabBarHeight );

        uint maxTabBarWidth = width() - lcw - rcw;

        for ( ; newMaxLength > (uint)d->m_minLength; newMaxLength-- ) {
            if ( tabBarWidthForMaxChars( newMaxLength ) < maxTabBarWidth )
                break;
        }
    }
    else
        newMaxLength = 4711;

    // Update hinted or all tabs
    if ( d->m_CurrentMaxLength != newMaxLength ) {
        d->m_CurrentMaxLength = newMaxLength;
        for( int i = 0; i < count(); ++i )
            updateTab( i );
    }
    else if ( changeTabIndex != -1 )
        updateTab( changeTabIndex );
}

void KTabWidget::updateTab( int index )
{
    TQString title = d->m_automaticResizeTabs ? d->m_tabNames[ index ] : TQTabWidget::label( index );
    removeTabToolTip( page( index ) );
    if ( title.length() > d->m_CurrentMaxLength ) {
        if ( TQStyleSheet::mightBeRichText( title ) )
            setTabToolTip( page( index ), TQStyleSheet::escape(title) );
        else
            setTabToolTip( page( index ), title );
    }

    title = KStringHandler::rsqueeze( title, d->m_CurrentMaxLength ).leftJustify( d->m_minLength, ' ' );
    title.replace( '&', "&&" );

    if ( TQTabWidget::label( index ) != title )
        TQTabWidget::setTabLabel( page( index ), title );
}

void KTabWidget::dragMoveEvent( TQDragMoveEvent *e )
{
    if ( isEmptyTabbarSpace( e->pos() ) ) {
        bool accept = false;
        // The receivers of the testCanDecode() signal has to adjust
        // 'accept' accordingly.
        emit testCanDecode( e, accept);
        e->accept( accept );
        return;
    }
    e->accept( false );
    TQTabWidget::dragMoveEvent( e );
}

void KTabWidget::dropEvent( TQDropEvent *e )
{
    if ( isEmptyTabbarSpace( e->pos() ) ) {
        emit ( receivedDropEvent( e ) );
        return;
    }
    TQTabWidget::dropEvent( e );
}

#ifndef TQT_NO_WHEELEVENT
void KTabWidget::wheelEvent( TQWheelEvent *e )
{
    if ( e->orientation() == TQt::Horizontal )
        return;

    if ( isEmptyTabbarSpace( e->pos() ) )
        wheelDelta( e->delta() );
    else
        e->ignore();
}

void KTabWidget::wheelDelta(int delta)
{
    if (count()<2 || !d->m_mouseWheelScroll)
      return;

    int page = currentPageIndex();
    if (delta<0)
       page = (page + 1) % count();
    else
    {
      page--;
      if (page<0)
        page = count() - 1;
    }
    setCurrentPage(page);
}
#endif

void KTabWidget::mouseDoubleClickEvent( TQMouseEvent *e )
{
    if( e->button() != TQt::LeftButton )
        return;

    if ( isEmptyTabbarSpace( e->pos() ) ) {
        emit( mouseDoubleClick() );
        return;
    }
    TQTabWidget::mouseDoubleClickEvent( e );
}

void KTabWidget::mousePressEvent( TQMouseEvent *e )
{
    if ( e->button() == TQt::RightButton ) {
        if ( isEmptyTabbarSpace( e->pos() ) ) {
            emit( contextMenu( mapToGlobal( e->pos() ) ) );
            return;
        }
    } else if ( e->button() == TQt::MidButton ) {
        if ( isEmptyTabbarSpace( e->pos() ) ) {
            emit( mouseMiddleClick() );
            return;
        }
    }
    TQTabWidget::mousePressEvent( e );
}

void KTabWidget::receivedDropEvent( int index, TQDropEvent *e )
{
    emit( receivedDropEvent( page( index ), e ) );
}

void KTabWidget::initiateDrag( int index )
{
    emit( initiateDrag( page( index ) ) );
}

void KTabWidget::contextMenu( int index, const TQPoint &p )
{
    emit( contextMenu( page( index ), p ) );
}

void KTabWidget::mouseDoubleClick( int index )
{
    emit( mouseDoubleClick( page( index ) ) );
}

void KTabWidget::mouseMiddleClick( int index )
{
    emit( mouseMiddleClick( page( index ) ) );
}

void KTabWidget::moveTab( int from, int to )
{
    TQString tablabel = label( from );
    TQWidget *w = page( from );
    TQColor color = tabColor( w );
    TQIconSet tabiconset = tabIconSet( w );
    TQString tabtooltip = tabToolTip( w );
    bool current = ( w == currentPage() );
    bool enabled = isTabEnabled( w );
    blockSignals(true);
    removePage( w );

    // Work-around tdemdi brain damage which calls showPage() in insertTab()
    TQTab * t = new TQTab();
    t->setText(tablabel);
    TQTabWidget::insertTab( w, t, to );
    if ( d->m_automaticResizeTabs ) {
        if ( to < 0 || to >= count() )
            d->m_tabNames.append( TQString::null );
        else
            d->m_tabNames.insert( d->m_tabNames.at( to ), TQString::null );
    }

    w = page( to );
    changeTab( w, tabiconset, tablabel );
    setTabToolTip( w, tabtooltip );
    setTabColor( w, color );
    if ( current )
        showPage( w );
    setTabEnabled( w, enabled );
    blockSignals(false);

    emit ( movedTab( from, to ) );
}

void KTabWidget::removePage( TQWidget * w ) {
    if ( d->m_automaticResizeTabs ) {
        int index = indexOf( w );
        if ( index != -1 )
            d->m_tabNames.remove( d->m_tabNames.at( index ) );
    }
    TQTabWidget::removePage( w );
    if ( d->m_automaticResizeTabs )
        resizeTabs();
}


bool KTabWidget::isEmptyTabbarSpace( const TQPoint &point ) const
{
    TQSize size( tabBar()->sizeHint() );
    if ( ( tabPosition()==Top && point.y()< size.height() ) || ( tabPosition()==Bottom && point.y()>(height()-size.height() ) ) ) {
        TQWidget *rightcorner = cornerWidget( TopRight );
        if ( rightcorner ) {
            if ( point.x()>=width()-rightcorner->width() )
                return false;
        }
        TQWidget *leftcorner = cornerWidget( TopLeft );
        if ( leftcorner ) {
            if ( point.x()<=leftcorner->width() )
                return false;
        }
        TQTab *tab = tabBar()->selectTab( tabBar()->mapFromParent( point ) );
        if( !tab )
            return true;
    }
    return false;
}

void KTabWidget::setHoverCloseButton( bool button )
{
    static_cast<KTabBar*>(tabBar())->setHoverCloseButton( button );
}

bool KTabWidget::hoverCloseButton() const
{
    return static_cast<KTabBar*>(tabBar())->hoverCloseButton();
}

void KTabWidget::setHoverCloseButtonDelayed( bool delayed )
{
    static_cast<KTabBar*>(tabBar())->setHoverCloseButtonDelayed( delayed );
}

bool KTabWidget::hoverCloseButtonDelayed() const
{
    return static_cast<KTabBar*>(tabBar())->hoverCloseButtonDelayed();
}

void KTabWidget::setAutomaticResizeTabs( bool enabled )
{
    if ( d->m_automaticResizeTabs==enabled )
        return;

    d->m_automaticResizeTabs = enabled;
    if ( enabled ) {
        d->m_tabNames.clear();
        for( int i = 0; i < count(); ++i )
            d->m_tabNames.append( tabBar()->tabAt( i )->text() );
    }
    else
        for( int i = 0; i < count(); ++i )
            tabBar()->tabAt( i )->setText( d->m_tabNames[ i ] );
    resizeTabs();
}

bool KTabWidget::automaticResizeTabs() const
{
    return d->m_automaticResizeTabs;
}

void KTabWidget::closeRequest( int index )
{
    emit( closeRequest( page( index ) ) );
}

void KTabWidget::resizeEvent( TQResizeEvent *e )
{
    TQTabWidget::resizeEvent( e );
    resizeTabs();
}

#include "ktabwidget.moc"
