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

#ifndef TDESOCKETBUFFER_P_H
#define TDESOCKETBUFFER_P_H

#include <tqmutex.h>
#include <tqcstring.h>
#include <tqvaluelist.h>
#include "tdeiobuffer.h"

namespace KNetwork {

class KActiveSocketBase;

  namespace Internal {

/**
 * @internal
 * @class TDESocketBuffer tdesocketbuffer_p.h tdesocketbuffer_p.h
 * @brief generic socket buffering code
 *
 * This class implements generic buffering used by @ref TDEBufferedSocket.
 *
 * @author Thiago Macieira <thiago.macieira@kdemail.net>
 */
class TDESocketBuffer: public TDEIOBufferBase
{
public:
  /**
   * Default constructor.
   *
   * @param size	the maximum size of the buffer
   */
  TDESocketBuffer(TQ_LONG size = -1);

  /**
   * Copy constructor.
   */
  TDESocketBuffer(const TDESocketBuffer& other);

  /**
   * Virtual destructor. Frees the buffer and discards its contents.
   */
  virtual ~TDESocketBuffer();

  /**
   * Assignment operator.
   */
  TDESocketBuffer& operator=(const TDESocketBuffer& other);

  /**
   * Returns true if a line can be read from the buffer.
   */
  virtual bool canReadLine() const;

  /**
   * Reads a line from the buffer and discard it from the buffer.
   */
  virtual TQCString readLine();

  /**
   * Returns the number of bytes in the buffer. Note that this is not
   * the size of the buffer.
   *
   * @sa size
   */
  virtual TQ_LONG length() const;

  /**
   * Retrieves the buffer size. The value of -1 indicates that
   * the buffer has no defined upper limit.
   *
   * @sa length for the length of the data stored
   */
  virtual TQ_LONG size() const;

  /**
   * Sets the size of the buffer, if allowed.
   *
   * @param size	the maximum size, use -1 for unlimited.
   * @returns true on success, false if an error occurred.
   * @note if the new size is less than length(), the buffer will be truncated
   */
  virtual bool setSize(TQ_LONG size);

  /**
   * Adds data to the end of the buffer.
   *
   * @param data	the data to be added
   * @param len		the data length, in bytes
   * @returns the number of bytes added to the end of the buffer.
   */
  virtual TQ_LONG feedBuffer(const char *data, TQ_LONG len);

  /**
   * Clears the buffer.
   */
  virtual void clear();

  /**
   * Consumes data from the beginning of the buffer.
   *
   * @param data	where to copy the data to
   * @param maxlen	the maximum length to copy, in bytes
   * @param discard	if true, the bytes copied will be discarded
   * @returns the number of bytes copied from the buffer
   */
  virtual TQ_LONG consumeBuffer(char *data, TQ_LONG maxlen, bool discard = true);

  /**
   * Sends at most @p len bytes of data to the I/O Device.
   *
   * @param device	the device to which to send data
   * @param len		the amount of data to send; -1 to send everything
   * @returns the number of bytes sent and discarded from the buffer, -1
   *          indicates an error.
   */
  virtual TQ_LONG sendTo(KActiveSocketBase* device, TQ_LONG len = -1);

  /**
   * Tries to receive @p len bytes of data from the I/O device.
   *
   * @param device	the device to receive from
   * @param len		the number of bytes to receive; -1 to read as much
   *                    as possible
   * @returns the number of bytes received and copied into the buffer,
   *	      -1 indicates an error.
   */
  virtual TQ_LONG receiveFrom(KActiveSocketBase* device, TQ_LONG len = -1);

protected:
  mutable TQMutex m_mutex;
  TQValueList<TQByteArray> m_list;
  TQIODevice::Offset m_offset;	///< offset of the start of data in the first element

  TQ_LONG m_size;		///< the maximum length of the buffer
  mutable TQ_LONG m_length;
};

} }			// namespace KNetwork::Internal

#endif
