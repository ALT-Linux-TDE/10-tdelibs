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
#include <tqcursor.h>
#include <tqpainter.h>
#include <tqstyle.h>
#include <tqtimer.h>
#include <tqpushbutton.h>
#include <tqtooltip.h>

#include <tdeglobalsettings.h>
#include <kiconloader.h>
#include <tdelocale.h>

#include "ktabbar.h"
#include "ktabwidget.h"

KTabBar::KTabBar( TQWidget *parent, const char *name )
    : TQTabBar( parent, name ), mReorderStartTab( -1 ), mReorderPreviousTab( -1 ),
      mHoverCloseButtonTab( 0 ), mDragSwitchTab( 0 ), mHoverCloseButton( 0 ),
      mHoverCloseButtonEnabled( false ), mHoverCloseButtonDelayed( true ),
      mTabReorderingEnabled( false ), mTabCloseActivatePrevious( false )
{
    setAcceptDrops( true );
    setMouseTracking( true );

    mEnableCloseButtonTimer = new TQTimer( this );
    connect( mEnableCloseButtonTimer, TQ_SIGNAL( timeout() ), TQ_SLOT( enableCloseButton() ) );

    mActivateDragSwitchTabTimer = new TQTimer( this );
    connect( mActivateDragSwitchTabTimer, TQ_SIGNAL( timeout() ), TQ_SLOT( activateDragSwitchTab() ) );

    connect(this, TQ_SIGNAL(layoutChanged()), TQ_SLOT(onLayoutChange()));
}

KTabBar::~KTabBar()
{
    //For the future
    //delete d;
}

void KTabBar::setTabEnabled( int id, bool enabled )
{
    TQTab * t = tab( id );
    if ( t ) {
        if ( t->isEnabled() != enabled ) {
            t->setEnabled( enabled );
            TQRect r( t->rect() );
            if ( !enabled && id == currentTab() && count()>1 ) {
                TQPtrList<TQTab> *tablist = tabList();
                if ( mTabCloseActivatePrevious )
                    t = tablist->at( count()-2 );
                else {
                int index = indexOf( id );
                index += ( index+1 == count() ) ? -1 : 1;
                t = tabAt( index );
                }

                if ( t->isEnabled() ) {
                    r = r.unite( t->rect() );
                    tablist->append( tablist->take( tablist->findRef( t ) ) );
                    emit selected( t->identifier() );
                }
            }
            repaint( r );
        }
    }
}

void KTabBar::mouseDoubleClickEvent( TQMouseEvent *e )
{
    if( e->button() != TQt::LeftButton )
        return;

    TQTab *tab = selectTab( e->pos() );
    if( tab ) {
        emit( mouseDoubleClick( indexOf( tab->identifier() ) ) );
        return;
    }
    TQTabBar::mouseDoubleClickEvent( e );
}

void KTabBar::mousePressEvent( TQMouseEvent *e )
{
    if( e->button() == TQt::LeftButton ) {
        mEnableCloseButtonTimer->stop();
        mDragStart = e->pos();
    }
    else if( e->button() == TQt::RightButton ) {
        TQTab *tab = selectTab( e->pos() );
        if( tab ) {
            emit( contextMenu( indexOf( tab->identifier() ), mapToGlobal( e->pos() ) ) );
            return;
        }
    }
    TQTabBar::mousePressEvent( e );
}

