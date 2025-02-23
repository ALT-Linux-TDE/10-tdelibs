/*
 *
 *
 * This file is part of the KDE libraries.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * Shared pixmap client for KDE.
 */
#include "config.h"

#include <tqrect.h>
#include <tqsize.h>
#include <tqstring.h>
#include <tqpixmap.h>
#include <tqwindowdefs.h>
#include <tqwidget.h>

#ifdef TQ_WS_X11

#include <tdeapplication.h>
#include <krootprop.h>
#include <ksharedpixmap.h>
#include <kdebug.h>
#include <stdlib.h> // for abs

#include <X11/Xlib.h> 

// Make sure to include all this X-based shit before we clean up the mess.
// Needed for --enable-final. Not needed by this file itself!
#include <X11/Xutil.h> 
#ifdef HAVE_MITSHM
#include <X11/extensions/XShm.h> 
#endif

#include <netwm.h> 

// Clean up the mess

#undef Bool
#undef Above
#undef Below
#undef KeyPress
#undef KeyRelease
#undef FocusOut

/**
 * TDESharedPixmap
 */

class TDESharedPixmapPrivate
{
public:
  Atom pixmap;
  Atom target;
  Atom selection;
  TQRect rect;
};

TDESharedPixmap::TDESharedPixmap()
    : TQWidget(0L, "shpixmap comm window")
{
    d = new TDESharedPixmapPrivate;
    init();
}


TDESharedPixmap::~TDESharedPixmap()
{
    delete d;
}


void TDESharedPixmap::init()
{
    d->pixmap = XInternAtom(tqt_xdisplay(), "PIXMAP", false);
    TQCString atom;
    atom.sprintf("target prop for window %lx", static_cast<unsigned long int>(winId()));
    d->target = XInternAtom(tqt_xdisplay(), atom.data(), false);
    d->selection = None;
}


bool TDESharedPixmap::isAvailable(const TQString & name) const
{
    TQString str = TQString("KDESHPIXMAP:%1").arg(name);
    Atom sel = XInternAtom(tqt_xdisplay(), str.latin1(), true);
    if (sel == None)
	return false;
    return XGetSelectionOwner(tqt_xdisplay(), sel) != None;
}


bool TDESharedPixmap::loadFromShared(const TQString & name, const TQRect & rect)
{
    d->rect = rect;
    if (d->selection != None)
	// already active
	return false;

    TQPixmap::resize(0, 0); // invalidate

    TQString str = TQString("KDESHPIXMAP:%1").arg(name);
    d->selection = XInternAtom(tqt_xdisplay(), str.latin1(), true);
    if (d->selection == None)
	return false;
    if (XGetSelectionOwner(tqt_xdisplay(), d->selection) == None)
    {
	d->selection = None;
	return false;
    }

    XConvertSelection(tqt_xdisplay(), d->selection, d->pixmap, d->target,
	    winId(), CurrentTime);
    return true;
}


bool TDESharedPixmap::x11Event(XEvent *event)
{
    if (event->type != SelectionNotify)
	return false;

    XSelectionEvent *ev = &event->xselection;
    if (ev->selection != d->selection)
	return false;

    if ((ev->target != d->pixmap) || (ev->property == None))
    {
	kdWarning(270) << k_funcinfo << "illegal selection notify event.\n";
	d->selection = None;
	emit done(false);
	return true;
    }

    // Read pixmap handle from ev->property

    int dummy, format;
    unsigned long nitems, ldummy;
    unsigned char *pixmap_id = 0;
    Atom type;

    XGetWindowProperty(tqt_xdisplay(), winId(), ev->property, 0, 1, false,
	    d->pixmap, &type, &format, &nitems, &ldummy,
	    &pixmap_id);

    if (nitems != 1 || !pixmap_id)
    {
	kdWarning(270) << k_funcinfo << "could not read property, nitems = " << nitems << "\n";
	emit done(false);
	return true;
    }

    Window root;
    unsigned int width, height, udummy;
    void *drawable_id = (void *) pixmap_id;
    Drawable pixmap = *(Drawable*) drawable_id;

    if (!XGetGeometry(tqt_xdisplay(), pixmap, &root, &dummy, &dummy, &width, &height, &udummy, &udummy)) {
        return false;
    }

    if (d->rect.isEmpty())
    {
	TQPixmap::resize(width, height);
	XCopyArea(tqt_xdisplay(), pixmap, ((KPixmap*)this)->handle(), tqt_xget_temp_gc(tqt_xscreen(), false),
		0, 0, width, height, 0, 0);

        XFree(pixmap_id);
	XDeleteProperty(tqt_xdisplay(), winId(), ev->property);
	d->selection = None;
	emit done(true);
	return true;
    }

    // Do some more processing here: Generate a tile that can be used as a
    // background tile for the rectangle "rect".

    //Check for origin off screen
    TQPoint origin(0, 0);
    if(  d->rect.topLeft().x() < 0 ||  d->rect.topLeft().y() < 0 ) {
        //Save the offset and relocate the rect corners
        TQPoint tl = d->rect.topLeft();
        TQPoint br = d->rect.bottomRight();
        if( tl.x() < 0 ) {
            origin.setX( abs( tl.x() ) );
            tl.setX( 0 );
        }
        if( tl.y() < 0 ) {
            origin.setY( abs( tl.y() ) );
            tl.setY( 0 );
        }
        TQRect adjustedRect( tl, br );
        d->rect = adjustedRect;
    }

    unsigned w = d->rect.width(), h = d->rect.height();
    unsigned tw = TQMIN(width, w), th = TQMIN(height, h);
    unsigned xa = d->rect.x() % width, ya = d->rect.y() % height;
    unsigned t1w = TQMIN(width-xa,tw), t1h = TQMIN(height-ya,th);

    TQPixmap::resize( tw+origin.x(), th+origin.y() );

    XCopyArea(tqt_xdisplay(), pixmap, (static_cast<KPixmap*>(this))->handle(), tqt_xget_temp_gc(tqt_xscreen(), false),
            xa, ya, t1w+origin.x(), t1h+origin.y(), origin.x(), origin.y() );
    XCopyArea(tqt_xdisplay(), pixmap, (static_cast<KPixmap*>(this))->handle(), tqt_xget_temp_gc(tqt_xscreen(), false),
	    0, ya, tw-t1w, t1h, t1w, 0);
    XCopyArea(tqt_xdisplay(), pixmap, (static_cast<KPixmap*>(this))->handle(), tqt_xget_temp_gc(tqt_xscreen(), false),
	    xa, 0, t1w, th-t1h, 0, t1h);
    XCopyArea(tqt_xdisplay(), pixmap, (static_cast<KPixmap*>(this))->handle(), tqt_xget_temp_gc(tqt_xscreen(), false),
	    0, 0, tw-t1w, th-t1h, t1w, t1h);

    XFree(pixmap_id);

    d->selection = None;
    XDeleteProperty(tqt_xdisplay(), winId(), ev->property);
    emit done(true);
    return true;
}


#include "ksharedpixmap.moc"
#endif
