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

#ifndef KSOCKSSOCKETDEVICE_H
#define KSOCKSSOCKETDEVICE_H

#include "tdesocketdevice.h"

namespace KNetwork {

/** 
 * @class KSocksSocketDevice ksockssocketdevice.h ksockssocketdevice.h
 * @brief The low-level class for SOCKS proxying.
 *
 * This class reimplements several functions from @ref TDESocketDevice in order
 * to implement SOCKS support.
 *
 * This works by using KSocks.
 *
 * @author Thiago Macieira <thiago.macieira@kdemail.net>
 *
 * @warning This code is untested!
 */
class TDECORE_EXPORT KSocksSocketDevice: public TDESocketDevice
{
public:
  /** 
   * Constructor.
   */
  KSocksSocketDevice(const TDESocketBase* = 0L);

  /**
   * Construct from a file descriptor.
   */
  explicit KSocksSocketDevice(int fd);

  /**
   * Destructor.
   */
  virtual ~KSocksSocketDevice();

  /**
   * Sets our capabilities.
   */
  virtual int capabilities() const;

  /**
   * Overrides binding.
   */
  virtual bool bind(const KResolverEntry& address);

  /**
   * Overrides listening.
   */
  virtual bool listen(int backlog);

  /**
   * Overrides connection.
   */
  virtual bool connect(const KResolverEntry& address);

  /**
   * Overrides accepting. The return type is specialised.
   */
  virtual KSocksSocketDevice* accept();

  /**
   * Overrides reading.
   */
  virtual TQ_LONG readBlock(char *data, TQ_ULONG maxlen);
  virtual TQ_LONG readBlock(char *data, TQ_ULONG maxlen, TDESocketAddress& from);

  /**
   * Overrides peeking.
   */
  virtual TQ_LONG peekBlock(char *data, TQ_ULONG maxlen);
  virtual TQ_LONG peekBlock(char *data, TQ_ULONG maxlen, TDESocketAddress& from);

  /**
   * Overrides writing.
   */
  virtual TQ_LONG writeBlock(const char *data, TQ_ULONG len);
  virtual TQ_LONG writeBlock(const char *data, TQ_ULONG len, const TDESocketAddress& to);

  /**
   * Overrides getting socket address.
   */
  virtual TDESocketAddress localAddress() const;

  /**
   * Overrides getting peer address.
   */
  virtual TDESocketAddress peerAddress() const;

  /**
   * Overrides getting external address.
   */
  virtual TDESocketAddress externalAddress() const;

  /**
   * Overrides polling.
   */
  virtual bool poll(bool* input, bool* output, bool* exception = 0L,
		    int timeout = -1, bool* timedout = 0L);

private:
  static void initSocks();
  friend class TDESocketDevice;
};

}				// namespace KNetwork

#endif