void KTabBar::mouseMoveEvent( TQMouseEvent *e )
{
    if ( e->state() == TQt::LeftButton ) {
        TQTab *tab = selectTab( e->pos() );
        if ( mDragSwitchTab && tab != mDragSwitchTab ) {
          mActivateDragSwitchTabTimer->stop();
          mDragSwitchTab = 0;
        }

        int delay = TDEGlobalSettings::dndEventDelay();
        TQPoint newPos = e->pos();
        if( newPos.x() > mDragStart.x()+delay || newPos.x() < mDragStart.x()-delay ||
            newPos.y() > mDragStart.y()+delay || newPos.y() < mDragStart.y()-delay )
         {
            if( tab ) {
                emit( initiateDrag( indexOf( tab->identifier() ) ) );
                return;
           }
       }
    }
    else if ( e->state() == TQt::MidButton ) {
        if (mReorderStartTab==-1) {
            int delay = TDEGlobalSettings::dndEventDelay();
            TQPoint newPos = e->pos();
            if( newPos.x() > mDragStart.x()+delay || newPos.x() < mDragStart.x()-delay ||
                newPos.y() > mDragStart.y()+delay || newPos.y() < mDragStart.y()-delay )
            {
                TQTab *tab = selectTab( e->pos() );
                if( tab && mTabReorderingEnabled ) {
                    mReorderStartTab = indexOf( tab->identifier() );
                    grabMouse( TQt::sizeAllCursor );
                    return;
                }
            }
        }
        else {
            TQTab *tab = selectTab( e->pos() );
            if( tab ) {
                int reorderStopTab = indexOf( tab->identifier() );
                if ( mReorderStartTab!=reorderStopTab && mReorderPreviousTab!=reorderStopTab ) {
                    emit( moveTab( mReorderStartTab, reorderStopTab ) );
                    mReorderPreviousTab=mReorderStartTab;
                    mReorderStartTab=reorderStopTab;
                    return;
                }
            }
        }
    }

    if ( mHoverCloseButtonEnabled && mReorderStartTab==-1) {
        TQTab *t = selectTab( e->pos() );
        if( t && t->iconSet() && t->isEnabled() ) {
            TQPixmap pixmap = t->iconSet()->pixmap( TQIconSet::Small, TQIconSet::Normal );
            TQRect rect( 0, 0, pixmap.width() + 4, pixmap.height() +4);

            int xoff = 0, yoff = 0;
            // The additional offsets were found by try and error, TODO: find the rational behind them
            if ( t == tab( currentTab() ) ) {
                xoff = style().pixelMetric( TQStyle::PM_TabBarTabShiftHorizontal, this ) + 3;
                yoff = style().pixelMetric( TQStyle::PM_TabBarTabShiftVertical, this ) - 4;
            }
            else {
                xoff = 7;
                yoff = 0;
            }
            rect.moveLeft( t->rect().left() + 2 + xoff );
            rect.moveTop( t->rect().center().y()-pixmap.height()/2 + yoff );
            if ( rect.contains( e->pos() ) ) {
                if ( mHoverCloseButton ) {
                    if ( mHoverCloseButtonTab == t )
                        return;
                    mEnableCloseButtonTimer->stop();
                    mHoverCloseButton->deleteLater();
                    mHoverCloseButton = 0;
                }

                mHoverCloseButton = new TQPushButton( this );
                mHoverCloseButton->setIconSet( TDEGlobal::iconLoader()->loadIconSet("window-close", TDEIcon::Toolbar, TDEIcon::SizeSmall) );
                mHoverCloseButton->setGeometry( rect );
                TQToolTip::add(mHoverCloseButton,i18n("Close this tab"));
                mHoverCloseButton->setFlat(true);
                mHoverCloseButton->show();
                if ( mHoverCloseButtonDelayed ) {
                  mHoverCloseButton->setEnabled(false);
                  mEnableCloseButtonTimer->start( TQApplication::doubleClickInterval(), true );
                }
                mHoverCloseButtonTab = t;
                connect( mHoverCloseButton, TQ_SIGNAL( clicked() ), TQ_SLOT( closeButtonClicked() ) );
                return;
            }
        }
        if ( mHoverCloseButton ) {
            mEnableCloseButtonTimer->stop();
            mHoverCloseButton->deleteLater();
            mHoverCloseButton = 0;
        }
    }

    TQTabBar::mouseMoveEvent( e );
}

void KTabBar::enableCloseButton()
{
    mHoverCloseButton->setEnabled(true);
}

void KTabBar::activateDragSwitchTab()
{
    TQTab *tab = selectTab( mapFromGlobal( TQCursor::pos() ) );
    if ( tab && mDragSwitchTab == tab )
    setCurrentTab( mDragSwitchTab );
    mDragSwitchTab = 0;
}

void KTabBar::mouseReleaseEvent( TQMouseEvent *e )
{
    if( e->button() == TQt::MidButton ) {
        if ( mReorderStartTab==-1 ) {
            TQTab *tab = selectTab( e->pos() );
            if( tab ) {
                emit( mouseMiddleClick( indexOf( tab->identifier() ) ) );
                return;
            }
        }
        else {
            releaseMouse();
            setCursor( TQt::arrowCursor );
            mReorderStartTab=-1;
            mReorderPreviousTab=-1;
        }
    }
    TQTabBar::mouseReleaseEvent( e );
}

void KTabBar::dragMoveEvent( TQDragMoveEvent *e )
{
    TQTab *tab = selectTab( e->pos() );
    if( tab ) {
        bool accept = false;
        // The receivers of the testCanDecode() signal has to adjust
        // 'accept' accordingly.
        emit testCanDecode( e, accept);
        if ( accept && tab != TQTabBar::tab( currentTab() ) ) {
          mDragSwitchTab = tab;
          mActivateDragSwitchTabTimer->start( TQApplication::doubleClickInterval()*2, true );
        }
        e->accept( accept );
        return;
    }
    e->accept( false );
    TQTabBar::dragMoveEvent( e );
}

void KTabBar::dropEvent( TQDropEvent *e )
{
    TQTab *tab = selectTab( e->pos() );
    if( tab ) {
        mActivateDragSwitchTabTimer->stop();
        mDragSwitchTab = 0;
        emit( receivedDropEvent( indexOf( tab->identifier() ) , e ) );
        return;
    }
    TQTabBar::dropEvent( e );
}

