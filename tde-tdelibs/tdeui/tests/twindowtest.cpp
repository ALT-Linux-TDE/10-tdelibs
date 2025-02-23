#include <tqmessagebox.h>
#include <tqpopupmenu.h>
#include <tqpixmap.h>
#include <tqkeycode.h>
#include <tqwidget.h>
#include <tqstring.h>
#include <tqcursor.h>

#include <stdlib.h>

#include <kstatusbar.h>
#include <tdeapplication.h>
#include <kcombobox.h>
#include <khelpmenu.h>
#include <tdecmdlineargs.h>
#include <tdemenubar.h>
#include <tdetoolbarradiogroup.h>
#include <kiconloader.h>
#include <tdepopupmenu.h>
#include <tqmultilineedit.h>
#include "twindowtest.h"

#include <tdeglobal.h>


//#include <dclock.h>

/*
 Ok this is a constructor of our top widget. It inherits TDEMainWindow.
 In constructor wi will create all of our interface elements:
 menubar, toolbar(s), statusbar, and main widget. Non of those
 interface is obligatory, i.e. you don't have to use menubar,
 toolbars or statusbar if you don't want to. Theoreticly, you
 don't need even main widget (but in that case, you'll get blank
 TDEMainWindow).
 */

static int itemId = 0;

