/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>

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
#ifndef __progressbase_h__
#define __progressbase_h__


#include <tqwidget.h>

#include <tdeio/global.h>

class KURL;
namespace TDEIO {
  class Job;
  class CopyJob;
  class DeleteJob;
}

namespace TDEIO
{
  enum Progress {
    DEFAULT = 1,
    STATUSBAR = 2,
    LIST = 3
  };

/**
* This class does all initialization stuff for progress,
* like connecting signals to slots.
* All slots are implemented as pure virtual methods.
*
* All custom IO progress dialog should inherit this class.
* Add your GUI code to the constructor and implemement those virtual
* methods which you need in order to display progress.
*
* E.g. StatusbarProgress only implements slotTotalSize(),
* slotPercent() and slotSpeed().
*
* Custom progress dialog will be used like this :
* \code
* // create job
* CopyJob* job = TDEIO::copy(...);
* // create a dialog
* MyCustomProgress *customProgress;
* customProgress = new MyCustomProgress();
* // connect progress with job
* customProgress->setJob( job );
* ...
* \endcode
*
* There is a special method setStopOnClose() that controls the behavior of
* the dialog.
* @short Base class for IO progress dialogs.
* @author Matej Koss <koss@miesto.sk>
*/
class TDEIO_EXPORT ProgressBase : public TQWidget {

  TQ_OBJECT

public:

  /**
   * Creates a new progress dialog.
   * @param parent the parent of this dialog window, or 0
   */
  ProgressBase( TQWidget *parent );
  ~ProgressBase() {}

  /**
   * Assign a TDEIO::Job to this progress dialog.
   * @param job the job to assign
   */
  void setJob( TDEIO::Job *job );
  /**
   * Assign a TDEIO::Job to this progress dialog.
   * @param job the job to assign
   */
  void setJob( TDEIO::CopyJob *job );
  /**
   * Assign a TDEIO::Job to this progress dialog.
   * @param job the job to assign
   */
  void setJob( TDEIO::DeleteJob *job );

  // should we stop the job when the dialog is closed ?
  void setStopOnClose( bool stopOnClose ) { m_bStopOnClose = stopOnClose; }
  bool stopOnClose() const { return m_bStopOnClose; }

  // should we delete the dialog or just clean it when the job is finished ?
  /**
   * This controls whether the dialog should be deleted or only cleaned when
   * the TDEIO::Job is finished (or canceled).
   *
   * If your dialog is an embedded widget and not a separate window, you should
   * setOnlyClean(true) in the constructor of your custom dialog.
   *
   * @param onlyClean If true the dialog will only call method slotClean.
   * If false the dialog will be deleted.
   * @see onlyClean()
   */
  void setOnlyClean( bool onlyClean ) { m_bOnlyClean = onlyClean; }

  /**
   * Checks whether the dialog should be deleted or cleaned.
   * @return true if the dialog only calls slotClean, false if it will be
   *         deleted
   * @see setOnlyClean()
   */
  bool onlyClean() const { return m_bOnlyClean; }

  /**
   * Call when the operation finished.
   * @since 3.1
   */
  void finished();

public slots:
  /**
   * This method should be called for correct cancellation of IO operation
   * Connect this to the progress widgets buttons etc.
   */
  void slotStop();
  /**
   * This method is called when the widget should be cleaned (after job is finished).
   * redefine this for custom behavior.
   */
  virtual void slotClean();

  // progress slots
  /**
   * Called to set the total size.
   * @param job the TDEIO::Job
   * @param size the total size in bytes
   */
  virtual void slotTotalSize( TDEIO::Job* job, TDEIO::filesize_t size ) {
    Q_UNUSED(job);Q_UNUSED(size);}
  /**
   * Called to set the total number of files.
   * @param job the TDEIO::Job
   * @param files the number of files
   */
  virtual void slotTotalFiles( TDEIO::Job* job, unsigned long files ) {
    Q_UNUSED(job);Q_UNUSED(files);}
  /**
   * Called to set the total number of directories.
   * @param job the TDEIO::Job
   * @param dirs the number of directories
   */
  virtual void slotTotalDirs( TDEIO::Job* job, unsigned long dirs ) {
    Q_UNUSED(job);Q_UNUSED(dirs);}

  /**
   * Called to set the processed size.
   * @param job the TDEIO::Job
   * @param bytes the processed size in bytes
   */
  virtual void slotProcessedSize( TDEIO::Job* job, TDEIO::filesize_t bytes ) {
    Q_UNUSED(job);Q_UNUSED(bytes);}
  /**
   * Called to set the number of processed files.
   * @param job the TDEIO::Job
   * @param files the number of files
   */
  virtual void slotProcessedFiles( TDEIO::Job* job, unsigned long files ) {
    Q_UNUSED(job);Q_UNUSED(files);}
  /**
   * Called to set the number of processed directories.
   * @param job the TDEIO::Job
   * @param dirs the number of directories
   */
  virtual void slotProcessedDirs( TDEIO::Job* job, unsigned long dirs ) {
    Q_UNUSED(job);Q_UNUSED(dirs);}

  /**
   * Called to set the speed.
   * @param job the TDEIO::Job
   * @param speed the speed in bytes/second
   */
  virtual void slotSpeed( TDEIO::Job* job, unsigned long speed ) {
    Q_UNUSED(job);Q_UNUSED(speed);}

  /**
   * Called to set the percentage.
   * @param job the TDEIO::Job
   * @param percent the percentage
   */
  virtual void slotPercent( TDEIO::Job* job, unsigned long percent ) {
    Q_UNUSED(job);Q_UNUSED(percent);}

  /**
   * Called when the job is copying.
   * @param job the TDEIO::Job
   * @param src the source of the operation
   * @param dest the destination of the operation
   */
  virtual void slotCopying( TDEIO::Job* job, const KURL& src, const KURL& dest ) {
    Q_UNUSED(job);Q_UNUSED(src);Q_UNUSED(dest);}
  /**
   * Called when the job is moving.
   * @param job the TDEIO::Job
   * @param src the source of the operation
   * @param dest the destination of the operation
   */
  virtual void slotMoving( TDEIO::Job* job, const KURL& src, const KURL& dest ) {
    Q_UNUSED(job);Q_UNUSED(src);Q_UNUSED(dest);}
  /**
   * Called when the job is deleting.
   * @param job the TDEIO::Job
   * @param url the URL to delete
   */
  virtual void slotDeleting( TDEIO::Job* job, const KURL& url) {
    Q_UNUSED(job);Q_UNUSED(url);}
  /**
   * Called when the job is creating a directory.
   * @param job the TDEIO::Job
   * @param dir the URL of the directory to create
   */
  virtual void slotCreatingDir( TDEIO::Job* job, const KURL& dir ) {
    Q_UNUSED(job);Q_UNUSED(dir);}

  /**
   * Called when the job is resuming..
   * @param job the TDEIO::Job
   * @param from the position to resume from in bytes
   */
  virtual void slotCanResume( TDEIO::Job* job, TDEIO::filesize_t from) {
    Q_UNUSED(job);Q_UNUSED(from);}

signals:
  /**
   * Called when the operation stopped.
   */
  void stopped();

protected slots:
  void slotFinished( TDEIO::Job* );

protected:

  virtual void closeEvent( TQCloseEvent * );

  TDEIO::Job* m_pJob;

private:
  bool m_bOnlyClean;
  bool m_bStopOnClose;


protected:
    virtual void virtual_hook( int id, void* data );
private:
    class ProgressBasePrivate* d;
};

} /* namespace */

#endif // __progressbase_h__