#ifndef TQT_NO_WHEELEVENT
void KTabBar::wheelEvent( TQWheelEvent *e )
{
    if ( e->orientation() == TQt::Horizontal )
        return;

    emit( wheelDelta( e->delta() ) );
}
#endif

void KTabBar::setTabColor( int id, const TQColor& color )
{
    TQTab *t = tab( id );
    if ( t ) {
        mTabColors.insert( id, color );
        repaint( t->rect(), false );
    }
}

void KTabBar::resetTabColor( int id )
{
    TQTab *t = tab(id);
    if (t) {
        if (mTabColors.contains(id))
            mTabColors.remove(id);
        repaint(t->rect(), false);
    }
}

const TQColor &KTabBar::tabColor( int id  ) const
{
    if ( mTabColors.contains(id) && mTabColors[id].isValid() )
        return mTabColors[id];

    return colorGroup().foreground();
}

int KTabBar::insertTab( TQTab *t, int index )
{
    int res = TQTabBar::insertTab( t, index );

    if ( mTabCloseActivatePrevious && count() > 2 ) {
        TQPtrList<TQTab> *tablist = tabList();
        tablist->insert( count()-2, tablist->take( tablist->findRef( t ) ) );
    }

    return res;
}

void KTabBar::removeTab( TQTab *t )
{
    mTabColors.remove( t->identifier() );
    TQTabBar::removeTab( t );
}

void KTabBar::paintLabel( TQPainter *p, const TQRect& br,
                          TQTab *t, bool has_focus ) const
{
    TQRect r = br;
    bool selected = currentTab() == t->identifier();
    if ( t->iconSet() ) {
        // the tab has an iconset, draw it in the right mode
        TQIconSet::Mode mode = ( t->isEnabled() && isEnabled() )
                                 ? TQIconSet::Normal : TQIconSet::Disabled;
        if ( mode == TQIconSet::Normal && has_focus )
            mode = TQIconSet::Active;
        TQPixmap pixmap = t->iconSet()->pixmap( TQIconSet::Small, mode );
        int pixw = pixmap.width();
        int pixh = pixmap.height();
        r.setLeft( r.left() + pixw + 4 );
        r.setRight( r.right() + 2 );

        int inactiveXShift = style().pixelMetric( TQStyle::PM_TabBarTabShiftHorizontal, this );
        int inactiveYShift = style().pixelMetric( TQStyle::PM_TabBarTabShiftVertical, this );

        int right = t->text().isEmpty() ? br.right() - pixw : br.left() + 2;

        p->drawPixmap( right + (selected ? 0 : inactiveXShift),
                       br.center().y() - pixh / 2 + (selected ? 0 : inactiveYShift),
                       pixmap );
    }

    TQStyle::SFlags flags = TQStyle::Style_Default;

    if ( isEnabled() && t->isEnabled() )
        flags |= TQStyle::Style_Enabled;
    if ( has_focus )
        flags |= TQStyle::Style_HasFocus;

    TQColorGroup cg( colorGroup() );
    if ( mTabColors.contains(t->identifier()) && mTabColors[t->identifier()].isValid() )
        cg.setColor( TQColorGroup::Foreground, mTabColors[t->identifier()] );

    style().drawControl( TQStyle::CE_TabBarLabel, p, this, r,
                             t->isEnabled() ? cg : palette().disabled(),
                             flags, TQStyleOption(t) );
}

bool KTabBar::isTabReorderingEnabled() const
{
    return mTabReorderingEnabled;
}

void KTabBar::setTabReorderingEnabled( bool on )
{
    mTabReorderingEnabled = on;
}

bool KTabBar::tabCloseActivatePrevious() const
{
    return mTabCloseActivatePrevious;
}

void KTabBar::setTabCloseActivatePrevious( bool on )
{
    mTabCloseActivatePrevious = on;
}

void KTabBar::closeButtonClicked()
{
    emit closeRequest( indexOf( mHoverCloseButtonTab->identifier() ) );
}

void KTabBar::setHoverCloseButton( bool button )
{
    mHoverCloseButtonEnabled = button;
    if ( !button )
        onLayoutChange();
}

bool KTabBar::hoverCloseButton() const
{
    return mHoverCloseButtonEnabled;
}

void KTabBar::setHoverCloseButtonDelayed( bool delayed )
{
    mHoverCloseButtonDelayed = delayed;
}

bool KTabBar::hoverCloseButtonDelayed() const
{
    return mHoverCloseButtonDelayed;
}

void KTabBar::onLayoutChange()
{
    mEnableCloseButtonTimer->stop();
    delete mHoverCloseButton;
    mHoverCloseButton = 0;
    mHoverCloseButtonTab = 0;
    mActivateDragSwitchTabTimer->stop();
    mDragSwitchTab = 0;
}

#include "ktabbar.moc"
