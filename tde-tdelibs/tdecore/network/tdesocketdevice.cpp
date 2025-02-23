/*
 *  Copyright (C) 2003,2005 Thiago Macieira <thiago.macieira@kdemail.net>
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

#include <config.h>

#include <tqmap.h>

#ifdef USE_SOLARIS
# include <sys/filio.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>

#ifdef HAVE_POLL
# include <poll.h>
#else
# ifdef HAVE_SYS_SELECT
#  include <sys/select.h>
# endif
#endif

// Include syssocket before our local includes
#include "syssocket.h"

#include <tqmutex.h>
#include <tqsocketnotifier.h>

#include "kresolver.h"
#include "tdesocketaddress.h"
#include "tdesocketbase.h"
#include "tdesocketdevice.h"
#include "ksockssocketdevice.h"

using namespace KNetwork;

class KNetwork::TDESocketDevicePrivate
{
public:
  mutable TQSocketNotifier *input, *output, *exception;
  TDESocketAddress local, peer;
  int af;

  inline TDESocketDevicePrivate()
  {
    input = output = exception = 0L;
    af = 0;
  }
};


TDESocketDevice::TDESocketDevice(const TDESocketBase* parent)
  : m_sockfd(-1), d(new TDESocketDevicePrivate)
{
  setSocketDevice(this);
  if (parent)
    setSocketOptions(parent->socketOptions());
}

TDESocketDevice::TDESocketDevice(int fd)
  : m_sockfd(fd), d(new TDESocketDevicePrivate)
{
  setState(IO_Open);
  setFlags(IO_Sequential | IO_Raw | IO_ReadWrite);
  setSocketDevice(this);
  d->af = localAddress().family();
}

TDESocketDevice::TDESocketDevice(bool, const TDESocketBase* parent)
  : m_sockfd(-1), d(new TDESocketDevicePrivate)
{
  // do not set parent
  if (parent)
    setSocketOptions(parent->socketOptions());
}

TDESocketDevice::~TDESocketDevice()
{
  close();			// deletes the notifiers
  unsetSocketDevice(); 		// prevent double deletion
  delete d;
}

bool TDESocketDevice::setSocketOptions(int opts)
{
  // must call parent
  TQMutexLocker locker(mutex());
  TDESocketBase::setSocketOptions(opts);

  if (m_sockfd == -1)
    return true;		// flags are stored

    {
      int fdflags = fcntl(m_sockfd, F_GETFL, 0);
      if (fdflags == -1)
	{
	  setError(IO_UnspecifiedError, UnknownError);
	  return false;		// error
	}

      if (opts & Blocking)
	fdflags &= ~O_NONBLOCK;
      else
	fdflags |= O_NONBLOCK;

      if (fcntl(m_sockfd, F_SETFL, fdflags) == -1)
	{
	  setError(IO_UnspecifiedError, UnknownError);
	  return false;		// error
	}
    }

    {
      int on = opts & AddressReuseable ? 1 : 0;
      if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) == -1)
	{
	  setError(IO_UnspecifiedError, UnknownError);
	  return false;		// error
	}
    }

#if defined(IPV6_V6ONLY) && defined(AF_INET6)
  if (d->af == AF_INET6)
    {
      // don't try this on non-IPv6 sockets, or we'll get an error

      int on = opts & IPv6Only ? 1 : 0;
      if (setsockopt(m_sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&on, sizeof(on)) == -1)
	{
	  setError(IO_UnspecifiedError, UnknownError);
	  return false;		// error
	}
    }
#endif

   {
     int on = opts & Broadcast ? 1 : 0;
     if (setsockopt(m_sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&on, sizeof(on)) == -1)
       {
	 setError(IO_UnspecifiedError, UnknownError);
	 return false;		// error
       }
   }

  return true;			// all went well
}

bool TDESocketDevice::open(int)
{
  resetError();
  return false;
}

void TDESocketDevice::close()
{
  resetError();
  if (m_sockfd != -1)
    {
      delete d->input;
      delete d->output;
      delete d->exception;

      d->input = d->output = d->exception = 0L;

      d->local.setFamily(AF_UNSPEC);
      d->peer.setFamily(AF_UNSPEC);

      ::close(m_sockfd);
    }
  setState(0);

  m_sockfd = -1;
}

bool TDESocketDevice::create(int family, int type, int protocol)
{
  resetError();

  if (m_sockfd != -1)
    {
      // it's already created!
      setError(IO_SocketCreateError, AlreadyCreated);
      return false;
    }

  // no socket yet; we have to create it
  m_sockfd = kde_socket(family, type, protocol);

  if (m_sockfd == -1)
    {
      setError(IO_SocketCreateError, NotSupported);
      return false;
    }

  d->af = family;
  setSocketOptions(socketOptions());
  setState(IO_Open);
  return true;		// successfully created
}

bool TDESocketDevice::create(const KResolverEntry& address)
{
  return create(address.family(), address.socketType(), address.protocol());
}

bool TDESocketDevice::bind(const KResolverEntry& address)
{
  resetError();

  if (m_sockfd == -1 && !create(address))
    return false;		// failed creating

  // we have a socket, so try and bind
  if (kde_bind(m_sockfd, address.address(), address.length()) == -1)
    {
      if (errno == EADDRINUSE)
	setError(IO_BindError, AddressInUse);
      else if (errno == EINVAL)
	setError(IO_BindError, AlreadyBound);
      else
	// assume the address is the cause
	setError(IO_BindError, NotSupported);
      return false;
    }

  return true;
}

bool TDESocketDevice::listen(int backlog)
{
  if (m_sockfd != -1)
    {
      if (kde_listen(m_sockfd, backlog) == -1)
	{
	  setError(IO_ListenError, NotSupported);
	  return false;
	}

      resetError();
      setFlags(IO_Sequential | IO_Raw | IO_ReadWrite);
      return true;
    }

  // we don't have a socket
  // can't listen
  setError(IO_ListenError, NotCreated);
  return false;
}

bool TDESocketDevice::connect(const KResolverEntry& address)
{
  resetError();

  if (m_sockfd == -1 && !create(address))
    return false;		// failed creating!

  if (kde_connect(m_sockfd, address.address(), address.length()) == -1)
    {
      if (errno == EISCONN)
	return true;		// we're already connected
      else if (errno == EALREADY || errno == EINPROGRESS)
	{
	  setError(IO_ConnectError, InProgress);
	  return true;
	}
      else if (errno == ECONNREFUSED)
	setError(IO_ConnectError, ConnectionRefused);
      else if (errno == ENETDOWN || errno == ENETUNREACH ||
	       errno == ENETRESET || errno == ECONNABORTED ||
	       errno == ECONNRESET || errno == EHOSTDOWN ||
	       errno == EHOSTUNREACH)
	setError(IO_ConnectError, NetFailure);
      else
	setError(IO_ConnectError, NotSupported);

      return false;
    }

  setFlags(IO_Sequential | IO_Raw | IO_ReadWrite);
  return true;			// all is well
}

TDESocketDevice* TDESocketDevice::accept()
{
  if (m_sockfd == -1)
    {
      // can't accept without a socket
      setError(IO_AcceptError, NotCreated);
      return 0L;
    }

  struct sockaddr sa;
  socklen_t len = sizeof(sa);
  int newfd = kde_accept(m_sockfd, &sa, &len);
  if (newfd == -1)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
	setError(IO_AcceptError, WouldBlock);
      else
	setError(IO_AcceptError, UnknownError);
      return NULL;
    }

  return new TDESocketDevice(newfd);
}

bool TDESocketDevice::disconnect()
{
  resetError();

  if (m_sockfd == -1)
    return false;		// can't create

  TDESocketAddress address;
  address.setFamily(AF_UNSPEC);
  if (kde_connect(m_sockfd, address.address(), address.length()) == -1)
    {
      if (errno == EALREADY || errno == EINPROGRESS)
	{
	  setError(IO_ConnectError, InProgress);
	  return false;
	}
      else if (errno == ECONNREFUSED)
	setError(IO_ConnectError, ConnectionRefused);
      else if (errno == ENETDOWN || errno == ENETUNREACH ||
	       errno == ENETRESET || errno == ECONNABORTED ||
	       errno == ECONNRESET || errno == EHOSTDOWN ||
	       errno == EHOSTUNREACH)
	setError(IO_ConnectError, NetFailure);
      else
	setError(IO_ConnectError, NotSupported);

      return false;
    }

  setFlags(IO_Sequential | IO_Raw | IO_ReadWrite);
  setState(IO_Open);
  return true;			// all is well
}

TQ_LONG TDESocketDevice::bytesAvailable() const
{
  if (m_sockfd == -1)
    return -1;			// there's nothing to read in a closed socket

  int nchars;
  if (ioctl(m_sockfd, FIONREAD, &nchars) == -1)
    return -1;			// error!

  return nchars;
}

TQ_LONG TDESocketDevice::waitForMore(int msecs, bool *timeout)
{
  if (m_sockfd == -1)
    return -1;			// there won't ever be anything to read...

  bool input;
  if (!poll(&input, 0, 0, msecs, timeout))
    return -1;			// failed polling

  return bytesAvailable();
}

static int do_read_common(int sockfd, char *data, TQ_ULONG maxlen, TDESocketAddress* from, ssize_t &retval, bool peek = false)
{
  socklen_t len;
  if (from)
    {
      from->setLength(len = 128); // arbitrary length
      retval = ::recvfrom(sockfd, data, maxlen, peek ? MSG_PEEK : 0, from->address(), &len);
    }
  else
    retval = ::recvfrom(sockfd, data, maxlen, peek ? MSG_PEEK : 0, NULL, NULL);

  if (retval == -1)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
	return TDESocketDevice::WouldBlock;
      else
	return TDESocketDevice::UnknownError;
    }
  if (retval == 0)
    return TDESocketDevice::RemotelyDisconnected;

  if (from)
    from->setLength(len);
  return 0;
}

TQ_LONG TDESocketDevice::readBlock(char *data, TQ_ULONG maxlen)
{
  resetError();
  if (m_sockfd == -1)
    return -1;

  if (maxlen == 0 || data == 0L)
    return 0;			// can't read

  ssize_t retval;
  int err = do_read_common(m_sockfd, data, maxlen, 0L, retval);

  if (err)
    {
      setError(IO_ReadError, static_cast<SocketError>(err));
      return -1;
    }

  return retval;
}

TQ_LONG TDESocketDevice::readBlock(char *data, TQ_ULONG maxlen, TDESocketAddress &from)
{
  resetError();
  if (m_sockfd == -1)
    return -1;			// nothing to do here

  if (data == 0L || maxlen == 0)
    return 0;			// user doesn't want to read

  ssize_t retval;
  int err = do_read_common(m_sockfd, data, maxlen, &from, retval);

  if (err)
    {
      setError(IO_ReadError, static_cast<SocketError>(err));
      return -1;
    }

  return retval;
}

TQ_LONG TDESocketDevice::peekBlock(char *data, TQ_ULONG maxlen)
{
  resetError();
  if (m_sockfd == -1)
    return -1;

  if (maxlen == 0 || data == 0L)
    return 0;			// can't read

  ssize_t retval;
  int err = do_read_common(m_sockfd, data, maxlen, 0L, retval, true);

  if (err)
    {
      setError(IO_ReadError, static_cast<SocketError>(err));
      return -1;
    }

  return retval;
}

TQ_LONG TDESocketDevice::peekBlock(char *data, TQ_ULONG maxlen, TDESocketAddress& from)
{
  resetError();
  if (m_sockfd == -1)
    return -1;			// nothing to do here

  if (data == 0L || maxlen == 0)
    return 0;			// user doesn't want to read

  ssize_t retval;
  int err = do_read_common(m_sockfd, data, maxlen, &from, retval, true);

  if (err)
    {
      setError(IO_ReadError, static_cast<SocketError>(err));
      return -1;
    }

  return retval;
}

TQ_LONG TDESocketDevice::writeBlock(const char *data, TQ_ULONG len)
{
  return writeBlock(data, len, TDESocketAddress());
}

TQ_LONG TDESocketDevice::writeBlock(const char *data, TQ_ULONG len, const TDESocketAddress& to)
{
  resetError();
  if (m_sockfd == -1)
    return -1;			// can't write to unopen socket

  if (data == 0L || len == 0)
    return 0;			// nothing to be written

  ssize_t retval = ::sendto(m_sockfd, data, len, 0, to.address(), to.length());
  if (retval == -1)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
	setError(IO_WriteError, WouldBlock);
      else
	setError(IO_WriteError, UnknownError);
      return -1;		// nothing written
    }
  else if (retval == 0)
    setError(IO_WriteError, RemotelyDisconnected);

  return retval;
}

TDESocketAddress TDESocketDevice::localAddress() const
{
  if (m_sockfd == -1)
    return TDESocketAddress();	// not open, empty value

  if (d->local.family() != AF_UNSPEC)
    return d->local;

  socklen_t len;
  TDESocketAddress localAddress;
  localAddress.setLength(len = 32);	// arbitrary value
  if (kde_getsockname(m_sockfd, localAddress.address(), &len) == -1)
    // error!
    return d->local = TDESocketAddress();

#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
  len = localAddress.address()->sa_len;
#endif

  if (len <= localAddress.length())
    {
      // it has fit already
      localAddress.setLength(len);
      return d->local = localAddress;
    }

  // no, the socket address is actually larger than we had anticipated
  // call again
  localAddress.setLength(len);
  if (kde_getsockname(m_sockfd, localAddress.address(), &len) == -1)
    // error!
    return d->local = TDESocketAddress();

  return d->local = localAddress;
}

TDESocketAddress TDESocketDevice::peerAddress() const
{
  if (m_sockfd == -1)
    return TDESocketAddress();	// not open, empty value

  if (d->peer.family() != AF_UNSPEC)
    return d->peer;

  socklen_t len;
  TDESocketAddress peerAddress;
  peerAddress.setLength(len = 32);	// arbitrary value
  if (kde_getpeername(m_sockfd, peerAddress.address(), &len) == -1)
    // error!
    return d->peer = TDESocketAddress();

#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
  len = peerAddress.address()->sa_len;
#endif

  if (len <= peerAddress.length())
    {
      // it has fit already
      peerAddress.setLength(len);
      return d->peer = peerAddress;
    }

  // no, the socket address is actually larger than we had anticipated
  // call again
  peerAddress.setLength(len);
  if (kde_getpeername(m_sockfd, peerAddress.address(), &len) == -1)
    // error!
    return d->peer = TDESocketAddress();

  return d->peer = peerAddress;
}

TDESocketAddress TDESocketDevice::externalAddress() const
{
  // for normal sockets, the externally visible address is the same
  // as the local address
  return localAddress();
}

TQSocketNotifier* TDESocketDevice::readNotifier() const
{
  if (d->input)
    return d->input;

  TQMutexLocker locker(mutex());
  if (d->input)
    return d->input;

  if (m_sockfd == -1)
    {
      // socket doesn't exist; can't create notifier
      return 0L;
    }

  return d->input = createNotifier(TQSocketNotifier::Read);
}

TQSocketNotifier* TDESocketDevice::writeNotifier() const
{
  if (d->output)
    return d->output;

  TQMutexLocker locker(mutex());
  if (d->output)
    return d->output;

  if (m_sockfd == -1)
    {
      // socket doesn't exist; can't create notifier
      return 0L;
    }

  return d->output = createNotifier(TQSocketNotifier::Write);
}

TQSocketNotifier* TDESocketDevice::exceptionNotifier() const
{
  if (d->exception)
    return d->exception;

  TQMutexLocker locker(mutex());
  if (d->exception)
    return d->exception;

  if (m_sockfd == -1)
    {
      // socket doesn't exist; can't create notifier
      return 0L;
    }

  return d->exception = createNotifier(TQSocketNotifier::Exception);
}

bool TDESocketDevice::poll(bool *input, bool *output, bool *exception,
			 int timeout, bool* timedout)
{
  if (m_sockfd == -1)
    {
      setError(IO_UnspecifiedError, NotCreated);
      return false;
    }

  resetError();
#ifdef HAVE_POLL
  struct pollfd fds;
  fds.fd = m_sockfd;
  fds.events = 0;

  if (input)
    {
      fds.events |= POLLIN;
      *input = false;
    }
  if (output)
    {
      fds.events |= POLLOUT;
      *output = false;
    }
  if (exception)
    {
      fds.events |= POLLPRI;
      *exception = false;
    }

  int retval = ::poll(&fds, 1, timeout);
  if (retval == -1)
    {
      setError(IO_UnspecifiedError, UnknownError);
      return false;
    }
  if (retval == 0)
    {
      // timeout
      if (timedout)
	*timedout = true;
      return true;
    }

  if (input && fds.revents & POLLIN)
    *input = true;
  if (output && fds.revents & POLLOUT)
    *output = true;
  if (exception && fds.revents & POLLPRI)
    *exception = true;
  if (timedout)
    *timedout = false;

  return true;
#else
  /*
   * We don't have poll(2). We'll have to make do with select(2).
   */

  fd_set readfds, writefds, exceptfds;
  fd_set *preadfds = 0L, *pwritefds = 0L, *pexceptfds = 0L;

  if (input)
    {
      preadfds = &readfds;
      FD_ZERO(preadfds);
      FD_SET(m_sockfd, preadfds);
      *input = false;
    }
  if (output)
    {
      pwritefds = &writefds;
      FD_ZERO(pwritefds);
      FD_SET(m_sockfd, pwritefds);
      *output = false;
    }
  if (exception)
    {
      pexceptfds = &exceptfds;
      FD_ZERO(pexceptfds);
      FD_SET(m_sockfd, pexceptfds);
      *exception = false;
    }

  int retval;
  if (timeout < 0)
    retval = select(m_sockfd + 1, preadfds, pwritefds, pexceptfds, 0L);
  else
    {
      // convert the milliseconds to timeval
      struct timeval tv;
      tv.tv_sec = timeout / 1000;
      tv.tv_usec = timeout % 1000 * 1000;

      retval = select(m_sockfd + 1, preadfds, pwritefds, pexceptfds, &tv);
    }

  if (retval == -1)
    {
      setError(IO_UnspecifiedError, UnknownError);
      return false;
    }
  if (retval == 0)
    {
      // timeout
      if (timedout)
	*timedout = true;
      return true;
    }

  if (input && FD_ISSET(m_sockfd, preadfds))
    *input = true;
  if (output && FD_ISSET(m_sockfd, pwritefds))
    *output = true;
  if (exception && FD_ISSET(m_sockfd, pexceptfds))
    *exception = true;

  return true;
