/*
 *  Copyright (C) 2003 Thiago Macieira <thiago@kde.org>
 *
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included 
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef KSERVERSOCKET_H
#define KSERVERSOCKET_H

#include <tqobject.h>
#include "tdesocketbase.h"

namespace KNetwork {

class TDESocketDevice;
class KStreamSocket; 
class KResolver;
class KResolverResults;

class TDEServerSocketPrivate;
/**
 * @class TDEServerSocket kserversocket.h kserversocket.h
 * @brief A server socket for accepting connections.
 *
 * This class provides functionality for creating a socket to
 * listen for incoming connections and subsequently accept them.
 *
 * To use this class, you must first set the parameters for the listening
 * socket's address, then place it in listening mode.
 *
 * A typical example would look like:
 * \code
 *   TQString service = "http";
 *   TDEServerSocket *ss = new TDEServerSocket(service);
 *   connect(ss, TQ_SIGNAL(readyAccept()), this, TQ_SLOT(slotReadyAccept()));
 *   connect(ss, TQ_SIGNAL(gotError(int)), this, TQ_SLOT(slotSocketError(int)));
 *   ss->listen();
 * \endcode
 * 
 * In this case, this class will place the socket into listening mode on the
 * service pointed to by @p service and will emit the @ref readyAccept signal
 * when a connection is ready for accepting. The called slot is responsible for
 * calling @ref accept.
 * 
 * The location of the services file (where @p service is looked up) 
 * is defined by _PATH_SERVICES in /usr/include/netdb.h.  This is
 * usually set to /etc/services.
 * See RFC 1700 for more information on services.
 * You can specify @p service as a port number directly, rather than as a service
 * name.  This is discouraged as it prevents the end user from easily modifying
 * the port number.
 *
 * For another example of usage, this below code attempts to make a connection on any port within a range:
 * \code
 *   TDEServerSocket *ss = new TDEServerSocket();
 *   ss->setFamily(KResolver::InetFamily);
 *   bool found = false;
 *   for( unsigned int port = firstport; port <= lastport; ++port) {
 *     ss->setAddress( TQString::number( port ) );
 *     bool success = ss->listen();
 *     if( found = ( success && ss->error() == 
 *                              TDESocketBase::NoError ) )
 *       break;
 *     ss->close();
 *   }
 *   if( !found ) {
 *     // Couldn't connect to any port.
 *   } else {
 *     connect(ss, TQ_SIGNAL(readyAccept()), this, TQ_SLOT(slotReadyAccept()));
 *     connect(ss, TQ_SIGNAL(gotError(int)), this, TQ_SLOT(slotSocketError(int)));
 *     ss->listen();
 *   }
 * \endcode
 *
 * The called slot slotReadyAccept() is responsible for calling
 * @ref accept.
 *
 * It is important to note that @ref accept can return either an
 * object of type KNetwork::KStreamSocket or
 * KNetwork::TDEBufferedSocket (default). If you want to accept a
 * non-buffered socket, you must first call setAcceptBuffered.
 *
 * @warning If you use TDEServerSocket in an auxiliary (non-GUI) thread,
 *          you need to accept only KNetwork::KStreamSocket objects.
 *
 * @see KNetwork::KStreamSocket, KNetwork::TDEBufferedSocket
 * @author Thiago Macieira <thiago@kde.org>
 */
class TDECORE_EXPORT TDEServerSocket: public TQObject, public KPassiveSocketBase
{
  TQ_OBJECT
  
public:
  /**
   * Default constructor.
   *
   * If the binding address isn't changed by setAddress, this socket will
   * bind to all interfaces on this node and the port will be selected by the
   * operating system.
   *
   * @param parent		the parent TQObject object
   * @param name		the name of this object
   */
  TDEServerSocket(TQObject* parent = 0L, const char *name = 0L);

  /**
   * Construct this object specifying the service to listen on.
   *
   * If the binding address isn't changed by setAddress, this socket will
   * bind to all interfaces and will listen on the port specified by
   * @p service.  This is either a service name (e.g. 'www') or a port
   * number (e.g. '80').
   * 
   * The location of the services file (where @p service is looked up) 
   * is defined by _PATH_SERVICES in /usr/include/netdb.h.  This is
   * usually set to /etc/services.
   * See RFC 1700 for more information on services.
   *
   * @param service		the service name to listen on
   * @param parent		the parent TQObject object
   * @param name		the name of this object
   */
  TDEServerSocket(const TQString& service, TQObject* parent = 0L, const char *name = 0L);

