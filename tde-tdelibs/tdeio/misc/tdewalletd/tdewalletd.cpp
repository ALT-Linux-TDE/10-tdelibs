/*
   This file is part of the KDE libraries

   Copyright (c) 2002-2004 George Staikos <staikos@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#include "kbetterthankdialogbase.h"
#include "tdewalletwizard.h"
#include "tdewalletd.h"
#include "ktimeout.h"

#include <dcopclient.h>
#include <dcopref.h>
#include <kactivelabel.h>
#include <tdeapplication.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kpassdlg.h>
#include <kstandarddirs.h>
#include <tdewalletentry.h>
#include <twin.h>

#include <tqdir.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqregexp.h>
#include <tqstylesheet.h>
#include <tqvbox.h>

#include <assert.h>

extern "C" {
   TDE_EXPORT KDEDModule *create_tdewalletd(const TQCString &name) {
	   return new TDEWalletD(name);
   }
}


class TDEWalletTransaction {
	public:
		TDEWalletTransaction() {
			tType = Unknown;
			transaction = 0L;
			client = 0L;
			modal = false;
		}

		~TDEWalletTransaction() {
			// Don't delete these!
			transaction = 0L;
			client = 0L;
		}

		enum Type { Unknown, Open, ChangePassword, OpenFail };
		DCOPClient *client;
		DCOPClientTransaction *transaction;
		Type tType;
		TQCString rawappid, returnObject;
		TQCString appid;
		uint wId;
		TQString wallet;
		bool modal;
};


TDEWalletD::TDEWalletD(const TQCString &name)
: KDEDModule(name), _failed(0) {
	srand(time(0));
	_showingFailureNotify = false;
	_transactions.setAutoDelete(true);
	_timeouts = new KTimeout(17);
	_closeIdle = false;
	_idleTime = 0;
	connect(_timeouts, TQ_SIGNAL(timedOut(int)), this, TQ_SLOT(timedOut(int)));
	reconfigure();
	TDEGlobal::dirs()->addResourceType("tdewallet", "share/apps/tdewallet");
	connect(TDEApplication::dcopClient(),
		TQ_SIGNAL(applicationRemoved(const TQCString&)),
		this,
		TQ_SLOT(slotAppUnregistered(const TQCString&)));
	_dw = new KDirWatch(this, "TDEWallet Directory Watcher");
	_dw->addDir(TDEGlobal::dirs()->saveLocation("tdewallet"));
	_dw->startScan(true);
	connect(_dw, TQ_SIGNAL(dirty(const TQString&)), this, TQ_SLOT(emitWalletListDirty()));
}


TDEWalletD::~TDEWalletD() {
	delete _timeouts;
	_timeouts = 0;

	closeAllWallets();
	_transactions.clear();
}


int TDEWalletD::generateHandle() {
	int rc;

	// ASSUMPTION: RAND_MAX is fairly large.
	do {
		rc = rand();
	} while (_wallets.find(rc) || rc == 0);

	return rc;
}


void TDEWalletD::processTransactions() {
	static bool processing = false;

	if (processing) {
		return;
	}

	processing = true;

	// Process remaining transactions
	TDEWalletTransaction *xact;
	while (!_transactions.isEmpty()) {
		xact = _transactions.first();
		TQCString replyType;
		int res;

		assert(xact->tType != TDEWalletTransaction::Unknown);

		switch (xact->tType) {
			case TDEWalletTransaction::Open:
				res = doTransactionOpen(xact->appid, xact->wallet, xact->wId, xact->modal);
				replyType = "int";
				if (!xact->returnObject.isEmpty()) {
					DCOPRef(xact->rawappid, xact->returnObject).send("walletOpenResult", res);
				}

				// multiple requests from the same client
				// should not produce multiple password
				// dialogs on a failure
				if (res < 0) {
					TQPtrListIterator<TDEWalletTransaction> it(_transactions);
					TDEWalletTransaction *x;
					while ((x = it.current()) && x != xact) {
						++it;
					}
					if (x) {
						++it;
					}
					while ((x = it.current())) {
						if (xact->appid == x->appid && x->tType == TDEWalletTransaction::Open && x->wallet == xact->wallet && x->wId == xact->wId) {
							x->tType = TDEWalletTransaction::OpenFail;
						}
						++it;
					}
				}
				break;
			case TDEWalletTransaction::OpenFail:
				res = -1;
				replyType = "int";
				if (!xact->returnObject.isEmpty()) {
					DCOPRef(xact->rawappid, xact->returnObject).send("walletOpenResult", res);
				}
				break;
			case TDEWalletTransaction::ChangePassword:
				doTransactionChangePassword(xact->appid, xact->wallet, xact->wId);
				// fall through - no return
			default:
				_transactions.removeRef(xact);
				continue;
		}

		if (xact->returnObject.isEmpty() && xact->tType != TDEWalletTransaction::ChangePassword) {
			TQByteArray replyData;
			TQDataStream stream(replyData, IO_WriteOnly);
			stream << res;
			xact->client->endTransaction(xact->transaction, replyType, replyData);
		}
		_transactions.removeRef(xact);
	}

	processing = false;
}


void TDEWalletD::openAsynchronous(const TQString& wallet, const TQCString& returnObject, uint wId) {
	DCOPClient *dc = callingDcopClient();
	if (!dc) {
		return;
	}

	TQCString appid = dc->senderId();
	if (!_enabled ||
		!TQRegExp("^[A-Za-z0-9]+[A-Za-z0-9\\s\\-_]*$").exactMatch(wallet)) {
		DCOPRef(appid, returnObject).send("walletOpenResult", -1);
		return;
	}

	TQCString peerName = friendlyDCOPPeerName();

	TDEWalletTransaction *xact = new TDEWalletTransaction;

	xact->appid = peerName;
	xact->rawappid = appid;
	xact->client = callingDcopClient();
	xact->wallet = wallet;
	xact->wId = wId;
	xact->tType = TDEWalletTransaction::Open;
	xact->returnObject = returnObject;
	_transactions.append(xact);

	DCOPRef(appid, returnObject).send("walletOpenResult", 0);

	TQTimer::singleShot(0, this, TQ_SLOT(processTransactions()));
	checkActiveDialog();
}


int TDEWalletD::openPath(const TQString& path, uint wId) {
	if (!_enabled) { // guard
		return -1;
	}

	// FIXME: setup transaction
	int rc = internalOpen(friendlyDCOPPeerName(), path, true, wId);
	return rc;
}


int TDEWalletD::open(const TQString& wallet, uint wId) {
	if (!_enabled) { // guard
		return -1;
	}

	if (!TQRegExp("^[A-Za-z0-9]+[A-Za-z0-9\\s\\-_]*$").exactMatch(wallet)) {
		return -1;
	}

	TQCString appid = friendlyDCOPPeerName();

	TDEWalletTransaction *xact = new TDEWalletTransaction;
	_transactions.append(xact);

	xact->appid = appid;
	xact->client = callingDcopClient();
	xact->transaction = xact->client->beginTransaction();
	xact->wallet = wallet;
	xact->wId = wId;
	xact->tType = TDEWalletTransaction::Open;
	xact->modal = true; // mark dialogs as modal, the app has blocking wait
	TQTimer::singleShot(0, this, TQ_SLOT(processTransactions()));
	checkActiveDialog();
	return 0; // process later
}


// Sets up a dialog that will be shown by tdewallet.
void TDEWalletD::setupDialog( TQWidget* dialog, WId wId, const TQCString& appid, bool modal ) {
	if( wId != 0 ) 
		KWin::setMainWindow( dialog, wId ); // correct, set dialog parent
	else {
		if( appid.isEmpty())
			kdWarning() << "Using tdewallet without parent window!" << endl;
		else
			kdWarning() << "Application '" << appid << "' using tdewallet without parent window!" << endl;
		// allow dialog activation even if it interrupts, better than trying hacks
		// with keeping the dialog on top or on all desktops
		kapp->updateUserTimestamp();
	}
	if( modal )
		KWin::setState( dialog->winId(), NET::Modal );
	else
		KWin::clearState( dialog->winId(), NET::Modal );
	activeDialog = dialog;
}

// If there's a dialog already open and another application tries some operation that'd lead to
// opening a dialog, that application will be blocked by this dialog. A proper solution would
// be to set the second application's window also as a parent for the active dialog, so that
// KWin properly handles focus changes and so on, but there's currently no support for multiple
// dialog parents. Hopefully to be done in KDE4, for now just use all kinds of bad hacks to make
//  sure the user doesn't overlook the active dialog.
void TDEWalletD::checkActiveDialog() {
	if( !activeDialog || !activeDialog->isShown())
		return;
	kapp->updateUserTimestamp();
	KWin::setState( activeDialog->winId(), NET::KeepAbove );
	KWin::setOnAllDesktops( activeDialog->winId(), true );
	KWin::forceActiveWindow( activeDialog->winId());
}

int TDEWalletD::doTransactionOpen(const TQCString& appid, const TQString& wallet, uint wId, bool modal) {
	if (_firstUse && !wallets().contains(TDEWallet::Wallet::LocalWallet())) {
		// First use wizard
		TDEWalletWizard *wiz = new TDEWalletWizard(0);
		setupDialog( wiz, wId, appid, modal );
		int rc = wiz->exec();
		if (rc == TQDialog::Accepted) {
			TDEConfig cfg("tdewalletrc");
			cfg.setGroup("Wallet");
			cfg.writeEntry("First Use", false);
			cfg.writeEntry("Enabled", wiz->_useWallet->isChecked());
			cfg.writeEntry("Close When Idle", wiz->_closeIdle->isChecked());
			cfg.writeEntry("Use One Wallet", !wiz->_networkWallet->isChecked());
			cfg.sync();
			reconfigure();

			if (!wiz->_useWallet->isChecked()) {
				delete wiz;
				return -1;
			}

			// Create the wallet
			TDEWallet::Backend *b = new TDEWallet::Backend(TDEWallet::Wallet::LocalWallet());
			TQByteArray p;
			p.duplicate(wiz->_pass1->text().utf8(), wiz->_pass1->text().length());
			b->open(p);
			b->createFolder(TDEWallet::Wallet::PasswordFolder());
			b->createFolder(TDEWallet::Wallet::FormDataFolder());
			b->close(p);
			p.fill(0);
			delete b;
			delete wiz;
		} else {
			delete wiz;
			return -1;
		}
	} else if (_firstUse) {
		TDEConfig cfg("tdewalletrc");
		_firstUse = false;
		cfg.setGroup("Wallet");
		cfg.writeEntry("First Use", false);
		cfg.sync();
	}

	int rc = internalOpen(appid, wallet, false, wId, modal);
	return rc;
}

int TDEWalletD::tryOpen(const TQString& wallet, const TQCString& password)
{
    if (isOpen(wallet))
        return 0;

    if (_tryOpenBlocked.isActive()) {
        kdDebug() << "tryOpen is active.." << endl;
        return -1;
    }

    if (!TDEWallet::Backend::exists(wallet))
        return -2;

    TDEWallet::Backend *b = new TDEWallet::Backend(wallet, false /*isPath*/);
    int rc = b->open(TQByteArray().duplicate(password, strlen(password)));
    if (rc == 0) {
        _wallets.insert(rc = generateHandle(), b);
        _passwords[wallet] = password;
        b->ref();
        _tryOpenBlocked.stop();
        TQByteArray data;
        TQDataStream ds(data, IO_WriteOnly);
        ds << wallet;
        emitDCOPSignal("walletOpened(TQString)", data);
    }
    else {
        delete b;
        // make sure that we're not bombed with a dictionary attack
        _tryOpenBlocked.start (30 * 1000, true /*single shot*/);
        if (++_failed > 5) {
            _failed = 0;
            TQTimer::singleShot(0, this, TQ_SLOT(notifyFailures()));
        }

        rc = -1;
    }
    return rc;
}

