/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

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

#include "config.h"

#include <tqtimer.h>
#include <tqpainter.h>
#include <tqpixmapcache.h>
#include <tqcleanuphandler.h>

#include "kiconview.h"
#include "kwordwrap.h"
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <tdeapplication.h>
#include <kipc.h> 

#include <kcursor.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>

class TDEIconView::TDEIconViewPrivate
{
public:
    TDEIconViewPrivate() {
        mode = TDEIconView::Execute;
        fm = 0L;
        doAutoSelect = true;
        textHeight = 0;
        dragHoldItem = 0L;
    }
    TDEIconView::Mode mode;
    bool doAutoSelect;
    TQFontMetrics *fm;
    TQPixmapCache maskCache;
    int textHeight;
    TQIconViewItem *dragHoldItem;
    TQTimer dragHoldTimer;
    TQTimer doubleClickIgnoreTimer;
};

TDEIconView::TDEIconView( TQWidget *parent, const char *name, WFlags f )
    : TQIconView( parent, name, f )
{
    d = new TDEIconViewPrivate;

    connect( this, TQ_SIGNAL( onViewport() ),
             this, TQ_SLOT( slotOnViewport() ) );
    connect( this, TQ_SIGNAL( onItem( TQIconViewItem * ) ),
             this, TQ_SLOT( slotOnItem( TQIconViewItem * ) ) );
    slotSettingsChanged( TDEApplication::SETTINGS_MOUSE );
    if ( kapp ) { // maybe null when used inside designer
        connect( kapp, TQ_SIGNAL( settingsChanged(int) ), TQ_SLOT( slotSettingsChanged(int) ) );
        kapp->addKipcEventMask( KIPC::SettingsChanged );
    }

    m_pCurrentItem = 0L;

    m_pAutoSelect = new TQTimer( this );
    connect( m_pAutoSelect, TQ_SIGNAL( timeout() ),
             this, TQ_SLOT( slotAutoSelect() ) );

    connect( &d->dragHoldTimer, TQ_SIGNAL(timeout()), this, TQ_SLOT(slotDragHoldTimeout()) );
}

TDEIconView::~TDEIconView()
{
    delete d->fm;
    delete d;
}


void TDEIconView::setMode( TDEIconView::Mode mode )
{
    d->mode = mode;
}

TDEIconView::Mode TDEIconView::mode() const
{
    return d->mode;
}

void TDEIconView::slotOnItem( TQIconViewItem *item )
{
    if ( item ) {
        if ( m_bUseSingle ) {
            if ( m_bChangeCursorOverItem )
                viewport()->setCursor( KCursor().handCursor() );

            if ( (m_autoSelectDelay > -1) ) {
                m_pAutoSelect->start( m_autoSelectDelay, true );
            }
        }
        m_pCurrentItem = item;
    }
}

void TDEIconView::slotOnViewport()
{
    if ( m_bUseSingle && m_bChangeCursorOverItem )
        viewport()->unsetCursor();

    m_pAutoSelect->stop();
    m_pCurrentItem = 0L;
}

void TDEIconView::slotSettingsChanged(int category)
{
    if ( category != TDEApplication::SETTINGS_MOUSE )
      return;
    m_bUseSingle = TDEGlobalSettings::singleClick();
    //kdDebug() << "TDEIconView::slotSettingsChanged for mouse, usesingle=" << m_bUseSingle << endl;

    disconnect( this, TQ_SIGNAL( mouseButtonClicked( int, TQIconViewItem *,
						  const TQPoint & ) ),
		this, TQ_SLOT( slotMouseButtonClicked( int, TQIconViewItem *,
						    const TQPoint & ) ) );
//         disconnect( this, TQ_SIGNAL( doubleClicked( TQIconViewItem *,
// 						 const TQPoint & ) ),
// 		    this, TQ_SLOT( slotExecute( TQIconViewItem *,
// 					     const TQPoint & ) ) );

    if( m_bUseSingle ) {
      connect( this, TQ_SIGNAL( mouseButtonClicked( int, TQIconViewItem *,
						 const TQPoint & ) ),
	       this, TQ_SLOT( slotMouseButtonClicked( int, TQIconViewItem *,
						   const TQPoint & ) ) );
    }
    else {
//         connect( this, TQ_SIGNAL( doubleClicked( TQIconViewItem *,
// 					      const TQPoint & ) ),
//                  this, TQ_SLOT( slotExecute( TQIconViewItem *,
// 					  const TQPoint & ) ) );
    }

    m_bChangeCursorOverItem = TDEGlobalSettings::changeCursorOverIcon();
    m_autoSelectDelay = m_bUseSingle ? TDEGlobalSettings::autoSelectDelay() : -1;

    if( !m_bUseSingle || !m_bChangeCursorOverItem )
        viewport()->unsetCursor();
}

