/*
   Copyright (C) 2000, 2001, 2002 Dawit Alemayehu <adawit@kde.org>

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
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

#define HAVE_TERMIOS_H 1
#define HAVE_GRANTPT 1

#include <stdlib.h>
#ifdef HAVE_PTY_H
#include <pty.h>
#endif
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#ifdef HAVE_STROPTS
#include <stropts.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif
#ifdef HAVE_UTIL_H
#include <util.h>
#endif

#include <tdeio/global.h>
#include <tdeio/slavebase.h>

#include "tdersync.h"
#include "rsyncconfigdialog.h"

#define myDebug(x) kdDebug(7127) << __LINE__ << ": " x

#define CONFIGURATION_FILE_SEPARATOR ';'

TDE_EXPORT KRsync::KRsync (TQObject* parent, const char* name)
                : TQObject (parent, name), m_progressDialog(0), m_progressDialogExists(false), m_bSettingsLoaded(false), m_bInSpecialSync(false)
{
    loadSettings();

    childPid = 0;
    isLoggedIn = false;
    firstLogin = true;
    connectionAuth.keepPassword = true;
    outBufPos = -1;
    outBuf = NULL;
    outBufLen = 0;
    isStat = false; // FIXME: just a workaround for konq deficiencies
    redirectUser = ""; // FIXME: just a workaround for konq deficiencies
    redirectPass = ""; // FIXME: just a workaround for konq deficiencies
}

TDE_EXPORT KRsync::~KRsync()
{

}

static int open_pty_pair(int fd[2])
{
#if defined(HAVE_TERMIOS_H) && defined(HAVE_GRANTPT) && !defined(HAVE_OPENPTY)
/** with kind regards to The GNU C Library
Reference Manual for Version 2.2.x of the GNU C Library */
    int master, slave;
    char *name;
    struct ::termios ti;
    memset(&ti,0,sizeof(ti));

    ti.c_cflag = CLOCAL|CREAD|CS8;
    ti.c_cc[VMIN] = 1;

#ifdef HAVE_GETPT
    master = getpt();
#elif defined(HAVE_POSIX_OPENTPT)
    master = posix_openpt(O_RDWR);
#else
    master = open("/dev/ptmx", O_RDWR);
#endif
    if (master < 0) return 0;

    if (grantpt(master) < 0 || unlockpt(master) < 0) goto close_master;

    name = ptsname(master);
    if (name == NULL) goto close_master;

    slave = open(name, O_RDWR);
    if (slave == -1) goto close_master;

#if (defined(HAVE_ISASTREAM) || defined(isastream)) && defined(I_PUSH)
    if (isastream(slave) &&
        (ioctl(slave, I_PUSH, "ptem") < 0 ||
         ioctl(slave, I_PUSH, "ldterm") < 0))
            goto close_slave;
#endif

    tcsetattr(slave, TCSANOW, &ti);
    fd[0] = master;
    fd[1] = slave;
    return 0;

#if (defined(HAVE_ISASTREAM) || defined(isastream)) && defined(I_PUSH)
close_slave:
#endif
    close(slave);

close_master:
    close(master);
    return -1;
#else
#ifdef HAVE_OPENPTY
    struct ::termios ti;
    memset(&ti,0,sizeof(ti));

    ti.c_cflag = CLOCAL|CREAD|CS8;
    ti.c_cc[VMIN] = 1;

    return openpty(fd,fd+1,NULL,&ti,NULL);
#else
#ifdef __GNUC__
#warning "No tty support available. Password dialog won't work."
#endif
    return socketpair(PF_UNIX,SOCK_STREAM,0,fd);