testWindow::testWindow (TQWidget *parent, const char *name)
    : TDEMainWindow (parent,name)
{
    ena=false;
    setCaption("test window");
setAutoSaveSettings();
    /******************************/
    /* First, we setup setup Menus */
    /******************************/
    menuBar = new KMenuBar (this);

    // First popup... 
    fileMenu = new TQPopupMenu;
    // We insert this popup in menubar with caption "File".
    // Prefix "&" means that "F" will be underlined
    menuBar->insertItem ("&File", fileMenu);
    // We insert item "Exit" with accelerator ALT-Q, and connect
    // it to application's exit-slot.
    fileMenu->insertItem ("&Exit", TDEApplication::kApplication(),
                          TQ_SLOT( quit() ), ALT + Key_Q );

    // Another popup...
    toolBarMenu = new TQPopupMenu;
    menuBar->insertItem ("&Toolbars", toolBarMenu);
    toolBarMenu->insertItem ("(Un)Hide tollbar 1", this, TQ_SLOT(slotHide1()));
    toolBarMenu->insertItem ("(Un)Hide tollbar 2", this, TQ_SLOT(slotHide2()));

    itemsMenu = new TQPopupMenu;
    menuBar->insertItem ("&Items", itemsMenu);

    exitB = true;   // exit button is shown
    lineL = true;   // Lined is enabled
    greenF = false;  // Frame not inserted
    
    itemsMenu->insertItem ("delete/insert exit button", this, TQ_SLOT(slotExit()));
    itemsMenu->insertItem ("insert/delete green frame!", this, TQ_SLOT(slotFrame()));
    itemsMenu->insertItem ("enable/disable Lined", this, TQ_SLOT(slotLined()));
    itemsMenu->insertItem ("Toggle fileNew", this, TQ_SLOT(slotNew()));
    itemsMenu->insertItem ("Clear comboBox", this, TQ_SLOT(slotClearCombo()));
    itemsMenu->insertItem ("Insert List in Combo", this, TQ_SLOT(slotInsertListInCombo()));
    itemsMenu->insertItem ("Make item 3 curent", this, TQ_SLOT(slotMakeItem3Current()));
    //itemsMenu->insertItem ("Insert clock!", this, TQ_SLOT(slotInsertClock()));
    itemsMenu->insertItem ("Important!", this, TQ_SLOT(slotImportant()));

    menuBar->insertSeparator();
    helpMenu = new KHelpMenu(this, "KWindowTest was programmed by Sven Radej");
    menuBar->insertItem( "&Help", helpMenu->menu() );

    /**************************************************/
    /*Now, we setup statusbar order is not important. */
    /**************************************************/
    statusBar = new KStatusBar (this);
    statusBar->insertItem("Hi there!                         ", 0);
    statusBar->insertItem("Look for tooltips to see functions", 1);

    //DigitalClock *clk = new DigitalClock (statusBar);
    //clk->setFrameStyle(TQFrame::NoFrame);
    //statusBar->insertWidget(clk, 70, 2);

    /***********************/
    /* And now the toolbar */
    /***********************/

    // pixmap which we will use
    TQPixmap pix;

    // Create toolbar...
    tb = toolBar();

    // and set it to full width
    tb->setFullSize(true);


    
    // First four  buttons
    pix = BarIcon("document-new");
    itemId = tb->insertButton(pix, 0, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotNew()),
                         true, "Create.. (toggles upper button)", 50);
    pix = BarIcon("document-open");
    tb->insertButton(pix, 1, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotOpen()),
                         false, "Open");
    pix = BarIcon("filefloppy");
    tb->insertButton(pix, 2, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotSave()),
                          true, "Save (beep or delayed popup)");
    tb->setDelayedPopup(2, itemsMenu);
    pix = BarIcon("document-print");
    tb->insertButton(pix, 3, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotPrint()),
                         true, "Print (enables/disables open)");

    // And a combobox
    // arguments: text (or strList), ID, writable, signal, object, slot, enabled,
    //            tooltiptext, size
    tb->insertCombo (TQString("one"), 4, true, TQ_SIGNAL(activated(const TQString&)), this,
                          TQ_SLOT(slotList(const TQString&)), true, "ComboBox", 150);


    // Then one line editor
    // arguments: text, id, signal, object (this), slot, enabled, tooltiptext, size
    tb->insertLined ("ftp://ftp.kde.org/pub/kde", 5, TQ_SIGNAL(returnPressed()), this,
                          TQ_SLOT(slotReturn()), true, "Location", 200);

    // Set this Lined to auto size itself. Note that only one item (Lined or Combo)
    // Can be set to autosize; If you specify more of them only last (according to
    /// index) will be set to autosize itself. Only Lined or Combo can be
    // set to autosize. All items after autoSized one must  be aligned right.
    // Auto size is valid only for fullWidth toolbars.

    tb->setItemAutoSized (5);

    // Now add another button and align it right
    pix = BarIcon("system-log-out");
    tb->insertButton(pix, 6, TQ_SIGNAL(clicked()), TDEApplication::kApplication(),
                          TQ_SLOT( quit() ), true, "Exit");
    tb->alignItemRight (6);

    // Another toolbar
    tb1 = new TDEToolBar(this, TQMainWindow::DockTop); // this one is normal and has separators


    pix = BarIcon("document-new");
    tb1->insertButton(pix, 0, true, "Create new file2 (Toggle)");
    tb1->setToggle(0);
    tb1->addConnection (0, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotToggle(bool)));

    pix = BarIcon("document-open");
    tb1->insertButton(pix, 1, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotOpen()),
                          true, "Open (starts progres in sb)");

    tb1->insertSeparator ();
    
    pix = BarIcon("filefloppy");
    tb1->insertButton(pix, 2, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotSave()),
                      true, "Save file2 (autorepeat)");
    tb1->setAutoRepeat(2);
    
    pix = BarIcon("document-print");
    tb1->insertButton(pix, 3, itemsMenu, true, "Print (pops menu)");
    
    tb1->insertSeparator ();
    /**** RADIO BUTTONS */
    pix = BarIcon("document-new");
    tb1->insertButton(pix, 4, true, "Radiobutton1");
    tb1->setToggle(4);

    pix = BarIcon("document-open");
    tb1->insertButton(pix, 5, true, "Radiobutton2");
    tb1->setToggle(5);
    
    pix = BarIcon("filefloppy");
    tb1->insertButton(pix, 6, true, "Radiobutton3");
    tb1->setToggle(6);
    
    pix = BarIcon("document-print");
    tb1->insertButton(pix, 7, true, "Radiobutton4");
    tb1->setToggle(7);

    //Create
    rg = new TDEToolBarRadioGroup (tb1);

    rg->addButton(4);
    rg->addButton(5);
    rg->addButton(6);
    rg->addButton(7);

    connect (tb1, TQ_SIGNAL(toggled(int)), this, TQ_SLOT(slotToggled(int)));
    
    // Set caption for floating toolbars
    tb->setTitle ("Toolbar 1");
    tb1->setTitle ("Toolbar 2");

    // Set main widget. In this example it is Qt's multiline editor.
    widget = new TQMultiLineEdit (this);

    // Setup is now complete

    // add two toolbars
    //addToolBar (tb1);
    //addToolBar (tb);

    connect (tb, TQ_SIGNAL(highlighted(int,bool)), this, TQ_SLOT(slotMessage(int, bool)));
    connect (tb1, TQ_SIGNAL(highlighted(int, bool)), this, TQ_SLOT(slotMessage(int, bool)));

    // Floating is enabled by default, so you don't need this.
    // tb->enableFloating(true);
    // tb1->enableFloating(true);

    // Show toolbars
    tb->show();
    tb1->show();

    //... and main widget
    setCentralWidget (widget);

    // This is not strictly related to toolbars, menubars or TDEMainWindow.
    // Setup popup for completions
    completions = new TQPopupMenu;
  
    completions->insertItem("/");
    completions->insertItem("/usr/");
    completions->insertItem("/lib/");
    completions->insertItem("/var/");
    completions->insertItem("/bin/");
    completions->insertItem("/kde/");
    completions->insertItem("/home/");
    completions->insertItem("/vmlinuz :-)");

    connect (completions, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotCompletionsMenu(int)));
    pr = 0;
}
/***********************************/
/*  Now slots for toolbar actions  */
/***********************************/
void testWindow::slotToggled(int)
{
  statusBar->message ("Buton toggled", 1500);
}

