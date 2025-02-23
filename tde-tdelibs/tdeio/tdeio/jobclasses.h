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

#ifndef __tdeio_jobclasses_h__
#define __tdeio_jobclasses_h__

#include <tqobject.h>
#include <tqptrlist.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqguardedptr.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <kurl.h>
#include <tdeio/global.h>

class Observer;
class TQTimer;

#define TDEIO_COPYJOB_HAS_SETINTERACTIVE // new in 3.4. Used by tdeio_trash.

namespace TDEIO {

    class Slave;
    class SlaveInterface;


    /**
     * The base class for all jobs.
     * For all jobs created in an application, the code looks like
     *
     * \code
     *   TDEIO::Job * job = TDEIO::someoperation( some parameters );
     *   connect( job, TQ_SIGNAL( result( TDEIO::Job * ) ),
     *            this, TQ_SLOT( slotResult( TDEIO::Job * ) ) );
     * \endcode
     *   (other connects, specific to the job)
     *
     * And slotResult is usually at least:
     *
     * \code
     *  if ( job->error() )
     *      job->showErrorDialog( this or 0L  );
     * \endcode
     * @see TDEIO::Scheduler
     * @see TDEIO::Slave
     */
    class TDEIO_EXPORT Job : public TQObject {
        TQ_OBJECT

    protected:
        Job( bool showProgressInfo );

    public:
        virtual ~Job();

        /**
         * Abort this job.
         * This kills all subjobs and deletes the job.
	 *
         * @param quietly if false, Job will emit signal result
         * and ask tdeio_uiserver to close the progress window.
         * @p quietly is set to true for subjobs. Whether applications
         * should call with true or false depends on whether they rely
         * on result being emitted or not.
         */
        virtual void kill( bool quietly = true );

        /**
	 * Returns the error code, if there has been an error.
         * Only call this method from the slot connected to result().
         * @return the error code for this job, 0 if no error.
         * Error codes are defined in TDEIO::Error.
         */
        int error() const { return m_error; }

        /**
	 * Returns the progress id for this job.
         * @return the progress id for this job, as returned by uiserver
         */
        int progressId() const { return m_progressId; }

        /**
	 * Returns the error text if there has been an error.
         * Only call if error is not 0.
         * This is really internal, better use errorString() or errorDialog().
	 *
         * @return a string to help understand the error, usually the url
         * related to the error. Only valid if error() is not 0.
         */
        const TQString & errorText() const { return m_errorText; }

        /**
         * Converts an error code and a non-i18n error message into an
         * error message in the current language. The low level (non-i18n)
         * error message (usually a url) is put into the translated error
         * message using %1.
         *
         * Example for errid == ERR_CANNOT_OPEN_FOR_READING:
	 * \code
         *   i18n( "Could not read\n%1" ).arg( errortext );
	 * \endcode
         * Use this to display the error yourself, but for a dialog box
         * use Job::showErrorDialog. Do not call it if error()
	 * is not 0.
	 * @return the error message and if there is no error, a message
	 *         telling the user that the app is broken, so check with
	 *         error() whether there is an error
         */
        TQString errorString() const;

        /**
         * Converts an error code and a non-i18n error message into i18n
         * strings suitable for presentation in a detailed error message box.
         *
         * @param reqUrl the request URL that generated this error message
         * @param method the method that generated this error message
         * (unimplemented)
         * @return the following strings: caption, error + description,
	 *         causes+solutions
         */
        TQStringList detailedErrorStrings(const KURL *reqUrl = 0L,
                                         int method = -1) const;

        /**
         * Display a dialog box to inform the user of the error given by
         * this job.
         * Only call if error is not 0, and only in the slot connected
         * to result.
         * @param parent the parent widget for the dialog box, can be 0 for
	 *        top-level
         */
        void showErrorDialog( TQWidget * parent = 0L );

        /**
         * Enable or disable the automatic error handling. When automatic
         * error handling is enabled and an error occurs, then showErrorDialog()
         * is called with the specified @p parentWidget (if supplied) , right before
         * the emission of the result signal.
         *
         * The default is false.
         *
         * @param enable enable or disable automatic error handling
         * @param parentWidget the parent widget, passed to showErrorDialog.
	 *        Can be 0 for top-level
	 * @see isAutoErrorHandlingEnabled(), showErrorDialog()
         */
        void setAutoErrorHandlingEnabled( bool enable, TQWidget *parentWidget = 0 );

        /**
         * Returns whether automatic error handling is enabled or disabled.
	 * @return true if automatic error handling is enabled
	 * @see setAutoErrorHandlingEnabled()
         */
        bool isAutoErrorHandlingEnabled() const;

        /**
         * Enable or disable the automatic warning handling. When automatic
         * warning handling is enabled and an error occurs, then a message box
         * is displayed with the warning message
         *
         * The default is true.
         *
         * See also isAutoWarningHandlingEnabled , showErrorDialog
         *
         * @param enable enable or disable automatic warning handling
         * @see isAutoWarningHandlingEnabled()
         * @since 3.5
         */
        void setAutoWarningHandlingEnabled( bool enable );

        /**
         * Returns whether automatic warning handling is enabled or disabled.
         * See also setAutoWarningHandlingEnabled .
         * @return true if automatic warning handling is enabled
         * @see setAutoWarningHandlingEnabled()
         * @since 3.5
         */
        bool isAutoWarningHandlingEnabled() const;

        /**
         * Enable or disable the message display from the job.
         *
         * The default is true.
         * @param enable enable or disable message display
         * @since 3.4.1
         */
        void setInteractive(bool enable);

        /**
         * Returns whether message display is enabled or disabled.
         * @return true if message display is enabled
         * @see setInteractive()
         * @since 3.4.1
         */
        bool isInteractive() const;
        /**
         * Associate this job with a window given by @p window.
	 * @param window the window to associate to
	 * @see window()
         */
        void setWindow(TQWidget *window);

        /**
         * Returns the window this job is associated with.
	 * @return the associated window
	 * @see setWindow()
         */
        TQWidget *window() const;

        /**
         * Updates the last user action timestamp to the given time.
         * See TDEApplication::updateUserTimestamp() .
         * @since 3.5.6
         */
        void updateUserTimestamp( unsigned long time );

        /**
         * Set the parent Job.
         * One example use of this is when FileCopyJob calls open_RenameDlg,
         * it must pass the correct progress ID of the parent CopyJob
         * (to hide the progress dialog).
	 * You can set the parent job only once. By default a job does not
	 * have a parent job.
	 * @param parentJob the new parent job
         * @since 3.1
         */
        void setParentJob( Job* parentJob );

        /**
	 * Returns the parent job, if there is one.
	 * @return the parent job, or 0 if there is none
         * @see setParentJob
         * @since 3.1
         */
        Job* parentJob() const;

