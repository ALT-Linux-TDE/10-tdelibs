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

#include <config.h>

#include <assert.h>
#include <string.h>

#include "tdesocketbase.h"
#include "tdesocketbuffer_p.h"

using namespace KNetwork;
using namespace KNetwork::Internal;

TDESocketBuffer::TDESocketBuffer(TQ_LONG size)
  : m_mutex(true), m_offset(0), m_size(size), m_length(0)
{
}

TDESocketBuffer::TDESocketBuffer(const TDESocketBuffer& other)
  : TDEIOBufferBase(other), m_mutex(true)
{
  *this = other;
}

TDESocketBuffer::~TDESocketBuffer()
{
  // TQValueList takes care of deallocating memory
}

TDESocketBuffer& TDESocketBuffer::operator=(const TDESocketBuffer& other)
{
  TQMutexLocker locker1(&m_mutex);
  TQMutexLocker locker2(&other.m_mutex);

  TDEIOBufferBase::operator=(other);

  m_list = other.m_list;	// copy-on-write
  m_offset = other.m_offset;
  m_size = other.m_size;
  m_length = other.m_length;

  return *this;
}

bool TDESocketBuffer::canReadLine() const
{
  TQMutexLocker locker(&m_mutex);

  TQValueListConstIterator<TQByteArray> it = m_list.constBegin(),
    end = m_list.constEnd();
  TQIODevice::Offset offset = m_offset;

  // walk the buffer
  for ( ; it != end; ++it)
    {
      if ((*it).find('\n', offset) != -1)
	return true;
      if ((*it).find('\r', offset) != -1)
	return true;
      offset = 0;
    }

  return false;			// not found
}

TQCString TDESocketBuffer::readLine()
{
  if (!canReadLine())
    return TQCString();		// empty

  TQMutexLocker locker(&m_mutex);

  // find the offset of the newline in the buffer
  int newline = 0;
  TQValueListConstIterator<TQByteArray> it = m_list.constBegin(),
    end = m_list.constEnd();
  TQIODevice::Offset offset = m_offset;

  // walk the buffer
  for ( ; it != end; ++it)
    {
      int posnl = (*it).find('\n', offset);
      if (posnl == -1)
	{
	  // not found in this one
	  newline += (*it).size();
	  offset = 0;
	  continue;
	}

      // we found it
      newline += posnl;
      break;
    }

  TQCString result(newline + 2 - m_offset);
  consumeBuffer(result.data(), newline + 1 - m_offset);
  return result;
}

TQ_LONG TDESocketBuffer::length() const
{
  return m_length;
}

TQ_LONG TDESocketBuffer::size() const
{
  return m_size;
}

bool TDESocketBuffer::setSize(TQ_LONG size)
{
  m_size = size;
  if (size == -1 || m_length < m_size)
    return true;

  // size is now smaller than length
  TQMutexLocker locker(&m_mutex);

  // repeat the test
  if (m_length < m_size)
    return true;

  // discard from the beginning
  return (m_length - m_size) == consumeBuffer(0L, m_length - m_size, true);
}

TQ_LONG TDESocketBuffer::feedBuffer(const char *data, TQ_LONG len)
{
  if (data == 0L || len == 0)
    return 0;			// nothing to write
  if (isFull())
    return -1;			// can't write

  TQMutexLocker locker(&m_mutex);

  // verify if we can add len bytes
  if (m_size != -1 && (m_size - m_length) < len)
    len = m_size - m_length;

  TQByteArray a(len);
  a.duplicate(data, len);
  m_list.append(a);

  m_length += len;
  return len;
}

TQ_LONG TDESocketBuffer::consumeBuffer(char *destbuffer, TQ_LONG maxlen, bool discard)
{
  if (maxlen == 0 || isEmpty())
    return 0;

  TQValueListIterator<TQByteArray> it = m_list.begin(),
    end = m_list.end();
  TQIODevice::Offset offset = m_offset;
  TQ_LONG copied = 0;

  // walk the buffer
  while (it != end && maxlen)
    {
      // calculate how much we'll copy
      size_t to_copy = (*it).size() - offset;
      if (to_copy > (size_t)maxlen)
	to_copy = maxlen;

      // do the copying
      if (destbuffer)
	memcpy(destbuffer + copied, (*it).data() + offset, to_copy);
      maxlen -= to_copy;
      copied += to_copy;

      if ((*it).size() - offset > to_copy)
	{
	  // we did not copy everything
	  offset += to_copy;
	  break;
	}
      else
	{
	  // we copied everything
	  // discard this element;
	  offset = 0;
	  if (discard)
	    it = m_list.remove(it);
	  else
	    ++it;
	}
    }

  if (discard)
    {
      m_offset = offset;
      m_length -= copied;
      assert(m_length >= 0);
    }

  return copied;
}

void TDESocketBuffer::clear()
{
  TQMutexLocker locker(&m_mutex);
  m_list.clear();
  m_offset = 0;
  m_length = 0;
}

TQ_LONG TDESocketBuffer::sendTo(KActiveSocketBase* dev, TQ_LONG len)
{
  if (len == 0 || isEmpty())
    return 0;

  TQMutexLocker locker(&m_mutex);

  TQValueListIterator<TQByteArray> it = m_list.begin(),
    end = m_list.end();
  TQIODevice::Offset offset = m_offset;
  TQ_LONG written = 0;

  // walk the buffer
  while (it != end && (len || len == -1))
    {
      // we have to write each element up to len bytes
      // but since we can have several very small buffers, we can make things
      // better by concatenating a few of them into a big buffer
      // question is: how big should that buffer be? 2 kB should be enough

      TQ_ULONG bufsize = 1460;
      if ((len != -1) && ((TQ_ULONG)len < bufsize)) {
	bufsize = len;
      }
      TQByteArray buf(bufsize);
      TQ_LONG count = 0;

      while (it != end && count + ((*it).size() - offset) <= bufsize)
	{
	  memcpy(buf.data() + count, (*it).data() + offset, (*it).size() - offset);
	  count += (*it).size() - offset;
	  offset = 0;
	  ++it;
	}

      // see if we can still fit more
      if ((TQ_ULONG)count < bufsize && it != end)
	{
	  // getting here means this buffer (*it) is larger than
	  // (bufsize - count) (even for count == 0).
	  memcpy(buf.data() + count, (*it).data() + offset, bufsize - count);
	  offset += bufsize - count;
	  count = bufsize;
	}

      // now try to write those bytes
      TQ_LONG wrote = dev->writeBlock(buf, count);

      if (wrote == -1)
	// error?
	break;

      written += wrote;
      if (wrote != count)
	// can't fit more?
	break;
    }

  // discard data that has been written
  // this updates m_length too
  if (written)
    consumeBuffer(0L, written);

  return written;
}

TQ_LONG TDESocketBuffer::receiveFrom(KActiveSocketBase* dev, TQ_LONG len)
{
  if (len == 0 || isFull())
    return 0;

  TQMutexLocker locker(&m_mutex);

  if (len == -1)
    len = dev->bytesAvailable();
  if (len <= 0)
    // error or closing socket
    return len;

  // see if we can read that much
  if (m_size != -1 && len > (m_size - m_length))
    len = m_size - m_length;

  // here, len contains just as many bytes as we're supposed to read

  // now do the reading
  TQByteArray a(len);
  len = dev->readBlock(a.data(), len);

  if (len == -1)
    // error?
    return -1;

  // success
  // resize the buffer and add it
  a.truncate(len);
  m_list.append(a);
  m_length += len;
  return len;
}
