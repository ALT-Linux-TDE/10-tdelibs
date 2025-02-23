/*
 testqtargs -- is there really a bug in TDECmdLineArgs or am I on crack?

  I used the following compile options:

  g++ -g -Wall -o testqtargs testqtargs.cpp  -I/usr/X11R6/include \
  -I/opt/qt3/include -I/opt/trinity/include -L/usr/X11R6/lib -L/opt/qt3/lib \
  -L/opt/trinity/lib -lqt -ltdecore

  if invoked like this ./testqtargs --bg blue --caption something --hello hi

  The program should list argv[] then produce output like this:
  
  qt arg[0] = background
  qt arg[1] = blue
  arg bg = blue
  arg caption = something
  arg hello = hi

  Instead for me it prints:

  qt arg[0] = -background
  qt arg[1] = blue
  arg caption = something
  arg hello = hi

  See the extra dash in qt arg[0]?  I believe that is the cause of the problem.
  --bg is aliased to --background but If you try it with --background or 
  -background, you get the same thing.

  in tdecore/tdeapplication.cpp, TDECmdLineOption qt_options is defined and used 
  by the static method Kapplication::addCmdLineOptions to add the Qt options
  but its' entries look like this:

  { "background <color>", I18N_NOOP("sets the default background color and an\n
application palette (light and dark shades are\ncalculated)."), 0},

  it looks for "background"  instead of "-background" so never find the arg.

  Software:  g++ 2.95, tdelibs from CVS Jan 28, Qt 3.01
  OS: Debian GNU/Linux 3.0 (sid)


*/

#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>
#include <tdelocale.h>

static const TDECmdLineOptions options[] =
{
  { "hello ", I18N_NOOP("Says hello"), 0 },
  TDECmdLineLastOption
};

int main(int argc, char *argv[])
{
  for (int i = 0; i < argc; i++)
  {
    tqDebug("argv[%d] = %s", i, argv[i]);
  }
  TDEAboutData aboutData( "testqtargs", I18N_NOOP("testqtargs"),
    "1.0", I18N_NOOP("testqtargs"), TDEAboutData::License_GPL,
    "", "", "", "");

  TDECmdLineArgs::init(argc, argv, &aboutData);
  TDECmdLineArgs::addCmdLineOptions(options);

  TDECmdLineArgs *qtargs = TDECmdLineArgs::parsedArgs("qt");
  for (int i = 0; i < qtargs->count(); i++)
  {
    tqDebug("qt arg[%d] = %s", i, qtargs->arg(i));
  }

  TDEApplication app;

  TDECmdLineArgs *kdeargs = TDECmdLineArgs::parsedArgs("tde");
  TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

  // An arg set by Qt
  if(qtargs->isSet("background"))
  {
    tqDebug("arg bg = %s", (const char*)qtargs->getOption("background"));
  }
  // An arg set by KDE
  if(kdeargs->isSet("caption"))
  {
    tqDebug("arg caption = %s", (const char*)kdeargs->getOption("caption"));
  }
  // An arg set by us.
  if(args->isSet("hello"))
  {
    tqDebug("arg hello = %s", (const char*)args->getOption("hello"));
  }
  args->clear();

  TQWidget *w = new TQWidget();
  app.setMainWidget(w);
  w->show();

  return app.exec();
}

