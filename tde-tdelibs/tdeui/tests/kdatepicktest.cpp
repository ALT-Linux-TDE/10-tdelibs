#include "kdatepicker.h"
#include <tqlineedit.h>
#include <tdeapplication.h>
#include <tdelocale.h>

int main(int argc, char** argv)
{
  TDELocale::setMainCatalogue("tdelibs");
  TDEApplication app(argc, argv, "KDatePickertest");
  KDatePicker picker;
  app.setMainWidget(&picker);
  picker.show();
  // picker.setEnabled(false);
  return app.exec();
}

