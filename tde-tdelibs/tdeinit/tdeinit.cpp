/*
 * This file is part of the KDE libraries
 * Copyright (c) 1999-2000 Waldo Bastian <bastian@kde.org>
 *           (c) 1999 Mario Weilguni <mweilguni@sime.com>
 *           (c) 2001 Lubos Lunak <l.lunak@kde.org>
 *
 * $Id$
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
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

#include "config.h"
#include <config.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>		// Needed on some systems.
#endif

#include <errno.h>
#include <fcntl.h>
#include <setproctitle.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <locale.h>

#include <tqstring.h>
#include <tqfile.h>
#include <tqdatetime.h>
#include <tqfileinfo.h>
#include <tqtextstream.h>
#include <tqregexp.h>
#include <tqfont.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <tdeglobal.h>
#include <tdeconfig.h>
#include <klibloader.h>
#include <tdeapplication.h>
#include <tdelocale.h>
#include <dcopglobal.h>

#ifdef HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#ifndef PR_SET_NAME
#define PR_SET_NAME 15
#endif
#endif

#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
#include <tdestartupinfo.h> // schroder
#endif

#include <tdeversion.h>

#include "ltdl.h"
#include "tdelauncher_cmds.h"

//#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
#ifdef TQ_WS_X11
//#undef K_WS_QTONLY
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#ifdef RTLD_GLOBAL
# define LTDL_GLOBAL	RTLD_GLOBAL
#else
# ifdef DL_GLOBAL
#  define LTDL_GLOBAL	DL_GLOBAL
# else
#  define LTDL_GLOBAL	0
# endif
#endif

#if defined(TDEINIT_USE_XFT) && defined(TDEINIT_USE_FONTCONFIG)
#include <X11/Xft/Xft.h>
extern "C" FcBool XftInitFtLibrary (void);
#include <fontconfig/fontconfig.h>
#endif

extern char **environ;

extern int lt_dlopen_flag;
//#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
#ifdef TQ_WS_X11
static int X11fd = -1;
static Display *X11display = 0;
static int X11_startup_notify_fd = -1;
static Display *X11_startup_notify_display = 0;
#endif
static const TDEInstance *s_instance = 0;
#define MAX_SOCK_FILE 255
static char sock_file[MAX_SOCK_FILE];
static char sock_file_old[MAX_SOCK_FILE];

//#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
#ifdef TQ_WS_X11
#define DISPLAY "DISPLAY"
#elif defined(TQ_WS_QWS)
#define DISPLAY "QWS_DISPLAY"
#elif defined(TQ_WS_MACX)
#define DISPLAY "MAC_DISPLAY"
#elif defined(K_WS_QTONLY)
#define DISPLAY "QT_DISPLAY"
#else
#error Use QT/X11 or QT/Embedded
#endif

/* Group data */
static struct {
  int maxname;
  int fd[2];
  int launcher[2]; /* socket pair for launcher communication */
  int deadpipe[2]; /* pipe used to detect dead children */
  int initpipe[2];
  int wrapper; /* socket for wrapper communication */
  int wrapper_old; /* old socket for wrapper communication */
  char result;
  int exit_status;
  pid_t fork;
  pid_t launcher_pid;
  pid_t my_pid;
  int n;
  lt_dlhandle handle;
  lt_ptr sym;
  char **argv;
  int (*func)(int, char *[]);
  int (*launcher_func)(int);
  bool debug_wait;
  int lt_dlopen_flag;
  TQCString errorMsg;
  bool launcher_ok;
  bool suicide;
} d;

//#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
#ifdef TQ_WS_X11
extern "C" {
int tdeinit_xio_errhandler( Display * );
int tdeinit_x_errhandler( Display *, XErrorEvent *err );
}
#endif

/* These are to link libtdeparts even if 'smart' linker is used */
#include <tdeparts/plugin.h>
extern "C" KParts::Plugin* _tdeinit_init_tdeparts() { return new KParts::Plugin(); }
/* These are to link libtdeio even if 'smart' linker is used */
#include <tdeio/authinfo.h>
extern "C" TDEIO::AuthInfo* _tdeioslave_init_tdeio() { return new TDEIO::AuthInfo(); }

/*
 * Close fd's which are only useful for the parent process.
 * Restore default signal handlers.
 */
static void close_fds()
{
   if (d.deadpipe[0] != -1)
   {
      close(d.deadpipe[0]);
      d.deadpipe[0] = -1;
   }

   if (d.deadpipe[1] != -1)
   {
      close(d.deadpipe[1]);
      d.deadpipe[1] = -1;
   }

   if (d.initpipe[0] != -1)
   {
      close(d.initpipe[0]);
      d.initpipe[0] = -1;
   }

   if (d.initpipe[1] != -1)
   {
      close(d.initpipe[1]);
      d.initpipe[1] = -1;
   }

   if (d.launcher_pid)
   {
      close(d.launcher[0]);
      d.launcher_pid = 0;
   }
   if (d.wrapper)
   {
      close(d.wrapper);
      d.wrapper = 0;
   }
   if (d.wrapper_old)
   {
      close(d.wrapper_old);
      d.wrapper_old = 0;
   }
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
//#ifdef TQ_WS_X11
   if (X11fd >= 0)
   {
      close(X11fd);
      X11fd = -1;
   }
   if (X11_startup_notify_fd >= 0 && X11_startup_notify_fd != X11fd )
   {
      close(X11_startup_notify_fd);
      X11_startup_notify_fd = -1;
   }
#endif

   signal(SIGCHLD, SIG_DFL);
   signal(SIGPIPE, SIG_DFL);
}

static void exitWithErrorMsg(const TQString &errorMsg)
{
   fprintf( stderr, "[tdeinit] %s\n", errorMsg.local8Bit().data() );
   TQCString utf8ErrorMsg = errorMsg.utf8();
   d.result = 3; // Error with msg
   write(d.fd[1], &d.result, 1);
   int l = utf8ErrorMsg.length();
   write(d.fd[1], &l, sizeof(int));
   write(d.fd[1], utf8ErrorMsg.data(), l);
   close(d.fd[1]);
   exit(255);
}

static void setup_tty( const char* tty )
{
    if( tty == NULL || *tty == '\0' )
        return;
    int fd = open( tty, O_WRONLY );
    if( fd < 0 )
    {
        fprintf(stderr, "[tdeinit] Couldn't open() %s: %s\n", tty, strerror (errno) );
        return;
    }
    if( dup2( fd, STDOUT_FILENO ) < 0 )
    {
        fprintf(stderr, "[tdeinit] Couldn't dup2() %s: %s\n", tty, strerror (errno) );
        close( fd );
        return;
    }
    if( dup2( fd, STDERR_FILENO ) < 0 )
    {
        fprintf(stderr, "[tdeinit] Couldn't dup2() %s: %s\n", tty, strerror (errno) );
        close( fd );
        return;
    }
    close( fd );
}

