/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Richard Moore <rich@kde.org>
 *               2000 Wynn Wilkes <wynnw@caldera.com>
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

#ifndef KJAVAAPPLETCONTEXT_H
#define KJAVAAPPLETCONTEXT_H

#include <tqobject.h>

/**
 * @short Provides a context for KJavaAppletWidgets
 *
 * Applets run in a context- (see the Java documentation for more information
 * on contexts).  Currently, each document in TDEHTML creates one context, in
 * which multiple applets can run.
 *
 * @author Richard J. Moore, rich@kde.org
 * @author Wynn Wilkes, wynnw@caldera.com
 */


class KJavaAppletServer;
class KJavaApplet;
class KJavaAppletContextPrivate;

class KJavaAppletContext : public TQObject
{
TQ_OBJECT

public:
    KJavaAppletContext();
    ~KJavaAppletContext();

    /**
     * Returns the ID of this context.
     */
    int  contextId();

    /**
     * Sets the ID of this context.
     */
    void setContextId( int id );

    /**
     * registers applet
     **/
    void registerApplet( KJavaApplet* );

    /**
     * Sends a message to create the applet.
     */
    bool create( KJavaApplet* );

    /**
     * Sends a message to destroy the applet.
     */
    void destroy( KJavaApplet* );

    /**
     * Sends a message to initialize the applet.
     */
    void init( KJavaApplet* );

    /**
     * Sends a message to start the applet.
     */
    void start( KJavaApplet* );

    /**
     * Sends a message to stop the applet.
     */
    void stop( KJavaApplet* );

    /**
     * use this for applet call backs, the AppletServer
     * calls this directly.
     */
    void processCmd( TQString cmd, TQStringList args );

    /**
     * LiveConnect functions
     */
    bool getMember(TQStringList & args, TQStringList & ret_args);
    bool putMember(TQStringList & args);
    bool callMember(TQStringList & args, TQStringList & ret_args);
    void derefObject(TQStringList & args);

    KJavaAppletServer* getServer() const { return server; }
signals:
    /**
     * Signals the KHMTL Part to show this as the status message.
     */
    void showStatus  ( const TQString& txt );

    /**
     * Signals the TDEHTML Part to show a url in a given target
     */
    void showDocument( const TQString& url, const TQString& target );

    /**
     * Signals the TDEHTML Part an applet is loaded
     **/
    void appletLoaded();

protected:
    //The counter to generate ID's for the contexts
    static int contextCount;

    // The applet server this context is attached to.
    KJavaAppletServer* server;

protected slots:
    void received( const TQString& cmd, const TQStringList& arg );
    void javaProcessExited(int);

private:
    int id;
    KJavaAppletContextPrivate* d;

};

#endif // KJAVAAPPLETCONTEXT_H