#endif
}

bool TDESocketDevice::poll(int timeout, bool *timedout)
{
  bool input, output, exception;
  return poll(&input, &output, &exception, timeout, timedout);
}

TQSocketNotifier* TDESocketDevice::createNotifier(TQSocketNotifier::Type type) const
{
  if (m_sockfd == -1)
    return 0L;

  return new TQSocketNotifier(m_sockfd, type);
}

namespace
{
  // simple class to avoid pointer stuff
  template<class T> class ptr
  {
    typedef T type;
    type* obj;
  public:
    ptr() : obj(0)
    { }

    ptr(const ptr<T>& other) : obj(other.obj)
    { }

    ptr(type* _obj) : obj(_obj)
    { }

    ~ptr()
    { }

    ptr<T>& operator=(const ptr<T>& other)
    { obj = other.obj; return *this; }

    ptr<T>& operator=(T* _obj)
    { obj = _obj; return  *this; }

    type* operator->() const { return obj; }

    operator T*() const { return obj; }

    bool isNull() const
    { return obj == 0; }
  };
}

static TDESocketDeviceFactoryBase* defaultImplFactory;
static TQMutex defaultImplFactoryMutex;
typedef TQMap<int, TDESocketDeviceFactoryBase* > factoryMap;
static factoryMap factories;
 
