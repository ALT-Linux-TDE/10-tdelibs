/* This file is part of the KDE libraries
    Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kuniqueapplication.h"
#include "tdeglobal.h"
#include "kdebug.h"
#include "ksock.h"
#include "ksockaddr.h"
#include "kextsock.h"

#include <tdeaboutdata.h>
#include <tdecmdlineargs.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

bool check(TQString txt, TQString a, TQString b)
{
  if (a.isEmpty())
     a = TQString::null;
  if (b.isEmpty())
     b = TQString::null;
  if (a == b) {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}


int
main(int argc, char *argv[])
{
   TDEAboutData about("socktest", "SockTest", "version");
   TDECmdLineArgs::init(argc, argv, &about);
   TDEApplication::addCmdLineOptions();

   TDEApplication app;

   TQString host, port;

   KInetSocketAddress host_address("213.203.58.36", 80);

   check("KInetSocketAddress(\"213.203.58.36\", 80)", host_address.pretty(), "213.203.58.36 port 80");

   int result = KExtendedSocket::resolve(&host_address, host, port, NI_NAMEREQD);
   printf( "resolve result: %d\n", result );
   check("KExtendedSocket::resolve() host=", host, "www.kde.org");
//   check("KExtendedSocket::resolve() port=", port, "http");
   TQPtrList<KAddressInfo> list;
   list = KExtendedSocket::lookup("www.kde.org", "http", KExtendedSocket::inetSocket);
   for(KAddressInfo *info = list.first(); info; info = list.next())
   {
      tqWarning("Lookup: %s %s %s", info->address()->pretty().latin1(),
		                   info->address()->isEqual(KInetSocketAddress("213.203.58.36", 80)) ?
				   "is equal to" : "is NOT equal to",
				   "213.203.58.36 port 80");
   }
   check("KExtendedSocket::lookup()", list.first()->address()->pretty(), "213.203.58.36 port 80");



   int err;

   TQPtrList<KAddressInfo> cns = KExtendedSocket::lookup("www.kde.org", 0, KExtendedSocket::canonName, &err);
   for (KAddressInfo *x = cns.first(); x; x = cns.next()) {
        const char *canon = x->canonname();
        tqWarning( "Lookup: %s", canon ? canon : "<Null>");
   }
   check("KExtendedSocket::lookup() canonical", cns.first()->canonname(), "www.kde.org");

   KExtendedSocket * sock2 = new KExtendedSocket( "www.kde.org", 80 );
   check( "KExtendedSocket ctor / connect", TQString::number( sock2->connect() ), "0" );

   printf("FD %d\n", sock2->fd());

   TDESocketAddress* addr = KExtendedSocket::peerAddress( sock2->fd() );
   check( "peerAddress:", addr->nodeName().latin1(), "213.203.58.36" );

   check( "isEqual:", addr->isEqual(KInetSocketAddress("213.203.58.36", 80)) ? "TRUE" : "FALSE", "TRUE");
   check( "isEqual:", addr->isEqual(KInetSocketAddress("213.203.58.36", 8080)) ? "TRUE" : "FALSE", "FALSE");
   check( "isEqual:", addr->isCoreEqual(KInetSocketAddress("213.203.58.36", 8080)) ? "TRUE" : "FALSE", "TRUE");
}
