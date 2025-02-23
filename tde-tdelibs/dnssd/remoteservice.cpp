/* This file is part of the KDE project
 *
 * Copyright (C) 2004, 2005 Jakub Stachowski <qbast@go2.pl>
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

#include <config.h>

#include <tqeventloop.h>
#include <tqapplication.h>
#include <kurl.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <netinet/in.h>
#ifdef HAVE_DNSSD
#include <avahi-client/client.h>
#include <avahi-common/strlst.h>
#ifdef AVAHI_API_0_6
#include <avahi-client/lookup.h>
#endif
#endif
#include "remoteservice.h"
#include "responder.h"
#include "sdevent.h"

namespace DNSSD
{
#ifdef HAVE_DNSSD
#ifdef AVAHI_API_0_6
void resolve_callback(AvahiServiceResolver*, AvahiIfIndex, AvahiProtocol proto, AvahiResolverEvent e,
    const char* name, const char* type, const char* domain, const char* hostname, const AvahiAddress* a,
    uint16_t port, AvahiStringList* txt, AvahiLookupResultFlags, void* context);
#else
void resolve_callback(AvahiServiceResolver*, AvahiIfIndex, AvahiProtocol proto, AvahiResolverEvent e,
    const char* name, const char* type, const char* domain, const char* hostname, const AvahiAddress* a,
    uint16_t port, AvahiStringList* txt, void* context);
#endif
#endif

class RemoteServicePrivate : public Responder
{
public:
	RemoteServicePrivate() :  m_resolved(false), m_running(false)
#ifdef HAVE_DNSSD
	, m_resolver(0)
#endif
	{}
	bool m_resolved;
	bool m_running;
#ifdef HAVE_DNSSD
	AvahiServiceResolver* m_resolver;
	void stop() {
	    m_running = false;
	    if (m_resolver) avahi_service_resolver_free(m_resolver);
	    m_resolver=0;
	}
#endif
};

RemoteService::RemoteService(const TQString& label)
{
	decode(label);
	d =  new RemoteServicePrivate();
}
RemoteService::RemoteService(const TQString& name,const TQString& type,const TQString& domain)
		: ServiceBase(name, type, domain)
{
	d = new RemoteServicePrivate();
}

RemoteService::RemoteService(const KURL& url)
{
	d = new RemoteServicePrivate();
	if (!url.isValid()) return;
	if (url.protocol()!="invitation") return;
	if (!url.hasPath()) return;
	m_hostName = url.host();
	m_port = url.port();
	m_type = url.path().section('/',1,1);
	m_serviceName = url.path().section('/',2);
	m_textData = url.queryItems();
	d->m_resolved=true;
}

RemoteService::~RemoteService()
{
#ifdef HAVE_DNSSD
	if (d->m_resolver) avahi_service_resolver_free(d->m_resolver);
#endif
	delete d;
}

bool RemoteService::resolve()
{
	resolveAsync();
	while (d->m_running && !d->m_resolved) Responder::self().process();
#ifdef HAVE_DNSSD
	d->stop();
#endif
	return d->m_resolved;
}

void RemoteService::resolveAsync()
{
	if (d->m_running) return;
	d->m_resolved = false;
	// FIXME: first protocol should be set?
#ifdef HAVE_DNSSD
#ifdef AVAHI_API_0_6
	d->m_resolver = avahi_service_resolver_new(Responder::self().client(),AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
	    m_serviceName.utf8(), m_type.ascii(), domainToDNS(m_domain), AVAHI_PROTO_UNSPEC, AVAHI_LOOKUP_NO_ADDRESS,
	    resolve_callback, this);
#else
	d->m_resolver = avahi_service_resolver_new(Responder::self().client(),AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
	    m_serviceName.utf8(), m_type.ascii(), m_domain.utf8(), AVAHI_PROTO_UNSPEC, resolve_callback, this);
#endif
	if (d->m_resolver) d->m_running=true;
	    else  emit resolved(false);
#endif
}

bool RemoteService::isResolved() const
{
	return d->m_resolved;
}

void RemoteService::customEvent(TQCustomEvent* event)
{
	if (event->type() == TQEvent::User+SD_ERROR) {
#ifdef HAVE_DNSSD
		d->stop();
#endif
		d->m_resolved=false;
		emit resolved(false);
	}
	if (event->type() == TQEvent::User+SD_RESOLVE) {
		ResolveEvent* rev = static_cast<ResolveEvent*>(event);
		m_hostName = rev->m_hostname;
		m_port = rev->m_port;
		m_textData = rev->m_txtdata;
		d->m_resolved = true;
		emit resolved(true);
	}
}

void RemoteService::virtual_hook(int, void*)
{
	// BASE::virtual_hook(int, void*);
}

TQDataStream & operator<< (TQDataStream & s, const RemoteService & a)
{
	s << (static_cast<ServiceBase>(a));
	TQ_INT8 resolved = a.d->m_resolved ? 1:0;
	s << resolved;
	return s;
}

TQDataStream & operator>> (TQDataStream & s, RemoteService & a)
{
	// stop any possible resolve going on
#ifdef HAVE_DNSSD
	a.d->stop();
#endif
	TQ_INT8 resolved;
	operator>>(s,(static_cast<ServiceBase&>(a)));
	s >> resolved;
	a.d->m_resolved = (resolved == 1);
	return s;
}

#ifdef HAVE_DNSSD
#ifdef AVAHI_API_0_6
void resolve_callback(AvahiServiceResolver*, AvahiIfIndex, AvahiProtocol, AvahiResolverEvent e,
    const char*, const char*, const char*, const char* hostname, const AvahiAddress*,
    uint16_t port, AvahiStringList* txt, AvahiLookupResultFlags, void* context)
#else
void resolve_callback(AvahiServiceResolver*, AvahiIfIndex, AvahiProtocol, AvahiResolverEvent e,
    const char*, const char*, const char*, const char* hostname, const AvahiAddress*,
    uint16_t port, AvahiStringList* txt, void* context)
#endif
{
	TQObject *obj = reinterpret_cast<TQObject*>(context);
	if (e != AVAHI_RESOLVER_FOUND) {
		ErrorEvent err;
		TQApplication::sendEvent(obj, &err);
		return;
	}
	TQMap<TQString,TQString> map;
	while (txt) {
	    char *key, *value;
	    size_t size;
	    if (avahi_string_list_get_pair(txt,&key,&value,&size)) break;
	    map[TQString::fromUtf8(key)]=(value) ? TQString::fromUtf8(value) : TQString::null;
	    txt = txt->next;
	}
	ResolveEvent rev(DNSToDomain(hostname),port,map);
	TQApplication::sendEvent(obj, &rev);
}
#endif


}

#include "remoteservice.moc"