#endif
#endif
}
/**
creates the unidirectional sync subprocess
*/
TDE_EXPORT bool KRsync::syncUnidirectional(TQString synccommand, TQString syncflags, int parameter_order, TQString localfolder, TQString remotepath) {
    int fd[2];
    int rc, flags;
    thisFn = TQString();

    rc = open_pty_pair(fd);
    if (rc == -1) {
        myDebug( << "socketpair failed, error: " << strerror(errno) << endl);
        return true;
    }

    childPid = fork();
    if (childPid == -1) {
        myDebug( << "fork failed, error: " << strerror(errno) << endl);
        close(fd[0]);
        close(fd[1]);
        childPid = 0;
        return true;
    }
    if (childPid == 0) {
        // Create the rsync command to run
        TQString execstring;
        if (parameter_order == 0) {
            execstring = synccommand + syncflags + localfolder + TQString("/ ") + remotepath;
        }
        else {
            execstring = synccommand + syncflags + remotepath + TQString("/ ") + localfolder;
        }

        // taken from konsole, see TEPty.C for details
        // note: if we're running on socket pairs,
        // this will fail, but thats what we expect

        for (int sig = 1; sig < NSIG; sig++) signal(sig,SIG_DFL);

        struct rlimit rlp;
        getrlimit(RLIMIT_NOFILE, &rlp);
        for (int i = 0; i < (int)rlp.rlim_cur; i++)
            if (i != fd[1]) close(i);

        dup2(fd[1],0);
        dup2(fd[1],1);
        dup2(fd[1],2);
        if (fd[1] > 2) close(fd[1]);

        setsid();

#if defined(TIOCSCTTY)
        ioctl(0, TIOCSCTTY, 0);
#endif

        int pgrp = getpid();
#if defined( _AIX) || defined( __hpux)
        tcsetpgrp(0, pgrp);
#else
        ioctl(0, TIOCSPGRP, (char *)&pgrp);
#endif

        const char *dev = ttyname(0);
        setpgid(0,0);
        if (dev) close(open(dev, O_WRONLY, 0));
        setpgid(0,0);

        if (system(execstring.ascii()) < 0) {
		// ERROR
	}
        #undef common_args
        myDebug( << "could not exec! " << strerror(errno) << endl);
        ::exit(-1);
    }
    close(fd[1]);
    rc = fcntl(fd[0],F_GETFL,&flags);
    rc = fcntl(fd[0],F_SETFL,flags|O_NONBLOCK);
    childFd = fd[0];

    fd_set rfds, wfds;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    char buf[32768];
    int offset = 0;
    while (!isLoggedIn) {
        FD_SET(childFd,&rfds);
        FD_ZERO(&wfds);
        if (outBufPos >= 0) FD_SET(childFd,&wfds);
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;
        rc = select(childFd+1, &rfds, &wfds, NULL, &timeout);
        if (rc < 0) {
            if (errno == EINTR)
                continue;
            myDebug( << "select failed, rc: " << rc << ", error: " << strerror(errno) << endl);
            return true;
        }
        if (FD_ISSET(childFd,&wfds) && outBufPos >= 0) {
            if (outBuf) {
                rc = write(childFd,outBuf+outBufPos,outBufLen-outBufPos);
                fflush(stdout);
            }
            else {
                rc = 0;
            }
            if (rc >= 0) outBufPos += rc;
            else {
                if (errno == EINTR)
                    continue;
                myDebug( << "write failed, rc: " << rc << ", error: " << strerror(errno) << endl);
                outBufPos = -1;
                //return true;
            }
            if (outBufPos >= outBufLen) {
                outBufPos = -1;
                outBuf = NULL;
                outBufLen = 0;
            }
        }
        if (FD_ISSET(childFd,&rfds)) {
            rc = read(childFd,buf+offset,32768-offset);
            if (rc > 0) {
                int noff = establishConnectionRsync(buf,rc+offset);
                if (noff < 0) return false;
                if (noff > 0) memmove(buf,buf+offset+rc-noff,noff);
                offset = noff;
            } else {
                if (errno == EINTR)
                    continue;
                //if (errno == EAGAIN)
                //    continue;
                myDebug( << "read failed, rc: " << rc << ", error: " << strerror(errno) << endl);
                return true;
            }
        }
    }
    return false;
}