// from tdecore/netwm.cpp
static int get_current_desktop( Display* disp )
{
    int desktop = 0; // no desktop by default
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
//#ifdef TQ_WS_X11 // Only X11 supports multiple desktops
    Atom net_current_desktop = XInternAtom( disp, "_NET_CURRENT_DESKTOP", False );
    Atom type_ret;
    int format_ret;
    unsigned char *data_ret;
    unsigned long nitems_ret, unused;
    if( XGetWindowProperty( disp, DefaultRootWindow( disp ), net_current_desktop,
        0l, 1l, False, XA_CARDINAL, &type_ret, &format_ret, &nitems_ret, &unused, &data_ret )
	    == Success)
    {
	if (type_ret == XA_CARDINAL && format_ret == 32 && nitems_ret == 1)
	    desktop = *((long *) data_ret) + 1;
        if (data_ret)
            XFree ((char*) data_ret);
    }
#endif
    return desktop;
}

// var has to be e.g. "DISPLAY=", i.e. with =
const char* get_env_var( const char* var, int envc, const char* envs )
{
    if( envc > 0 )
    { // get the var from envs
        const char* env_l = envs;
        int ln = strlen( var );
        for (int i = 0;  i < envc; i++)
        {
            if( strncmp( env_l, var, ln ) == 0 )
                return env_l + ln;
            while(*env_l != 0) env_l++;
                env_l++;
        }
    }
    return NULL;
}

#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
//#ifdef TQ_WS_X11 // FIXME(E): Implement for Qt/Embedded
static void init_startup_info( TDEStartupInfoId& id, const char* bin,
    int envc, const char* envs )
{
    const char* dpy = get_env_var( DISPLAY"=", envc, envs );
    // this may be called in a child, so it can't use display open using X11display
    // also needed for multihead
    X11_startup_notify_display = XOpenDisplay( dpy );
    if( X11_startup_notify_display == NULL )
        return;
    X11_startup_notify_fd = XConnectionNumber( X11_startup_notify_display );
    TDEStartupInfoData data;
    int desktop = get_current_desktop( X11_startup_notify_display );
    data.setDesktop( desktop );
    data.setBin( bin );
    TDEStartupInfo::sendChangeX( X11_startup_notify_display, id, data );
    XFlush( X11_startup_notify_display );
}

static void complete_startup_info( TDEStartupInfoId& id, pid_t pid )
{
    if( X11_startup_notify_display == NULL )
        return;
    if( pid == 0 ) // failure
        TDEStartupInfo::sendFinishX( X11_startup_notify_display, id );
    else
    {
        TDEStartupInfoData data;
        data.addPid( pid );
        data.setHostname();
        TDEStartupInfo::sendChangeX( X11_startup_notify_display, id, data );
    }
    XCloseDisplay( X11_startup_notify_display );
    X11_startup_notify_display = NULL;
    X11_startup_notify_fd = -1;
}
#endif

TQCString execpath_avoid_loops( const TQCString& exec, int envc, const char* envs, bool avoid_loops )
{
     TQStringList paths;
     if( envc > 0 ) /* use the passed environment */
     {
         const char* path = get_env_var( "PATH=", envc, envs );
         if( path != NULL )
             paths = TQStringList::split( TQRegExp( "[:\b]" ), path, true );
     }
     else
         paths = TQStringList::split( TQRegExp( "[:\b]" ), getenv( "PATH" ), true );
     TQCString execpath = TQFile::encodeName(
         s_instance->dirs()->findExe( exec, paths.join( TQString( ":" ))));
     if( avoid_loops && !execpath.isEmpty())
     {
         int pos = execpath.findRev( '/' );
         TQString bin_path = execpath.left( pos );
         for( TQStringList::Iterator it = paths.begin();
              it != paths.end();
              ++it )
             if( ( *it ) == bin_path || ( *it ) == bin_path + '/' )
             {
                 paths.remove( it );
                 break; // -->
             }
         execpath = TQFile::encodeName(
             s_instance->dirs()->findExe( exec, paths.join( TQString( ":" ))));
     }
     return execpath;
}

#ifdef TDEINIT_OOM_PROTECT
static int oom_pipe = -1;

static void oom_protect_sighandler( int ) {
}

static void reset_oom_protect() {
   if( oom_pipe <= 0 )
      return;
   struct sigaction act, oldact;
   act.sa_handler = oom_protect_sighandler;
   act.sa_flags = 0;
   sigemptyset( &act.sa_mask );
   sigaction( SIGUSR1, &act, &oldact );
   sigset_t sigs, oldsigs;
   sigemptyset( &sigs );
   sigaddset( &sigs, SIGUSR1 );
   sigprocmask( SIG_BLOCK, &sigs, &oldsigs );
   pid_t pid = getpid();
   if( write( oom_pipe, &pid, sizeof( pid_t )) > 0 ) {
      sigsuspend( &oldsigs ); // wait for the signal to come
    }
   sigprocmask( SIG_SETMASK, &oldsigs, NULL );
   sigaction( SIGUSR1, &oldact, NULL );
   close( oom_pipe );
   oom_pipe = -1;
}
#else
static void reset_oom_protect() {
}
#endif