int TDEWalletD::internalOpen(const TQCString& appid, const TQString& wallet, bool isPath, WId w, bool modal) {
	int rc = -1;
	bool brandNew = false;

	TQCString thisApp;
	if (appid.isEmpty()) {
		thisApp = "TDE System";
	} else {
		thisApp = appid;
	}

	if (implicitDeny(wallet, thisApp)) {
		return -1;
	}

	for (TQIntDictIterator<TDEWallet::Backend> i(_wallets); i.current(); ++i) {
		if (i.current()->walletName() == wallet) {
			rc = i.currentKey();
			break;
		}
	}

	if (rc == -1) {
		if (_wallets.count() > 20) {
			kdDebug() << "Too many wallets open." << endl;
			return -1;
		}

		TDEWallet::Backend *b = new TDEWallet::Backend(wallet, isPath);
		KPasswordDialog *kpd = 0L;
		bool emptyPass = false;
		if ((isPath && TQFile::exists(wallet)) || (!isPath && TDEWallet::Backend::exists(wallet))) {
			int pwless = b->open(TQByteArray());
			if (0 != pwless || !b->isOpen()) {
				if (pwless == 0) {
					// release, start anew
					delete b;
					b = new TDEWallet::Backend(wallet, isPath);
				}
				kpd = new KPasswordDialog(KPasswordDialog::Password, false, 0);
				if (appid.isEmpty()) {
					kpd->setPrompt(i18n("<qt>TDE has requested to open the wallet '<b>%1</b>'. Please enter the password for this wallet below.").arg(TQStyleSheet::escape(wallet)));
				} else {
					kpd->setPrompt(i18n("<qt>The application '<b>%1</b>' has requested to open the wallet '<b>%2</b>'. Please enter the password for this wallet below.").arg(TQStyleSheet::escape(appid)).arg(TQStyleSheet::escape(wallet)));
				}
				brandNew = false;
				kpd->setButtonOK(KGuiItem(i18n("&Open"),"document-open"));
			} else {
				emptyPass = true;
			}
		} else if (wallet == TDEWallet::Wallet::LocalWallet() ||
				wallet == TDEWallet::Wallet::NetworkWallet()) {
			// Auto create these wallets.
			kpd = new KPasswordDialog(KPasswordDialog::NewPassword, false, 0);
			if (appid.isEmpty()) {
				kpd->setPrompt(i18n("TDE has requested to open the wallet. This is used to store sensitive data in a secure fashion. Please enter a password to use with this wallet or click cancel to deny the application's request."));
			} else {
				kpd->setPrompt(i18n("<qt>The application '<b>%1</b>' has requested to open the TDE wallet. This is used to store sensitive data in a secure fashion. Please enter a password to use with this wallet or click cancel to deny the application's request.").arg(TQStyleSheet::escape(appid)));
			}
			brandNew = true;
			kpd->setButtonOK(KGuiItem(i18n("&Open"),"document-open"));
		} else {
			kpd = new KPasswordDialog(KPasswordDialog::NewPassword, false, 0);
			if (appid.length() == 0) {
				kpd->setPrompt(i18n("<qt>TDE has requested to create a new wallet named '<b>%1</b>'. Please choose a password for this wallet, or cancel to deny the application's request.").arg(TQStyleSheet::escape(wallet)));
			} else {
				kpd->setPrompt(i18n("<qt>The application '<b>%1</b>' has requested to create a new wallet named '<b>%2</b>'. Please choose a password for this wallet, or cancel to deny the application's request.").arg(TQStyleSheet::escape(appid)).arg(TQStyleSheet::escape(wallet)));
			}
			brandNew = true;
			kpd->setButtonOK(KGuiItem(i18n("C&reate"),"document-new"));
		}

		if (kpd) {
			kpd->setCaption(i18n("TDE Wallet Service"));
			kpd->setAllowEmptyPasswords(true);
		}

		TQCString p;
		while (!b->isOpen()) {
			assert(kpd); // kpd can't be null if isOpen() is false
			setupDialog( kpd, w, appid, modal );
			if (kpd->exec() == KDialog::Accepted) {
				p = kpd->password().utf8();
				int rc = b->open(TQByteArray().duplicate(p, p.length()));
				if (!b->isOpen()) {
					// For compatibility with TDE << T14.0.6, try ascii() as fallback option in case of errors
					p = kpd->password().ascii();
					rc = b->open(TQByteArray().duplicate(p, p.length()));
					if (!b->isOpen()) {
						kpd->setPrompt(i18n("<qt>Error opening the wallet '<b>%1</b>'. Please try again.<br>(Error code %2: %3)").arg(TQStyleSheet::escape(wallet)).arg(rc).arg(TDEWallet::Backend::openRCToString(rc)));
						kpd->clearPassword();
					}
				}
			} else {
				break;
			}
		}

		if (!emptyPass && (p.isEmpty() || !b->isOpen())) {
			delete b;
			delete kpd;
			return -1;
		}

		if (emptyPass && _openPrompt && !isAuthorizedApp(appid, wallet, w)) {
			delete b;
			delete kpd;
			return -1;
		}

		_wallets.insert(rc = generateHandle(), b);
		if (emptyPass) {
			_passwords[wallet] = "";
		} else {
			_passwords[wallet] = p;
		}
		_handles[appid].append(rc);

		delete kpd; // don't refactor this!!  Argh I hate KPassDlg

		if (brandNew) {
			createFolder(rc, TDEWallet::Wallet::PasswordFolder());
			createFolder(rc, TDEWallet::Wallet::FormDataFolder());
		}

		b->ref();
		if (_closeIdle && _timeouts) {
			_timeouts->addTimer(rc, _idleTime);
		}
		TQByteArray data;
		TQDataStream ds(data, IO_WriteOnly);
		ds << wallet;
		if (brandNew) {
			emitDCOPSignal("walletCreated(TQString)", data);
		}
		emitDCOPSignal("walletOpened(TQString)", data);
		if (_wallets.count() == 1 && _launchManager) {
			TDEApplication::startServiceByDesktopName("tdewalletmanager-tdewalletd");
		}
	} else {
		if (!_handles[appid].contains(rc) && _openPrompt && !isAuthorizedApp(appid, wallet, w)) {
			return -1;
		}
		_handles[appid].append(rc);
		_wallets.find(rc)->ref();
	}

	return rc;
}


