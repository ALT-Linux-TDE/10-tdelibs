#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>
#include <kstandarddirs.h>

#include "addressbook.h"
#include "address.h"

using namespace TDEABC;

static const TDECmdLineOptions options[] =
{
  { "save", "", 0 },
  { "number", "", 0 },
  TDECmdLineLastOption
};

int main(int argc,char **argv)
{
  TDEAboutData aboutData("testaddressfmt","TestAddressFormat","0.1");
  TDECmdLineArgs::init(argc, argv, &aboutData);
  TDECmdLineArgs::addCmdLineOptions(options);

  TDEApplication app;

  Address a;
  a.setStreet("Lummerlandstr. 1");
  a.setPostalCode("12345");
  a.setLocality("Lummerstadt");
  a.setCountry ("Germany");

  Address b;
  b.setStreet("457 Foobar Ave");
  b.setPostalCode("1A2B3C");
  b.setLocality("Nervousbreaktown");
  b.setRegion("DC");
  b.setCountry("United States of America");

  Address c;
  c.setStreet("Lummerlandstr. 1");
  c.setPostalCode("12345");
  c.setLocality("Lummerstadt");
  c.setCountry ("Deutschland");

  Address d;
  d.setStreet("Lummerlandstr. 1");
  d.setPostalCode("12345");
  d.setLocality("Lummerstadt");
  d.setCountry ("");

  tqDebug( "-------------------------------------\nShould have german address formatting, local country formatting\n" );
  tqDebug( "%s", a.formattedAddress("Jim Knopf").latin1() );
  tqDebug( "-------------------------------------\nShould have US address formatting, local country formatting\n" );
  tqDebug( "%s", b.formattedAddress("Huck Finn").latin1() );
  tqDebug( "-------------------------------------\nShould have german address formatting, local country formatting\n" );
  tqDebug( "%s", c.formattedAddress("Jim Knopf").latin1() );
  tqDebug( "-------------------------------------\nShould have local address formatting, local country formatting\n" );
  tqDebug( "%s", d.formattedAddress("Jim Knopf").latin1() );
}


