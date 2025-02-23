#include <tqwidget.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>

#include "addresseedialog.h"

using namespace TDEABC;

static const TDECmdLineOptions options[] =
{
  {"multiple", I18N_NOOP("Allow selection of multiple addressees"), 0},
  TDECmdLineLastOption
};

int main(int argc,char **argv)
{
  TDEAboutData aboutData("testkabcdlg",I18N_NOOP("TestKabc"),"0.1");
  TDECmdLineArgs::init(argc,argv,&aboutData);
  TDECmdLineArgs::addCmdLineOptions( options );

  TDEApplication app;

  TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
  if (args->isSet("multiple")) {
    Addressee::List al = AddresseeDialog::getAddressees( 0 );
    Addressee::List::ConstIterator it;
    kdDebug() << "Selected Addressees:" << endl;
    for( it = al.begin(); it != al.end(); ++it ) {
      kdDebug() << "  " << (*it).fullEmail() << endl;
    }
  } else {
    Addressee a = AddresseeDialog::getAddressee( 0 );
  
    if ( !a.isEmpty() ) {
      kdDebug() << "Selected Addressee:" << endl;
      a.dump();
    } else {
      kdDebug() << "No Addressee selected." << endl;
    }
  }
}
