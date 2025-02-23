/*
  This file is part of the KDE libraries
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>

#include <tqfile.h>

#include <tdeconfig.h>
#include <kdebug.h>
#include <klibloader.h>
#include <tdelocale.h>
#include <tdeprotocolmanager.h>
#include <kprotocolinfo.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <tdetempfile.h>
#include <kurl.h>

#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
#include <tdestartupinfo.h> // schroder
#endif


#include "tdeio/global.h"
#include "tdeio/connection.h"
#include "tdeio/slaveinterface.h"

#include "tdelauncher.h"
#include "tdelauncher_cmds.h"

//#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
#ifdef TQ_WS_X11
//#undef K_WS_QTONLY
#include <X11/Xlib.h> // schroder
#endif

// Dispose slaves after being idle for SLAVE_MAX_IDLE seconds
#define SLAVE_MAX_IDLE	30

using namespace TDEIO;

template class TQPtrList<TDELaunchRequest>;
template class TQPtrList<IdleSlave>;

IdleSlave::IdleSlave(TDESocket *socket)
{
   mConn.init(socket);
   mConn.connect(this, TQ_SLOT(gotInput()));
   mConn.send( CMD_SLAVE_STATUS );
   mPid = 0;
   mBirthDate = time(0);
   mOnHold = false;
}

void
IdleSlave::gotInput()
{
   int cmd;
   TQByteArray data;
   if (mConn.read( &cmd, data) == -1)
   {
      // Communication problem with slave.
      kdError(7016) << "SlavePool: No communication with slave." << endl;
      delete this;
   }
   else if (cmd == MSG_SLAVE_ACK)
   {
      delete this;
   }
   else if (cmd != MSG_SLAVE_STATUS)
   {
      kdError(7016) << "SlavePool: Unexpected data from slave." << endl;
      delete this;
   }
   else
   {
      TQDataStream stream( data, IO_ReadOnly );
      pid_t pid;
      TQCString protocol;
      TQString host;
      TQ_INT8 b;
      stream >> pid >> protocol >> host >> b;
// Overload with (bool) onHold, (KURL) url.
      if (!stream.atEnd())
      {
         KURL url;
         stream >> url;
         mOnHold = true;
         mUrl = url;
      }

      mPid = pid;
      mConnected = (b != 0);
      mProtocol = protocol;
      mHost = host;
      emit statusUpdate(this);
   }
}

void
IdleSlave::connect(const TQString &app_socket)
{
   TQByteArray data;
   TQDataStream stream( data, IO_WriteOnly);
   stream << app_socket;
   mConn.send( CMD_SLAVE_CONNECT, data );
   // Timeout!
}

void
IdleSlave::reparseConfiguration()
{
   mConn.send( CMD_REPARSECONFIGURATION );
}

bool
IdleSlave::match(const TQString &protocol, const TQString &host, bool connected)
{
   if (mOnHold) return false;
   if (protocol != mProtocol) return false;
   if (host.isEmpty()) return true;
   if (host != mHost) return false;
   if (!connected) return true;
   if (!mConnected) return false;
   return true;
}

bool
IdleSlave::onHold(const KURL &url)
{
   if (!mOnHold) return false;
   return (url == mUrl);
}

int
IdleSlave::age(time_t now)
{
   return (int) difftime(now, mBirthDate);
}

TDELauncher::TDELauncher(int _tdeinitSocket, bool new_startup)
//  : TDEApplication( false, false ), // No Styles, No GUI
  : TDEApplication( false, true ),	// TQClipboard tries to construct a TQWidget so a GUI is technically needed, even though it is not used
    DCOPObject("tdelauncher"),
    tdeinitSocket(_tdeinitSocket), mAutoStart( new_startup ),
    dontBlockReading(false), newStartup( new_startup )
{
#ifdef TQ_WS_X11
   mCached_dpy = NULL;
#endif
   connect(&mAutoTimer, TQ_SIGNAL(timeout()), this, TQ_SLOT(slotAutoStart()));
   requestList.setAutoDelete(true);
   mSlaveWaitRequest.setAutoDelete(true);
   dcopClient()->setNotifications( true );
   connect(dcopClient(), TQ_SIGNAL( applicationRegistered( const TQCString &)),
           this, TQ_SLOT( slotAppRegistered( const TQCString &)));
   dcopClient()->connectDCOPSignal( "DCOPServer", "", "terminateTDE()",
                                    objId(), "terminateTDE()", false );

   TQString prefix = locateLocal("socket", "tdelauncher");
   KTempFile domainname(prefix, TQString::fromLatin1(".slave-socket"));
   if (domainname.status() != 0)
   {
      // Sever error!
      tqDebug("TDELauncher: Fatal error, can't create tempfile!");
      ::exit(1);
   }
   mPoolSocketName = domainname.name();
#ifdef __CYGWIN__
   domainname.close();
   domainname.unlink();
#endif
   mPoolSocket = new TDEServerSocket(static_cast<const char*>(TQFile::encodeName(mPoolSocketName)));
   connect(mPoolSocket, TQ_SIGNAL(accepted( TDESocket *)),
           TQ_SLOT(acceptSlave(TDESocket *)));

   connect(&mTimer, TQ_SIGNAL(timeout()), TQ_SLOT(idleTimeout()));

   tdeinitNotifier = new TQSocketNotifier(tdeinitSocket, TQSocketNotifier::Read);
   connect(tdeinitNotifier, TQ_SIGNAL( activated( int )),
           this, TQ_SLOT( slotKDEInitData( int )));
   tdeinitNotifier->setEnabled( true );
   lastRequest = 0;
   bProcessingQueue = false;

   mSlaveDebug = getenv("TDE_SLAVE_DEBUG_WAIT");
   if (!mSlaveDebug.isEmpty())
   {
      tqWarning("Klauncher running in slave-debug mode for slaves of protocol '%s'", mSlaveDebug.data());
   }
   mSlaveValgrind = getenv("TDE_SLAVE_VALGRIND");
   if (!mSlaveValgrind.isEmpty())
   {
      mSlaveValgrindSkin = getenv("TDE_SLAVE_VALGRIND_SKIN");
      tqWarning("Klauncher running slaves through valgrind for slaves of protocol '%s'", mSlaveValgrind.data());
   }
   tdelauncher_header request_header;
   request_header.cmd = LAUNCHER_OK;
   request_header.arg_length = 0;
   write(tdeinitSocket, &request_header, sizeof(request_header));
}

TDELauncher::~TDELauncher()
{
   close();
}

void TDELauncher::close()
{
   if (!mPoolSocketName.isEmpty())
   {
      TQCString filename = TQFile::encodeName(mPoolSocketName);
      unlink(filename.data());
   }
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
//#ifdef TQ_WS_X11
   if( mCached_dpy != NULL )
       XCloseDisplay( mCached_dpy );
#endif
}

void
TDELauncher::destruct(int exit_code)
{
   if (kapp) ((TDELauncher*)kapp)->close();
   // We don't delete kapp here, that's intentional.
   ::exit(exit_code);
}

bool
TDELauncher::process(const TQCString &fun, const TQByteArray &data,
                   TQCString &replyType, TQByteArray &replyData)
{
   if ((fun == "exec_blind(TQCString,TQValueList<TQCString>)")
       || (fun == "exec_blind(TQCString,TQValueList<TQCString>,TQValueList<TQCString>,TQCString)"))
   {
      TQDataStream stream(data, IO_ReadOnly);
      replyType = "void";
      TQCString name;
      TQValueList<TQCString> arg_list;
      TQCString startup_id = "0";
      TQValueList<TQCString> envs;
      stream >> name >> arg_list;
      if( fun == "exec_blind(TQCString,TQValueList<TQCString>,TQValueList<TQCString>,TQCString)" )
          stream >> envs >> startup_id;
      kdDebug(7016) << "TDELauncher: Got exec_blind('" << name << "', ...)" << endl;
      exec_blind( name, arg_list, envs, startup_id);
      return true;
   }
   if ((fun == "start_service_by_name(TQString,TQStringList)") ||
       (fun == "start_service_by_desktop_path(TQString,TQStringList)")||
       (fun == "start_service_by_desktop_name(TQString,TQStringList)")||
       (fun == "tdeinit_exec(TQString,TQStringList)") ||
       (fun == "tdeinit_exec_wait(TQString,TQStringList)") ||
       (fun == "start_service_by_name(TQString,TQStringList,TQValueList<TQCString>,TQCString)") ||
       (fun == "start_service_by_desktop_path(TQString,TQStringList,TQValueList<TQCString>,TQCString)")||
       (fun == "start_service_by_desktop_name(TQString,TQStringList,TQValueList<TQCString>,TQCString)") ||
       (fun == "start_service_by_name(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)") ||
       (fun == "start_service_by_desktop_path(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)")||
       (fun == "start_service_by_desktop_name(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)") ||
       (fun == "tdeinit_exec(TQString,TQStringList,TQValueList<TQCString>)") ||
       (fun == "tdeinit_exec_wait(TQString,TQStringList,TQValueList<TQCString>)") ||
       (fun == "tdeinit_exec(TQString,TQStringList,TQValueList<TQCString>,TQCString)") ||
       (fun == "tdeinit_exec_wait(TQString,TQStringList,TQValueList<TQCString>,TQCString)"))
   {
      TQDataStream stream(data, IO_ReadOnly);
      bool bNoWait = false;
      TQString serviceName;
      TQStringList urls;
      TQValueList<TQCString> envs;
      TQCString startup_id = "";
      DCOPresult.result = -1;
      DCOPresult.dcopName = 0;
      DCOPresult.error = TQString::null;
      DCOPresult.pid = 0;
      stream >> serviceName >> urls;
      if ((fun == "start_service_by_name(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)") ||
          (fun == "start_service_by_desktop_path(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)")||
          (fun == "start_service_by_desktop_name(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)"))
         stream >> envs >> startup_id >> bNoWait;
      else if ((fun == "start_service_by_name(TQString,TQStringList,TQValueList<TQCString>,TQCString)") ||
          (fun == "start_service_by_desktop_path(TQString,TQStringList,TQValueList<TQCString>,TQCString)")||
          (fun == "start_service_by_desktop_name(TQString,TQStringList,TQValueList<TQCString>,TQCString)"))
         stream >> envs >> startup_id;
      else if ((fun == "tdeinit_exec(TQString,TQStringList,TQValueList<TQCString>)") ||
          (fun == "tdeinit_exec_wait(TQString,TQStringList,TQValueList<TQCString>)"))
         stream >> envs;
      else if ((fun == "tdeinit_exec(TQString,TQStringList,TQValueList<TQCString>,TQCString)") ||
          (fun == "tdeinit_exec_wait(TQString,TQStringList,TQValueList<TQCString>,TQCString)"))
         stream >> envs >> startup_id;
      bool finished;
      if (strncmp(fun, "start_service_by_name(", 22) == 0)
      {
         kdDebug(7016) << "TDELauncher: Got start_service_by_name('" << serviceName << "', ...)" << endl;
         finished = start_service_by_name(serviceName, urls, envs, startup_id, bNoWait);
      }
      else if (strncmp(fun, "start_service_by_desktop_path(", 30) == 0)
      {
         kdDebug(7016) << "TDELauncher: Got start_service_by_desktop_path('" << serviceName << "', ...)" << endl;
         finished = start_service_by_desktop_path(serviceName, urls, envs, startup_id, bNoWait);
      }
      else if (strncmp(fun, "start_service_by_desktop_name(", 30) == 0)
      {
         kdDebug(7016) << "TDELauncher: Got start_service_by_desktop_name('" << serviceName << "', ...)" << endl;
         finished = start_service_by_desktop_name(serviceName, urls, envs, startup_id, bNoWait );
      }
      else if ((fun == "tdeinit_exec(TQString,TQStringList)")
              || (fun == "tdeinit_exec(TQString,TQStringList,TQValueList<TQCString>)")
              || (fun == "tdeinit_exec(TQString,TQStringList,TQValueList<TQCString>,TQCString)"))
      {
         kdDebug(7016) << "TDELauncher: Got tdeinit_exec('" << serviceName << "', ...)" << endl;
         finished = tdeinit_exec(serviceName, urls, envs, startup_id, false);
      }
      else
      {
         kdDebug(7016) << "TDELauncher: Got tdeinit_exec_wait('" << serviceName << "', ...)" << endl;
         finished = tdeinit_exec(serviceName, urls, envs, startup_id, true);
      }
      if (!finished)
      {
         replyType = "serviceResult";
         TQDataStream stream2(replyData, IO_WriteOnly);
         stream2 << DCOPresult.result << DCOPresult.dcopName << DCOPresult.error << DCOPresult.pid;
      }
      return true;
   }
   else if (fun == "requestSlave(TQString,TQString,TQString)")
   {
      TQDataStream stream(data, IO_ReadOnly);
      TQString protocol;
      TQString host;
      TQString app_socket;
      stream >> protocol >> host >> app_socket;
      replyType = "TQString";
      TQString error;
      pid_t pid = requestSlave(protocol, host, app_socket, error);
      TQDataStream stream2(replyData, IO_WriteOnly);
      stream2 << pid << error;
      return true;
   }
   else if (fun == "requestHoldSlave(KURL,TQString)")
   {
      TQDataStream stream(data, IO_ReadOnly);
      KURL url;
      TQString app_socket;
      stream >> url >> app_socket;
      replyType = "pid_t";
      pid_t pid = requestHoldSlave(url, app_socket);
      TQDataStream stream2(replyData, IO_WriteOnly);
      stream2 << pid;
      return true;
   }
   else if (fun == "waitForSlave(pid_t)")
   {
      TQDataStream stream(data, IO_ReadOnly);
      pid_t pid;
      stream >> pid;
      waitForSlave(pid);
      replyType = "void";
      return true;

   }
   else if (fun == "setLaunchEnv(TQCString,TQCString)")
   {
      TQDataStream stream(data, IO_ReadOnly);
      TQCString name;
      TQCString value;
      stream >> name >> value;
      setLaunchEnv(name, value);
      replyType = "void";
      return true;
   }
   else if (fun == "reparseConfiguration()")
   {
      TDEGlobal::config()->reparseConfiguration();
      kdDebug(7016) << "TDELauncher::process : reparseConfiguration" << endl;
      KProtocolManager::reparseConfiguration();
      IdleSlave *slave;
      for(slave = mSlaveList.first(); slave; slave = mSlaveList.next())
          slave->reparseConfiguration();
      replyType = "void";
      return true;
   }
   else if (fun == "terminateTDE()")
   {
      ::signal( SIGHUP, SIG_IGN);
      ::signal( SIGTERM, SIG_IGN);
      kdDebug() << "TDELauncher::process ---> terminateTDE" << endl;
      tdelauncher_header request_header;
      request_header.cmd = LAUNCHER_TERMINATE_KDE;
      request_header.arg_length = 0;
      write(tdeinitSocket, &request_header, sizeof(request_header));
      destruct(0);
   }
   else if (fun == "autoStart()")
   {
      kdDebug() << "TDELauncher::process ---> autoStart" << endl;
      autoStart(1);
      replyType = "void";
      return true;
   }
   else if (fun == "autoStart(int)")
   {
      kdDebug() << "TDELauncher::process ---> autoStart(int)" << endl;
      TQDataStream stream(data, IO_ReadOnly);
      int phase;
      stream >> phase;
      autoStart(phase);
      replyType = "void";
      return true;
   }

   if (DCOPObject::process(fun, data, replyType, replyData))
   {
      return true;
   }
   kdWarning(7016) << "Got unknown DCOP function: " << fun << endl;
   return false;
}

QCStringList
TDELauncher::interfaces()
{
    QCStringList ifaces = DCOPObject::interfaces();
    ifaces += "TDELauncher";
    return ifaces;
}

QCStringList
TDELauncher::functions()
{
    QCStringList funcs = DCOPObject::functions();
    funcs << "void exec_blind(TQCString,TQValueList<TQCString>)";
    funcs << "void exec_blind(TQCString,TQValueList<TQCString>,TQValueList<TQCString>,TQCString)";
    funcs << "serviceResult start_service_by_name(TQString,TQStringList)";
    funcs << "serviceResult start_service_by_desktop_path(TQString,TQStringList)";
    funcs << "serviceResult start_service_by_desktop_name(TQString,TQStringList)";
    funcs << "serviceResult tdeinit_exec(TQString,TQStringList)";
    funcs << "serviceResult tdeinit_exec_wait(TQString,TQStringList)";
    funcs << "serviceResult start_service_by_name(TQString,TQStringList,TQValueList<TQCString>,TQCString)";
    funcs << "serviceResult start_service_by_desktop_path(TQString,TQStringList,TQValueList<TQCString>,TQCString)";
    funcs << "serviceResult start_service_by_desktop_name(TQString,TQStringList,TQValueList<TQCString>,TQCString)";
    funcs << "serviceResult start_service_by_name(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)";
    funcs << "serviceResult start_service_by_desktop_path(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)";
    funcs << "serviceResult start_service_by_desktop_name(TQString,TQStringList,TQValueList<TQCString>,TQCString,bool)";
    funcs << "serviceResult tdeinit_exec(TQString,TQStringList,TQValueList<TQCString>)";
    funcs << "serviceResult tdeinit_exec_wait(TQString,TQStringList,TQValueList<TQCString>)";
    funcs << "TQString requestSlave(TQString,TQString,TQString)";
    funcs << "pid_t requestHoldSlave(KURL,TQString)";
    funcs << "void waitForSlave(pid_t)";
    funcs << "void setLaunchEnv(TQCString,TQCString)";
    funcs << "void reparseConfiguration()";
//    funcs << "void terminateTDE()";
    funcs << "void autoStart()";
    funcs << "void autoStart(int)";
    return funcs;
}

void TDELauncher::setLaunchEnv(const TQCString &name, const TQCString &_value)
{
   TQCString value(_value);
   if (value.isNull())
      value = "";
   tdelauncher_header request_header;
   TQByteArray requestData(name.length()+value.length()+2);
   memcpy(requestData.data(), name.data(), name.length()+1);
   memcpy(requestData.data()+name.length()+1, value.data(), value.length()+1);
   request_header.cmd = LAUNCHER_SETENV;
   request_header.arg_length = requestData.size();
   write(tdeinitSocket, &request_header, sizeof(request_header));
   write(tdeinitSocket, requestData.data(), request_header.arg_length);
}

/*
 * Read 'len' bytes from 'sock' into buffer.
 * returns -1 on failure, 0 on no data.
 */