        /**
         * Set meta data to be sent to the slave, replacing existing
	 * meta data.
	 * @param metaData the meta data to set
	 * @see addMetaData()
	 * @see mergeMetaData()
         */
        void setMetaData( const TDEIO::MetaData &metaData);

        /**
         * Add key/value pair to the meta data that is sent to the slave.
	 * @param key the key of the meta data
	 * @param value the value of the meta data
	 * @see setMetaData()
	 * @see mergeMetaData()
         */
        void addMetaData(const TQString &key, const TQString &value);

        /**
         * Add key/value pairs to the meta data that is sent to the slave.
         * If a certain key already existed, it will be overridden.
	 * @param values the meta data to add
	 * @see setMetaData()
	 * @see mergeMetaData()
         */
        void addMetaData(const TQMap<TQString,TQString> &values);

        /**
         * Add key/value pairs to the meta data that is sent to the slave.
         * If a certain key already existed, it will remain unchanged.
	 * @param values the meta data to merge
	 * @see setMetaData()
	 * @see addMetaData()
         */
        void mergeMetaData(const TQMap<TQString,TQString> &values);

        /**
         * @internal. For the scheduler. Do not use.
         */
        MetaData outgoingMetaData() const;

        /**
         * Get meta data received from the slave.
         * (Valid when first data is received and/or slave is finished)
	 * @return the job's meta data
         */
        MetaData metaData() const;

        /**
         * Query meta data received from the slave.
         * (Valid when first data is received and/or slave is finished)
	 * @param key the key of the meta data to retrieve
	 * @return the value of the meta data, or TQString::null if the
	 *         @p key does not exist
         */
        TQString queryMetaData(const TQString &key);

        /**
         * Returns the processed size for this job.
         * @see processedSize
         * @since 3.2
         */
        TDEIO::filesize_t getProcessedSize();

    signals:
        /**
         * Emitted when the job is finished, in any case (completed, canceled,
         * failed...). Use error to know the result.
	 * @param job the job that emitted this signal
         */
        void result( TDEIO::Job *job );

        /**
         * @deprecated. Don't use !
         * Emitted when the job is canceled.
         * Signal result() is emitted as well, and error() is,
         * in this case, ERR_USER_CANCELED.
	 * @param job the job that emitted this signal
         */
        void canceled( TDEIO::Job *job );

        /**
         * Emitted to display information about this job, as sent by the slave.
         * Examples of message are "Resolving host", "Connecting to host...", etc.
	 * @param job the job that emitted this signal
	 * @param msg the info message
         */
        void infoMessage( TDEIO::Job *job, const TQString & msg );
        // KDE4: Separate rich-text string from plain-text string, for different widgets.

        /**
         * Emitted to display a warning about this job, as sent by the slave.
         * @param job the job that emitted this signal
         * @param msg the info message
         * @since 3.5
         */
        void warning( TDEIO::Job *job, const TQString & msg );
        // KDE4: Separate rich-text string from plain-text string, for different widgets.

        /**
         * Emitted when the slave successfully connected to the host.
         * There is no guarantee the slave will send this, and this is
         * currently unused (in the applications).
	 * @param job the job that emitted this signal
         */
        void connected( TDEIO::Job *job );

        /**
         * Progress signal showing the overall progress of the job
         * This is valid for any kind of job, and allows using a
         * a progress bar very easily. (see KProgress).
	 * Note that this signal is not emitted for finished jobs.
	 * @param job the job that emitted this signal
	 * @param percent the percentage
         */
        void percent( TDEIO::Job *job, unsigned long percent );

        /**
         * Emitted when we know the size of this job (data size for transfers,
         * number of entries for listings).
	 * @param job the job that emitted this signal
	 * @param size the total size in bytes
         */
        void totalSize( TDEIO::Job *job, TDEIO::filesize_t size );

        /**
         * Regularly emitted to show the progress of this job
         * (current data size for transfers, entries listed).
	 * @param job the job that emitted this signal
	 * @param size the processed size in bytes
         */
        void processedSize( TDEIO::Job *job, TDEIO::filesize_t size );

        /**
         * Emitted to display information about the speed of this job.
	 * @param job the job that emitted this signal
	 * @param speed the speed in bytes/s
         */
        void speed( TDEIO::Job *job, unsigned long speed );

    protected slots:
        /**
         * Called whenever a subjob finishes.
         * Default implementation checks for errors and propagates
         * to parent job, then calls removeSubjob.
         * Override if you don't want subjobs errors to be propagated.
	 * @param job the subjob
	 * @see result()
         */
        virtual void slotResult( TDEIO::Job *job );

        /**
         * Forward signal from subjob.
	 * @param job the subjob
	 * @param speed the speed in bytes/s
	 * @see speed()
         */
        void slotSpeed( TDEIO::Job *job, unsigned long speed );
        /**
         * Forward signal from subjob.
	 * @param job the subjob
	 * @param msg the info message
	 * @see infoMessage()
         */
        void slotInfoMessage( TDEIO::Job *job, const TQString &msg );

        /**
         * Remove speed information.
         */
        void slotSpeedTimeout();

    protected:
        /**
         * Add a job that has to be finished before a result
         * is emitted. This has obviously to be called before
         * the finish signal is emitted by the slave.
         *
	 * @param job the subjob to add
         * @param inheritMetaData if true, the subjob will
         * inherit the meta data from this job.
         */
        virtual void addSubjob( Job *job, bool inheritMetaData=true );

        /**
         * Mark a sub job as being done. If it's the last to
         * wait on the job will emit a result - jobs with
         * two steps might want to override slotResult
         * in order to avoid calling this method.
	 *
	 * @param job the subjob to add
         */
        virtual void removeSubjob( Job *job );
        /**
         * Overloaded version of removeSubjob
         * @param job the subjob to remove
         * @param mergeMetaData if set, the metadata received by the subjob is
         *                      merged into this job.
         * @param emitResultIfLast if this was the last subjob, emit result,
         *                         i.e. terminate this job.
         */
        void removeSubjob( Job *job, bool mergeMetaData, bool emitResultIfLast ); // KDE4: merge with above, with =true to both

        /**
         * Utility function for inherited jobs.
         * Emits the percent signal if bigger than m_percent,
         * after calculating it from the parameters.
	 *
	 * @param processedSize the processed size in bytes
	 * @param totalSize the total size in bytes
         */
        void emitPercent( TDEIO::filesize_t processedSize, TDEIO::filesize_t totalSize );

        /**
         * Utility function for inherited jobs.
         * Emits the speed signal and starts the timer for removing that info
	 *
	 * @param speed the speed in bytes/s
         */
        void emitSpeed( unsigned long speed );

