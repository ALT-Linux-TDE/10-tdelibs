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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tqregexp.h>

#include "ksslpeerinfo.h"
#include <kdebug.h>

#include <ksockaddr.h>
#include <kextsock.h>
#include <netsupp.h>
#ifndef TQ_WS_WIN //TODO kresolver not ported
#include "kresolver.h"
#endif

#include "ksslx509map.h"

class KSSLPeerInfoPrivate {
public:
	KSSLPeerInfoPrivate() {}
	~KSSLPeerInfoPrivate() { }
	TQString peerHost;
};



KSSLPeerInfo::KSSLPeerInfo() {
	d = new KSSLPeerInfoPrivate;
}

KSSLPeerInfo::~KSSLPeerInfo() {
	delete d;
}

KSSLCertificate& KSSLPeerInfo::getPeerCertificate() {
	return m_cert;
}

void KSSLPeerInfo::setPeerHost(TQString realHost) {
	d->peerHost = realHost.stripWhiteSpace();
	while(d->peerHost.endsWith("."))
		d->peerHost.truncate(d->peerHost.length()-1);

#ifdef TQ_WS_WIN //TODO kresolver not ported
	d->peerHost = d->peerHost.lower();
#else	
	d->peerHost = TQString::fromLatin1(KNetwork::KResolver::domainToAscii(d->peerHost));
#endif	
}

bool KSSLPeerInfo::certMatchesAddress() {
#ifdef KSSL_HAVE_SSL
	KSSLX509Map certinfo(m_cert.getSubject());
	TQStringList cns = TQStringList::split(TQRegExp("[ \n\r]"), certinfo.getValue("CN"));
	cns += m_cert.subjAltNames();

	for (TQStringList::Iterator cn = cns.begin(); cn != cns.end(); ++cn) {
		if (cnMatchesAddress((*cn).stripWhiteSpace().lower()))
			return true;
	}

#endif

	return false;
}


bool KSSLPeerInfo::cnMatchesAddress(TQString cn) {
#ifdef KSSL_HAVE_SSL
	TQRegExp rx;

	kdDebug(7029) << "Matching CN=[" << cn << "] to ["
		      << d->peerHost << "]" << endl;

	// Check for invalid characters
	if (TQRegExp("[^a-zA-Z0-9\\.\\*\\-]").search(cn) >= 0) {
		kdDebug(7029) << "CN contains invalid characters!  Failing." << endl;
		return false;
	}

	// Domains can legally end with '.'s.  We don't need them though.
	while(cn.endsWith("."))
		cn.truncate(cn.length()-1);

	// Do not let empty CN's get by!!
	if (cn.isEmpty())
		return false;

	// Check for IPv4 address
	rx.setPattern("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
	if (rx.exactMatch(d->peerHost))
		return d->peerHost == cn;

	// Check for IPv6 address here...
	rx.setPattern("^\\[.*\\]$");
	if (rx.exactMatch(d->peerHost))
		return d->peerHost == cn;

	if (cn.contains('*')) {
		// First make sure that there are at least two valid parts
		// after the wildcard (*).
		TQStringList parts = TQStringList::split('.', cn, false);

		while (parts.count() > 2)
			parts.remove(parts.begin());

		if (parts.count() != 2) {
			return false;  // we don't allow *.root - that's bad
		}

		if (parts[0].contains('*') || parts[1].contains('*')) {
			return false;
		}

		// RFC2818 says that *.example.com should match against
		// foo.example.com but not bar.foo.example.com
		// (ie. they must have the same number of parts)
		if (TQRegExp(cn, false, true).exactMatch(d->peerHost) &&
		    TQStringList::split('.', cn, false).count() ==
		    TQStringList::split('.', d->peerHost, false).count())
			return true;

		// *.example.com must match example.com also.  Sigh..
		if (cn.startsWith("*.")) {
			TQString chopped = cn.mid(2);
			if (chopped == d->peerHost) {
				return true;
			}
		}
		return false;
	}

	// We must have an exact match in this case (insensitive though)
	// (note we already did .lower())
	if (cn == d->peerHost)
		return true;
#endif
	return false;
}


void KSSLPeerInfo::reset() {
	d->peerHost = TQString::null;
}


const TQString& KSSLPeerInfo::peerHost() const {
	return d->peerHost;
}