bool TDEWalletD::isAuthorizedApp(const TQCString& appid, const TQString& wallet, WId w) {
	int response = 0;

	TQCString thisApp;
	if (appid.isEmpty()) {
		thisApp = "TDE System";
	} else {
		thisApp = appid;
	}

	if (!implicitAllow(wallet, thisApp)) {
		KBetterThanKDialogBase *dialog = new KBetterThanKDialogBase;
		if (appid.isEmpty()) {
			dialog->setLabel(i18n("<qt>TDE has requested access to the open wallet '<b>%1</b>'.").arg(TQStyleSheet::escape(wallet)));
		} else {
			dialog->setLabel(i18n("<qt>The application '<b>%1</b>' has requested access to the open wallet '<b>%2</b>'.").arg(TQStyleSheet::escape(TQString(appid))).arg(TQStyleSheet::escape(wallet)));
		}
		setupDialog( dialog, w, appid, false );
		response = dialog->exec();
		delete dialog;
	}

	if (response == 0 || response == 1) {
		if (response == 1) {
			TDEConfig cfg("tdewalletrc");
			cfg.setGroup("Auto Allow");
			TQStringList apps = cfg.readListEntry(wallet);
			if (!apps.contains(thisApp)) {
				apps += thisApp;
				_implicitAllowMap[wallet] += thisApp;
				cfg.writeEntry(wallet, apps);
				cfg.sync();
			}
		}
	} else if (response == 3) {
		TDEConfig cfg("tdewalletrc");
		cfg.setGroup("Auto Deny");
		TQStringList apps = cfg.readListEntry(wallet);
		if (!apps.contains(thisApp)) {
			apps += thisApp;
			_implicitDenyMap[wallet] += thisApp;
			cfg.writeEntry(wallet, apps);
			cfg.sync();
		}
		return false;
	} else {
		return false;
	}
	return true;
}


