/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1997 Torben Weis (weis@kde.org)
 *
 *  $Id$
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
 **/

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
// on Linux/libc5, this includes linux/socket.h where SOMAXCONN is defined
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/un.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
extern "C" {
#include <netinet/in.h>

#include <arpa/inet.h>
}

#define KSOCK_NO_BROKEN
#include "kdebug.h"
// FIXME
// FOR BINARY COMPATIBILITY ONLY
// REMOVE WHEN PRACTICAL!
#define TDESOCKET_BINARY_COMPAT_HACK 1
#include "ksock.h"
#undef TDESOCKET_BINARY_COMPAT_HACK
#include "kextsock.h"
#include "ksockaddr.h"

#include "ksocks.h"

extern "C" {
#include <errno.h>
#include <fcntl.h>

#ifdef HAVE_GETADDRINFO
#include <netdb.h>
#endif

// defines MAXDNAME under Solaris
#include <arpa/nameser.h>
#include <resolv.h>
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>

#ifdef HAVE_SYSENT_H
#include <sysent.h>
#endif

#if TIME_WITH_SYS_TIME
#include <time.h>
#endif


// Play it safe, use a reasonable default, if SOMAXCONN was nowhere defined.
#ifndef SOMAXCONN
#warning Your header files do not seem to support SOMAXCONN
#define SOMAXCONN 5
#endif

#include <tqapplication.h>
#include <tqsocketnotifier.h>

#include "netsupp.h"		// leave this last

#ifdef __CYGWIN__
#include "tqwindowdefs.h"
#endif 

class TDESocketPrivate
{
public:
  TQSocketNotifier *readNotifier;
  TQSocketNotifier *writeNotifier;

  TDESocketPrivate() :
    readNotifier(0), writeNotifier(0)
  { }
};

// I moved this into here so we could accurately detect the domain, for
// posterity.  Really.
TDESocket::TDESocket( int _sock)
  : sock(_sock), d(new TDESocketPrivate)
{
  struct sockaddr_in sin;
  ksocklen_t len = sizeof(sin);

  memset(&sin, 0, len);

  // getsockname will fill in all the appropriate details, and
  // since sockaddr_in will exist everywhere and is somewhat compatible
  // with sockaddr_in6, we can use it to avoid needless ifdefs.
  KSocks::self()->getsockname(_sock, (struct sockaddr *)&sin, &len);
}

TDESocket::TDESocket( const char *_host, unsigned short int _port, int _timeout ) :
  sock( -1 ), d(new TDESocketPrivate)
{
    connect( _host, _port, _timeout );
}

TDESocket::TDESocket( const char *_path ) :
  sock( -1 ), d(new TDESocketPrivate)
{
  connect( _path );
}

void TDESocket::enableRead( bool _state )
{
  if ( _state )
    {
	  if ( !d->readNotifier  )
		{
		  d->readNotifier = new TQSocketNotifier( sock, TQSocketNotifier::Read );
		  TQObject::connect( d->readNotifier, TQ_SIGNAL( activated(int) ), this, TQ_SLOT( slotRead(int) ) );
		}
	  else
	    d->readNotifier->setEnabled( true );
    }
  else if ( d->readNotifier )
	d->readNotifier->setEnabled( false );
}

void TDESocket::enableWrite( bool _state )
{
  if ( _state )
    {
	  if ( !d->writeNotifier )
		{
		  d->writeNotifier = new TQSocketNotifier( sock, TQSocketNotifier::Write );
		  TQObject::connect( d->writeNotifier, TQ_SIGNAL( activated(int) ), this,
							TQ_SLOT( slotWrite(int) ) );
		}
	  else
	    d->writeNotifier->setEnabled( true );
    }
  else if ( d->writeNotifier )
	d->writeNotifier->setEnabled( false );
}

void TDESocket::slotRead( int )
{
  char buffer[2];

  int n = recv( sock, buffer, 1, MSG_PEEK );
  if ( n <= 0 )
	emit closeEvent( this );
  else
	emit readEvent( this );
}

void TDESocket::slotWrite( int )
{
  emit writeEvent( this );
}

/*
 * Connects the PF_UNIX domain socket to _path.
 */
bool TDESocket::connect( const char *_path )
{
  KExtendedSocket ks(TQString::null, _path, KExtendedSocket::unixSocket);

  ks.connect();
  sock = ks.fd();
  ks.release();

  return sock >= 0;
}

/*
 * Connects the socket to _host, _port.
 */
bool TDESocket::connect( const TQString& _host, unsigned short int _port, int _timeout )
{
  KExtendedSocket ks(_host, _port, KExtendedSocket::inetSocket);
  ks.setTimeout(_timeout, 0);

  ks.connect();
  sock = ks.fd();
  ks.release();

  return sock >= 0;
}

// only for doxygen - the define is always true as defined above
#ifdef KSOCK_NO_BROKEN
unsigned long TDESocket::ipv4_addr()
{
  unsigned long retval = 0;
  TDESocketAddress *sa = KExtendedSocket::peerAddress(sock);
  if (sa == NULL)
    return 0;

  if (sa->address() != NULL && (sa->address()->sa_family == PF_INET
#ifdef PF_INET6
				|| sa->address()->sa_family == PF_INET6
#endif
      ))
    {
      KInetSocketAddress *ksin = (KInetSocketAddress*)sa;
      const sockaddr_in *sin = ksin->addressV4();
      if (sin != NULL)
	retval = sin->sin_addr.s_addr;
    }
  delete sa;
  return retval;
}

bool TDESocket::initSockaddr (ksockaddr_in *server_name, const char *hostname, unsigned short int port, int domain)
{
  // This function is now IPv4 only
  // if you want something better, you should use KExtendedSocket::lookup yourself

  kdWarning(170) << "deprecated TDESocket::initSockaddr called" << endl;

  if (domain != PF_INET)
    return false;

  TQPtrList<KAddressInfo> list = KExtendedSocket::lookup(hostname, TQString::number(port),
                                                        KExtendedSocket::ipv4Socket);
  list.setAutoDelete(true);

  if (list.isEmpty())
    return false;

  memset(server_name, 0, sizeof(*server_name));

  // We are sure that only KInetSocketAddress objects are in the list
  KInetSocketAddress *sin = (KInetSocketAddress*)list.getFirst()->address();
  if (sin == NULL)
    return false;

  memcpy(server_name, sin->addressV4(), sizeof(*server_name));
  kdDebug(170) << "TDESocket::initSockaddr: returning " << sin->pretty() << endl;
  return true;
}

#endif

TDESocket::~TDESocket()
{
  // Coolo says delete 0 is ok :) -thiago
  delete d->readNotifier;
  delete d->writeNotifier;

  delete d;

  if (sock != -1) {
    ::close( sock );
  }
}

class TDEServerSocketPrivate
{
public:
   bool bind;
   TQCString path;
   unsigned short int port;
   KExtendedSocket *ks;
};


TDEServerSocket::TDEServerSocket( const char *_path, bool _bind ) :
  sock( -1 )
{
  d = new TDEServerSocketPrivate();
  d->bind = _bind;

  init ( _path );
}

TDEServerSocket::TDEServerSocket( unsigned short int _port, bool _bind ) :
  sock( -1 )
{
  d = new TDEServerSocketPrivate();
  d->bind = _bind;

  init ( _port );
}

bool TDEServerSocket::init( const char *_path )
{
  unlink(_path );
  d->path = _path;

  KExtendedSocket *ks = new KExtendedSocket(TQString::null, _path, KExtendedSocket::passiveSocket |
					    KExtendedSocket::unixSocket);
  d->ks = ks;

  if (d->bind)
    return bindAndListen(false);
  return true;
}


bool TDEServerSocket::init( unsigned short int _port )
{
  d->port = _port;
  KExtendedSocket *ks;
  ks = new KExtendedSocket(TQString::null, _port, KExtendedSocket::passiveSocket |
			   KExtendedSocket::inetSocket);
  d->ks = ks;

  if (d->bind)
    return bindAndListen(false);
  return true;
}

bool TDEServerSocket::bindAndListen(bool suppressFailureMessages)
{
  if (d == NULL || d->ks == NULL)
    return false;


  int ret = d->ks->listen( SOMAXCONN );
  if (ret < 0)
    {
	if (!suppressFailureMessages)
	    {
		kdWarning(170) << "Error listening on socket for port " << d->ks->port() << ": " << ret << "\n";
	    }
	delete d->ks;
	d->ks = NULL;
	sock = -1;
	return false;
    }


  sock = d->ks->fd();

  connect( d->ks->readNotifier(), TQ_SIGNAL( activated(int) ), this, TQ_SLOT( slotAccept(int) ) );
  return true;
}


unsigned short int TDEServerSocket::port()
{
  if (d == NULL || d->ks == NULL || sock == -1)
    return 0;
  const TDESocketAddress *sa = d->ks->localAddress();
  if (sa == NULL)
    return 0;

  // we can use sockaddr_in here even if it isn't IPv4
  sockaddr_in *sin = (sockaddr_in*)sa->address();

  if (sin->sin_family == PF_INET)
    // correct family
    return sin->sin_port;
#ifdef PF_INET6
  else if (sin->sin_family == PF_INET6)
    {
      kde_sockaddr_in6 *sin6 = (kde_sockaddr_in6*)sin;
      return sin6->sin6_port;
    }
#endif
  return 0;			// not a port we know
}

unsigned long TDEServerSocket::ipv4_addr()
{
  if (d == NULL || d->ks == NULL || sock == -1)
    return 0;
  const TDESocketAddress *sa = d->ks->localAddress();

  const sockaddr_in *sin = (sockaddr_in*)sa->address();

  if (sin->sin_family == PF_INET)
    // correct family
    return ntohl(sin->sin_addr.s_addr);
#ifdef PF_INET6
  else if (sin->sin_family == PF_INET6)
    {
      KInetSocketAddress *ksin = (KInetSocketAddress*)sa;
      sin = ksin->addressV4();
      if (sin != NULL)
	return sin->sin_addr.s_addr;
    }
#endif
  return 0;			// this is dumb, isn't it?
}

void TDEServerSocket::slotAccept( int )
{
  if (d == NULL || d->ks == NULL || sock == -1)
    return;			// nothing!

  KExtendedSocket *s;
  if (d->ks->accept(s) < 0)
    {
        kdWarning(170) << "Error accepting\n";
        return;
    }

  int new_sock = s->fd();
  s->release();			// we're getting rid of the KExtendedSocket
  delete s;

  emit accepted( new TDESocket( new_sock ) );
}

TDEServerSocket::~TDEServerSocket()
{
  if (d != NULL)
    {
      if (d->ks != NULL)
	delete d->ks;
      delete d;
    }
  // deleting d->ks closes the socket
  //  ::close( sock );
}

// DEPRECATED
bool TDEServerSocket::bindAndListen()
{
  return bindAndListen(false);
}

#include "ksock.moc"
