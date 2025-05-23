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

#ifndef KSTREAMSOCKET_H
#define KSTREAMSOCKET_H

#include <tqstring.h>

#include "kclientsocketbase.h"

/** A namespace to store all networking-related (socket) classes. */
namespace KNetwork {

class KResolverEntry;
class KResolverResults;
class TDEServerSocket;
class TDEBufferedSocket;

class KStreamSocketPrivate;
/** @class KStreamSocket kstreamsocket.h kstreamsocket.h
 *  @brief Simple stream socket
 *
 * This class provides functionality to creating unbuffered, stream
 * sockets. In the case of Internet (IP) sockets, this class creates and
 * uses TCP/IP sockets.
 *
 * Objects of this class start, by default, on non-blocking mode. Call
 * setBlocking if you wish to change that.
 *
 * KStreamSocket objects are thread-safe and can be used in auxiliary
 * threads (i.e., not the thread in which the Qt event loop runs in).
 * Note that TDEBufferedSocket cannot be used reliably in an auxiliary thread.
 *
 * Sample usage:
 * \code
 *   TQByteArray httpGet(const TQString& hostname)
 *   {
 *     KStreamSocket socket(hostname, "http");
 *     if (!socket.connect())
 *       return TQByteArray();
 *     TQByteArray data = socket.readAll();
 *     return data;
 *   }
 * \endcode
 *
 * Here's another sample, showing asynchronous operation:
 * \code
 *  DataRetriever::DataRetriever(const TQString& hostname, const TQString& port)
 *    : socket(hostname, port)
 *  {
 *    // connect signals to our slots
 *    TQObject::connect(&socket, TQ_SIGNAL(connected(const KResolverEntry&)),
 *                     this, TQ_SLOT(slotSocketConnected()));
 *    TQObject::connect(&socket, TQ_SIGNAL(gotError(int)),
 *                     this, TQ_SLOT(slotSocketError(int)));
 *    TQObject::connect(&socket, TQ_SIGNAL(readyRead()),
 *                     this, TQ_SLOT(slotSocketReadyToRead()));
 *    TQObject::connect(&socket, TQ_SIGNAL(readyWrite()),
 *                     this, TQ_SLOT(slotSocketReadyToWrite()));
 *
 *    // set non-blocking mode in order to work asynchronously
 *    socket.setBlocking(false);
 *
 *    // turn on signal emission
 *    socket.enableRead(true);
 *    socket.enableWrite(true);
 *
 *    // start connecting
 *    socket.connect();
 *  }
 * \endcode
 *
 * @see KNetwork::TDEBufferedSocket, KNetwork::TDEServerSocket
 * @author Thiago Macieira <thiago@kde.org>
 */
class TDECORE_EXPORT KStreamSocket: public KClientSocketBase
{
  TQ_OBJECT
  

public:
  /**
   * Default constructor.
   *
   * @param node	destination host
   * @param service	destination service to connect to
   * @param parent	the parent TQObject object
   * @param name        name for this object
   */
  KStreamSocket(const TQString& node = TQString::null, const TQString& service = TQString::null,
		TQObject* parent = 0L, const char *name = 0L);

  /**
   * Destructor. This closes the socket.
   */
  virtual ~KStreamSocket();

  /**
   * Retrieves the timeout value (in milliseconds).
   */
  int timeout() const;

  /**
   * Retrieves the remaining timeout time (in milliseconds). This value
   * equals @ref timeout() if there's no connection in progress.
   */
  int remainingTimeout() const;

  /**
   * Sets the timeout value. Setting this value while a connection attempt
   * is in progress will reset the timer.
   *
   * Please note that the timeout value is valid for the connection attempt
   * only. No other operations are timed against this value -- including the
   * name lookup associated.
   *
   * @param msecs		the timeout value in milliseconds
   */
  void setTimeout(int msecs);

  /**
   * Binds this socket to the given nodename and service,
   * or use the default ones if none are given. In order to bind to a service
   * and allow the operating system to choose the interface, set @p node to
   * TQString::null.
   * 
   * Reimplemented from KClientSocketBase.
   *
   * Upon successful binding, the @ref bound signal will be
   * emitted. If an error is found, the @ref gotError
   * signal will be emitted.
   *
   * @note Due to the internals of the name lookup and binding
   *       mechanism, some (if not most) implementations of this function
   *       do not actually bind the socket until the connection
   *       is requested (see @ref connect). They only set the values
   *       for future reference.
   *
   * This function returns true on success.
   *
   * @param node	the nodename
   * @param service	the service
   */
  virtual bool bind(const TQString& node = TQString::null,
		    const TQString& service = TQString::null);

  /**
   * Reimplemented from KClientSocketBase. Connect this socket to this
   * specific address.
   *
   * Unlike @ref bind(const TQString&, const TQString&) above, this function
   * really does bind the socket. No lookup is performed. The @ref bound
   * signal will be emitted.
   */
  virtual bool bind(const KResolverEntry& entry)
  { return KClientSocketBase::bind(entry); }

  /**
   * Reimplemented from KClientSocketBase.
   *
   * Attempts to connect to the these hostname and service,
   * or use the default ones if none are given. If a connection attempt
   * is already in progress, check on its state and set the error status
   * (NoError, meaning the connection is completed, or InProgress).
   *
   * If the blocking mode for this object is on, this function will only
   * return when all the resolved peer addresses have been tried or when
   * a connection is established.
   *
   * Upon successfully connecting, the @ref connected signal 
   * will be emitted. If an error is found, the @ref gotError
   * signal will be emitted.
   *
   * This function also implements timeout handling.
   *
   * @param node	the remote node to connect to
   * @param service	the service on the remote node to connect to
   */
  virtual bool connect(const TQString& node = TQString::null,
		       const TQString& service = TQString::null);

  /**
   * Unshadowing from KClientSocketBase.
   */
  virtual bool connect(const KResolverEntry& entry);

signals:
  /**
   * This signal is emitted when a connection timeout occurs.
   */
  void timedOut();

private slots:
  void hostFoundSlot();
  void connectionEvent();
  void timeoutSlot();

private:
  /**
   * @internal
   * If the user requested local bind before connection, bind the socket to one
   * suitable address and return true. Also sets d->local to the address used.
   *
   * Return false in case of error.
   */
  bool bindLocallyFor(const KResolverEntry& peer);

  /**
   * @internal
   * Finishes the connection process by setting internal values and
   * emitting the proper signals.
   *
   * Note: assumes d->local iterator points to the address that we bound
   * to.
   */
  void connectionSucceeded(const KResolverEntry& peer);

  KStreamSocket(const KStreamSocket&);
  KStreamSocket& operator=(const KStreamSocket&);

  KStreamSocketPrivate *d;

  friend class TDEServerSocket;
  friend class TDEBufferedSocket;
};

} 				// namespace KNetwork

#endif
