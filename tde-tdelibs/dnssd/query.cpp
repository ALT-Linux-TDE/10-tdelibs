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

#include "query.h"
#include "responder.h"
#include "remoteservice.h"
#include "sdevent.h"
#include <tqdatetime.h>
#include <tqapplication.h>
#include <tqtimer.h>

#ifdef HAVE_DNSSD
#include <avahi-client/client.h>
#ifdef AVAHI_API_0_6
#include <avahi-client/lookup.h>
#endif
#endif

#define TIMEOUT_LAN 200

namespace DNSSD
{
#ifdef HAVE_DNSSD
#ifdef AVAHI_API_0_6

void services_callback(AvahiServiceBrowser*, AvahiIfIndex, AvahiProtocol, AvahiBrowserEvent event, const char* name,
    const char* regtype, const char* domain, AvahiLookupResultFlags, void* context);
void types_callback(AvahiServiceTypeBrowser*, AvahiIfIndex, AvahiProtocol, AvahiBrowserEvent event, const char* regtype,
    const char* replyDomain, AvahiLookupResultFlags, void* context);
#else
void services_callback(AvahiServiceBrowser*, AvahiIfIndex, AvahiProtocol, AvahiBrowserEvent event, const char* name,
    const char* regtype, const char* domain, void* context);
void types_callback(AvahiServiceTypeBrowser*, AvahiIfIndex, AvahiProtocol, AvahiBrowserEvent event, const char* regtype,
    const char* replyDomain, void* context);
void domains_callback(AvahiDomainBrowser*,  AvahiIfIndex, AvahiProtocol, AvahiBrowserEvent event, const char* replyDomain,
     void* context);
#endif
#endif

enum BrowserType { Types, Services };

class QueryPrivate
{
public:
	QueryPrivate(const TQString& type, const TQString& domain) : m_finished(false), m_browser(0),
	m_running(false), m_domain(domain), m_type(type) {}

	bool m_finished;
	BrowserType m_browserType;
	void* m_browser;
	bool m_running;
	TQString m_domain;
	TQTimer timeout;
	TQString m_type;
};

Query::Query(const TQString& type, const TQString& domain)
{
	d = new QueryPrivate(type,domain);
	connect(&d->timeout,TQ_SIGNAL(timeout()),this,TQ_SLOT(timeout()));
}


Query::~Query()
{
#ifdef HAVE_DNSSD
	if (d->m_browser) {
	    switch (d->m_browserType) {
		case Services: avahi_service_browser_free((AvahiServiceBrowser*)d->m_browser); break;
		case Types: avahi_service_type_browser_free((AvahiServiceTypeBrowser*)d->m_browser); break;
	    }
	}
#endif
	delete d;
}

bool Query::isRunning() const
{
	return d->m_running;
}

bool Query::isFinished() const
{
	return d->m_finished;
}

const TQString& Query::domain() const
{
	return d->m_domain;
}

void Query::startQuery()
{
	if (d->m_running) return;
	d->m_finished = false;
	if (d->m_type=="_services._dns-sd._udp") {
	    d->m_browserType = Types;
#ifdef HAVE_DNSSD
#ifdef AVAHI_API_0_6
	    d->m_browser = avahi_service_type_browser_new(Responder::self().client(), AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
		domainToDNS(d->m_domain), (AvahiLookupFlags)0, types_callback, this);
#else
	    d->m_browser = avahi_service_type_browser_new(Responder::self().client(), AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
		d->m_domain.utf8(), types_callback, this);
#endif
#endif
	} else {
	    d->m_browserType = Services;
#ifdef HAVE_DNSSD
#ifdef AVAHI_API_0_6
	    d->m_browser = avahi_service_browser_new(Responder::self().client(), AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
	    d->m_type.ascii(),domainToDNS(d->m_domain),  (AvahiLookupFlags)0, services_callback,this);
#else
	    d->m_browser = avahi_service_browser_new(Responder::self().client(), AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
	    d->m_type.ascii(),d->m_domain.utf8(),services_callback,this);
#endif
#endif
	}
	if (d->m_browser) {
		d->m_running=true;
		d->timeout.start(TIMEOUT_LAN,true);
	} else emit finished();
}
void Query::virtual_hook(int, void*)
{
}

void Query::customEvent(TQCustomEvent* event)
{
	if (event->type()==TQEvent::User+SD_ADDREMOVE) {
		d->timeout.start(TIMEOUT_LAN,true);
		d->m_finished=false;
		AddRemoveEvent *aev = static_cast<AddRemoveEvent*>(event);
		// m_type has useless trailing dot
		RemoteService*  svr = new RemoteService(aev->m_name,
		    	aev->m_type,aev->m_domain);
		if (aev->m_op==AddRemoveEvent::Add) emit serviceAdded(svr);
			else emit serviceRemoved(svr);
	}
}

void Query::timeout()
{
	d->m_finished=true;
	emit finished();
}

#ifdef HAVE_DNSSD
#ifdef AVAHI_API_0_6
void services_callback (AvahiServiceBrowser*, AvahiIfIndex, AvahiProtocol, AvahiBrowserEvent event,
    const char* serviceName, const char* regtype, const char* replyDomain, AvahiLookupResultFlags, void* context)
#else
void services_callback (AvahiServiceBrowser*, AvahiIfIndex, AvahiProtocol, AvahiBrowserEvent event,
    const char* serviceName, const char* regtype, const char* replyDomain, void* context)
#endif
{
	TQObject *obj = reinterpret_cast<TQObject*>(context);
	AddRemoveEvent* arev = new AddRemoveEvent((event==AVAHI_BROWSER_NEW) ? AddRemoveEvent::Add :
			AddRemoveEvent::Remove, TQString::fromUtf8(serviceName), regtype,
			DNSToDomain(replyDomain));
		TQApplication::postEvent(obj, arev);
}

#ifdef AVAHI_API_0_6
void types_callback(AvahiServiceTypeBrowser*, AvahiIfIndex, AvahiProtocol, AvahiBrowserEvent event, const char* regtype,
    const char* replyDomain, AvahiLookupResultFlags, void* context)
#else
void types_callback(AvahiServiceTypeBrowser*, AvahiIfIndex, AvahiProtocol, AvahiBrowserEvent event, const char* regtype,
    const char* replyDomain, void* context)
#endif
{
	TQObject *obj = reinterpret_cast<TQObject*>(context);
	AddRemoveEvent* arev = new AddRemoveEvent((event==AVAHI_BROWSER_NEW) ? AddRemoveEvent::Add :
			AddRemoveEvent::Remove, TQString::null, regtype,
			DNSToDomain(replyDomain));
		TQApplication::postEvent(obj, arev);
}
#endif

}
#include "query.moc"