int TDEWalletD::deleteWallet(const TQString& wallet) {
	TQString path = TDEGlobal::dirs()->saveLocation("tdewallet") + TQDir::separator() + wallet + ".kwl";

	if (TQFile::exists(path)) {
		close(wallet, true);
		TQFile::remove(path);
		TQByteArray data;
		TQDataStream ds(data, IO_WriteOnly);
		ds << wallet;
		emitDCOPSignal("walletDeleted(TQString)", data);
		return 0;
	}

	return -1;
}


void TDEWalletD::changePassword(const TQString& wallet, uint wId) {
	TQCString appid = friendlyDCOPPeerName();

	TDEWalletTransaction *xact = new TDEWalletTransaction;

	xact->appid = appid;
	xact->client = callingDcopClient();
	xact->wallet = wallet;
	xact->wId = wId;
	xact->tType = TDEWalletTransaction::ChangePassword;

	_transactions.append(xact);

	TQTimer::singleShot(0, this, TQ_SLOT(processTransactions()));
	checkActiveDialog();
}


void TDEWalletD::doTransactionChangePassword(const TQCString& appid, const TQString& wallet, uint wId) {
	TQIntDictIterator<TDEWallet::Backend> it(_wallets);
	TDEWallet::Backend *w = 0L;
	int handle = -1;
	bool reclose = false;

	for (; it.current(); ++it) {
		if (it.current()->walletName() == wallet) {
			break;
		}
	}

	if (!it.current()) {
		handle = doTransactionOpen(appid, wallet, wId,false);
		if (-1 == handle) {
			KMessageBox::sorryWId(wId, i18n("Unable to open wallet. The wallet must be opened in order to change the password."), i18n("TDE Wallet Service"));
			return;
		}

		w = _wallets.find(handle);
		reclose = true;
	} else {
		handle = it.currentKey();
		w = it.current();
	}

	assert(w);

	KPasswordDialog *kpd;
	kpd = new KPasswordDialog(KPasswordDialog::NewPassword, false, 0);
	kpd->setPrompt(i18n("<qt>Please choose a new password for the wallet '<b>%1</b>'.").arg(TQStyleSheet::escape(wallet)));
	kpd->setCaption(i18n("TDE Wallet Service"));
	kpd->setAllowEmptyPasswords(true);
	setupDialog( kpd, wId, appid, false );
	if (kpd->exec() == KDialog::Accepted) {
		TQCString p = kpd->password().utf8();
		if (!p.isEmpty()) {
			_passwords[wallet] = p;
			TQByteArray pa;
			pa.duplicate(p);
			int rc = w->close(pa);
			if (rc < 0) {
				KMessageBox::sorryWId(wId, i18n("Error re-encrypting the wallet. Password was not changed."), i18n("TDE Wallet Service"));
				reclose = true;
			} else {
				rc = w->open(pa);
				if (rc < 0) {
					KMessageBox::sorryWId(wId, i18n("Error reopening the wallet. Data may be lost."), i18n("TDE Wallet Service"));
					reclose = true;
				}
			}
		}
	}

	delete kpd;

	if (reclose) {
		close(handle, true);
	}
}


