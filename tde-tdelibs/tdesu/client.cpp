/*
 *
 * $Id$
 *
 * This file is part of the KDE project, module tdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; you can use this library under the GNU Library
 * General Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 * client.cpp: A client for tdesud.
 */

#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>
#ifdef HAVE_UCRED_H
#include <ucred.h>
#endif /* HAVE_UCRED_H */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include <tqglobal.h>
#include <tqcstring.h>
#include <tqfile.h>
#include <tqregexp.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <tdeapplication.h>
#include <kde_file.h>

#include "client.h"

class TDEsuClient::TDEsuClientPrivate {
public:
    TQString daemon;
};

#ifndef SUN_LEN
#define SUN_LEN(ptr) ((socklen_t) (((struct sockaddr_un *) 0)->sun_path) \
	             + strlen ((ptr)->sun_path))
#endif

TDEsuClient::TDEsuClient()
{
    sockfd = -1;
#ifdef TQ_WS_X11
    TQCString display(getenv("DISPLAY"));
    if (display.isEmpty())
    {
        kdWarning(900) << k_lineinfo << "$DISPLAY is not set\n";
        return;
    }

    // strip the screen number from the display
    display.replace(TQRegExp("\\.[0-9]+$"), "");
#else
    TQCString display("QWS");
#endif

    sock = TQFile::encodeName(locateLocal("socket", TQString("tdesud_%1").arg(display.data())));
    d = new TDEsuClientPrivate;
    connect();
}


TDEsuClient::~TDEsuClient()
{
    delete d;
    if (sockfd >= 0)
	close(sockfd);
}

int TDEsuClient::connect()
{
    if (sockfd >= 0)
	close(sockfd);
    if (access(sock, R_OK|W_OK))
    {
	sockfd = -1;
	return -1;
    }

    sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
	kdWarning(900) << k_lineinfo << "socket(): " << perror << "\n";
	return -1;
    }
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sock);

    if (::connect(sockfd, (struct sockaddr *) &addr, SUN_LEN(&addr)) < 0)
    {
        kdWarning(900) << k_lineinfo << "connect():" << perror << endl;
	close(sockfd); sockfd = -1;
	return -1;
    }

#if !defined(SO_PEERCRED) || !defined(HAVE_STRUCT_UCRED)
# if defined(HAVE_GETPEEREID)
    uid_t euid;
    gid_t egid;
    // Security: if socket exists, we must own it
    if (getpeereid(sockfd, &euid, &egid) == 0)
    {
       if (euid != getuid())
       {
            kdWarning(900) << "socket not owned by me! socket uid = " << euid << endl;
            close(sockfd); sockfd = -1;
            return -1;
       }
    }
# else
#  ifdef __GNUC__
#   warning "Using sloppy security checks"
#  endif
    // We check the owner of the socket after we have connected.
    // If the socket was somehow not ours an attacker will be able
    // to delete it after we connect but shouldn't be able to
    // create a socket that is owned by us.
    KDE_struct_stat s;
    if (KDE_lstat(sock, &s)!=0)
    {
        kdWarning(900) << "stat failed (" << sock << ")" << endl;
	close(sockfd); sockfd = -1;
	return -1;
    }
    if (s.st_uid != getuid())
    {
        kdWarning(900) << "socket not owned by me! socket uid = " << s.st_uid << endl;
	close(sockfd); sockfd = -1;
	return -1;
    }
    if (!S_ISSOCK(s.st_mode))
    {
        kdWarning(900) << "socket is not a socket (" << sock << ")" << endl;
	close(sockfd); sockfd = -1;
	return -1;
    }
# endif
#elif defined(HAVE_GETPEERUCRED)
    ucred_t *cred = nullptr;

    if (getpeerucred(sockfd, &cred) == 0) {
        uid_t peer_uid = ucred_geteuid(cred);

        ucred_free(cred);
        if (peer_uid != getuid()) {
            kdWarning(900) << "socket not owned by me! socket uid = " << peer_uid << endl;
            close(sockfd); sockfd = -1;
            return -1;
        }
    }
    if (cred != nullptr)
        ucred_free(cred);
#else
    struct ucred cred;
    socklen_t siz = sizeof(cred);

    // Security: if socket exists, we must own it
    if (getsockopt(sockfd, SOL_SOCKET, SO_PEERCRED, &cred, &siz) == 0)
    {
        if (cred.uid != getuid())
        {
            kdWarning(900) << "socket not owned by me! socket uid = " << cred.uid << endl;
            close(sockfd); sockfd = -1;
            return -1;
        }
    }
#endif

    return 0;
}

TQCString TDEsuClient::escape(const TQCString &str)
{
    TQCString copy = str;
    int n = 0;
    while ((n = copy.find("\\", n)) != -1)
    {
        copy.insert(n, '\\');
        n += 2;
    }
    n = 0;
    while ((n = copy.find("\"", n)) != -1)
    {
        copy.insert(n, '\\');
        n += 2;
    }
    copy.prepend("\"");
    copy.append("\"");
    return copy;
}

int TDEsuClient::command(const TQCString &cmd, TQCString *result)
{
    if (sockfd < 0)
	return -1;

    if (send(sockfd, cmd, cmd.length(), 0) != (int) cmd.length())
	return -1;

    char buf[1024];
    int nbytes = recv(sockfd, buf, 1023, 0);
    if (nbytes <= 0)
    {
	kdWarning(900) << k_lineinfo << "no reply from daemon\n";
	return -1;
    }
    buf[nbytes] = '\000';

    TQCString reply = buf;
    if (reply.left(2) != "OK")
	return -1;

    if (result)
	*result = reply.mid(3, reply.length()-4);
    return 0;
}

