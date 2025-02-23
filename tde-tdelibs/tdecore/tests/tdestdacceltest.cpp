#include <config.h>

#include <tdeapplication.h>
#include <kdebug.h>
#include <tdestdaccel.h>
#include <stdlib.h> // for exit

static bool check(TQString txt, TQString a, TQString b)
{
  if (a.isEmpty())
     a = TQString::null;
  if (b.isEmpty())
     b = TQString::null;
  if (a == b) {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

int main(int argc, char *argv[])
{
  TDEApplication::disableAutoDcopRegistration();
  TDEApplication app(argc,argv,"tdestdacceltest",false,false);

  check( "shortcutDefault FullScreen", TDEStdAccel::shortcutDefault( TDEStdAccel::FullScreen ).toString(), "Ctrl+Shift+F" );
  check( "shortcutDefault BeginningOfLine", TDEStdAccel::shortcutDefault( TDEStdAccel::BeginningOfLine ).toString(), "Home" );
  check( "shortcutDefault EndOfLine", TDEStdAccel::shortcutDefault( TDEStdAccel::EndOfLine ).toString(), "End" );

  check( "name BeginningOfLine", TDEStdAccel::name( TDEStdAccel::BeginningOfLine ), "BeginningOfLine" );
  check( "name EndOfLine", TDEStdAccel::name( TDEStdAccel::EndOfLine ), "EndOfLine" );

  check( "shortcut method", TDEStdAccel::shortcut( TDEStdAccel::ZoomIn ).toString(), TDEStdAccel::zoomIn().toString() );

  return 0;
}