static pid_t launch(int argc, const char *_name, const char *args,
                    const char *cwd=0, int envc=0, const char *envs=0,
                    bool reset_env = false,
                    const char *tty=0, bool avoid_loops = false,
                    const char* startup_id_str = "0" )
{
  int launcher = 0;
  TQCString lib;
  TQCString name;
  TQCString exec;

  if (strcmp(_name, "tdelauncher") == 0) {
     /* tdelauncher is launched in a special way:
      * It has a communication socket on LAUNCHER_FD
      */
     if (0 > socketpair(AF_UNIX, SOCK_STREAM, 0, d.launcher))
     {
        perror("[tdeinit] socketpair() failed!\n");
        exit(255);
     }
     launcher = 1;
  }

  TQCString libpath;
  TQCString execpath;
  if (_name[0] != '/')
  {
     /* Relative name without '.la' */
     name = _name;
     lib = name + ".la";
     exec = name;
     libpath = TQFile::encodeName(KLibLoader::findLibrary( lib, s_instance ));
     execpath = execpath_avoid_loops( exec, envc, envs, avoid_loops );
  }
  else
  {
     lib = _name;
     name = _name;
     name = name.mid( name.findRev('/') + 1);
     exec = _name;
     if (lib.right(3) == ".la")
        libpath = lib;
     else
        execpath = exec;
  }
  if (!args)
  {
    argc = 1;
  }

  if (0 > pipe(d.fd))
  {
     perror("[tdeinit] pipe() failed!\n");
     d.result = 3;
     d.errorMsg = i18n("Unable to start new process.\n"
                       "The system may have reached the maximum number of open files possible or the maximum number of open files that you are allowed to use has been reached.").utf8();
     close(d.fd[0]);
     close(d.fd[1]);
     d.fork = 0;
     return d.fork;
  }

#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
//#ifdef TQ_WS_X11
  TDEStartupInfoId startup_id;
  startup_id.initId( startup_id_str );
  if( !startup_id.none())
      init_startup_info( startup_id, name, envc, envs );
#endif

  d.errorMsg = 0;
  d.fork = fork();
  switch(d.fork) {
  case -1:
     perror("[tdeinit] fork() failed!\n");
     d.result = 3;
     d.errorMsg = i18n("Unable to create new process.\n"
                       "The system may have reached the maximum number of processes possible or the maximum number of processes that you are allowed to use has been reached.").utf8();
     close(d.fd[0]);
     close(d.fd[1]);
     d.fork = 0;
     break;
  case 0:
     /** Child **/
     close(d.fd[0]);
     close_fds();
     if (launcher)
     {
        if (d.fd[1] == LAUNCHER_FD)
        {
          d.fd[1] = dup(d.fd[1]); // Evacuate from LAUNCHER_FD
        }
        if (d.launcher[1] != LAUNCHER_FD)
        {
          dup2( d.launcher[1], LAUNCHER_FD); // Make sure the socket has fd LAUNCHER_FD
          close( d.launcher[1] );
        }
        close( d.launcher[0] );
     }
     reset_oom_protect();

     if (cwd && *cwd)
        chdir(cwd);

     if( reset_env ) // KWRAPPER/SHELL
     {

         TQStrList unset_envs;
         for( int tmp_env_count = 0;
              environ[tmp_env_count];
              tmp_env_count++)
             unset_envs.append( environ[ tmp_env_count ] );
         for( TQStrListIterator it( unset_envs );
              it.current() != NULL ;
              ++it )
         {
             TQCString tmp( it.current());
             int pos = tmp.find( '=' );
             if( pos >= 0 )
                 unsetenv( tmp.left( pos ));
         }
     }

     for (int i = 0;  i < envc; i++)
     {
        putenv((char *)envs);
        while(*envs != 0) envs++;
        envs++;
     }

#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
//#ifdef TQ_WS_X11
      if( startup_id.none())
          TDEStartupInfo::resetStartupEnv();
      else
          startup_id.setupStartupEnv();
#endif
     {
       int r;
       TQCString procTitle;
       d.argv = (char **) malloc(sizeof(char *) * (argc+1));
       d.argv[0] = (char *) _name;
       for (int i = 1;  i < argc; i++)
       {
          d.argv[i] = (char *) args;
          procTitle += " ";
          procTitle += (char *) args;
          while(*args != 0) args++;
          args++;
       }
       d.argv[argc] = 0;

       /** Give the process a new name **/
#ifdef HAVE_SYS_PRCTL_H
       /* set the process name, so that killall works like intended */
       r = prctl(PR_SET_NAME, (unsigned long) name.data(), 0, 0, 0);
       if ( r == 0 )
           tdeinit_setproctitle( "%s [tdeinit]%s", name.data(), procTitle.data() ? procTitle.data() : "" );
       else
           tdeinit_setproctitle( "[tdeinit] %s%s", name.data(), procTitle.data() ? procTitle.data() : "" );
#else
       tdeinit_setproctitle( "[tdeinit] %s%s", name.data(), procTitle.data() ? procTitle.data() : "" );
#endif
     }

     d.handle = 0;
     if (libpath.isEmpty() && execpath.isEmpty())
     {
        TQString errorMsg = i18n("Could not find '%1' executable.").arg(TQFile::decodeName(_name));
        exitWithErrorMsg(errorMsg);
     }

     if ( getenv("TDE_IS_PRELINKED") && !execpath.isEmpty() && !launcher)
         libpath.truncate(0);

     if ( !libpath.isEmpty() )
     {
       d.handle = lt_dlopen( TQFile::encodeName(libpath) );
       if (!d.handle )
       {
          const char * ltdlError = lt_dlerror();
          if (execpath.isEmpty())
          {
             // Error
             TQString errorMsg = i18n("Could not open library '%1'.\n%2").arg(TQFile::decodeName(libpath))
		.arg(ltdlError ? TQFile::decodeName(ltdlError) : i18n("Unknown error"));
             exitWithErrorMsg(errorMsg);
          }
          else
          {
             // Print warning
             fprintf(stderr, "Could not open library %s: %s\n", lib.data(), ltdlError != 0 ? ltdlError : "(null)" );
          }
       }
     }
     lt_dlopen_flag = d.lt_dlopen_flag;
     if (!d.handle )
     {
        d.result = 2; // Try execing
        write(d.fd[1], &d.result, 1);

        // We set the close on exec flag.
        // Closing of d.fd[1] indicates that the execvp succeeded!
        fcntl(d.fd[1], F_SETFD, FD_CLOEXEC);

        setup_tty( tty );

        execvp(execpath.data(), d.argv);
        d.result = 1; // Error
        write(d.fd[1], &d.result, 1);
        close(d.fd[1]);
        exit(255);
     }

     d.sym = lt_dlsym( d.handle, "tdeinitmain");
     if (!d.sym )
     {
        d.sym = lt_dlsym( d.handle, "kdemain" );
        if ( !d.sym )
        {
#if ! KDE_IS_VERSION( 3, 90, 0 )
           d.sym = lt_dlsym( d.handle, "main");
#endif
           if (!d.sym )
           {
              const char * ltdlError = lt_dlerror();
              fprintf(stderr, "Could not find kdemain: %s\n", ltdlError != 0 ? ltdlError : "(null)" );
              TQString errorMsg = i18n("Could not find 'kdemain' in '%1'.\n%2").arg(TQString(libpath))
                 .arg(ltdlError ? TQFile::decodeName(ltdlError) : i18n("Unknown error"));
              exitWithErrorMsg(errorMsg);
           }
        }
     }

     d.result = 0; // Success
     write(d.fd[1], &d.result, 1);
     close(d.fd[1]);

     d.func = (int (*)(int, char *[])) d.sym;
     if (d.debug_wait)
     {
        fprintf(stderr, "[tdeinit] Suspending process\n"
                        "[tdeinit] 'gdb tdeinit %d' to debug\n"
                        "[tdeinit] 'kill -SIGCONT %d' to continue\n",
                        getpid(), getpid());
        kill(getpid(), SIGSTOP);
     }
     else
     {
        setup_tty( tty );
     }

     exit( d.func(argc, d.argv)); /* Launch! */

     break;
  default:
     /** Parent **/
     close(d.fd[1]);
     if (launcher)
     {
        close(d.launcher[1]);
        d.launcher_pid = d.fork;
     }
     bool exec = false;
     for(;;)
     {
       d.n = read(d.fd[0], &d.result, 1);
       if (d.n == 1)
       {
          if (d.result == 2)
          {
#ifndef NDEBUG
             fprintf(stderr, "[tdeinit] %s is executable. Launching.\n", _name );
#endif
             exec = true;
             continue;
          }
          if (d.result == 3)
          {
             int l = 0;
             d.n = read(d.fd[0], &l, sizeof(int));
             if (d.n == sizeof(int))
             {
                TQCString tmp;
                tmp.resize(l+1);
                d.n = read(d.fd[0], tmp.data(), l);
                tmp[l] = 0;
                if (d.n == l)
                   d.errorMsg = tmp;
             }
          }
          // Finished
          break;
       }
       if (d.n == -1)
       {
          if (errno == ECHILD) {  // a child died.
             continue;
          }
          if (errno == EINTR || errno == EAGAIN) { // interrupted or more to read
             continue;
          }
       }
       if (exec)
       {
          d.result = 0;
          break;
       }
       if (d.n == 0)
       {
          perror("[tdeinit] Pipe closed unexpectedly");
          d.result = 1; // Error
          break;
       }
       perror("[tdeinit] Error reading from pipe");
       d.result = 1; // Error
       break;
     }
     close(d.fd[0]);
     if (launcher && (d.result == 0))
     {
        // Trader launched successful
        d.launcher_pid = d.fork;
     }
  }
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
//#ifdef TQ_WS_X11
  if( !startup_id.none())
  {
     if( d.fork && d.result == 0 ) // launched successfully
        complete_startup_info( startup_id, d.fork );
     else // failure, cancel ASN
        complete_startup_info( startup_id, 0 );
  }
#endif
  return d.fork;
}

