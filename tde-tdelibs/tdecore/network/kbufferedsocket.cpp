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
#include <tqtimer.h>

#include "tdesocketdevice.h"
#include "tdesocketaddress.h"
#include "tdesocketbuffer_p.h"
#include "kbufferedsocket.h"

using namespace KNetwork;
using namespace KNetwork::Internal;

class KNetwork::TDEBufferedSocketPrivate
{
public:
  mutable TDESocketBuffer *input, *output;

  TDEBufferedSocketPrivate()
  {
    input = 0L;
    output = 0L;
  }
};

TDEBufferedSocket::TDEBufferedSocket(const TQString& host, const TQString& service,
				 TQObject *parent, const char *name)
  : KStreamSocket(host, service, parent, name),
    d(new TDEBufferedSocketPrivate)
{
  setInputBuffering(true);
  setOutputBuffering(true);
}

TDEBufferedSocket::~TDEBufferedSocket()
{
  closeNow();
  delete d->input;
  delete d->output;
  delete d;
}

void TDEBufferedSocket::setSocketDevice(TDESocketDevice* device)
{
  KStreamSocket::setSocketDevice(device);
  device->setBlocking(false);
}

bool TDEBufferedSocket::setSocketOptions(int opts)
{
  if (opts == Blocking)
    return false;

  opts &= ~Blocking;
  return KStreamSocket::setSocketOptions(opts);
}

void TDEBufferedSocket::close()
{
  if (!d->output || d->output->isEmpty())
    closeNow();
  else
    {
      setState(Closing);
      TQSocketNotifier *n = socketDevice()->readNotifier();
      if (n)
	n->setEnabled(false);
      emit stateChanged(Closing);
    }
}

TQ_LONG TDEBufferedSocket::bytesAvailable() const
{
  if (!d->input)
    return KStreamSocket::bytesAvailable();

  return d->input->length();
}

TQ_LONG TDEBufferedSocket::waitForMore(int msecs, bool *timeout)
{
  TQ_LONG retval = KStreamSocket::waitForMore(msecs, timeout);
  if (d->input)
    {
      resetError();
      slotReadActivity();
      return bytesAvailable();
    }
  return retval;
}

TQ_LONG TDEBufferedSocket::readBlock(char *data, TQ_ULONG maxlen)
{
  if (d->input)
    {
      if (d->input->isEmpty())
	{
	  setError(IO_ReadError, WouldBlock);
	  emit gotError(WouldBlock);
	  return -1;
	}
      resetError();
      return d->input->consumeBuffer(data, maxlen);
    }
  return KStreamSocket::readBlock(data, maxlen);
}

TQ_LONG TDEBufferedSocket::readBlock(char *data, TQ_ULONG maxlen, TDESocketAddress& from)
{
  from = peerAddress();
  return readBlock(data, maxlen);
}

TQ_LONG TDEBufferedSocket::peekBlock(char *data, TQ_ULONG maxlen)
{
  if (d->input)
    {
      if (d->input->isEmpty())
	{
	  setError(IO_ReadError, WouldBlock);
	  emit gotError(WouldBlock);
	  return -1;
	}
      resetError();
      return d->input->consumeBuffer(data, maxlen, false);
    }
  return KStreamSocket::peekBlock(data, maxlen);
}

TQ_LONG TDEBufferedSocket::peekBlock(char *data, TQ_ULONG maxlen, TDESocketAddress& from)
{
  from = peerAddress();
  return peekBlock(data, maxlen);
}

TQ_LONG TDEBufferedSocket::writeBlock(const char *data, TQ_ULONG len)
{
  if (state() != Connected)
    {
      // cannot write now!
      setError(IO_WriteError, NotConnected);
      return -1;
    }

  if (d->output)
    {
      if (d->output->isFull())
	{
	  setError(IO_WriteError, WouldBlock);
	  emit gotError(WouldBlock);
	  return -1;
	}
      resetError();

      // enable notifier to send data
      TQSocketNotifier *n = socketDevice()->writeNotifier();
      if (n)
	n->setEnabled(true);

      return d->output->feedBuffer(data, len);
    }

  return KStreamSocket::writeBlock(data, len);
}

TQ_LONG TDEBufferedSocket::writeBlock(const char *data, TQ_ULONG maxlen,
				   const TDESocketAddress&)
{
  // ignore the third parameter
  return writeBlock(data, maxlen);
}

void TDEBufferedSocket::enableRead(bool enable)
{
  KStreamSocket::enableRead(enable);
  if (!enable && d->input)
    {
      // reenable it
      TQSocketNotifier *n = socketDevice()->readNotifier();
      if (n)
	n->setEnabled(true);
    }

  if (enable && state() != Connected && d->input && !d->input->isEmpty())
    // this means the buffer is still dirty
    // allow the signal to be emitted
    TQTimer::singleShot(0, this, TQ_SLOT(slotReadActivity()));
}

