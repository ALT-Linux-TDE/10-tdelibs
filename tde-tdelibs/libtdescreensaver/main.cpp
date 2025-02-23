/* This file is part of the KDE libraries

    Copyright (c) 2001  Martin R. Jones <mjones@kde.org>

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
#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <tqdialog.h>
#include <tdelocale.h>
#include <tdeglobal.h>
#include <kdebug.h>
#include <tdecmdlineargs.h>
#include <tdeapplication.h>
#include <kcrash.h>

#include "tdescreensaver.h"
#include "tdescreensaver_vroot.h"

bool argb_visual = FALSE;

extern "C"
{
    extern const char *kss_applicationName;
    extern const char *kss_description;
    extern const char *kss_version;
    KScreenSaver *kss_create( WId d );
    TQDialog *kss_setup();
}

static const TDECmdLineOptions options[] =
{
  { "setup", I18N_NOOP("Setup screen saver"), 0 },
  { "window-id wid", I18N_NOOP("Run in the specified XWindow"), 0 },
  { "root", I18N_NOOP("Run in the root XWindow"), 0 },
  { "demo", I18N_NOOP("Start screen saver in demo mode"), "default"},
  TDECmdLineLastOption
};

static void crashHandler( int  )
{
#ifdef SIGABRT
    signal (SIGABRT, SIG_DFL);
#endif
    abort();
}

//----------------------------------------------------------------------------

class DemoWindow : public TQWidget
{
public:
    DemoWindow() : TQWidget()
    {
	setFixedSize(600, 420);
    }

protected:
    virtual void keyPressEvent(TQKeyEvent *e)
    {
        if (e->ascii() == 'q')
        {
            kapp->quit();
        }
    }

    virtual void closeEvent( TQCloseEvent * )
    {
        kapp->quit();
    }
};


//----------------------------------------------------------------------------
#if defined(TQ_WS_QWS) || defined(TQ_WS_MACX)
typedef WId Window;
#endif

TDE_EXPORT int main(int argc, char *argv[])
{
    TDELocale::setMainCatalogue("libtdescreensaver");
    TDECmdLineArgs::init(argc, argv, kss_applicationName, kss_description, kss_version);

    TDECmdLineArgs::addCmdLineOptions(options);

#ifdef HAVE_XCOMPOSITE
    TDEApplication app(TDEApplication::openX11RGBADisplay());
    argb_visual = app.isX11CompositionAvailable();
#else
    TDEApplication app;
#endif

    TDECrash::setCrashHandler( crashHandler );
    TDEGlobal::locale()->insertCatalogue("klock");
    TDEGlobal::locale()->insertCatalogue("tdescreensaver");

    DemoWindow *demoWidget = 0;
    Window saveWin = 0;
    KScreenSaver *target;

    TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

    if (args->isSet("setup"))
    {
       TQDialog *dlg = kss_setup();
       args->clear();
       dlg->exec();
       delete dlg;
       exit(0);
    }

    if (args->isSet("window-id"))
    {
        saveWin = atol(args->getOption("window-id"));
    }

#ifdef TQ_WS_X11 //FIXME
    if (args->isSet("root"))
    {
        saveWin = RootWindow(tqt_xdisplay(), tqt_xscreen());
    }
#endif

    if (args->isSet("demo"))
    {
        saveWin = 0;
    }

    if (saveWin == 0)
    {
        demoWidget = new DemoWindow();
        demoWidget->setBackgroundMode(TQWidget::NoBackground);
        saveWin = demoWidget->winId();
        app.setMainWidget(demoWidget);
        app.processEvents();
    }

    target = kss_create( saveWin );

    if ( demoWidget )
    {
        demoWidget->setFixedSize( 600, 420 );
        demoWidget->show();
    }
    args->clear();
    app.exec();

    delete target;
    delete demoWidget;

    return 0;
}

