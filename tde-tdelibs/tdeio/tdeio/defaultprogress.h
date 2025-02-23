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
#ifndef __defaultprogress_h__
#define __defaultprogress_h__

#include <tqlabel.h>

#include <tdeio/global.h>

#include <kprogress.h>

#include "progressbase.h"

class KLineEdit;

namespace TDEIO {

/*
 * A default implementation of the progress dialog ProgressBase.
 * ProgressBase
 */
class TDEIO_EXPORT DefaultProgress : public ProgressBase {

  TQ_OBJECT

public:
  /**
   * Creates a new default progress dialog.
   * @param showNow true to show immediately, false to show when
   *                needed
   */
  DefaultProgress( bool showNow = true );
  /**
   * Creates a new default progress dialog.
   * @param parent the parent of the dialog (or 0 for top-level)
   * @param name the name of the dialog, can be 0
   * @since 3.1
   */
  DefaultProgress( TQWidget* parent, const char* name = 0 );
  ~DefaultProgress();

  bool keepOpen() const;

  /// Shared with uiserver.cpp
  static TQString makePercentString( unsigned long percent,
                                    TDEIO::filesize_t totalSize,
                                    unsigned long totalFiles );

public slots:
  virtual void slotTotalSize( TDEIO::Job *job, TDEIO::filesize_t size );
  virtual void slotTotalFiles( TDEIO::Job *job, unsigned long files );
  virtual void slotTotalDirs( TDEIO::Job *job, unsigned long dirs );

  virtual void slotProcessedSize( TDEIO::Job *job, TDEIO::filesize_t bytes );
  virtual void slotProcessedFiles( TDEIO::Job *job, unsigned long files );
  virtual void slotProcessedDirs( TDEIO::Job *job, unsigned long dirs );

  virtual void slotSpeed( TDEIO::Job *job, unsigned long speed );
  virtual void slotPercent( TDEIO::Job *job, unsigned long percent );
  /**
   * Called to set an information message.
   * @param job the TDEIO::Job
   * @param msg the message to set
   */
  virtual void slotInfoMessage( TDEIO::Job *job, const TQString & msg );

  virtual void slotCopying( TDEIO::Job* job, const KURL& src, const KURL& dest );
  virtual void slotMoving( TDEIO::Job* job, const KURL& src, const KURL& dest );
  virtual void slotDeleting( TDEIO::Job* job, const KURL& url );
  /**
   * Called when the job is transferring.
   * @param job the TDEIO::Job
   * @param url the url to transfer
   * @since 3.1
   */
  void slotTransferring( TDEIO::Job* job, const KURL& url );
  virtual void slotCreatingDir( TDEIO::Job* job, const KURL& dir );
  /**
   * Called when the job is requesting a stat.
   * @param job the TDEIO::Job
   * @param dir the dir to stat
   * @since 3.1
   */
  virtual void slotStating( TDEIO::Job* job, const KURL& dir );
  /**
   * Called when the job is mounting.
   * @param job the TDEIO::Job
   * @param dev the device to mount
   * @param point the mount point
   */
  virtual void slotMounting( TDEIO::Job* job, const TQString & dev, const TQString & point );
  /**
   * Called when the job is unmounting.
   * @param job the TDEIO::Job
   * @param point the mount point
   */
  virtual void slotUnmounting( TDEIO::Job* job, const TQString & point );
  virtual void slotCanResume( TDEIO::Job* job, TDEIO::filesize_t from);

  /**
   * Called when the job is cleaned.
   * @since 3.1
   */
  void slotClean();

protected:
  /// @since 3.1
  void init();
  void showTotals();
  void setDestVisible( bool visible );
  /// @since 3.1
  void checkDestination( const KURL& dest);

  KLineEdit* sourceEdit;
  KLineEdit* destEdit;
  TQLabel* progressLabel;
  TQLabel* destInvite;
  TQLabel* speedLabel;
  TQLabel* sizeLabel;
  TQLabel* resumeLabel;

  KProgress* m_pProgressBar;

  TDEIO::filesize_t m_iTotalSize;
  unsigned long m_iTotalFiles;
  unsigned long m_iTotalDirs;

  TDEIO::filesize_t m_iProcessedSize;
  unsigned long m_iProcessedDirs;
  unsigned long m_iProcessedFiles;

  enum ModeType { Copy, Move, Delete, Create, Done };
  ModeType mode;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class DefaultProgressPrivate;
  DefaultProgressPrivate* d;
private slots:
  void slotKeepOpenToggled(bool);
  void slotOpenFile();
  void slotOpenLocation();
};

} /* namespace */

#endif // __defaultprogress_h__

