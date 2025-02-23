/*  This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
    Copyright (C) 1999 Cristian Tibirna (ctibirna@kde.org)

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

#include <config.h>

#include <tqpainter.h>
#include <tqdrawutil.h>
#include <tqapplication.h>
#include <tqclipboard.h>
#include <tqstyle.h>
#include <tdeglobalsettings.h>
#include <tdestdaccel.h>
#include "kcolordialog.h"
#include "kcolorbutton.h"
#include "kcolordrag.h"

class KColorButton::KColorButtonPrivate
{
public:
    bool m_bdefaultColor;
    TQColor m_defaultColor;
};

KColorButton::KColorButton( TQWidget *parent, const char *name )
  : TQPushButton( parent, name )
{
  d = new KColorButtonPrivate;
  d->m_bdefaultColor = false;
  d->m_defaultColor = TQColor();
  setAcceptDrops( true);

  // 2000-10-15 (putzer): fixes broken keyboard usage
  connect (this, TQ_SIGNAL(clicked()), this, TQ_SLOT(chooseColor()));
}

KColorButton::KColorButton( const TQColor &c, TQWidget *parent,
			    const char *name )
  : TQPushButton( parent, name ), col(c)
{
  d = new KColorButtonPrivate;
  d->m_bdefaultColor = false;
  d->m_defaultColor = TQColor();
  setAcceptDrops( true);

  // 2000-10-15 (putzer): fixes broken keyboard usage
  connect (this, TQ_SIGNAL(clicked()), this, TQ_SLOT(chooseColor()));
}

KColorButton::KColorButton( const TQColor &c, const TQColor &defaultColor, TQWidget *parent,
			    const char *name )
  : TQPushButton( parent, name ), col(c)
{
  d = new KColorButtonPrivate;
  d->m_bdefaultColor = true;
  d->m_defaultColor = defaultColor;
  setAcceptDrops( true);

  // 2000-10-15 (putzer): fixes broken keyboard usage
  connect (this, TQ_SIGNAL(clicked()), this, TQ_SLOT(chooseColor()));
}

KColorButton::~KColorButton()
{
  delete d;
}

void KColorButton::setColor( const TQColor &c )
{
  if ( col != c ) {
    col = c;
    repaint( false );
    emit changed( col );
  }
}

TQColor KColorButton::defaultColor() const
{
  return d->m_defaultColor;
}

void KColorButton::setDefaultColor( const TQColor &c )
{
  d->m_bdefaultColor = c.isValid();
  d->m_defaultColor = c;
}


void KColorButton::drawButtonLabel( TQPainter *painter )
{
  int x, y, w, h;
  TQRect r = style().subRect( TQStyle::SR_PushButtonContents, this );
  r.rect(&x, &y, &w, &h);

  int margin = style().pixelMetric( TQStyle::PM_ButtonMargin, this );
  x += margin;
  y += margin;
  w -= 2*margin;
  h -= 2*margin;

  if (isOn() || isDown()) {
    x += style().pixelMetric( TQStyle::PM_ButtonShiftHorizontal, this );
    y += style().pixelMetric( TQStyle::PM_ButtonShiftVertical, this );
  }

  TQColor fillCol = isEnabled() ? col : backgroundColor();
  qDrawShadePanel( painter, x, y, w, h, colorGroup(), true, 1, NULL);
  if ( fillCol.isValid() )
    painter->fillRect( x+1, y+1, w-2, h-2, fillCol );

  if ( hasFocus() ) {
    TQRect focusRect = style().subRect( TQStyle::SR_PushButtonFocusRect, this );
    style().drawPrimitive( TQStyle::PE_FocusRect, painter, focusRect, colorGroup() );
  }
}

TQSize KColorButton::sizeHint() const
{
  return style().sizeFromContents(TQStyle::CT_PushButton, this, TQSize(40, 15)).
	  	expandedTo(TQApplication::globalStrut());
}

void KColorButton::dragEnterEvent( TQDragEnterEvent *event)
{
  event->accept( KColorDrag::canDecode( event) && isEnabled());
}

void KColorButton::dropEvent( TQDropEvent *event)
{
  TQColor c;
  if( KColorDrag::decode( event, c)) {
    setColor(c);
  }
}

void KColorButton::keyPressEvent( TQKeyEvent *e )
{
  KKey key( e );

  if ( TDEStdAccel::copy().contains( key ) ) {
    TQMimeSource* mime = new KColorDrag( color() );
    TQApplication::clipboard()->setData( mime, TQClipboard::Clipboard );
  }
  else if ( TDEStdAccel::paste().contains( key ) ) {
    TQColor color;
    KColorDrag::decode( TQApplication::clipboard()->data( TQClipboard::Clipboard ), color );
    setColor( color );
  }
  else
    TQPushButton::keyPressEvent( e );
}

void KColorButton::mousePressEvent( TQMouseEvent *e)
{
  mPos = e->pos();
  TQPushButton::mousePressEvent(e);
}

void KColorButton::mouseMoveEvent( TQMouseEvent *e)
{
  if( (e->state() & TQt::LeftButton) &&
    (e->pos()-mPos).manhattanLength() > TDEGlobalSettings::dndEventDelay() )
  {
    // Drag color object
    KColorDrag *dg = new KColorDrag( color(), this);
    dg->dragCopy();
    setDown(false);
  }
}

void KColorButton::chooseColor()
{
  TQColor c = color();
  if ( d->m_bdefaultColor )
  {
      if( KColorDialog::getColor( c, d->m_defaultColor, this ) != TQDialog::Rejected ) {
          setColor( c );
      }
  }
  else
  {
      if( KColorDialog::getColor( c, this ) != TQDialog::Rejected ) {
          setColor( c );
      }
  }
}

void KColorButton::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kcolorbutton.moc"