static int
read_socket(int sock, char *buffer, int len)
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
     else if ((result == -1) && (errno != EINTR))
        return -1;
  }
  return 0;
}


void
TDELauncher::slotKDEInitData(int)
{
   tdelauncher_header request_header;
   TQByteArray requestData;
   if( dontBlockReading )
   {
   // in case we get a request to start an application and data arrive
   // to tdeinitSocket at the same time, requestStart() will already
   // call slotKDEInitData(), so we must check there's still something
   // to read, otherwise this would block
      fd_set in;
      timeval tm = { 0, 0 };
      FD_ZERO ( &in );
      FD_SET( tdeinitSocket, &in );
      select( tdeinitSocket + 1, &in, 0, 0, &tm );
      if( !FD_ISSET( tdeinitSocket, &in ))
         return;
   }
   dontBlockReading = false;
   int result = read_socket(tdeinitSocket, (char *) &request_header,
                            sizeof( request_header));
   if (result == -1)
   {
      kdDebug() << "Exiting on read_socket errno: " << errno << endl;
      ::signal( SIGHUP, SIG_IGN);
      ::signal( SIGTERM, SIG_IGN);
      destruct(255); // Exit!
   }
   requestData.resize(request_header.arg_length);
   result = read_socket(tdeinitSocket, (char *) requestData.data(),
                        request_header.arg_length);

   if (request_header.cmd == LAUNCHER_DIED)
   {
     long *request_data;
     request_data = (long *) requestData.data();
     processDied(request_data[0], request_data[1]);
     return;
   }
   if (lastRequest && (request_header.cmd == LAUNCHER_OK))
   {
     long *request_data;
     request_data = (long *) requestData.data();
     lastRequest->pid = (pid_t) (*request_data);
     kdDebug(7016) << lastRequest->name << " (pid " << lastRequest->pid <<
        ") up and running." << endl;
     switch(lastRequest->dcop_service_type)
     {
       case KService::DCOP_None:
       {
         lastRequest->status = TDELaunchRequest::Running;
         break;
       }

       case KService::DCOP_Unique:
       {
         lastRequest->status = TDELaunchRequest::Launching;
         break;
       }

       case KService::DCOP_Wait:
       {
         lastRequest->status = TDELaunchRequest::Launching;
         break;
       }

       case KService::DCOP_Multi:
       {
         lastRequest->status = TDELaunchRequest::Launching;
         break;
       }
     }
     lastRequest = 0;
     return;
   }
   if (lastRequest && (request_header.cmd == LAUNCHER_ERROR))
   {
     lastRequest->status = TDELaunchRequest::Error;
     if (!requestData.isEmpty())
        lastRequest->errorMsg = TQString::fromUtf8((char *) requestData.data());
     lastRequest = 0;
     return;
   }

   kdWarning(7016) << "Unexpected command from TDEInit (" << (unsigned int) request_header.cmd
                 << ")" << endl;
}

