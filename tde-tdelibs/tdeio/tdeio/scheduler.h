/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       Waldo Bastian <bastian@kde.org>

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

#ifndef _tdeio_scheduler_h
#define _tdeio_scheduler_h

#include "tdeio/job.h"
#include "tdeio/jobclasses.h"
#include <tqtimer.h>
#include <tqptrdict.h>
#include <tqmap.h>

#include <dcopobject.h>

namespace TDEIO {

    class Slave;
    class SlaveList;
    class SlaveConfig;
    class SessionData;

    /**
     * The TDEIO::Scheduler manages io-slaves for the application.
     * It also queues jobs and assigns the job to a slave when one
     * becomes available.
     *
     * There are 3 possible ways for a job to get a slave:
     *
     * <h3>1. Direct</h3>
     * This is the default. When you create a job the
     * TDEIO::Scheduler will be notified and will find either an existing
     * slave that is idle or it will create a new slave for the job.
     *
     * Example:
     * \code
     *    TransferJob *job = TDEIO::get(KURL("http://www.kde.org"));
     * \endcode
     *
     *
     * <h3>2. Scheduled</h3>
     * If you create a lot of jobs, you might want not want to have a
     * slave for each job. If you schedule a job, a maximum number
     * of slaves will be created. When more jobs arrive, they will be
     * queued. When a slave is finished with a job, it will be assigned
     * a job from the queue.
     *
     * Example:
     * \code
     *    TransferJob *job = TDEIO::get(KURL("http://www.kde.org"));
     *    TDEIO::Scheduler::scheduleJob(job);
     * \endcode
     *
     * <h3>3. Connection Oriented</h3>
     * For some operations it is important that multiple jobs use
     * the same connection. This can only be ensured if all these jobs
     * use the same slave.
     *
     * You can ask the scheduler to open a slave for connection oriented
     * operations. You can then use the scheduler to assign jobs to this
     * slave. The jobs will be queued and the slave will handle these jobs
     * one after the other.
     *
     * Example:
     * \code
     *    Slave *slave = TDEIO::Scheduler::getConnectedSlave(
     *            KURL("pop3://bastian:password@mail.kde.org"));
     *    TransferJob *job1 = TDEIO::get(
     *            KURL("pop3://bastian:password@mail.kde.org/msg1"));
     *    TDEIO::Scheduler::assignJobToSlave(slave, job1);
     *    TransferJob *job2 = TDEIO::get(
     *            KURL("pop3://bastian:password@mail.kde.org/msg2"));
     *    TDEIO::Scheduler::assignJobToSlave(slave, job2);
     *    TransferJob *job3 = TDEIO::get(
     *            KURL("pop3://bastian:password@mail.kde.org/msg3"));
     *    TDEIO::Scheduler::assignJobToSlave(slave, job3);
     *
     *    // ... Wait for jobs to finish...
     *
     *    TDEIO::Scheduler::disconnectSlave(slave);
     * \endcode
     *
     * Note that you need to explicitly disconnect the slave when the 
     * connection goes down, so your error handler should contain:
     * \code
     *    if (error == TDEIO::ERR_CONNECTION_BROKEN)
     *        TDEIO::Scheduler::disconnectSlave(slave);
     * \endcode
     *
     * @see TDEIO::Slave
     * @see TDEIO::Job
     **/

    class TDEIO_EXPORT Scheduler : public TQObject, virtual public DCOPObject {
        TQ_OBJECT

    public:
        typedef TQPtrList<SimpleJob> JobList;

        // InfoDict needs Info, so we can't declare it private
        class ProtocolInfo;
        class JobData;

        ~Scheduler();

        /**
         * Register @p job with the scheduler. 
         * The default is to create a new slave for the job if no slave
         * is available. This can be changed by calling scheduleJob.
	 * @param job the job to register
         */
        static void doJob(SimpleJob *job)
        { self()->_doJob(job); }