/**
creates the bidirectional sync subprocess
*/
TDE_EXPORT bool KRsync::syncBidirectional(TQString synccommand, TQString syncflags, int parameter_order, TQString localfolder, TQString remotepath) {
    int fd[2];
    int rc, flags;
    thisFn = TQString();

    // Check for and remove the trailing slash in localfolder
    if (localfolder.endsWith("/")) {
        localfolder.remove(localfolder.length()-1, 1);
    }

    rc = open_pty_pair(fd);
    if (rc == -1) {
        myDebug( << "socketpair failed, error: " << strerror(errno) << endl);
        return true;
    }

    childPid = fork();
    if (childPid == -1) {
        myDebug( << "fork failed, error: " << strerror(errno) << endl);
        close(fd[0]);
        close(fd[1]);
        childPid = 0;
        return true;
    }
    if (childPid == 0) {
        // Create the rsync command to run
        TQString execstring;
        execstring = synccommand + syncflags + localfolder + TQString(" ") + remotepath;

        // taken from konsole, see TEPty.C for details
        // note: if we're running on socket pairs,
        // this will fail, but thats what we expect

        for (int sig = 1; sig < NSIG; sig++) signal(sig,SIG_DFL);

        struct rlimit rlp;
        getrlimit(RLIMIT_NOFILE, &rlp);
        for (int i = 0; i < (int)rlp.rlim_cur; i++)
            if (i != fd[1]) close(i);

        dup2(fd[1],0);
        dup2(fd[1],1);
        dup2(fd[1],2);
        if (fd[1] > 2) close(fd[1]);

        setsid();

#if defined(TIOCSCTTY)
        ioctl(0, TIOCSCTTY, 0);
#endif

        int pgrp = getpid();
#if defined( _AIX) || defined( __hpux)
        tcsetpgrp(0, pgrp);
#else
        ioctl(0, TIOCSPGRP, (char *)&pgrp);
#endif

        const char *dev = ttyname(0);
        setpgid(0,0);
        if (dev) close(open(dev, O_WRONLY, 0));
        setpgid(0,0);

        if (system(execstring.ascii()) < 0) {
		// ERROR
	}
        #undef common_args
        myDebug( << "could not exec! " << strerror(errno) << endl);
        ::exit(-1);
    }
    close(fd[1]);
    rc = fcntl(fd[0],F_GETFL,&flags);
    rc = fcntl(fd[0],F_SETFL,flags|O_NONBLOCK);
    childFd = fd[0];

    fd_set rfds, wfds;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    char buf[32768];
    int offset = 0;
    while (!isLoggedIn) {
        FD_SET(childFd,&rfds);
        FD_ZERO(&wfds);
        if (outBufPos >= 0) FD_SET(childFd,&wfds);
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;
        rc = select(childFd+1, &rfds, &wfds, NULL, &timeout);
        if (rc < 0) {
            if (errno == EINTR)
                continue;
            myDebug( << "select failed, rc: " << rc << ", error: " << strerror(errno) << endl);
            return true;
        }
        if (FD_ISSET(childFd,&wfds) && outBufPos >= 0) {
            if (outBuf) {
                rc = write(childFd,outBuf+outBufPos,outBufLen-outBufPos);
                fflush(stdout);
            }
            else {
                rc = 0;
            }
            if (rc >= 0) outBufPos += rc;
            else {
                if (errno == EINTR)
                    continue;
                myDebug( << "write failed, rc: " << rc << ", error: " << strerror(errno) << endl);
                outBufPos = -1;
                //return true;
            }
            if (outBufPos >= outBufLen) {
                outBufPos = -1;
                outBuf = NULL;
                outBufLen = 0;
            }
        }
        if (FD_ISSET(childFd,&rfds)) {
            rc = read(childFd,buf+offset,32768-offset);
            if (rc > 0) {
                int noff = establishConnectionUnison(buf,rc+offset, localfolder, remotepath);
                if (noff < 0) return false;
                if (noff > 0) memmove(buf,buf+offset+rc-noff,noff);
                offset = noff;
            } else {
                if (errno == EINTR)
                    continue;
                //if (errno == EAGAIN)
                //    continue;
                myDebug( << "read failed, rc: " << rc << ", error: " << strerror(errno) << endl);
                return true;
            }
        }
    }
    return false;
}

/**
writes one chunk of data to stdin of child process
*/
TDE_EXPORT void KRsync::writeChild(const char *buf, TDEIO::fileoffset_t len) {
    if (outBufPos >= 0 && outBuf) {
#if 0
        TQString debug;
        debug.setLatin1(outBuf,outBufLen);
        if (len > 0) myDebug( << "write request while old one is pending, throwing away input (" << outBufLen << "," << outBufPos << "," << debug.left(10) << "...)" << endl);
#endif
        return;
    }
    outBuf = buf;
    outBufPos = 0;
    outBufLen = len;
}

