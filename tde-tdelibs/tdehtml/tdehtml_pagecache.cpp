/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
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

#include "tdehtml_pagecache.h"

#include <kstaticdeleter.h>
#include <tdetempfile.h>
#include <kstandarddirs.h>

#include <tqintdict.h>
#include <tqtimer.h>

#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

// We keep 12 pages in memory.
#ifndef TDEHTML_PAGE_CACHE_SIZE
#define TDEHTML_PAGE_CACHE_SIZE 12
#endif

template class TQPtrList<TDEHTMLPageCacheDelivery>;
class TDEHTMLPageCacheEntry
{
  friend class TDEHTMLPageCache;
public:
  TDEHTMLPageCacheEntry(long id);

  ~TDEHTMLPageCacheEntry();

  void addData(const TQByteArray &data);

  void endData();

  bool isComplete()
   { return m_complete; }

  TDEHTMLPageCacheDelivery *fetchData(TQObject *recvObj, const char *recvSlot);
private:
  long m_id;
  bool m_complete;
  TQValueList<TQByteArray> m_data;
  KTempFile *m_file;
};

class TDEHTMLPageCachePrivate
{
public:
  long newId;
  TQIntDict<TDEHTMLPageCacheEntry> dict;
  TQPtrList<TDEHTMLPageCacheDelivery> delivery;
  TQPtrList<TDEHTMLPageCacheEntry> expireQueue;
  bool deliveryActive;
};

TDEHTMLPageCacheEntry::TDEHTMLPageCacheEntry(long id) : m_id(id), m_complete(false)
{
  TQString path = locateLocal("tmp", "tdehtmlcache");
  m_file = new KTempFile(path);
  m_file->unlink();
}

TDEHTMLPageCacheEntry::~TDEHTMLPageCacheEntry()
{
  delete m_file;
}


void
TDEHTMLPageCacheEntry::addData(const TQByteArray &data)
{
  if (m_file->status() == 0)
     m_file->dataStream()->writeRawBytes(data.data(), data.size());
}

void
TDEHTMLPageCacheEntry::endData()
{
  m_complete = true;
  if ( m_file->status() == 0) {
    m_file->dataStream()->device()->flush();
    m_file->dataStream()->device()->at(0);
  }
}


TDEHTMLPageCacheDelivery *
TDEHTMLPageCacheEntry::fetchData(TQObject *recvObj, const char *recvSlot)
{
  // Duplicate fd so that entry can be safely deleted while delivering the data.
  int fd = dup(m_file->handle());
  lseek(fd, 0, SEEK_SET);
  TDEHTMLPageCacheDelivery *delivery = new TDEHTMLPageCacheDelivery(fd);
  recvObj->connect(delivery, TQ_SIGNAL(emitData(const TQByteArray&)), recvSlot);
  delivery->recvObj = recvObj;
  return delivery;
}

static KStaticDeleter<TDEHTMLPageCache> pageCacheDeleter;

TDEHTMLPageCache *TDEHTMLPageCache::_self = 0;

TDEHTMLPageCache *
TDEHTMLPageCache::self()
{
  if (!_self)
     _self = pageCacheDeleter.setObject(_self, new TDEHTMLPageCache);
  return _self;
}

TDEHTMLPageCache::TDEHTMLPageCache()
{
  d = new TDEHTMLPageCachePrivate;
  d->newId = 1;
  d->deliveryActive = false;
}

TDEHTMLPageCache::~TDEHTMLPageCache()
{
  d->delivery.setAutoDelete(true);
  d->dict.setAutoDelete(true);
  delete d;
}

long
TDEHTMLPageCache::createCacheEntry()
{
  TDEHTMLPageCacheEntry *entry = new TDEHTMLPageCacheEntry(d->newId);
  d->dict.insert(d->newId, entry);
  d->expireQueue.append(entry);
  if (d->expireQueue.count() > TDEHTML_PAGE_CACHE_SIZE)
  {
     TDEHTMLPageCacheEntry *entry = d->expireQueue.take(0);
     d->dict.remove(entry->m_id);
     delete entry;
  }
  return (d->newId++);
}