void TDEIconView::slotAutoSelect()
{
  // check that the item still exists
  if( index( m_pCurrentItem ) == -1 || !d->doAutoSelect )
    return;

  //Give this widget the keyboard focus.
  if( !hasFocus() )
    setFocus();

  ButtonState keybstate = TDEApplication::keyboardMouseState();
  TQIconViewItem* previousItem = currentItem();
  setCurrentItem( m_pCurrentItem );

  if( m_pCurrentItem ) {
    //Shift pressed?
    if( (keybstate & ShiftButton) ) {
      //Temporary implementation of the selection until TQIconView supports it
      bool block = signalsBlocked();
      blockSignals( true );

      //No Ctrl? Then clear before!
      if( !(keybstate & ControlButton) )
	clearSelection();

      bool select = !m_pCurrentItem->isSelected();
      bool update = viewport()->isUpdatesEnabled();
      viewport()->setUpdatesEnabled( false );

      //Calculate the smallest rectangle that contains the current Item
      //and the one that got the autoselect event
      TQRect r;
      TQRect redraw;
      if ( previousItem )
	r = TQRect( TQMIN( previousItem->x(), m_pCurrentItem->x() ),
		   TQMIN( previousItem->y(), m_pCurrentItem->y() ),
		   0, 0 );
      else
	r = TQRect( 0, 0, 0, 0 );
      if ( previousItem->x() < m_pCurrentItem->x() )
	r.setWidth( m_pCurrentItem->x() - previousItem->x() + m_pCurrentItem->width() );
      else
	r.setWidth( previousItem->x() - m_pCurrentItem->x() + previousItem->width() );
      if ( previousItem->y() < m_pCurrentItem->y() )
	r.setHeight( m_pCurrentItem->y() - previousItem->y() + m_pCurrentItem->height() );
      else
	r.setHeight( previousItem->y() - m_pCurrentItem->y() + previousItem->height() );
      r = r.normalize();

      //Check for each item whether it is within the rectangle.
      //If yes, select it
      for( TQIconViewItem* i = firstItem(); i; i = i->nextItem() ) {
	if( i->intersects( r ) ) {
	  redraw = redraw.unite( i->rect() );
	  setSelected( i, select, true );
	}
      }

      blockSignals( block );
      viewport()->setUpdatesEnabled( update );
      repaintContents( redraw, false );

      emit selectionChanged();

      if( selectionMode() == TQIconView::Single )
	emit selectionChanged( m_pCurrentItem );

      //setSelected( m_pCurrentItem, true, (keybstate & ControlButton), (keybstate & ShiftButton) );
    }
    else if( (keybstate & ControlButton) )
      setSelected( m_pCurrentItem, !m_pCurrentItem->isSelected(), true );
    else
      setSelected( m_pCurrentItem, true );
  }
  else
    kdDebug() << "TDEIconView: That's not supposed to happen!!!!" << endl;
}

void TDEIconView::emitExecute( TQIconViewItem *item, const TQPoint &pos )
{
  if ( d->mode != Execute )
  {
    // kdDebug() << "TDEIconView::emitExecute : not in execute mode !" << endl;
    return;
  }

  ButtonState keybstate = TDEApplication::keyboardMouseState();

  m_pAutoSelect->stop();

  //Don't emit executed if in SC mode and Shift or Ctrl are pressed
  if( !( m_bUseSingle && ((keybstate & ShiftButton) || (keybstate & ControlButton)) ) ) {
    setSelected( item, false );
    viewport()->unsetCursor();
    emit executed( item );
    emit executed( item, pos );
  }
}

void TDEIconView::updateDragHoldItem( TQDropEvent *e )
{
  TQIconViewItem *item = findItem( e->pos() );

  if ( d->dragHoldItem != item)
  {
    d->dragHoldItem = item;
    if( item  )
    {
      d->dragHoldTimer.start( 1000, true );
    }
    else
    {
      d->dragHoldTimer.stop();
    }
  }
}