int TDEWalletD::close(const TQString& wallet, bool force) {
	int handle = -1;
	TDEWallet::Backend *w = 0L;

	for (TQIntDictIterator<TDEWallet::Backend> it(_wallets);
						it.current();
							++it) {
		if (it.current()->walletName() == wallet) {
			handle = it.currentKey();
			w = it.current();
			break;
		}
	}

	return closeWallet(w, handle, force);
}


int TDEWalletD::closeWallet(TDEWallet::Backend *w, int handle, bool force) {
	if (w) {
		const TQString& wallet = w->walletName();
		assert(_passwords.contains(wallet));
		if (w->refCount() == 0 || force) {
			invalidateHandle(handle);
			if (_closeIdle && _timeouts) {
				_timeouts->removeTimer(handle);
			}
			_wallets.remove(handle);
			if (_passwords.contains(wallet)) {
				w->close(TQByteArray().duplicate(_passwords[wallet].data(), _passwords[wallet].length()));
				_passwords[wallet].fill(0);
				_passwords.remove(wallet);
			}
			doCloseSignals(handle, wallet);
			delete w;
			return 0;
		}
		return 1;
	}

	return -1;
}


int TDEWalletD::close(int handle, bool force) {
	TQCString appid = friendlyDCOPPeerName();
	TDEWallet::Backend *w = _wallets.find(handle);
	bool contains = false;

	if (w) { // the handle is valid
		if (_handles.contains(appid)) { // we know this app
			if (_handles[appid].contains(handle)) {
				// the app owns this handle
				_handles[appid].remove(_handles[appid].find(handle));
				contains = true;
				if (_handles[appid].isEmpty()) {
					_handles.remove(appid);
				}
			}
		}

		// watch the side effect of the deref()
		if ((contains && w->deref() == 0 && !_leaveOpen) || force) {
			if (_closeIdle && _timeouts) {
				_timeouts->removeTimer(handle);
			}
			_wallets.remove(handle);
			if (force) {
				invalidateHandle(handle);
			}
			if (_passwords.contains(w->walletName())) {
				w->close(TQByteArray().duplicate(_passwords[w->walletName()].data(), _passwords[w->walletName()].length()));
				_passwords[w->walletName()].fill(0);
				_passwords.remove(w->walletName());
			}
			doCloseSignals(handle, w->walletName());
			delete w;
			return 0;
		}
		return 1; // not closed
	}

	return -1; // not open to begin with, or other error
}


bool TDEWalletD::isOpen(const TQString& wallet) const {
	for (TQIntDictIterator<TDEWallet::Backend> it(_wallets);
						it.current();
							++it) {
		if (it.current()->walletName() == wallet) {
			return true;
		}
	}
	return false;
}


bool TDEWalletD::isOpen(int handle) {
	if (handle == 0) {
		return false;
	}

	TDEWallet::Backend *rc = _wallets.find(handle);

	if (rc == 0 && ++_failed > 5) {
		_failed = 0;
		TQTimer::singleShot(0, this, TQ_SLOT(notifyFailures()));
	} else if (rc != 0) {
		_failed = 0;
	}

	return rc != 0;
}


TQStringList TDEWalletD::wallets() const {
	TQString path = TDEGlobal::dirs()->saveLocation("tdewallet");
	TQDir dir(path, "*.kwl");
	TQStringList rc;

	dir.setFilter(TQDir::Files | TQDir::NoSymLinks);

	const TQFileInfoList *list = dir.entryInfoList();
	TQFileInfoListIterator it(*list);
	TQFileInfo *fi;
	while ((fi = it.current()) != 0L) {
		TQString fn = fi->fileName();
		if (fn.endsWith(".kwl")) {
			fn.truncate(fn.length()-4);
		}
		rc += fn;
		++it;
	}
	return rc;
}


void TDEWalletD::sync(int handle) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		TQByteArray p;
		TQString wallet = b->walletName();
		p.duplicate(_passwords[wallet].data(), _passwords[wallet].length());
		b->sync(p);
		p.fill(0);
	}
}


TQStringList TDEWalletD::folderList(int handle) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		return b->folderList();
	}

	return TQStringList();
}


bool TDEWalletD::hasFolder(int handle, const TQString& f) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		return b->hasFolder(f);
	}

	return false;
}


bool TDEWalletD::removeFolder(int handle, const TQString& f) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		bool rc = b->removeFolder(f);
		// write changes to disk immediately
		TQByteArray p;
		TQString wallet = b->walletName();
		p.duplicate(_passwords[wallet].data(), _passwords[wallet].length());
		b->sync(p);
		p.fill(0);
		TQByteArray data;
		TQDataStream ds(data, IO_WriteOnly);
		ds << b->walletName();
		emitDCOPSignal("folderListUpdated(TQString)", data);
		return rc;
	}

	return false;
}