void
TDELauncher::processDied(pid_t pid, long /* exitStatus */)
{
   TDELaunchRequest *request = requestList.first();
   for(; request; request = requestList.next())
   {
      if (request->pid == pid)
      {
         if (request->dcop_service_type == KService::DCOP_Wait)
            request->status = TDELaunchRequest::Done;
         else if ((request->dcop_service_type == KService::DCOP_Unique) &&
		(dcopClient()->isApplicationRegistered(request->dcop_name)))
            request->status = TDELaunchRequest::Running;
         else
            request->status = TDELaunchRequest::Error;
         requestDone(request);
         return;
      }
   }
}

void
TDELauncher::slotAppRegistered(const TQCString &appId)
{
   const char *cAppId = appId.data();
   if (!cAppId) return;

   TDELaunchRequest *request = requestList.first();
   TDELaunchRequest *nextRequest;
   for(; request; request = nextRequest)
   {
      nextRequest = requestList.next();
      if (request->status != TDELaunchRequest::Launching)
         continue;

      // For unique services check the requested service name first
      if ((request->dcop_service_type == KService::DCOP_Unique) &&
          ((appId == request->dcop_name) ||
           dcopClient()->isApplicationRegistered(request->dcop_name)))
      {
         request->status = TDELaunchRequest::Running;
         requestDone(request);
         continue;
      }

      const char *rAppId = request->dcop_name.data();
      if (!rAppId) continue;

      int l = strlen(rAppId);
      if ((strncmp(rAppId, cAppId, l) == 0) &&
          ((cAppId[l] == '\0') || (cAppId[l] == '-')))
      {
         request->dcop_name = appId;
         request->status = TDELaunchRequest::Running;
         requestDone(request);
         continue;
      }
   }
}