void TDEIconView::focusOutEvent( TQFocusEvent *fe )
{
  m_pAutoSelect->stop();

  TQIconView::focusOutEvent( fe );
}

void TDEIconView::leaveEvent( TQEvent *e )
{
  m_pAutoSelect->stop();

  TQIconView::leaveEvent( e );
}

void TDEIconView::contentsMousePressEvent( TQMouseEvent *e )
{
  if( (selectionMode() == Extended) && (e->state() & ShiftButton) && !(e->state() & ControlButton) ) {
    bool block = signalsBlocked();
    blockSignals( true );

    clearSelection();

    blockSignals( block );
  }

  TQIconView::contentsMousePressEvent( e );
  d->doAutoSelect = false;
}

void TDEIconView::contentsMouseDoubleClickEvent ( TQMouseEvent * e )
{
  TQIconView::contentsMouseDoubleClickEvent( e );

  TQIconViewItem* item = findItem( e->pos() );

  if( item ) {
    if( (e->button() == TQt::LeftButton) && !m_bUseSingle )
      emitExecute( item, e->globalPos() );

    emit doubleClicked( item, e->globalPos() );
  }
  d->doubleClickIgnoreTimer.start(0, true);
}

void TDEIconView::slotMouseButtonClicked( int btn, TQIconViewItem *item, const TQPoint &pos )
{
  //kdDebug() << " TDEIconView::slotMouseButtonClicked() item=" << item << endl;
  if( d->doubleClickIgnoreTimer.isActive() )
    return; // Ignore double click
    
  if( (btn == TQt::LeftButton) && item )
    emitExecute( item, pos );
}

void TDEIconView::contentsMouseReleaseEvent( TQMouseEvent *e )
{
    d->doAutoSelect = true;
    TQIconView::contentsMouseReleaseEvent( e );
}

void TDEIconView::contentsDragEnterEvent( TQDragEnterEvent *e )
{
    updateDragHoldItem( e );
    TQIconView::contentsDragEnterEvent( e );
}

void TDEIconView::contentsDragLeaveEvent( TQDragLeaveEvent *e )
{
    d->dragHoldTimer.stop();
    d->dragHoldItem = 0L;
    TQIconView::contentsDragLeaveEvent( e );
}


void TDEIconView::contentsDragMoveEvent( TQDragMoveEvent *e )
{
    updateDragHoldItem( e );
    TQIconView::contentsDragMoveEvent( e );
}

void TDEIconView::contentsDropEvent( TQDropEvent* e )
{
    d->dragHoldTimer.stop();
    TQIconView::contentsDropEvent( e );
}

void TDEIconView::slotDragHoldTimeout()
{
    TQIconViewItem *tmp = d->dragHoldItem;
    d->dragHoldItem = 0L;

    emit held( tmp );
}

void TDEIconView::takeItem( TQIconViewItem * item )
{
    if ( item == d->dragHoldItem )
    {
        d->dragHoldTimer.stop();
        d->dragHoldItem = 0L;
    }

    TQIconView::takeItem( item );
}

void TDEIconView::cancelPendingHeldSignal()
{
    d->dragHoldTimer.stop();
    d->dragHoldItem = 0L;
}

void TDEIconView::wheelEvent( TQWheelEvent *e )
{
    if (horizontalScrollBar() && (arrangement() == TQIconView::TopToBottom)) {
        TQWheelEvent ce(e->pos(), e->delta(), e->state(), TQt::Horizontal);
        TQApplication::sendEvent( horizontalScrollBar(), &ce);
	if (ce.isAccepted()) {
            e->accept();
	    return;
	}
    }
    TQIconView::wheelEvent(e);
}

void TDEIconView::setFont( const TQFont &font )
{
    delete d->fm;
    d->fm = 0L;
    TQIconView::setFont( font );
}

TQFontMetrics *TDEIconView::itemFontMetrics() const
{
    if (!d->fm) {
        // TQIconView creates one too, but we can't access it
        d->fm = new TQFontMetrics( font() );
    }
    return d->fm;
}