        /**
         * Utility function to emit the result signal, and suicide this job.
         * It first tells the observer to hide the progress dialog for this job.
         */
        void emitResult();

        /**
         * Set the processed size, does not emit processedSize
         * @since 3.2
         */
        void setProcessedSize(TDEIO::filesize_t size);

        /**
         * @internal
         */
        unsigned long userTimestamp() const;

        /**
         * @internal
         * Some extra storage space for jobs that don't have their own
         * private d pointer.
         */
        enum { EF_TransferJobAsync    = (1 << 0),
               EF_TransferJobNeedData = (1 << 1),
               EF_TransferJobDataSent = (1 << 2),
               EF_ListJobUnrestricted = (1 << 3) };
        int &extraFlags();

        TQPtrList<Job> subjobs;
        int m_error;
        TQString m_errorText;
        unsigned long m_percent;
        int m_progressId; // for uiserver
        TQTimer *m_speedTimer;
        TQGuardedPtr<TQWidget> m_window;
        MetaData m_outgoingMetaData;
        MetaData m_incomingMetaData;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
        class JobPrivate;
        JobPrivate *d;
    };

    /**
     * A simple job (one url and one command).
     * This is the base class for all jobs that are scheduled.
     * Other jobs are high-level jobs (CopyJob, DeleteJob, FileCopyJob...)
     * that manage subjobs but aren't scheduled directly.
     */
    class TDEIO_EXPORT SimpleJob : public TDEIO::Job {
    TQ_OBJECT

    public:
        /**
	 * Creates a new simple job. You don't need to use this constructor,
	 * unless you create a new job that inherits from SimpleJob.
	 * @param url the url of the job
	 * @param command the command of the job
	 * @param packedArgs the arguments
	 * @param showProgressInfo true to show progress information to the user
	 */
        SimpleJob(const KURL& url, int command, const TQByteArray &packedArgs,
                  bool showProgressInfo);

        ~SimpleJob();

        /**
	 * Returns the SimpleJob's URL
	 * @return the url
	 */
        const KURL& url() const { return m_url; }

        /**
         * Abort job.
         * This kills all subjobs and deletes the job.
         * @param quietly if true, Job will emit signal result
         * Should only be set to false when the user kills the job
         * (from tdeio_uiserver), not when you want to abort a job.
         */
        virtual void kill( bool quietly = true );

        /**
         * Abort job.
         * Suspends slave to be reused by another job for the same request.
         */
        virtual void putOnHold();

        /**
         * Discard suspended slave.
         */
        static void removeOnHold();

        /**
         * @internal
         * Called by the scheduler when a slave gets to
         * work on this job.
         **/
        virtual void start( Slave *slave );

        /**
         * @internal
         * Called to detach a slave from a job.
         **/
        void slaveDone();

        /**
         * @internal
         * Slave in use by this job.
         */
        Slave *slave() const { return m_slave; }

        /**
         * @internal
         */
        int command() const { return m_command; }

    public slots:
        /**
         * Forward signal from the slave
         * Can also be called by the parent job, when it knows the size.
	 * @param data_size the total size
         */
        void slotTotalSize( TDEIO::filesize_t data_size );

    protected slots:
        /**
         * Called when the slave marks the job
         * as finished.
         */
        virtual void slotFinished( );

        /**
         * @internal
         * Called on a slave's warning.
         */
        void slotWarning( const TQString & ); // KDE4: make virtual

        /**
         * Called on a slave's info message.
	 * @param s the info message
	 * @see infoMessage()
         */
        void slotInfoMessage( const TQString &s ); // KDE4: make virtual

        /**
         * Called on a slave's connected signal.
	 * @see connected()
         */
        void slotConnected();

        /**
         * Forward signal from the slave.
	 * @param data_size the processed size in bytes
	 * @see processedSize()
         */
        void slotProcessedSize( TDEIO::filesize_t data_size );
        /**
         * Forward signal from the slave.
	 * @param speed the speed in bytes/s
	 * @see speed()
         */
        void slotSpeed( unsigned long speed );

        /**
         * MetaData from the slave is received.
	 * @param _metaData the meta data
	 * @see metaData()
         */
        virtual void slotMetaData( const TDEIO::MetaData &_metaData);

    public slots:
        /**
         * @internal
         * Called on a slave's error.
         * Made public for the scheduler.
         */
        virtual void slotError( int , const TQString & );

    protected slots:
        /**
         * @internal
         */
        void slotNeedProgressId();

    protected:
        Slave * m_slave;
        TQByteArray m_packedArgs;
        KURL m_url;
        KURL m_subUrl;
        int m_command;
        TDEIO::filesize_t m_totalSize;
    protected:
	virtual void virtual_hook( int id, void* data );
	/*
	 * Allow jobs that inherit SimpleJob and are aware
	 * of redirections to store the SSL session used.
	 * Retrieval is handled by SimpleJob::start
	 * @param m_redirectionURL Reference to redirection URL,
	 * used instead of m_url if not empty
	 */
	void storeSSLSessionFromJob(const KURL &m_redirectionURL);
    private:
	class SimpleJobPrivate* d;
    };

    /**
     * A TDEIO job that retrieves information about a file or directory.
     * @see TDEIO::stat()
     */
    class TDEIO_EXPORT StatJob : public SimpleJob {

    TQ_OBJECT

    public:
        /**
	 * Do not use this constructor to create a StatJob, use TDEIO::stat() instead.
	 * @param url the url of the file or directory to check
	 * @param command the command to issue
	 * @param packedArgs the arguments
	 * @param showProgressInfo true to show progress information to the user
	 */
        StatJob(const KURL& url, int command, const TQByteArray &packedArgs, bool showProgressInfo);

        /**
	 * A stat() can have two meanings. Either we want to read from this URL,
         * or to check if we can write to it. First case is "source", second is "dest".
         * It is necessary to know what the StatJob is for, to tune the tdeioslave's behavior
         * (e.g. with FTP).
	 * @param source true for "source" mode, false for "dest" mode
         */
        void setSide( bool source ) { m_bSource = source; }

        /**
         * Selects the level of @p details we want.
         * By default this is 2 (all details wanted, including modification time, size, etc.),
         * setDetails(1) is used when deleting: we don't need all the information if it takes
         * too much time, no need to follow symlinks etc.
         * setDetails(0) is used for very simple probing: we'll only get the answer
         * "it's a file or a directory, or it doesn't exist". This is used by KRun.
	 * @param details 2 for all details, 1 for simple, 0 for very simple
         */
        void setDetails( short int details ) { m_details = details; }

        /**
         * Call this in the slot connected to result,
         * and only after making sure no error happened.
	 * @return the result of the stat
         */
        const UDSEntry & statResult() const { return m_statResult; }

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start( Slave *slave );

