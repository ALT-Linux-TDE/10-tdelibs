#include "ksqueezedtextlabel.h"
#include <tdeapplication.h>

int main( int argc, char **argv )
{
	TDEApplication app( argc, argv, "KSqueezedTextLabelTest" );

	KSqueezedTextLabel l( "This is a rather long string", 0);
	app.setMainWidget( &l );
	l.show();

	return app.exec();
}