TQPixmap TDEIconView::selectedIconPixmap( TQPixmap *pix, const TQColor &col ) const
{
    TQPixmap m;
    if ( d->maskCache.find( TQString::number( pix->serialNumber() ), m ) )
	return m;
    m = KPixmapEffect::selectedPixmap( KPixmap(*pix), col );
    d->maskCache.insert( TQString::number( pix->serialNumber() ), m );
    return m;
}

int TDEIconView::iconTextHeight() const
{
    return d->textHeight > 0 ? d->textHeight : ( wordWrapIconText() ? 99 : 1 );
}

void TDEIconView::setIconTextHeight( int n )
{
    int oldHeight = iconTextHeight();
    if ( n > 1 )
        d->textHeight = n;
    else
        d->textHeight = 1;

    // so that Qt still shows the tooltip when even a wrapped text is too long
    setWordWrapIconText( false );

    // update view if needed
    if ( iconTextHeight() != oldHeight )
        setFont( font() );  // hack to recalc items
}

/////////////

class TDEIconViewItem::TDEIconViewItemPrivate
{
    public:
        TDEIconViewItemPrivate() {
            m_pixmapSize = TQSize(0,0);
        }

    public:
        TQSize m_pixmapSize;
        int realTextHeight;
};

void TDEIconViewItem::init()
{
    m_wordWrap = 0L;
    d = 0L;
    calcRect();
}

TDEIconViewItem::~TDEIconViewItem()
{
    delete m_wordWrap;
    if (d) {
        delete d;
    }
}

