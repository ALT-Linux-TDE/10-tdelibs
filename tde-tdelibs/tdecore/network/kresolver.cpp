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

#include "config.h"

// System includes
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>

// Qt includes
#include <tqapplication.h>
#include <tqstring.h>
#include <tqcstring.h>
#include <tqstrlist.h>
#include <tqstringlist.h>
#include <tqshared.h>
#include <tqdatetime.h>
#include <tqtimer.h>
#include <tqmutex.h>
#include <tqguardedptr.h>

// IDN
#ifdef HAVE_IDNA_H
# include <idna.h>
#endif

// KDE
#include <tdelocale.h>

// Us
#include "kresolver.h"
#include "kresolver_p.h"
#include "tdesocketaddress.h"

#ifdef NEED_MUTEX
#warning "mutex"
TQMutex getXXbyYYmutex;
#endif

#ifdef __OpenBSD__
#define USE_OPENBSD 1
#endif

using namespace KNetwork;
using namespace KNetwork::Internal;

/////////////////////////////////////////////
// class KResolverEntry

class KNetwork::KResolverEntryPrivate: public TQShared
{
public:
  TDESocketAddress addr;
  int socktype;
  int protocol;
  TQString canonName;
  TQCString encodedName;

  inline KResolverEntryPrivate() :
    socktype(0), protocol(0)
  { }
};

// default constructor
KResolverEntry::KResolverEntry() :
  d(0L)
{
}

// constructor with stuff
KResolverEntry::KResolverEntry(const TDESocketAddress& addr, int socktype, int protocol,
			       const TQString& canonName, const TQCString& encodedName) :
  d(new KResolverEntryPrivate)
{
  d->addr = addr;
  d->socktype = socktype;
  d->protocol = protocol;
  d->canonName = canonName;
  d->encodedName = encodedName;
}

// constructor with even more stuff
KResolverEntry::KResolverEntry(const struct sockaddr* sa, TQ_UINT16 salen, int socktype,
			       int protocol, const TQString& canonName,
			       const TQCString& encodedName) :
  d(new KResolverEntryPrivate)
{
  d->addr = TDESocketAddress(sa, salen);
  d->socktype = socktype;
  d->protocol = protocol;
  d->canonName = canonName;
  d->encodedName = encodedName;
}

// copy constructor
KResolverEntry::KResolverEntry(const KResolverEntry& that) :
  d(0L)
{
  *this = that;
}

// destructor
KResolverEntry::~KResolverEntry()
{
  if (d == 0L)
    return;

  if (d->deref())
    delete d;
}

// returns the socket address
TDESocketAddress KResolverEntry::address() const
{
  return d ? d->addr : TDESocketAddress();
}

// returns the length
TQ_UINT16 KResolverEntry::length() const
{
  return d ? d->addr.length() : 0;
}

// returns the family
int KResolverEntry::family() const
{
  return d ? d->addr.family() : AF_UNSPEC;
}

// returns the canonical name
TQString KResolverEntry::canonicalName() const
{
  return d ? d->canonName : TQString::null;
}

// returns the encoded name
TQCString KResolverEntry::encodedName() const
{
  return d ? d->encodedName : TQCString();
}

// returns the socket type
int KResolverEntry::socketType() const
{
  return d ? d->socktype : 0;
}

// returns the protocol
int KResolverEntry::protocol() const
{
  return d ? d->protocol : 0;
}

// assignment operator
KResolverEntry& KResolverEntry::operator= (const KResolverEntry& that)
{
  // copy the data
  if (that.d)
    that.d->ref();

  if (d && d->deref())
    delete d;

  d = that.d;
  return *this;
}

/////////////////////////////////////////////
// class KResolverResults

class KNetwork::KResolverResultsPrivate
{
public:
  TQString node, service;
  int errorcode, syserror;

  KResolverResultsPrivate() :
    errorcode(0), syserror(0)
  { }
};

// default constructor
KResolverResults::KResolverResults()
  : d(new KResolverResultsPrivate)
{
}

// copy constructor
KResolverResults::KResolverResults(const KResolverResults& other)
  : TQValueList<KResolverEntry>(other), d(new KResolverResultsPrivate)
{
  *d = *other.d;
}

// destructor
KResolverResults::~KResolverResults()
{
  delete d;
}

