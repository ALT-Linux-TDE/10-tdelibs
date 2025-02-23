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
#ifndef _TDEWALLETD_H_
#define _TDEWALLETD_H_

#include <kded/kdedmodule.h>
#include <tqintdict.h>
#include <tqstring.h>
#include <tqwidget.h>
#include <tqtimer.h>
#include <tqguardedptr.h>
#include "tdewalletbackend.h"

#include <time.h>
#include <stdlib.h>

class KDirWatch;
class KTimeout;

// @Private
class TDEWalletTransaction;

class TDEWalletD : public KDEDModule {
	TQ_OBJECT
	K_DCOP
	public:
		TDEWalletD(const TQCString &name);
		virtual ~TDEWalletD();

	k_dcop:
		// Is the wallet enabled?  If not, all open() calls fail.
		virtual bool isEnabled() const;

		// Open and unlock the wallet
		virtual int open(const TQString& wallet, uint wId);
		// Open and unlock the wallet
		virtual int tryOpen(const TQString& wallet, const TQCString& passwd);
		// Open and unlock the wallet with this path

		virtual int openPath(const TQString& path, uint wId);

		// Asynchronous open - must give the object to return the handle
		// to.
		virtual void openAsynchronous(const TQString& wallet, const TQCString& returnObject, uint wId);

		// Close and lock the wallet
		// If force = true, will close it for all users.  Behave.  This
		// can break applications, and is generally intended for use by
		// the wallet manager app only.
		virtual int close(const TQString& wallet, bool force);
		virtual int close(int handle, bool force);

		// Save to disk but leave open
		virtual ASYNC sync(int handle);

		// Physically deletes the wallet from disk.
		virtual int deleteWallet(const TQString& wallet);

		// Returns true if the wallet is open
		virtual bool isOpen(const TQString& wallet) const;
		virtual bool isOpen(int handle);

		// List the users of this wallet
		virtual TQStringList users(const TQString& wallet) const;

		// Change the password of this wallet
		virtual void changePassword(const TQString& wallet, uint wId);

		// A list of all wallets
		virtual TQStringList wallets() const;

		// A list of all folders in this wallet
		virtual TQStringList folderList(int handle);

		// Does this wallet have this folder?
		virtual bool hasFolder(int handle, const TQString& folder);

		// Create this folder
		virtual bool createFolder(int handle, const TQString& folder);

		// Remove this folder
		virtual bool removeFolder(int handle, const TQString& folder);

		// List of entries in this folder
		virtual TQStringList entryList(int handle, const TQString& folder);

		// Read an entry.  If the entry does not exist, it just
		// returns an empty result.  It is your responsibility to check
		// hasEntry() first.
		virtual TQByteArray readEntry(int handle, const TQString& folder, const TQString& key);
		virtual TQByteArray readMap(int handle, const TQString& folder, const TQString& key);
		virtual TQString readPassword(int handle, const TQString& folder, const TQString& key);
		virtual TQMap<TQString, TQByteArray> readEntryList(int handle, const TQString& folder, const TQString& key);
		virtual TQMap<TQString, TQByteArray> readMapList(int handle, const TQString& folder, const TQString& key);
		virtual TQMap<TQString, TQString> readPasswordList(int handle, const TQString& folder, const TQString& key);

		// Rename an entry.  rc=0 on success.
		virtual int renameEntry(int handle, const TQString& folder, const TQString& oldName, const TQString& newName);

		// Write an entry.  rc=0 on success.
		virtual int writeEntry(int handle, const TQString& folder, const TQString& key, const TQByteArray& value, int entryType);
		virtual int writeEntry(int handle, const TQString& folder, const TQString& key, const TQByteArray& value);
		virtual int writeMap(int handle, const TQString& folder, const TQString& key, const TQByteArray& value);
		virtual int writePassword(int handle, const TQString& folder, const TQString& key, const TQString& value);

		// Does the entry exist?
		virtual bool hasEntry(int handle, const TQString& folder, const TQString& key);

		// What type is the entry?
		virtual int entryType(int handle, const TQString& folder, const TQString& key);

		// Remove an entry.  rc=0 on success.
		virtual int removeEntry(int handle, const TQString& folder, const TQString& key);

		// Disconnect an app from a wallet
		virtual bool disconnectApplication(const TQString& wallet, const TQCString& application);

		virtual void reconfigure();

		// Determine
		virtual bool folderDoesNotExist(const TQString& wallet, const TQString& folder);
		virtual bool keyDoesNotExist(const TQString& wallet, const TQString& folder, const TQString& key);

		virtual void closeAllWallets();

		virtual TQString networkWallet();

		virtual TQString localWallet();

	private slots:
		void slotAppUnregistered(const TQCString& app);
		void emitWalletListDirty();
		void timedOut(int);
		void notifyFailures();
		void processTransactions();

	private:
		int internalOpen(const TQCString& appid, const TQString& wallet, bool isPath = false, WId w = 0, bool modal = false);
		bool isAuthorizedApp(const TQCString& appid, const TQString& wallet, WId w);
		// This also validates the handle.  May return NULL.
		TDEWallet::Backend* getWallet(const TQCString& appid, int handle);
		// Generate a new unique handle.
		int generateHandle();
		// Invalidate a handle (remove it from the TQMap)
		void invalidateHandle(int handle);
		// Emit signals about closing wallets
		void doCloseSignals(int,const TQString&);
		void emitFolderUpdated(const TQString&, const TQString&);
		// Internal - close this wallet.
		int closeWallet(TDEWallet::Backend *w, int handle, bool force);
		// Implicitly allow access for this application
		bool implicitAllow(const TQString& wallet, const TQCString& app);
		bool implicitDeny(const TQString& wallet, const TQCString& app);
		TQCString friendlyDCOPPeerName();

		void doTransactionChangePassword(const TQCString& appid, const TQString& wallet, uint wId);
		int doTransactionOpen(const TQCString& appid, const TQString& wallet, uint wId, bool modal);

		void setupDialog( TQWidget* dialog, WId wId, const TQCString& appid, bool modal );
		void checkActiveDialog();

		TQIntDict<TDEWallet::Backend> _wallets;
		TQMap<TQCString,TQValueList<int> > _handles;
		TQMap<TQString,TQCString> _passwords;
		KDirWatch *_dw;
		int _failed;

		bool _leaveOpen, _closeIdle, _launchManager, _enabled;
	       	bool _openPrompt, _firstUse, _showingFailureNotify;
		int _idleTime;
		TQMap<TQString,TQStringList> _implicitAllowMap, _implicitDenyMap;
		KTimeout *_timeouts;
		TQTimer _tryOpenBlocked;

		TQPtrList<TDEWalletTransaction> _transactions;
		TQGuardedPtr< TQWidget > activeDialog;
};


#endif
