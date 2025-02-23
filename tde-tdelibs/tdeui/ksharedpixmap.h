/*
 *
 * $Id$
 *
 * This file is part of the KDE libraries.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */

#ifndef __TDESharedPixmap_h_Included__
#define __TDESharedPixmap_h_Included__

#include <kpixmap.h>

#ifdef TQ_WS_X11

#include <tqstring.h>
#include <tqpixmap.h>
#include <tqwidget.h>

class TDESharedPixmapPrivate;

/**
 * Shared pixmap client.
 *
 * A shared pixmap is a pixmap that resides on the X server, is referenced
 * by a global id string, and can be accessed by all X clients.
 *
 * This class is a client class to shared pixmaps in KDE. You can use it
 * to copy (a part of) a shared pixmap into. TDESharedPixmap inherits KPixmap
 * for that purpose.
 *
 * The server part of shared pixmaps is not implemented here. 
 * That part is provided by KPixmapServer, in the source file:
 * tdebase/kdesktop/pixmapserver.cpp.
 *
 * An example: copy from a shared pixmap:
 * \code
 *   TDESharedPixmap *pm = new TDESharedPixmap;
 *   connect(pm, TQ_SIGNAL(done(bool)), TQ_SLOT(slotDone(bool)));
 *   pm->loadFromShared("My Pixmap");
 * \endcode
 *
 * @author Geert Jansen <jansen@kde.org>
 * @version $Id$
 *
 */
class TDEUI_EXPORT TDESharedPixmap: 
    public TQWidget,
    public KPixmap
{
    TQ_OBJECT
    

public:

    /**
     * Construct an empty pixmap.
     */
    TDESharedPixmap();

    /**
     * Destroys the pixmap.
     */
    ~TDESharedPixmap();

    /**
     * Load from a shared pixmap reference. The signal done() is emitted
     * when the operation has finished.
     *
     * @param name The shared pixmap name.
     * @param rect If you pass a nonzero rectangle, a tile is generated which 
     * is able to fill up the specified rectangle completely. This is solely 
     * for optimization: in some cases the tile will be much smaller than the 
     * original pixmap. It reflects TDESharedPixmap's original use: sharing
     * of the desktop background to achieve pseudo transparency.
     * @return True if the shared pixmap exists and loading has started
     * successfully, false otherwise.
     */
    bool loadFromShared(const TQString & name, const TQRect & rect=TQRect());

    /**
     * Check whether a shared pixmap is available.
     *
     * @param name The shared pixmap name.
     * @return True if the shared pixmap is available, false otherwise.
     */
    bool isAvailable(const TQString & name) const;

signals:
    /** 
     * This signal is raised when a pixmap load operation has finished.
     *
     * @param success True if successful, false otherwise.
     */
    void done(bool success);

protected:
    bool x11Event(XEvent *);
    
private:
    bool copy(const TQString & id, const TQRect & rect);
    void init();

    TDESharedPixmapPrivate *d;
};
#else // WIN32, Qt Embedded
// Let's simply assume KPixmap will do for now. Yes, I know that's broken.
#define TDESharedPixmap KPixmap
#endif

#endif