// assignment operator
KResolverResults&
KResolverResults::operator= (const KResolverResults& other)
{
  if (this == &other)
    return *this;

  // copy over the other data
  *d = *other.d;

  // now let TQValueList do the rest of the work
  TQValueList<KResolverEntry>::operator =(other);

  return *this;
}

// gets the error code
int KResolverResults::error() const
{
  return d->errorcode;
}

// gets the system errno
int KResolverResults::systemError() const
{
  return d->syserror;
}

// sets the error codes
void KResolverResults::setError(int errorcode, int systemerror)
{
  d->errorcode = errorcode;
  d->syserror = systemerror;
}

// gets the hostname
TQString KResolverResults::nodeName() const
{
  return d->node;
}

// gets the service name
TQString KResolverResults::serviceName() const
{
  return d->service;
}

// sets the address
void KResolverResults::setAddress(const TQString& node,
				  const TQString& service)
{
  d->node = node;
  d->service = service;
}

void KResolverResults::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


///////////////////////
// class KResolver

TQStringList *KResolver::idnDomains = 0;


// default constructor
KResolver::KResolver(TQObject *parent, const char *name)
  : TQObject(parent, name), d(new KResolverPrivate(this))
{
}

// constructor with host and service
KResolver::KResolver(const TQString& nodename, const TQString& servicename,
		   TQObject *parent, const char *name)
  : TQObject(parent, name), d(new KResolverPrivate(this, nodename, servicename))
{
}

// destructor
KResolver::~KResolver()
{
  cancel(false);
  delete d;
}

// get the status
int KResolver::status() const
{
  return d->status;
}

// get the error code
int KResolver::error() const
{
  return d->errorcode;
}

// get the errno
int KResolver::systemError() const
{
  return d->syserror;
}

// are we running?
bool KResolver::isRunning() const
{
  return d->status > 0 && d->status < Success;
}

// get the hostname
TQString KResolver::nodeName() const
{
  return d->input.node;
}

// get the service
TQString KResolver::serviceName() const
{
  return d->input.service;
}

// sets the hostname
void KResolver::setNodeName(const TQString& nodename)
{
  // don't touch those values if we're working!
  if (!isRunning())
    {
      d->input.node = nodename;
      d->status = Idle;
      d->results.setAddress(nodename, d->input.service);
    }
}

// sets the service
void KResolver::setServiceName(const TQString& service)
{
  // don't change if running
  if (!isRunning())
    {
      d->input.service = service;
      d->status = Idle;
      d->results.setAddress(d->input.node, service);
    }
}

// sets the address
void KResolver::setAddress(const TQString& nodename, const TQString& service)
{
  setNodeName(nodename);
  setServiceName(service);
}

// get the flags
int KResolver::flags() const
{
  return d->input.flags;
}

// sets the flags
int KResolver::setFlags(int flags)
{
  int oldflags = d->input.flags;
  if (!isRunning())
    {
      d->input.flags = flags;
      d->status = Idle;
    }
  return oldflags;
}

// sets the family mask
void KResolver::setFamily(int families)
{
  if (!isRunning())
    {
      d->input.familyMask = families;
      d->status = Idle;
    }
}

// sets the socket type
void KResolver::setSocketType(int type)
{
  if (!isRunning())
    {
      d->input.socktype = type;
      d->status = Idle;
    }
}

// sets the protocol
void KResolver::setProtocol(int protonum, const char *name)
{
  if (isRunning())
    return;			// can't change now

  // we copy the given protocol name. If it isn't an empty string
  // and the protocol number was 0, we will look it up in /etc/protocols
  // we also leave the error reporting to the actual lookup routines, in
  // case the given protocol name doesn't exist

  d->input.protocolName = name;
  if (protonum == 0 && name != 0L && *name != '\0')
    {
      // must look up the protocol number
      d->input.protocol = KResolver::protocolNumber(name);
    }
  else
    d->input.protocol = protonum;
  d->status = Idle;
}

bool KResolver::start()
{
  if (!isRunning())
    {
      d->results.empty();

      // is there anything to be queued?
      if (d->input.node.isEmpty() && d->input.service.isEmpty())
	{
	  d->status = KResolver::Success;
	  emitFinished();
	}
      else
	KResolverManager::manager()->enqueue(this, 0L);
    }

  return true;
}

