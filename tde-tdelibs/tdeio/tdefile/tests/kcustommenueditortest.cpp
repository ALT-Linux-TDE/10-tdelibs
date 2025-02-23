#include "kcustommenueditor.h"
#include <tdeapplication.h>
#include <tdelocale.h>
#include <tdeconfig.h>

int main(int argc, char** argv)
{
  TDELocale::setMainCatalogue("tdelibs");
  TDEApplication app(argc, argv, "KCustomMenuEditorTest", true);
  KCustomMenuEditor editor(0);
  TDEConfig *cfg = new TDEConfig("kdesktop_custom_menu2");
  editor.load(cfg);
  if (editor.exec())
  {
     editor.save(cfg);
     cfg->sync();
  }
}

