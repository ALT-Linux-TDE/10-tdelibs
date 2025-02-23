#include "kpixmapregionselectordialog.h"
#include <tqpixmap.h>
#include <tqimage.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <iostream>

static const TDECmdLineOptions options[] =
{
   { "+file", "The image file to open", 0 },
   TDECmdLineLastOption
};

int main(int argc, char**argv)
{
  TDECmdLineArgs::init(argc, argv, "test", "test" ,"test" ,"1.0");
  TDECmdLineArgs::addCmdLineOptions( options );
  TDEApplication app("test");

  TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
  if (args->count()!=1)
  {
    std::cout << "Usage: kpixmapregionselectordialogtest <imageFile>" << std::endl;
    return 1;
  }

  TQImage image=
     KPixmapRegionSelectorDialog::getSelectedImage(TQPixmap(args->arg(0)),100,100);

  image.save("output.png", "PNG");

  return 0;
}
