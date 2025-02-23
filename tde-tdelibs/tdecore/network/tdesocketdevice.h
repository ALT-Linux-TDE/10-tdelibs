/*
 *  Copyright (C) 2003 Thiago Macieira <thiago.macieira@kdemail.net>
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

#ifndef TDESOCKETDEVICE_H
#define TDESOCKETDEVICE_H

#include <tqsocketnotifier.h>
#include "tdesocketbase.h"

namespace KNetwork {

class TDESocketDevice;
class TDESocketDeviceFactoryBase;

class TDESocketDevicePrivate;
/** @class TDESocketDevice tdesocketdevice.h tdesocketdevice.h
 *  @brief Low-level socket functionality.
 *
 * This class provides low-level socket functionality. 
 *
 * Most users will prefer "cooked" interfaces like those of @ref KStreamSocket or
 * @ref TDEServerSocket.
 *
 * Descended classes from this one provide some other kinds of socket functionality,
 * like proxying or specific socket types.
 *
 * @author Thiago Macieira <thiago.macieira@kdemail.net>
 */
class TDECORE_EXPORT TDESocketDevice: public KActiveSocketBase, public KPassiveSocketBase
{
public:
  /**
   * Capabilities for the socket implementation.
   *
   * TDESocketDevice-derived classes can implement certain capabilities that are not
   * available in the default class. These capabilities are described by these flags.
   * The default TDESocketDevice class has none of these capabilities.
   *
   * For the negative capabilities (inabilities, the CanNot* forms), when a capability 
   * is not present, the implementation will default to the original behaviour.
   */
  enum Capabilities
    {
      /** Can connect to hostnames.
       *  If this flag is present, the string form of @ref connect can be used. */
      CanConnectString = 0x01,

      /** Can bind to hostnames.
       *  If this flag is present, the string form of @ref bind can be used */
      CanBindString = 0x02,

      /** Can not bind.
       *  If this flag is present, this implementation cannot bind */
      CanNotBind = 0x04,

      /** Can not listen.
       *  If this flag is present, this implementation cannot listen */
      CanNotListen = 0x08,

      /**
       * Can send multicast as well as join/leave multicast groups.
       */
      CanMulticast = 0x10,

      /**
       * Can not use datagrams.
       * Note that this implies multicast capability not being available either.
       */
      CanNotUseDatagrams = 0x20
    };
protected:
  /// The socket file descriptor. It is used throughout the implementation
  /// and subclasses.
  int m_sockfd;

public:
  /**
   * Default constructor.
   *
   * The parameter is used to specify which socket this object is used as
   * a device for.
   */
  explicit TDESocketDevice(const TDESocketBase* = 0L);

  /**
   * Constructs a new object around an already-open socket.
   *
   * Note: you should write programs that create sockets through
   * the classes whenever possible.
   */
  explicit TDESocketDevice(int fd);

  /**
   * Destructor. This closes the socket if it's open.
   */
  virtual ~TDESocketDevice();

  /**
   * Returns the file descriptor for this socket.
   */
  inline int socket() const
  { return m_sockfd; }

  /**
   * Returns the set of capabilities this socket class implements.
   * The set of capabilities is defined as an OR-ed mask of 
   * @ref Capabilities bits.
   *
   * The default implementation is guaranteed to always return 0. That
   * is, derived implementations always return bits where they differ
   * from the system standard sockets.
   */
  virtual int capabilities() const
  { return 0; }

  /**
   * This implementation sets the options on the socket.
   */
  virtual bool setSocketOptions(int opts);

  /**
   * Reimplementation from TQIODevice. You should not call this function in sockets.
   */
  virtual bool open(int mode);

  /**
   * Closes the socket. Reimplemented from TQIODevice.
   *
   * Use this function to close the socket this object is holding open.
   */
  virtual void close();

  /**
   * This call is not supported on sockets. Reimplemented from TQIODevice.
   */
  virtual void flush()
  { }

