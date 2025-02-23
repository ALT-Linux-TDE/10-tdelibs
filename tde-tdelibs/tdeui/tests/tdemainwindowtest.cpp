#include <tqtimer.h>
#include <tqlabel.h>

#include <tdeapplication.h>
#include <kstatusbar.h>
#include <tdemenubar.h>

#include "tdemainwindowtest.h"

MainWindow::MainWindow()
{
    TQTimer::singleShot( 2*1000, this, TQ_SLOT( showMessage() ) );

    setCentralWidget( new TQLabel( "foo", this ) );

    menuBar()->insertItem( "hi" );
}

void MainWindow::showMessage()
{
    statusBar()->show();
    statusBar()->message( "test" );
}

int main( int argc, char **argv )
{
    TDEApplication app( argc, argv, "tdemainwindowtest" );

    MainWindow* mw = new MainWindow; // deletes itself when closed
    mw->show();

    return app.exec();
}

#include "tdemainwindowtest.moc"