static void sig_child_handler(int)
{
   /*
    * Write into the pipe of death.
    * This way we are sure that we return from the select()
    *
    * A signal itself causes select to return as well, but
    * this creates a race-condition in case the signal arrives
    * just before we enter the select.
    */
   char c = 0;
   write(d.deadpipe[1], &c, 1);
}

static void init_signals()
{
  struct sigaction act;
  long options;

  if (pipe(d.deadpipe) != 0)
  {
     perror("[tdeinit] Aborting. Can't create pipe: ");
     exit(255);
  }

  options = fcntl(d.deadpipe[0], F_GETFL);
  if (options == -1)
  {
     perror("[tdeinit] Aborting. Can't make pipe non-blocking: ");
     exit(255);
  }

  if (fcntl(d.deadpipe[0], F_SETFL, options | O_NONBLOCK) == -1)
  {
     perror("[tdeinit] Aborting. Can't make pipe non-blocking: ");
     exit(255);
  }

  /*
   * A SIGCHLD handler is installed which sends a byte into the
   * pipe of death. This is to ensure that a dying child causes
   * an exit from select().
   */
  act.sa_handler=sig_child_handler;
  sigemptyset(&(act.sa_mask));
  sigaddset(&(act.sa_mask), SIGCHLD);
  sigprocmask(SIG_UNBLOCK, &(act.sa_mask), 0L);
  act.sa_flags = SA_NOCLDSTOP;

  // CC: take care of SunOS which automatically restarts interrupted system
  // calls (and thus does not have SA_RESTART)

#ifdef SA_RESTART
  act.sa_flags |= SA_RESTART;
#endif
  sigaction( SIGCHLD, &act, 0L);

  act.sa_handler=SIG_IGN;
  sigemptyset(&(act.sa_mask));
  sigaddset(&(act.sa_mask), SIGPIPE);
  sigprocmask(SIG_UNBLOCK, &(act.sa_mask), 0L);
  act.sa_flags = 0;
  sigaction( SIGPIPE, &act, 0L);
}

static void init_tdeinit_socket()
{
  struct sockaddr_un sa;
  struct sockaddr_un sa_old;
  kde_socklen_t socklen;
  long options;
  const char *home_dir = getenv("HOME");
  int max_tries = 10;
  if (!home_dir || !home_dir[0])
  {
     fprintf(stderr, "[tdeinit] Aborting. $HOME not set!");
     exit(255);
  }
  chdir(home_dir);

  {
     TQCString path = home_dir;
     TQCString readOnly = getenv("TDE_HOME_READONLY");
     if (access(path.data(), R_OK|W_OK))
     {
       if (errno == ENOENT)
       {
          fprintf(stderr, "[tdeinit] Aborting. $HOME directory (%s) does not exist.\n", path.data());
          exit(255);
       }
       else if (readOnly.isEmpty())
       {
          fprintf(stderr, "[tdeinit] Aborting. No write access to $HOME directory (%s).\n", path.data());
          exit(255);
       }
     }
     path = IceAuthFileName();
     if (access(path.data(), R_OK|W_OK) && (errno != ENOENT))
     {
       fprintf(stderr, "[tdeinit] Aborting. No write access to '%s'.\n", path.data());
       exit(255);
     }
  }

  /** Test if socket file is already present
   *  note that access() resolves symlinks, and so we check the actual
   *  socket file if it exists
   */
  if (access(sock_file, W_OK) == 0)
  {
     int s;
     struct sockaddr_un server;

//     fprintf(stderr, "[tdeinit] Warning, socket_file already exists!\n");
     /*
      * create the socket stream
      */
     s = socket(PF_UNIX, SOCK_STREAM, 0);
     if (s < 0)
     {
        perror("socket() failed: ");
        exit(255);
     }
     server.sun_family = AF_UNIX;
     strcpy(server.sun_path, sock_file);
     socklen = sizeof(server);

     if(connect(s, (struct sockaddr *)&server, socklen) == 0)
     {
        fprintf(stderr, "[tdeinit] Shutting down running client.\n");
        tdelauncher_header request_header;
        request_header.cmd = LAUNCHER_TERMINATE_TDEINIT;
        request_header.arg_length = 0;
        write(s, &request_header, sizeof(request_header));
        sleep(1); // Give it some time
     }
     close(s);
  }

  /** Delete any stale socket file (and symlink) **/
  unlink(sock_file);
  unlink(sock_file_old);

  /** create socket **/
  d.wrapper = socket(PF_UNIX, SOCK_STREAM, 0);
  if (d.wrapper < 0)
  {
     perror("[tdeinit] Aborting. socket() failed: ");
     exit(255);
  }

  options = fcntl(d.wrapper, F_GETFL);
  if (options == -1)
  {
     perror("[tdeinit] Aborting. Can't make socket non-blocking: ");
     close(d.wrapper);
     exit(255);
  }

  if (fcntl(d.wrapper, F_SETFL, options | O_NONBLOCK) == -1)
  {
     perror("[tdeinit] Aborting. Can't make socket non-blocking: ");
     close(d.wrapper);
     exit(255);
  }

  while (1) {
      /** bind it **/
      socklen = sizeof(sa);
      memset(&sa, 0, socklen);
      sa.sun_family = AF_UNIX;
      strcpy(sa.sun_path, sock_file);
      if(bind(d.wrapper, (struct sockaddr *)&sa, socklen) != 0)
      {
          if (max_tries == 0) {
	      perror("[tdeinit] Aborting. bind() failed: ");
	      fprintf(stderr, "Could not bind to socket '%s'\n", sock_file);
	      close(d.wrapper);
	      exit(255);
	  }
	  max_tries--;
      } else
          break;
  }

  /** set permissions **/
  if (chmod(sock_file, 0600) != 0)
  {
     perror("[tdeinit] Aborting. Can't set permissions on socket: ");
     fprintf(stderr, "Wrong permissions of socket '%s'\n", sock_file);
     unlink(sock_file);
     close(d.wrapper);
     exit(255);
  }

  if(listen(d.wrapper, SOMAXCONN) < 0)
  {
     perror("[tdeinit] Aborting. listen() failed: ");
     unlink(sock_file);
     close(d.wrapper);
     exit(255);
  }

  /** create compatibility socket **/
  d.wrapper_old = socket(PF_UNIX, SOCK_STREAM, 0);
  if (d.wrapper_old < 0)
  {
     // perror("[tdeinit] Aborting. socket() failed: ");
     return;
  }

  options = fcntl(d.wrapper_old, F_GETFL);
  if (options == -1)
  {
     // perror("[tdeinit] Aborting. Can't make socket non-blocking: ");
     close(d.wrapper_old);
     d.wrapper_old = 0;
     return;
  }

  if (fcntl(d.wrapper_old, F_SETFL, options | O_NONBLOCK) == -1)
  {
     // perror("[tdeinit] Aborting. Can't make socket non-blocking: ");
     close(d.wrapper_old);
     d.wrapper_old = 0;
     return;
  }

  max_tries = 10;
  while (1) {
      /** bind it **/
      socklen = sizeof(sa_old);
      memset(&sa_old, 0, socklen);
      sa_old.sun_family = AF_UNIX;
      strcpy(sa_old.sun_path, sock_file_old);
      if(bind(d.wrapper_old, (struct sockaddr *)&sa_old, socklen) != 0)
      {
          if (max_tries == 0) {
	      // perror("[tdeinit] Aborting. bind() failed: ");
	      fprintf(stderr, "Could not bind to socket '%s'\n", sock_file_old);
	      close(d.wrapper_old);
	      d.wrapper_old = 0;
	      return;
	  }
	  max_tries--;
      } else
          break;
  }

  /** set permissions **/
  if (chmod(sock_file_old, 0600) != 0)
  {
     fprintf(stderr, "Wrong permissions of socket '%s'\n", sock_file);
     unlink(sock_file_old);
     close(d.wrapper_old);
     d.wrapper_old = 0;
     return;
  }

  if(listen(d.wrapper_old, SOMAXCONN) < 0)
  {
     // perror("[tdeinit] Aborting. listen() failed: ");
     unlink(sock_file_old);
     close(d.wrapper_old);
     d.wrapper_old = 0;
  }
}

