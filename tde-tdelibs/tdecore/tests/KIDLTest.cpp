#include "KIDLTest.h"

#include <tdeapplication.h>
#include <dcopclient.h>

KIDLTest::KIDLTest( const TQCString& id )
    : DCOPObject( id )
{
}

TQString KIDLTest::hello( const TQString& name )
{
    tqDebug("Du heter %s", name.latin1() );
    tqDebug("Ha det %s", name.latin1() );
	
    return TQString("Jeg heter KIDLTest");
}

int main( int argc, char** argv )
{
    TDEApplication app( argc, argv, "kidlservertest", false /* No GUI */ );

    app.dcopClient()->attach();
    app.dcopClient()->registerAs( "kidlservertest" );

    tqDebug("Server process started...");

    (void) new KIDLTest( "Hello" );

    tqDebug("Server listening ...");

    return app.exec();
}