/**
manages initial communication setup including password queries
*/
TDE_EXPORT int KRsync::establishConnectionRsync(char *buffer, TDEIO::fileoffset_t len) {
    TQString buf;
    buf.setLatin1(buffer,len);
    int pos;
    // Strip trailing whitespace
    while (buf.length() && (buf[buf.length()-1] == ' '))
       buf.truncate(buf.length()-1);

    myDebug( << "establishing: got " << buf << endl);
    while (childPid && ((pos = buf.find('\n')) >= 0 || buf.endsWith(":") || buf.endsWith("?"))) {
        if (m_progressDialogExists == true) {
            tqApp->processEvents();
        }
        pos++;
        TQString str = buf.left(pos);
        buf = buf.mid(pos);
        if (str == "\n")
            continue;
        //if (str.contains("rsync error:")) {
        if (str.contains("rsync:") || str.contains("failed.") || (str.contains("Could not") && str.endsWith("."))) {
            KMessageBox::error(NULL, str, i18n("Remote Folder Synchronization"));
        }
        else if (!str.isEmpty()) {
            thisFn += str;
            if ((buf.endsWith(":") == false) && (buf.endsWith("?") == false)) {
                // Display a nice little progress bar with text box
                if (m_progressDialogExists == false) {
                    m_progressDialog = new KProgressBoxDialog(0, "rsyncProgress", i18n("Synchronizing Folder..."), i18n("Synchronizing Folder..."), true);
                    m_progressDialog->progressBar()->setFormat("%v / %m");
                    m_progressDialog->setAutoClose(true);
                    m_progressDialog->progressBar()->setTotalSteps(2);
                    m_progressDialog->progressBar()->setValue(1);
                    connect (m_progressDialog, TQ_SIGNAL(cancelClicked()), TQ_SLOT(slotRsyncCancelled()));
                    if (m_bInSpecialSync) m_progressDialog->move(0,0);
                    m_progressDialog->show();
                    m_progressDialogExists = true;
                }
            }
        }
        else if (buf.endsWith(":")) {
            if (!redirectUser.isEmpty() && connectionUser != redirectUser) {
               // FIXME: Possibly do something here; is this the success response?
                return -1;
            } else if (!connectionPassword.isEmpty()) {
                myDebug( << "sending cpass" << endl);
                connectionAuth.password = connectionPassword+"\n";
                connectionPassword = TQString();
                writeChild(connectionAuth.password.latin1(),connectionAuth.password.length());
            } else {
                myDebug( << "sending mpass" << endl);
                connectionAuth.prompt = thisFn+buf;
                connectionAuth.password = TQString(); // don't prefill
                TQString thispass;
                if (KPasswordDialog::getPassword (thispass, i18n("Remote authorization required") + TQString("\n") + i18n("Please input") + TQString(" ") + TQString(buf), NULL) != 1) {
                    shutdownConnection(true, false);
                    return -1;
                }
                else {
                    connectionAuth.password = thispass;
                }
                connectionAuth.password += "\n";
                myDebug( << "sending pass" << endl);
                writeChild(connectionAuth.password.latin1(),connectionAuth.password.length());
            }
            thisFn = TQString();
            return 0;
        }
        else if (buf.endsWith("?")) {
            int rc = KMessageBox::questionYesNo(NULL, thisFn+buf, i18n("Remote Folder Synchronization"));
            if (rc == KMessageBox::Yes) {
                writeChild("yes\n",4);
            } else {
                writeChild("no\n",3);
            }
            thisFn = TQString();
            return 0;
        }

        if (m_progressDialogExists == true) {
            while (!m_progressDialog) {
                usleep(100);
            }
            while (!m_progressDialog->textEdit()) {
                usleep(100);
            }
            if (str.contains("exit()") && str.contains("ICE default IO")) {
                if (m_progressDialogExists == true) {
                    m_progressDialog->progressBar()->setValue(m_progressDialog->progressBar()->totalSteps());
                }
            }
            else {
                if (str.contains(", to-check=")) {
                    // Parse the to-check output
                    TQString tocheck_out_cur;
                    TQString tocheck_out_tot;
                    tocheck_out_cur = str.mid(str.find(", to-check=") + 11, str.length());
                    tocheck_out_tot = tocheck_out_cur.mid(tocheck_out_cur.find("/") + 1, tocheck_out_cur.length());
                    tocheck_out_cur = tocheck_out_cur.left(tocheck_out_cur.find("/"));
                    tocheck_out_tot = tocheck_out_tot.left(tocheck_out_tot.find(")"));
                    m_progressDialog->progressBar()->setTotalSteps(tocheck_out_tot.toInt()-1);
                    m_progressDialog->progressBar()->setValue(tocheck_out_tot.toInt()-tocheck_out_cur.toInt()-2);
                }
                else {
                    m_progressDialog->textEdit()->append(str);
                    m_progressDialog->textEdit()->scrollToBottom();
                }
            }
        }
    }
    return buf.length();
}

