#include "kdatetimewidget.h"
#include <tdeapplication.h>
#include <tdelocale.h>

int main(int argc, char** argv)
{
  TDELocale::setMainCatalogue("tdelibs");
  TDEApplication app(argc, argv, "KDateTimeWidgettest");
  KDateTimeWidget dateTimeWidget;
  app.setMainWidget(&dateTimeWidget);
  dateTimeWidget.show();
  // dateTimeWidget.setEnabled(false);
  return app.exec();
}