void TDEBufferedSocket::enableWrite(bool enable)
{
  KStreamSocket::enableWrite(enable);
  if (!enable && d->output && !d->output->isEmpty())
    {
      // reenable it
      TQSocketNotifier *n = socketDevice()->writeNotifier();
      if (n)
	n->setEnabled(true);
    }
}

void TDEBufferedSocket::stateChanging(SocketState newState)
{
  if (newState == Connecting || newState == Connected)
    {
      // we're going to connect
      // make sure the buffers are clean
      if (d->input)
	d->input->clear();
      if (d->output)
	d->output->clear();

      // also, turn on notifiers
      enableRead(emitsReadyRead());
      enableWrite(emitsReadyWrite());
    }
  KStreamSocket::stateChanging(newState);
}

void TDEBufferedSocket::setInputBuffering(bool enable)
{
  TQMutexLocker locker(mutex());
  if (!enable)
    {
      delete d->input;
      d->input = 0L;
    }
  else if (d->input == 0L)
    {
      d->input = new TDESocketBuffer;
    }
}

TDEIOBufferBase* TDEBufferedSocket::inputBuffer()
{
  return d->input;
}

void TDEBufferedSocket::setOutputBuffering(bool enable)
{
  TQMutexLocker locker(mutex());
  if (!enable)
    {
      delete d->output;
      d->output = 0L;
    }
  else if (d->output == 0L)
    {
      d->output = new TDESocketBuffer;
    }
}

TDEIOBufferBase* TDEBufferedSocket::outputBuffer()
{
  return d->output;
}

TQ_ULONG TDEBufferedSocket::bytesToWrite() const
{
  if (!d->output)
    return 0;

  return d->output->length();
}

void TDEBufferedSocket::closeNow()
{
  KStreamSocket::close();
  if (d->output)
    d->output->clear();
}

bool TDEBufferedSocket::canReadLine() const
{
  if (!d->input)
    return false;

  return d->input->canReadLine();
}

TQCString TDEBufferedSocket::readLine()
{
  return d->input->readLine();
}

void TDEBufferedSocket::waitForConnect()
{
  if (state() != Connecting)
    return;			// nothing to be waited on

  KStreamSocket::setSocketOptions(socketOptions() | Blocking);
  connectionEvent();
  KStreamSocket::setSocketOptions(socketOptions() & ~Blocking);
}

void TDEBufferedSocket::slotReadActivity()
{
  if (d->input && state() == Connected)
    {
      mutex()->lock();
      TQ_LONG len = d->input->receiveFrom(socketDevice());

      if (len == -1)
	{
	  if (socketDevice()->error() != WouldBlock)
	    {
	      // nope, another error!
	      copyError();
	      mutex()->unlock();
	      emit gotError(error());
	      closeNow();	// emits closed
	      return;
	    }
	}
      else if (len == 0)
	{
	  // remotely closed
	  setError(IO_ReadError, RemotelyDisconnected);
	  mutex()->unlock();
	  emit gotError(error());
	  closeNow();		// emits closed
	  return;
	}

      // no error
      mutex()->unlock();
    }

  if (state() == Connected)
    KStreamSocket::slotReadActivity(); // this emits readyRead
  else if (emitsReadyRead())	// state() != Connected
    {
      if (d->input && !d->input->isEmpty())
	{
	  // buffer isn't empty
	  // keep emitting signals till it is
	  TQTimer::singleShot(0, this, TQ_SLOT(slotReadActivity()));
	  emit readyRead();
	}
    }
}

void TDEBufferedSocket::slotWriteActivity()
{
  if (d->output && !d->output->isEmpty() &&
      (state() == Connected || state() == Closing))
    {
      mutex()->lock();
      TQ_LONG len = d->output->sendTo(socketDevice());

      if (len == -1)
	{
	  if (socketDevice()->error() != WouldBlock)
	    {
	      // nope, another error!
	      copyError();
	      mutex()->unlock();
	      emit gotError(error());
	      closeNow();
	      return;
	    }
	}
      else if (len == 0)
	{
	  // remotely closed
	  setError(IO_ReadError, RemotelyDisconnected);
	  mutex()->unlock();
	  emit gotError(error());
	  closeNow();
	  return;
	}

      if (d->output->isEmpty())
	// deactivate the notifier until we have something to send
	// writeNotifier can't return NULL here
	socketDevice()->writeNotifier()->setEnabled(false);

      mutex()->unlock();
      emit bytesWritten(len);
    }

  if (state() != Closing)
    KStreamSocket::slotWriteActivity();
  else if (d->output && d->output->isEmpty() && state() == Closing)
    {
      KStreamSocket::close();	// finished sending data
    }
}

#include "kbufferedsocket.moc"