void
TDELauncher::autoStart(int phase)
{
   if( mAutoStart.phase() >= phase )
       return;
   mAutoStart.setPhase(phase);
   if( newStartup )
   {
      if (phase == 0)
         mAutoStart.loadAutoStartList();
   }
   else
   {
      if (phase == 1)
         mAutoStart.loadAutoStartList();
   }
   mAutoTimer.start(0, true);
}

void
TDELauncher::slotAutoStart()
{
   KService::Ptr s;
   do
   {
      TQString service = mAutoStart.startService();
      if (service.isEmpty())
      {
         // Done
	 if( !mAutoStart.phaseDone())
	 {
	    mAutoStart.setPhaseDone();
	    // Emit signal
            if( newStartup )
            {
	       TQCString autoStartSignal;
               autoStartSignal.sprintf( "autoStart%dDone()", mAutoStart.phase());
               emitDCOPSignal(autoStartSignal, TQByteArray());
            }
            else
            {
	       TQCString autoStartSignal( "autoStartDone()" );
	       int phase = mAutoStart.phase();
	       if ( phase > 1 )
	           autoStartSignal.sprintf( "autoStart%dDone()", phase );
               emitDCOPSignal(autoStartSignal, TQByteArray());
            }
	 }
         return;
      }
      s = new KService(service);
   }
   while (!start_service(s, TQStringList(), TQValueList<TQCString>(), "0", false, true));
   // Loop till we find a service that we can start.
}

