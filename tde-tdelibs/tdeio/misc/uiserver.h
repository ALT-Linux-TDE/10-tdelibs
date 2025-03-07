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
#ifndef __tdeio_uiserver_h__
#define __tdeio_uiserver_h__

#include <tqintdict.h>
#include <tqdatetime.h>
#include <tqtimer.h>

#include <dcopobject.h>
#include <tdeio/global.h>
#include <tdeio/authinfo.h>
#include <kurl.h>
#include <tdemainwindow.h>
#include <kdatastream.h>
#include <tdelistview.h>
#include <ksslcertdlg.h>

class ListProgress;
class KSqueezedTextLabel;
class ProgressItem;
class UIServer;

namespace TDEIO {
  class Job;
  class DefaultProgress;
}


struct ListProgressColumnConfig
{
   TQString title;
   int index;
   int width;
   bool enabled;
};

/**
* List view in the UIServer.
* @internal
*/
class TDEIO_EXPORT ListProgress : public TDEListView {

  TQ_OBJECT

public:

  ListProgress (TQWidget *parent = 0, const char *name = 0 );

  virtual ~ListProgress();

  /**
   * Field constants
   */
  enum ListProgressFields {
    TB_OPERATION = 0,
    TB_LOCAL_FILENAME = 1,
    TB_RESUME = 2,
    TB_COUNT = 3,     //lv_count
    TB_PROGRESS = 4,  // lv_progress
    TB_TOTAL = 5,
    TB_SPEED = 6,
    TB_REMAINING_TIME = 7,
    TB_ADDRESS = 8,
    TB_MAX = 9
  };

  friend class ProgressItem;
  friend class UIServer;
protected slots:
  void columnWidthChanged(int column);
protected:

  void writeSettings();
  void readSettings();
  void applySettings();
  void createColumns();

  bool m_showHeader;
  bool m_fixedColumnWidths;
  ListProgressColumnConfig m_lpcc[TB_MAX];
  //hack, alexxx
  KSqueezedTextLabel *m_squeezer;
};

/**
* One item in the ListProgress
* @internal
*/
class TDEIO_EXPORT ProgressItem : public TQObject, public TQListViewItem {

  TQ_OBJECT

public:
  ProgressItem( ListProgress* view, TQListViewItem *after, TQCString app_id, int job_id,
                bool showDefault = true );
  ~ProgressItem();

  TQCString appId() { return m_sAppId; }
  int jobId() { return m_iJobId; }

    bool keepOpen() const;
  void finished();

  void setVisible( bool visible );
  void setDefaultProgressVisible( bool visible );
  bool isVisible() const { return m_visible; }

  void setTotalSize( TDEIO::filesize_t bytes );
  void setTotalFiles( unsigned long files );
  void setTotalDirs( unsigned long dirs );

  void setProcessedSize( TDEIO::filesize_t size );
  void setProcessedFiles( unsigned long files );
  void setProcessedDirs( unsigned long dirs );

  void setPercent( unsigned long percent );
  void setSpeed( unsigned long bytes_per_second );
  void setInfoMessage( const TQString & msg );

  void setCopying( const KURL& from, const KURL& to );
  void setMoving( const KURL& from, const KURL& to );
  void setDeleting( const KURL& url );
  void setTransferring( const KURL& url );
  void setCreatingDir( const KURL& dir );
  void setStating( const KURL& url );
  void setMounting( const TQString & dev, const TQString & point );
  void setUnmounting( const TQString & point );

  void setCanResume( TDEIO::filesize_t offset );

  TDEIO::filesize_t totalSize() { return m_iTotalSize; }
  unsigned long totalFiles() { return m_iTotalFiles; }
  TDEIO::filesize_t processedSize() { return m_iProcessedSize; }
  unsigned long processedFiles() { return m_iProcessedFiles; }
  unsigned long speed() { return m_iSpeed; }
  unsigned int remainingSeconds() { return m_remainingSeconds; }

  const TQString& fullLengthAddress() const {return m_fullLengthAddress;}
  void setText(ListProgress::ListProgressFields field, const TQString& text);
public slots:
  void slotShowDefaultProgress();
  void slotToggleDefaultProgress();

protected slots:
  void slotCanceled();

signals:
  void jobCanceled( ProgressItem* );

protected:
  void updateVisibility();

