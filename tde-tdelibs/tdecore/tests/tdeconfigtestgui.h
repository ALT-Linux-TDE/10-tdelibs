/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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
#ifndef _TDECONFIG_TEST_H
#define _TDECONFIG_TEST_H

#include <tdeapplication.h>
#include <tqdialog.h>
#include <tqfile.h>
#include <tqfileinfo.h>
#include <kdebug.h>
#include <ksimpleconfig.h>
#include <tqtextstream.h>

// Standard Qt widgets

#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqpushbutton.h>

#include <tdeconfig.h>

//
// TDEConfigTestView contains lots of Qt widgets.
//

class TDEConfigTestView : public TQDialog
{
  TQ_OBJECT
public:
  TDEConfigTestView( TQWidget *parent=0, const char *name=0 );
  ~TDEConfigTestView();

private slots:
  void appConfigEditReturnPressed();
  void groupEditReturnPressed();
  void keyEditReturnPressed();
  void writeButtonClicked();

private:
  TQLabel* pAppFileLabel;
  TQLineEdit* pAppFileEdit;
  TQLabel* pGroupLabel;
  TQLineEdit* pGroupEdit;
  TQLineEdit* pKeyEdit;
  TQLabel* pEqualsLabel;
  TQLineEdit* pValueEdit;
  TQPushButton* pWriteButton;
  TQLabel* pInfoLabel1, *pInfoLabel2;
  TQPushButton* pQuitButton;

  TDEConfig* pConfig;
  TQFile* pFile;
  TQTextStream* pStream;
};

#endif