void
TDELauncher::requestDone(TDELaunchRequest *request)
{
   if ((request->status == TDELaunchRequest::Running) ||
       (request->status == TDELaunchRequest::Done))
   {
      DCOPresult.result = 0;
      DCOPresult.dcopName = request->dcop_name;
      DCOPresult.error = TQString::null;
      DCOPresult.pid = request->pid;
   }
   else
   {
      DCOPresult.result = 1;
      DCOPresult.dcopName = "";
      DCOPresult.error = i18n("TDEInit could not launch '%1'.").arg(TQString(request->name));
      if (!request->errorMsg.isEmpty())
         DCOPresult.error += ":\n" + request->errorMsg;
      DCOPresult.pid = 0;

#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
//#ifdef TQ_WS_X11
      if (!request->startup_dpy.isEmpty())
      {
         Display* dpy = NULL;
         if( (mCached_dpy != NULL) &&
              (request->startup_dpy == XDisplayString( mCached_dpy )))
            dpy = mCached_dpy;
         if( dpy == NULL )
            dpy = XOpenDisplay( request->startup_dpy );
         if( dpy )
         {
            TDEStartupInfoId id;
            id.initId( request->startup_id );
            TDEStartupInfo::sendFinishX( dpy, id );
            if( mCached_dpy != dpy && mCached_dpy != NULL )
               XCloseDisplay( mCached_dpy );
            mCached_dpy = dpy;
         }
      }
#endif
   }

   if (request->autoStart)
   {
      mAutoTimer.start(0, true);
   }

   if (request->transaction)
   {
      TQByteArray replyData;
      TQCString replyType;
      replyType = "serviceResult";
      TQDataStream stream2(replyData, IO_WriteOnly);
      stream2 << DCOPresult.result << DCOPresult.dcopName << DCOPresult.error << DCOPresult.pid;
      dcopClient()->endTransaction( request->transaction,
                                    replyType, replyData);
   }
   requestList.removeRef( request );
}

void
TDELauncher::requestStart(TDELaunchRequest *request)
{
   requestList.append( request );
   // Send request to tdeinit.
   tdelauncher_header request_header;
   TQByteArray requestData;
   int length = 0;
   length += sizeof(long); // Nr of. Args
   length += request->name.length() + 1; // Cmd
   for(TQValueList<TQCString>::Iterator it = request->arg_list.begin();
       it != request->arg_list.end();
       it++)
   {
      length += (*it).length() + 1; // Args...
   }
   length += sizeof(long); // Nr of. envs
   for(TQValueList<TQCString>::ConstIterator it = request->envs.begin();
       it != request->envs.end();
       it++)
   {
      length += (*it).length() + 1; // Envs...
   }
   length += sizeof( long ); // avoid_loops
#ifdef TQ_WS_X11
   bool startup_notify = !request->startup_id.isNull() && request->startup_id != "0";
   if( startup_notify )
       length += request->startup_id.length() + 1;
#endif
   if (!request->cwd.isEmpty())
       length += request->cwd.length() + 1;

   requestData.resize( length );

   char *p = requestData.data();
   long l = request->arg_list.count()+1;
   memcpy(p, &l, sizeof(long));
   p += sizeof(long);
   strcpy(p, request->name.data());
   p += strlen(p) + 1;
   for(TQValueList<TQCString>::Iterator it = request->arg_list.begin();
       it != request->arg_list.end();
       it++)
   {
      strcpy(p, (*it).data());
      p += strlen(p) + 1;
   }
   l = request->envs.count();
   memcpy(p, &l, sizeof(long));
   p += sizeof(long);
   for(TQValueList<TQCString>::ConstIterator it = request->envs.begin();
       it != request->envs.end();
       it++)
   {
      strcpy(p, (*it).data());
      p += strlen(p) + 1;
   }
   l = 0; // avoid_loops, always false here
   memcpy(p, &l, sizeof(long));
   p += sizeof(long);
#ifdef TQ_WS_X11
   if( startup_notify )
   {
      strcpy(p, request->startup_id.data());
      p += strlen( p ) + 1;
   }
#endif
   if (!request->cwd.isEmpty())
   {
      strcpy(p, request->cwd.data());
      p += strlen( p ) + 1;
   }
#ifdef TQ_WS_X11
   request_header.cmd = startup_notify ? LAUNCHER_EXT_EXEC : LAUNCHER_EXEC_NEW;
#else
   request_header.cmd = LAUNCHER_EXEC_NEW;
#endif
   request_header.arg_length = length;
   write(tdeinitSocket, &request_header, sizeof(request_header));
   write(tdeinitSocket, requestData.data(), request_header.arg_length);

   // Wait for pid to return.
   lastRequest = request;
   dontBlockReading = false;
   do {
      slotKDEInitData( tdeinitSocket );
   }
   while (lastRequest != 0);
   dontBlockReading = true;
}

