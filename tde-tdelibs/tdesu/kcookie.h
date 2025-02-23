/*
 *
 * $Id$
 *
 * This file is part of the KDE project, module tdesu
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; you can use this library under the GNU Library 
 * General Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 */

#ifndef __KCookie_h_Included__
#define __KCookie_h_Included__

#include <tqcstring.h>
#include <tqvaluelist.h>

typedef TQValueList<TQCString> QCStringList;


/**
 * Utility class to access the authentication tokens needed to run a KDE
 * program (X11 and DCOP cookies).
 */

class TDESU_EXPORT KCookie
{
public:
    KCookie();

    /**
     * Returns the X11 display.
     */
    TQCString display() { return m_Display; }

#ifdef TQ_WS_X11
    /**
     * Returns the X11 magic cookie, if available.
     */
    TQCString displayAuth() { return m_DisplayAuth; }
#endif

    /**
     * Select the DCOP transport to look for. Default: "local"
     */
    void setDcopTransport(const TQCString &dcopTransport);

    /**
     * Returns the netid where the dcopserver is running
     */
    TQCString dcopServer();

    /** 
     * Returns a list of magic cookies for DCOP protocol authentication. 
     * The order is the same as in dcopServer().
     */
    TQCString dcopAuth();

    /**
     * Returns a list of magic cookies for the ICE protocol.
     */
    TQCString iceAuth();

private:
    void getXCookie();
    void getICECookie();
    QCStringList split(const TQCString &line, char ch);

    void blockSigChild();
    void unblockSigChild();

    bool m_bHaveDCOPCookies;
    bool m_bHaveICECookies;

    TQCString m_Display;
#ifdef TQ_WS_X11
    TQCString m_DisplayAuth;
#endif
    TQCString m_DCOPSrv;
    TQCString m_DCOPAuth;
    TQCString m_ICEAuth;
    TQCString m_dcopTransport;

    class KCookiePrivate;
    KCookiePrivate *d;
};


#endif // __KCookie_h_Included__
