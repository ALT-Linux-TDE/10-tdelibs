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

#ifndef KDATAGRAMSOCKET_H
#define KDATAGRAMSOCKET_H

#include <tqcstring.h>

#include "tdesocketaddress.h"
#include "kclientsocketbase.h"

namespace KNetwork {

class KResolverEntry;

/**
 * @class KDatagramPacket kdatagramsocket.h kdatagramsocket.h
 * @brief one datagram
 *
 * This object represents one datagram of data sent or received through
 * a datagram socket (as @ref KDatagramSocket or derived classes). A datagram
 * consists of data as well as a network address associated (whither to send
 * the data or whence it came).
 *
 * This is a lightweight class. Data is stored in a @ref TQByteArray, which means
 * that it is explicitly shared.
 *
 * @author Thiago Macieira <thiago.macieira@kdemail.net>
 */
class TDECORE_EXPORT KDatagramPacket
{
  TQByteArray m_data;
  TDESocketAddress m_address;

public:
  /**
   * Default constructor.
   */
  KDatagramPacket()
  { }

  /**
   * Constructs the datagram with the specified content.
   */
  KDatagramPacket(const TQByteArray& content)
    : m_data(content)
  { }

  /**
   * Constructs the datagram with the specified content.
   *
   * @see setData for information on data sharing.
   */
  KDatagramPacket(const char* content, uint length)
  { setData(content, length); }

  /**
   * Constructs the datagram with the specified content and address.
   */
  KDatagramPacket(const TQByteArray& content, const TDESocketAddress& addr)
    : m_data(content), m_address(addr)
  { }

  /**
   * Constructs the datagram with the specified content and address.
   */
  KDatagramPacket(const char *content, uint length, const TDESocketAddress& addr)
    : m_address(addr)
  { setData(content, length); }

  /**
   * Copy constructor. Note that data is explicitly shared.
   */
  KDatagramPacket(const KDatagramPacket& other)
  { *this = other; }

  /**
   * Destructor. Non-virtual.
   */
  ~KDatagramPacket()
  { }

  /**
   * Returns the data.
   */
  const TQByteArray& data() const
  { return m_data; }

  /**
   * Returns the data length.
   */
  uint length() const
  { return m_data.size(); }

  /**
   * Returns the data length.
   */
  uint size() const
  { return m_data.size(); }

  /**
   * Returns true if this object is empty.
   */
  bool isEmpty() const
  { return m_data.isEmpty(); }

  /**
   * Returns true if this object is null.
   */
  bool isNull() const
  { return m_data.isNull(); }

  /**
   * Returns the socket address
   */
  const TDESocketAddress& address() const
  { return m_address; }

  /**
   * Sets the address stored to the given value.
   */
  void setAddress(const TDESocketAddress& addr)
  { m_address = addr; }

  /**
   * Detaches our data from a shared pool.
   * @see TQByteArray::detach
   */
  void detach()
  { m_data.detach(); }

  /**
   * Sets the data to the given value. Data is explicitly shared.
   */
  void setData(const TQByteArray& data)
  { m_data = data; }

  /**
   * Sets the data to the given buffer and size.
   */
  void setData(const char* data, uint length)
  { m_data.duplicate(data, length); }
};

class KDatagramSocketPrivate;
/**
 * @class KDatagramSocket kdatagramsocket.h kdatagramsocket.h
 * @brief A socket that operates on datagrams.
 *
 * Unlike @ref KStreamSocket, which operates on a connection-based stream
 * socket (generally TCP), this class and its descendants operates on datagrams, 
 * which are normally connectionless.
 *
 * This class in specific provides easy access to the system's connectionless
 * SOCK_DGRAM sockets. 
 *
 * @author Thiago Macieira <thiago.macieira@kdemail.net>
 */
class TDECORE_EXPORT KDatagramSocket: public KClientSocketBase
{
  TQ_OBJECT
  

public:
  /**
   * Default constructor.
   */
  KDatagramSocket(TQObject* parent = 0L, const char *name = 0L);

  /**
   * Destructor. This closes the socket.
   */
  virtual ~KDatagramSocket();

  /**
   * Performs host lookups.
   */
  //  virtual bool lookup();

  /**
   * Binds this socket to the given address. If the socket is blocking,
   * the socket will be bound when this function returns.
   *
   * Note that binding a socket is not necessary to be able to send datagrams.
   * Some protocol families will use anonymous source addresses, while others
   * will allocate an address automatically.
   */
  virtual bool bind(const TQString& node = TQString::null,
		    const TQString& service = TQString::null);

  /**
   * @overload
   * Binds this socket to the given address.
   */
  virtual bool bind(const KResolverEntry& entry)
  { return KClientSocketBase::bind(entry); }

  /**
   * "Connects" this socket to the given address. Note that connecting
   * a datagram socket normally does not establish a permanent connection
   * with the peer nor normally returns an error in case of failure.
   *
   * Connecting means only to designate the given address as the default
   * destination address for datagrams sent without destination addresses
   * ( writeBlock(const char *, TQ_ULONG) ).
   *
   * @note Calling connect will not cause the socket to be bound. You have
   *       to call @ref bind explicitly.
   */
  virtual bool connect(const TQString& node = TQString::null,
		       const TQString& service = TQString::null);

  /**
   * @overload
   * "Connects" this socket to the given address.
   */
  virtual bool connect(const KResolverEntry& entry)
  { return KClientSocketBase::connect(entry); }

  /**
   * Writes data to the socket. Reimplemented from KClientSocketBase.
   */
  virtual TQ_LONG writeBlock(const char *data, TQ_ULONG len, const TDESocketAddress& to);

  /**
   * Receives one datagram from the stream. The reading process is guaranteed
   * to be atomical and not lose data from the packet.
   *
   * If nothing could be read, a null object will be returned.
   */
  virtual KDatagramPacket receive();

  /**
   * Sends one datagram into the stream. The destination address must be
   * set if this socket has not been connected (see @ref connect).
   *   
   * The data in this packet will be sent only in one single datagram. If the
   * system cannot send it like that, this function will fail. So, please take
   * into consideration the datagram size limits.
   *
   * @returns the number of bytes written or -1 in case of error.
   */
  virtual TQ_LONG send(const KDatagramPacket& packet);

private slots:
  void lookupFinishedLocal();
  void lookupFinishedPeer();

private:
  bool doBind();
  void setupSignals();

  KDatagramSocketPrivate *d;
};

}				// namespace KNetwork

#endif
