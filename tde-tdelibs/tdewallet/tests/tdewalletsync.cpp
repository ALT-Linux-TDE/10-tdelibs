#include <tqtextstream.h>
#include <tqtimer.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <kstandarddirs.h>
#include <dcopclient.h>
#include <tdewallet.h>

static TQTextStream _out( stdout, IO_WriteOnly );

void openWallet()
{
	_out << "About to ask for wallet sync" << endl;

	TDEWallet::Wallet *w = TDEWallet::Wallet::openWallet( TDEWallet::Wallet::NetworkWallet(), 0, TDEWallet::Wallet::Synchronous );

	_out << "Got sync wallet: " << (w != 0) << endl;
}

int main( int argc, char *argv[] )
{
	TDEAboutData aboutData( "tdewalletsync", "tdewalletsync", "version" );
	TDECmdLineArgs::init( argc, argv, &aboutData );
	TDEApplication app( "tdewalletsync" );

	// register with DCOP
	_out << "DCOP registration returned " << app.dcopClient()->registerAs(app.name()) << endl;

	openWallet();

	return 0;
}
