/* This file is part of the KDE project
 *
 * Copyright (C) 2002-2004 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "tdewallettypes.h"
#include "tdewallet.h"
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdeversion.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <tqpopupmenu.h>
#include <tqapplication.h>

#include <assert.h>

using namespace TDEWallet;


const TQString Wallet::LocalWallet() {
	TDEConfig cfg("tdewalletrc", true);
	cfg.setGroup("Wallet");
	if (!cfg.readBoolEntry("Use One Wallet", true)) {
		TQString tmp = cfg.readEntry("Local Wallet", "localwallet");
		if (tmp.isEmpty()) {
			return "localwallet";
		}
		return tmp;
	}

	TQString tmp = cfg.readEntry("Default Wallet", "kdewallet");
	if (tmp.isEmpty()) {
		return "kdewallet";
	}
	return tmp;
}

const TQString Wallet::NetworkWallet() {
	TDEConfig cfg("tdewalletrc", true);
	cfg.setGroup("Wallet");

	TQString tmp = cfg.readEntry("Default Wallet", "kdewallet");
	if (tmp.isEmpty()) {
		return "kdewallet";
	}
	return tmp;
}

const TQString Wallet::PasswordFolder() {
	return "Passwords";
}

const TQString Wallet::FormDataFolder() {
	return "Form Data";
}



Wallet::Wallet(int handle, const TQString& name)
: TQObject(0L), DCOPObject(), d(0L), _name(name), _handle(handle) {

	_dcopRef = new DCOPRef("kded", "tdewalletd");

	_dcopRef->dcopClient()->setNotifications(true);
	connect(_dcopRef->dcopClient(),
			TQ_SIGNAL(applicationRemoved(const TQCString&)),
			this,
			TQ_SLOT(slotAppUnregistered(const TQCString&)));

	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletClosed(int)", "slotWalletClosed(int)", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "folderListUpdated(TQString)", "slotFolderListUpdated(TQString)", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "folderUpdated(TQString, TQString)", "slotFolderUpdated(TQString, TQString)", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "applicationDisconnected(TQString, TQCString)", "slotApplicationDisconnected(TQString, TQCString)", false);

	// Verify that the wallet is still open
	if (_handle != -1) {
		DCOPReply r = _dcopRef->call("isOpen", _handle);
		if (r.isValid()) {
			bool rc = false;
			r.get(rc);
			if (!rc) {
				_handle = -1;
				_name = TQString::null;
			}
		}
	}
}


Wallet::~Wallet() {
	if (_handle != -1) {
		_dcopRef->call("close", _handle, false);
		_handle = -1;
		_folder = TQString::null;
		_name = TQString::null;
	}

	delete _dcopRef;
	_dcopRef = 0L;
}


TQStringList Wallet::walletList() {
	DCOPReply r = DCOPRef("kded", "tdewalletd").call("wallets");
	TQStringList rc;
	if (r.isValid()) {
		r.get(rc);
	}
	return rc;
}


void Wallet::changePassword(const TQString& name, WId w) {
	DCOPRef("kded", "tdewalletd").send("changePassword", name, uint(w));
}


bool Wallet::isEnabled() {
	DCOPReply r = DCOPRef("kded", "tdewalletd").call("isEnabled");
	bool rc = false;
	if (r.isValid()) {
		r.get(rc);
	}
	return rc;
}


bool Wallet::isOpen(const TQString& name) {
	DCOPReply r = DCOPRef("kded", "tdewalletd").call("isOpen", name);
	bool rc = false;
	if (r.isValid()) {
		r.get(rc);
	}
	return rc;
}


int Wallet::closeWallet(const TQString& name, bool force) {
	DCOPReply r = DCOPRef("kded", "tdewalletd").call("close", name, force);
	int rc = -1;
	if (r.isValid()) {
		r.get(rc);
	}
	return rc;
}


int Wallet::deleteWallet(const TQString& name) {
	DCOPReply r = DCOPRef("kded", "tdewalletd").call("deleteWallet", name);
	int rc = -1;
	if (r.isValid()) {
		r.get(rc);
	}
	return rc;
}


Wallet *Wallet::openWallet(const TQString& name, WId w, OpenType ot) {
	if (ot == Asynchronous) {
		Wallet *wallet = new Wallet(-1, name);
		DCOPRef("kded", "tdewalletd").send("openAsynchronous", name, wallet->objId(), uint(w));
		return wallet;
	}

        // avoid deadlock if the app has some popup open (#65978/#71048)
        while( TQWidget* widget = tqApp->activePopupWidget())
            widget->close();

	bool isPath = ot == Path;
	DCOPReply r;

	if (isPath) {
		r = DCOPRef("kded", "tdewalletd").call("openPath", name, uint(w));
	} else {
		r = DCOPRef("kded", "tdewalletd").call("open", name, uint(w));
	}

	if (r.isValid()) {
		int drc = -1;
		r.get(drc);
		if (drc != -1) {
			return new Wallet(drc, name);
		}
	}

	return 0;
}


bool Wallet::disconnectApplication(const TQString& wallet, const TQCString& app) {
	DCOPReply r = DCOPRef("kded", "tdewalletd").call("disconnectApplication", wallet, app);
	bool rc = false;
	if (r.isValid()) {
		r.get(rc);
	}
	return rc;
}


TQStringList Wallet::users(const TQString& name) {
	DCOPReply r = DCOPRef("kded", "tdewalletd").call("users", name);
	TQStringList drc;
	if (r.isValid()) {
		r.get(drc);
	}
	return drc;
}


int Wallet::sync() {
	if (_handle == -1) {
		return -1;
	}

	_dcopRef->call("sync", _handle);
	return 0;
}


int Wallet::lockWallet() {
	if (_handle == -1) {
		return -1;
	}

	DCOPReply r = _dcopRef->call("close", _handle, true);
	_handle = -1;
	_folder = TQString::null;
	_name = TQString::null;
	if (r.isValid()) {
		int drc = -1;
		r.get(drc);
		return drc;
	}
	return -1;
}


const TQString& Wallet::walletName() const {
	return _name;
}


bool Wallet::isOpen() const {
	return _handle != -1;
}


void Wallet::requestChangePassword(WId w) {
	if (_handle == -1) {
		return;
	}

	_dcopRef->send("changePassword", _name, uint(w));
}


void Wallet::slotWalletClosed(int handle) {
	if (_handle == handle) {
		_handle = -1;
		_folder = TQString::null;
		_name = TQString::null;
		emit walletClosed();
	}
}


TQStringList Wallet::folderList() {
	TQStringList rc;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("folderList", _handle);
	if (r.isValid()) {
		r.get(rc);
	}

	return rc;
}


TQStringList Wallet::entryList() {
	TQStringList rc;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("entryList", _handle, _folder);
	if (r.isValid()) {
		r.get(rc);
	}

	return rc;
}


bool Wallet::hasFolder(const TQString& f) {
	bool rc = false;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("hasFolder", _handle, f);
	if (r.isValid()) {
		r.get(rc);
	}

	return rc;
}


bool Wallet::createFolder(const TQString& f) {
	bool rc = true;

	if (_handle == -1) {
		return false;
	}

	if (!hasFolder(f)) {
		DCOPReply r = _dcopRef->call("createFolder", _handle, f);
		if (r.isValid()) {
			r.get(rc);
		}
	}

	return rc;
}


bool Wallet::setFolder(const TQString& f) {
	bool rc = false;

	if (_handle == -1) {
		return rc;
	}

	// Don't do this - the folder could have disappeared?
#if 0
	if (f == _folder) {
		return true;
	}
#endif

	if (hasFolder(f)) {
		_folder = f;
		rc = true;
	}

	return rc;
}


bool Wallet::removeFolder(const TQString& f) {
	bool rc = false;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("removeFolder", _handle, f);
	if (r.isValid()) {
		r.get(rc);
	}

	if (_folder == f) {
		setFolder(TQString::null);
	}

	return rc;
}


const TQString& Wallet::currentFolder() const {
	return _folder;
}


int Wallet::readEntry(const TQString& key, TQByteArray& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("readEntry", _handle, _folder, key);
	if (r.isValid()) {
		r.get(value);
		rc = 0;
	}

	return rc;
}


int Wallet::readEntryList(const TQString& key, TQMap<TQString, TQByteArray>& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("readEntryList", _handle, _folder, key);
	if (r.isValid()) {
		r.get(value);
		rc = 0;
	}

	return rc;
}


int Wallet::renameEntry(const TQString& oldName, const TQString& newName) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("renameEntry", _handle, _folder, oldName, newName);
	if (r.isValid()) {
		r.get(rc);
	}

	return rc;
}


int Wallet::readMap(const TQString& key, TQMap<TQString,TQString>& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("readMap", _handle, _folder, key);
	if (r.isValid()) {
		TQByteArray v;
		r.get(v);
		if (!v.isEmpty()) {
			TQDataStream ds(v, IO_ReadOnly);
			ds >> value;
		}
		rc = 0;
	}

	return rc;
}


int Wallet::readMapList(const TQString& key, TQMap<TQString, TQMap<TQString, TQString> >& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("readMapList", _handle, _folder, key);
	if (r.isValid()) {
		TQMap<TQString,TQByteArray> unparsed;
		r.get(unparsed);
		for (TQMap<TQString,TQByteArray>::ConstIterator i = unparsed.begin(); i != unparsed.end(); ++i) {
			if (!i.data().isEmpty()) {
				TQDataStream ds(i.data(), IO_ReadOnly);
				TQMap<TQString,TQString> v;
				ds >> v;
				value.insert(i.key(), v);
			}
		}
		rc = 0;
	}

	return rc;
}


int Wallet::readPassword(const TQString& key, TQString& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("readPassword", _handle, _folder, key);
	if (r.isValid()) {
		r.get(value);
		rc = 0;
	}

	return rc;
}


int Wallet::readPasswordList(const TQString& key, TQMap<TQString, TQString>& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("readPasswordList", _handle, _folder, key);
	if (r.isValid()) {
		r.get(value);
		rc = 0;
	}

	return rc;
}


int Wallet::writeEntry(const TQString& key, const TQByteArray& value, EntryType entryType) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("writeEntry", _handle, _folder, key, value, int(entryType));
	if (r.isValid()) {
		r.get(rc);
	}

	return rc;
}


int Wallet::writeEntry(const TQString& key, const TQByteArray& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("writeEntry", _handle, _folder, key, value);
	if (r.isValid()) {
		r.get(rc);
	}

	return rc;
}


int Wallet::writeMap(const TQString& key, const TQMap<TQString,TQString>& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	TQByteArray a;
	TQDataStream ds(a, IO_WriteOnly);
	ds << value;
	DCOPReply r = _dcopRef->call("writeMap", _handle, _folder, key, a);
	if (r.isValid()) {
		r.get(rc);
	}

	return rc;
}


int Wallet::writePassword(const TQString& key, const TQString& value) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("writePassword", _handle, _folder, key, value);
	if (r.isValid()) {
		r.get(rc);
	}

	return rc;
}


bool Wallet::hasEntry(const TQString& key) {
	bool rc = false;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("hasEntry", _handle, _folder, key);
	if (r.isValid()) {
		r.get(rc);
	}

	return rc;
}


int Wallet::removeEntry(const TQString& key) {
	int rc = -1;

	if (_handle == -1) {
		return rc;
	}

	DCOPReply r = _dcopRef->call("removeEntry", _handle, _folder, key);
	if (r.isValid()) {
		r.get(rc);
	}

	return rc;
}


Wallet::EntryType Wallet::entryType(const TQString& key) {
	int rc = 0;

	if (_handle == -1) {
		return Wallet::Unknown;
	}

	DCOPReply r = _dcopRef->call("entryType", _handle, _folder, key);
	if (r.isValid()) {
		r.get(rc);
	}

	return static_cast<EntryType>(rc);
}


void Wallet::slotAppUnregistered(const TQCString& app) {
	if (_handle >= 0 && app == "kded") {
		slotWalletClosed(_handle);
	}
}


void Wallet::slotFolderUpdated(const TQString& wallet, const TQString& folder) {
	if (_name == wallet) {
		emit folderUpdated(folder);
	}
}


void Wallet::slotFolderListUpdated(const TQString& wallet) {
	if (_name == wallet) {
		emit folderListUpdated();
	}
}


void Wallet::slotApplicationDisconnected(const TQString& wallet, const TQCString& application) {
	if (_handle >= 0
			&& _name == wallet
			&& application == _dcopRef->dcopClient()->appId()) {
		slotWalletClosed(_handle);
	}
}


void Wallet::walletOpenResult(int id) {
	if (_handle != -1) {
		// This is BAD.
		return;
	}

	if (id > 0) {
		_handle = id;
		emit walletOpened(true);
	} else if (id < 0) {
		emit walletOpened(false);
	} // id == 0 => wait
}


bool Wallet::folderDoesNotExist(const TQString& wallet, const TQString& folder) {
DCOPReply r = DCOPRef("kded", "tdewalletd").call("folderDoesNotExist", wallet, folder);
bool rc = true;
	if (r.isValid()) {
		r.get(rc);
	}
return rc;
}


bool Wallet::keyDoesNotExist(const TQString& wallet, const TQString& folder, const TQString& key) {
DCOPReply r = DCOPRef("kded", "tdewalletd").call("keyDoesNotExist", wallet, folder, key);
bool rc = true;
	if (r.isValid()) {
		r.get(rc);
	}
return rc;
}


void Wallet::virtual_hook(int, void*) {
	//BASE::virtual_hook( id, data );
}

#include "tdewallet.moc"