void
TDELauncher::exec_blind( const TQCString &name, const TQValueList<TQCString> &arg_list,
    const TQValueList<TQCString> &envs, const TQCString& startup_id )
{
   TDELaunchRequest *request = new TDELaunchRequest;
   request->autoStart = false;
   request->name = name;
   request->arg_list =  arg_list;
   request->dcop_name = 0;
   request->dcop_service_type = KService::DCOP_None;
   request->pid = 0;
   request->status = TDELaunchRequest::Launching;
   request->transaction = 0; // No confirmation is send
   request->envs = envs;
   // Find service, if any - strip path if needed
   KService::Ptr service = KService::serviceByDesktopName( name.mid( name.findRev( '/' ) + 1 ));
   if (service != NULL)
       send_service_startup_info( request,  service,
           startup_id, TQValueList< TQCString >());
   else // no .desktop file, no startup info
       cancel_service_startup_info( request, startup_id, envs );

   requestStart(request);
   // We don't care about this request any longer....
   requestDone(request);
}


bool
TDELauncher::start_service_by_name(const TQString &serviceName, const TQStringList &urls,
    const TQValueList<TQCString> &envs, const TQCString& startup_id, bool blind)
{
   KService::Ptr service = 0;
   // Find service
   service = KService::serviceByName(serviceName);
   if (!service)
   {
      DCOPresult.result = ENOENT;
      DCOPresult.error = i18n("Could not find service '%1'.").arg(serviceName);
      cancel_service_startup_info( NULL, startup_id, envs ); // cancel it if any
      return false;
   }
   return start_service(service, urls, envs, startup_id, blind);
}

bool
TDELauncher::start_service_by_desktop_path(const TQString &serviceName, const TQStringList &urls,
    const TQValueList<TQCString> &envs, const TQCString& startup_id, bool blind)
{
   KService::Ptr service = 0;
   // Find service
   if (serviceName[0] == '/')
   {
      // Full path
      service = new KService(serviceName);
   }
   else
   {
      service = KService::serviceByDesktopPath(serviceName);
   }
   if (!service)
   {
      DCOPresult.result = ENOENT;
      DCOPresult.error = i18n("Could not find service '%1'.").arg(serviceName);
      cancel_service_startup_info( NULL, startup_id, envs ); // cancel it if any
      return false;
   }
   return start_service(service, urls, envs, startup_id, blind);
}

bool
TDELauncher::start_service_by_desktop_name(const TQString &serviceName, const TQStringList &urls,
    const TQValueList<TQCString> &envs, const TQCString& startup_id, bool blind)
{
   KService::Ptr service = 0;
   // Find service
   service = KService::serviceByDesktopName(serviceName);
   if (!service)
   {
      DCOPresult.result = ENOENT;
      DCOPresult.error = i18n("Could not find service '%1'.").arg(serviceName);
      cancel_service_startup_info( NULL, startup_id, envs ); // cancel it if any
      return false;
   }
   return start_service(service, urls, envs, startup_id, blind);
}

bool
TDELauncher::start_service(KService::Ptr service, const TQStringList &_urls,
    const TQValueList<TQCString> &envs, const TQCString& startup_id, bool blind, bool autoStart)
{
   TQStringList urls = _urls;
   if (!service->isValid())
   {
      DCOPresult.result = ENOEXEC;
      DCOPresult.error = i18n("Service '%1' is malformatted.").arg(service->desktopEntryPath());
      cancel_service_startup_info( NULL, startup_id, envs ); // cancel it if any
      return false;
   }
   TDELaunchRequest *request = new TDELaunchRequest;
   request->autoStart = autoStart;

   if ((urls.count() > 1) && !service->allowMultipleFiles())
   {
      // We need to launch the application N times. That sucks.
      // We ignore the result for application 2 to N.
      // For the first file we launch the application in the
      // usual way. The reported result is based on this
      // application.
      TQStringList::ConstIterator it = urls.begin();
      for(++it;
          it != urls.end();
          ++it)
      {
         TQStringList singleUrl;
         singleUrl.append(*it);
         TQCString startup_id2 = startup_id;
         if( !startup_id2.isEmpty() && startup_id2 != "0" )
             startup_id2 = "0"; // can't use the same startup_id several times
         start_service( service, singleUrl, envs, startup_id2, true);
      }
      TQString firstURL = *(urls.begin());
      urls.clear();
      urls.append(firstURL);
   }
   createArgs(request, service, urls);

   // We must have one argument at least!
   if (!request->arg_list.count())
   {
      DCOPresult.result = ENOEXEC;
      DCOPresult.error = i18n("Service '%1' is malformatted.").arg(service->desktopEntryPath());
      delete request;
      cancel_service_startup_info( NULL, startup_id, envs );
      return false;
   }

   request->name = request->arg_list.first();
   request->arg_list.remove(request->arg_list.begin());

   request->dcop_service_type =  service->DCOPServiceType();

   if ((request->dcop_service_type == KService::DCOP_Unique) ||
       (request->dcop_service_type == KService::DCOP_Multi))
   {
      TQVariant v = service->property("X-DCOP-ServiceName");
      if (v.isValid())
         request->dcop_name = v.toString().utf8();
      if (request->dcop_name.isEmpty())
      {
         request->dcop_name = TQFile::encodeName(KRun::binaryName(service->exec(), true));
      }
   }

   request->pid = 0;
   request->transaction = 0;
   request->envs = envs;
   send_service_startup_info( request, service, startup_id, envs );

   // Request will be handled later.
   if (!blind && !autoStart)
   {
      request->transaction = dcopClient()->beginTransaction();
   }
   queueRequest(request);
   return true;
}

