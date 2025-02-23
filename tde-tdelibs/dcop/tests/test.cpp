#include "test.h"
#include <tdeapplication.h>
#include <iostream>
#include <dcopclient.h>
#include <tdecmdlineargs.h>



Test::~Test()
{
}

#include "definitions.generated"

using namespace std;

void batch()
{
	TQTextStream output(  fopen( "batch.returns", "w" ) , IO_WriteOnly );	
	Test* object = new Test;
#include "batch.generated"
}

#ifdef Q_OS_WIN
# define main kdemain
#endif

int main(int argc, char** argv)
{
	if ( argc > 1 ) {
		batch();
		return 0;
	}
	TDECmdLineArgs::init( argc, argv, argv[0], "TestApp", "Tests the dcop familly of tools + libraries", "1.0" ); // FIXME
	TDEApplication app (/*stylesEnabled=*/ false, /*GUIEnabled=*/ false);
	if(!app.dcopClient()->attach(  ))
		return 1;

	app.dcopClient()->registerAs( "TestApp" );
	new Test;
	return app.exec();
}
	
	