TDESocketDevice* TDESocketDevice::createDefault(TDESocketBase* parent)
{
  TDESocketDevice* device = dynamic_cast<TDESocketDevice*>(parent);
  if (device != 0L)
    return device;

  KSocksSocketDevice::initSocks();

  if (defaultImplFactory)
    return defaultImplFactory->create(parent);

  // the really default
  return new TDESocketDevice(parent);
}

TDESocketDevice* TDESocketDevice::createDefault(TDESocketBase* parent, int capabilities)
{
  TDESocketDevice* device = dynamic_cast<TDESocketDevice*>(parent);
  if (device != 0L)
    return device;

  TQMutexLocker locker(&defaultImplFactoryMutex);
  factoryMap::ConstIterator it = factories.constBegin();
  for ( ; it != factories.constEnd(); ++it)
    if ((it.key() & capabilities) == capabilities)
      // found a match
      return it.data()->create(parent);

  return 0L;			// no default
}

TDESocketDeviceFactoryBase*
TDESocketDevice::setDefaultImpl(TDESocketDeviceFactoryBase* factory)
{
  TQMutexLocker locker(&defaultImplFactoryMutex);
  TDESocketDeviceFactoryBase* old = defaultImplFactory;
  defaultImplFactory = factory;
  return old;
}

void TDESocketDevice::addNewImpl(TDESocketDeviceFactoryBase* factory, int capabilities)
{
  TQMutexLocker locker(&defaultImplFactoryMutex);
  if (factories.contains(capabilities))
    delete factories[capabilities];
  factories.insert(capabilities, factory);
}