void
TDELauncher::send_service_startup_info( TDELaunchRequest *request, KService::Ptr service, const TQCString& startup_id,
    const TQValueList<TQCString> &envs )
{
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
//#ifdef TQ_WS_X11 // TDEStartup* isn't implemented for Qt/Embedded yet
    request->startup_id = "0";
    if( startup_id == "0" )
        return;
    bool silent;
    TQCString wmclass;
    if( !KRun::checkStartupNotify( TQString::null, service, &silent, &wmclass ))
        return;
    TDEStartupInfoId id;
    id.initId( startup_id );
    const char* dpy_str = NULL;
    for( TQValueList<TQCString>::ConstIterator it = envs.begin();
         it != envs.end();
         ++it )
        if( strncmp( *it, "DISPLAY=", 8 ) == 0 )
            dpy_str = static_cast< const char* >( *it ) + 8;
    Display* dpy = NULL;
    if( dpy_str != NULL && mCached_dpy != NULL
        && qstrcmp( dpy_str, XDisplayString( mCached_dpy )) == 0 )
        dpy = mCached_dpy;
    if( dpy == NULL )
        dpy = XOpenDisplay( dpy_str );
    request->startup_id = id.id();
    if( dpy == NULL )
    {
        cancel_service_startup_info( request, startup_id, envs );
        return;
    }

    request->startup_dpy = dpy_str;

    TDEStartupInfoData data;
    data.setName( service->name());
    data.setIcon( service->icon());
    data.setDescription( i18n( "Launching %1" ).arg( service->name()));
    if( !wmclass.isEmpty())
        data.setWMClass( wmclass );
    if( silent )
        data.setSilent( TDEStartupInfoData::Yes );
    // the rest will be sent by tdeinit
    TDEStartupInfo::sendStartupX( dpy, id, data );
    if( mCached_dpy != dpy && mCached_dpy != NULL )
        XCloseDisplay( mCached_dpy );
    mCached_dpy = dpy;
    return;
#else
    return;
#endif
}

void
TDELauncher::cancel_service_startup_info( TDELaunchRequest* request, const TQCString& startup_id,
    const TQValueList<TQCString> &envs )
{
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
//#ifdef TQ_WS_X11 // TDEStartup* isn't implemented for Qt/Embedded yet
    if( request != NULL )
        request->startup_id = "0";
    if( !startup_id.isEmpty() && startup_id != "0" )
    {
        const char* dpy_str = NULL;
        for( TQValueList<TQCString>::ConstIterator it = envs.begin();
             it != envs.end();
             ++it )
            if( strncmp( *it, "DISPLAY=", 8 ) == 0 )
                dpy_str = static_cast< const char* >( *it ) + 8;
        Display* dpy = NULL;
        if( dpy_str != NULL && mCached_dpy != NULL
            && qstrcmp( dpy_str, XDisplayString( mCached_dpy )) == 0 )
            dpy = mCached_dpy;
        if( dpy == NULL )
            dpy = XOpenDisplay( dpy_str );
        if( dpy == NULL )
            return;
        TDEStartupInfoId id;
        id.initId( startup_id );
        TDEStartupInfo::sendFinishX( dpy, id );
        if( mCached_dpy != dpy && mCached_dpy != NULL )
           XCloseDisplay( mCached_dpy );
        mCached_dpy = dpy;
    }
#endif
}

bool
TDELauncher::tdeinit_exec(const TQString &app, const TQStringList &args,
   const TQValueList<TQCString> &envs, TQCString startup_id, bool wait)
{
   TDELaunchRequest *request = new TDELaunchRequest;
   request->autoStart = false;

   for(TQStringList::ConstIterator it = args.begin();
       it != args.end();
       it++)
   {
       TQString arg = *it;
       request->arg_list.append(arg.local8Bit());
   }

   request->name = app.local8Bit();

   if (wait)
      request->dcop_service_type = KService::DCOP_Wait;
   else
      request->dcop_service_type = KService::DCOP_None;
   request->dcop_name = 0;
   request->pid = 0;
#ifdef TQ_WS_X11
   request->startup_id = startup_id;
#endif
   request->envs = envs;
   if( app != "tdebuildsycoca" ) // avoid stupid loop
   {
       // Find service, if any - strip path if needed
       KService::Ptr service = KService::serviceByDesktopName( app.mid( app.findRev( '/' ) + 1 ));
       if (service != NULL)
           send_service_startup_info( request,  service,
               startup_id, TQValueList< TQCString >());
       else // no .desktop file, no startup info
           cancel_service_startup_info( request, startup_id, envs );
   }
   request->transaction = dcopClient()->beginTransaction();
   queueRequest(request);
   return true;
}

void
TDELauncher::queueRequest(TDELaunchRequest *request)
{
   requestQueue.append( request );
   if (!bProcessingQueue)
   {
      bProcessingQueue = true;
      TQTimer::singleShot(0, this, TQ_SLOT( slotDequeue() ));
   }
}

void
TDELauncher::slotDequeue()
{
   do {
      TDELaunchRequest *request = requestQueue.take(0);
      // process request
      request->status = TDELaunchRequest::Launching;
      requestStart(request);
      if (request->status != TDELaunchRequest::Launching)
      {
         // Request handled.
         requestDone( request );
         continue;
      }
   } while(requestQueue.count());
   bProcessingQueue = false;
}

void
TDELauncher::createArgs( TDELaunchRequest *request, const KService::Ptr service ,
                       const TQStringList &urls)
{
  TQStringList params = KRun::processDesktopExec(*service, urls, false);

  for(TQStringList::ConstIterator it = params.begin();
      it != params.end(); ++it)
  {
     request->arg_list.append((*it).local8Bit());
  }
  request->cwd = TQFile::encodeName(service->path());
}

///// IO-Slave functions

