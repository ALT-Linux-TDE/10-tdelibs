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

#ifndef KJAVAPROCESS_H
#define KJAVAPROCESS_H

#include <kprocess.h>
#include <tqcstring.h>

/**
 * @short A class for invoking a Java VM
 *
 * This class is a general tool for invoking a Java interpreter. It allows you
 * to specify some of the standard options that should be understood by all
 * JVMs.
 *
 * @author Richard J. Moore, rich@kde.org
 * @author Wynn Wilkes, wynnw@calderasystems.com
 */

class KJavaProcessPrivate;
class KJavaProcess : public TDEProcess //TQObject
{
TQ_OBJECT

public:
    /**
     * Creates a process object, the process is NOT invoked at this point.
     * You should first set the process's parameters, and then call startJava.
     */
    KJavaProcess();
    virtual ~KJavaProcess();

    /**
     * Invoke the JVM with the parameters that have been set.  The Java process
     * will start after this call.
     */
    bool startJava();

    /**
     * Stop the JVM (if it's running).
     */
    void stopJava();

    /**
     * Returns the status of the java Process- true if it's ok, false if it has died.
     * It calls TDEProcess::isRunning()
     */
    bool isRunning();

    /**
     * Used to specify the path to the Java executable to be run.
     */
    void setJVMPath( const TQString& path );

    /**
     * This will set the classpath the Java process will use.  It's used as a the
     * -cp command line option.  It adds every jar file stored in $TDEDIRS/share/apps/kjava/
     * to the classpath, and then adds the $CLASSPATH environmental variable.  This allows
     * users to simply drop the JSSE (Java Secure Sockets Extension classes into that directory
     * without having to modify the jvm configuration files.
     */
    void setClasspath( const TQString& classpath );

    /**
     * Set a property on the java command line as -Dname=value, or -Dname if value is TQString::null.
     * For example, you could call setSystemProperty( "kjas.debug", "" ) to set the kjas.debug property.
     */
    void setSystemProperty( const TQString& name, const TQString& value );

    /**
     * The class to be called when startJava() is called.
     */
    void setMainClass( const TQString& clazzName );

    /**
     * Extra flags passed to the JVM.
     */
    void setExtraArgs( const TQString& args );

    /**
     * Arguments passed to the main class.  They will be very last in the java
     * command line, after the main class.
     */
    void setClassArgs( const TQString& classArgs );

    /**
     * Sends a command to the KJAS Applet Server by building a QByteArray
     * out of the data, and then writes it standard out.
     */
    void send( char cmd_code, const TQStringList& args );

    /**
     * Sends a command to the KJAS Applet Server by building a QByteArray
     * out of the data, and then writes it standard out.  It adds each TQString
     * in the arg list, and then adds the data array.
     */
    void send( char cmd_code, const TQStringList& args, const TQByteArray& data );

    /**
     * Writes all pending data to JVM
     **/
    void flushBuffers();

protected slots:
    /**
     * This slot is called whenever something is written to stdin of the process.
     * It's called again to make sure we keep emptying out the buffer that contains
     * the messages we need send.
     */
    void slotWroteData();

    /**
     * This slot is called when the Java Process writes to standard out.  We then
     * process the data from the file descriptor that is passed to us and send the
     * command to the AppletServer
     */
    void slotReceivedData( int, int& );
    /**
     * This slot is called when the Java Process exited.
     */
    void slotExited( TDEProcess *process );

protected:
    virtual bool invokeJVM();
    virtual void killJVM();

    TQByteArray* addArgs( char cmd_code, const TQStringList& args );
    void        popBuffer();
    void        sendBuffer( TQByteArray* buff );
    void        storeSize( TQByteArray* buff );

    TDEProcess* javaProcess;

signals:
    void received( const TQByteArray& );
    void exited( int status );

private:
    KJavaProcessPrivate *d;

};

#endif // KJAVAPROCESS_H
