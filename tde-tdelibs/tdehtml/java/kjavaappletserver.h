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

#ifndef KJAVAAPPLETSERVER_H
#define KJAVAAPPLETSERVER_H

#include "kjavaprocess.h"
#include <tqobject.h>
#include <tqmap.h>


/**
 * @short Communicates with a KJAS server to display and control Java applets.
 *
 * @author Richard J. Moore, rich@kde.org
 */

class KJavaAppletContext;
class KJavaAppletServerPrivate;
class JSStackFrame;

class KJavaAppletServer : public TQObject
{
TQ_OBJECT

public:
    /**
     * Create the applet server.  These shouldn't be used directly,
     * use allocateJavaServer instead
     */
    KJavaAppletServer();
    ~KJavaAppletServer();

    /**
     * A factory method that returns the default server. This is the way this
     * class is usually instantiated.
     */
    static KJavaAppletServer *allocateJavaServer();

    /**
     * When you are done using your reference to the AppletServer,  you must
     * dereference it by calling freeJavaServer().
     */
    static void freeJavaServer();

    /**
     * This allows the KJavaAppletWidget to display some feedback in a QLabel
     * while the applet is being loaded.  If the java process could not be
     * started, an error message is displayed instead.
     */
    static TQString getAppletLabel();

    /**
     * Create an applet context with the specified id.
     */
    void createContext( int contextId, KJavaAppletContext* context );

    /**
     * Destroy the applet context with the specified id. All the applets in the
     * context will be destroyed as well.
     */
    void destroyContext( int contextId );

    /**
     * Create an applet in the specified context with the specified id. The applet
     * name, class etc. are specified in the same way as in the HTML APPLET tag.
     */
    bool createApplet( int contextId, int appletId,
                       const TQString & name, const TQString & clazzName,
                       const TQString & baseURL, const TQString & user,
                       const TQString & password, const TQString & authname,
                       const TQString & codeBase, const TQString & jarFile,
                       TQSize size, const TQMap<TQString, TQString>& params,
                       const TQString & windowTitle );

    /**
     * This should be called by the KJavaAppletWidget
     */
    void initApplet( int contextId, int appletId );

    /**
     * Destroy an applet in the specified context with the specified id.
     */
    void destroyApplet( int contextId, int appletId );

    /**
     * Start the specified applet.
     */
    void startApplet( int contextId, int appletId );

    /**
     * Stop the specified applet.
     */
    void stopApplet( int contextId, int appletId );

    /**
     * Show java console.
     */
    void showConsole();

    /**
     * Send data we got back from a KJavaDownloader back to the appropriate
     * class loader.
     */
    void sendURLData( int loaderID, int code, const TQByteArray& data );
    /**
     * Removes KJavaDownloader from the list (deletes it too).
     */
    void removeDataJob( int loaderID );

    /**
     * Shut down the KJAS server.
     */
    void quit();
    KJavaProcess* javaProcess() { return process; }

    TQString appletLabel();

    void waitForReturnData(JSStackFrame *);
    void endWaitForReturnData();

    bool getMember(TQStringList & args, TQStringList & ret_args);
    bool putMember(TQStringList & args);
    bool callMember(TQStringList & args, TQStringList & ret_args);
    void derefObject(TQStringList & args);

    bool usingKIO();
protected:
    void setupJava( KJavaProcess* p );

    KJavaProcess* process;

protected slots:
    void slotJavaRequest( const TQByteArray& qb );
    void checkShutdown();
    void timerEvent(TQTimerEvent *);

private:
    KJavaAppletServerPrivate* d;

};


class PermissionDialog : public TQObject
{
    TQ_OBJECT
public:
    PermissionDialog( TQWidget* );
    ~PermissionDialog();

    TQCString exec( const TQString & cert, const TQString & perm );

private slots:
     void clicked();

private:
    TQCString m_button;
};

#endif // KJAVAAPPLETSERVER_H