bool TDEWalletD::createFolder(int handle, const TQString& f) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		bool rc = b->createFolder(f);
		// write changes to disk immediately
		TQByteArray p;
		TQString wallet = b->walletName();
		p.duplicate(_passwords[wallet].data(), _passwords[wallet].length());
		b->sync(p);
		p.fill(0);
		TQByteArray data;
		TQDataStream ds(data, IO_WriteOnly);
		ds << b->walletName();
		emitDCOPSignal("folderListUpdated(TQString)", data);
		return rc;
	}

	return false;
}


TQByteArray TDEWalletD::readMap(int handle, const TQString& folder, const TQString& key) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		TDEWallet::Entry *e = b->readEntry(key);
		if (e && e->type() == TDEWallet::Wallet::Map) {
			return e->map();
		}
	}

	return TQByteArray();
}


TQMap<TQString,TQByteArray> TDEWalletD::readMapList(int handle, const TQString& folder, const TQString& key) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		TQPtrList<TDEWallet::Entry> e = b->readEntryList(key);
		TQMap<TQString, TQByteArray> rc;
		TQPtrListIterator<TDEWallet::Entry> it(e);
		TDEWallet::Entry *entry;
		while ((entry = it.current())) {
			if (entry->type() == TDEWallet::Wallet::Map) {
				rc.insert(entry->key(), entry->map());
			}
			++it;
		}
		return rc;
	}

	return TQMap<TQString, TQByteArray>();
}


TQByteArray TDEWalletD::readEntry(int handle, const TQString& folder, const TQString& key) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		TDEWallet::Entry *e = b->readEntry(key);
		if (e) {
			return e->value();
		}
	}

	return TQByteArray();
}


TQMap<TQString, TQByteArray> TDEWalletD::readEntryList(int handle, const TQString& folder, const TQString& key) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		TQPtrList<TDEWallet::Entry> e = b->readEntryList(key);
		TQMap<TQString, TQByteArray> rc;
		TQPtrListIterator<TDEWallet::Entry> it(e);
		TDEWallet::Entry *entry;
		while ((entry = it.current())) {
			rc.insert(entry->key(), entry->value());
			++it;
		}
		return rc;
	}

	return TQMap<TQString, TQByteArray>();
}


TQStringList TDEWalletD::entryList(int handle, const TQString& folder) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		return b->entryList();
	}

	return TQStringList();
}


TQString TDEWalletD::readPassword(int handle, const TQString& folder, const TQString& key) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		TDEWallet::Entry *e = b->readEntry(key);
		if (e && e->type() == TDEWallet::Wallet::Password) {
			return e->password();
		}
	}

	return TQString::null;
}


TQMap<TQString, TQString> TDEWalletD::readPasswordList(int handle, const TQString& folder, const TQString& key) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		TQPtrList<TDEWallet::Entry> e = b->readEntryList(key);
		TQMap<TQString, TQString> rc;
		TQPtrListIterator<TDEWallet::Entry> it(e);
		TDEWallet::Entry *entry;
		while ((entry = it.current())) {
			if (entry->type() == TDEWallet::Wallet::Password) {
				rc.insert(entry->key(), entry->password());
			}
			++it;
		}
		return rc;
	}

	return TQMap<TQString, TQString>();
}


int TDEWalletD::writeMap(int handle, const TQString& folder, const TQString& key, const TQByteArray& value) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		TDEWallet::Entry e;
		e.setKey(key);
		e.setValue(value);
		e.setType(TDEWallet::Wallet::Map);
		b->writeEntry(&e);
		// write changes to disk immediately
		TQByteArray p;
		TQString wallet = b->walletName();
		p.duplicate(_passwords[wallet].data(), _passwords[wallet].length());
		b->sync(p);
		p.fill(0);
		emitFolderUpdated(b->walletName(), folder);
		return 0;
	}

	return -1;
}


int TDEWalletD::writeEntry(int handle, const TQString& folder, const TQString& key, const TQByteArray& value, int entryType) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		TDEWallet::Entry e;
		e.setKey(key);
		e.setValue(value);
		e.setType(TDEWallet::Wallet::EntryType(entryType));
		b->writeEntry(&e);
		// write changes to disk immediately
		TQByteArray p;
		TQString wallet = b->walletName();
		p.duplicate(_passwords[wallet].data(), _passwords[wallet].length());
		b->sync(p);
		p.fill(0);
		emitFolderUpdated(b->walletName(), folder);
		return 0;
	}

	return -1;
}


int TDEWalletD::writeEntry(int handle, const TQString& folder, const TQString& key, const TQByteArray& value) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		TDEWallet::Entry e;
		e.setKey(key);
		e.setValue(value);
		e.setType(TDEWallet::Wallet::Stream);
		b->writeEntry(&e);
		// write changes to disk immediately
		TQByteArray p;
		TQString wallet = b->walletName();
		p.duplicate(_passwords[wallet].data(), _passwords[wallet].length());
		b->sync(p);
		p.fill(0);
		emitFolderUpdated(b->walletName(), folder);
		return 0;
	}

	return -1;
}


int TDEWalletD::writePassword(int handle, const TQString& folder, const TQString& key, const TQString& value) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		TDEWallet::Entry e;
		e.setKey(key);
		e.setValue(value);
		e.setType(TDEWallet::Wallet::Password);
		b->writeEntry(&e);
		// write changes to disk immediately
		TQByteArray p;
		TQString wallet = b->walletName();
		p.duplicate(_passwords[wallet].data(), _passwords[wallet].length());
		b->sync(p);
		p.fill(0);
		emitFolderUpdated(b->walletName(), folder);
		return 0;
	}

	return -1;
}


