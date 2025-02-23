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

#include "tdewallettest.h"

static TQTextStream _out( stdout, IO_WriteOnly );

void openWallet()
{
	_out << "About to ask for wallet async" << endl;

        // we have no wallet: ask for one.
	TDEWallet::Wallet *wallet = TDEWallet::Wallet::openWallet( TDEWallet::Wallet::NetworkWallet(), 0, TDEWallet::Wallet::Asynchronous );

	WalletReceiver r;
	r.connect( wallet, TQ_SIGNAL( walletOpened(bool) ), TQ_SLOT( walletOpened(bool) ) );

	_out << "About to start 30 second event loop" << endl;

	TQTimer::singleShot( 30000, tqApp, TQ_SLOT( quit() ) );
	int ret = tqApp->exec();

	if ( ret == 0 )
		_out << "Timed out!" << endl;
	else
		_out << "Success!" << endl;
}

void WalletReceiver::walletOpened( bool got )
{
	_out << "Got async wallet: " << got << endl;
	tqApp->exit( 1 );
}

int main( int argc, char *argv[] )
{
	TDEAboutData aboutData( "tdewalletasync", "tdewalletasync", "version" );
	TDECmdLineArgs::init( argc, argv, &aboutData );
	TDEApplication app( "tdewalletasync" );

	// register with DCOP
	_out << "DCOP registration returned " << app.dcopClient()->registerAs(app.name()) << endl;

	openWallet();

	return 0;
}