  // ids that uniquely identify this progress item
  TQCString m_sAppId;
  int m_iJobId;

  // whether shown or not (it is hidden if a rename dialog pops up for the same job)
  bool m_visible;
  bool m_defaultProgressVisible;

  // parent listview
  ListProgress *listProgress;

  // associated default progress dialog
  TDEIO::DefaultProgress *defaultProgress;

  // we store these values for calculation of totals ( for statusbar )
  TDEIO::filesize_t m_iTotalSize;
  unsigned long m_iTotalFiles;
  TDEIO::filesize_t m_iProcessedSize;
  unsigned long m_iProcessedFiles;
  unsigned long m_iSpeed;
  int m_remainingSeconds;
  TQTimer m_showTimer;
  TQString m_fullLengthAddress;
};

class TQResizeEvent;
class TQHideEvent;
class TQShowEvent;
class ProgressConfigDialog;
class TQPopupMenu;
class UIServerSystemTray;

/**
 * It's purpose is to show progress of IO operations.
 * There is only one instance of this window for all jobs.
 *
 * All IO operations ( jobs ) are displayed in this window, one line per operation.
 * User can cancel operations with Cancel button on toolbar.
 *
 * Double clicking an item in the list opens a small download window ( DefaultProgress ).
 *
 * @short Graphical server for progress information with an optional all-in-one progress window.
 * @author David Faure <faure@kde.org>
 * @author Matej Koss <koss@miesto.sk>
 *
 * @internal
 */
class TDEIO_EXPORT UIServer : public TDEMainWindow, public DCOPObject {

  K_DCOP
  TQ_OBJECT

  UIServer();
  virtual ~UIServer();

public:
   static UIServer* createInstance();

k_dcop:

  /**
   * Signal a new job
   * @param appId the DCOP application id of the job's parent application
   * @see TDEIO::Observer::newJob
   * @param showProgress whether to popup the progress for the job.
   *   Usually true, but may be false when we use tdeio_uiserver for
   *   other things, like SSL dialogs.
   * @return the job id
   */
  int newJob( TQCString appId, bool showProgress );

  ASYNC jobFinished( int id );

  ASYNC totalSize( int id, unsigned long size );
  ASYNC totalSize64( int id, TDEIO::filesize_t size );
  ASYNC totalFiles( int id, unsigned long files );
  ASYNC totalDirs( int id, unsigned long dirs );

  ASYNC processedSize( int id, unsigned long bytes );
  ASYNC processedSize64( int id, TDEIO::filesize_t bytes );
  ASYNC processedFiles( int id, unsigned long files );
  ASYNC processedDirs( int id, unsigned long dirs );

  ASYNC percent( int id, unsigned long ipercent );
  ASYNC speed( int id, unsigned long bytes_per_second );
  ASYNC infoMessage( int id, const TQString & msg );

  ASYNC copying( int id, KURL from, KURL to );
  ASYNC moving( int id, KURL from, KURL to );
  ASYNC deleting( int id, KURL url );
  ASYNC transferring( int id, KURL url );
  ASYNC creatingDir( int id, KURL dir );
  ASYNC stating( int id, KURL url );

  ASYNC mounting( int id, TQString dev, TQString point );
  ASYNC unmounting( int id, TQString point );

  ASYNC canResume( int id, unsigned long offset );
  ASYNC canResume64( int id, TDEIO::filesize_t offset );

  /**
   * @deprecated (it blocks other apps).
   * Use TDEIO::PasswordDialog::getNameAndPassword instead.
   * To be removed in KDE 4.0.
   */
  TQByteArray openPassDlg( const TDEIO::AuthInfo &info );

  /**
   * Popup a message box.
   * @param id The message identifier.
   * @param type type of message box: QuestionYesNo, WarningYesNo, WarningContinueCancel...
   *   This enum is defined in slavebase.h, it currently is:
   *   QuestionYesNo = 1, WarningYesNo = 2, WarningContinueCancel = 3,
   *   WarningYesNoCancel = 4, Information = 5, SSLMessageBox = 6
   * @param text Message string. May contain newlines.
   * @param caption Message box title.
   * @param buttonYes The text for the first button.
   *                  The default is i18n("&Yes").
   * @param buttonNo  The text for the second button.
   *                  The default is i18n("&No").
   * Note: for ContinueCancel, buttonYes is the continue button and buttonNo is unused.
   *       and for Information, none is used.
   * @return a button code, as defined in KMessageBox, or 0 on communication error.
   */
  int messageBox( int id, int type, const TQString &text, const TQString &caption,
                  const TQString &buttonYes, const TQString &buttonNo );

