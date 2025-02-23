#include "kdatewidget.h"
#include <tqlineedit.h>
#include <tdeapplication.h>
#include <tdelocale.h>

int main(int argc, char** argv)
{
  TDELocale::setMainCatalogue("tdelibs");
  TDEApplication app(argc, argv, "KDateWidgettest");
  KDateWidget dateWidget;
  app.setMainWidget(&dateWidget);
  dateWidget.show();
  // dateWidget.setEnabled(false);
  return app.exec();
}

