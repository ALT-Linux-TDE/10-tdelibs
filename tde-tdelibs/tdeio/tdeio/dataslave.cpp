/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2003 Leo Savernik <l.savernik@aon.at>
 *  Derived from slave.cpp
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
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
 **/

#include <config.h>

#include "dataslave.h"
#include "dataprotocol.h"

#include <tdelocale.h>
#include <kdebug.h>

#include <tqtimer.h>

using namespace TDEIO;

#define TDEIO_DATA_POLL_INTERVAL 0

// don't forget to sync DISPATCH_DECL in dataslave.h
#define DISPATCH_IMPL(type) \
	void DataSlave::dispatch_##type() { \
	  if (_suspended) { \
	    QueueStruct q(Queue_##type); \
	    dispatchQueue.push_back(q); \
	    if (!timer->isActive()) timer->start(TDEIO_DATA_POLL_INTERVAL); \
	  } else \
	    type(); \
	}

// don't forget to sync DISPATCH_DECL1 in dataslave.h
#define DISPATCH_IMPL1(type, paramtype, paramname) \
	void DataSlave::dispatch_##type(paramtype paramname) { \
	  if (_suspended) { \
	    QueueStruct q(Queue_##type); \
	    q.paramname = paramname; \
	    dispatchQueue.push_back(q); \
	    if (!timer->isActive()) timer->start(TDEIO_DATA_POLL_INTERVAL); \
	  } else \
	    type(paramname); \
	}


DataSlave::DataSlave() :
	Slave(true, 0, "data", TQString::null)
{
  //kdDebug() << this << k_funcinfo << endl;
  _suspended = false;
  timer = new TQTimer(this);
  connect(timer, TQ_SIGNAL(timeout()), TQ_SLOT(dispatchNext()));
}

DataSlave::~DataSlave() {
  //kdDebug() << this << k_funcinfo << endl;
}

void DataSlave::hold(const KURL &/*url*/) {
  // ignored
}

void DataSlave::suspend() {
  _suspended = true;
  //kdDebug() << this << k_funcinfo << endl;
  timer->stop();
}

void DataSlave::resume() {
  _suspended = false;
  //kdDebug() << this << k_funcinfo << endl;
  // aarrrgh! This makes the once hyper fast and efficient data protocol
  // implementation slow as molasses. But it wouldn't work otherwise,
  // and I don't want to start messing around with threads
  timer->start(TDEIO_DATA_POLL_INTERVAL);
}

// finished is a special case. If we emit it right away, then
// TransferJob::start can delete the job even before the end of the method
void DataSlave::dispatch_finished() {
    QueueStruct q(Queue_finished);
    dispatchQueue.push_back(q);
    if (!timer->isActive()) timer->start(TDEIO_DATA_POLL_INTERVAL);
}

void DataSlave::dispatchNext() {
  if (dispatchQueue.empty()) {
    timer->stop();
    return;
  }

  const QueueStruct &q = dispatchQueue.front();
  //kdDebug() << this << k_funcinfo << "dispatching " << q.type << " " << dispatchQueue.size() << " left" << endl;
  switch (q.type) {
    case Queue_mimeType:	mimeType(q.s); break;
    case Queue_totalSize:	totalSize(q.size); break;
    case Queue_sendMetaData:	sendMetaData(); break;
    case Queue_data:		data(q.ba); break;
    case Queue_finished:	finished(); break;
  }/*end switch*/

  dispatchQueue.pop_front();
}

void DataSlave::send(int cmd, const TQByteArray &arr) {
  TQDataStream stream(arr, IO_ReadOnly);

  KURL url;

  switch (cmd) {
    case CMD_GET: {
      stream >> url;
      get(url);
      break;
    }
    case CMD_MIMETYPE: {
      stream >> url;
      mimetype(url);
      break;
    }
    // ignore these (must not emit error, otherwise SIGSEGV occurs)
    case CMD_META_DATA:
    case CMD_SUBURL:
      break;
    default:
      error(ERR_UNSUPPORTED_ACTION,
		unsupportedActionErrorString(TQString::fromLatin1("data"),cmd));
  }/*end switch*/
}

bool DataSlave::suspended() {
  return _suspended;
}

void DataSlave::setHost(const TQString &/*host*/, int /*port*/,
                     const TQString &/*user*/, const TQString &/*passwd*/) {
  // irrelevant -> will be ignored
}

void DataSlave::setConfig(const MetaData &/*config*/) {
  // FIXME: decide to handle this directly or not at all
#if 0
    TQByteArray data;
    TQDataStream stream( data, IO_WriteOnly );
    stream << config;
    slaveconn.send( CMD_CONFIG, data );
#endif
}

void DataSlave::setAllMetaData(const MetaData &md) {
  meta_data = md;
}

void DataSlave::sendMetaData() {
  emit metaData(meta_data);
}

void DataSlave::virtual_hook( int id, void* data ) {
  switch (id) {
    case VIRTUAL_SUSPEND: suspend(); return;
    case VIRTUAL_RESUME: resume(); return;
    case VIRTUAL_SEND: {
      SendParams *params = reinterpret_cast<SendParams *>(data);
      send(params->cmd, *params->arr);
      return;
    }
    case VIRTUAL_HOLD: {
      HoldParams *params = reinterpret_cast<HoldParams *>(data);
      hold(*params->url);
      return;
    }
    case VIRTUAL_SUSPENDED: {
      SuspendedParams *params = reinterpret_cast<SuspendedParams *>(data);
      params->retval = suspended();
      return;
    }
    case VIRTUAL_SET_HOST: {
      SetHostParams *params = reinterpret_cast<SetHostParams *>(data);
      setHost(*params->host,params->port,*params->user,*params->passwd);
      return;
    }
    case VIRTUAL_SET_CONFIG: {
      SetConfigParams *params = reinterpret_cast<SetConfigParams *>(data);
      setConfig(*params->config);
      return;
    }
    default:
      TDEIO::Slave::virtual_hook( id, data );
  }
}

DISPATCH_IMPL1(mimeType, const TQString &, s)
DISPATCH_IMPL1(totalSize, TDEIO::filesize_t, size)
DISPATCH_IMPL(sendMetaData)
DISPATCH_IMPL1(data, const TQByteArray &, ba)

#undef DISPATCH_IMPL
#undef DISPATCH_IMPL1

#include "dataslave.moc"