/**
manages initial communication setup including password queries
*/
TDE_EXPORT int KRsync::establishConnectionUnison(char *buffer, TDEIO::fileoffset_t len, TQString localfolder, TQString remotepath) {
    TQString buf;
    buf.setLatin1(buffer,len);
    int pos;
    // Strip trailing whitespace
    while (buf.length() && (buf[buf.length()-1] == ' '))
       buf.truncate(buf.length()-1);

    myDebug( << "establishing: got " << buf << endl);
    while (childPid && (((pos = buf.find('\n')) >= 0) || buf.endsWith(":") || buf.endsWith("?") || buf.endsWith("]"))) {
        if (m_progressDialogExists == true) {
            tqApp->processEvents();
        }
        pos++;
        TQString str = buf.left(pos);
        buf = buf.mid(pos);
        if (str == "\n")
            continue;
        //if (str.contains("rsync error:")) {
        if (((str.contains("rsync:") || str.contains("failed.") || str.contains("Could not")) && str.endsWith("."))) {
            KMessageBox::error(NULL, str, i18n("Remote Folder Synchronization"));
        }
        else if (str.startsWith("Fatal error")) {
            TQString fullError = str + buf;
            fullError.replace("Fatal error: Server:", TQString("<b>").append(i18n("An error ocurred on the remote system")).append(":</b><br>"));
            fullError.replace("\n", "");
            fullError.replace(": unable", ":<br>Unable");
            fullError.replace(")", ")</i><br>");
            fullError.replace("(", "<br><i>(");
            KMessageBox::error(NULL, fullError, i18n("Remote Folder Synchronization"));
        }
        else if (!str.isEmpty()) {
            thisFn += str;
            if ((buf.endsWith(":") == false) && (buf.endsWith("?") == false)) {
                // Display a nice little progress bar with text box
                if (m_progressDialogExists == false) {
                    m_progressDialog = new KProgressBoxDialog(0, "rsyncProgress", i18n("Synchronizing Folder..."), i18n("Synchronizing Folder..."), true);
                    m_progressDialog->progressBar()->setFormat("%v / %m");
                    m_progressDialog->progressBar()->setTotalSteps(0);
                    m_progressDialog->setAutoClose(true);
                    connect (m_progressDialog, TQ_SIGNAL(cancelClicked()), TQ_SLOT(slotUnisonCancelled()));
                    if (m_bInSpecialSync) m_progressDialog->move(0,0);
                    m_progressDialog->show();
                    m_progressDialogExists = true;
                }
            }
        }
        else if (buf.endsWith(":")) {
            if (!redirectUser.isEmpty() && connectionUser != redirectUser) {
               // FIXME: Possibly do something here; is this the success response?
                return -1;
            } else if (!connectionPassword.isEmpty()) {
                myDebug( << "sending cpass" << endl);
                connectionAuth.password = connectionPassword+"\n";
                connectionPassword = TQString();
                writeChild(connectionAuth.password.latin1(),connectionAuth.password.length());
            } else {
                myDebug( << "sending mpass" << endl);
                connectionAuth.prompt = thisFn+buf;
                connectionAuth.password = TQString(); // don't prefill
                TQString thispass;
                if (KPasswordDialog::getPassword (thispass, i18n("Remote authorization required") + TQString("\n") + i18n("Please input") + TQString(" ") + TQString(buf), NULL) != 1) {
                    slotUnisonCancelled();
                    return -1;
                }
                else {
                    connectionAuth.password = thispass;
                }
                connectionAuth.password += "\n";
                myDebug( << "sending pass" << endl);
                writeChild(connectionAuth.password.latin1(),connectionAuth.password.length());
            }
            thisFn = TQString();
            return 0;
        }
        else if (buf.endsWith("?") || buf.endsWith("? []")) {
            buf.replace("[]", "");
            if (buf.endsWith("? []")) {
                if (buf.startsWith("Proceed with propagating updates")) {
                    writeChild("y\n",3);
                }
                else {
                    int rc = KMessageBox::questionYesNo(NULL, buf, i18n("Remote Folder Synchronization"));
                    if (rc == KMessageBox::Yes) {
                        writeChild("y\n",3);
                    } else {
                        writeChild("n\n",3);
                    }
                }
            }
            else {
                if (buf.startsWith("Proceed with propagating updates")) {
                    writeChild("y\n",3);
                }
                else {
                    int rc = KMessageBox::questionYesNo(NULL, buf, i18n("Remote Folder Synchronization"));
                    if (rc == KMessageBox::Yes) {
                        writeChild("yes\n",4);
                    } else {
                        writeChild("no\n",3);
                    }
                }
            }
            thisFn = TQString();
            buf = "";
            return 0;
        }
        else if (buf.endsWith("]")) {
            if (m_progressDialogExists == true) {
                m_progressDialog->textEdit()->append(buf);
                m_progressDialog->textEdit()->scrollToBottom();
                int currentPos;
                currentPos = m_progressDialog->progressBar()->progress();
                m_progressDialog->progressBar()->setProgress(++currentPos);
            }
            TQString file_name;
            file_name = buf;
            file_name.replace("[]", "");
            file_name.replace(TQString("changed "), "");
            file_name.replace(TQString("new file "), "");
            file_name.replace("<-?->", "");
            file_name = file_name.stripWhiteSpace();
            if (localfolder.endsWith("Press return to continue.[]")) localfolder.truncate(localfolder.length()-TQString("Press return to continue.[]").length());
            if (remotepath.endsWith("Press return to continue.[]")) remotepath.truncate(remotepath.length()-TQString("Press return to continue.[]").length());
            KDialogBase *dialog= new KDialogBase(i18n("User Intervention Required"), KDialogBase::Yes | KDialogBase::No | KDialogBase::Cancel, KDialogBase::Yes, KDialogBase::Cancel, NULL, "warningYesNoCancel", true, true, i18n("Use &Local File"), i18n("Use &Remote File"), i18n("&Ignore"));
            TQString prettyremotepath = remotepath;
            TQString prettylocalfolder = localfolder;
            if (!prettyremotepath.endsWith("/")) prettyremotepath = prettyremotepath + "/";
            if (!prettylocalfolder.endsWith("/")) prettylocalfolder = prettylocalfolder + "/";
            prettyremotepath = prettyremotepath.stripWhiteSpace();
            prettylocalfolder = prettylocalfolder.stripWhiteSpace();
            int rc = KMessageBox::createKMessageBox(dialog, TQMessageBox::Warning, TQString("<b>") + i18n("WARNING: Both the local and remote file have been modified") + TQString("</b><p>") + i18n("Local") + TQString(": ") + prettylocalfolder + file_name + TQString("<br>") + i18n("Remote") + TQString(": ") + prettyremotepath + file_name + TQString("<p>") + i18n("Please select the file to duplicate (the other will be overwritten)") + TQString("<br>") + i18n("Or, select Ignore to skip synchronization of this file for now"), TQStringList(), TQString(), NULL, 1);
            if (rc == KDialogBase::Yes) {
                writeChild(">\n",3);
            }
            else if (rc == KDialogBase::No) {
                writeChild("<\n",3);
            }
            else {
                writeChild("/\n",3);
            }
            return 0;
        }

        if (m_progressDialogExists == true) {
            if ((str.contains("exit()") && str.contains("ICE default IO")) || (str.startsWith("Fatal error"))) {
                if (m_progressDialogExists == true) {
                    while (!m_progressDialog) {
                        usleep(100);
                    }
                    while (!m_progressDialog->textEdit()) {
                        usleep(100);
                    }
                    m_progressDialog->progressBar()->setFormat("%v / %m");
                    m_progressDialog->progressBar()->setTotalSteps(2);
                    m_progressDialog->progressBar()->setValue(m_progressDialog->progressBar()->totalSteps());
                }
            }
            else {
                if (m_progressDialogExists == true) {
                    while (!m_progressDialog) {
                        usleep(100);
                    }
                    while (!m_progressDialog->textEdit()) {
                        usleep(100);
                    }
                    m_progressDialog->textEdit()->append(str);
                    m_progressDialog->textEdit()->scrollToBottom();
                    int currentPos;
                    currentPos = m_progressDialog->progressBar()->progress();
                    m_progressDialog->progressBar()->setProgress(++currentPos);
                }
            }
        }
    }
    return buf.length();
}