/*
 * Read 'len' bytes from 'sock' into buffer.
 * returns 0 on success, -1 on failure.
 */
static int read_socket(int sock, char *buffer, int len)
{
  ssize_t result;
  int bytes_left = len;
  while ( bytes_left > 0)
  {
     result = read(sock, buffer, bytes_left);
     if (result > 0)
     {
        buffer += result;
        bytes_left -= result;
     }
     else if (result == 0)
        return -1;
     else if ((result == -1) && (errno != EINTR) && (errno != EAGAIN))
        return -1;
  }
  return 0;
}

static void WaitPid( pid_t waitForPid)
{
  int result;
  while(1)
  {
    result = waitpid(waitForPid, &d.exit_status, 0);
    if ((result == -1) && (errno == ECHILD))
       return;
  }
}

static void launcher_died()
{
   if (!d.launcher_ok)
   {
      /* This is bad. */
      fprintf(stderr, "[tdeinit] Communication error with launcher. Exiting!\n");
      ::exit(255);
      return;
   }

   // TDELauncher died... restart
#ifndef NDEBUG
   fprintf(stderr, "[tdeinit] TDELauncher died unexpectedly.\n");
#endif
   // Make sure it's really dead.
   if (d.launcher_pid)
   {
      kill(d.launcher_pid, SIGKILL);
      sleep(1); // Give it some time
   }

   d.launcher_ok = false;
   d.launcher_pid = 0;
   close(d.launcher[0]);
   d.launcher[0] = -1;

   pid_t pid = launch( 1, "tdelauncher", 0 );
#ifndef NDEBUG
   fprintf(stderr, "[tdeinit] Relaunching TDELauncher, pid = %ld result = %d\n", (long) pid, d.result);
#endif
}

static void handle_launcher_request(int sock = -1)
{
   bool launcher = false;
   if (sock < 0)
   {
       sock = d.launcher[0];
       launcher = true;
   }

   tdelauncher_header request_header;
   char *request_data = 0L;
   int result = read_socket(sock, (char *) &request_header, sizeof(request_header));
   if (result != 0)
   {
      if (launcher)
         launcher_died();
      return;
   }

   if ( request_header.arg_length != 0 )
   {
       request_data = (char *) malloc(request_header.arg_length);

       result = read_socket(sock, request_data, request_header.arg_length);
       if (result != 0)
       {
           if (launcher)
               launcher_died();
           free(request_data);
           return;
       }
   }

   if (request_header.cmd == LAUNCHER_OK)
   {
      d.launcher_ok = true;
   }
   else if (request_header.arg_length && 
      ((request_header.cmd == LAUNCHER_EXEC) ||
       (request_header.cmd == LAUNCHER_EXT_EXEC) ||
       (request_header.cmd == LAUNCHER_SHELL ) ||
       (request_header.cmd == LAUNCHER_KWRAPPER) ||
       (request_header.cmd == LAUNCHER_EXEC_NEW)))
   {
      pid_t pid;
      tdelauncher_header response_header;
      long response_data;
      long l;
      memcpy( &l, request_data, sizeof( long ));
      int argc = l;
      const char *name = request_data + sizeof(long);
      const char *args = name + strlen(name) + 1;
      const char *cwd = 0;
      int envc = 0;
      const char *envs = 0;
      const char *tty = 0;
      int avoid_loops = 0;
      const char *startup_id_str = "0";

#ifndef NDEBUG
     fprintf(stderr, "[tdeinit] Got %s '%s' from %s.\n",
        (request_header.cmd == LAUNCHER_EXEC ? "EXEC" :
        (request_header.cmd == LAUNCHER_EXT_EXEC ? "EXT_EXEC" :
        (request_header.cmd == LAUNCHER_EXEC_NEW ? "EXEC_NEW" :
        (request_header.cmd == LAUNCHER_SHELL ? "SHELL" : "KWRAPPER" )))),
         name, launcher ? "launcher" : "socket" );
#endif

      const char *arg_n = args;
      for(int i = 1; i < argc; i++)
      {
        arg_n = arg_n + strlen(arg_n) + 1;
      }

      if( request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER )
      {
         // Shell or kwrapper
         cwd = arg_n; arg_n += strlen(cwd) + 1;
      }
      if( request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER
          || request_header.cmd == LAUNCHER_EXT_EXEC || request_header.cmd == LAUNCHER_EXEC_NEW )
      {
         memcpy( &l, arg_n, sizeof( long ));
         envc = l;
         arg_n += sizeof(long);
         envs = arg_n;
         for(int i = 0; i < envc; i++)
         {
           arg_n = arg_n + strlen(arg_n) + 1;
         }
         if( request_header.cmd == LAUNCHER_KWRAPPER )
         {
             tty = arg_n;
             arg_n += strlen( tty ) + 1;
         }
      }

     if( request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER
         || request_header.cmd == LAUNCHER_EXT_EXEC || request_header.cmd == LAUNCHER_EXEC_NEW )
     {
         memcpy( &l, arg_n, sizeof( long ));
         avoid_loops = l;
         arg_n += sizeof( long );
     }

     if( request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER
         || request_header.cmd == LAUNCHER_EXT_EXEC )
     {
         startup_id_str = arg_n;
         arg_n += strlen( startup_id_str ) + 1;
     }

     if ((request_header.arg_length > (arg_n - request_data)) &&
         (request_header.cmd == LAUNCHER_EXT_EXEC || request_header.cmd == LAUNCHER_EXEC_NEW ))
     {
         // Optional cwd
         cwd = arg_n; arg_n += strlen(cwd) + 1;
     }

     if ((arg_n - request_data) != request_header.arg_length)
     {
#ifndef NDEBUG
       fprintf(stderr, "[tdeinit] EXEC request has invalid format.\n");
#endif
       free(request_data);
       d.debug_wait = false;
       return;
     }

      // support for the old a bit broken way of setting DISPLAY for multihead
      TQCString olddisplay = getenv(DISPLAY);
      TQCString kdedisplay = getenv("TDE_DISPLAY");
      bool reset_display = (! olddisplay.isEmpty() &&
                            ! kdedisplay.isEmpty() &&
                            olddisplay != kdedisplay);

      if (reset_display)
          setenv(DISPLAY, kdedisplay, true);

      pid = launch( argc, name, args, cwd, envc, envs,
          request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER,
          tty, avoid_loops, startup_id_str );

      if (reset_display) {
          unsetenv("TDE_DISPLAY");
          setenv(DISPLAY, olddisplay, true);
      }

      if (pid && (d.result == 0))
      {
         response_header.cmd = LAUNCHER_OK;
         response_header.arg_length = sizeof(response_data);
         response_data = pid;
         write(sock, &response_header, sizeof(response_header));
         write(sock, &response_data, response_header.arg_length);
      }
      else
      {
         int l = d.errorMsg.length();
         if (l) l++; // Include trailing null.
         response_header.cmd = LAUNCHER_ERROR;
         response_header.arg_length = l;
         write(sock, &response_header, sizeof(response_header));
         if (l)
            write(sock, d.errorMsg.data(), l);
      }
      d.debug_wait = false;
   }
   else if (request_header.arg_length && request_header.cmd == LAUNCHER_SETENV)
   {
      const char *env_name;
      const char *env_value;
      env_name = request_data;
      env_value = env_name + strlen(env_name) + 1;

#ifndef NDEBUG
      if (launcher)
         fprintf(stderr, "[tdeinit] Got SETENV '%s=%s' from tdelauncher.\n", env_name, env_value);
      else
         fprintf(stderr, "[tdeinit] Got SETENV '%s=%s' from socket.\n", env_name, env_value);
#endif

      if ( request_header.arg_length !=
          (int) (strlen(env_name) + strlen(env_value) + 2))
      {
#ifndef NDEBUG
         fprintf(stderr, "[tdeinit] SETENV request has invalid format.\n");
#endif
         free(request_data);
         return;
      }
      setenv( env_name, env_value, 1);
   }
   else if (request_header.cmd == LAUNCHER_TERMINATE_KDE)
   {
#ifndef NDEBUG
       fprintf(stderr,"[tdeinit] Terminating Trinity.\n");
#endif
#ifdef TQ_WS_X11
       tdeinit_xio_errhandler( 0L );
#endif
   }
   else if (request_header.cmd == LAUNCHER_TERMINATE_TDEINIT)
   {
#ifndef NDEBUG
       fprintf(stderr,"[tdeinit] Killing tdeinit/tdelauncher.\n");
#endif
       if (d.launcher_pid)
          kill(d.launcher_pid, SIGTERM);
       if (d.my_pid)
          kill(d.my_pid, SIGTERM);
   }
   else if (request_header.cmd == LAUNCHER_DEBUG_WAIT)
   {
#ifndef NDEBUG
       fprintf(stderr,"[tdeinit] Debug wait activated.\n");
#endif
       d.debug_wait = true;
   }
   if (request_data)
       free(request_data);
}

