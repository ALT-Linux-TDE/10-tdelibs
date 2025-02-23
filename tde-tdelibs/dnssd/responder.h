/* This file is part of the KDE project
 *
 * Copyright (C) 2004 Jakub Stachowski <qbast@go2.pl>
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

#ifndef DNSSDRESPONDER_H
#define DNSSDRESPONDER_H

#include <tqobject.h>
#include <tqsocketnotifier.h>
#include <tqsignal.h>
#include <config.h>
#ifdef HAVE_DNSSD
#include <avahi-client/client.h>
#else
#define DNSServiceRef void*
#define AvahiClientState void*
#endif

namespace DNSSD
{

/**
This class should not be used directly.

@author Jakub Stachowski
@short Internal class wrapping avahi client
 */
class Responder : public TQObject
{
	TQ_OBJECT

public:
	Responder();

	~Responder();

	static Responder& self();
#ifdef HAVE_DNSSD
	AvahiClientState state() const;
	AvahiClient* client() const { return m_client; }
#endif
	void process();
signals:
	void stateChanged(AvahiClientState);
private:
#ifdef HAVE_DNSSD
	AvahiClient* m_client;
#endif
	static Responder* m_self;
#ifdef HAVE_DNSSD
	friend void client_callback(AvahiClient*, AvahiClientState, void*);
#endif

};

/* Utils functions */

bool domainIsLocal(const TQString& domain);
// Encodes domain name using utf8() or IDN
TQCString domainToDNS(const TQString &domain);
TQString DNSToDomain(const char* domain);


}

#endif
