/*
 *  Copyright (C) 2003-2005 Thiago Macieira <thiago.macieira@kdemail.net>
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
#include <tqmutex.h>
#include "tdelocale.h"

#include "tdesocketbase.h"
#include "tdesocketdevice.h"

using namespace KNetwork;

class KNetwork::TDESocketBasePrivate
{
public:
  int socketOptions;
  int socketError;
  int capabilities;

  mutable TDESocketDevice* device;

  TQMutex mutex;

  TDESocketBasePrivate()
    : mutex(true)		// create recursive
  { }
};

TDESocketBase::TDESocketBase()
  : d(new TDESocketBasePrivate)
{
  d->socketOptions = Blocking;
  d->socketError = 0;
  d->device = 0L;
  d->capabilities = 0;
}

TDESocketBase::~TDESocketBase()
{
  delete d->device;
  delete d;
}

bool TDESocketBase::setSocketOptions(int opts)
{
  d->socketOptions = opts;
  return true;
}

int TDESocketBase::socketOptions() const
{
  return d->socketOptions;
}

bool TDESocketBase::setBlocking(bool enable)
{
  return setSocketOptions((socketOptions() & ~Blocking) | (enable ? Blocking : 0));
}

bool TDESocketBase::blocking() const
{
  return socketOptions() & Blocking;
}

bool TDESocketBase::setAddressReuseable(bool enable)
{
  return setSocketOptions((socketOptions() & ~AddressReuseable) | (enable ? AddressReuseable : 0));
}

bool TDESocketBase::addressReuseable() const
{
  return socketOptions() & AddressReuseable;
}

bool TDESocketBase::setIPv6Only(bool enable)
{
  return setSocketOptions((socketOptions() & ~IPv6Only) | (enable ? IPv6Only : 0));
}

bool TDESocketBase::isIPv6Only() const
{
  return socketOptions() & IPv6Only;
}

bool TDESocketBase::setBroadcast(bool enable)
{
  return setSocketOptions((socketOptions() & ~Broadcast) | (enable ? Broadcast : 0));
}

bool TDESocketBase::broadcast() const
{
  return socketOptions() & Broadcast;
}

TDESocketDevice* TDESocketBase::socketDevice() const
{
  if (d->device)
    return d->device;

  // it doesn't exist, so create it
  TQMutexLocker locker(mutex());
  if (d->device)
    return d->device;

  TDESocketBase* that = const_cast<TDESocketBase*>(this);
  TDESocketDevice* dev = 0;
  if (d->capabilities)
    dev = TDESocketDevice::createDefault(that, d->capabilities);
  if (!dev)
    dev = TDESocketDevice::createDefault(that);
  that->setSocketDevice(dev);
  return d->device;
}

void TDESocketBase::setSocketDevice(TDESocketDevice* device)
{
  TQMutexLocker locker(mutex());
  if (d->device == 0L)
    d->device = device;
}

int TDESocketBase::setRequestedCapabilities(int add, int remove)
{
  d->capabilities |= add;
  d->capabilities &= ~remove;
  return d->capabilities;
}

bool TDESocketBase::hasDevice() const
{
  return d->device != 0L;
}

void TDESocketBase::setError(SocketError error)
{
  d->socketError = error;
}

TDESocketBase::SocketError TDESocketBase::error() const
{
  return static_cast<TDESocketBase::SocketError>(d->socketError);
}

// static
TQString TDESocketBase::errorString(TDESocketBase::SocketError code)
{
  TQString reason;
  switch (code)
    {
    case NoError:
      reason = i18n("Socket error code NoError", "no error");
      break;

    case LookupFailure:
      reason = i18n("Socket error code LookupFailure",
		    "name lookup has failed");
      break;

    case AddressInUse:
      reason = i18n("Socket error code AddressInUse",
		    "address already in use");
      break;

    case AlreadyBound:
      reason = i18n("Socket error code AlreadyBound",
		    "socket is already bound");
      break;

    case AlreadyCreated:
      reason = i18n("Socket error code AlreadyCreated",
		    "socket is already created");
      break;
      
    case NotBound:
      reason = i18n("Socket error code NotBound",
		    "socket is not bound");
      break;

    case NotCreated:
      reason = i18n("Socket error code NotCreated",
		    "socket has not been created");
      break;

    case WouldBlock:
      reason = i18n("Socket error code WouldBlock",
		    "operation would block");
      break;

    case ConnectionRefused:
      reason = i18n("Socket error code ConnectionRefused",
		    "connection actively refused");
      break;

    case ConnectionTimedOut:
      reason = i18n("Socket error code ConnectionTimedOut",
		    "connection timed out");
      break;

    case InProgress:
      reason = i18n("Socket error code InProgress",
		    "operation is already in progress");
      break;

    case NetFailure:
      reason = i18n("Socket error code NetFailure",
		    "network failure occurred");
      break;

    case NotSupported:
      reason = i18n("Socket error code NotSupported",
		    "operation is not supported");
      break;

    case Timeout:
      reason = i18n("Socket error code Timeout",
		    "timed operation timed out");
      break;

    case UnknownError:
      reason = i18n("Socket error code UnknownError",
		    "an unknown/unexpected error has happened");
      break;

    case RemotelyDisconnected:
      reason = i18n("Socket error code RemotelyDisconnected",
		    "remote host closed connection");
      break;

    default:
      reason = TQString::null;
      break;
    }

  return reason;
}

// static
bool TDESocketBase::isFatalError(int code)
{
  switch (code)
    {
    case WouldBlock:
    case InProgress:
    case NoError:
    case RemotelyDisconnected:
      return false;
    }

  return true;
}

void TDESocketBase::unsetSocketDevice()
{
  d->device = 0L;
}

TQMutex* TDESocketBase::mutex() const
{
  return &d->mutex;
}

KActiveSocketBase::KActiveSocketBase()
{
}

KActiveSocketBase::~KActiveSocketBase()
{
}

int KActiveSocketBase::getch()
{
  unsigned char c;
  if (readBlock((char*)&c, 1) != 1)
    return -1;

  return c;
}

int KActiveSocketBase::putch(int ch)
{
  unsigned char c = (unsigned char)ch;
  if (writeBlock((char*)&c, 1) != 1)
    return -1;

  return c;
}

void KActiveSocketBase::setError(int status, SocketError error)
{
  TDESocketBase::setError(error);
  setStatus(status);
}

void KActiveSocketBase::resetError()
{
  TDESocketBase::setError(NoError);
  resetStatus();
}

KPassiveSocketBase::KPassiveSocketBase()
{
}

KPassiveSocketBase::~KPassiveSocketBase()
{
}
