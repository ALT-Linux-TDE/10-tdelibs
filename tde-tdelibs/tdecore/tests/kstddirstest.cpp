#include <tdeapplication.h>
#include <kdebug.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <tdeconfig.h>

int main(int argc, char **argv)
{
  TDEApplication a(argc, argv, "whatever", false);
  TDEStandardDirs t;
  TDEConfig config; // to add custom entries - a bit tricky :/

  TQStringList list;
  TQString s;

  t.saveLocation("icon");

  s = t.findResource("icon", "xv.xpm");
  if (!s.isNull()) kdDebug() << s << endl;

  list = t.findAllResources("data", "kfind/toolbar", true);
  for (TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
    kdDebug() << "data " << (*it).ascii() << endl;
  }

  list = t.findAllResources("config", "kcmdisplayrc");
  for (TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
    kdDebug() << "config " << (*it).ascii() << endl;
  }

  list = t.findAllResources("config", "kcmdisplayrc", false, true);
  for (TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
    kdDebug() << "config2 " << (*it).ascii() << endl;
  }

  list = t.findAllResources("html", "en/*/index.html", false);
  for (TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
    kdDebug() << "docs " << (*it).ascii() << endl;
  }

  list = t.findAllResources("html", "*/*/*.html", false);
  for (TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
    kdDebug() << "docs " << (*it).ascii() << endl;
  }

  list = t.findDirs("data", "twin");
  for (TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
    kdDebug() << "twin dirs " << (*it).ascii() << endl;
  }

  kdDebug() << "hit " << t.findResourceDir("config", "kcmdisplayrc") << endl;
}