static void handle_requests(pid_t waitForPid)
{
   int max_sock = d.wrapper;
   if (d.wrapper_old > max_sock)
      max_sock = d.wrapper_old;
   if (d.launcher_pid && (d.launcher[0] > max_sock))
      max_sock = d.launcher[0];
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
//#ifdef _WS_X11
   if (X11fd > max_sock)
      max_sock = X11fd;
#endif
   max_sock++;

   while(1)
   {
      fd_set rd_set;
      fd_set wr_set;
      fd_set e_set;
      int result;
      pid_t exit_pid;
      char c;

      /* Flush the pipe of death */
      while( read(d.deadpipe[0], &c, 1) == 1);

      /* Handle dying children */
      do {
        exit_pid = waitpid(-1, 0, WNOHANG);
        if (exit_pid > 0)
        {
// FIXME: This disabled fprintf might need to be reinstated when converting to kdDebug.
// #ifndef NDEBUG
//            fprintf(stderr, "[tdeinit] PID %ld terminated.\n", (long) exit_pid);
// #endif
           if (waitForPid && (exit_pid == waitForPid))
              return;

           if (d.launcher_pid)
           {
           // TODO send process died message
              tdelauncher_header request_header;
              long request_data[2];
              request_header.cmd = LAUNCHER_DIED;
              request_header.arg_length = sizeof(long) * 2;
              request_data[0] = exit_pid;
              request_data[1] = 0; /* not implemented yet */
              write(d.launcher[0], &request_header, sizeof(request_header));
              write(d.launcher[0], request_data, request_header.arg_length);
           }
        }
      }
      while( exit_pid > 0);

      FD_ZERO(&rd_set);
      FD_ZERO(&wr_set);
      FD_ZERO(&e_set);

      if (d.launcher_pid)
      {
         FD_SET(d.launcher[0], &rd_set);
      }
      FD_SET(d.wrapper, &rd_set);
      if (d.wrapper_old)
      {
         FD_SET(d.wrapper_old, &rd_set);
      }
      FD_SET(d.deadpipe[0], &rd_set);
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
//#ifdef TQ_WS_X11
      if(X11fd >= 0) FD_SET(X11fd, &rd_set);
#endif

      result = select(max_sock, &rd_set, &wr_set, &e_set, 0);

      /* Handle wrapper request */
      if ((result > 0) && (FD_ISSET(d.wrapper, &rd_set)))
      {
         struct sockaddr_un client;
         kde_socklen_t sClient = sizeof(client);
         int sock = accept(d.wrapper, (struct sockaddr *)&client, &sClient);
         if (sock >= 0)
         {
#if defined(TDEINIT_USE_XFT) && defined(TDEINIT_USE_FONTCONFIG)
            if( FcGetVersion() < 20390 && !FcConfigUptoDate(NULL))
               FcInitReinitialize();
#endif
            if (fork() == 0)
            {
                close_fds();
                reset_oom_protect();
                handle_launcher_request(sock);
                exit(255); /* Terminate process. */
            }
            close(sock);
         }
      }
      if ((result > 0) && (FD_ISSET(d.wrapper_old, &rd_set)))
      {
         struct sockaddr_un client;
         kde_socklen_t sClient = sizeof(client);
         int sock = accept(d.wrapper_old, (struct sockaddr *)&client, &sClient);
         if (sock >= 0)
         {
#if defined(TDEINIT_USE_XFT) && defined(TDEINIT_USE_FONTCONFIG)
            if( FcGetVersion() < 20390 && !FcConfigUptoDate(NULL))
               FcInitReinitialize();
#endif
            if (fork() == 0)
            {
                close_fds();
                reset_oom_protect();
                handle_launcher_request(sock);
                exit(255); /* Terminate process. */
            }
            close(sock);
         }
      }

      /* Handle launcher request */
      if ((result > 0) && (d.launcher_pid) && (FD_ISSET(d.launcher[0], &rd_set)))
      {
         handle_launcher_request();
         if (waitForPid == d.launcher_pid)
            return;
      }

//#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
#ifdef TQ_WS_X11
      /* Look for incoming X11 events */
      if((result > 0) && (X11fd >= 0))
      {
        if(FD_ISSET(X11fd,&rd_set))
        {
          if (X11display != 0) {
	    XEvent event_return;
	    while (XPending(X11display))
	      XNextEvent(X11display, &event_return);
	  }
        }
      }
#endif
   }
}

