#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <kdebug.h>
#include <tqstring.h>
#include <stdio.h>
#include <unistd.h>

#include "java/kjavaappletserver.h"
#include "java/kjavaapplet.h"
#include "java/kjavaappletwidget.h"

static TDECmdLineOptions options[] =
{
    { "+[tdelibs_path]", "path to tdelibs directory", 0 },
    TDECmdLineLastOption
};

int main(int argc, char **argv)
{
    TDECmdLineArgs::init( argc, argv, "testKJASSever", "testKJASServer", "test program", "0.0" );

    TDECmdLineArgs::addCmdLineOptions( options );

    TDEApplication app;

    TQString path_to_tdelibs = "/build/wynnw/kde-src";

    KJavaAppletWidget *a = new KJavaAppletWidget;

    a->show();

    a->applet()->setBaseURL( "file:" + path_to_tdelibs + "/tdelibs/tdehtml/test/" );
    a->applet()->setAppletName( "Lake" );
    a->applet()->setAppletClass( "lake.class" );
    a->applet()->setParameter( "image", "konqi.gif" );

    a->showApplet();
    a->applet()->start();

    app.exec();
}