/**
Forced close of the connection

This function gets called from the application side of the universe,
it shouldn't send any response.
 */
TDE_EXPORT void KRsync::closeConnection(){
    myDebug( << "closeConnection()" << endl);
    shutdownConnection(true, false);
}

/**
Closes the connection
 */
TDE_EXPORT void KRsync::shutdownConnection(bool forced, bool wait){
    if (childPid) {
        kill(childPid,SIGTERM); // We may not have permission...
        childPid = 0;
        if (wait == false) {
            close(childFd); // ...in which case this should do the trick
            childFd = -1;
        }
    }
    outBufPos = -1;
    outBuf = NULL;
    outBufLen = 0;
    isLoggedIn = false;
}

// --------------------------------------------------------------------------------------------
//
// Here begins the standard load/save/search/Konqy stuff
//
// --------------------------------------------------------------------------------------------

TDE_EXPORT void KRsync::saveSettings()
{
  TDEConfig cfg ("rsyncrc", false, false);
  cfg.setGroup ("General");
  cfg.writeEntry("LocalFolders", cfgfolderlist, CONFIGURATION_FILE_SEPARATOR);
  cfg.writeEntry("AutoSyncOnLogout", cfgautosync_onlogout_list, CONFIGURATION_FILE_SEPARATOR);
  cfg.sync();
}

TDE_EXPORT void KRsync::loadSettings()
{
  if (m_bSettingsLoaded)
    return;

  TDEConfig cfg ("rsyncrc", false, false);
  cfg.setGroup ("General");

  cfgfolderlist = cfg.readListEntry("LocalFolders", CONFIGURATION_FILE_SEPARATOR);
  cfgautosync_onlogout_list = cfg.readListEntry("AutoSyncOnLogout", CONFIGURATION_FILE_SEPARATOR);

  m_bSettingsLoaded = true;
}

TDE_EXPORT void KRsync::executeLogoutAutoSync()
{
  for (TQStringList::Iterator i(cfgautosync_onlogout_list.begin()); i != cfgautosync_onlogout_list.end(); ++i) {
	setCurrentDirectoryURL(*i);
	m_bInSpecialSync = true;
	slotSync();
	m_bInSpecialSync = false;
  }
}