bool KResolver::wait(int msec)
{
  if (!isRunning())
    {
      emitFinished();
      return true;
    }

  TQMutexLocker locker(&d->mutex);

  if (!isRunning())
    {
      // it was running and no longer is?
      // That means the manager has finished its processing and has posted
      // an event for the signal to be emitted already. This means the signal
      // will be emitted twice!

      emitFinished();
      return true;
    }
  else
    {
      TQTime t;
      t.start();

      while (!msec || t.elapsed() < msec)
	{
	  // wait on the manager to broadcast completion
	  d->waiting = true;
	  if (msec)
	    KResolverManager::manager()->notifyWaiters.wait(&d->mutex, msec - t.elapsed());
	  else
	    KResolverManager::manager()->notifyWaiters.wait(&d->mutex);

	  // the manager has processed
	  // see if this object is done
	  if (!isRunning())
	    {
	      // it's done
	      d->waiting = false;
	      emitFinished();
	      return true;
	    }
	}

      // if we've got here, we've timed out
      d->waiting = false;
      return false;
    }
}

void KResolver::cancel(bool emitSignal)
{
  KResolverManager::manager()->dequeue(this);
  if (emitSignal)
    emitFinished();
}

KResolverResults
KResolver::results() const
{
  if (!isRunning())
    return d->results;

  // return a dummy, empty result
  KResolverResults r;
  r.setAddress(d->input.node, d->input.service);
  r.setError(d->errorcode, d->syserror);
  return r;
}

bool KResolver::event(TQEvent* e)
{
  if (static_cast<int>(e->type()) == KResolverManager::ResolutionCompleted)
    {
      emitFinished();
      return true;
    }

  return false;
}

void KResolver::emitFinished()
{
  if (isRunning())
    d->status = KResolver::Success;

  TQGuardedPtr<TQObject> p = this; // guard against deletion

  emit finished(d->results);

  if (p && d->deleteWhenDone)
    deleteLater();		// in TQObject
}

TQString KResolver::errorString(int errorcode, int syserror)
{
  // no i18n now...
  static const char * const messages[] =
  {
    I18N_NOOP("no error"),	// NoError
    I18N_NOOP("requested family not supported for this host name"), // AddrFamily
    I18N_NOOP("temporary failure in name resolution"),	// TryAgain
    I18N_NOOP("non-recoverable failure in name resolution"), // NonRecoverable
    I18N_NOOP("invalid flags"),			// BadFlags
    I18N_NOOP("memory allocation failure"),	// Memory
    I18N_NOOP("name or service not known"),	// NoName
    I18N_NOOP("requested family not supported"),	// UnsupportedFamily
    I18N_NOOP("requested service not supported for this socket type"), // UnsupportedService
    I18N_NOOP("requested socket type not supported"),	// UnsupportedSocketType
    I18N_NOOP("unknown error"),			// UnknownError
    I18N_NOOP2("1: the i18n'ed system error code, from errno",
	      "system error: %1")		// SystemError
  };

  // handle the special value
  if (errorcode == Canceled)
    return i18n("request was canceled");

  if (errorcode > 0 || errorcode < SystemError)
    return TQString::null;

  TQString msg = i18n(messages[-errorcode]);
  if (errorcode == SystemError)
    msg.arg(TQString::fromLocal8Bit(strerror(syserror)));

  return msg;
}

KResolverResults
KResolver::resolve(const TQString& host, const TQString& service, int flags,
		  int families)
{
  KResolver qres(host, service, tqApp, "synchronous KResolver");
  qres.setFlags(flags);
  qres.setFamily(families);
  qres.start();
  qres.wait();
  return qres.results();
}

bool KResolver::resolveAsync(TQObject* userObj, const char *userSlot,
			     const TQString& host, const TQString& service,
			     int flags, int families)
{
  KResolver* qres = new KResolver(host, service, tqApp, "asynchronous KResolver");
  TQObject::connect(qres, TQ_SIGNAL(finished(KResolverResults)), userObj, userSlot);
  qres->setFlags(flags);
  qres->setFamily(families);
  qres->d->deleteWhenDone = true; // this is the only difference from the example code
  return qres->start();
}

