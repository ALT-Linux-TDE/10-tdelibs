#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>

#include "addressbook.h"
#include "vcardformat.h"
#include "resourcesql.h"

using namespace TDEABC;

int main(int argc,char **argv)
{
  TDEAboutData aboutData("testdb","TestKabcDB","0.1");
  TDECmdLineArgs::init(argc,argv,&aboutData);

//  TDEApplication app( false, false );
  TDEApplication app;

  AddressBook ab;
  
  ResourceSql r( &ab, "root", "kde4ever", "tdeabc", "localhost" );
  if ( ! r.open() ) {
    kdDebug() << "Failed to open resource." << endl;
  }
  
  r.load();
  
  r.close();
  
  ab.dump();
}
