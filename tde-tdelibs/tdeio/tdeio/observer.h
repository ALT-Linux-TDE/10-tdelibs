/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>
                      David Faure <faure@kde.org>

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
#ifndef __tdeio_observer_h__
#define __tdeio_observer_h__

#include <tqobject.h>
#include <dcopobject.h>
#include <tqintdict.h>

#include <tdeio/global.h>
#include <tdeio/authinfo.h>
#include "tdeio/job.h"
#include "tdeio/skipdlg.h"
#include "tdeio/renamedlg.h"

class UIServer_stub;
class KURL;

namespace TDEIO {
  class Job;
}

/**
 * Observer for TDEIO::Job progress information.
 *
 * This class, of which there is always only one instance,
 * "observes" what jobs do and forwards this information
 * to the progress-info server.
 *
 * It is a DCOP object so that the UI server can call the
 * kill method when the user presses Cancel.
 *
 * Usually jobs are automatically registered by the
 * TDEIO::Scheduler, so you do not have to care about that.
 *
 * @short Observer for TDEIO::Job progress information
 * @author David Faure <faure@kde.org>
 */
class TDEIO_EXPORT Observer : public TQObject, public DCOPObject {

  K_DCOP
  TQ_OBJECT

public:

  /**
   * Returns the unique observer object.
   * @return the observer object
   */
  static Observer * self() {
      if (!s_pObserver) s_pObserver = new Observer;
      return s_pObserver;
  }

  /**
   * Called by the job constructor, to signal its presence to the
   * UI Server.
   * @param job the new job
   * @param showProgress true to show progress, false otherwise
   * @return the progress ID assigned by the UI Server to the Job.
   */
  int newJob( TDEIO::Job * job, bool showProgress );

  /**
   * Called by the job destructor, to tell the UI Server that
   * the job ended.
   * @param progressId the progress ID of the job, as returned by newJob()
   */
  void jobFinished( int progressId );

  /**
   * @deprecated use TDEIO::AutoInfo
   */
  bool openPassDlg( const TQString& prompt, TQString& user, TQString& pass,
                    bool readOnly );

  /**
   * Opens a password dialog.
   * @param info the authentication information
   * @return true if successful ("ok" clicked), false otherwise
   */
  bool openPassDlg( TDEIO::AuthInfo& info );

  /**
   * Popup a message box. See TDEIO::SlaveBase.
   * This doesn't use DCOP anymore, it shows the dialog in the application's process.
   * Otherwise, other apps would block when trying to communicate with UIServer.
   * @param progressId the progress ID of the job, as returned by newJob()
   * @param type the type of the message box
   * @param text the text to show
   * @param caption the window caption
   * @param buttonYes the text of the "Yes" button
   * @param buttonNo the text of the "No button
   */
  static int messageBox( int progressId, int type, const TQString &text, const TQString &caption,
                         const TQString &buttonYes, const TQString &buttonNo );

  /**
   * Popup a message box. See TDEIO::SlaveBase.
   * This doesn't use DCOP anymore, it shows the dialog in the application's process.
   * Otherwise, other apps would block when trying to communicate with UIServer.
   * @param progressId the progress ID of the job, as returned by newJob()
   * @param type the type of the message box
   * @param text the text to show
   * @param caption the window caption
   * @param buttonYes the text of the "Yes" button
   * @param buttonNo the text of the "No button
   * @param dontAskAgainName A checkbox is added with which further confirmation can be turned off.
   *        The string is used to lookup and store the setting in tdeioslaverc.
   * @since 3.3
   */
  static int messageBox( int progressId, int type, const TQString &text, const TQString &caption,
                         const TQString &buttonYes, const TQString &buttonNo, const TQString &dontAskAgainName );

  /**
   * @internal
   * See renamedlg.h
   */
  TDEIO::RenameDlg_Result open_RenameDlg( TDEIO::Job * job,
                                        const TQString & caption,
                                        const TQString& src, const TQString & dest,
                                        TDEIO::RenameDlg_Mode mode,
                                        TQString& newDest,
                                        TDEIO::filesize_t sizeSrc = (TDEIO::filesize_t) -1,
                                        TDEIO::filesize_t sizeDest = (TDEIO::filesize_t) -1,
                                        time_t ctimeSrc = (time_t) -1,
                                        time_t ctimeDest = (time_t) -1,
                                        time_t mtimeSrc = (time_t) -1,
                                        time_t mtimeDest = (time_t) -1
                                        );

  /**
   * @internal
   * See skipdlg.h
   */
  TDEIO::SkipDlg_Result open_SkipDlg( TDEIO::Job * job,
                                    bool multi,
                                    const TQString & error_text );

k_dcop:
  /**
   * Called by the UI Server (using DCOP) if the user presses cancel.
   * @param progressId the progress ID of the job, as returned by newJob()
   */
  void killJob( int progressId );

  /**
   * Called by the UI Server (using DCOP) to get all the metadata of the job
   * @param progressId the progress IDof the job, as returned by newJob()
   */
  TDEIO::MetaData metadata( int progressId );

protected:

  static Observer * s_pObserver;
  Observer();
  ~Observer() {}

  UIServer_stub * m_uiserver;

  TQIntDict< TDEIO::Job > m_dctJobs;

public slots:

  void slotTotalSize( TDEIO::Job*, TDEIO::filesize_t size );
  void slotTotalFiles( TDEIO::Job*, unsigned long files );
  void slotTotalDirs( TDEIO::Job*, unsigned long dirs );

  void slotProcessedSize( TDEIO::Job*, TDEIO::filesize_t size );
  void slotProcessedFiles( TDEIO::Job*, unsigned long files );
  void slotProcessedDirs( TDEIO::Job*, unsigned long dirs );

  void slotSpeed( TDEIO::Job*, unsigned long speed );
  void slotPercent( TDEIO::Job*, unsigned long percent );
  void slotInfoMessage( TDEIO::Job*, const TQString & msg );

  void slotCopying( TDEIO::Job*, const KURL& from, const KURL& to );
  void slotMoving( TDEIO::Job*, const KURL& from, const KURL& to );
  void slotDeleting( TDEIO::Job*, const KURL& url );
  /// @since 3.1
  void slotTransferring( TDEIO::Job*, const KURL& url );
  void slotCreatingDir( TDEIO::Job*, const KURL& dir );
  // currently unused
  void slotCanResume( TDEIO::Job*, TDEIO::filesize_t offset );

public:
  void stating( TDEIO::Job*, const KURL& url );
  void mounting( TDEIO::Job*, const TQString & dev, const TQString & point );
  void unmounting( TDEIO::Job*, const TQString & point );
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class ObserverPrivate* d;
};
#endif
