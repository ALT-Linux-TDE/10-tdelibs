/* This file is part of the KDE libraries
   Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                      Waldo Bastian <bastian@kde.org>

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

#include "tdeio/sessiondata.h"
#include "tdeio/slaveconfig.h"
#include "tdeio/scheduler.h"
#include "tdeio/authinfo.h"
#include "tdeio/slave.h"
#include <tqptrlist.h>
#include <tqdict.h>

#include <dcopclient.h>

#include <kdebug.h>
#include <tdeglobal.h>
#include <tdeprotocolmanager.h>
#include <kprotocolinfo.h>
#include <assert.h>
#include <kstaticdeleter.h>
#include <tdesu/client.h>


// Slaves may be idle for MAX_SLAVE_IDLE time before they are being returned
// to the system wide slave pool. (3 minutes)
#define MAX_SLAVE_IDLE (3*60)

using namespace TDEIO;

template class TQDict<TDEIO::Scheduler::ProtocolInfo>;

Scheduler *Scheduler::instance = 0;

class TDEIO::SlaveList: public TQPtrList<Slave>
{
   public:
      SlaveList() { }
};

//
// There are two kinds of protocol:
// (1) The protocol of the url
// (2) The actual protocol that the io-slave uses.
//
// These two often match, but not necasserily. Most notably, they don't
// match when doing ftp via a proxy.
// In that case (1) is ftp, but (2) is http.
//
// JobData::protocol stores (2) while Job::url().protocol() returns (1).
// The ProtocolInfoDict is indexed with (2).
//
// We schedule slaves based on (2) but tell the slave about (1) via
// Slave::setProtocol().

class TDEIO::Scheduler::JobData
{
public:
    JobData() : checkOnHold(false) { }

public:
    TQString protocol;
    TQString proxy;
    bool checkOnHold;
};

class TDEIO::Scheduler::ExtraJobData: public TQPtrDict<TDEIO::Scheduler::JobData>
{
public:
    ExtraJobData() { setAutoDelete(true); }
};

class TDEIO::Scheduler::ProtocolInfo
{
public:
    ProtocolInfo() : maxSlaves(1), skipCount(0)
    {
       joblist.setAutoDelete(false);
    }

    TQPtrList<SimpleJob> joblist;
    SlaveList activeSlaves;
    int maxSlaves;
    int skipCount;
    TQString protocol;
};

class TDEIO::Scheduler::ProtocolInfoDict : public TQDict<TDEIO::Scheduler::ProtocolInfo>
{
  public:
    ProtocolInfoDict() { }

    TDEIO::Scheduler::ProtocolInfo *get( const TQString &protocol);
};

TDEIO::Scheduler::ProtocolInfo *
TDEIO::Scheduler::ProtocolInfoDict::get(const TQString &protocol)
{
    ProtocolInfo *info = find(protocol);
    if (!info)
    {
        info = new ProtocolInfo;
        info->protocol = protocol;
        info->maxSlaves = KProtocolInfo::maxSlaves( protocol );

        insert(protocol, info);
    }
    return info;
}


Scheduler::Scheduler()
          : DCOPObject( "TDEIO::Scheduler" ),
           TQObject(kapp, "scheduler"),
           slaveTimer(0, "Scheduler::slaveTimer"),
           coSlaveTimer(0, "Scheduler::coSlaveTimer"),
           cleanupTimer(0, "Scheduler::cleanupTimer")
{
    checkOnHold = true; // !! Always check with TDELauncher for the first request.
    slaveOnHold = 0;
    protInfoDict = new ProtocolInfoDict;
    slaveList = new SlaveList;
    idleSlaves = new SlaveList;
    coIdleSlaves = new SlaveList;
    extraJobData = new ExtraJobData;
    sessionData = new SessionData;
    slaveConfig = SlaveConfig::self();
    connect(&slaveTimer, TQ_SIGNAL(timeout()), TQ_SLOT(startStep()));
    connect(&coSlaveTimer, TQ_SIGNAL(timeout()), TQ_SLOT(slotScheduleCoSlave()));
    connect(&cleanupTimer, TQ_SIGNAL(timeout()), TQ_SLOT(slotCleanIdleSlaves()));
    busy = false;
}

Scheduler::~Scheduler()
{
    protInfoDict->setAutoDelete(true);
    delete protInfoDict; protInfoDict = 0;
    delete idleSlaves; idleSlaves = 0;
    delete coIdleSlaves; coIdleSlaves = 0;
    slaveList->setAutoDelete(true);
    delete slaveList; slaveList = 0;
    delete extraJobData; extraJobData = 0;
    delete sessionData; sessionData = 0;
    instance = 0;
}

void
Scheduler::debug_info()
{
}

bool Scheduler::process(const TQCString &fun, const TQByteArray &data, TQCString &replyType, TQByteArray &replyData )
{
    if ( fun != "reparseSlaveConfiguration(TQString)" )
        return DCOPObject::process( fun, data, replyType, replyData );

    slaveConfig = SlaveConfig::self();
    replyType = "void";
    TQDataStream stream( data, IO_ReadOnly );
    TQString proto;
    stream >> proto;

    kdDebug( 7006 ) << "reparseConfiguration( " << proto << " )" << endl;
    KProtocolManager::reparseConfiguration();
    slaveConfig->reset();
    sessionData->reset();
    NetRC::self()->reload();

    Slave *slave = slaveList->first();
    for (; slave; slave = slaveList->next() )
    if ( slave->slaveProtocol() == proto || proto.isEmpty() )
    {
      slave->send( CMD_REPARSECONFIGURATION );
      slave->resetHost();
    }
    return true;
}

QCStringList Scheduler::functions()
{
    QCStringList funcs = DCOPObject::functions();
    funcs << "void reparseSlaveConfiguration(TQString)";
    return funcs;
}

void Scheduler::_doJob(SimpleJob *job) {
    JobData *jobData = new JobData;
    jobData->protocol = KProtocolManager::slaveProtocol(job->url(), jobData->proxy);
//    kdDebug(7006) << "Scheduler::_doJob protocol=" << jobData->protocol << endl;
    if (job->command() == CMD_GET)
    {
       jobData->checkOnHold = checkOnHold;
       checkOnHold = false;
    }
    extraJobData->replace(job, jobData);
    newJobs.append(job);
    slaveTimer.start(0, true);
#ifndef NDEBUG
    if (newJobs.count() > 150)
	kdDebug() << "WARNING - TDEIO::Scheduler got more than 150 jobs! This shows a misuse in your app (yes, a job is a TQObject)." << endl;
#endif
}

void Scheduler::_scheduleJob(SimpleJob *job) {
    newJobs.removeRef(job);
    JobData *jobData = extraJobData->find(job);
    if (!jobData)
{
    kdFatal(7006) << "BUG! _ScheduleJob(): No extraJobData for job!" << endl;
    return;
}
    TQString protocol = jobData->protocol;
//    kdDebug(7006) << "Scheduler::_scheduleJob protocol=" << protocol << endl;
    ProtocolInfo *protInfo = protInfoDict->get(protocol);
    protInfo->joblist.append(job);

    slaveTimer.start(0, true);
}

void Scheduler::_cancelJob(SimpleJob *job) {
//    kdDebug(7006) << "Scheduler: canceling job " << job << endl;
    Slave *slave = job->slave();
    if ( !slave  )
    {
        // was not yet running (don't call this on a finished job!)
        JobData *jobData = extraJobData->find(job);
        if (!jobData)
           return; // I said: "Don't call this on a finished job!"

        newJobs.removeRef(job);
        ProtocolInfo *protInfo = protInfoDict->get(jobData->protocol);
        protInfo->joblist.removeRef(job);

        // Search all slaves to see if job is in the queue of a coSlave
        slave = slaveList->first();
        for(; slave; slave = slaveList->next())
        {
           JobList *list = coSlaves.find(slave);
           if (list && list->removeRef(job))
              break; // Job was found and removed.
                     // Fall through to kill the slave as well!
        }
        if (!slave)
        {
           extraJobData->remove(job);
           return; // Job was not yet running and not in a coSlave queue.
        }
    }
    kdDebug(7006) << "Scheduler: killing slave " << slave->slave_pid() << endl;
    slave->kill();
    _jobFinished( job, slave );
    slotSlaveDied( slave);
}

void Scheduler::startStep()
{
    while(newJobs.count())
    {
       (void) startJobDirect();
    }
    TQDictIterator<TDEIO::Scheduler::ProtocolInfo> it(*protInfoDict);
    while(it.current())
    {
       if (startJobScheduled(it.current())) return;
       ++it;
    }
}

void Scheduler::setupSlave(TDEIO::Slave *slave, const KURL &url, const TQString &protocol, const TQString &proxy , bool newSlave, const TDEIO::MetaData *config)
{
    TQString host = url.host();
    int port = url.port();
    TQString user = url.user();
    TQString passwd = url.pass();

    if ((newSlave) ||
        (slave->host() != host) ||
        (slave->port() != port) ||
        (slave->user() != user) ||
        (slave->passwd() != passwd))
    {
        slaveConfig = SlaveConfig::self();

        MetaData configData = slaveConfig->configData(protocol, host);
        sessionData->configDataFor( configData, protocol, host );

        configData["UseProxy"] = proxy;

        TQString autoLogin = configData["EnableAutoLogin"].lower();
        if ( autoLogin == "true" )
        {
            NetRC::AutoLogin l;
            l.login = user;
            bool usern = (protocol == "ftp");
            if ( NetRC::self()->lookup( url, l, usern) )
            {
                configData["autoLoginUser"] = l.login;
                configData["autoLoginPass"] = l.password;
                if ( usern )
                {
                    TQString macdef;
                    TQMap<TQString, TQStringList>::ConstIterator it = l.macdef.begin();
                    for ( ; it != l.macdef.end(); ++it )
                        macdef += it.key() + '\\' + it.data().join( "\\" ) + '\n';
                    configData["autoLoginMacro"] = macdef;
                }
            }
        }
        if (config)
           configData += *config;
        slave->setConfig(configData);
        slave->setProtocol(url.protocol());
        slave->setHost(host, port, user, passwd);
    }
}

bool Scheduler::startJobScheduled(ProtocolInfo *protInfo)
{
    if (protInfo->joblist.isEmpty())
       return false;

//       kdDebug(7006) << "Scheduling job" << endl;
    debug_info();
    bool newSlave = false;

    SimpleJob *job = 0;
    Slave *slave = 0;

    if (protInfo->skipCount > 2)
    {
       bool dummy;
       // Prevent starvation. We skip the first entry in the queue at most
       // 2 times in a row. The
       protInfo->skipCount = 0;
       job = protInfo->joblist.at(0);
       slave = findIdleSlave(protInfo, job, dummy );
    }
    else
    {
       bool exact=false;
       SimpleJob *firstJob = 0;
       Slave *firstSlave = 0;
       for(uint i = 0; (i < protInfo->joblist.count()) && (i < 10); i++)
       {
          job = protInfo->joblist.at(i);
          slave = findIdleSlave(protInfo, job, exact);
          if (!firstSlave)
          {
             firstJob = job;
             firstSlave = slave;
          }
          if (!slave) break;
          if (exact) break;
       }

       if (!exact)
       {
         slave = firstSlave;
         job = firstJob;
       }
       if (job == firstJob)
         protInfo->skipCount = 0;
       else
         protInfo->skipCount++;
    }

    if (!slave)
    {
       if ( protInfo->maxSlaves > static_cast<int>(protInfo->activeSlaves.count()) )
       {
          newSlave = true;
          slave = createSlave(protInfo, job, job->url());
          if (!slave)
             slaveTimer.start(0, true);
       }
    }

    if (!slave)
    {
//          kdDebug(7006) << "No slaves available" << endl;
//          kdDebug(7006) << " -- active: " << protInfo->activeSlaves.count() << endl;
       return false;
    }

    protInfo->activeSlaves.append(slave);
    idleSlaves->removeRef(slave);
    protInfo->joblist.removeRef(job);
//       kdDebug(7006) << "scheduler: job started " << job << endl;


    JobData *jobData = extraJobData->find(job);
    setupSlave(slave, job->url(), jobData->protocol, jobData->proxy, newSlave);
    job->start(slave);

    slaveTimer.start(0, true);
    return true;
}

bool Scheduler::startJobDirect()
{
    debug_info();
    SimpleJob *job = newJobs.take(0);
    JobData *jobData = extraJobData->find(job);
    if (!jobData)
    {
        kdFatal(7006) << "BUG! startjobDirect(): No extraJobData for job!"
                      << endl;
        return false;
    }
    TQString protocol = jobData->protocol;
    ProtocolInfo *protInfo = protInfoDict->get(protocol);

    bool newSlave = false;
    bool dummy;

    // Look for matching slave
    Slave *slave = findIdleSlave(protInfo, job, dummy);

    if (!slave)
    {
       newSlave = true;
       slave = createSlave(protInfo, job, job->url());
    }

    if (!slave)
       return false;

    idleSlaves->removeRef(slave);
//       kdDebug(7006) << "scheduler: job started " << job << endl;

    setupSlave(slave, job->url(), protocol, jobData->proxy, newSlave);
    job->start(slave);
    return true;
}

static Slave *searchIdleList(SlaveList *idleSlaves, const KURL &url, const TQString &protocol, bool &exact)
{
    TQString host = url.host();
    int port = url.port();
    TQString user = url.user();
    exact = true;

    for( Slave *slave = idleSlaves->first();
         slave;
         slave = idleSlaves->next())
    {
       if ((protocol == slave->slaveProtocol()) &&
           (host == slave->host()) &&
           (port == slave->port()) &&
           (user == slave->user()))
           return slave;
    }

    exact = false;

    // Look for slightly matching slave
    for( Slave *slave = idleSlaves->first();
         slave;
         slave = idleSlaves->next())
    {
       if (protocol == slave->slaveProtocol())
          return slave;
    }
    return 0;
}

Slave *Scheduler::findIdleSlave(ProtocolInfo *, SimpleJob *job, bool &exact)
{
    Slave *slave = 0;
    JobData *jobData = extraJobData->find(job);
    if (!jobData)
    {
        kdFatal(7006) << "BUG! findIdleSlave(): No extraJobData for job!" << endl;
        return 0;
    }
    if (jobData->checkOnHold)
    {
       slave = Slave::holdSlave(jobData->protocol, job->url());
       if (slave)
          return slave;
    }
    if (slaveOnHold)
    {
       // Make sure that the job wants to do a GET or a POST, and with no offset
       bool bCanReuse = (job->command() == CMD_GET);
       TDEIO::TransferJob * tJob = dynamic_cast<TDEIO::TransferJob *>(job);
       if ( tJob )
       {
          bCanReuse = (job->command() == CMD_GET || job->command() == CMD_SPECIAL);
          if ( bCanReuse )
          {
            TDEIO::MetaData outgoing = tJob->outgoingMetaData();
            TQString resume = (!outgoing.contains("resume")) ? TQString() : outgoing["resume"];
            kdDebug(7006) << "Resume metadata is '" << resume << "'" << endl;
            bCanReuse = (resume.isEmpty() || resume == "0");
          }
       }
//       kdDebug(7006) << "bCanReuse = " << bCanReuse << endl;
       if (bCanReuse)
       {
          if (job->url() == urlOnHold)
          {
             kdDebug(7006) << "HOLD: Reusing held slave for " << urlOnHold.prettyURL() << endl;
             slave = slaveOnHold;
          }
          else
          {
             kdDebug(7006) << "HOLD: Discarding held slave (" << urlOnHold.prettyURL() << ")" << endl;
             slaveOnHold->kill();
          }
          slaveOnHold = 0;
          urlOnHold = KURL();
       }
       if (slave)
          return slave;
    }

    return searchIdleList(idleSlaves, job->url(), jobData->protocol, exact);
}

Slave *Scheduler::createSlave(ProtocolInfo *protInfo, SimpleJob *job, const KURL &url)
{
   int error;
   TQString errortext;
   Slave *slave = Slave::createSlave(protInfo->protocol, url, error, errortext);
   if (slave)
   {
      slaveList->append(slave);
      idleSlaves->append(slave);
      connect(slave, TQ_SIGNAL(slaveDied(TDEIO::Slave *)),
                TQ_SLOT(slotSlaveDied(TDEIO::Slave *)));
      connect(slave, TQ_SIGNAL(slaveStatus(pid_t,const TQCString &,const TQString &, bool)),
                TQ_SLOT(slotSlaveStatus(pid_t,const TQCString &, const TQString &, bool)));

      connect(slave,TQ_SIGNAL(authorizationKey(const TQCString&, const TQCString&, bool)),
              sessionData,TQ_SLOT(slotAuthData(const TQCString&, const TQCString&, bool)));
      connect(slave,TQ_SIGNAL(delAuthorization(const TQCString&)), sessionData,
              TQ_SLOT(slotDelAuthData(const TQCString&)));
   }
   else
   {
      kdError() <<": couldn't create slave : " << errortext << endl;
      if (job)
      {
         protInfo->joblist.removeRef(job);
         extraJobData->remove(job);
         job->slotError( error, errortext );
      }
   }
   return slave;
}

void Scheduler::slotSlaveStatus(pid_t, const TQCString &, const TQString &, bool)
{
}

void Scheduler::_jobFinished(SimpleJob *job, Slave *slave)
{
    JobData *jobData = extraJobData->take(job);
    if (!jobData)
    {
        kdFatal(7006) << "BUG! _jobFinished(): No extraJobData for job!" << endl;
        return;
    }
    ProtocolInfo *protInfo = protInfoDict->get(jobData->protocol);
    delete jobData;
    slave->disconnect(job);
    protInfo->activeSlaves.removeRef(slave);
    if (slave->isAlive())
    {
       JobList *list = coSlaves.find(slave);
       if (list)
       {
          assert(slave->isConnected());
          assert(!coIdleSlaves->contains(slave));
          coIdleSlaves->append(slave);
          if (!list->isEmpty())
             coSlaveTimer.start(0, true);
          return;
       }
       else
       {
          assert(!slave->isConnected());
          idleSlaves->append(slave);
          slave->setIdle();
          _scheduleCleanup();
//          slave->send( CMD_SLAVE_STATUS );
       }
    }
    if (protInfo->joblist.count())
    {
       slaveTimer.start(0, true);
    }
}

void Scheduler::slotSlaveDied(TDEIO::Slave *slave)
{
    assert(!slave->isAlive());
    ProtocolInfo *protInfo = protInfoDict->get(slave->slaveProtocol());
    protInfo->activeSlaves.removeRef(slave);
    if (slave == slaveOnHold)
    {
       slaveOnHold = 0;
       urlOnHold = KURL();
    }
    idleSlaves->removeRef(slave);
    JobList *list = coSlaves.find(slave);
    if (list)
    {
       // coSlave dies, kill jobs waiting in queue
       disconnectSlave(slave);
    }

    if (!slaveList->removeRef(slave))
        kdDebug(7006) << "Scheduler: BUG!! Slave " << slave << "/" << slave->slave_pid() << " died, but is NOT in slaveList!!!\n" << endl;
    else
        slave->deref(); // Delete slave
}

void Scheduler::slotCleanIdleSlaves()
{
    for(Slave *slave = idleSlaves->first();slave;)
    {
        if (slave->idleTime() >= MAX_SLAVE_IDLE)
        {
           // kdDebug(7006) << "Removing idle slave: " << slave->slaveProtocol() << " " << slave->host() << endl;
           Slave *removeSlave = slave;
           slave = idleSlaves->next();
           idleSlaves->removeRef(removeSlave);
           slaveList->removeRef(removeSlave);
           removeSlave->connection()->close();
           removeSlave->deref();
        }
        else
        {
            slave = idleSlaves->next();
        }
    }
    _scheduleCleanup();
}

void Scheduler::_scheduleCleanup()
{
    if (idleSlaves->count())
    {
        if (!cleanupTimer.isActive())
            cleanupTimer.start( MAX_SLAVE_IDLE*1000, true );
    }
}

void Scheduler::_putSlaveOnHold(TDEIO::SimpleJob *job, const KURL &url)
{
    Slave *slave = job->slave();
    slave->disconnect(job);

    if (slaveOnHold)
    {
        slaveOnHold->kill();
    }
    slaveOnHold = slave;
    urlOnHold = url;
    slaveOnHold->suspend();
}

void Scheduler::_publishSlaveOnHold()
{
    if (!slaveOnHold)
       return;

    slaveOnHold->hold(urlOnHold);
}

void Scheduler::_removeSlaveOnHold()
{
    if (slaveOnHold)
    {
        slaveOnHold->kill();
    }
    slaveOnHold = 0;
    urlOnHold = KURL();
}

Slave *
Scheduler::_getConnectedSlave(const KURL &url, const TDEIO::MetaData &config )
{
    TQString proxy;
    TQString protocol = KProtocolManager::slaveProtocol(url, proxy);
    bool dummy;
    Slave *slave = searchIdleList(idleSlaves, url, protocol, dummy);
    if (!slave)
    {
       ProtocolInfo *protInfo = protInfoDict->get(protocol);
       slave = createSlave(protInfo, 0, url);
    }
    if (!slave)
       return 0; // Error
    idleSlaves->removeRef(slave);

    setupSlave(slave, url, protocol, proxy, true, &config);

    slave->send( CMD_CONNECT );
    connect(slave, TQ_SIGNAL(connected()),
                TQ_SLOT(slotSlaveConnected()));
    connect(slave, TQ_SIGNAL(error(int, const TQString &)),
                TQ_SLOT(slotSlaveError(int, const TQString &)));

    coSlaves.insert(slave, new TQPtrList<SimpleJob>());
//    kdDebug(7006) << "_getConnectedSlave( " << slave << ")" << endl;
    return slave;
}

void
Scheduler::slotScheduleCoSlave()
{
    Slave *nextSlave;
    slaveConfig = SlaveConfig::self();
    for(Slave *slave = coIdleSlaves->first();
        slave;
        slave = nextSlave)
    {
        nextSlave = coIdleSlaves->next();
        JobList *list = coSlaves.find(slave);
        assert(list);
        if (list && !list->isEmpty())
        {
           SimpleJob *job = list->take(0);
           coIdleSlaves->removeRef(slave);
//           kdDebug(7006) << "scheduler: job started " << job << endl;

           assert(!coIdleSlaves->contains(slave));

           KURL url =job->url();
           TQString host = url.host();
           int port = url.port();

           if (slave->host() == "<reset>")
           {
              TQString user = url.user();
              TQString passwd = url.pass();

              MetaData configData = slaveConfig->configData(url.protocol(), url.host());
              slave->setConfig(configData);
              slave->setProtocol(url.protocol());
              slave->setHost(host, port, user, passwd);
           }

           assert(slave->protocol() == url.protocol());
           assert(slave->host() == host);
           assert(slave->port() == port);
           job->start(slave);
        }
    }
}

void
Scheduler::slotSlaveConnected()
{
    Slave *slave = (Slave *)sender();
//    kdDebug(7006) << "slotSlaveConnected( " << slave << ")" << endl;
    slave->setConnected(true);
    disconnect(slave, TQ_SIGNAL(connected()),
               this, TQ_SLOT(slotSlaveConnected()));
    emit slaveConnected(slave);
    assert(!coIdleSlaves->contains(slave));
    coIdleSlaves->append(slave);
    coSlaveTimer.start(0, true);
}

void
Scheduler::slotSlaveError(int errorNr, const TQString &errorMsg)
{
    Slave *slave = (Slave *)sender();
    if (!slave->isConnected() || (coIdleSlaves->find(slave) != -1))
    {
       // Only forward to application if slave is idle or still connecting.
       emit slaveError(slave, errorNr, errorMsg);
    }
}

bool
Scheduler::_assignJobToSlave(TDEIO::Slave *slave, SimpleJob *job)
{
//    kdDebug(7006) << "_assignJobToSlave( " << job << ", " << slave << ")" << endl;
    TQString dummy;
    if (!job)
    {
        kdDebug(7006) << "_assignJobToSlave(): ERROR, non-existing job." << endl;
        return false;
    }
    if (!slave)
    {
        kdDebug(7006) << "_assignJobToSlave(): ERROR, non-existing slave." << endl;
        job->kill();
        return false;
    }

    if ((slave->slaveProtocol() != KProtocolManager::slaveProtocol( job->url(), dummy ))
        ||
        (!newJobs.removeRef(job)))
    {
        kdDebug(7006) << "_assignJobToSlave(): ERROR, nonmatching or unknown job." << endl;
        job->kill();
        return false;
    }

    JobList *list = coSlaves.find(slave);
    assert(list);
    if (!list)
    {
        kdDebug(7006) << "_assignJobToSlave(): ERROR, unknown slave." << endl;
        job->kill();
        return false;
    }

    assert(list->contains(job) == 0);
    list->append(job);
    coSlaveTimer.start(0, true); // Start job on timer event

    return true;
}

bool
Scheduler::_disconnectSlave(TDEIO::Slave *slave)
{
//    kdDebug(7006) << "_disconnectSlave( " << slave << ")" << endl;
    JobList *list = coSlaves.take(slave);
    assert(list);
    if (!list)
       return false;
    // Kill jobs still in queue.
    while(!list->isEmpty())
    {
       Job *job = list->take(0);
       job->kill();
    }
    delete list;
    coIdleSlaves->removeRef(slave);
    assert(!coIdleSlaves->contains(slave));
    disconnect(slave, TQ_SIGNAL(connected()),
               this, TQ_SLOT(slotSlaveConnected()));
    disconnect(slave, TQ_SIGNAL(error(int, const TQString &)),
               this, TQ_SLOT(slotSlaveError(int, const TQString &)));
    if (slave->isAlive())
    {
       idleSlaves->append(slave);
       slave->send( CMD_DISCONNECT );
       slave->setIdle();
       slave->setConnected(false);
       _scheduleCleanup();
    }
    return true;
}

void
Scheduler::_checkSlaveOnHold(bool b)
{
    checkOnHold = b;
}

void
Scheduler::_registerWindow(TQWidget *wid)
{
   if (!wid)
      return;

   TQObject *obj = wid;
   if (!m_windowList.contains(obj))
   {
      // We must store the window Id because by the time
      // the destroyed signal is emitted we can no longer
      // access TQWidget::winId() (already destructed)
      WId windowId = wid->winId();
      m_windowList.insert(obj, windowId);
      connect(wid, TQ_SIGNAL(destroyed(TQObject *)),
              this, TQ_SLOT(slotUnregisterWindow(TQObject*)));
      TQByteArray params;
      TQDataStream stream(params, IO_WriteOnly);
      stream << windowId;
      if( !kapp->dcopClient()->send( "kded", "kded",
                    "registerWindowId(long int)", params ) )
      kdDebug(7006) << "Could not register window with kded!" << endl;
   }
}

void
Scheduler::slotUnregisterWindow(TQObject *obj)
{
   if (!obj)
      return;

   TQMap<TQObject *, WId>::Iterator it = m_windowList.find(obj);
   if (it == m_windowList.end())
      return;
   WId windowId = it.data();
   disconnect( it.key(), TQ_SIGNAL(destroyed(TQObject *)),
              this, TQ_SLOT(slotUnregisterWindow(TQObject*)));
   m_windowList.remove( it );
   if (kapp)
   {
      TQByteArray params;
      TQDataStream stream(params, IO_WriteOnly);
      stream << windowId;
      kapp->dcopClient()->send( "kded", "kded",
                    "unregisterWindowId(long int)", params );
   }
}

Scheduler* Scheduler::self() {
    if ( !instance ) {
        instance = new Scheduler;
    }
    return instance;
}

void Scheduler::virtual_hook( int id, void* data )
{ DCOPObject::virtual_hook( id, data ); }



#include "scheduler.moc"
