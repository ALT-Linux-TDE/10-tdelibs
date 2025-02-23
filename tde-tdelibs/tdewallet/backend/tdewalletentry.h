/* This file is part of the KDE project
 *
 * Copyright (C) 2001-2003 George Staikos <staikos@kde.org>
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


#ifndef _TDEWALLETENTRY_H
#define _TDEWALLETENTRY_H

#include <tqstring.h>
#include <tqdatastream.h>

#include "tdewallet.h"

namespace TDEWallet {

/* @internal
 */
class TDE_EXPORT Entry {
	public:
		Entry();
		~Entry();
		
		const TQString& key() const;
		const TQByteArray& value() const;
		TQString password() const;
		const TQByteArray& map() const { return value(); }

		void setValue(const TQByteArray& val);
		void setValue(const TQString& val);
		void setKey(const TQString& key);

		Wallet::EntryType type() const;
		void setType(Wallet::EntryType type);

		void copy(const Entry* x);

	private:
		TQString _key;
		TQByteArray _value;
		Wallet::EntryType _type;
};

}

#endif