    signals:
        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param url the new url
         */
        void redirection( TDEIO::Job *job, const KURL &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param fromUrl the original URL
	 * @param toUrl the new URL
	 * @since 3.1
         */
        void permanentRedirection( TDEIO::Job *job, const KURL &fromUrl, const KURL &toUrl );

    protected slots:
        void slotStatEntry( const TDEIO::UDSEntry & entry );
        void slotRedirection( const KURL &url);
        virtual void slotFinished();
        virtual void slotMetaData( const TDEIO::MetaData &_metaData);

    protected:
        UDSEntry m_statResult;
        KURL m_redirectionURL;
        bool m_bSource;
        short int m_details;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
        class StatJobPrivate;
        StatJobPrivate *d;
    };

    /**
     * A TDEIO job that creates a directory
     * @see TDEIO::mkdir()
     * @since 3.3
     */
    class TDEIO_EXPORT MkdirJob : public SimpleJob {

    TQ_OBJECT

    public:
        /**
	 * Do not use this constructor to create a MkdirJob, use TDEIO::mkdir() instead.
	 * @param url the url of the file or directory to check
	 * @param command the command to issue
	 * @param packedArgs the arguments
	 * @param showProgressInfo true to show progress information to the user
	 */
        MkdirJob(const KURL& url, int command, const TQByteArray &packedArgs, bool showProgressInfo);

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start( Slave *slave );

    signals:
        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param url the new url
         */
        void redirection( TDEIO::Job *job, const KURL &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param fromUrl the original URL
	 * @param toUrl the new URL
         */
        void permanentRedirection( TDEIO::Job *job, const KURL &fromUrl, const KURL &toUrl );

    protected slots:
        void slotRedirection( const KURL &url);
        virtual void slotFinished();

    protected:
        KURL m_redirectionURL;

    protected:
	virtual void virtual_hook( int id, void* data );
    private:
        class MkdirJobPrivate;
        MkdirJobPrivate *d;
    };

    /**
     * @internal
     * Used for direct copy from or to the local filesystem (i.e. SlaveBase::copy())
     */
    class TDEIO_EXPORT DirectCopyJob : public SimpleJob {
    TQ_OBJECT

    public:
        /**
         * Do not create a DirectCopyJob. Use TDEIO::copy() or TDEIO::file_copy() instead.
         */
        DirectCopyJob(const KURL& url, int command, const TQByteArray &packedArgs,
                      bool showProgressInfo);
        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start(Slave *slave);

    signals:
        /**
         * @internal
         * Emitted if the job found an existing partial file
         * and supports resuming. Used by FileCopyJob.
         */
        void canResume( TDEIO::Job *job, TDEIO::filesize_t offset );

    private slots:
        void slotCanResume( TDEIO::filesize_t offset );
    };


    /**
     * The transfer job pumps data into and/or out of a Slave.
     * Data is sent to the slave on request of the slave ( dataReq).
     * If data coming from the slave can not be handled, the
     * reading of data from the slave should be suspended.
     */
    class TDEIO_EXPORT TransferJob : public SimpleJob {
    TQ_OBJECT

    public:
       /**
	* Do not create a TransferJob. Use TDEIO::get() or TDEIO::put()
	* instead.
	* @param url the url to get or put
	* @param command the command to issue
	* @param packedArgs the arguments
	* @param _staticData additional data to transmit (e.g. in a HTTP Post)
	* @param showProgressInfo true to show progress information to the user
	*/
        TransferJob(const KURL& url, int command,
                    const TQByteArray &packedArgs,
                    const TQByteArray &_staticData,
                    bool showProgressInfo);

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start(Slave *slave);

        /**
         * Called when m_subJob finishes.
	 * @param job the job that finished
         */
        virtual void slotResult( TDEIO::Job *job );

        /**
         * Flow control. Suspend data processing from the slave.
         */
        void suspend();

        /**
         * Flow control. Resume data processing from the slave.
         */
        void resume();

        /**
         * Flow control.
	 * @return true if the job is suspended
         */
	bool isSuspended() const { return m_suspended; }


        /**
	 * Checks whether we got an error page. This currently only happens
	 * with HTTP urls. Call this from your slot connected to result().
	 *
         * @return true if we got an (HTML) error page from the server
         * instead of what we asked for.
         */
        bool isErrorPage() const { return m_errorPage; }

        /**
         * Enable the async data mode.
         * When async data is enabled, data should be provided to the job by
         * calling sendAsyncData() instead of returning data in the
         * dataReq() signal.
         * @since 3.2
         */
        void setAsyncDataEnabled(bool enabled);

        /**
         * Provide data to the job when async data is enabled.
         * Should be called exactly once after receiving a dataReq signal
         * Sending an empty block indicates end of data.
         * @since 3.2
         */
        void sendAsyncData(const TQByteArray &data);

        /**
         * When enabled, the job reports the amount of data that has been sent,
         * instead of the amount of data that that has been received.
         * @see slotProcessedSize
         * @see slotSpeed
         * @since 3.2
         */
        void setReportDataSent(bool enabled);

        /**
         *  Returns whether the job reports the amount of data that has been
         *  sent (true), or whether the job reports the amount of data that
         * has been received (false)
         * @since 3.2
         */
        bool reportDataSent();

    signals:
        /**
         * Data from the slave has arrived.
         * @param job the job that emitted this signal
         * @param data data received from the slave.
         *
         * End of data (EOD) has been reached if data.size() == 0, however, you
         * should not be certain of data.size() == 0 ever happening (e.g. in case
         * of an error), so you should rely on result() instead.
         */
        void data( TDEIO::Job *job, const TQByteArray &data );

