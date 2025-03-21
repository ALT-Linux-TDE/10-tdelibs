/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// $Id$

#include <config.h>

#include <kde_file.h>
#include <ksock.h>
#include <tqtimer.h>

#include <sys/types.h>
#include <sys/time.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "tdeio/connection.h"

#include <kdebug.h>
#include <tqsocketnotifier.h>

#if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)
#define __progname getprogname()
#elif defined(_GNU_SOURCE) && defined(__GLIBC__)
#define __progname program_invocation_short_name
#else
extern char *__progname;
#endif

using namespace TDEIO;

Connection::Connection()
{
    f_out = 0;
    fd_in = -1;
    socket = 0;
    notifier = 0;
    receiver = 0;
    member = 0;
    m_suspended = false;
    tasks.setAutoDelete(true);
}

Connection::~Connection()
{
    close();
}

void Connection::suspend()
{
    m_suspended = true;
    if (notifier)
       notifier->setEnabled(false);
}

void Connection::resume()
{
    m_suspended = false;
    if (notifier)
       notifier->setEnabled(true);
}

void Connection::close()
{
    delete notifier;
    notifier = 0;
    delete socket;
    socket = 0;

    // TDESocket has already closed the file descriptor, but we need to
    // close the file-stream as well otherwise we leak memory. 
    // As a result we close the file descriptor twice, but that should
    // be harmless
    // KDE4: fix this
    if (f_out)
       fclose(f_out);
    f_out = 0;
    fd_in = -1;
    tasks.clear();
}

void Connection::send(int cmd, const TQByteArray& data)
{
    if (!inited() || tasks.count() > 0) {
	Task *task = new Task();
	task->cmd = cmd;
	task->data = data;
	tasks.append(task);
    } else {
	sendnow( cmd, data );
    }
}

void Connection::dequeue()
{
    if (!inited())
	return;

    while (tasks.count())
    {
       tasks.first();
       Task *task = tasks.take();
       sendnow( task->cmd, task->data );
       delete task;
    }
}

void Connection::init(TDESocket *sock)
{
    delete notifier;
    notifier = 0;
#ifdef Q_OS_UNIX //TODO: not yet available on WIN32
    delete socket;
    socket = sock;
    fd_in = socket->socket();
    f_out = KDE_fdopen( socket->socket(), "wb" );
#endif
    if (receiver && ( fd_in != -1 )) {
	notifier = new TQSocketNotifier(fd_in, TQSocketNotifier::Read);
	if ( m_suspended ) {
            suspend();
	}
	TQObject::connect(notifier, TQ_SIGNAL(activated(int)), receiver, member);
    }
    dequeue();
}

void Connection::init(int _fd_in, int fd_out)
{
    delete notifier;
    notifier = 0;
    fd_in = _fd_in;
    f_out = KDE_fdopen( fd_out, "wb" );
    if (receiver && ( fd_in != -1 )) {
	notifier = new TQSocketNotifier(fd_in, TQSocketNotifier::Read);
	if ( m_suspended ) {
            suspend();
	}
	TQObject::connect(notifier, TQ_SIGNAL(activated(int)), receiver, member);
    }
    dequeue();
}


void Connection::connect(TQObject *_receiver, const char *_member)
{
    receiver = _receiver;
    member = _member;
    delete notifier;
    notifier = 0;
    if (receiver && (fd_in != -1 )) {
	notifier = new TQSocketNotifier(fd_in, TQSocketNotifier::Read);
        if ( m_suspended )
            suspend();
	TQObject::connect(notifier, TQ_SIGNAL(activated(int)), receiver, member);
    }
}

bool Connection::sendnow( int _cmd, const TQByteArray &data )
{
    if (f_out == 0) {
	return false;
    }

    if (data.size() > 0xffffff)
        return false;

    static char buffer[ 64 ];
    sprintf( buffer, "%6x_%2x_", data.size(), _cmd );

    size_t n = fwrite( buffer, 1, 10, f_out );

    if ( n != 10 ) {
	kdError(7017) << "Could not send header (pid " << getpid() << " process \"" << __progname << "\")" << endl;
	return false;
    }

    n = fwrite( data.data(), 1, data.size(), f_out );

    if ( n != data.size() ) {
	kdError(7017) << "Could not write data (pid " << getpid() << " process \"" << __progname << "\")" << endl;
	return false;
    }

    if (fflush( f_out )) {
	kdError(7017) << "Could not write data (pid " << getpid() << " process \"" << __progname << "\")" << endl;
	return false;
    }

    return true;
}

int Connection::read( int* _cmd, TQByteArray &data )
{
    if (fd_in == -1 ) {
	kdError(7017) << "read: not yet inited (pid " << getpid() << " process \"" << __progname << "\")" << endl;
	return -1;
    }

    static char buffer[ 10 ];

 again1:
    ssize_t n = ::read( fd_in, buffer, 10);
    if ( n == -1 && errno == EINTR )
	goto again1;

    if ( n == -1) {
	kdError(7017) << "Header read failed, errno=" << errno << " (pid " << getpid() << " process \"" << __progname << "\")" << endl;
    }

    if ( n != 10 ) {
      if ( n ) // 0 indicates end of file
        kdError(7017) << "Header has invalid size (" << n << ") (pid " << getpid() << " process \"" << __progname << "\")" << endl;
      return -1;
    }

    buffer[ 6 ] = 0;
    buffer[ 9 ] = 0;

    char *p = buffer;
    while( *p == ' ' ) p++;
    long int len = strtol( p, 0L, 16 );

    p = buffer + 7;
    while( *p == ' ' ) p++;
    long int cmd = strtol( p, 0L, 16 );

    data.resize( len );

    if ( len > 0L ) {
	size_t bytesToGo = len;
	size_t bytesRead = 0;
	do {
	    n = ::read(fd_in, data.data()+bytesRead, bytesToGo);
	    if (n == -1) {
		if (errno == EINTR)
		    continue;

		kdError(7017) << "Data read failed, errno=" << errno << " (pid " << getpid() << " process \"" << __progname << "\")" << endl;
		return -1;
	    }
	    if ( !n ) { // 0 indicates end of file
        	kdError(7017) << "Connection ended unexpectedly (" << n << "/" << bytesToGo << ") (pid " << getpid() << " process \"" << __progname << "\")" << endl;
      		return -1;
    	    }

	    bytesRead += n;
	    bytesToGo -= n;
	}
	while(bytesToGo);
    }

    *_cmd = cmd;
    return len;
}

#include "connection.moc"