void testWindow::slotInsertClock()
{
  //DigitalClock *clock = new DigitalClock(tb1);
  //clock->setFrameStyle(TQFrame::NoFrame);
  //tb1->insertWidget(8, 70, clock);
}

void testWindow::slotNew()
{
 tb1->toggleButton(0);
 toolBar()->removeItem( itemId );
}
void testWindow::slotOpen()
{
  if (pr == 0)
    pr = new TQProgressBar (statusBar);
//  statusBar->message(pr);
  timer = new TQTimer (pr);

  connect (timer, TQ_SIGNAL(timeout()), this, TQ_SLOT(slotGoGoGoo()));
  timer->start(100);
}

void testWindow::slotGoGoGoo()
{
  pr->setProgress(pr->progress()+1);
  if (pr->progress()==100)
  {
    timer->stop();
    statusBar->clear();
    delete pr;
    pr = 0;
  }
}

void testWindow::slotSave()
{
  kapp->beep();
  statusBar->changeItem("Saving properties...", 0);
}

void testWindow::slotPrint()
{
    statusBar->changeItem("Print file pressed", 0);
    ena=!ena;
    tb->setItemEnabled(1,ena );
}
void testWindow::slotReturn()
{
    TQString s = "You entered ";
    s = s + tb->getLinedText(5);
    statusBar->changeItem(s, 0);

}
void testWindow::slotList(const TQString &str)
{
    TQString s = "You chose ";
    s = s + str;
    statusBar->changeItem(s, 0);
}

void testWindow::slotCompletion()
{
  // Now do a completion
  // Call your completing function and set that text in klined
  // TQString s = tb->getLinedText(/* ID */ 4)
  // TQString completed = complete (s);
  // tb->setLinedText(/* ID */ 4, completed.data())

  // for now this:

  completions->popup(TQCursor::pos()); // This popup should understunf keys up and down

  /* This is just an example. KLined automatically sets cursor at end of string
   when ctrl-d or ctrl-s is pressed. TDEToolBar will also put cursor at end of text in Lined
   after inserting text with setLinedText (...).
  */

}

void testWindow::slotListCompletion()
{
    /*
     Combo is not behaving good and it is ugly. I will see how it behaves in Qt-1.3,
     and then decide should I make a new combobox.
     */
  TQString s(tb->getComboItem(4));  // get text in combo
  s+= "(completing)";
  //tb->getCombo(4)->changeItem(s.data());   // setTextIncombo

}

