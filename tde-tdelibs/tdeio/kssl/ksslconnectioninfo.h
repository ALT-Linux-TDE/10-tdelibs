/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 George Staikos <staikos@kde.org>
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

#ifndef _KSSLCONNECTIONINFO_H
#define _KSSLCONNECTIONINFO_H

#include <tqstring.h>

#include <tdelibs_export.h>

class KSSL;

/**
 * KDE SSL Connection Information
 *
 * This class contains the information about an SSL connection.  It is
 * generally referenced through KSSL.
 *
 * @author George Staikos <staikos@kde.org>
 * @see KSSL
 * @short KDE SSL Connection Information
 */
class TDEIO_EXPORT KSSLConnectionInfo {
friend class KSSL;
public:
	/**
	 *  Destroy this object.
	 */
	~KSSLConnectionInfo();

	/**
	 *  Get the cipher in use.
	 *  @return the cipher in use
	 */
	const TQString& getCipher() const;

	/**
	 *  Describe the cipher in use.
	 *  @return the cipher description (from OpenSSL)
	 */
	const TQString& getCipherDescription() const;

	/**
	 *  Get the version of the cipher in use.
	 *  @return the version of the cipher
	 */
	const TQString& getCipherVersion() const;

	/**
	 *  Get the number of bits of the cipher that are actually used.
	 *  @return the number of bits in use
	 */
	int getCipherUsedBits() const;

	/**
	 *  Get bit-size of the cipher
	 *  @return the number of bits
	 */
	int getCipherBits() const;

protected:
	KSSLConnectionInfo();
	void clean();

	// These are here so KSSL can access them directly
	// It's just as easy as making accessors - they're friends afterall!
	int m_iCipherUsedBits, m_iCipherBits;
	TQString m_cipherName;
	TQString m_cipherDescription;
	TQString m_cipherVersion;

private:
	class KSSLConnectionInfoPrivate;
	KSSLConnectionInfoPrivate *d;
};

#endif

