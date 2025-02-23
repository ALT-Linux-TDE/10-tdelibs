#ifndef TDEWALLETASYNC_H
#define TDEWALLETASYNC_H

#include <tqobject.h>

namespace TDEWallet { class Wallet; }

class WalletReceiver : public TQObject
{
	TQ_OBJECT
public slots:
	void walletOpened( bool );
};

#endif