static void tdeinit_library_path()
{
   TQStringList ltdl_library_path =
     TQStringList::split(':', TQFile::decodeName(getenv("LTDL_LIBRARY_PATH")));
   TQStringList ld_library_path =
     TQStringList::split(':', TQFile::decodeName(getenv("LD_LIBRARY_PATH")));

   TQCString extra_path;
   TQStringList candidates = s_instance->dirs()->resourceDirs("lib");
   for (TQStringList::ConstIterator it = candidates.begin();
        it != candidates.end();
        it++)
   {
      TQString d = *it;
      if (ltdl_library_path.contains(d))
          continue;
      if (ld_library_path.contains(d))
          continue;
      if (d[d.length()-1] == '/')
      {
         d.truncate(d.length()-1);
         if (ltdl_library_path.contains(d))
            continue;
         if (ld_library_path.contains(d))
            continue;
      }
      if ((d == "/lib") || (d == "/usr/lib"))
         continue;

      TQCString dir = TQFile::encodeName(d);

      if (access(dir, R_OK))
          continue;

      if ( !extra_path.isEmpty())
         extra_path += ":";
      extra_path += dir;
   }

   if (lt_dlinit())
   {
      const char * ltdlError = lt_dlerror();
      fprintf(stderr, "[tdeinit] Can't initialize dynamic loading: %s\n", ltdlError != 0 ? ltdlError : "(null)" );
   }
   if (!extra_path.isEmpty())
      lt_dlsetsearchpath(extra_path.data());

   TQCString display = getenv(DISPLAY);
   if (display.isEmpty())
   {
     fprintf(stderr, "[tdeinit] Aborting. $" DISPLAY " is not set.\n");
     exit(255);
   }
   int i;
   if((i = display.findRev('.')) > display.findRev(':') && i >= 0)
     display.truncate(i);

   TQCString socketName = TQFile::encodeName(locateLocal("socket", TQString("tdeinit-%1").arg(TQString(display)), s_instance));
   if (socketName.length() >= MAX_SOCK_FILE)
   {
     fprintf(stderr, "[tdeinit] Aborting. Socket name will be too long:\n");
     fprintf(stderr, "         '%s'\n", socketName.data());
     exit(255);
   }
   strcpy(sock_file_old, socketName.data());

   display.replace(":","_");
   socketName = TQFile::encodeName(locateLocal("socket", TQString("tdeinit_%1").arg(TQString(display)), s_instance));
   if (socketName.length() >= MAX_SOCK_FILE)
   {
     fprintf(stderr, "[tdeinit] Aborting. Socket name will be too long:\n");
     fprintf(stderr, "         '%s'\n", socketName.data());
     exit(255);
   }
   strcpy(sock_file, socketName.data());
}

int tdeinit_xio_errhandler( Display *disp )
{
    // disp is 0L when KDE shuts down. We don't want those warnings then.

    if ( disp )
    tqWarning( "[tdeinit] Fatal IO error: client killed" );

    if (sock_file[0])
    {
      /** Delete any stale socket file **/
      unlink(sock_file);
    }
    if (sock_file_old[0])
    {
      /** Delete any stale socket file **/
      unlink(sock_file_old);
    }

    // Don't kill our children in suicide mode, they may still be in use
    if (d.suicide)
    {
       if (d.launcher_pid)
          kill(d.launcher_pid, SIGTERM);
      exit( 0 );
    }

    if ( disp )
    tqWarning( "[tdeinit] sending SIGHUP to children." );

    /* this should remove all children we started */
    signal(SIGHUP, SIG_IGN);
    kill(0, SIGHUP);

    sleep(2);

    if ( disp )
    tqWarning( "[tdeinit] sending SIGTERM to children." );

    /* and if they don't listen to us, this should work */
    signal(SIGTERM, SIG_IGN);
    kill(0, SIGTERM);

    if ( disp )
    tqWarning( "[tdeinit] Exit." );

    exit( 0 );
    return 0;
}

#ifdef TQ_WS_X11
int tdeinit_x_errhandler( Display *dpy, XErrorEvent *err )
{
#ifndef NDEBUG
    char errstr[256];
    // tdeinit almost doesn't use X, and therefore there shouldn't be any X error
    XGetErrorText( dpy, err->error_code, errstr, 256 );
    fprintf(stderr, "[tdeinit] TDE detected X Error: %s %d\n"
                    "         Major opcode: %d\n"
                    "         Minor opcode: %d\n"
                    "         Resource id:  0x%lx\n",
            errstr, err->error_code, err->request_code, err->minor_code, err->resourceid );
#else
    Q_UNUSED(dpy);
    Q_UNUSED(err);
#endif
    return 0;
}
#endif

//#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
#ifdef TQ_WS_X11
// needs to be done sooner than initXconnection() because of also opening
// another X connection for startup notification purposes
static void setupX()
{
    XInitThreads();
    XSetIOErrorHandler(tdeinit_xio_errhandler);
    XSetErrorHandler(tdeinit_x_errhandler);
}

// Borrowed from tdebase/kaudio/kaudioserver.cpp
static int initXconnection()
{
  X11display = XOpenDisplay(NULL);
  if ( X11display != 0 ) {
    XCreateSimpleWindow(X11display, DefaultRootWindow(X11display), 0,0,1,1, \
        0,
        BlackPixelOfScreen(DefaultScreenOfDisplay(X11display)),
        BlackPixelOfScreen(DefaultScreenOfDisplay(X11display)) );
#ifndef NDEBUG
    fprintf(stderr, "[tdeinit] Opened connection to %s\n", DisplayString(X11display));
#endif
    int fd = XConnectionNumber( X11display );
    int on = 1;
    (void) setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &on, (int) sizeof(on));
    return fd;
  } else
    fprintf(stderr, "[tdeinit] Can't connect to the X Server.\n" \
     "[tdeinit] Might not terminate at end of session.\n");

  return -1;
}
#endif

#ifdef __KCC
/* One of my horrible hacks.  KCC includes in each "main" function a call
   to _main(), which is provided by the C++ runtime system.  It is
   responsible for calling constructors for some static objects.  That must
   be done only once, so _main() is guarded against multiple calls.
   For unknown reasons the designers of KAI's libKCC decided it would be
   a good idea to actually abort() when it's called multiple times, instead
   of ignoring further calls.  This breaks our mechanism of KLM's, because
   most KLM's have a main() function which is called from us.
   The "solution" is to simply define our own _main(), which ignores multiple
   calls, which is easy, and which does the same work as KAI'c _main(),
   which is difficult.  Currently (KAI 4.0f) it only calls __call_ctors(void)
   (a C++ function), but if that changes we need to change our's too.
   (matz) */
/*
 Those 'unknown reasons' are C++ standard forbidding recursive calls to main()
 or any means that would possibly allow that (e.g. taking address of main()).
 The correct solution is not using main() as entry point for tdeinit modules,
 but only kdemain().
*/
extern "C" void _main(void);
extern "C" void __call_ctors__Fv(void);
static int main_called = 0;
void _main(void)
{
  if (main_called)
    return;
  main_called = 1;
  __call_ctors__Fv ();
}
#endif

static void secondary_child_handler(int)
{
   waitpid(-1, 0, WNOHANG);
}

