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

#ifndef _TDELAUNCHER_H_
#define _TDELAUNCHER_H_

#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <tqstring.h>
#include <tqvaluelist.h>
#include <tqsocketnotifier.h>
#include <tqptrlist.h>
#include <tqtimer.h>

#include <dcopclient.h>
#include <tdeio/connection.h>
#include <ksock.h>
#include <kurl.h>
#include <kuniqueapplication.h>

#include <kservice.h>

#include "autostart.h"

class IdleSlave : public TQObject
{
   TQ_OBJECT
public:
   IdleSlave(TDESocket *socket);
   bool match( const TQString &protocol, const TQString &host, bool connected);
   void connect( const TQString &app_socket);
   pid_t pid() const { return mPid;}
   int age(time_t now);
   void reparseConfiguration();
   bool onHold(const KURL &url);
   TQString protocol() const   {return mProtocol;}

signals:
   void statusUpdate(IdleSlave *);

protected slots:
   void gotInput();

protected:
   TDEIO::Connection mConn;
   TQString mProtocol;
   TQString mHost;
   bool mConnected;
   pid_t mPid;
   time_t mBirthDate;
   bool mOnHold;
   KURL mUrl;
};

class SlaveWaitRequest
{
public:
   pid_t pid;
   DCOPClientTransaction *transaction;
};

class TDELaunchRequest
{
public:
   TQCString name;
   TQValueList<TQCString> arg_list;
   TQCString dcop_name;
   enum status_t { Init = 0, Launching, Running, Error, Done };
   pid_t pid;
   status_t status;
   DCOPClientTransaction *transaction;
   KService::DCOPServiceType_t dcop_service_type;
   bool autoStart;
   TQString errorMsg;
#ifdef TQ_WS_X11
   TQCString startup_id; // "" is the default, "0" for none
   TQCString startup_dpy; // Display to send startup notification to.
#endif
   TQValueList<TQCString> envs; // env. variables to be app's environment
   TQCString cwd;
};

struct serviceResult
{
  int result;        // 0 means success. > 0 means error (-1 means pending)
  TQCString dcopName; // Contains DCOP name on success
  TQString error;     // Contains error description on failure.
  pid_t pid;
};

class TDELauncher : public TDEApplication, public DCOPObject
{
   TQ_OBJECT

public:
   TDELauncher(int _tdeinitSocket, bool new_startup);

   ~TDELauncher();

   void close();
   static void destruct(int exit_code); // exit!

   // DCOP
   virtual bool process(const TQCString &fun, const TQByteArray &data,
                TQCString &replyType, TQByteArray &replyData);
   virtual QCStringList functions();
   virtual QCStringList interfaces();

protected:
   void processDied(pid_t pid, long exitStatus);

   void requestStart(TDELaunchRequest *request);
   void requestDone(TDELaunchRequest *request);

   void setLaunchEnv(const TQCString &name, const TQCString &value);
   void exec_blind(const TQCString &name, const TQValueList<TQCString> &arg_list,
       const TQValueList<TQCString> &envs, const TQCString& startup_id = "" );
   bool start_service(KService::Ptr service, const TQStringList &urls,
       const TQValueList<TQCString> &envs, const TQCString& startup_id = "",
       bool blind = false, bool autoStart = false );
   bool start_service_by_name(const TQString &serviceName, const TQStringList &urls,
       const TQValueList<TQCString> &envs, const TQCString& startup_id, bool blind);
   bool start_service_by_desktop_path(const TQString &serviceName, const TQStringList &urls,
       const TQValueList<TQCString> &envs, const TQCString& startup_id, bool blind);
   bool start_service_by_desktop_name(const TQString &serviceName, const TQStringList &urls,
       const TQValueList<TQCString> &envs, const TQCString& startup_id, bool blind);
   bool tdeinit_exec(const TQString &app, const TQStringList &args,
       const TQValueList<TQCString> &envs, TQCString startup_id, bool wait);

   void waitForSlave(pid_t pid);

   void autoStart(int phase);

   void createArgs( TDELaunchRequest *request, const KService::Ptr service,
                    const TQStringList &url);

   pid_t requestHoldSlave(const KURL &url, const TQString &app_socket);
   pid_t requestSlave(const TQString &protocol, const TQString &host,
                      const TQString &app_socket, TQString &error);


   void queueRequest(TDELaunchRequest *);

   void send_service_startup_info( TDELaunchRequest *request, KService::Ptr service, const TQCString& startup_id,
       const TQValueList<TQCString> &envs );
   void cancel_service_startup_info( TDELaunchRequest *request, const TQCString& startup_id,
       const TQValueList<TQCString> &envs );

public slots:
   void slotAutoStart();
   void slotDequeue();
   void slotKDEInitData(int);
   void slotAppRegistered(const TQCString &appId);
   void slotSlaveStatus(IdleSlave *);
   void acceptSlave( TDESocket *);
   void slotSlaveGone();
   void idleTimeout();

protected:
   TQPtrList<TDELaunchRequest> requestList; // Requests being handled
   TQPtrList<TDELaunchRequest> requestQueue; // Requests waiting to being handled
   int tdeinitSocket;
   TQSocketNotifier *tdeinitNotifier;
   serviceResult DCOPresult;
   TDELaunchRequest *lastRequest;
   TQPtrList<SlaveWaitRequest> mSlaveWaitRequest;
   TQString mPoolSocketName;
   TDEServerSocket *mPoolSocket;
   TQPtrList<IdleSlave> mSlaveList;
   TQTimer mTimer;
   TQTimer mAutoTimer;
   bool bProcessingQueue;
   AutoStart mAutoStart;
   TQCString mSlaveDebug;
   TQCString mSlaveValgrind;
   TQCString mSlaveValgrindSkin;
   bool dontBlockReading;
   bool newStartup;
#ifdef TQ_WS_X11
   Display *mCached_dpy;
#endif
};
#endif
