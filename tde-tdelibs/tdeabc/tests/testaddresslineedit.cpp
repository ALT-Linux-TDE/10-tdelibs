#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>

#include "addresslineedit.h"

using namespace TDEABC;

int main( int argc,char **argv )
{
  TDEAboutData aboutData( "testaddresslineedit",
                        I18N_NOOP( "Test Address LineEdit" ), "0.1" );
  TDECmdLineArgs::init( argc, argv, &aboutData );

  TDEApplication app;

  AddressLineEdit *lineEdit = new AddressLineEdit( 0 );

  lineEdit->show();
  app.setMainWidget( lineEdit );
  
  TQObject::connect( &app, TQ_SIGNAL( lastWindowClosed() ), &app, TQ_SLOT( quit() ) );

  app.exec();
  
  delete lineEdit;
}