  /**
   * Creates a socket but don't connect or bind anywhere.
   * This function is the equivalent of the system call socket(2).
   */
  virtual bool create(int family, int type, int protocol);

  /**
   * @overload
   * Creates a socket but don't connect or bind anywhere.
   */
  bool create(const KResolverEntry& address);

  /**
   * Binds this socket to the given address.
   */
  virtual bool bind(const KResolverEntry& address);

  /**
   * Puts this socket into listening mode.
   */
  virtual bool listen(int backlog = 5);	// 5 is arbitrary

  /**
   * Connect to a remote host.
   */
  virtual bool connect(const KResolverEntry& address);

  /**
   * Accepts a new incoming connection.
   * Note: this function returns a socket of type TDESocketDevice.
   */
  virtual TDESocketDevice* accept();

  /**
   * Disconnects this socket.
   */
  virtual bool disconnect();

  /**
   * Returns the number of bytes available for reading without blocking.
   */
  virtual TQ_LONG bytesAvailable() const;

  /**
   * Waits up to @p msecs for more data to be available on this socket.
   *
   * This function is a wrapper against @ref poll. This function will wait
   * for any read events.
   */
  virtual TQ_LONG waitForMore(int msecs, bool *timeout = 0L);

  /**
   * Reads data from this socket.
   */
  virtual TQ_LONG readBlock(char *data, TQ_ULONG maxlen);

  /**
   * Reads data and the source address from this socket.
   */
  virtual TQ_LONG readBlock(char *data, TQ_ULONG maxlen, TDESocketAddress& from);

  /**
   * Peeks data in the socket.
   */
  virtual TQ_LONG peekBlock(char *data, TQ_ULONG maxlen);

  /**
   * Peeks the data in the socket and the source address.
   */
  virtual TQ_LONG peekBlock(char *data, TQ_ULONG maxlen, TDESocketAddress& from);

  /**
   * Writes data to the socket.
   */
  virtual TQ_LONG writeBlock(const char *data, TQ_ULONG len);

  /**
   * Writes the given data to the given destination address.
   */
  virtual TQ_LONG writeBlock(const char *data, TQ_ULONG len, const TDESocketAddress& to);

  /**
   * Returns this socket's local address.
   */
  virtual TDESocketAddress localAddress() const;

  /**
   * Returns this socket's peer address. If this implementation does proxying
   * of some sort, this is the real external address, not the proxy's address.
   */
  virtual TDESocketAddress peerAddress() const;

  /**
   * Returns this socket's externally visible local address.
   *
   * If this socket has a local address visible externally different
   * from the normal local address (as returned by @ref localAddress), then
   * return it.
   *
   * Certain implementations will use proxies and thus have externally visible
   * addresses different from the local socket values. The default implementation
   * returns the same value as @ref localAddress.
   *
   * @note This function may return an empty TDESocketAddress. In that case, the
   *       externally visible address could/can not be determined.
   */
  virtual TDESocketAddress externalAddress() const;

  /**
   * Returns a socket notifier for input on this socket.
   * The notifier is created only when requested. Whether
   * it is enabled or not depends on the implementation.
   *
   * This function might return NULL.
   */
  TQSocketNotifier* readNotifier() const;

  /**
   * Returns a socket notifier for output on this socket.
   * The is created only when requested.
   *
   * This function might return NULL.
   */
  TQSocketNotifier* writeNotifier() const;

  /**
   * Returns a socket notifier for exceptional events on this socket.
   * The is created only when requested.
   *
   * This function might return NULL.
   */
  TQSocketNotifier* exceptionNotifier() const;

