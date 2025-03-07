/* This file is part of the KDE project
 *
 * Copyright (C) 2000  Waldo Bastian <bastian@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef __tdehtml_pagecache_h__
#define __tdehtml_pagecache_h__

#include <tqobject.h>
#include <tqcstring.h>
#include <tqvaluelist.h>
#include <tqptrlist.h>

class TDEHTMLPageCachePrivate;

/**
 * Singleton Object that handles a binary cache on top of
 * the http cache management of tdeio.
 *
 * A limited number of HTML pages are stored in this cache. This
 * cache is used for the history and operations like "view source".
 * These operations always want to use the original document and 
 * don't want to fetch the data from the network again.
 *
 * It operates completely independent from the tdeio_http cache.
 */
class TDEHTMLPageCache : public TQObject
{
  TQ_OBJECT
public:
  /**
   * static "constructor".
   * @return returns a pointer to the cache, if it exists.
   * creates a new cache otherwise.
   */
  static TDEHTMLPageCache *self();
  ~TDEHTMLPageCache();
  
  /**
   * Create a new cache entry. 
   *
   * @return a cache entry ID is returned.
   */
  long createCacheEntry();

  /**
   * Add @p data to the cache entry with id @p id.
   */
  void addData(long id, const TQByteArray &data);

  /**
   * Signal end of data for the cache entry with id @p id.
   * After calling this the entry is marked complete 
   */
  void endData(long id);

  /**
   * Cancel the entry.
   */
  void cancelEntry(long id);

  /**
   * @return true when the cache entry with id @p is still valid,
   * and at least some of the data is available for reading (the
   * complete data may not yet be loaded)
   */
  bool isValid(long id);

  /**
   * @return true when the cache entry with id @p is still valid,
   * and the complete data is available for reading
   */
  bool isComplete(long id);
  
  /**
   * Fetch data for cache entry @p id and send it to slot @p recvSlot
   * in the object @p recvObj
   */
  void fetchData(long id, TQObject *recvObj, const char *recvSlot);

  /**
   * Cancel sending data to @p recvObj
   */
  void cancelFetch(TQObject *recvObj);

  /**
   * Save the data of cache entry @p id to the datastream @p str
   */
  void saveData(long id, TQDataStream *str);

private slots:
  void sendData();

private:  
  TDEHTMLPageCache();

  static TDEHTMLPageCache *_self;

  TDEHTMLPageCachePrivate *d;  
};

class TDEHTMLPageCacheDelivery : public TQObject
{
   friend class TDEHTMLPageCache;
TQ_OBJECT
public:
   TDEHTMLPageCacheDelivery(int _fd)
    : fd(_fd) { }
   ~TDEHTMLPageCacheDelivery();

signals:
   void emitData(const TQByteArray &data);

public: 
   TQObject *recvObj;
   int fd;      
};


#endif