int TDEWalletD::entryType(int handle, const TQString& folder, const TQString& key) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		if (!b->hasFolder(folder)) {
			return TDEWallet::Wallet::Unknown;
		}
		b->setFolder(folder);
		if (b->hasEntry(key)) {
			return b->readEntry(key)->type();
		}
	}

	return TDEWallet::Wallet::Unknown;
}


bool TDEWalletD::hasEntry(int handle, const TQString& folder, const TQString& key) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		if (!b->hasFolder(folder)) {
			return false;
		}
		b->setFolder(folder);
		return b->hasEntry(key);
	}

	return false;
}


int TDEWalletD::removeEntry(int handle, const TQString& folder, const TQString& key) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		if (!b->hasFolder(folder)) {
			return 0;
		}
		b->setFolder(folder);
		bool rc = b->removeEntry(key);
		// write changes to disk immediately
		TQByteArray p;
		TQString wallet = b->walletName();
		p.duplicate(_passwords[wallet].data(), _passwords[wallet].length());
		b->sync(p);
		p.fill(0);
		emitFolderUpdated(b->walletName(), folder);
		return rc ? 0 : -3;
	}

	return -1;
}


void TDEWalletD::slotAppUnregistered(const TQCString& app) {
	if (_handles.contains(app)) {
		TQValueList<int> l = _handles[app];
		for (TQValueList<int>::Iterator i = l.begin(); i != l.end(); ++i) {
			_handles[app].remove(*i);
			TDEWallet::Backend *w = _wallets.find(*i);
			if (w && !_leaveOpen && 0 == w->deref()) {
				close(w->walletName(), true);
			}
		}
		_handles.remove(app);
	}
}


void TDEWalletD::invalidateHandle(int handle) {
	for (TQMap<TQCString,TQValueList<int> >::Iterator i = _handles.begin();
							i != _handles.end();
									++i) {
		i.data().remove(handle);
	}
}


TDEWallet::Backend *TDEWalletD::getWallet(const TQCString& appid, int handle) {
	if (handle == 0) {
		return 0L;
	}

	TDEWallet::Backend *w = _wallets.find(handle);

	if (w) { // the handle is valid
		if (_handles.contains(appid)) { // we know this app
			if (_handles[appid].contains(handle)) {
				// the app owns this handle
				_failed = 0;
				if (_closeIdle && _timeouts) {
					_timeouts->resetTimer(handle, _idleTime);
				}
				return w;
			}
		}
	}

	if (++_failed > 5) {
		_failed = 0;
		TQTimer::singleShot(0, this, TQ_SLOT(notifyFailures()));
	}

	return 0L;
}


void TDEWalletD::notifyFailures() {
	if (!_showingFailureNotify) {
		_showingFailureNotify = true;
		KMessageBox::information(0, i18n("There have been repeated failed attempts to gain access to a wallet. An application may be misbehaving."), i18n("TDE Wallet Service"));
		_showingFailureNotify = false;
	}
}


void TDEWalletD::doCloseSignals(int handle, const TQString& wallet) {
	TQByteArray data;
	TQDataStream ds(data, IO_WriteOnly);
	ds << handle;
	emitDCOPSignal("walletClosed(int)", data);

	TQByteArray data2;
	TQDataStream ds2(data2, IO_WriteOnly);
	ds2 << wallet;
	emitDCOPSignal("walletClosed(TQString)", data2);

	if (_wallets.isEmpty()) {
		emitDCOPSignal("allWalletsClosed()", TQByteArray());
	}
}


int TDEWalletD::renameEntry(int handle, const TQString& folder, const TQString& oldName, const TQString& newName) {
	TDEWallet::Backend *b;

	if ((b = getWallet(friendlyDCOPPeerName(), handle))) {
		b->setFolder(folder);
		int rc = b->renameEntry(oldName, newName);
		// write changes to disk immediately
		TQByteArray p;
		TQString wallet = b->walletName();
		p.duplicate(_passwords[wallet].data(), _passwords[wallet].length());
		b->sync(p);
		p.fill(0);
		emitFolderUpdated(b->walletName(), folder);
		return rc;
	}

	return -1;
}


TQStringList TDEWalletD::users(const TQString& wallet) const {
	TQStringList rc;

	for (TQIntDictIterator<TDEWallet::Backend> it(_wallets);
						it.current();
							++it) {
		if (it.current()->walletName() == wallet) {
			for (TQMap<TQCString,TQValueList<int> >::ConstIterator hit = _handles.begin(); hit != _handles.end(); ++hit) {
				if (hit.data().contains(it.currentKey())) {
					rc += hit.key();
				}
			}
			break;
		}
	}

	return rc;
}


bool TDEWalletD::disconnectApplication(const TQString& wallet, const TQCString& application) {
	for (TQIntDictIterator<TDEWallet::Backend> it(_wallets);
						it.current();
							++it) {
		if (it.current()->walletName() == wallet) {
			if (_handles[application].contains(it.currentKey())) {
				_handles[application].remove(it.currentKey());

				if (_handles[application].isEmpty()) {
					_handles.remove(application);
				}

				if (it.current()->deref() == 0) {
					close(it.current()->walletName(), true);
				}

				TQByteArray data;
				TQDataStream ds(data, IO_WriteOnly);
				ds << wallet;
				ds << application;
				emitDCOPSignal("applicationDisconnected(TQString,TQCString)", data);

				return true;
			}
		}
	}

	return false;
}


void TDEWalletD::emitFolderUpdated(const TQString& wallet, const TQString& folder) {
	TQByteArray data;
	TQDataStream ds(data, IO_WriteOnly);
	ds << wallet;
	ds << folder;
	emitDCOPSignal("folderUpdated(TQString,TQString)", data);
}


void TDEWalletD::emitWalletListDirty() {
	emitDCOPSignal("walletListDirty()", TQByteArray());
}