  /**
   * Executes a poll in the socket, via select(2) or poll(2).
   * The events polled are returned in the parameters pointers.
   * Set any of them to NULL to disable polling of that event.
   *
   * On exit, @p input, @p output and @p exception will contain
   * true if an event of that kind is waiting on the socket or false
   * if not. If a timeout occurred, set @p timedout to true (all other
   * parameters are necessarily set to false).
   *
   * @param input	if set, turns on polling for input events
   * @param output	if set, turns on polling for output events
   * @param exception	if set, turns on polling for exceptional events
   * @param timeout	the time in milliseconds to wait for an event;
   *			0 for no wait and any negative value to wait forever
   * @param timedout	on exit, will contain true if the polling timed out
   * @return true if the poll call succeeded and false if an error occurred
   */
  virtual bool poll(bool* input, bool* output, bool* exception = 0L,
		    int timeout = -1, bool* timedout = 0L);

  /**
   * Shorter version to poll for any events in a socket. This call
   * polls for input, output and exceptional events in a socket but
   * does not return their states. This is useful if you need to wait for
   * any event, but don't need to know which; or for timeouts.
   *
   * @param timeout	the time in milliseconds to wait for an event;
   *			0 for no wait and any negative value to wait forever
   * @param timedout	on exit, will contain true if the polling timed out
   * @return true if the poll call succeeded and false if an error occurred
   */
  bool poll(int timeout = -1, bool* timedout = 0L);

protected:
  /**
   * Special constructor. This constructor will cause the internal
   * socket device NOT to be set. Use this if your socket device class
   * takes another underlying socket device.
   *
   * @param parent	the parent, if any
   */
  TDESocketDevice(bool, const TDESocketBase* parent = 0L);

  /**
   * Creates a socket notifier of the given type.
   *
   * This function is called by @ref readNotifier, @ref writeNotifier and
   * @ref exceptionNotifier when they need to create a socket notifier
   * (i.e., the first call to those functions after the socket is open).
   * After that call, those functions cache the socket notifier and will
   * not need to call this function again.
   *
   * Reimplement this function in your derived class if your socket type
   * requires a different kind of TQSocketNotifier. The return value should
   * be deleteable with delete. (@ref close deletes them).
   *
   * @param type	the socket notifier type
   */
  virtual TQSocketNotifier* createNotifier(TQSocketNotifier::Type type) const;

public:
  /**
   * Creates a new default TDESocketDevice object given
   * the parent object.
   *
   * The capabilities flag indicates the desired capabilities the object being
   * created should possess. Those capabilities are not guaranteed: if no factory
   * can provide such an object, a default object will be created.
   *
   * @param parent	the TDESocketBase parent
   */
  static TDESocketDevice* createDefault(TDESocketBase* parent);

  /**
   * @overload
   *
   * This will create an object only if the requested capabilities match.
   *
   * @param parent		the parent
   * @param capabilities	the requested capabilities
   */
  static TDESocketDevice* createDefault(TDESocketBase* parent, int capabilities);

  /**
   * Sets the default TDESocketDevice implementation to use and
   * return the old factory.
   *
   * @param factory	the factory object for the implementation
   */
  static TDESocketDeviceFactoryBase* setDefaultImpl(TDESocketDeviceFactoryBase* factory);

  /**
   * Adds a factory of TDESocketDevice objects to the list, along with its
   * capabilities flag.
   */
  static void addNewImpl(TDESocketDeviceFactoryBase* factory, int capabilities);

private:
  TDESocketDevice(const TDESocketDevice&);
  TDESocketDevice& operator=(const TDESocketDevice&);

  TDESocketDevicePrivate *d;
};

/** @internal
 * This class provides functionality for creating and registering
 *  socket implementations.
 */
class TDESocketDeviceFactoryBase
{
public:
  TDESocketDeviceFactoryBase() {}
  virtual ~TDESocketDeviceFactoryBase() {}

  virtual TDESocketDevice* create(TDESocketBase*) const = 0;
};

/**
 * This class provides functionality for creating and registering 
 * socket implementations.
 */
template<class Impl>
class TDESocketDeviceFactory: public TDESocketDeviceFactoryBase
{
public:
  TDESocketDeviceFactory() {}
  virtual ~TDESocketDeviceFactory() {}

  virtual TDESocketDevice* create(TDESocketBase* parent) const
  { return new Impl(parent); }
};

}				// namespaces

#endif