pid_t
TDELauncher::requestHoldSlave(const KURL &url, const TQString &app_socket)
{
    IdleSlave *slave;
    for(slave = mSlaveList.first(); slave; slave = mSlaveList.next())
    {
       if (slave->onHold(url))
          break;
    }
    if (slave)
    {
       mSlaveList.removeRef(slave);
       slave->connect(app_socket);
       return slave->pid();
    }
    return 0;
}


pid_t
TDELauncher::requestSlave(const TQString &protocol,
                        const TQString &host,
                        const TQString &app_socket,
                        TQString &error)
{
    IdleSlave *slave;
    for(slave = mSlaveList.first(); slave; slave = mSlaveList.next())
    {
       if (slave->match(protocol, host, true))
          break;
    }
    if (!slave)
    {
       for(slave = mSlaveList.first(); slave; slave = mSlaveList.next())
       {
          if (slave->match(protocol, host, false))
             break;
       }
    }
    if (!slave)
    {
       for(slave = mSlaveList.first(); slave; slave = mSlaveList.next())
       {
          if (slave->match(protocol, TQString::null, false))
             break;
       }
    }
    if (slave)
    {
       mSlaveList.removeRef(slave);
       slave->connect(app_socket);
       return slave->pid();
    }

    TQString _name = KProtocolInfo::exec(protocol);
    if (_name.isEmpty())
    {
	error = i18n("Unknown protocol '%1'.\n").arg(protocol);
        return 0;
    }

    TQCString name = _name.latin1(); // ex: "tdeio_ftp"
    TQCString arg1 = protocol.latin1();
    TQCString arg2 = TQFile::encodeName(mPoolSocketName);
    TQCString arg3 = TQFile::encodeName(app_socket);
    TQValueList<TQCString> arg_list;
    arg_list.append(arg1);
    arg_list.append(arg2);
    arg_list.append(arg3);

//    kdDebug(7016) << "TDELauncher: launching new slave " << _name << " with protocol=" << protocol << endl;
    if (mSlaveDebug == arg1)
    {
       tdelauncher_header request_header;
       request_header.cmd = LAUNCHER_DEBUG_WAIT;
       request_header.arg_length = 0;
       write(tdeinitSocket, &request_header, sizeof(request_header));
    }
    if (mSlaveValgrind == arg1)
    {
       arg_list.prepend(TQFile::encodeName(KLibLoader::findLibrary(name)));
       arg_list.prepend(TQFile::encodeName(locate("exe", "tdeioslave")));
       name = "valgrind";
       if (!mSlaveValgrindSkin.isEmpty()) {
           arg_list.prepend(TQCString("--tool=") + mSlaveValgrindSkin);
       } else
	   arg_list.prepend("--tool=memcheck");
    }

    TDELaunchRequest *request = new TDELaunchRequest;
    request->autoStart = false;
    request->name = name;
    request->arg_list =  arg_list;
    request->dcop_name = 0;
    request->dcop_service_type = KService::DCOP_None;
    request->pid = 0;
#ifdef TQ_WS_X11
    request->startup_id = "0";
#endif
    request->status = TDELaunchRequest::Launching;
    request->transaction = 0; // No confirmation is send
    requestStart(request);
    pid_t pid = request->pid;

//    kdDebug(7016) << "Slave launched, pid = " << pid << endl;

    // We don't care about this request any longer....
    requestDone(request);
    if (!pid)
    {
       error = i18n("Error loading '%1'.\n").arg(TQString(name));
    }
    return pid;
}

void
TDELauncher::waitForSlave(pid_t pid)
{
    IdleSlave *slave;
    for(slave = mSlaveList.first(); slave; slave = mSlaveList.next())
    {
        if (slave->pid() == pid)
           return; // Already here.
    }
    SlaveWaitRequest *waitRequest = new SlaveWaitRequest;
    waitRequest->transaction = dcopClient()->beginTransaction();
    waitRequest->pid = pid;
    mSlaveWaitRequest.append(waitRequest);
}

void
TDELauncher::acceptSlave(TDESocket *slaveSocket)
{
    IdleSlave *slave = new IdleSlave(slaveSocket);
    // Send it a SLAVE_STATUS command.
    mSlaveList.append(slave);
    connect(slave, TQ_SIGNAL(destroyed()), this, TQ_SLOT(slotSlaveGone()));
    connect(slave, TQ_SIGNAL(statusUpdate(IdleSlave *)),
	    this, TQ_SLOT(slotSlaveStatus(IdleSlave *)));
    if (!mTimer.isActive())
    {
       mTimer.start(1000*10);
    }
}

void
TDELauncher::slotSlaveStatus(IdleSlave *slave)
{
    SlaveWaitRequest *waitRequest = mSlaveWaitRequest.first();
    while(waitRequest)
    {
       if (waitRequest->pid == slave->pid())
       {
          TQByteArray replyData;
          TQCString replyType;
          replyType = "void";
          dcopClient()->endTransaction( waitRequest->transaction, replyType, replyData);
          mSlaveWaitRequest.removeRef(waitRequest);
          waitRequest = mSlaveWaitRequest.current();
       }
       else
       {
          waitRequest = mSlaveWaitRequest.next();
       }
    }
}

void
TDELauncher::slotSlaveGone()
{
    IdleSlave *slave = (IdleSlave *) sender();
    mSlaveList.removeRef(slave);
    if ((mSlaveList.count() == 0) && (mTimer.isActive()))
    {
       mTimer.stop();
    }
}

void
TDELauncher::idleTimeout()
{
    bool keepOneFileSlave=true;
    time_t now = time(0);
    IdleSlave *slave;
    for(slave = mSlaveList.first(); slave; slave = mSlaveList.next())
    {
        if ((slave->protocol()=="file") && (keepOneFileSlave))
           keepOneFileSlave=false;
        else if (slave->age(now) > SLAVE_MAX_IDLE)
        {
           // killing idle slave
           delete slave;
        }
    }
}

#include "tdelauncher.moc"