TDE_EXPORT TQString KRsync::findLocalFolderByName(TQString folderurl)
{
  TQString folderurl_stripped;
  folderurl_stripped = folderurl;
  folderurl_stripped.replace(TQString("file://"), TQString(""));
  for (TQStringList::Iterator i(cfgfolderlist.begin()); i != cfgfolderlist.end(); ++i) {
	if (TQString::compare((*i), folderurl_stripped) == 0) {
		i++;
		return (*i);
		i++;
		i++;
		i++;
		i++;
		i++;
	}
  }
  return NULL;
}

TDE_EXPORT TQString KRsync::findSyncMethodByName(TQString folderurl)
{
  TQString folderurl_stripped;
  folderurl_stripped = folderurl;
  folderurl_stripped.replace(TQString("file://"), TQString(""));
  for (TQStringList::Iterator i(cfgfolderlist.begin()); i != cfgfolderlist.end(); ++i) {
	if (TQString::compare((*i), folderurl_stripped) == 0) {
		i++;
		i++;
		return (*i);
		i++;
		i++;
		i++;
		i++;
	}
  }
  return NULL;
}

TDE_EXPORT TQString KRsync::findLoginSyncEnabledByName(TQString folderurl)
{
  TQString folderurl_stripped;
  folderurl_stripped = folderurl;
  folderurl_stripped.replace(TQString("file://"), TQString(""));
  for (TQStringList::Iterator i(cfgfolderlist.begin()); i != cfgfolderlist.end(); ++i) {
	if (TQString::compare((*i), folderurl_stripped) == 0) {
		i++;
		i++;
		i++;
		i++;
		return (*i);
		i++;
		i++;
	}
  }
  return NULL;
}

TDE_EXPORT TQString KRsync::findLogoutSyncEnabledByName(TQString folderurl)
{
  TQString folderurl_stripped;
  folderurl_stripped = folderurl;
  folderurl_stripped.replace(TQString("file://"), TQString(""));
  for (TQStringList::Iterator i(cfgfolderlist.begin()); i != cfgfolderlist.end(); ++i) {
	if (TQString::compare((*i), folderurl_stripped) == 0) {
		i++;
		i++;
		i++;
		i++;
		i++;
		return (*i);
		i++;
	}
  }
  return NULL;
}

TDE_EXPORT TQString KRsync::findTimedSyncEnabledByName(TQString folderurl)
{
  TQString folderurl_stripped;
  folderurl_stripped = folderurl;
  folderurl_stripped.replace(TQString("file://"), TQString(""));
  for (TQStringList::Iterator i(cfgfolderlist.begin()); i != cfgfolderlist.end(); ++i) {
	if (TQString::compare((*i), folderurl_stripped) == 0) {
		i++;
		i++;
		i++;
		i++;
		i++;
		i++;
		return (*i);
	}
  }
  return NULL;
}

TDE_EXPORT int KRsync::deleteLocalFolderByName(TQString folderurl)
{
  TQString folderurl_stripped;
  folderurl_stripped = folderurl;
  folderurl_stripped.replace(TQString("file://"), TQString(""));
  for (TQStringList::Iterator i(cfgfolderlist.begin()); i != cfgfolderlist.end(); ++i) {
	if (TQString::compare((*i), folderurl_stripped) == 0) {
		i=cfgfolderlist.remove(i);
		i=cfgfolderlist.remove(i);
		i=cfgfolderlist.remove(i);
		i=cfgfolderlist.remove(i);
		i=cfgfolderlist.remove(i);
		i=cfgfolderlist.remove(i);
		cfgfolderlist.remove(i);
		return 0;
	}
  }
  return 1;
}

TDE_EXPORT int KRsync::addLocalFolderByName(TQString folderurl, TQString remoteurl, TQString syncmethod, TQString excludelist, TQString sync_on_login, TQString sync_on_logout, TQString sync_timed_interval)
{
  TQString folderurl_stripped;
  folderurl_stripped = folderurl;
  folderurl_stripped.replace(TQString("file://"), TQString(""));
  cfgfolderlist.append(folderurl);
  cfgfolderlist.append(remoteurl);
  cfgfolderlist.append(syncmethod);
  cfgfolderlist.append(excludelist);
  cfgfolderlist.append(sync_on_login);
  cfgfolderlist.append(sync_on_logout);
  cfgfolderlist.append(sync_timed_interval);
  return 1;
}

TDE_EXPORT void KRsync::setCurrentDirectoryURL (KURL url)
{
	m_pURL = url;
}

