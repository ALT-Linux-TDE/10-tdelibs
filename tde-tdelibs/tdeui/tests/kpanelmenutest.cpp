#include "kpanelmenutest.h"
#include <tdeapplication.h>
#include <kstandarddirs.h>
#include <dcopclient.h>

TestWidget::TestWidget(TQWidget *parent, const char *name)
    : TQLabel(parent, name)
{
    testMenu = new KPanelAppMenu(locate("mini", "x.png"), "Client Test", this,
                              "menu1");

    subMenu = testMenu->insertMenu(locate("mini", "x.png"), "Submenu Test");
    subMenu->insertItem(locate("mini", "bx2.png"), "First Entry", 100);
    subMenu->insertItem(locate("mini", "bx2.png"), "Second Entry", 101);

    KPanelAppMenu *ssub = subMenu->insertMenu(locate("mini", "x.png"), "One more");
   
    connect(testMenu, TQ_SIGNAL(activated(int)), TQ_SLOT(slotMenuCalled(int)));
    connect(subMenu, TQ_SIGNAL(activated(int)), TQ_SLOT(slotSubMenuCalled(int)));
    init();
    
    setText("We added \"Client Test\" to kicker. Click the TDE Menu to check.");
    resize(sizeHint());
}

void TestWidget::init()
{
    testMenu->insertItem(locate("mini", "bx2.png"),
                                "Add another entry!", 1);
    testMenu->insertItem("Attention, this will clear the menu", 2);
}

void TestWidget::slotMenuCalled(int id)
{
    switch ( id ) {
    case 1:
	setText("Selected \"Add another entry!\" ");
	testMenu->insertItem("Another entry");
    break;
    case 2:
	setText("Selected \"Attention, this will clear the menu\"");
	testMenu->clear();
	init();
    break;
    default:
	setText("Selected \"Another entry...\"");
    }
}


void TestWidget::slotSubMenuCalled(int id)
{
  TQString msg("Called with id=%1");
  setText(msg.arg(id));
}


int main(int argc, char **argv)
{
    TDEApplication *app = new TDEApplication(argc, argv, "menutest");
    
    TestWidget w;
    app->setMainWidget(&w);
    w.show();
    
    app->exec();
}

#include "kpanelmenutest.moc"