  /**
   * Construct this object specifying the node and service names to listen on.
   *
   * If the binding address isn't changed by setAddress, this socket will
   * bind to the interface specified by @p node and the port specified by
   * @p service.  This is either a service name (e.g. 'www') or a port
   * number (e.g. '80').
   *   
   * The location of the services file (where @p service is looked up) 
   * is defined by _PATH_SERVICES in /usr/include/netdb.h.  This is
   * usually set to /etc/services.
   * See RFC 1700 for more information on services.   
   *
   * @param node		the node to bind to
   * @param service		the service port to listen on
   * @param parent		the parent TQObject object
   * @param name		the name of this object
   */
  TDEServerSocket(const TQString& node, const TQString& service,
		TQObject* parent = 0L, const char *name = 0L);

  /**
   * Destructor. This will close the socket, if open.
   *
   * Note, however, that accepted sockets do not get closed when this
   * object closes.
   */
  ~TDEServerSocket();

protected:
  /**
   * Sets the socket options. Reimplemented from TDESocketBase.
   */
  virtual bool setSocketOptions(int opts);

public:
  /**
   * Returns the internal KResolver object used for
   * looking up the host name and service.
   *
   * This can be used to set extra options to the
   * lookup process other than the default values, as well
   * as obtaining the error codes in case of lookup failure.
   */
  KResolver& resolver() const;

  /**
   * Returns the internal list of resolved results for the binding address.
   */
  const KResolverResults& resolverResults() const;

  /**
   * Enables or disables name resolution. If this flag is set to true,
   * the @ref bind operation will trigger name lookup
   * operations (i.e., converting a hostname into its binary form).
   * If the flag is set to false, those operations will instead
   * try to convert a string representation of an address without
   * attempting name resolution.
   *
   * This is useful, for instance, when IP addresses are in
   * their string representation (such as "1.2.3.4") or come
   * from other sources like @ref TDESocketAddress.
   *
   * @param enable	whether to enable
   */
  void setResolutionEnabled(bool enable);

  /**
   * Sets the allowed families for the resolutions.
   *
   * @param families		the families that we want/accept
   * @see KResolver::SocketFamilies for possible values
   */
  void setFamily(int families);

  /**
   * Sets the address on which we will listen. The port to listen on is given by
   * @p service, and we will bind to all interfaces. To let the operating system choose a
   * port, set the service to "0".  @p service can either be a service name
   * (e.g. 'www') or a port number (e.g. '80').
   *
   * The location of the services file (where @p service is looked up) 
   * is defined by _PATH_SERVICES in /usr/include/netdb.h.  This is
   * usually set to /etc/services.
   * See RFC 1700 for more information on services.
   *
   * @param service		the service name to listen on
   */
  void setAddress(const TQString& service);

  /**
   * @overload
   * Sets the address on which we will listen. This will cause the socket to listen
   * only on the interface given by @p node and on the port given by @p service.
   * @p service can either be a service name (e.g. 'www') or a port number
   * (e.g. '80').
   *
   * The location of the services file (where @p service is looked up) 
   * is defined by _PATH_SERVICES in /usr/include/netdb.h.  This is
   * usually set to /etc/services.
   * See RFC 1700 for more information on services. 
   *
   * @param node		the node to bind to
   * @param service		the service port to listen on
   */
  void setAddress(const TQString& node, const TQString& service);

  /**
   * Sets the timeout for accepting. When you call @ref accept,
   * it will wait at most @p msecs milliseconds or return with an error
   * (returning a NULL object).
   *
   * @param msecs		the time in milliseconds to wait, 0 to wait forever
   */
  void setTimeout(int msecs);

  /**
   * Starts the lookup for peer and local hostnames as
   * well as their services.
   *
   * If the blocking mode for this object is on, this function will
   * wait for the lookup results to be available (by calling the 
   * @ref KResolver::wait method on the resolver objects).
   *
   * When the lookup is done, the signal @ref hostFound will be
   * emitted (only once, even if we're doing a double lookup).
   * If the lookup failed (for any of the two lookups) the 
   * @ref gotError signal will be emitted with the appropriate
   * error condition (see @ref TDESocketBase::SocketError).
   *
   * This function returns true on success and false on error. Note that
   * this is not the lookup result!
   */
  virtual bool lookup();