int main(int argc, char **argv, char **envp)
{
   int i;
   pid_t pid;
   int launch_dcop = 1;
   int launch_tdelauncher = 1;
   int launch_kded = 1;
   int keep_running = 1;
   int new_startup = 0;
   d.suicide = false;

   /** Save arguments first... **/
   char **safe_argv = (char **) malloc( sizeof(char *) * argc);
   for(i = 0; i < argc; i++)
   {
      safe_argv[i] = strcpy((char*)malloc(strlen(argv[i])+1), argv[i]);
      if (strcmp(safe_argv[i], "--no-dcop") == 0)
         launch_dcop = 0;
      if (strcmp(safe_argv[i], "--no-tdelauncher") == 0)
         launch_tdelauncher = 0;
      if (strcmp(safe_argv[i], "--no-kded") == 0)
         launch_kded = 0;
      if (strcmp(safe_argv[i], "--suicide") == 0)
         d.suicide = true;
      if (strcmp(safe_argv[i], "--exit") == 0)
         keep_running = 0;
      if (strcmp(safe_argv[i], "--new-startup") == 0)
         new_startup = 1;
#ifdef TDEINIT_OOM_PROTECT
      if (strcmp(safe_argv[i], "--oom-pipe") == 0 && i+1<argc)
         oom_pipe = atol(argv[i+1]);
#endif
      if (strcmp(safe_argv[i], "--help") == 0)
      {
        printf("Usage: tdeinit [options]\n");
     // printf("    --no-dcop         Do not start dcopserver\n");
     // printf("    --no-tdelauncher    Do not start tdelauncher\n");
        printf("    --no-kded         Do not start kded\n");
        printf("    --suicide         Terminate when no TDE applications are left running\n");
     // printf("    --exit            Terminate when kded has run\n");
        exit(0);
      }
   }

   pipe(d.initpipe);

   // Fork here and let parent process exit.
   // Parent process may only exit after all required services have been
   // launched. (dcopserver/tdelauncher and services which start with '+')
   signal( SIGCHLD, secondary_child_handler);
   if (fork() > 0) // Go into background
   {
      close(d.initpipe[1]);
      d.initpipe[1] = -1;
      // wait till init is complete
      char c;
      while( read(d.initpipe[0], &c, 1) < 0);
      // then exit;
      close(d.initpipe[0]);
      d.initpipe[0] = -1;
      return 0;
   }
   close(d.initpipe[0]);
   d.initpipe[0] = -1;
   d.my_pid = getpid();

   /** Make process group leader (for shutting down children later) **/
   if(keep_running)
      setsid();

   /** Create our instance **/
   s_instance = new TDEInstance("tdeinit");

   /** Prepare to change process name **/
   tdeinit_initsetproctitle(argc, argv, envp);
   tdeinit_library_path();
   // Don't make our instance the global instance
   // (do it only after tdeinit_library_path, that one indirectly uses TDEConfig,
   // which seems to be buggy and always use TDEGlobal instead of the maching TDEInstance)
   TDEGlobal::_instance = 0L;
   // don't change envvars before tdeinit_initsetproctitle()
   unsetenv("LD_BIND_NOW");
   unsetenv("DYLD_BIND_AT_LAUNCH");
   TDEApplication::loadedByKdeinit = true;

   d.maxname = strlen(argv[0]);
   d.launcher_pid = 0;
   d.wrapper = 0;
   d.wrapper_old = 0;
   d.debug_wait = false;
   d.launcher_ok = false;
   d.lt_dlopen_flag = lt_dlopen_flag;
   lt_dlopen_flag |= LTDL_GLOBAL;
   init_signals();
#ifdef TQ_WS_X11
   setupX();
#endif

   if (keep_running)
   {
      /*
       * Create ~/.trinity/tmp-<hostname>/tdeinit-<display> socket for incoming wrapper
       * requests.
       */
      init_tdeinit_socket();
   }

   if (launch_dcop)
   {
      if (d.suicide)
         pid = launch( 3, "dcopserver", "--nosid\0--suicide" );
      else
         pid = launch( 2, "dcopserver", "--nosid" );
#ifndef NDEBUG
      fprintf(stderr, "[tdeinit] Launched DCOPServer, pid = %ld result = %d\n", (long) pid, d.result);
#endif
      WaitPid(pid);
      if (!WIFEXITED(d.exit_status) || (WEXITSTATUS(d.exit_status) != 0))
      {
         fprintf(stderr, "[tdeinit] DCOPServer could not be started, aborting.\n");
         exit(1);
      }
   }
#ifndef __CYGWIN__
   if (!d.suicide && !getenv("TDE_IS_PRELINKED"))
   {
      TQString konq = locate("lib", "libkonq.la", s_instance);
      if (!konq.isEmpty())
	  (void) lt_dlopen(TQFile::encodeName(konq).data());
   }
#endif 
   if (launch_tdelauncher)
   {
      if( new_startup )
         pid = launch( 2, "tdelauncher", "--new-startup" );
      else
         pid = launch( 1, "tdelauncher", 0 );
#ifndef NDEBUG
      fprintf(stderr, "[tdeinit] Launched TDELauncher, pid = %ld result = %d\n", (long) pid, d.result);
#endif
      handle_requests(pid); // Wait for tdelauncher to be ready
   }
   
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
//#ifdef TQ_WS_X11
   X11fd = initXconnection();
#endif

   {
#if defined(TDEINIT_USE_XFT) && defined(TDEINIT_USE_FONTCONFIG)
      if( FcGetVersion() < 20390 )
      {
        XftInit(0);
        XftInitFtLibrary();
      }
#endif
      TQFont::initialize();
      setlocale (LC_ALL, "");
      setlocale (LC_NUMERIC, "C");
#ifdef TQ_WS_X11
      if (XSupportsLocale ())
      {
         // Similar to TQApplication::create_xim()
	 // but we need to use our own display
	 XOpenIM (X11display, 0, 0, 0);
      }
#endif
   }

   if (launch_kded)
   {
      if( new_startup )
         pid = launch( 2, "kded", "--new-startup" );
      else
         pid = launch( 1, "kded", 0 );
#ifndef NDEBUG
      fprintf(stderr, "[tdeinit] Launched KDED, pid = %ld result = %d\n", (long) pid, d.result);
#endif
      handle_requests(pid);
   }

   for(i = 1; i < argc; i++)
   {
      if (safe_argv[i][0] == '+')
      {
         pid = launch( 1, safe_argv[i]+1, 0);
#ifndef NDEBUG
      fprintf(stderr, "[tdeinit] Launched '%s', pid = %ld result = %d\n", safe_argv[i]+1, (long) pid, d.result);
#endif
         handle_requests(pid);
      }
      else if (safe_argv[i][0] == '-'
#ifdef TDEINIT_OOM_PROTECT
          || isdigit(safe_argv[i][0])
#endif
          )
      {
         // Ignore
      }
      else
      {
         pid = launch( 1, safe_argv[i], 0 );
#ifndef NDEBUG
      fprintf(stderr, "[tdeinit] Launched '%s', pid = %ld result = %d\n", safe_argv[i], (long) pid, d.result);
#endif
      }
   }

   /** Free arguments **/
   for(i = 0; i < argc; i++)
   {
      free(safe_argv[i]);
   }
   free (safe_argv);

   tdeinit_setproctitle("[tdeinit] tdeinit Running...");

   if (!keep_running)
      return 0;

   char c = 0;
   write(d.initpipe[1], &c, 1); // Kdeinit is started.
   close(d.initpipe[1]);
   d.initpipe[1] = -1;

   handle_requests(0);

   return 0;
}