void testWindow::slotCompletionsMenu(int id)
{
  // Now set text in lined
  TQString s =completions->text(id);
  tb->setLinedText(5, s);  // Cursor is automatically at the end of string after this
}

void testWindow::slotHide2 ()
{
  tb1->show();
}

void testWindow::slotHide1 ()
{
  tb->show();
}

testWindow::~testWindow ()
{
  /********************************************************/
  /*                                                      */
  /*   THIS IS NOT ANY MORE IMPORTANT BUT ALLOWED!!!      */
  /*                                                      */
  /********************************************************/

  delete tb1->getWidget(8);
  //debug ("twindowtest: deleted clock");
  
  delete tb;
  delete tb1;
  delete menuBar;

  tqDebug ("twindowtest finished");
}

void testWindow::beFixed()
{
    widget->setFixedSize (400, 200);
}

void testWindow::beYFixed()
{
    widget->setMinimumSize(400, 200);
    widget->setMaximumSize(9999, 200);
}

void testWindow::slotImportant ()
{
  statusBar->message("This important message will go away in 15 seconds", 15000);
}

void testWindow::slotExit ()
{
  if (exitB == true)
   {
     tb->removeItem(6);
     exitB = false;
   }
  else
   {
     TQPixmap pix;
     pix = BarIcon("system-log-out");
     tb->insertButton(pix, 6, TQ_SIGNAL(clicked()), TDEApplication::kApplication(),
                           TQ_SLOT( quit() ), true, "Exit");
     tb->alignItemRight (6);
     exitB = true;
   }
}

void testWindow::slotLined()
{
  lineL = !lineL;
  tb->setItemEnabled(5, lineL); // enable/disable lined
}

void testWindow::slotToggle (bool on)
{
  if (on == true)
    statusBar->changeItem("Toggle is on", 0);
  else
    statusBar->changeItem("Toggle is off", 0);
}

void testWindow::slotFrame()
{
#if 0
  if (greenF == false)
   {
     tb1->insertFrame(10, 100);
     tb1->alignItemRight (10); // this is pointless 'cause tb1 is not fullwidth

     TQFrame *myFrame = tb1->getFrame(10); // get frame pointer

     if (myFrame == 0)
      {
        warning ("bad frame ID");
        return;
      }

     //paint it green
     // Or do whatever you want with it, just don't change its height (height = hardcoded = 24)
     // And don't move it
     // If you want to have something right from your toolbar you can reduce its
     // max_width with setMaxWidth()
     myFrame->setBackgroundColor (TQColor("green"));

     greenF = true;
   }
  else
   {
     tb1->removeItem (10);
     greenF = false;
   }
#endif
}

void testWindow::slotMessage(int, bool boo)
{
  if (boo)
    statusBar->message("This button does this and that", 1500);
  else
    statusBar->clear();
}
// Now few Combo slots, for Torben

void testWindow::slotClearCombo()
{
  tb->getCombo(4)->clear();
}

void testWindow::slotInsertListInCombo()
{
  TQStringList list;
  list.append("ListOne");
  list.append("ListTwo");
  list.append("ListThree");
  list.append("ListFour");
  list.append("ListFive");
  list.append("ListSix");
  list.append("ListSeven");
  list.append("ListEight");
  list.append("ListNine");
  list.append("ListTen");
  list.append("ListEleven");
  list.append("ListAndSoOn");
  tb->getCombo(4)->insertStringList (list,0);
}

void testWindow::slotMakeItem3Current()
{
  tb->getCombo(4)->setCurrentItem(3);
}

int main( int argc, char *argv[] )
{
    int i;
    TDECmdLineArgs::init(argc, argv, "KWindowTest", "description", "version");

    TDEApplication *myApp = new TDEApplication();
    testWindow *test = new testWindow;

    myApp->setMainWidget(test);

    i = TQMessageBox::information(0, "Select", "Select type of mainwidget",
                             "Fixed", "Y-fixed", "Resizable");
    if (i == 0)
        test->beFixed();
    else if (i == 1)
        test->beYFixed();

    test->show();
    test->resize(400, 500);
    int ret = myApp->exec();

    //delete test;
    return ret;
}

#include "twindowtest.moc"

