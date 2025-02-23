 /*
  This file is or will be part of KDE desktop environment

  Copyright 1999 Matt Koss <koss@miesto.sk>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#ifndef _TDEIOSLAVETEST_H
#define _TDEIOSLAVETEST_H

#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqradiobutton.h>
#include <tqpushbutton.h>
#include <tqbuttongroup.h>
#include <tqwidget.h>

#include <tdemainwindow.h>

#include "tdeio/job.h"
#include "tdeio/global.h"
#include "tdeio/statusbarprogress.h"
#include "tdeio/slave.h"

class KioslaveTest : public TDEMainWindow {
  TQ_OBJECT

public:
  KioslaveTest( TQString src, TQString dest, uint op, uint pr );
  ~KioslaveTest() {}

  enum Operations { List, ListRecursive, Stat, Get, Put, Copy, Move, Delete, Shred, Mkdir, Mimetype };

  enum ProgressModes { ProgressNone, ProgressDefault, ProgressStatus };

protected:

  void closeEvent( TQCloseEvent * );

  void printUDSEntry( const TDEIO::UDSEntry & entry );

  // info stuff
  TQLabel *lb_from;
  TQLineEdit *le_source;

  TQLabel *lb_to;
  TQLineEdit *le_dest;

  // operation stuff
  TQButtonGroup *opButtons;

  TQRadioButton *rbList;
  TQRadioButton *rbListRecursive;
  TQRadioButton *rbStat;
  TQRadioButton *rbGet;
  TQRadioButton *rbPut;
  TQRadioButton *rbCopy;
  TQRadioButton *rbMove;
  TQRadioButton *rbDelete;
  TQRadioButton *rbShred;
  TQRadioButton *rbMkdir;
  TQRadioButton *rbMimetype;

  // progress stuff
  TQButtonGroup *progressButtons;

  TQRadioButton *rbProgressNone;
  TQRadioButton *rbProgressDefault;
  TQRadioButton *rbProgressStatus;

  TQPushButton *pbStart;
  TQPushButton *pbStop;

  TQPushButton *close;

protected slots:
  void changeOperation( int id );
  void changeProgressMode( int id );

  void startJob();
  void stopJob();

  void slotResult( TDEIO::Job * );
  void slotEntries( TDEIO::Job *, const TDEIO::UDSEntryList& );
  void slotData( TDEIO::Job *, const TQByteArray &data );
  void slotDataReq( TDEIO::Job *, TQByteArray &data );

  void slotQuit();
  void slotSlaveConnected();
  void slotSlaveError();

private:
  TDEIO::Job *job;
  TQWidget *main_widget;

  TDEIO::StatusbarProgress *statusProgress;

  int selectedOperation;
  int progressMode;
  int putBuffer;
  TDEIO::Slave *slave;
};

#endif // _TDEIOSLAVETEST_H