TQStrList KResolver::protocolName(int protonum)
{
  struct protoent *pe = 0L;
#ifndef HAVE_GETPROTOBYNAME_R
  TQMutexLocker locker(&getXXbyYYmutex);

  pe = getprotobynumber(protonum);

#else
# ifdef USE_OPENBSD // OpenBSD uses an HP/IBM/DEC API
  struct protoent protobuf;
  struct protoent_data pdata;
  ::memset(&pdata, 0, sizeof pdata);

  if (getprotobynumber_r(protonum, &protobuf, &pdata) == 0)
    pe = &protobuf;
  else
    pe = 0;

# else
  size_t buflen = 1024;
  struct protoent protobuf;
  char *buf;
  do
    {
      buf = new char[buflen];
#  ifdef USE_SOLARIS // Solaris uses a 4 argument getprotobynumber_r which returns struct *protoent or NULL
      if ((pe = getprotobynumber_r(protonum, &protobuf, buf, buflen)) && (errno == ERANGE))
#  else
      if (getprotobynumber_r(protonum, &protobuf, buf, buflen, &pe) == ERANGE)
#  endif
	{
          pe = 0L;
	  buflen += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (pe == 0L);
# endif
#endif

  // Do common processing
  TQStrList lst(true);	// use deep copies
  if (pe != NULL)
    {
      lst.append(pe->p_name);
      for (char **p = pe->p_aliases; *p; p++)
	lst.append(*p);
    }

#ifdef HAVE_GETPROTOBYNAME_R
# ifndef USE_OPENBSD
  delete [] buf;
# endif
#endif

  return lst;
}

TQStrList KResolver::protocolName(const char *protoname)
{
  struct protoent *pe = 0L;
#ifndef HAVE_GETPROTOBYNAME_R
  TQMutexLocker locker(&getXXbyYYmutex);

  pe = getprotobyname(protoname);

#else
# ifdef USE_OPENBSD // OpenBSD uses an HP/IBM/DEC API
  struct protoent protobuf;
  struct protoent_data pdata;
  ::memset(&pdata, 0, sizeof pdata);

  if (getprotobyname_r(protoname, &protobuf, &pdata) == 0)
    pe = &protobuf;
  else
    pe = 0;

# else
  size_t buflen = 1024;
  struct protoent protobuf;
  char *buf;
  do
    {
      buf = new char[buflen];
#  ifdef USE_SOLARIS // Solaris uses a 4 argument getprotobyname_r which returns struct *protoent or NULL
      if ((pe = getprotobyname_r(protoname, &protobuf, buf, buflen)) && (errno == ERANGE))
#  else
      if (getprotobyname_r(protoname, &protobuf, buf, buflen, &pe) == ERANGE)
#  endif
	{
          pe = 0L;
	  buflen += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (pe == 0L);
# endif
#endif

  // Do common processing
  TQStrList lst(true);	// use deep copies
  if (pe != NULL)
    {
      lst.append(pe->p_name);
      for (char **p = pe->p_aliases; *p; p++)
	lst.append(*p);
    }

#ifdef HAVE_GETPROTOBYNAME_R
# ifndef USE_OPENBSD
  delete [] buf;
# endif
#endif

  return lst;
}

int KResolver::protocolNumber(const char *protoname)
{
  struct protoent *pe = 0L;
#ifndef HAVE_GETPROTOBYNAME_R
  TQMutexLocker locker(&getXXbyYYmutex);

  pe = getprotobyname(protoname);

#else
# ifdef USE_OPENBSD // OpenBSD uses an HP/IBM/DEC API
  struct protoent protobuf;
  struct protoent_data pdata;
  ::memset(&pdata, 0, sizeof pdata);

  if (getprotobyname_r(protoname, &protobuf, &pdata) == 0)
    pe = &protobuf;
  else
    pe = 0;

# else
  size_t buflen = 1024;
  struct protoent protobuf;
  char *buf;
  do
    {
      buf = new char[buflen];
#  ifdef USE_SOLARIS // Solaris uses a 4 argument getprotobyname_r which returns struct *protoent or NULL
      if ((pe = getprotobyname_r(protoname, &protobuf, buf, buflen)) && (errno == ERANGE))
#  else
      if (getprotobyname_r(protoname, &protobuf, buf, buflen, &pe) == ERANGE)
#  endif
	{
          pe = 0L;
	  buflen += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (pe == 0L);
# endif
#endif

  // Do common processing
  int protonum = -1;
  if (pe != NULL)
    protonum = pe->p_proto;

#ifdef HAVE_GETPROTOBYNAME_R
# ifndef USE_OPENBSD
  delete [] buf;
# endif
#endif

  return protonum;
}

int KResolver::servicePort(const char *servname, const char *protoname)
{
  struct servent *se = 0L;
#ifndef HAVE_GETSERVBYNAME_R
  TQMutexLocker locker(&getXXbyYYmutex);

  se = getservbyname(servname, protoname);

#else
# ifdef USE_OPENBSD // OpenBSD uses an HP/IBM/DEC API
  struct servent servbuf;
  struct servent_data sdata;
  ::memset(&sdata, 0, sizeof sdata);
  if (getservbyname_r(servname, protoname, &servbuf, &sdata) == 0)
    se = &servbuf;
  else
    se = 0;

# else
  size_t buflen = 1024;
  struct servent servbuf;
  char *buf;
  do
    {
      buf = new char[buflen];
#  ifdef USE_SOLARIS // Solaris uses a 5 argument getservbyname_r which returns struct *servent or NULL
      if ((se = getservbyname_r(servname, protoname, &servbuf, buf, buflen)) && (errno == ERANGE))
#  else
      if (getservbyname_r(servname, protoname, &servbuf, buf, buflen, &se) == ERANGE)
#  endif
	{
          se = 0L;
	  buflen += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (se == 0L);
# endif
#endif

  // Do common processing
  int servport = -1;
  if (se != NULL)
    servport = ntohs(se->s_port);

#ifdef HAVE_GETSERVBYNAME_R
# ifndef USE_OPENBSD
  delete [] buf;
# endif
#endif

  return servport;
}

TQStrList KResolver::serviceName(const char* servname, const char *protoname)
{
  struct servent *se = 0L;
#ifndef HAVE_GETSERVBYNAME_R
  TQMutexLocker locker(&getXXbyYYmutex);

  se = getservbyname(servname, protoname);

#else
# ifdef USE_OPENBSD // OpenBSD uses an HP/IBM/DEC API
  struct servent servbuf;
  struct servent_data sdata;
  ::memset(&sdata, 0, sizeof sdata);
  if (getservbyname_r(servname, protoname, &servbuf, &sdata) == 0)
    se = &servbuf;
  else
    se = 0;

# else
  size_t buflen = 1024;
  struct servent servbuf;
  char *buf;
  do
    {
      buf = new char[buflen];
#  ifdef USE_SOLARIS // Solaris uses a 5 argument getservbyname_r which returns struct *servent or NULL
      if ((se = getservbyname_r(servname, protoname, &servbuf, buf, buflen)) && (errno == ERANGE))
#  else
      if (getservbyname_r(servname, protoname, &servbuf, buf, buflen, &se) == ERANGE)
#  endif
	{
          se = 0L;
	  buflen += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (se == 0L);
# endif
#endif

  // Do common processing
  TQStrList lst(true);	// use deep copies
  if (se != NULL)
    {
      lst.append(se->s_name);
      for (char **p = se->s_aliases; *p; p++)
	lst.append(*p);
    }

#ifdef HAVE_GETSERVBYNAME_R
# ifndef USE_OPENBSD
  delete [] buf;
# endif
#endif

  return lst;
}

TQStrList KResolver::serviceName(int port, const char *protoname)
{
  struct servent *se = 0L;
#ifndef HAVE_GETSERVBYPORT_R
  TQMutexLocker locker(&getXXbyYYmutex);

  se = getservbyport(port, protoname);

#else
# ifdef USE_OPENBSD // OpenBSD uses an HP/IBM/DEC API
  struct servent servbuf;
  struct servent_data sdata;
  ::memset(&sdata, 0, sizeof sdata);
  if (getservbyport_r(port, protoname, &servbuf, &sdata) == 0)
    se = &servbuf;
  else
    se = 0;

# else
  size_t buflen = 1024;
  struct servent servbuf;
  char *buf;
  do
    {
      buf = new char[buflen];
#  ifdef USE_SOLARIS // Solaris uses a 5 argument getservbyport_r which returns struct *servent or NULL
      if ((se = getservbyport_r(port, protoname, &servbuf, buf, buflen)) && (errno == ERANGE))
#  else
      if (getservbyport_r(port, protoname, &servbuf, buf, buflen, &se) == ERANGE)
#  endif
	{
          se = 0L;
	  buflen += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (se == 0L);
# endif
#endif

  // Do common processing
  TQStrList lst(true);	// use deep copies
  if (se != NULL)
    {
      lst.append(se->s_name);
      for (char **p = se->s_aliases; *p; p++)
	lst.append(*p);
    }

#ifdef HAVE_GETSERVBYPORT_R
# ifndef USE_OPENBSD
  delete [] buf;
# endif
#endif

  return lst;
}

TQString KResolver::localHostName()
{
  TQCString name;
  int len;

#ifdef MAXHOSTNAMELEN
  len = MAXHOSTNAMELEN;
#else
  len = 256;
#endif

  while (true)
    {
      name.resize(len);

      if (gethostname(name.data(), len - 1) == 0)
	{
	  // Call succeeded, but it's not guaranteed to be NUL-terminated
	  // Note that some systems return success even if they did truncation
	  name[len - 1] = '\0';
	  break;
	}

      // Call failed
      if (errno == ENAMETOOLONG || errno == EINVAL)
	len += 256;
      else
	{
	  // Oops! Unknown error!
	  name = TQCString();
	}
    }

  if (name.isEmpty())
    return TQString::fromLatin1("localhost");

  if (name.find('.') == -1)
    {
      // not fully qualified
      // must resolve
      KResolverResults results = resolve(name, "0", CanonName);
      if (results.isEmpty())
	// cannot find a valid hostname!
	return TQString::fromLatin1("localhost");
      else
	return results.first().canonicalName();
    }

  return domainToUnicode(name);
}


// forward declaration
static TQStringList splitLabels(const TQString& unicodeDomain);
static TQCString ToASCII(const TQString& label);
static TQString ToUnicode(const TQString& label);

static TQStringList *KResolver_initIdnDomains()
{
  const char *kde_use_idn = getenv("TDE_USE_IDN");
  if (!kde_use_idn)
     kde_use_idn = "ac:at:br:cat:ch:cl:cn:de:dk:fi:gr:hu:info:io:is:jp:kr:li:lt:museum:org:no:se:sh:th:tm:tw:vn";
  return new TQStringList(TQStringList::split(':', TQString::fromLatin1(kde_use_idn).lower()));
}

// implement the ToAscii function, as described by IDN documents
TQCString KResolver::domainToAscii(const TQString& unicodeDomain)
{
  if (!idnDomains)
    idnDomains = KResolver_initIdnDomains();

  TQCString retval;
  // RFC 3490, section 4 describes the operation:
  // 1) this is a query, so don't allow unassigned

  // 2) split the domain into individual labels, without
  // separators.
  TQStringList input = splitLabels(unicodeDomain);

  // Do we allow IDN names for this TLD?
  if (input.count() && !idnDomains->contains(input[input.count()-1].lower()))
    return input.join(".").lower().latin1(); // No IDN allowed for this TLD

  // 3) decide whether to enforce the STD3 rules for chars < 0x7F
  // we don't enforce

  // 4) for each label, apply ToASCII
  TQStringList::Iterator it = input.begin();
  const TQStringList::Iterator end = input.end();
  for ( ; it != end; ++it)
    {
      TQCString cs = ToASCII(*it);
      if (cs.isNull())
	return TQCString();	// error!

      // no, all is Ok.
      if (!retval.isEmpty())
	retval += '.';
      retval += cs;
    }

  return retval;
}

TQString KResolver::domainToUnicode(const TQCString& asciiDomain)
{
  return domainToUnicode(TQString::fromLatin1(asciiDomain));
}

// implement the ToUnicode function, as described by IDN documents
TQString KResolver::domainToUnicode(const TQString& asciiDomain)
{
  if (asciiDomain.isEmpty())
    return asciiDomain;
  if (!idnDomains)
    idnDomains = KResolver_initIdnDomains();

  TQString retval;

  // draft-idn-idna-14.txt, section 4 describes the operation:
  // 1) this is a query, so don't allow unassigned
  //   besides, input is ASCII

  // 2) split the domain into individual labels, without
  // separators.
  TQStringList input = splitLabels(asciiDomain);

  // Do we allow IDN names for this TLD?
  if (input.count() && !idnDomains->contains(input[input.count()-1].lower()))
    return asciiDomain.lower(); // No TLDs allowed

  // 3) decide whether to enforce the STD3 rules for chars < 0x7F
  // we don't enforce

  // 4) for each label, apply ToUnicode
  TQStringList::Iterator it;
  const TQStringList::Iterator end = input.end();
  for (it = input.begin(); it != end; ++it)
    {
      TQString label = ToUnicode(*it).lower();

      // ToUnicode can't fail
      if (!retval.isEmpty())
	retval += '.';
      retval += label;
    }

  return retval;
}

TQString KResolver::normalizeDomain(const TQString& domain)
{
  return domainToUnicode(domainToAscii(domain));
}

void KResolver::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

// here follows IDN functions
// all IDN functions conform to the following documents:
//  RFC 3454 - Preparation of Internationalized Strings
//  RFC 3490 - Internationalizing Domain Names in Applications (IDNA)
//  RFC 3491 - Nameprep: A Stringprep Profile for
//                Internationalized Domain Names (IDN
//  RFC 3492 - Punycode: A Bootstring encoding of Unicode
//          for Internationalized Domain Names in Applications (IDNA)

static TQStringList splitLabels(const TQString& unicodeDomain)
{
  // From RFC 3490 section 3.1:
  // "Whenever dots are used as label separators, the following characters
  // MUST be recognized as dots: U+002E (full stop), U+3002 (ideographic full
  // stop), U+FF0E (fullwidth full stop), U+FF61 (halfwidth ideographic full
  // stop)."
  static const unsigned int separators[] = { 0x002E, 0x3002, 0xFF0E, 0xFF61 };

  TQStringList lst;
  int start = 0;
  uint i;
  for (i = 0; i < unicodeDomain.length(); i++)
    {
      unsigned int c = unicodeDomain[i].unicode();

      if (c == separators[0] ||
	  c == separators[1] ||
	  c == separators[2] ||
	  c == separators[3])
	{
	  // found a separator!
	  lst << unicodeDomain.mid(start, i - start);
	  start = i + 1;
	}
    }
  if ((long)i >= start)
    // there is still one left
    lst << unicodeDomain.mid(start, i - start);

  return lst;
}

static TQCString ToASCII(const TQString& label)
{
#ifdef HAVE_IDNA_H
  // We have idna.h, so we can use the idna_to_ascii
  // function :)

  if (label.length() > 64)
    return (char*)0L;		// invalid label

  if (label.length() == 0)
    // this is allowed
    return TQCString("");	// empty, not null

  TQCString retval;
  char buf[65];

  TQ_UINT32* ucs4 = new TQ_UINT32[label.length() + 1];

  uint i;
  for (i = 0; i < label.length(); i++)
    ucs4[i] = (unsigned long)label[i].unicode();
  ucs4[i] = 0;			// terminate with NUL, just to be on the safe side

  if (idna_to_ascii_4i(ucs4, label.length(), buf, 0) == IDNA_SUCCESS)
    // success!
    retval = buf;

  delete [] ucs4;
  return retval;
#else
  return label.latin1();
#endif
}

static TQString ToUnicode(const TQString& label)
{
#ifdef HAVE_IDNA_H
  // We have idna.h, so we can use the idna_to_unicode
  // function :)

  TQ_UINT32 *ucs4_input, *ucs4_output;
  size_t outlen;

  ucs4_input = new TQ_UINT32[label.length() + 1];
  for (uint i = 0; i < label.length(); i++)
    ucs4_input[i] = (unsigned long)label[i].unicode();

  // try the same length for output
  ucs4_output = new TQ_UINT32[outlen = label.length()];

  idna_to_unicode_44i(ucs4_input, label.length(),
		      ucs4_output, &outlen,
		      0);

  if (outlen > label.length())
    {
      // it must have failed
      delete [] ucs4_output;
      ucs4_output = new TQ_UINT32[outlen];

      idna_to_unicode_44i(ucs4_input, label.length(),
			  ucs4_output, &outlen,
			  0);
    }

  // now set the answer
  TQString result;
  result.setLength(outlen);
  for (uint i = 0; i < outlen; i++)
    result[i] = (unsigned int)ucs4_output[i];

  delete [] ucs4_input;
  delete [] ucs4_output;

  return result;
#else
  return label;
#endif
}

#include "kresolver.moc"
