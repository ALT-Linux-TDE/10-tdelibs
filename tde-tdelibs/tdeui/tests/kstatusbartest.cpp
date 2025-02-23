#include <tqpopupmenu.h>
#include <tqwidget.h>
#include <tqstring.h>
#include <tqmessagebox.h>
#include <tqmultilineedit.h>
#include <tqkeycode.h>
#include <tqpixmap.h>
#include <tqcursor.h>

#include <stdlib.h>

#include "kstatusbar.h"
#include <tdeapplication.h>
#include <tdemainwindow.h>
#include <tdemenubar.h>
#include "kstatusbartest.h"

testWindow::testWindow (TQWidget *, const char *name)
    : TDEMainWindow (0, name)
 {
    // Setup Menus
    menuBar = new KMenuBar (this);
    fileMenu = new TQPopupMenu;
    menuBar->insertItem ("&File", fileMenu);
    fileMenu->insertItem ("&Exit", TDEApplication::kApplication(),
                          TQ_SLOT( quit() ), ALT + Key_Q );
    statusbar = new KStatusBar (this);
    statusbar->insertItem("Zoom: XXXX", 0);
    statusbar->insertItem("XXX", 1);
    statusbar->insertItem("Line: XXXXX", 2);

    statusbar->changeItem("Zoom: 100%", 0);
    statusbar->changeItem("INS", 1);
    insert = true;
    statusbar->changeItem("Line: 13567", 2);

    connect (statusbar, TQ_SIGNAL(pressed(int)), this, TQ_SLOT(slotPress(int)));
    connect (statusbar, TQ_SIGNAL(released(int)), this, TQ_SLOT(slotClick(int)));

    widget = new TQMultiLineEdit (this);

    setCentralWidget(widget);

    setCaption( TDEApplication::kApplication()->caption() );

    smenu = new TQPopupMenu;
  
    smenu->insertItem("50%");
    smenu->insertItem("75%");
    smenu->insertItem("100%");
    smenu->insertItem("150%");
    smenu->insertItem("200%");
    smenu->insertItem("400%");
    smenu->insertItem("oo%");

    connect (smenu, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotMenu(int)));
}

void testWindow::slotClick(int id)
{
  switch (id)
   {
    case 0:
      break;

    case 1:
      if (insert == true)
       {
         insert = false;
         statusbar->changeItem("OVR", 1);
       }
      else
       {
         insert = true;
         statusbar->changeItem("INS", 1);
       }
      break;

    case 2:
      TQMessageBox::information(0, "Go to line", "Enter line number:", "where?");
      statusbar->changeItem("16543", 2);
      break;
   }
}
       
void testWindow::slotPress(int id)
{
  if (id == 0)
    smenu->popup(TQCursor::pos()); // This popup should understand keys up and down
}

void testWindow::slotMenu(int id)
{
  TQString s = "Zoom: ";
  s.append (smenu->text(id));
  statusbar->changeItem(s,0);
}

testWindow::~testWindow ()
{
  // I would delete toolbars here, but there are none
  delete statusbar;
}

int main( int argc, char *argv[] )
{
        TDEApplication *myApp = new TDEApplication( argc, argv, "KStatusBarTest" );
        testWindow *test = new testWindow;

        myApp->setMainWidget(test);
        test->show();
        test->resize(test->width(), test->height()); // I really really really dunno why it doesn't show
        int ret = myApp->exec();

        delete test;

        return ret;
} 

#include "kstatusbartest.moc"