void TDEIconViewItem::calcRect( const TQString& text_ )
{
    if ( !d ) {
        d = new TDEIconViewItemPrivate;
    }
    d->realTextHeight = -1;

    bool drawRoundedRect = TDEGlobalSettings::iconUseRoundedRect();

    Q_ASSERT( iconView() );
    if ( !iconView() )
        return;
    delete m_wordWrap;
    m_wordWrap = 0L;
#ifndef NDEBUG // be faster for the end-user, such a bug will have been fixed before hand :)
    if ( !iconView()->inherits("TDEIconView") )
    {
        kdWarning() << "TDEIconViewItem used in a " << iconView()->className() << " !!" << endl;
        return;
    }
#endif
    //kdDebug() << "TDEIconViewItem::calcRect - " << text() << endl;
    TDEIconView *view = static_cast<TDEIconView *>(iconView());
    TQRect itemIconRect = pixmapRect();
    TQRect itemTextRect = textRect();
    TQRect itemRect = rect();

    int pw = 0;
    int ph = 0;

#ifndef TQT_NO_PICTURE
    if ( picture() ) {
        TQRect br = picture()->boundingRect();
        pw = br.width() + 2;
        ph = br.height() + 2;
    } else
#endif
    {
        // Qt uses unknown_icon if no pixmap. Let's see if we need that - I doubt it
        if (!pixmap())
            return;
        pw = pixmap()->width() + 2;
        ph = pixmap()->height() + 2;
    }
    itemIconRect.setWidth( pw );
#if 1 // FIXME 
    // There is a bug in Qt which prevents the item from being placed
    // properly when the pixmapRect is not at the top of the itemRect, so we
    // have to increase the height of the pixmapRect and leave it at the top
    // of the itemRect...
    if ( d && !d->m_pixmapSize.isNull() ) {
        itemIconRect.setHeight( d->m_pixmapSize.height() + 2 );
    }
    else
#endif
    itemIconRect.setHeight( ph );

    int tw = 0;
    if ( d && !d->m_pixmapSize.isNull() ) {
        tw = view->maxItemWidth() - ( view->itemTextPos() == TQIconView::Bottom ? 0 :
                                      d->m_pixmapSize.width() + 2 );
    }
    else {
        tw = view->maxItemWidth() - ( view->itemTextPos() == TQIconView::Bottom ? 0 :
                                      itemIconRect.width() );
    }

    TQFontMetrics *fm = view->itemFontMetrics();
    TQString t;
    TQRect r;

    // When is text_ set ? Doesn't look like it's ever set.
    t = text_.isEmpty() ? text() : text_;

    // Max text height
    int nbLines = static_cast<TDEIconView*>( iconView() )->iconTextHeight();
    int height = nbLines > 0 ? fm->height() * nbLines : 0xFFFFFFFF;

    // Should not be higher than pixmap if text is alongside icons
    if ( view->itemTextPos() != TQIconView::Bottom ) {
        if ( d && !d->m_pixmapSize.isNull() )
            height = TQMIN( d->m_pixmapSize.height() + 2, height );
        else
            height = TQMIN( itemIconRect.height(), height );
        height = TQMAX( height, fm->height() );
    }

    // Calculate the word-wrap
    TQRect outerRect( 0, 0, tw - 6, height );
    m_wordWrap = KWordWrap::formatText( *fm, outerRect, 0, t );
    r = m_wordWrap->boundingRect();

    int realWidth = TQMAX( TQMIN( r.width() + 4, tw ), fm->width( "X" ) );
    if (drawRoundedRect == true) {
      itemTextRect.setWidth( realWidth + 2);
    }
    else {
      itemTextRect.setWidth( realWidth );
    }
    itemTextRect.setHeight( r.height() );

    int w = 0;    int h = 0;    int y = 0;
    if ( view->itemTextPos() == TQIconView::Bottom ) {
        // If the pixmap size has been specified, use it
        if ( d && !d->m_pixmapSize.isNull() )
        {
            w = TQMAX( itemTextRect.width(), d->m_pixmapSize.width() + 2 );
            h = itemTextRect.height() + d->m_pixmapSize.height() + 2 + 1;
#if 0 // FIXME 
            // Waiting for the qt bug to be solved, the pixmapRect must
            // stay on the top...
            y = d->m_pixmapSize.height() + 2 - itemIconRect.height();
#endif
        }
        else {
            w = TQMAX( itemTextRect.width(), itemIconRect.width() );
            h = itemTextRect.height() + itemIconRect.height() + 1;
        }

        itemRect.setWidth( w );
        itemRect.setHeight( h );
        int width = TQMAX( w, TQApplication::globalStrut().width() ); // see TQIconViewItem::width()
        int height = TQMAX( h, TQApplication::globalStrut().height() ); // see TQIconViewItem::height()
        itemTextRect = TQRect( ( width - itemTextRect.width() ) / 2, height - itemTextRect.height(),
                              itemTextRect.width(), itemTextRect.height() );
        itemIconRect = TQRect( ( width - itemIconRect.width() ) / 2, y,
                              itemIconRect.width(), itemIconRect.height() );
    }
    else {
        // If the pixmap size has been specified, use it
        if ( d && !d->m_pixmapSize.isNull() ) {
            h = TQMAX( itemTextRect.height(), d->m_pixmapSize.height() + 2 );
#if 0 // FIXME 
            // Waiting for the qt bug to be solved, the pixmapRect must
            // stay on the top...
            y = ( d->m_pixmapSize.height() + 2 - itemIconRect.height() ) / 2;
#endif
        }
        else {
            h = TQMAX( itemTextRect.height(), itemIconRect.height() );
        }
        w = itemTextRect.width() + itemIconRect.width() + 1;

        itemRect.setWidth( w );
        itemRect.setHeight( h );
        int width = TQMAX( w, TQApplication::globalStrut().width() ); // see TQIconViewItem::width()
        int height = TQMAX( h, TQApplication::globalStrut().height() ); // see TQIconViewItem::height()

        itemTextRect = TQRect( width - itemTextRect.width(), ( height - itemTextRect.height() ) / 2,
                              itemTextRect.width(), itemTextRect.height() );
        if ( itemIconRect.height() > itemTextRect.height() ) { // icon bigger than text -> center vertically
            itemIconRect = TQRect( 0, ( height - itemIconRect.height() ) / 2,
                                  itemIconRect.width(), itemIconRect.height() );
        }
        else { // icon smaller than text -> place in top or center with first line
	    itemIconRect = TQRect( 0, TQMAX(( fm->height() - itemIconRect.height() ) / 2 + y, 0),
                                  itemIconRect.width(), itemIconRect.height() );
        }
        if ( ( itemIconRect.height() <= 20 ) && ( itemTextRect.height() < itemIconRect.height() ) ) {
            d->realTextHeight = itemTextRect.height();
            itemTextRect.setY( itemIconRect.y() );
            itemTextRect.setHeight( itemIconRect.height() - 2 );
        }
    }

    if ( itemIconRect != pixmapRect() ) {
        setPixmapRect( itemIconRect );
    }
    if ( itemTextRect != textRect() ) {
        setTextRect( itemTextRect );
    }
    if ( itemRect != rect() ) {
        setItemRect( itemRect );
    }

    // Done by setPixmapRect, setTextRect and setItemRect !  [and useless if no rect changed]
    //view->updateItemContainer( this );

}