void TDEWalletD::reconfigure() {
	TDEConfig cfg("tdewalletrc");
	cfg.setGroup("Wallet");
	_firstUse = cfg.readBoolEntry("First Use", true);
	_enabled = cfg.readBoolEntry("Enabled", true);
	_launchManager = cfg.readBoolEntry("Launch Manager", true);
	_leaveOpen = cfg.readBoolEntry("Leave Open", false);
	bool idleSave = _closeIdle;
	_closeIdle = cfg.readBoolEntry("Close When Idle", false);
	_openPrompt = cfg.readBoolEntry("Prompt on Open", true);
	int timeSave = _idleTime;
	// in minutes!
	_idleTime = cfg.readNumEntry("Idle Timeout", 10) * 60 * 1000;

	if (cfg.readBoolEntry("Close on Screensaver", false)) {
		connectDCOPSignal("kdesktop", "KScreensaverIface", "KDE_start_screensaver()", "closeAllWallets()", false);
	} else {
		disconnectDCOPSignal("kdesktop", "KScreensaverIface", "KDE_start_screensaver()", "closeAllWallets()");
	}

	// Handle idle changes
	if (_closeIdle) {
		if (_idleTime != timeSave) { // Timer length changed
			TQIntDictIterator<TDEWallet::Backend> it(_wallets);
			for (; it.current(); ++it) {
				_timeouts->resetTimer(it.currentKey(), _idleTime);
			}
		}

		if (!idleSave) { // add timers for all the wallets
			TQIntDictIterator<TDEWallet::Backend> it(_wallets);
			for (; it.current(); ++it) {
				_timeouts->addTimer(it.currentKey(), _idleTime);
			}
		}
	} else {
		_timeouts->clear();
	}

	// Update the implicit allow stuff
	_implicitAllowMap.clear();
	cfg.setGroup("Auto Allow");
	TQStringList entries = cfg.entryMap("Auto Allow").keys();
	for (TQStringList::Iterator i = entries.begin(); i != entries.end(); ++i) {
		_implicitAllowMap[*i] = cfg.readListEntry(*i);
	}

	// Update the implicit allow stuff
	_implicitDenyMap.clear();
	cfg.setGroup("Auto Deny");
	entries = cfg.entryMap("Auto Deny").keys();
	for (TQStringList::Iterator i = entries.begin(); i != entries.end(); ++i) {
		_implicitDenyMap[*i] = cfg.readListEntry(*i);
	}

	// Update if wallet was enabled/disabled
	if (!_enabled) { // close all wallets
		while (!_wallets.isEmpty()) {
			TQIntDictIterator<TDEWallet::Backend> it(_wallets);
			if (!it.current()) { // necessary?
				break;
			}
			closeWallet(it.current(), it.currentKey(), true);
		}
	}
}


bool TDEWalletD::isEnabled() const {
	return _enabled;
}


bool TDEWalletD::folderDoesNotExist(const TQString& wallet, const TQString& folder) {
	if (!wallets().contains(wallet)) {
		return true;
	}

	for (TQIntDictIterator<TDEWallet::Backend> it(_wallets); it.current(); ++it) {
		if (it.current()->walletName() == wallet) {
			return it.current()->folderDoesNotExist(folder);
		}
	}

	TDEWallet::Backend *b = new TDEWallet::Backend(wallet);
	b->open(TQByteArray());
	bool rc = b->folderDoesNotExist(folder);
	delete b;
	return rc;
}


bool TDEWalletD::keyDoesNotExist(const TQString& wallet, const TQString& folder, const TQString& key) {
	if (!wallets().contains(wallet)) {
		return true;
	}

	for (TQIntDictIterator<TDEWallet::Backend> it(_wallets); it.current(); ++it) {
		if (it.current()->walletName() == wallet) {
			return it.current()->entryDoesNotExist(folder, key);
		}
	}

	TDEWallet::Backend *b = new TDEWallet::Backend(wallet);
	b->open(TQByteArray());
	bool rc = b->entryDoesNotExist(folder, key);
	delete b;
	return rc;
}


bool TDEWalletD::implicitAllow(const TQString& wallet, const TQCString& app) {
	return _implicitAllowMap[wallet].contains(TQString::fromLocal8Bit(app));
}


bool TDEWalletD::implicitDeny(const TQString& wallet, const TQCString& app) {
	return _implicitDenyMap[wallet].contains(TQString::fromLocal8Bit(app));
}


TQCString TDEWalletD::friendlyDCOPPeerName() {
	DCOPClient *dc = callingDcopClient();
	if (!dc) {
		return "";
	}
	return dc->senderId().replace(TQRegExp("-[0-9]+$"), "");
}


void TDEWalletD::timedOut(int id) {
	TDEWallet::Backend *w = _wallets.find(id);
	if (w) {
		closeWallet(w, id, true);
	}
}


void TDEWalletD::closeAllWallets() {
	TQIntDict<TDEWallet::Backend> tw = _wallets;

	for (TQIntDictIterator<TDEWallet::Backend> it(tw); it.current(); ++it) {
		closeWallet(it.current(), it.currentKey(), true);
	}

	tw.clear();

	// All of this should be basically noop.  Let's just be safe.
	_wallets.clear();

	for (TQMap<TQString,TQCString>::Iterator it = _passwords.begin();
						it != _passwords.end();
						++it) {
		it.data().fill(0);
	}
	_passwords.clear();
}


TQString TDEWalletD::networkWallet() {
	return TDEWallet::Wallet::NetworkWallet();
}


TQString TDEWalletD::localWallet() {
	return TDEWallet::Wallet::LocalWallet();
}


#include "tdewalletd.moc"
