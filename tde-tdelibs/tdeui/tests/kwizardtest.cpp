/*
 *   kwizardtest - a test program for the KWizard dialog
 *   Copyright (C) 1998  Thomas Tanghus (tanghus@kde.org)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <tqlabel.h>
#include <tqlayout.h>
#include <tdeapplication.h>
#include <kwizard.h>

int main(int argc, char **argv)
{
  TDEApplication a(argc,argv,"kwizardtest");
  KWizard *wiz = new KWizard(0, "kwizardtest", false);
  TQObject::connect((TQObject*) wiz->cancelButton(), TQ_SIGNAL(clicked()),
		   &a, TQ_SLOT(quit()));
  TQObject::connect((TQObject*) wiz->finishButton(), TQ_SIGNAL(clicked()),
		   &a, TQ_SLOT(quit()));
  for(int i = 1; i < 11; i++)
  {
    TQWidget *p = new TQWidget;
    TQString msg = TQString("This is page %1 out of 10").arg(i);
    TQLabel *label = new TQLabel(msg, p);
    TQHBoxLayout *layout = new TQHBoxLayout(p, 5);
    label->setAlignment(TQt::AlignCenter);
    label->setFixedSize(300, 200);
    layout->addWidget(label);
    TQString title = TQString("%1. page").arg(i);
    wiz->addPage(p, title);
    wiz->setFinishEnabled(p, (i==10));
  }

  a.setMainWidget(wiz);
  wiz->show();
  return a.exec();
}