  /**
   * Binds this socket to the given nodename and service,
   * or use the default ones if none are given.
   *
   * Upon successful binding, the @ref bound signal will be
   * emitted. If an error is found, the @ref gotError
   * signal will be emitted.
   *
   * This function returns true on success.
   *
   * @param node	the nodename
   * @param service	the service
   */
  virtual bool bind(const TQString& node, const TQString& service);

  /**
   * Binds the socket to the given service name.
   * @overload
   *
   * @param service	the service
   */
  virtual bool bind(const TQString& service);

  /**
   * Binds the socket to the addresses previously set with @ref setAddress.
   * @overload
   *
   */
  virtual bool bind();

  /**
   * Connect this socket to this specific address. Reimplemented from TDESocketBase.
   *
   * Unlike @ref bind(const TQString&, const TQString&) above, this function
   * really does bind the socket. No lookup is performed. The @ref bound signal
   * will be emitted.
   */
  virtual bool bind(const KResolverEntry& address);

  /**
   * Puts this socket into listening mode. Reimplemented from @ref KPassiveSocketBase.
   *
   * Placing a socket into listening mode means it will be able to receive incoming
   * connections through the @ref accept method.
   *
   * If you do not call this method but call @ref accept directly, the socket will
   * be placed into listening mode automatically.
   *
   * @param backlog		the number of connection the system is to
   *                            queue without @ref accept being called
   * @returns true if the socket is now in listening mode.
   */
  virtual bool listen(int backlog = 5);	// 5 is arbitrary

  /**
   * Closes this socket.
   */
  virtual void close();

  /**
   * Toggles whether the accepted socket will be buffered or not.
   * That is, the @ref accept function will always return a KStreamSocket
   * object or descended from it. If buffering is enabled, the class
   * to be returned will be TDEBufferedSocket.
   *
   * By default, this flag is set to true.
   *
   * @param enable		whether to set the accepted socket to
   *				buffered mode
   */
  void setAcceptBuffered(bool enable);

  /**
   * Accepts one incoming connection and return the associated, open
   * socket.
   *
   * If this function cannot accept a new connection, it will return NULL.
   * The specific object class returned by this function may vary according
   * to the implementation: derived classes may return specialised objects
   * descended from KStreamSocket.
   *
   * @note This function should return a KStreamSocket object, but compiler
   *       deficiencies prevent such an adjustment. Therefore, we return
   *       the base class for active sockets, but it is guaranteed
   *       that the object will be a KStreamSocket or derived from it.
   *
   * @sa TDEBufferedSocket
   * @sa setAcceptBuffered
   */
  virtual KActiveSocketBase* accept();

  /**
   * Returns this socket's local address.
   */
  virtual TDESocketAddress localAddress() const;

  /**
   * Returns this socket's externally-visible address if know.
   */
  virtual TDESocketAddress externalAddress() const;

private slots:
  void lookupFinishedSlot();

signals:
  /**
   * This signal is emitted when this object finds an error.
   * The @p code parameter contains the error code that can
   * also be found by calling @ref error.
   */
  void gotError(int code);

  /**
   * This signal is emitted when the lookup is successfully completed.
   */
  void hostFound();

  /**
   * This signal is emitted when the socket successfully binds
   * to an address.
   *
   * @param local	the local address we bound to
   */
  void bound(const KResolverEntry& local);

  /**
   * This signal is emitted when the socket completes the
   * closing/shut down process.
   */
  void closed();

  /**
   * This signal is emitted whenever the socket is ready for
   * accepting -- i.e., there is at least one connection waiting to
   * be accepted.
   */
  void readyAccept();

protected:
  /**
   * Convenience function to set this object's error code to match
   * that of the socket device.
   */
  void copyError();

private:
  bool doBind();
  bool doListen();

private:
  TDEServerSocket(const TDEServerSocket&);
  TDEServerSocket& operator=(const TDEServerSocket&);

  TDEServerSocketPrivate *d;
};

}				// namespace KNetwork

#endif
