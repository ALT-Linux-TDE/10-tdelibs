/*
 *  Copyright (C) 2004 Thiago Macieira <thiago.macieira@kdemail.net>
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

#include <config.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#ifdef __CYGWIN__
#undef kde_socklen_t 
#define kde_socklen_t ksocklen_t 
#endif 

#include "tdeapplication.h"

#include "ksocks.h"
#include "tdesocketaddress.h"
#include "kresolver.h"
#include "ksockssocketdevice.h"

using namespace KNetwork;

// constructor
// nothing to do
KSocksSocketDevice::KSocksSocketDevice(const TDESocketBase* obj)
  : TDESocketDevice(obj)
{
}

// constructor with argument
// nothing to do
KSocksSocketDevice::KSocksSocketDevice(int fd)
  : TDESocketDevice(fd)
{
}

// destructor
// also nothing to do
KSocksSocketDevice::~KSocksSocketDevice()
{
}

// returns the capabilities
int KSocksSocketDevice::capabilities() const
{
  return 0;			// can do everything!
}

// From here on, the code is almost exactly a copy of TDESocketDevice
// the differences are the use of KSocks where appropriate

bool KSocksSocketDevice::bind(const KResolverEntry& address)
{
  resetError();

  if (m_sockfd == -1 && !create(address))
    return false;		// failed creating

  // we have a socket, so try and bind
  if (KSocks::self()->bind(m_sockfd, address.address(), address.length()) == -1)
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


bool KSocksSocketDevice::listen(int backlog)
{
  if (m_sockfd != -1)
    {
      if (KSocks::self()->listen(m_sockfd, backlog) == -1)
	{
	  setError(IO_ListenError, NotSupported);
	  return false;
	}

      resetError();
      setFlags(IO_Sequential | IO_Raw | IO_ReadWrite);
      setState(IO_Open);
      return true;
    }

  // we don't have a socket
  // can't listen
  setError(IO_ListenError, NotCreated);
  return false;
}

bool KSocksSocketDevice::connect(const KResolverEntry& address)
{
  resetError();

  if (m_sockfd == -1 && !create(address))
    return false;		// failed creating!

  int retval;
  if (KSocks::self()->hasWorkingAsyncConnect())
    retval = KSocks::self()->connect(m_sockfd, address.address(), 
				     address.length());
  else
    {
      // work around some SOCKS implementation bugs
      // we will do a *synchronous* connection here!
      // FIXME: KDE4, write a proper SOCKS implementation
      bool isBlocking = blocking();
      setBlocking(true);
      retval = KSocks::self()->connect(m_sockfd, address.address(), 
				       address.length());
      setBlocking(isBlocking);
    }

  if (retval == -1)
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
  setState(IO_Open);
  return true;			// all is well
}

KSocksSocketDevice* KSocksSocketDevice::accept()
{
  if (m_sockfd == -1)
    {
      // can't accept without a socket
      setError(IO_AcceptError, NotCreated);
      return 0L;
    }

  struct sockaddr sa;
  kde_socklen_t len = sizeof(sa);
  int newfd = KSocks::self()->accept(m_sockfd, &sa, &len);
  if (newfd == -1)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
	setError(IO_AcceptError, WouldBlock);
      else
	setError(IO_AcceptError, UnknownError);
      return NULL;
    }

  return new KSocksSocketDevice(newfd);
}

static int socks_read_common(int sockfd, char *data, TQ_ULONG maxlen, TDESocketAddress* from, ssize_t &retval, bool peek = false)
{
  kde_socklen_t len;
  if (from)
    {
      from->setLength(len = 128); // arbitrary length
      retval = KSocks::self()->recvfrom(sockfd, data, maxlen, peek ? MSG_PEEK : 0, from->address(), &len);
    }
  else
    retval = KSocks::self()->recvfrom(sockfd, data, maxlen, peek ? MSG_PEEK : 0, NULL, NULL);

  if (retval == -1)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
	return TDESocketDevice::WouldBlock;
      else
	return TDESocketDevice::UnknownError;
    }

  if (from)
    from->setLength(len);
  return 0;
}

TQ_LONG KSocksSocketDevice::readBlock(char *data, TQ_ULONG maxlen)
{
  resetError();
  if (m_sockfd == -1)
    return -1;

  if (maxlen == 0 || data == 0L)
    return 0;			// can't read

  ssize_t retval;
  int err = socks_read_common(m_sockfd, data, maxlen, 0L, retval);

  if (err)
    {
      setError(IO_ReadError, static_cast<SocketError>(err));
      return -1;
    }

  return retval;
}

TQ_LONG KSocksSocketDevice::readBlock(char *data, TQ_ULONG maxlen, TDESocketAddress &from)
{
  resetError();
  if (m_sockfd == -1)
    return -1;			// nothing to do here

  if (data == 0L || maxlen == 0)
    return 0;			// user doesn't want to read

  ssize_t retval;
  int err = socks_read_common(m_sockfd, data, maxlen, &from, retval);

  if (err)
    {
      setError(IO_ReadError, static_cast<SocketError>(err));
      return -1;
    }

  return retval;
}

TQ_LONG KSocksSocketDevice::peekBlock(char *data, TQ_ULONG maxlen)
{
  resetError();
  if (m_sockfd == -1)
    return -1;

  if (maxlen == 0 || data == 0L)
    return 0;			// can't read

  ssize_t retval;
  int err = socks_read_common(m_sockfd, data, maxlen, 0L, retval, true);

  if (err)
    {
      setError(IO_ReadError, static_cast<SocketError>(err));
      return -1;
    }

  return retval;
}

TQ_LONG KSocksSocketDevice::peekBlock(char *data, TQ_ULONG maxlen, TDESocketAddress& from)
{
  resetError();
  if (m_sockfd == -1)
    return -1;			// nothing to do here

  if (data == 0L || maxlen == 0)
    return 0;			// user doesn't want to read

  ssize_t retval;
  int err = socks_read_common(m_sockfd, data, maxlen, &from, retval, true);

  if (err)
    {
      setError(IO_ReadError, static_cast<SocketError>(err));
      return -1;
    }

  return retval;
}

TQ_LONG KSocksSocketDevice::writeBlock(const char *data, TQ_ULONG len)
{
  return writeBlock(data, len, TDESocketAddress());
}

TQ_LONG KSocksSocketDevice::writeBlock(const char *data, TQ_ULONG len, const TDESocketAddress& to)
{
  resetError();
  if (m_sockfd == -1)
    return -1;			// can't write to unopen socket

  if (data == 0L || len == 0)
    return 0;			// nothing to be written

  ssize_t retval = KSocks::self()->sendto(m_sockfd, data, len, 0, to.address(), to.length());
  if (retval == -1)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
	setError(IO_WriteError, WouldBlock);
      else
	setError(IO_WriteError, UnknownError);
      return -1;		// nothing written
    }

  return retval;
}

TDESocketAddress KSocksSocketDevice::localAddress() const
{
  if (m_sockfd == -1)
    return TDESocketAddress();	// not open, empty value

  kde_socklen_t len;
  TDESocketAddress localAddress;
  localAddress.setLength(len = 32);	// arbitrary value
  if (KSocks::self()->getsockname(m_sockfd, localAddress.address(), &len) == -1)
    // error!
    return TDESocketAddress();

  if (len <= localAddress.length())
    {
      // it has fit already
      localAddress.setLength(len);
      return localAddress;
    }

  // no, the socket address is actually larger than we had anticipated
  // call again
  localAddress.setLength(len);
  if (KSocks::self()->getsockname(m_sockfd, localAddress.address(), &len) == -1)
    // error!
    return TDESocketAddress();

  return localAddress;
}

TDESocketAddress KSocksSocketDevice::peerAddress() const
{
  if (m_sockfd == -1)
    return TDESocketAddress();	// not open, empty value

  kde_socklen_t len;
  TDESocketAddress peerAddress;
  peerAddress.setLength(len = 32);	// arbitrary value
  if (KSocks::self()->getpeername(m_sockfd, peerAddress.address(), &len) == -1)
    // error!
    return TDESocketAddress();

  if (len <= peerAddress.length())
    {
      // it has fit already
      peerAddress.setLength(len);
      return peerAddress;
    }

  // no, the socket address is actually larger than we had anticipated
  // call again
  peerAddress.setLength(len);
  if (KSocks::self()->getpeername(m_sockfd, peerAddress.address(), &len) == -1)
    // error!
    return TDESocketAddress();

  return peerAddress;
}

TDESocketAddress KSocksSocketDevice::externalAddress() const
{
  // return empty, indicating unknown external address
  return TDESocketAddress();
}

bool KSocksSocketDevice::poll(bool *input, bool *output, bool *exception,
			      int timeout, bool *timedout)
{
  if (m_sockfd == -1)
    {
      setError(IO_UnspecifiedError, NotCreated);
      return false;
    }

  resetError();
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
    retval = KSocks::self()->select(m_sockfd + 1, preadfds, pwritefds, pexceptfds, 0L);
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
}

void KSocksSocketDevice::initSocks()
{
  static bool init = false;

  if (init)
    return;

  if (kapp == 0L)
    return;			// no TDEApplication, so don't initialise
                                // this should, however, test for TDEInstance

  init = true;

  if (KSocks::self()->hasSocks())
    delete TDESocketDevice::setDefaultImpl(new TDESocketDeviceFactory<KSocksSocketDevice>);
}

#if 0
static bool register()
{
  TDESocketDevice::addNewImpl(new TDESocketDeviceFactory<KSocksSocketDevice>, 0);
}

static bool register = registered();
#endif
