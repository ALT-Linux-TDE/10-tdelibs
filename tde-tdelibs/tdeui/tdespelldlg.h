/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>
   Copyright (C) 2000 Rik Hemsley <rik@kde.org>
   Copyright (C) 2003 Zack Rusin  <zack@kde.org>

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
#ifndef __TDESPELLDLG_H__
#define __TDESPELLDLG_H__

#include <kdialogbase.h>

class TQStringList;
class TQLabel;
class TQListViewItem;
class TQPushButton;
class TDEListBox;
class KLineEdit;
class KProgress;

//Possible result codes
enum KS_RESULT {
  KS_CANCEL=     0,
  KS_REPLACE=    1,
  KS_REPLACEALL= 2,
  KS_IGNORE=     3,
  KS_IGNOREALL=  4,
  KS_ADD=        5,
  KS_STOP=       7,
  KS_SUGGEST=    8,
  KS_CONFIG=     9
};

class TDEUI_EXPORT KSpellDlg : public KDialogBase
{
  TQ_OBJECT

  class KSpellDlgPrivate;
  KSpellDlgPrivate *d;
  KLineEdit *editbox;
  KProgress *progbar;
  TDEListBox *listbox;
  TQStringList *sugg;
  TQPushButton *qpbrep, *qpbrepa;
  TQLabel *wordlabel;
  TQString word, newword;
  bool progressbar;

public:
  KSpellDlg( TQWidget *parent, const char *name,
	     bool _progressbar = false, bool _modal = false );
  ~KSpellDlg();

  TQString replacement() const
    { return newword; }

  /**
   * Change the misspelled word and suggested replacements
   *  and enable the disabled buttons on the dialog box.
   * (Buttons are disabled by standby().)
   **/
  void init( const TQString& _word, TQStringList* _sugg );
  void init( const TQString& _word, TQStringList* _sugg,
             const TQString& context );

  void standby() { emit ready( false ); }

  public slots:
  /**
   * Adjust the progress bar to @p p percent.
   **/
  void slotProgress( unsigned int p );

protected:
  virtual void closeEvent( TQCloseEvent * e );
  void done( int i );

 signals:
    /**
      This signal is emitted when a button is pressed.
      */
  void command( int );

  void ready( bool );

protected slots:
  void ignore();
  void add();
  void ignoreAll();
  void cancel();
  void replace();
  void replaceAll();
  void suggest();
  void stop();
  void slotConfigChanged();

  void textChanged( const TQString & );

  void slotSelectionChanged( TQListViewItem* item );
};

#endif