        /**
         * Request for data.
         * Please note, that you shouldn't put too large chunks
         * of data in it as this requires copies within the frame
         * work, so you should rather split the data you want
         * to pass here in reasonable chunks (about 1MB maximum)
         *
	 * @param job the job that emitted this signal
         * @param data buffer to fill with data to send to the
         * slave. An empty buffer indicates end of data. (EOD)
         */
        void dataReq( TDEIO::Job *job, TQByteArray &data );

        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
	 * @param job the job that emitted this signal
	 * @param url the new URL
         */
        void redirection( TDEIO::Job *job, const KURL &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @param job the job that emitted this signal
	 * @param fromUrl the original URL
	 * @param toUrl the new URL
	 * @since 3.1
         */
        void permanentRedirection( TDEIO::Job *job, const KURL &fromUrl, const KURL &toUrl );

        /**
         * Mimetype determined.
	 * @param job the job that emitted this signal
	 * @param type the mime type
         */
        void mimetype( TDEIO::Job *job, const TQString &type );

        /**
         * @internal
         * Emitted if the "put" job found an existing partial file
         * (in which case offset is the size of that file)
         * and emitted by the "get" job if it supports resuming to
         * the given offset - in this case @p offset is unused)
         */
        void canResume( TDEIO::Job *job, TDEIO::filesize_t offset );


    protected slots:
        virtual void slotRedirection( const KURL &url);
        virtual void slotFinished();
        virtual void slotData( const TQByteArray &data);
        virtual void slotDataReq();
        virtual void slotMimetype( const TQString &mimetype );
        virtual void slotNeedSubURLData();
        virtual void slotSubURLData(TDEIO::Job*, const TQByteArray &);
        virtual void slotMetaData( const TDEIO::MetaData &_metaData);
        void slotErrorPage();
        void slotCanResume( TDEIO::filesize_t offset );
        void slotPostRedirection();

    protected:
        bool m_suspended;
        bool m_errorPage;
        TQByteArray staticData;
        KURL m_redirectionURL;
        KURL::List m_redirectionList;
        TQString m_mimetype;
        TransferJob *m_subJob;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class TransferJobPrivate *d;
    };

    /**
     * StoredTransferJob is a TransferJob (for downloading or uploading data) that
     * also stores a TQByteArray with the data, making it simpler to use than the
     * standard TransferJob.
     *
     * For TDEIO::storedGet it puts the data into the member TQByteArray, so the user
     * of this class can get hold of the whole data at once by calling data()
     * when the result signal is emitted.
     * You should only use StoredTransferJob to download data if you cannot
     * process the data by chunks while it's being downloaded, since storing
     * everything in a TQByteArray can potentially require a lot of memory.
     *
     * For TDEIO::storedPut the user of this class simply provides the bytearray from
     * the start, and the job takes care of uploading it.
     * You should only use StoredTransferJob to upload data if you cannot
     * provide the in chunks while it's being uploaded, since storing
     * everything in a TQByteArray can potentially require a lot of memory.
     *
     * @since 3.3
     */
    class TDEIO_EXPORT StoredTransferJob : public TDEIO::TransferJob {
        TQ_OBJECT

    public:
       /**
	* Do not create a StoredTransferJob. Use storedGet() or storedPut()
	* instead.
	* @param url the url to get or put
	* @param command the command to issue
	* @param packedArgs the arguments
	* @param _staticData additional data to transmit (e.g. in a HTTP Post)
	* @param showProgressInfo true to show progress information to the user
	*/
        StoredTransferJob(const KURL& url, int command,
                          const TQByteArray &packedArgs,
                          const TQByteArray &_staticData,
                          bool showProgressInfo);

        /**
         * Set data to be uploaded. This is for put jobs.
         * Automatically called by TDEIO::storedPut(const TQByteArray &, ...),
         * do not call this yourself.
         */
        void setData( const TQByteArray& arr );

        /**
         * Get hold of the downloaded data. This is for get jobs.
         * You're supposed to call this only from the slot connected to the result() signal.
         */
        TQByteArray data() const { return m_data; }

    private slots:
        void slotStoredData( TDEIO::Job *job, const TQByteArray &data );
        void slotStoredDataReq( TDEIO::Job *job, TQByteArray &data );
    private:
        TQByteArray m_data;
        int m_uploadOffset;
    };

    /**
     * The MultiGetJob is a TransferJob that allows you to get
     * several files from a single server. Don't create directly,
     * but use TDEIO::multi_get() instead.
     * @see TDEIO::multi_get()
     */
    class TDEIO_EXPORT MultiGetJob : public TransferJob {
    TQ_OBJECT

    public:
        /**
	 * Do not create a MultiGetJob directly, use TDEIO::multi_get()
	 * instead.
	 *
	 * @param url the first url to get
	 * @param showProgressInfo true to show progress information to the user
	 */
        MultiGetJob(const KURL& url, bool showProgressInfo);

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
         virtual void start(Slave *slave);

	/**
	 * Get an additional file.
	 *
	 * @param id the id of the file
	 * @param url the url of the file to get
	 * @param metaData the meta data for this request
	 */
        void get(long id, const KURL &url, const MetaData &metaData);

    signals:
        /**
         * Data from the slave has arrived.
	 * @param id the id of the request
         * @param data data received from the slave.
         * End of data (EOD) has been reached if data.size() == 0
         */
        void data( long id, const TQByteArray &data);

        /**
         * Mimetype determined
	 * @param id the id of the request
	 * @param type the mime type
         */
        void mimetype( long id, const TQString &type );

        /**
         * File transfer completed.
         *
         * When all files have been processed, result(TDEIO::Job *) gets
         * emitted.
	 * @param id the id of the request
         */
        void result( long id);

    protected slots:
        virtual void slotRedirection( const KURL &url);
        virtual void slotFinished();
        virtual void slotData( const TQByteArray &data);
        virtual void slotMimetype( const TQString &mimetype );
    private:
        struct GetRequest {
        public:
           GetRequest(long _id, const KURL &_url, const MetaData &_metaData)
             : id(_id), url(_url), metaData(_metaData) { }
           long id;
           KURL url;
           MetaData metaData;
        };
        bool findCurrentEntry();
        void flushQueue(TQPtrList<GetRequest> &queue);

        TQPtrList<GetRequest> m_waitQueue;
        TQPtrList<GetRequest> m_activeQueue;
        bool b_multiGetActive;
        GetRequest *m_currentEntry;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class MultiGetJobPrivate* d;
    };

    /**
     * A MimetypeJob is a TransferJob that  allows you to get
     * the mime type of an URL. Don't create directly,
     * but use TDEIO::mimetype() instead.
     * @see TDEIO::mimetype()
     */
    class TDEIO_EXPORT MimetypeJob : public TransferJob {
    TQ_OBJECT

    public:
       /**
	* Do not create a MimetypeJob directly. Use TDEIO::mimetype()
	* instead.
	* @param url the url to get
	* @param command the command to issue
	* @param packedArgs the arguments
	* @param showProgressInfo true to show progress information to the user
	*/
        MimetypeJob(const KURL& url, int command, const TQByteArray &packedArgs, bool showProgressInfo);

        /**
         * Call this in the slot connected to result,
         * and only after making sure no error happened.
	 * @return the mimetype of the URL
         */
         TQString mimetype() const { return m_mimetype; }

        /**
	 * @internal
         * Called by the scheduler when a slave gets to
         * work on this job.
	 * @param slave the slave that works on the job
         */
        virtual void start( Slave *slave );

    protected slots:
        virtual void slotFinished( );
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class MimetypeJobPrivate* d;
    };

    /**
     * The FileCopyJob copies data from one place to another.
     * @see TDEIO::file_copy()
     * @see TDEIO::file_move()
     */
    class TDEIO_EXPORT FileCopyJob : public Job {
    TQ_OBJECT

