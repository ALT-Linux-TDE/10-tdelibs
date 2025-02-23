#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>
#include <kstandarddirs.h>

#include "addressbook.h"
#include "plugins/file/resourcefile.h"
#include "formats/binaryformat.h"
#include "vcardformat.h"
#include "phonenumber.h"

using namespace TDEABC;

static const TDECmdLineOptions options[] =
{
  { "save", "", 0 },
  { "number", "", 0 },
  TDECmdLineLastOption
};

int main(int argc,char **argv)
{
  TDEAboutData aboutData("testaddressee","TestAddressee","0.1");
  TDECmdLineArgs::init(argc, argv, &aboutData);
  TDECmdLineArgs::addCmdLineOptions(options);

  TDEApplication app;
  TDECmdLineArgs* args = TDECmdLineArgs::parsedArgs();

  kdDebug() << "Creating a" << endl;
  Addressee a;
  
  kdDebug() << "tick1" << endl;
  a.setGivenName("Hans");
  kdDebug() << "tick2" << endl;
  a.setPrefix("Dr.");

  kdDebug() << "Creating b" << endl;
  Addressee b( a );
  
  kdDebug() << "tack1" << endl;
  a.setFamilyName("Wurst");
  kdDebug() << "tack2" << endl;
  a.setNickName("hansi");

  kdDebug() << "Creating c" << endl;
  Addressee c = a;
  
  kdDebug() << "tock1" << endl;
  c.setGivenName("Eberhard");
  
  a.dump();  
  b.dump();
  c.dump();
}