        /**
         * Calling ths function makes that @p job gets scheduled for later
         * execution, if multiple jobs are registered it might wait for
         * other jobs to finish.
	 * @param job the job to schedule
         */
        static void scheduleJob(SimpleJob *job)
        { self()->_scheduleJob(job); }

        /**
         * Stop the execution of a job.
	 * @param job the job to cancel
         */
        static void cancelJob(SimpleJob *job)
        { self()->_cancelJob(job); }

        /**
         * Called when a job is done.
	 * @param job the finished job
	 * @param slave the slave that executed the @p job
         */
        static void jobFinished(TDEIO::SimpleJob *job, TDEIO::Slave *slave)
        { self()->_jobFinished(job, slave); }

        /**
         * Puts a slave on notice. A next job may reuse this slave if it
         * requests the same URL.
         *
         * A job can be put on hold after it has emit'ed its mimetype.
         * Based on the mimetype, the program can give control to another
         * component in the same process which can then resume the job
         * by simply asking for the same URL again.
	 * @param job the job that should be stopped
	 * @param url the URL that is handled by the @p url
         */
        static void putSlaveOnHold(TDEIO::SimpleJob *job, const KURL &url)
        { self()->_putSlaveOnHold(job, url); }

        /**
         * Removes any slave that might have been put on hold. If a slave 
         * was put on hold it will be killed.
         */
        static void removeSlaveOnHold()
        { self()->_removeSlaveOnHold(); }

        /**
         * Send the slave that was put on hold back to TDELauncher. This
         * allows another process to take over the slave and resume the job
         * that was started.
         */
        static void publishSlaveOnHold()
        { self()->_publishSlaveOnHold(); }

        /**
         * Requests a slave for use in connection-oriented mode.
         *
         * @param url This defines the username,password,host & port to
         *            connect with.
         * @param config Configuration data for the slave.
         *
         * @return A pointer to a connected slave or 0 if an error occurred.
         * @see assignJobToSlave()
         * @see disconnectSlave()
         */
        static TDEIO::Slave *getConnectedSlave(const KURL &url, const TDEIO::MetaData &config = MetaData() )
        { return self()->_getConnectedSlave(url, config); }

        /*
         * Uses @p slave to do @p job.
         * This function should be called immediately after creating a Job.
         *
         * @param slave The slave to use. The slave must have been obtained
         *              with a call to getConnectedSlave and must not
         *              be currently assigned to any other job.
         * @param job The job to do.
         *
         * @return true is successful, false otherwise.
         *
         * @see getConnectedSlave()
         * @see disconnectSlave()
         * @see slaveConnected()
         * @see slaveError()
         */
        static bool assignJobToSlave(TDEIO::Slave *slave, TDEIO::SimpleJob *job)
        { return self()->_assignJobToSlave(slave, job); }

        /*
         * Disconnects @p slave.
         *
         * @param slave The slave to disconnect. The slave must have been
         *              obtained with a call to getConnectedSlave
         *              and must not be assigned to any job.
         *
         * @return true is successful, false otherwise.
         *
         * @see getConnectedSlave
         * @see assignJobToSlave
         */
        static bool disconnectSlave(TDEIO::Slave *slave)
        { return self()->_disconnectSlave(slave); }

        /**
         * Send the slave that was put on hold back to TDELauncher. This
         * allows another process to take over the slave and resume the job
         * the that was started.
         * Register the mainwindow @p wid with the TDEIO subsystem
         * Do not call this, it is called automatically from
         * void TDEIO::Job::setWindow(TQWidget*).
	 * @param wid the window to register
	 * @since 3.1
         */
        static void registerWindow(TQWidget *wid)
        { self()->_registerWindow(wid); }
        
        /**
         * @internal
         * Unregisters the window registered by registerWindow().
         */
        static void unregisterWindow(TQObject *wid)
        { self()->slotUnregisterWindow(wid); }