  /**
   * @deprecated (it blocks other apps).
   * Use TDEIO::open_RenameDlg instead.
   * To be removed in KDE 4.0.
   */
  TQByteArray open_RenameDlg64( int id,
                             const TQString & caption,
                             const TQString& src, const TQString & dest,
                             int /* TDEIO::RenameDlg_Mode */ mode,
                             TDEIO::filesize_t sizeSrc,
                             TDEIO::filesize_t sizeDest,
                             unsigned long /* time_t */ ctimeSrc,
                             unsigned long /* time_t */ ctimeDest,
                             unsigned long /* time_t */ mtimeSrc,
                             unsigned long /* time_t */ mtimeDest
                             );
  /**
   * @deprecated (it blocks other apps).
   * Use TDEIO::open_RenameDlg instead.
   * To be removed in KDE 4.0.
   */
  TQByteArray open_RenameDlg( int id,
                             const TQString & caption,
                             const TQString& src, const TQString & dest,
                             int /* TDEIO::RenameDlg_Mode */ mode,
                             unsigned long sizeSrc,
                             unsigned long sizeDest,
                             unsigned long /* time_t */ ctimeSrc,
                             unsigned long /* time_t */ ctimeDest,
                             unsigned long /* time_t */ mtimeSrc,
                             unsigned long /* time_t */ mtimeDest
                             );

  /**
   * @deprecated (it blocks other apps).
   * Use TDEIO::open_SkipDlg instead.
   * To be removed in KDE 4.0.
   */
  int open_SkipDlg( int id,
                    int /*bool*/ multi,
                    const TQString & error_text );

  /**
   * Switch to or from list mode - called by the kcontrol module
   */
  void setListMode( bool list );

  /**
   * Hide or show a job. Typically, we hide a job while a "skip" or "rename" dialog
   * is being shown for this job. This prevents killing it from the uiserver.
   */
  void setJobVisible( int id, bool visible );

  /**
   * Show a SSL Information Dialog
   */
  void showSSLInfoDialog(const TQString &url, const TDEIO::MetaData &data, int mainwindow);

  /**
   * @deprecated
   */
  void showSSLInfoDialog(const TQString &url, const TDEIO::MetaData &data);

  /*
   * Show an SSL Certificate Selection Dialog
   */
  KSSLCertDlgRet showSSLCertDialog(const TQString& host, const TQStringList& certList, int mainwindow);

  /*
   * @deprecated
   */
  KSSLCertDlgRet showSSLCertDialog(const TQString& host, const TQStringList& certList);

public slots:
  void slotConfigure();
  void slotRemoveSystemTrayIcon();
protected slots:

  void slotUpdate();
  void slotQuit();

  void slotCancelCurrent();

  void slotToggleDefaultProgress( TQListViewItem * );
  void slotSelection();

  void slotJobCanceled( ProgressItem * );
  void slotApplyConfig();
  void slotShowContextMenu(TDEListView*, TQListViewItem *item, const TQPoint& pos);

protected:

  ProgressItem* findItem( int id );

  virtual void resizeEvent(TQResizeEvent* e);
  virtual bool queryClose();

  void setItemVisible( ProgressItem * item, bool visible );

  TQTimer* updateTimer;
  ListProgress* listProgress;

  TDEToolBar::BarPosition toolbarPos;
  TQString properties;

  void applySettings();
  void readSettings();
  void writeSettings();
private:

  void killJob( TQCString observerAppId, int progressId );

  int m_initWidth;
  int m_initHeight;
  int m_idCancelItem;
  bool m_bShowList;
  bool m_showStatusBar;
  bool m_showToolBar;
  bool m_keepListOpen;
  bool m_showSystemTray;
  bool m_shuttingDown;

  // true if there's a new job that hasn't been shown yet.
  bool m_bUpdateNewJob;
  ProgressConfigDialog *m_configDialog;
  TQPopupMenu* m_contextMenu;
  UIServerSystemTray *m_systemTray;

  static int s_jobId;
  friend class no_bogus_warning_from_gcc;
};
#endif
