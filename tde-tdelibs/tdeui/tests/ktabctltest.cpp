/*
 *
 */

#include <tdeapplication.h>
#include <tqpushbutton.h>
#include <tqlabel.h>
#include <tqobject.h>
#include <tqlistbox.h>
#include <tqgroupbox.h>
#include <tqevent.h>
#include <tqcombobox.h>
#include <tqlineedit.h>
#include <tqradiobutton.h>
#include <tqcheckbox.h>
#include <tqtabdialog.h>
#include <tqtooltip.h>
#include <tqmessagebox.h>
#include <tqtabbar.h>
#include <tqpalette.h>
#include <tqmultilineedit.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ktabctl.h"
#include "ktabctltest.h"

TQFont default_font("Helvetica", 12);

TDEApplication *a;

TopLevel::TopLevel(TQWidget *parent, const char *name)
    : TQWidget(parent, name)
{
    setCaption("KTabCtl test application");
    setMinimumSize(300, 200);

    /*
     * add a tabctrl widget
     */
    
    test = new KTabCtl(this, "test");
    connect(test, TQ_SIGNAL(tabSelected(int)), this, TQ_SLOT(tabChanged(int)));
    TQWidget *w = new TQWidget(test, "_page1");
    TQPushButton *bt = new TQPushButton("Click me to quit", w, "_bt1");
    connect(bt, TQ_SIGNAL(clicked()), this, TQ_SLOT(okPressed()));
    bt->adjustSize();
    bt->move(20, 20);
    test->addTab(w, "Seite 1");
    pages[0] = w;
    w = new TQWidget(test, "_page2");
    e = new TQMultiLineEdit(w, "_editor");
    e->setText("Write some usesless stuff here :-)");
    w->resize(width(), height());
    test->addTab(w, "Seite 2");
    pages[1] = w;
    w = new TQWidget(test, "_page3");
    bt = new TQPushButton("This button does absolutely nothing", w, "_bt3");
    bt->adjustSize();
    bt->move(20, 20);
    test->addTab(w, "Seite 3");
    pages[2] = w;
    test->resize(200, 200);
    test->move(0, 0);
    move(20, 20);
    resize(400, 300);
    adjustSize();
}

void TopLevel::resizeEvent( TQResizeEvent * )
{
    test->resize(width(), height());
    e->setGeometry(10, 10, pages[1]->width() - 20, pages[1]->height() - 20);
}

void TopLevel::tabChanged(int newpage)
{
    printf("tab number %d selected\n", newpage);
    if(newpage == 1)
        e->setFocus();
}

void TopLevel::okPressed()
{
    a->quit();
}

int main( int argc, char ** argv )
{
    a = new TDEApplication ( argc, argv, "KTabCtlTest" );

    a->setFont(default_font);

    TopLevel *toplevel = new TopLevel(0, "_ktabctl_test");

    toplevel->show();
    a->setMainWidget(toplevel);
    a->exec();
}

#include "ktabctltest.moc"