void TDEIconViewItem::paintItem( TQPainter *p, const TQColorGroup &cg )
{
    TQIconView* view = iconView();
    Q_ASSERT( view );
    if ( !view )
        return;
#ifndef NDEBUG // be faster for the end-user, such a bug will have been fixed before hand :)
    if ( !view->inherits("TDEIconView") )
    {
        kdWarning() << "TDEIconViewItem used in a " << view->className() << " !!" << endl;
        return;
    }
#endif

    p->save();

    paintPixmap(p, cg);
    paintText(p, cg);

    p->restore();
}

KWordWrap * TDEIconViewItem::wordWrap()
{
    return m_wordWrap;
}

void TDEIconViewItem::paintPixmap( TQPainter *p, const TQColorGroup &cg )
{
    TDEIconView *kview = static_cast<TDEIconView *>(iconView());

#ifndef TQT_NO_PICTURE
    if ( picture() ) {
	TQPicture *pic = picture();
	if ( isSelected() ) {
            // TODO something as nice as selectedIconPixmap if possible ;)
	    p->fillRect( pixmapRect( false ), TQBrush( cg.highlight(), TQBrush::Dense4Pattern) );
	}
	p->drawPicture( x()-pic->boundingRect().x(), y()-pic->boundingRect().y(), *pic );
    } else
#endif
    {
        int iconX = pixmapRect( false ).x();
        int iconY = pixmapRect( false ).y();

        TQPixmap *pix = pixmap();
        if ( !pix || pix->isNull() )
            return;

#if 1 // FIXME 
        // Move the pixmap manually because the pixmapRect is at the
        // top of the itemRect
        // (won't be needed anymore in future versions of qt)
        if ( d && !d->m_pixmapSize.isNull() )
        {
            int offset = 0;
            if ( kview->itemTextPos() == TQIconView::Bottom )
                offset = d->m_pixmapSize.height() - pix->height();
            else
                offset = ( d->m_pixmapSize.height() - pix->height() ) / 2;
            if ( offset > 0 )
                iconY += offset;
        }
#endif
        if ( isSelected() ) {
            TQPixmap selectedPix = kview->selectedIconPixmap( pix, cg.highlight() );
            p->drawPixmap( iconX, iconY, selectedPix );
        } else {
            p->drawPixmap( iconX, iconY, *pix );
        }
    }
}

void TDEIconViewItem::paintText( TQPainter *p, const TQColorGroup &cg )
{
    bool drawRoundedRect = TDEGlobalSettings::iconUseRoundedRect();
    int textX;
    if (drawRoundedRect == true) {
      textX = textRect( false ).x() + 4;
    }
    else {
      textX = textRect( false ).x() + 2;
    }
    int textY;
    if ( d && (d->realTextHeight != -1) ) {
      textY = textRect( false ).y() + ((rect().height() - d->realTextHeight) / 2);
    }
    else {
      textY = textRect( false ).y();
    }

    if ( isSelected() ) {
      if (drawRoundedRect == true) {
	p->setBrush(TQBrush(cg.highlight()));
	p->setPen(TQPen(cg.highlight()));
	p->drawRoundRect( textRect( false ) ,1000/textRect(false).width(),1000/textRect(false).height() );
      }
      else {
        p->fillRect( textRect( false ), cg.highlight() );
      }
        p->setPen( TQPen( cg.highlightedText() ) );
    }
    else {
        if ( iconView()->itemTextBackground() != TQt::NoBrush ) {
            p->fillRect( textRect( false ), iconView()->itemTextBackground() );
        }
        p->setPen( cg.text() );
    }

    int align = iconView()->itemTextPos() == TQIconView::Bottom ? AlignHCenter : AlignAuto;
    m_wordWrap->drawText( p, textX, textY, align | KWordWrap::Truncate );
}

TQSize TDEIconViewItem::pixmapSize() const
{
    return d ? d->m_pixmapSize : TQSize( 0, 0 );
}

void TDEIconViewItem::setPixmapSize( const TQSize& size )
{
    if ( !d ) {
        d = new TDEIconViewItemPrivate;
    }

    d->m_pixmapSize = size;
}

void TDEIconView::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kiconview.moc"
