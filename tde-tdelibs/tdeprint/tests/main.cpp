/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "helpwindow.h"
#include <tdeapplication.h>
#include <tqwindowsstyle.h>
#include <tqstylesheet.h>
#include <stdlib.h>


int main( int argc, char ** argv )
{
    TDEApplication a(argc, argv, "helpviewer");

    TQString home;
    if (argc > 1)
        home = argv[1];
    else
        home = TQString(getenv("TQTDIR")) + "/doc/html/index.html";

    
    HelpWindow *help = new HelpWindow(home, ".", 0, "help viewer");

    if ( TQApplication::desktop()->width() > 400
	 && TQApplication::desktop()->height() > 500 )
	help->show();
    else
	help->showMaximized();

    TQObject::connect( &a, TQ_SIGNAL(lastWindowClosed()),
                      &a, TQ_SLOT(quit()) );

    return a.exec();
}
