/* This file is part of the KDE project
 *
 * Copyright (C) 2002 Till Krech <till@snafu.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KXEVENTUTIL_H
#define KXEVENTUTIL_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <tqstring.h>

class KXEventUtil {
    public:
        static TQString getXEventName(XEvent *e);
        static TQString getXAnyEventInfo(XEvent *xevent);
        static TQString getXButtonEventInfo(XEvent *xevent);
        static TQString getXKeyEventInfo(XEvent *xevent);
        static TQString getXMotionEventInfo(XEvent *xevent);
        static TQString getXCrossingEventInfo(XEvent *xevent);
        static TQString getXFocusChangeEventInfo(XEvent *xevent);
        static TQString getXExposeEventInfo(XEvent *xevent);
        static TQString getXGraphicsExposeEventInfo(XEvent *xevent);
        static TQString getXNoExposeEventInfo(XEvent *xevent);
        static TQString getXCreateWindowEventInfo(XEvent *xevent);
        static TQString getXDestroyWindowEventInfo(XEvent *xevent);
        static TQString getXMapEventInfo(XEvent *xevent);
        static TQString getXMappingEventInfo(XEvent *xevent);
        static TQString getXReparentEventInfo(XEvent *xevent);
        static TQString getXUnmapEventInfo(XEvent *xevent);
        static TQString getXConfigureEventInfo(XEvent *xevent);
        static TQString getXConfigureRequestEventInfo(XEvent *xevent);
        static TQString getX11EventInfo( XEvent* e );
};

#endif