int TDEsuClient::setPass(const char *pass, int timeout)
{
    TQCString cmd = "PASS ";
    cmd += escape(pass);
    cmd += " ";
    cmd += TQCString().setNum(timeout);
    cmd += "\n";
    return command(cmd);
}

int TDEsuClient::exec(const TQCString &prog, const TQCString &user, const TQCString &options, const QCStringList &env)
{
    TQCString cmd;
    cmd = "EXEC ";
    cmd += escape(prog);
    cmd += " ";
    cmd += escape(user);
    if (!options.isEmpty() || !env.isEmpty())
    {
       cmd += " ";
       cmd += escape(options);
       for(QCStringList::ConstIterator it = env.begin(); 
          it != env.end(); ++it)
       {
          cmd += " ";
          cmd += escape(*it);
       }
    }
    cmd += "\n";
    return command(cmd);
}

int TDEsuClient::setHost(const TQCString &host)
{
    TQCString cmd = "HOST ";
    cmd += escape(host);
    cmd += "\n";
    return command(cmd);
}

int TDEsuClient::setPriority(int prio)
{
    TQCString cmd;
    cmd.sprintf("PRIO %d\n", prio);
    return command(cmd);
}

int TDEsuClient::setScheduler(int sched)
{
    TQCString cmd;
    cmd.sprintf("SCHD %d\n", sched);
    return command(cmd);
}

int TDEsuClient::delCommand(const TQCString &key, const TQCString &user)
{
    TQCString cmd = "DEL ";
    cmd += escape(key);
    cmd += " ";
    cmd += escape(user);
    cmd += "\n";
    return command(cmd);
}
int TDEsuClient::setVar(const TQCString &key, const TQCString &value, int timeout,
                        const TQCString &group)
{
    TQCString cmd = "SET ";
    cmd += escape(key);
    cmd += " ";
    cmd += escape(value);
    cmd += " ";
    cmd += escape(group);
    cmd += " ";
    cmd += TQCString().setNum(timeout);
    cmd += "\n";
    return command(cmd);
}

TQCString TDEsuClient::getVar(const TQCString &key)
{
    TQCString cmd = "GET ";
    cmd += escape(key);
    cmd += "\n";
    TQCString reply;
    command(cmd, &reply);
    return reply;
}

TQValueList<TQCString> TDEsuClient::getKeys(const TQCString &group)
{
    TQCString cmd = "GETK ";
    cmd += escape(group);
    cmd += "\n";
    TQCString reply;
    command(cmd, &reply);
    int index=0, pos;
    TQValueList<TQCString> list;
    if( !reply.isEmpty() )
    {
        // kdDebug(900) << "Found a matching entry: " << reply << endl;
        while (1)
        {
            pos = reply.find( '\007', index );
            if( pos == -1 )
            {
                if( index == 0 )
                    list.append( reply );
                else
                    list.append( reply.mid(index) );
                break;
            }
            else
            {
                list.append( reply.mid(index, pos-index) );
            }
            index = pos+1;
        }
    }
    return list;
}

bool TDEsuClient::findGroup(const TQCString &group)
{
    TQCString cmd = "CHKG ";
    cmd += escape(group);
    cmd += "\n";
    if( command(cmd) == -1 )
        return false;
    return true;
}

int TDEsuClient::delVar(const TQCString &key)
{
    TQCString cmd = "DELV ";
    cmd += escape(key);
    cmd += "\n";
    return command(cmd);
}

int TDEsuClient::delGroup(const TQCString &group)
{
    TQCString cmd = "DELG ";
    cmd += escape(group);
    cmd += "\n";
    return command(cmd);
}

int TDEsuClient::delVars(const TQCString &special_key)
{
    TQCString cmd = "DELS ";
    cmd += escape(special_key);
    cmd += "\n";
    return command(cmd);
}

int TDEsuClient::ping()
{
    return command("PING\n");
}

int TDEsuClient::exitCode()
{
    TQCString result;
    if (command("EXIT\n", &result) != 0)
       return -1;
       
    return result.toLong();
}

int TDEsuClient::stopServer()
{
    return command("STOP\n");
}

static TQString findDaemon()
{
    TQString daemon = locate("bin", "tdesud");
    if (daemon.isEmpty()) // if not in TDEDIRS, rely on PATH
	daemon = TDEStandardDirs::findExe("tdesud");

    if (daemon.isEmpty())
    {
	kdWarning(900) << k_lineinfo << "daemon not found\n";
    }
    return daemon;
}

bool TDEsuClient::isServerSGID()
{
    if (d->daemon.isEmpty())
       d->daemon = findDaemon();
    if (d->daemon.isEmpty())
       return false;
   
    KDE_struct_stat sbuf;
    if (KDE_stat(TQFile::encodeName(d->daemon), &sbuf) < 0)
    {
	kdWarning(900) << k_lineinfo << "stat(): " << perror << "\n";
	return false;
    }
    return (sbuf.st_mode & S_ISGID);
}

int TDEsuClient::startServer()
{
    if (d->daemon.isEmpty())
       d->daemon = findDaemon();
    if (d->daemon.isEmpty())
       return -1;

    if (!isServerSGID()) {
	kdWarning(900) << k_lineinfo << "tdesud not setgid!\n";
    }

    // tdesud only forks to the background after it is accepting
    // connections.
    // We start it via tdeinit to make sure that it doesn't inherit
    // any fd's from the parent process.
    int ret = kapp->tdeinitExecWait(d->daemon);
    connect();
    return ret;
}
