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

	_out << "About to ask for wallet sync" << endl;

	wallet = TDEWallet::Wallet::openWallet( TDEWallet::Wallet::NetworkWallet(), 0, TDEWallet::Wallet::Synchronous );

	_out << "Got sync wallet: " << (wallet != 0) << endl;
	_out << "About to start 30 second event loop" << endl;

	TQTimer::singleShot( 30000, tqApp, TQ_SLOT( quit() ) );
	int ret = tqApp->exec();


	if ( ret == 0 )
		_out << "Timed out!" << endl;
	else
		_out << "Success!" << endl;

	TQMap<TQString,TQString> p;
	ret = wallet->readPasswordList("*", p);
	_out << "readPasswordList returned: " << ret << endl;
	_out << "readPasswordList returned " << p.keys().count() << " entries" << endl;
	TQMap<TQString, TQMap<TQString, TQString> > q;
	ret = wallet->readMapList("*", q);
	_out << "readMapList returned: " << ret << endl;
	_out << "readMapList returned " << q.keys().count() << " entries" << endl;

	TQMap<TQString, TQByteArray> s;
	ret = wallet->readEntryList("*", s);
	_out << "readEntryList returned: " << ret << endl;
	_out << "readEntryList returned " << s.keys().count() << " entries" << endl;

	delete wallet;
}

void WalletReceiver::walletOpened( bool got )
{
	_out << "Got async wallet: " << got << endl;
	tqApp->exit( 1 );
}

int main( int argc, char *argv[] )
{
	TDEAboutData aboutData( "tdewalletboth", "tdewalletboth", "version" );
	TDECmdLineArgs::init( argc, argv, &aboutData );
	TDEApplication app( "tdewalletboth" );

	// register with DCOP
	_out << "DCOP registration returned " << app.dcopClient()->registerAs(app.name()) << endl;

	openWallet();

	return 0;
}