TDE_EXPORT void KRsync::slotSetup()
{
	KURL url = m_pURL;

	// Look up settings
	TQString localfolder = url.directory(true, true) + TQString("/") + url.fileName(true);
	TQString remotefolder = findLocalFolderByName(url.directory(true, true) + TQString("/") + url.fileName(true));
	TQString syncmethod = findSyncMethodByName(url.directory(true, true) + TQString("/") + url.fileName(true));
	int syncint = 1;
	if (syncmethod == NULL) {
		syncint = 1;
	}
	else if (syncmethod == "rsync_upload") {
		syncint = 1;
	}
	else if (syncmethod == "rsync_download") {
		syncint = 2;
	}
	else if (syncmethod == "rsync_bidirectional") {
		syncint = 3;
	}

	m_configDialog = new RsyncConfigDialog(0, "rsyncConfig", i18n("Remote Folder Synchronization"), i18n("Configuring Remote Folder Synchronization"), localfolder, remotefolder, syncint, true);

	// Handle autosync flags
	if (localfolder != "") {
		int flags = 0;
		if (cfgautosync_onlogout_list.contains(localfolder))
			flags = flags | 0x1;
		m_configDialog->setAutoSyncFlags(flags);
	}

	m_configDialog->show();

	connect (m_configDialog, TQ_SIGNAL(okClicked()), TQ_SLOT(slotSetupOK()));
	connect (m_configDialog, TQ_SIGNAL(cancelClicked()), TQ_SLOT(slotSetupCancelled()));
}

TDE_EXPORT void KRsync::slotSetupOK()
{
	KURL url = m_pURL;

	// Look up settings
	TQString localfolder = url.directory(true, true) + TQString("/") + url.fileName(true);
	TQString remotefolder = findLocalFolderByName(localfolder);
	TQString remotefolder_new = m_configDialog->lineEdit()->text().ascii();
	int syncmethod = m_configDialog->getSyncMode();
	TQString syncmethod_new = "";
	if (syncmethod == 1) {
		syncmethod_new = "rsync_upload";
	}
	else if (syncmethod == 2) {
		syncmethod_new = "rsync_download";
	}
	else if (syncmethod == 3) {
		syncmethod_new = "rsync_bidirectional";
	}

	// Handle autosync settings
	int autosyncflags = m_configDialog->getAutoSyncFlags();
	cfgautosync_onlogout_list.remove(localfolder);
	if (autosyncflags & 0x1)
		cfgautosync_onlogout_list.append(localfolder);

	// See if an old entry has to be deleted
	if (remotefolder.isEmpty() == false) {
		deleteLocalFolderByName(localfolder);
	}
	if (remotefolder_new.isEmpty() == false) {
		addLocalFolderByName(localfolder, remotefolder_new, syncmethod_new, "", "0", "0", "-1");
	}
	saveSettings();

	emit setupDone();
}

TDE_EXPORT void KRsync::slotSetupCancelled()
{
	emit setupDone();
}

TDE_EXPORT void KRsync::slotRsyncCancelled()
{
        shutdownConnection(true, true);
        if (m_progressDialogExists == true) {
            m_progressDialog->progressBar()->setValue(m_progressDialog->progressBar()->totalSteps());
        }

        emit transferDone();
}

TDE_EXPORT void KRsync::slotUnisonCancelled()
{
        shutdownConnection(true, true);
        if (m_progressDialogExists == true) {
            m_progressDialog->progressBar()->setFormat("%v / %m");
            m_progressDialog->progressBar()->setTotalSteps(2);
            m_progressDialog->progressBar()->setValue(m_progressDialog->progressBar()->totalSteps());
        }

        emit transferDone();
}

TDE_EXPORT void KRsync::slotSync()
{
	KURL url = m_pURL;

	TQString syncmethod = findSyncMethodByName(url.directory(true, true) + TQString("/") + url.fileName(true));
	if (syncmethod == NULL) {
		// Do nothing
	}
	else if (syncmethod == "rsync_upload") {
		// Initiate rsync
		syncUnidirectional(TQString("rsync"), TQString(" -avtzAXE --delete --progress "), 0, url.directory(true, true) + TQString("/") + url.fileName(true), findLocalFolderByName(url.directory(true, true) + TQString("/") + url.fileName(true)));
	}
 	else if (syncmethod == "rsync_download") {
 		syncUnidirectional(TQString("rsync"), TQString(" -avtzAXE --delete --progress "), 1, url.directory(true, true) + TQString("/") + url.fileName(true), findLocalFolderByName(url.directory(true, true) + TQString("/") + url.fileName(true)));
 	}
 	else if (syncmethod == "rsync_bidirectional") {
 		syncBidirectional(TQString("unison"), TQString(" -ui text -auto "), 1, url.directory(true, true) + TQString("/") + url.fileName(true), findLocalFolderByName(url.directory(true, true) + TQString("/") + url.fileName(true)));
 	}

	m_progressDialogExists = false;

	emit transferDone();
}

#include "tdersync.moc"
