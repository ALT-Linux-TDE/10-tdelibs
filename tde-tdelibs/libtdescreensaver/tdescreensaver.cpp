/* This file is part of the KDE libraries

    Copyright (c) 2001  Martin R. Jones <mjones@kde.org>

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

#include <tqpainter.h>
#include <tqtimer.h>
#include <tdeapplication.h>
#include "tdescreensaver.h"
#ifdef TQ_WS_X11
#include <X11/Xlib.h>
#else
typedef WId Window;
#endif

#undef KeyPress
#undef KeyRelease

//-----------------------------------------------------------------------------

class KScreenSaverPrivate
{
public:
    TQWidget *owner;
};

KScreenSaver::KScreenSaver( WId id ) : TQWidget()
{
    Window root;
    int ai;
    unsigned int au;
    unsigned int w = 0;
    unsigned int h = 0;

    XSync(tqt_xdisplay(), false);

    d = new KScreenSaverPrivate;
    d->owner = find( id );
    if ( d->owner )
	installEventFilter( this );

    if ( id )
    {
#ifdef TQ_WS_X11 //FIXME
        XGetGeometry(tqt_xdisplay(), (Drawable)id, &root, &ai, &ai,
            &w, &h, &au, &au); 
#endif

        create( id, false, true );
    }

    if ( w == 0 ) w = 600;
    if ( h == 0 ) h = 420;
    resize( w, h );
    TDEApplication::sendPostedEvents();
    show();
}

KScreenSaver::~KScreenSaver()
{
    destroy( false, false );
    delete d;
}

void KScreenSaver::embed( TQWidget *w )
{
    TDEApplication::sendPostedEvents();
#ifdef TQ_WS_X11 //FIXME
    XReparentWindow(tqt_xdisplay(), w->winId(), winId(), 0, 0);
#endif
    w->setGeometry( 0, 0, width(), height() );
    TDEApplication::sendPostedEvents();
}

bool KScreenSaver::eventFilter( TQObject *o, TQEvent *e )
{
    // make sure events get to the original window owner
    if ( d->owner && o == this ) {
	TQApplication::sendEvent( d->owner, e );
	return false;
    }

    return TQWidget::eventFilter( o, e );
}

//============================================================================

class KBlankEffectPrivate
{
public:
    KBlankEffect::BlankEffect currentEffect;
    int effectProgress;
    TQTimer *timer;
    TQWidget *widget;
};

KBlankEffect::BlankEffect KBlankEffect::effects[] = {
    &KBlankEffect::blankNormal,
    &KBlankEffect::blankSweepRight,
    &KBlankEffect::blankSweepDown,
    &KBlankEffect::blankBlocks
};

KBlankEffect::KBlankEffect( TQObject *parent ) : TQObject( parent )
{
    d = new KBlankEffectPrivate;
    d->currentEffect = &KBlankEffect::blankNormal;
    d->effectProgress = 0;
    d->timer = new TQTimer( this );
    connect( d->timer, TQ_SIGNAL(timeout()), this, TQ_SLOT(timeout()) );
}


KBlankEffect::~KBlankEffect()
{
    delete d;
}

void KBlankEffect::finished()
{
    d->timer->stop();
    d->effectProgress = 0;
    emit doneBlank();
}


void KBlankEffect::blank( TQWidget *w, Effect effect )
{
    if ( !w ) {
        emit doneBlank();
        return;
    }

    if ( effect == Random )
        effect = (Effect)(kapp->random() % MaximumEffects);

    d->effectProgress = 0;
    d->widget = w;
    d->currentEffect = effects[ (int)effect ];
    d->timer->start( 10 );
}

void KBlankEffect::timeout()
{
    (this->*d->currentEffect)();
}

void KBlankEffect::blankNormal()
{
    TQPainter p( d->widget );
    p.fillRect( 0, 0, d->widget->width(), d->widget->height(), black );
    finished();
}


void KBlankEffect::blankSweepRight()
{
    TQPainter p( d->widget );
    p.fillRect( d->effectProgress, 0, 50, d->widget->height(), black );
    kapp->flushX();
    d->effectProgress += 50;
    if ( d->effectProgress >= d->widget->width() )
        finished();
}


void KBlankEffect::blankSweepDown()
{
    TQPainter p( d->widget );
    p.fillRect( 0, d->effectProgress, d->widget->width(), 50, black );
    kapp->flushX();
    d->effectProgress += 50;
    if ( d->effectProgress >= d->widget->height() )
        finished();
}


void KBlankEffect::blankBlocks()
{
    static int *block = 0;

    int bx = (d->widget->width()+63)/64;
    int by = (d->widget->height()+63)/64;

    if ( !d->effectProgress ) {
        block = new int [ bx*by ];

        for ( int i = 0; i < bx*by; i++ )
            block[i] = i;
        for ( int i = 0; i < bx*by; i++ ) {
            int swap = kapp->random()%(bx*by);
            int tmp = block[i];
            block[i] = block[swap];
            block[swap] = tmp;
        }
    }

    TQPainter p( d->widget );

    // erase a couple of blocks at a time, otherwise it looks too slow
    for ( int i = 0; i < 2 && d->effectProgress < bx*by; i++ ) {
        int x = block[d->effectProgress]%bx;
        int y = block[d->effectProgress]/bx;
        p.fillRect( x*64, y*64, 64, 64, black );
        d->effectProgress++;
    }

    kapp->flushX();

    if ( d->effectProgress >= bx*by ) {
        delete[] block;
        finished();
    }
}

#include "tdescreensaver.moc"
