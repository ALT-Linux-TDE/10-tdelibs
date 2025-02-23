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

#include "config.h"

// System includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>

// Qt
#include <tqevent.h>
#include <tqmutex.h>
#include <tqapplication.h>

// Us
#include "kreverseresolver.h"
#include "kresolver_p.h"
#include "kresolverworkerbase.h"
#include "tdesocketaddress.h"

#ifndef HAVE_GETNAMEINFO
// FIXME KDE4:
// move to syssocket or adapt
# include "netsupp.h"
#endif

using namespace KNetwork;
using namespace KNetwork::Internal;

namespace
{
  class ReverseThread: public KResolverWorkerBase
  {
  public:
    ReverseThread(const TDESocketAddress& addr, int flags)
      : m_addr(addr), m_flags(flags), m_parent(0L)
    { }

    virtual ~ReverseThread()
    { }

    virtual bool preprocess()
    { return true; }
    virtual bool run();
    virtual bool postprocess();

    // input:
    TDESocketAddress m_addr;
    int m_flags;
    KReverseResolver *m_parent;

    // output:
    TQString node;
    TQString service;
    bool success;
  };

  class KReverseResolverEvent: public TQEvent
  {
  public:
    static const int myType = TQEvent::User + 63; // arbitrary value
    TQString node;
    TQString service;
    bool success;

    KReverseResolverEvent(const TQString& _node, const TQString& _service,
			  bool _success)
      : TQEvent((Type)myType), node(_node),
	service(_service), success(_success)
    { }
  };
}

class KNetwork::KReverseResolverPrivate
{
public:
  TQString node;
  TQString service;
  TDESocketAddress addr;
  int flags;

  ReverseThread* worker;
  bool success;

  inline KReverseResolverPrivate(const TDESocketAddress& _addr)
    : addr(_addr), worker(0L), success(false)
  { }
};

KReverseResolver::KReverseResolver(const TDESocketAddress& addr, int flags,
				   TQObject *parent, const char* name)
  : TQObject(parent, name), d(new KReverseResolverPrivate(addr))
{
  d->flags = flags;
}

KReverseResolver::~KReverseResolver()
{
  if (d->worker)
    d->worker->m_parent = 0L;
}

bool KReverseResolver::isRunning() const
{
  return d->worker != 0L;
}

bool KReverseResolver::success() const
{
  return !isRunning() && d->success;
}

bool KReverseResolver::failure() const
{
  return !isRunning() && !d->success;
}

TQString KReverseResolver::node() const
{
  return d->node;
}

TQString KReverseResolver::service() const
{
  return d->service;
}

const TDESocketAddress& KReverseResolver::address() const
{
  return d->addr;
}

bool KReverseResolver::start()
{
  if (d->worker != 0L)
    return true;		// already started

  d->worker = new ReverseThread(d->addr, d->flags);
  d->worker->m_parent = this;

  RequestData *req = new RequestData;
  req->obj = 0L;
  req->input = 0L;
  req->requestor = 0L;
  req->worker = d->worker;
  KResolverManager::manager()->dispatch(req);
  return true;
}

bool KReverseResolver::event(TQEvent *e)
{
  if (e->type() != KReverseResolverEvent::myType)
    return TQObject::event(e);	// call parent

  KReverseResolverEvent *re = static_cast<KReverseResolverEvent*>(e);
  d->node = re->node;
  d->service = re->service;
  d->success = re->success;

  // don't delete d->worker!
  // KResolverManager::doNotifying takes care of that, if it hasn't already
  d->worker = 0L;

  // emit signal
  emit finished(*this);

  return true;
}

bool KReverseResolver::resolve(const TDESocketAddress& addr, TQString& node,
			       TQString& serv, int flags)
{
  ReverseThread th(addr, flags);
  if (th.run())
    {
      node = th.node;
      serv = th.service;
      return true;
    }
  return false;
}

bool KReverseResolver::resolve(const struct sockaddr* sa, TQ_UINT16 salen,
			       TQString& node, TQString& serv, int flags)
{
  return resolve(TDESocketAddress(sa, salen), node, serv, flags);
}

bool ReverseThread::run()
{
  int err;
  char h[NI_MAXHOST], s[NI_MAXSERV];
  int niflags = 0;

  h[0] = s[0] = '\0';

  if (m_flags & KReverseResolver::NumericHost)
    niflags |= NI_NUMERICHOST;
  if (m_flags & KReverseResolver::NumericService)
    niflags |= NI_NUMERICSERV;
  if (m_flags & KReverseResolver::NodeNameOnly)
    niflags |= NI_NOFQDN;
  if (m_flags & KReverseResolver::Datagram)
    niflags |= NI_DGRAM;
  if (m_flags & KReverseResolver::ResolutionRequired)
    niflags |= NI_NAMEREQD;

  {
#ifdef NEED_MUTEX
    TQMutexLocker locker(&::getXXbyYYmutex);
#endif
    err = ::getnameinfo(m_addr, m_addr.length(),
			h, sizeof(h) - 1, s, sizeof(s) - 1, niflags);
  }

  if (err == 0)
    {
      node = KResolver::domainToUnicode(TQString::fromLatin1(h));
      service = TQString::fromLatin1(s);
      success = true;
    }
  else
    {
      node = service = TQString::null;
      success = false;
    }

  return success;
}

bool ReverseThread::postprocess()
{
  // post an event
  if (m_parent)
    TQApplication::postEvent(m_parent,
			    new KReverseResolverEvent(node, service, success));
  return true;
}

#include "kreverseresolver.moc"