void
TDEHTMLPageCache::addData(long id, const TQByteArray &data)
{
  TDEHTMLPageCacheEntry *entry = d->dict.find(id);
  if (entry)
     entry->addData(data);
}

void
TDEHTMLPageCache::endData(long id)
{
  TDEHTMLPageCacheEntry *entry = d->dict.find(id);
  if (entry)
     entry->endData();
}

void
TDEHTMLPageCache::cancelEntry(long id)
{
  TDEHTMLPageCacheEntry *entry = d->dict.take(id);
  if (entry)
  {
     d->expireQueue.removeRef(entry);
     delete entry;
  }
}

bool
TDEHTMLPageCache::isValid(long id)
{
  return (d->dict.find(id) != 0);
}

bool
TDEHTMLPageCache::isComplete(long id)
{
  TDEHTMLPageCacheEntry *entry = d->dict.find(id);
  if (entry)
     return entry->isComplete();
  return false;
}

void
TDEHTMLPageCache::fetchData(long id, TQObject *recvObj, const char *recvSlot)
{
  TDEHTMLPageCacheEntry *entry = d->dict.find(id);
  if (!entry || !entry->isComplete()) return;

  // Make this entry the most recent entry.
  d->expireQueue.removeRef(entry);
  d->expireQueue.append(entry);

  d->delivery.append( entry->fetchData(recvObj, recvSlot) );
  if (!d->deliveryActive)
  {
     d->deliveryActive = true;
     TQTimer::singleShot(20, this, TQ_SLOT(sendData()));
  }
}

void
TDEHTMLPageCache::cancelFetch(TQObject *recvObj)
{
  TDEHTMLPageCacheDelivery *next;
  for(TDEHTMLPageCacheDelivery* delivery = d->delivery.first();
      delivery;
      delivery = next)
  {
      next = d->delivery.next();
      if (delivery->recvObj == recvObj)
      {
         d->delivery.removeRef(delivery);
         delete delivery;
      }
  }
}

void
TDEHTMLPageCache::sendData()
{
  if (d->delivery.isEmpty())
  {
     d->deliveryActive = false;
     return;
  }
  TDEHTMLPageCacheDelivery *delivery = d->delivery.take(0);
  assert(delivery);

  char buf[8192];
  TQByteArray byteArray;

  int n = read(delivery->fd, buf, 8192);

  if ((n < 0) && (errno == EINTR))
  {
     // try again later
     d->delivery.append( delivery );
  }
  else if (n <= 0)
  {
     // done.
     delivery->emitData(byteArray); // Empty array
     delete delivery;
  }
  else
  {
     byteArray.setRawData(buf, n);
     delivery->emitData(byteArray);
     byteArray.resetRawData(buf, n);
     d->delivery.append( delivery );
  }
  TQTimer::singleShot(0, this, TQ_SLOT(sendData()));
}

void
TDEHTMLPageCache::saveData(long id, TQDataStream *str)
{
  TDEHTMLPageCacheEntry *entry = d->dict.find(id);
  assert(entry);

  int fd = entry->m_file->handle();
  if ( fd < 0 ) return;

  off_t pos = lseek(fd, 0, SEEK_CUR);
  lseek(fd, 0, SEEK_SET);

  char buf[8192];

  while(true)
  {
     int n = read(fd, buf, 8192);
     if ((n < 0) && (errno == EINTR))
     {
        // try again
        continue;
     }
     else if (n <= 0)
     {
        // done.
        break;
     }
     else
     {
        str->writeRawBytes(buf, n);
     }
  }

  if (pos != (off_t)-1)
    lseek(fd, pos, SEEK_SET);
}

TDEHTMLPageCacheDelivery::~TDEHTMLPageCacheDelivery()
{
  close(fd);
}

#include "tdehtml_pagecache.moc"
