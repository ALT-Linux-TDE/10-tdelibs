/* This file is part of the KDE project
 *
 * Copyright (C) 2001-2004 George Staikos <staikos@kde.org>
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


#ifndef _TDEWALLETBACKEND_H
#define _TDEWALLETBACKEND_H

#include <kmdcodec.h>

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqmap.h>
#include "tdewalletentry.h"


namespace TDEWallet {

class MD5Digest;

/* @internal
 */
class TDE_EXPORT Backend {
	public:
		Backend(const TQString& name = "kdewallet", bool isPath = false);
		~Backend();

		// Open and unlock the wallet.
		int open(const TQByteArray& password);

		// Close and lock the wallet (saving changes).
		int close(const TQByteArray& password);

		// Close the wallet, losing any changes.
		int close();

		// Write the wallet to disk
		int sync(const TQByteArray& password);

		// Returns true if the current wallet is open.
		bool isOpen() const;

		// Returns the current wallet name.
		const TQString& walletName() const;

		// The list of folders.
		TQStringList folderList() const;

		// Force creation of a folder.
		bool createFolder(const TQString& f);

		// Change the folder.
		void setFolder(const TQString& f) { _folder = f; }

		// Current folder.  If empty, it's the global folder.
		const TQString& folder() const { return _folder; }

		// Does it have this folder?
		bool hasFolder(const TQString& f) const { return _entries.contains(f); }

		// Look up an entry.  Returns null if it doesn't exist.
		Entry *readEntry(const TQString& key);
		
		// Look up a list of entries.  Supports wildcards.
		// You delete the list.
		TQPtrList<Entry> readEntryList(const TQString& key);

		// Store an entry.
		void writeEntry(Entry *e);

		// Does this folder contain this entry?
		bool hasEntry(const TQString& key) const;

		// Returns true if the entry was removed
		bool removeEntry(const TQString& key);

		// Returns true if the folder was removed
		bool removeFolder(const TQString& f);

		// The list of entries in this folder.
		TQStringList entryList() const;

		// Rename an entry in this folder.
		int renameEntry(const TQString& oldName, const TQString& newName);

		int ref() { return ++_ref; }

		int deref() { return --_ref; }

		int refCount() const { return _ref; }

		static bool exists(const TQString& wallet);

		bool folderDoesNotExist(const TQString& folder) const;

		bool entryDoesNotExist(const TQString& folder, const TQString& entry) const;

		static TQString openRCToString(int rc);

	private:
		class BackendPrivate;
		BackendPrivate *d;
		TQString _name;
		TQString _path;
		bool _open;
		TQString _folder;
		int _ref;
		// Map Folder->Entries
		typedef TQMap< TQString, Entry* > EntryMap;
		typedef TQMap< TQString, EntryMap > FolderMap;
		FolderMap _entries;
		typedef TQMap<MD5Digest, TQValueList<MD5Digest> > HashMap;
		HashMap _hashes;
};

/**
 * @internal
 */
class MD5Digest : public TQByteArray {
	public:
		MD5Digest() : TQByteArray(16) {}
		MD5Digest(const KMD5::Digest d) : TQByteArray() { duplicate(reinterpret_cast<const char *>(d), 16); }
		virtual ~MD5Digest() {}

		int operator<(const MD5Digest& r) const {
				int i = 0;
				char x, y;
				for (; i < 16; ++i) {
					x = at(i);
					y = const_cast<MD5Digest&>(r).at(i);
					if (x != y) {
						break;
					}
				}
				if (i < 16 && x < y) {
					return 1;
				}
				return 0;
			}
};

}

#endif

