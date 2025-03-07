/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1997 Torben Weis (weis@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#ifndef KSOCK_H
#define KSOCK_H

#include "tdelibs_export.h"

#ifdef Q_OS_UNIX

#include <tqobject.h>
#include <sys/types.h>
// we define STRICT_ANSI to get rid of some warnings in glibc
#ifndef __STRICT_ANSI__
#define __STRICT_ANSI__
#define _WE_DEFINED_IT_
#endif
#include <sys/socket.h>
#ifdef _WE_DEFINED_IT_
#undef __STRICT_ANSI__
#undef _WE_DEFINED_IT_
#endif

#include <sys/un.h>

#include <netinet/in.h>
class TQSocketNotifier;

#ifdef KSOCK_NO_BROKEN
// This is here for compatibility with old applications still using the constants
// Never use them in new programs

// Here are a whole bunch of hackish macros that allow one to
// get at the correct member of ksockaddr_in
// But since ksockaddr_in is IPv4-only, and deprecated...

typedef sockaddr_in ksockaddr_in;
#define get_sin_addr(x) x.sin_addr
#define get_sin_port(x) x.sin_port
#define get_sin_family(x) x.sin_family
#define get_sin_paddr(x) x->sin_addr
#define get_sin_pport(x) x->sin_port
#define get_sin_pfamily(x) x->sin_family
#endif

#define KSOCK_DEFAULT_DOMAIN PF_INET

class TDESocketPrivate;
class TDEServerSocketPrivate;

/** @deprecated
 * You can connect this socket to any Internet address.
 *
 * This class is deprecated and will be removed in the future. For new
 * programs, please use KExtendedSocket class.
 *
 * The socket gives you three signals: When ready for reading,
 * ready for writing or if the connection is broken.
 * Using socket() you get a file descriptor
 * which you can use with the usual UNIX function like write() or
 * read().
 * If you have already such a socket identifier you can construct a TDESocket
 * on this identifier.
 *
 * If socket() delivers a value of -1 or less, the connection
 * was not successful.
 *
 * @author Torben Weis <weis@uni-frankfurt.de>
 * @short A TCP/IP client socket.
 */
class TDECORE_EXPORT TDESocket : public TQObject
{
    TQ_OBJECT
public:
    /**
     * Constructs a TDESocket with the provided file descriptor.
     * @param _sock	The file descriptor to use.
     */
    TDESocket( int _sock ) TDE_DEPRECATED;
    /**
     * Creates a socket and connects to a host.
     * @param _host	The remote host to which to connect.
     * @param _port	The port on the remote host.
     * @param timeOut	The number of seconds waiting for connect (default 30).
     */
    TDESocket( const char *_host, unsigned short int _port, int timeOut = 30) TDE_DEPRECATED;

    /**
     * Connects to a UNIX domain socket.
     * @param _path    The filename of the socket.
     */
    TDESocket( const char * _path ) TDE_DEPRECATED;

    /**
     * Destructor. Closes the socket if it is still open.
     */
    virtual ~TDESocket();

    /**
     * Returns a file descriptor for this socket.
     * @return the file descriptor, or -1 when an error occurred.
     */
    int socket() const { return sock; }

    /**
     * Enables the socket for reading.
     *
     * If you enable read mode, the socket will emit the signal
     * readEvent() whenever there is something to read out of this
     * socket.
     * @param enable true to enable reading signals
     */
    void enableRead( bool enable );

    /**
     * Enables the socket for writing.
     *
     * If you enable write mode, the socket will emit the signal
     * writeEvent() whenever the socket is ready for writing.
     *
     * Warning: If you forget to call enableWrite(false) when you are
     * not ready to send data, you will get lots of writeEvent() signals,
     * in the order of thousands a second !
     * @param enable true to enable writing signals
     */
    void enableWrite( bool enable );

#ifdef KSOCK_NO_BROKEN
    // BCI: remove in libtdecore.so.4
    /**
     * Return address.
     * This function is dumb. Don't ever use it
     * if you need the peer address of this socket, use KExtendedSocket::peerAddress(int)
     * instead
     * @deprecated
     */
    unsigned long ipv4_addr() TDE_DEPRECATED;

    // BCI: remove in libtdecore.so.4
    /**
      *  A small wrapper around gethostbyname() and such.
      *  Don't use this in new programs. Use KExtendedSocket::lookup
      *  @deprecated
      */
    static bool initSockaddr(ksockaddr_in *server_name, const char *hostname, unsigned short int port, int domain = PF_INET) TDE_DEPRECATED;
#endif

signals:
    /**
     * Data has arrived for reading.
     *
     * This signal will only be raised if enableRead( @p true ) was called
     * first.
     * @param s the TDESocket that triggered the event
     */
    void readEvent( TDESocket *s );

    /**
     * Socket is ready for writing.
     *
     * This signal will only be raised if enableWrite( @p true ) was
     * called first.
     *
     * Warning: If you forget to call enableWrite(false) when you are
     * not ready to send data, you will get lots of writeEvent() signals,
     * in the order of thousands a second !
     * @param s the TDESocket that triggered the event
     */
    void writeEvent( TDESocket *s );

    /**
     * Raised when the connection is broken.
     * @param s the TDESocket that triggered the event
     */
    void closeEvent( TDESocket *s );

public slots:
    /**
     * Connected to the writeNotifier.
     *
     * Called when
     *  the socket is ready for writing.
     * @param x ignored
     */
    void slotWrite( int x);

    /**
     * Connected to the readNotifier.
     *
     * Called when
     *  the socket is ready for reading.
     * @param x ignored
     */
    void slotRead( int x );

protected:
    bool connect( const TQString& _host, unsigned short int _port, int timeout = 0 );
    bool connect( const char *_path );

    /******************************************************
     * The file descriptor for this socket. sock may be -1.
     * This indicates that it is not connected.
     */
    int sock;

private:
    TDESocket(const TDESocket&);
    TDESocket& operator=(const TDESocket&);

    TDESocketPrivate *d;

};


/**
 * @short Monitors a port for incoming TCP/IP connections.
 *
 * @deprecated
 * This class is deprecated and will be removed in the future.
 * Please use the classes in KNetwork for new programs. 
 * In special, this class is replaced by KNetwork::KStreamSocket
 * and KNetwork::TDEServerSocket. 
 *
 * You can use a TDEServerSocket to listen on a port for incoming
 * connections. When a connection arrived in the port, a TDESocket
 * is created and the signal accepted is raised. Make sure you
 * always connect to this signal. If you don't the ServerSocket will
 * create new TDESocket's and no one will delete them!
 *
 * If socket() is -1 or less the socket was not created properly.
 *
 * @author Torben Weis <weis@stud.uni-frankfurt.de>
*/
class TDECORE_EXPORT TDEServerSocket : public TQObject
{
    TQ_OBJECT
public:
    /**
     * Constructor.
     * @param _port	the port number to monitor for incoming connections.
     * @param _bind     if false you need to call bindAndListen yourself.
     *                  This gives you the opportunity to set options on the
     *                  socket.
     */
    TDEServerSocket( unsigned short int _port, bool _bind = true );

    /**
     * Creates a UNIX domain server socket.
     * @param _path     path used for the socket.
     * @param _bind     if false you need to call bindAndListen yourself.
     *                  This gives you the opportunity to set options on the
     *                  socket.
     */
    TDEServerSocket( const char *_path, bool _bind = true);

    /**
     * Destructor. Closes the socket if it was not already closed.
     */
    virtual ~TDEServerSocket();

    /**
     * Binds the socket and start listening. This should only be called
     * once when the constructor was called with _bind false.
     * On error the socket will be closed.
     * @param suppressFailureMessages suppress warning messages generated if the socket cannot be opened.
     * @return true on success. false on error.
     * @warning If suppressFailureMessages is TRUE future debugging may be made more difficult.  Only set it
     * if your application expects to bind to unavailable ports, e.g. while scanning for open ports in a range.
     */
    bool bindAndListen(bool suppressFailureMessages = false);

    /**
     * Returns the file descriptor associated with the socket.
     * @return the file descriptor, -1 when an error occurred during
     *         construction or bindAndListen
     */
    int socket() const { return sock; }

    /**
     * Returns the port number which is being monitored.
     * @return the port number
     */
    unsigned short int port();

#ifdef KSOCK_NO_BROKEN
    // BCI: remove in libtdecore.so.4
    /**
     * The address.
     * This is dumb. Don't use it
     * Refer to KExtendedSocket::localAddress(int)
     * @deprecated
     */
    unsigned long ipv4_addr();
#endif

public slots:
    /**
     * Called when someone connected to our port.
     */
    virtual void slotAccept( int ); // why is this virtual?

signals:
    /**
     * A connection has been accepted.
     * It is your task to delete the TDESocket if it is no longer needed.
     *
     * WARNING: this signal is always emitted, even if you don't connect
     * anything to it. That would mean memory loss, because the TDESockets
     * created go to oblivion.
     * @param s the socket that accepted
     */
    void accepted( TDESocket*s );

protected:
    bool init( unsigned short int );
    bool init( const char *_path );

    /**
     * The file descriptor for this socket. sock may be -1.
     * This indicates that it is not connected.
     */
    int sock;

private:
    // HACK
#ifdef TDESOCKET_BINARY_COMPAT_HACK
    TDE_EXPORT bool bindAndListen();
#endif // TDESOCKET_BINARY_COMPAT_HACK

    TDEServerSocket(const TDEServerSocket&);
    TDEServerSocket& operator=(const TDEServerSocket&);

    TDEServerSocketPrivate *d;
};

#endif //Q_OS_UNIX

#endif