    public:
	/**
	* Do not create a FileCopyJob directly. Use TDEIO::file_move()
	* or TDEIO::file_copy() instead.
	* @param src the source URL
	* @param dest the destination URL
	* @param permissions the permissions of the resulting resource
	* @param move true to move, false to copy
	* @param overwrite true to allow overwriting, false otherwise
	* @param resume true to resume an operation, false otherwise
	* @param showProgressInfo true to show progress information to the user
	 */
        FileCopyJob( const KURL& src, const KURL& dest, int permissions,
                     bool move, bool overwrite, bool resume, bool showProgressInfo);

        ~FileCopyJob();
        /**
         * If you know the size of the source file, call this method
         * to inform this job. It will be displayed in the "resume" dialog.
	 * @param size the size of the source file
	 * @since 3.2
         */
        void setSourceSize64(TDEIO::filesize_t size);

        /**
         * Sets the modification time of the file
         *
         * Note that this is ignored if a direct copy (SlaveBase::copy) can be done,
         * in which case the mtime of the source is applied to the destination (if the protocol
         * supports the concept).
         */
        void setModificationTime( time_t mtime );

        /**
         * @deprecated
         */
        void setSourceSize( off_t size ) TDE_DEPRECATED;

	/**
	 * Returns the source URL.
	 * @return the source URL
	 */
        KURL srcURL() const { return m_src; }

	/**
	 * Returns the destination URL.
	 * @return the destination URL
	 */
        KURL destURL() const { return m_dest; }

    signals:
        /**
         * Mimetype determined during a file copy.
         * This is never emitted during a move, and might not be emitted during
         * a copy, depending on the slave.
         * @param job the job that emitted this signal
         * @param type the mime type
         *
         * @since 3.5.7
         */
        void mimetype( TDEIO::Job *job, const TQString &type );

    public slots:
        void slotStart();
        void slotData( TDEIO::Job *, const TQByteArray &data);
        void slotDataReq( TDEIO::Job *, TQByteArray &data);
        void slotMimetype( TDEIO::Job *, const TQString& type );

    protected slots:
        /**
         * Called whenever a subjob finishes.
	 * @param job the job that emitted this signal
         */
        virtual void slotResult( TDEIO::Job *job );

        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param size the processed size in bytes
         */
        void slotProcessedSize( TDEIO::Job *job, TDEIO::filesize_t size );
        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param size the total size
         */
        void slotTotalSize( TDEIO::Job *job, TDEIO::filesize_t size );
        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param pct the percentage
         */
        void slotPercent( TDEIO::Job *job, unsigned long pct );
        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param offset the offset to resume from
         */
        void slotCanResume( TDEIO::Job *job, TDEIO::filesize_t offset );

    protected:
        void startCopyJob();
        void startCopyJob(const KURL &slave_url);
        void startRenameJob(const KURL &slave_url);
        void startDataPump();
        void connectSubjob( SimpleJob * job );

    private:
        void startBestCopyMethod();

    protected:
        KURL m_src;
        KURL m_dest;
        int m_permissions;
        bool m_move:1;
        bool m_overwrite:1;
        bool m_resume:1;
        bool m_canResume:1;
        bool m_resumeAnswerSent:1;
        TQByteArray m_buffer;
        SimpleJob *m_moveJob;
        SimpleJob *m_copyJob;
        TransferJob *m_getJob;
        TransferJob *m_putJob;
        TDEIO::filesize_t m_totalSize;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class FileCopyJobPrivate;
	FileCopyJobPrivate* d;
    };

    /**
     * A ListJob is allows you to get the get the content of a directory.
     * Don't create the job directly, but use TDEIO::listRecursive() or
     * TDEIO::listDir() instead.
     * @see TDEIO::listRecursive()
     * @see TDEIO::listDir()
     */
    class TDEIO_EXPORT ListJob : public SimpleJob {
    TQ_OBJECT

    public:
       /**
	* Do not create a ListJob directly. Use TDEIO::listDir() or
	* TDEIO::listRecursive() instead.
	* @param url the url of the directory
	* @param showProgressInfo true to show progress information to the user
	* @param recursive true to get the data recursively from child directories,
	*        false to get only the content of the specified dir
	* @param prefix the prefix of the files, or TQString::null for no prefix
	* @param includeHidden true to include hidden files (those starting with '.')
	*/
        ListJob(const KURL& url, bool showProgressInfo,
                bool recursive = false, TQString prefix = TQString::null,
                bool includeHidden = true);

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start( Slave *slave );

        /**
         * Returns the ListJob's redirection URL. This will be invalid if there
         * was no redirection.
         * @return the redirection url
         * @since 3.4.1
         */
        const KURL& redirectionURL() const { return m_redirectionURL; }

        /**
         * Do not apply any KIOSK restrictions to this job.
         * @since 3.2
         */
        void setUnrestricted(bool unrestricted);

    signals:
        /**
         * This signal emits the entry found by the job while listing.
         * The progress signals aren't specific to ListJob. It simply
         * uses SimpleJob's processedSize (number of entries listed) and
         * totalSize (total number of entries, if known),
         * as well as percent.
	 * @param job the job that emitted this signal
	 * @param list the list of UDSEntries
         */
        void entries( TDEIO::Job *job, const TDEIO::UDSEntryList& list);

        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param url the new url
         */
        void redirection( TDEIO::Job *job, const KURL &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @param job the job that emitted this signal
	 * @param fromUrl the original URL
	 * @param toUrl the new URL
	 * @since 3.1
         */
        void permanentRedirection( TDEIO::Job *job, const KURL &fromUrl, const KURL &toUrl );

    protected slots:
        virtual void slotFinished( );
        virtual void slotMetaData( const TDEIO::MetaData &_metaData);
        virtual void slotResult( TDEIO::Job *job );
        void slotListEntries( const TDEIO::UDSEntryList& list );
        void slotRedirection( const KURL &url );
        void gotEntries( TDEIO::Job * subjob, const TDEIO::UDSEntryList& list );

    private:
        bool recursive;
        bool includeHidden;
        TQString prefix;
        unsigned long m_processedEntries;
        KURL m_redirectionURL;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class ListJobPrivate* d;
    };

    /// @internal
    struct TDEIO_EXPORT CopyInfo
    {
        KURL uSource;
        KURL uDest;
        TQString linkDest; // for symlinks only
        int permissions;
        //mode_t type;
        time_t ctime;
        time_t mtime;
        TDEIO::filesize_t size; // 0 for dirs
    };