        /**
         * Function to connect signals emitted by the scheduler.
         *
         * @see slaveConnected()
         * @see slaveError()
         */
        static bool connect( const char *signal, const TQObject *receiver,
                             const char *member)
        { return TQObject::connect(self(), signal, receiver, member); }

        static bool connect( const TQObject* sender, const char* signal,
                             const TQObject* receiver, const char* member )
        { return TQObject::connect(sender, signal, receiver, member); }

        static bool disconnect( const TQObject* sender, const char* signal,
                                const TQObject* receiver, const char* member )
        { return TQObject::disconnect(sender, signal, receiver, member); }

        bool connect( const TQObject *sender, const char *signal,
                      const char *member )
        { return TQObject::connect(sender, signal, member); }

        /**
         * When true, the next job will check whether TDELauncher has a slave 
         * on hold that is suitable for the job.
	 * @param b true when TDELauncher has a job on hold
         */
        static void checkSlaveOnHold(bool b) { self()->_checkSlaveOnHold(b); }

        void debug_info();

        virtual bool process(const TQCString &fun, const TQByteArray &data,
                             TQCString& replyType, TQByteArray &replyData);

        virtual QCStringList functions();

    public slots:
        void slotSlaveDied(TDEIO::Slave *slave);
        void slotSlaveStatus(pid_t pid, const TQCString &protocol,
                             const TQString &host, bool connected);
    signals:
        void slaveConnected(TDEIO::Slave *slave);
        void slaveError(TDEIO::Slave *slave, int error, const TQString &errorMsg);

    protected:
        void setupSlave(TDEIO::Slave *slave, const KURL &url, const TQString &protocol, const TQString &proxy , bool newSlave, const TDEIO::MetaData *config=0);
        bool startJobScheduled(ProtocolInfo *protInfo);
        bool startJobDirect();
        Scheduler();

    protected slots:
        void startStep();
        void slotCleanIdleSlaves();
        void slotSlaveConnected();
        void slotSlaveError(int error, const TQString &errorMsg);
        void slotScheduleCoSlave();
      /// @since 3.1
        void slotUnregisterWindow(TQObject *);

    private:
        class ProtocolInfoDict;
        class ExtraJobData;

        Scheduler(const Scheduler&);
        static Scheduler *self();
        static Scheduler *instance;
        void _doJob(SimpleJob *job);
        void _scheduleJob(SimpleJob *job);
        void _cancelJob(SimpleJob *job);
        void _jobFinished(TDEIO::SimpleJob *job, TDEIO::Slave *slave);
        void _scheduleCleanup();
        void _putSlaveOnHold(TDEIO::SimpleJob *job, const KURL &url);
        void _removeSlaveOnHold();
        Slave *_getConnectedSlave(const KURL &url, const TDEIO::MetaData &metaData );
        bool _assignJobToSlave(TDEIO::Slave *slave, TDEIO::SimpleJob *job);
        bool _disconnectSlave(TDEIO::Slave *slave);
        void _checkSlaveOnHold(bool b);
        void _publishSlaveOnHold();
        void _registerWindow(TQWidget *wid);
        
        Slave *findIdleSlave(ProtocolInfo *protInfo, SimpleJob *job, bool &exact);
        Slave *createSlave(ProtocolInfo *protInfo, SimpleJob *job, const KURL &url);
        

        TQTimer slaveTimer;
        TQTimer coSlaveTimer;
        TQTimer cleanupTimer;
        bool busy;

        SlaveList *slaveList;
        SlaveList *idleSlaves;
        SlaveList *coIdleSlaves;

        ProtocolInfoDict *protInfoDict;
        Slave *slaveOnHold;
        KURL urlOnHold;
        JobList newJobs;

        TQPtrDict<JobList> coSlaves;
        ExtraJobData *extraJobData;
        SlaveConfig *slaveConfig;
        SessionData *sessionData;
        bool checkOnHold;
        TQMap<TQObject *,WId> m_windowList;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class SchedulerPrivate* d;
};

}
#endif
