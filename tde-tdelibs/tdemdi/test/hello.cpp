/****************************************************************************
** $Id$
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "hello.h"
#include <tqpushbutton.h>
#include <tqtimer.h>
#include <tqpainter.h>
#include <tqpixmap.h>

/*
  Constructs a Hello widget. Starts a 40 ms animation timer.
*/

Hello::Hello( const char *title, const char *text, TQWidget *parentWidget )
    : KMdiChildView(title, parentWidget), t(text), b(0)
{
    TQTimer *timer = new TQTimer(this);
    TQObject::connect( timer, TQ_SIGNAL(timeout()), TQ_SLOT(animate()) );
    timer->start( 40 );

    resize( 260, 130 );
}


/*
  This private slot is called each time the timer fires.
*/

void Hello::animate()
{
    b = (b + 1) & 15;
    repaint( false );
}


/*
  Handles mouse button release events for the Hello widget.

  We emit the clicked() signal when the mouse is released inside
  the widget.
*/

void Hello::mouseReleaseEvent( TQMouseEvent *e )
{
    if ( rect().contains( e->pos() ) )
        emit clicked();
}


/*
  Handles paint events for the Hello widget.

  Flicker-free update. The text is first drawn in the pixmap and the
  pixmap is then blt'ed to the screen.
*/

void Hello::paintEvent( TQPaintEvent * )
{
    static int sin_tbl[16] = {
        0, 38, 71, 92, 100, 92, 71, 38,	0, -38, -71, -92, -100, -92, -71, -38};

    if ( t.isEmpty() )
        return;

    // 1: Compute some sizes, positions etc.
    TQFontMetrics fm = fontMetrics();
    int w = fm.width(t) + 20;
    int h = fm.height() * 2;
    int pmx = width()/2 - w/2;
    int pmy = height()/2 - h/2;

    // 2: Create the pixmap and fill it with the widget's background
    TQPixmap pm( w, h );
    pm.fill( this, pmx, pmy );

    // 3: Paint the pixmap. Cool wave effect
    TQPainter p;
    int x = 10;
    int y = h/2 + fm.descent();
    int i = 0;
    p.begin( &pm );
    p.setFont( font() );
    while ( !t[i].isNull() ) {
        int i16 = (b+i) & 15;
        p.setPen( TQColor((15-i16)*16,255,255,TQColor::Hsv) );
        p.drawText( x, y-sin_tbl[i16]*h/800, t.mid(i,1), 1 );
        x += fm.width( t[i] );
        i++;
    }
    p.end();

    // 4: Copy the pixmap to the Hello widget
    bitBlt( this, pmx, pmy, &pm );
}

#include "hello.moc"