    /**
     * CopyJob is used to move, copy or symlink files and directories.
     * Don't create the job directly, but use TDEIO::copy(),
     * TDEIO::move(), TDEIO::link() and friends.
     *
     * @see TDEIO::copy()
     * @see TDEIO::copyAs()
     * @see TDEIO::move()
     * @see TDEIO::moveAs()
     * @see TDEIO::link()
     * @see TDEIO::linkAs()
     */
    class TDEIO_EXPORT CopyJob : public Job {
    TQ_OBJECT

    public:
	/**
	 * Defines the mode of the operation
	 */
        enum CopyMode{ Copy, Move, Link };

	/**
	 * Do not create a CopyJob directly. Use TDEIO::copy(),
	 * TDEIO::move(), TDEIO::link() and friends instead.
	 *
	 * @param src the list of source URLs
	 * @param dest the destination URL
	 * @param mode specifies whether the job should copy, move or link
	 * @param asMethod if true, behaves like TDEIO::copyAs(),
	 * TDEIO::moveAs() or TDEIO::linkAs()
	 * @param showProgressInfo true to show progress information to the user
	 * @see TDEIO::copy()
	 * @see TDEIO::copyAs()
	 * @see TDEIO::move()
	 * @see TDEIO::moveAs()
	 * @see TDEIO::link()
	 * @see TDEIO::linkAs()
	 */
        CopyJob( const KURL::List& src, const KURL& dest, CopyMode mode, bool asMethod, bool showProgressInfo );

        virtual ~CopyJob();

	/**
	 * Returns the list of source URLs.
	 * @return the list of source URLs.
	 */
        KURL::List srcURLs() const { return m_srcList; }

	/**
	 * Returns the destination URL.
	 * @return the destination URL
	 */
        KURL destURL() const { return m_dest; }

        /**
         * By default the permissions of the copied files will be those of the source files.
         *
         * But when copying "template" files to "new" files, people prefer the umask
         * to apply, rather than the template's permissions.
         * For that case, call setDefaultPermissions(true)
         *
         * TODO KDE4: consider adding this as bool to copy/copyAs?
         * @since 3.2.3
         */
        void setDefaultPermissions( bool b );

        /**
         * When an error happens while copying/moving a file, the user will be presented with
         * a dialog for skipping the file that can't be copied/moved.
         * Or if the error is that the destination file already exists, the standard
         * rename dialog is shown.
         * If the program doesn't want CopyJob to show dialogs, but to simply fail on error,
         * call setInteractive( false ).
         *
         * KDE4: remove, already in Job
         * @since 3.4
         */
        void setInteractive( bool b );

    signals:

        /**
	 * Emitted when the total number of files is known.
	 * @param job the job that emitted this signal
	 * @param files the total number of files
	 */
        void totalFiles( TDEIO::Job *job, unsigned long files );
        /**
	 * Emitted when the toal number of direcotries is known.
	 * @param job the job that emitted this signal
	 * @param dirs the total number of directories
	 */
        void totalDirs( TDEIO::Job *job, unsigned long dirs );

        /**
	 * Emitted when it is known which files / directories are going
	 * to be created. Note that this may still change e.g. when
	 * existing files with the same name are discovered.
	 * @param job the job that emitted this signal
	 * @param files a list of items that are about to be created.
	 */
        void aboutToCreate( TDEIO::Job *job, const TQValueList<TDEIO::CopyInfo> &files);

        /**
	 * Sends the number of processed files.
	 * @param job the job that emitted this signal
	 * @param files the number of processed files
	 */
        void processedFiles( TDEIO::Job *job, unsigned long files );
        /**
	 * Sends the number of processed directories.
	 * @param job the job that emitted this signal
	 * @param dirs the number of processed dirs
	 */
        void processedDirs( TDEIO::Job *job, unsigned long dirs );

        /**
         * The job is copying a file or directory.
	 * @param job the job that emitted this signal
	 * @param from the URl of the file or directory that is currently
	 *             being copied
	 * @param to the destination of the current operation
         */
        void copying( TDEIO::Job *job, const KURL& from, const KURL& to );
        /**
         * The job is creating a symbolic link.
	 * @param job the job that emitted this signal
	 * @param target the URl of the file or directory that is currently
	 *             being linked
	 * @param to the destination of the current operation
         */
        void linking( TDEIO::Job *job, const TQString& target, const KURL& to );
        /**
         * The job is moving a file or directory.
	 * @param job the job that emitted this signal
	 * @param from the URl of the file or directory that is currently
	 *             being moved
	 * @param to the destination of the current operation
         */
        void moving( TDEIO::Job *job, const KURL& from, const KURL& to );
        /**
         * The job is creating the directory @p dir.
	 * @param job the job that emitted this signal
	 * @param dir the directory that is currently being created
         */
        void creatingDir( TDEIO::Job *job, const KURL& dir );
        /**
         * The user chose to rename @p from to @p to.
	 * @param job the job that emitted this signal
	 * @param from the original name
	 * @param to the new name
         */
        void renamed( TDEIO::Job *job, const KURL& from, const KURL& to );

        /**
         * The job emits this signal when copying or moving a file or directory successfully finished.
         * This signal is mainly for the Undo feature.
	 *
	 * @param job the job that emitted this signal
         * @param from the source URL
         * @param to the destination URL
         * @param directory indicates whether a file or directory was successfully copied/moved.
	 *                  true for a directoy, false for file
         * @param renamed indicates that the destination URL was created using a
         * rename operation (i.e. fast directory moving). true if is has been renamed
         */
        void copyingDone( TDEIO::Job *job, const KURL &from, const KURL &to, bool directory, bool renamed );
        /**
         * The job is copying or moving a symbolic link, that points to target.
         * The new link is created in @p to. The existing one is/was in @p from.
         * This signal is mainly for the Undo feature.
	 * @param job the job that emitted this signal
         * @param from the source URL
	 * @param target the target
         * @param to the destination URL
         */
        void copyingLinkDone( TDEIO::Job *job, const KURL &from, const TQString& target, const KURL& to );

    protected:
        void statCurrentSrc();
        void statNextSrc();

        // Those aren't slots but submethods for slotResult.
        void slotResultStating( TDEIO::Job * job );
        void startListing( const KURL & src );
        void slotResultCreatingDirs( TDEIO::Job * job );
        void slotResultConflictCreatingDirs( TDEIO::Job * job );
        void createNextDir();
        void slotResultCopyingFiles( TDEIO::Job * job );
        void slotResultConflictCopyingFiles( TDEIO::Job * job );
        void copyNextFile();
        void slotResultDeletingDirs( TDEIO::Job * job );
        void deleteNextDir();
        void skip( const KURL & sourceURL );
        void slotResultRenaming( TDEIO::Job * job );
        //void slotResultSettingDirAttributes( TDEIO::Job * job );
        void setNextDirAttribute();
    private:
        void startRenameJob(const KURL &slave_url);
        bool shouldOverwrite( const TQString& path ) const;
        bool shouldSkip( const TQString& path ) const;
        void skipSrc();

