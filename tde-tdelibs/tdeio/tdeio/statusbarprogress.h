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

#ifndef __statusbarprogress_h__
#define __statusbarprogress_h__

#include "progressbase.h"

class TQWidgetStack;
class TQBoxLayout;
class TQPushButton;
class TQLabel;
class KProgress;

namespace TDEIO {

class Job;

/**
* This is a special IO progress widget.
*
* Similarly to DefaultProgress,
* it's purpose is to show a progress of the IO operation.
*
* Instead of creating a separate window, this is only a widget that can be
* easily embedded in a statusbar.
*
* Usage of StatusbarProgress is little different.
* This dialog will be a part of some application.
* \code
* // create a dialog
* StatusbarProgress *statusProgress;
* statusProgress = new StatusbarProgress( statusBar() );
* statusBar()->insertWidget( statusProgress, statusProgress->width() , 0 );
* ...
* // create job and connect it to the progress
* CopyJob* job = TDEIO::copy(...);
* statusProgress->setJob( job );
* ...
* \endcode
*
* @short IO progress widget for embedding in a statusbar.
* @author Matej Koss <koss@miesto.sk>
*/
class TDEIO_EXPORT StatusbarProgress : public ProgressBase {

  TQ_OBJECT

public:

  /**
   * Creates a new StatusbarProgress.
   * @param parent the parent of this widget
   * @param button true to add an abort button. The button will be
   *               connected to ProgressBase::slotStop()
   */
  StatusbarProgress( TQWidget* parent, bool button = true );
  ~StatusbarProgress() {}

  /**
   * Sets the job to monitor.
   * @param job the job to monitor
   */
  void setJob( TDEIO::Job *job );

public slots:
  virtual void slotClean();
  virtual void slotTotalSize( TDEIO::Job* job, TDEIO::filesize_t size );
  virtual void slotPercent( TDEIO::Job* job, unsigned long percent );
  virtual void slotSpeed( TDEIO::Job* job, unsigned long speed );

protected:
  KProgress* m_pProgressBar;
  TQLabel* m_pLabel;
  TQPushButton* m_pButton;

  TDEIO::filesize_t m_iTotalSize;

  enum Mode { None, Label, Progress };

  uint mode;
  bool m_bShowButton;

  void setMode();

  virtual bool eventFilter( TQObject *, TQEvent * );
  TQBoxLayout *box;
  TQWidgetStack *stack;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class StatusbarProgressPrivate* d;
};

} /* namespace */

#endif  //  __statusbarprogress_h__
