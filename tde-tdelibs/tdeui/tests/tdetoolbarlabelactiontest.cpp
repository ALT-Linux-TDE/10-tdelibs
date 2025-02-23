/* This file is part of the KDE libraries
    Copyright (C) 2004 Felix Berger <felixberger@beldesign.de>

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

#include <tqguardedptr.h>

#include <tdeapplication.h>
#include <tdemainwindow.h>
#include <klineedit.h>
#include <tdelistview.h>
#include <kstandarddirs.h>
#include <tdetoolbarlabelaction.h>
#include <ksqueezedtextlabel.h> 
#include <kdebug.h>
#include <tqvbox.h>

#include <assert.h>

class MainWindow : public TDEMainWindow
{
public:
  MainWindow()
  {
    TQVBox* main = new TQVBox(this);
    setCentralWidget(main);

    KSqueezedTextLabel* accel = new KSqueezedTextLabel
      ("&Really long, long, long and boring text goes here", main, 
       "tde toolbar widget");
    new KSqueezedTextLabel
      ("Really long, long, long and boring text goes here", main, 
       "tde toolbar widget");


    // first constructor
    TDEToolBarLabelAction* label1 = new TDEToolBarLabelAction("&Label 1", 0,
							  0, 0,
							  actionCollection(),
							  "label1");
    // second constructor
    KLineEdit* lineEdit = new KLineEdit(this);
    new KWidgetAction(lineEdit, "Line Edit", 0, this, 0,
		      actionCollection(), "lineEdit");
    TDEToolBarLabelAction* label2 = 
      new TDEToolBarLabelAction(lineEdit, "L&abel 2", 0, 0, 0,
			      actionCollection(),
			      "label2");

    // set buddy for label1
    label1->setBuddy(lineEdit);
    accel->setBuddy(lineEdit);

     // third constructor
    TQLabel* customLabel =  new KSqueezedTextLabel
      ("&Really long, long, long and boring text goes here", this, 
        "tde toolbar widget");

    TDEToolBarLabelAction* label3 = new TDEToolBarLabelAction(customLabel, 0, 0, 0,
 							  actionCollection(),
							  "label3");
  
    // set buddy for label3
    label3->setBuddy(lineEdit);

    // customLabel->setText("&test me again some time soon");
    
    createGUI("tdetoolbarlabelactiontestui.rc");
  }
};

int main( int argc, char **argv )
{
  TDEApplication app( argc, argv, "tdetoolbarlabelactiontest" );

  TDEGlobal::instance()->dirs()->addResourceDir("data", ".");

  MainWindow* mw = new MainWindow;
  app.setMainWidget(mw);
  mw->show();

  return app.exec();
}