    protected slots:
        void slotStart();
        void slotEntries( TDEIO::Job*, const TDEIO::UDSEntryList& list );
        virtual void slotResult( TDEIO::Job *job );
        /**
         * Forward signal from subjob
         */
        void slotProcessedSize( TDEIO::Job*, TDEIO::filesize_t data_size );
        /**
         * Forward signal from subjob
	 * @param size the total size
         */
        void slotTotalSize( TDEIO::Job*, TDEIO::filesize_t size );

        void slotReport();
    private:
        CopyMode m_mode;
        bool m_asMethod;
        enum DestinationState { DEST_NOT_STATED, DEST_IS_DIR, DEST_IS_FILE, DEST_DOESNT_EXIST };
        DestinationState destinationState;
        enum { STATE_STATING, STATE_RENAMING, STATE_LISTING, STATE_CREATING_DIRS,
               STATE_CONFLICT_CREATING_DIRS, STATE_COPYING_FILES, STATE_CONFLICT_COPYING_FILES,
               STATE_DELETING_DIRS, STATE_SETTING_DIR_ATTRIBUTES } state;
        TDEIO::filesize_t m_totalSize;
        TDEIO::filesize_t m_processedSize;
        TDEIO::filesize_t m_fileProcessedSize;
        int m_processedFiles;
        int m_processedDirs;
        TQValueList<CopyInfo> files;
        TQValueList<CopyInfo> dirs;
        KURL::List dirsToRemove;
        KURL::List m_srcList;
        KURL::List::Iterator m_currentStatSrc;
        bool m_bCurrentSrcIsDir;
        bool m_bCurrentOperationIsLink;
        bool m_bSingleFileCopy;
        bool m_bOnlyRenames;
        KURL m_dest;
        KURL m_currentDest;
        //
        TQStringList m_skipList;
        TQStringList m_overwriteList;
        bool m_bAutoSkip;
        bool m_bOverwriteAll;
        int m_conflictError;

        TQTimer *m_reportTimer;
        //these both are used for progress dialog reporting
        KURL m_currentSrcURL;
        KURL m_currentDestURL;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class CopyJobPrivate;
        CopyJobPrivate* d;
        friend class CopyJobPrivate; // for DestinationState
    };

    /**
     * A more complex Job to delete files and directories.
     * Don't create the job directly, but use TDEIO::del() instead.
     *
     * @see TDEIO::del()
     */
    class TDEIO_EXPORT DeleteJob : public Job {
    TQ_OBJECT

    public:
	/**
	 * Do not create a DeleteJob directly. Use TDEIO::del()
	 * instead.
	 *
	 * @param src the list of URLs to delete
	 * @param shred true to shred (make sure that data is not recoverable)a
	 * @param showProgressInfo true to show progress information to the user
	 * @see TDEIO::del()
	 */
        DeleteJob( const KURL::List& src, bool shred, bool showProgressInfo );

	/**
	 * Returns the list of URLs.
	 * @return the list of URLs.
	 */
        KURL::List urls() const { return m_srcList; }

    signals:

        /**
	 * Emitted when the total number of files is known.
	 * @param job the job that emitted this signal
	 * @param files the total number of files
	 */
        void totalFiles( TDEIO::Job *job, unsigned long files );
        /**
	 * Emitted when the toal number of direcotries is known.
	 * @param job the job that emitted this signal
	 * @param dirs the total number of directories
	 */
        void totalDirs( TDEIO::Job *job, unsigned long dirs );

        /**
	 * Sends the number of processed files.
	 * @param job the job that emitted this signal
	 * @param files the number of processed files
	 */
        void processedFiles( TDEIO::Job *job, unsigned long files );
        /**
	 * Sends the number of processed directories.
	 * @param job the job that emitted this signal
	 * @param dirs the number of processed dirs
	 */
        void processedDirs( TDEIO::Job *job, unsigned long dirs );

        /**
	 * Sends the URL of the file that is currently being deleted.
	 * @param job the job that emitted this signal
	 * @param file the URL of the file or directory that is being
	 *        deleted
	 */
        void deleting( TDEIO::Job *job, const KURL& file );

    protected slots:
        void slotStart();
        void slotEntries( TDEIO::Job*, const TDEIO::UDSEntryList& list );
        virtual void slotResult( TDEIO::Job *job );

        /**
         * Forward signal from subjob
         */
        void slotProcessedSize( TDEIO::Job*, TDEIO::filesize_t data_size );
        void slotReport();

    private:
        void statNextSrc();
        void deleteNextFile();
        void deleteNextDir();

    private:
        enum { STATE_STATING, STATE_LISTING,
               STATE_DELETING_FILES, STATE_DELETING_DIRS } state;
        TDEIO::filesize_t m_totalSize;
        TDEIO::filesize_t m_processedSize;
        TDEIO::filesize_t m_fileProcessedSize;
        int m_processedFiles;
        int m_processedDirs;
        int m_totalFilesDirs;
        KURL m_currentURL;
        KURL::List files;
        KURL::List symlinks;
        KURL::List dirs;
        KURL::List m_srcList;
        KURL::List::Iterator m_currentStat;
	TQStringList m_parentDirs;
        bool m_shred; // BIC: remove in KDE4
        TQTimer *m_reportTimer;
    protected:
        /** \internal */
	virtual void virtual_hook( int id, void* data );
    private:
	class DeleteJobPrivate* d;
    };

    /**
     * A TDEIO job that finds a local URL
     * @see TDEIO::localURL()
     * @since R14.0.0
     */
    class TDEIO_EXPORT LocalURLJob : public SimpleJob {

    TQ_OBJECT

    public:
        /**
	 * Do not use this constructor to create a LocalURLJob, use TDEIO::localURL() instead.
	 * @param url the url of the file or directory to check
	 * @param command the command to issue
	 * @param packedArgs the arguments
	 * @param showProgressInfo true to show progress information to the user
	 */
        LocalURLJob(const KURL& url, int command, const TQByteArray &packedArgs, bool showProgressInfo);

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start( Slave *slave );

    signals:
        /**
	 * @param job the job that emitted this signal
	 * @param url the local url
	 * @param isLocal true if the returned URL is local, false if not
         */
        void localURL( TDEIO::LocalURLJob *job, const KURL &url, bool isLocal );

    protected slots:
        void slotLocalURL( const KURL &url, bool isLocal );
        virtual void slotFinished();

    protected:
	virtual void virtual_hook( int id, void* data );
    private:
        class LocalURLJobPrivate;
        LocalURLJobPrivate *d;
    };

}

#endif
