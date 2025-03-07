/*
 *  Copyright (C) 2003,2004 Thiago Macieira <thiago.macieira@kdemail.net>
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

#include <sys/types.h>
#include <sys/socket.h>

#include "tdesocketaddress.h"
#include "kresolver.h"
#include "tdesocketdevice.h"
#include "kdatagramsocket.h"

using namespace KNetwork;

/*
 * TODO:
 *
 * don't use signals and slots to track state changes: use stateChanging
 *
 */

KDatagramSocket::KDatagramSocket(TQObject* parent, const char *name)
  : KClientSocketBase(parent, name), d(0L)
{
  peerResolver().setFamily(KResolver::KnownFamily);
  localResolver().setFamily(KResolver::KnownFamily);

  peerResolver().setSocketType(SOCK_DGRAM);
  localResolver().setSocketType(SOCK_DGRAM);

  localResolver().setFlags(KResolver::Passive);

  //  TQObject::connect(localResolver(), TQ_SIGNAL(finished(KResolverResults)),
  //		   this, TQ_SLOT(lookupFinishedLocal()));
  TQObject::connect(&peerResolver(), TQ_SIGNAL(finished(KResolverResults)),
  		   this, TQ_SLOT(lookupFinishedPeer()));
  TQObject::connect(this, TQ_SIGNAL(hostFound()), this, TQ_SLOT(lookupFinishedLocal()));
}

KDatagramSocket::~KDatagramSocket()
{
  // KClientSocketBase's destructor closes the socket

  //delete d;
}

bool KDatagramSocket::bind(const TQString& node, const TQString& service)
{
  if (state() >= Bound)
    return false;

  if (localResolver().isRunning())
    localResolver().cancel(false);

  // no, we must do a host lookup
  localResolver().setAddress(node, service);

  if (!lookup())
    return false;

  // see if lookup has finished already
  // this also catches blocking mode, since lookup has to finish
  // its processing if we're in blocking mode
  if (state() > HostLookup)
    return doBind();

  return true;
}

bool KDatagramSocket::connect(const TQString& node, const TQString& service)
{
  if (state() >= Connected)
    return true;		// already connected

  if (peerResolver().nodeName() != node ||
      peerResolver().serviceName() != service)
    peerResolver().setAddress(node, service); // this resets the resolver's state

  // KClientSocketBase::lookup only works if the state is Idle or HostLookup
  // therefore, we store the old state, call the lookup routine and then set
  // it back.
  SocketState s = state();
  setState(s == Connecting ? HostLookup : Idle);
  bool ok = lookup();
  if (!ok)
    {
      setState(s);		// go back
      return false;
    }

  // check if lookup is finished
  // if we're in blocking mode, then the lookup has to be finished
  if (state() == HostLookup)
    {
      // it hasn't finished
      setState(Connecting);
      emit stateChanged(Connecting);
      return true;
    }

  // it has to be finished here
  if (state() != Connected)
    {
      setState(Connecting);
      emit stateChanged(Connecting);
      lookupFinishedPeer();
    }

  return state() == Connected;
}

KDatagramPacket KDatagramSocket::receive()
{
  TQ_LONG size = bytesAvailable();
  if (size == 0)
    {
      // nothing available yet to read
      // wait for data if we're not blocking
      if (blocking())
	socketDevice()->waitForMore(-1); // wait forever
      else
	{
	  // mimic error
	  setError(IO_ReadError, WouldBlock);
	  emit gotError(WouldBlock);
	  return KDatagramPacket();
	}

      // try again
      size = bytesAvailable();
    }

  TQByteArray data(size);
  TDESocketAddress address;
  
  // now do the reading
  size = readBlock(data.data(), size, address);
  if (size < 0)
    // error has been set
    return KDatagramPacket();

  data.resize(size);		// just to be sure
  return KDatagramPacket(data, address);
}

TQ_LONG KDatagramSocket::send(const KDatagramPacket& packet)
{
  return writeBlock(packet.data(), packet.size(), packet.address());
}

TQ_LONG KDatagramSocket::writeBlock(const char *data, TQ_ULONG len, const TDESocketAddress& to)
{
  if (to.family() != AF_UNSPEC)
    {
      // make sure the socket is open at this point
      if (!socketDevice()->isOpen())
	// error handling will happen below
	socketDevice()->create(to.family(), SOCK_DGRAM, 0);
    }
  return KClientSocketBase::writeBlock(data, len, to);
}

void KDatagramSocket::lookupFinishedLocal()
{
  // bind lookup has finished and succeeded
  // state() == HostFound

  if (!doBind())
    return;			// failed binding

  if (peerResults().count() > 0)
    {
      setState(Connecting);
      emit stateChanged(Connecting);

      lookupFinishedPeer();
    }
}

void KDatagramSocket::lookupFinishedPeer()
{
  // this function is called by lookupFinishedLocal above
  // and is also connected to a signal
  // so it might be called twice.

  if (state() != Connecting)
    return;

  if (peerResults().count() == 0)
    {
      setState(Unconnected);
      emit stateChanged(Unconnected);
      return;
    }

  KResolverResults::ConstIterator it = peerResults().begin();
  for ( ; it != peerResults().end(); ++it)
    if (connect(*it))
      {
	// weee, we connected

	setState(Connected);	// this sets up signals
	//setupSignals();	// setState sets up the signals

	emit stateChanged(Connected);
	emit connected(*it);
	return;
      }

  // no connection
  copyError();
  setState(Unconnected);
  emit stateChanged(Unconnected);
  emit gotError(error());
}

bool KDatagramSocket::doBind()
{
  if (localResults().count() == 0)
    return true;
  if (state() >= Bound)
    return true;		// already bound

  KResolverResults::ConstIterator it = localResults().begin();
  for ( ; it != localResults().end(); ++it)
    if (bind(*it))
      {
	// bound
	setupSignals();
	return true;
      }

  // not bound
  // no need to set state since it can only be HostFound already
  copyError();
  emit gotError(error());
  return false;
}

void KDatagramSocket::setupSignals()
{
  TQSocketNotifier *n = socketDevice()->readNotifier();
  if (n)
    {
      n->setEnabled(emitsReadyRead());
      TQObject::connect(n, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotReadActivity()));
    }
  else
    return;

  n = socketDevice()->writeNotifier();
  if (n)
    {
      n->setEnabled(emitsReadyWrite());
      TQObject::connect(n, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotWriteActivity()));
    }
  else
    return;
}

#include "kdatagramsocket.moc"
