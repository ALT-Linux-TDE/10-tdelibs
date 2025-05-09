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

#include <signal.h>
#include <errno.h>
#include <tqstringlist.h>
#include <tqfile.h>
#include "domainbrowser.h"
#include "responder.h"
#include "query.h"
#include "servicebrowser.h"
#ifdef HAVE_DNSSD
#include <avahi-client/client.h>
#endif
#include <config.h>

namespace DNSSD
{

const TQString ServiceBrowser::AllServices = "_services._dns-sd._udp";

class ServiceBrowserPrivate
{
public:
	ServiceBrowserPrivate() : m_running(false)
	{}
	TQValueList<RemoteService::Ptr> m_services;
	TQValueList<RemoteService::Ptr> m_duringResolve;
	TQStringList m_types;
	DomainBrowser* m_domains;
	int m_flags;
	bool m_running;
	bool m_finished;
	TQDict<Query> resolvers;
};

ServiceBrowser::ServiceBrowser(const TQString& type,DomainBrowser* domains,bool autoResolve)
{
	if (domains) init(type,domains,autoResolve ? AutoResolve : 0);
		else init(type,new DomainBrowser(this),autoResolve ?  AutoResolve|AutoDelete : AutoDelete);
}
ServiceBrowser::ServiceBrowser(const TQStringList& types,DomainBrowser* domains,int flags)
{
	if (domains) init(types,domains,flags);
		else init(types,new DomainBrowser(this),flags|AutoDelete);
}

void ServiceBrowser::init(const TQStringList& type,DomainBrowser* domains,int flags)
{
	d = new ServiceBrowserPrivate();
	d->resolvers.setAutoDelete(true);
	d->m_types=type;
	d->m_flags=flags;
	d->m_domains = domains;
	connect(d->m_domains,TQ_SIGNAL(domainAdded(const TQString& )),this,TQ_SLOT(addDomain(const TQString& )));
	connect(d->m_domains,TQ_SIGNAL(domainRemoved(const TQString& )),this,
		TQ_SLOT(removeDomain(const TQString& )));
}
ServiceBrowser::ServiceBrowser(const TQString& type,const TQString& domain,bool autoResolve)
{
	init(type,new DomainBrowser(domain,false,this),autoResolve ? AutoResolve|AutoDelete : AutoDelete);
}
ServiceBrowser::ServiceBrowser(const TQString& type,const TQString& domain,int flags)
{
	init(type,new DomainBrowser(domain,false,this),flags | AutoDelete);
}

const ServiceBrowser::State ServiceBrowser::isAvailable()
{
#ifdef HAVE_DNSSD
	AvahiClientState s = Responder::self().state();
#ifdef AVAHI_API_0_6
	return (s==AVAHI_CLIENT_FAILURE) ? Stopped : Working;
#else
	return (s==AVAHI_CLIENT_S_INVALID || s==AVAHI_CLIENT_DISCONNECTED) ? Stopped : Working;
#endif
#else
	return Unsupported;
#endif
}
ServiceBrowser::~ ServiceBrowser()
{
	if (d->m_flags & AutoDelete) delete d->m_domains;
	delete d;
}

const DomainBrowser* ServiceBrowser::browsedDomains() const
{
	return d->m_domains;
}

void ServiceBrowser::serviceResolved(bool success)
{
	TQObject* sender_obj = const_cast<TQObject*>(sender());
	RemoteService* svr = static_cast<RemoteService*>(sender_obj);
	disconnect(svr,TQ_SIGNAL(resolved(bool)),this,TQ_SLOT(serviceResolved(bool)));
	TQValueList<RemoteService::Ptr>::Iterator it = d->m_duringResolve.begin();
	TQValueList<RemoteService::Ptr>::Iterator itEnd = d->m_duringResolve.end();
	while ( it!= itEnd && svr!= (*it)) ++it;
	if (it != itEnd) {
		if (success) {
		  	d->m_services+=(*it);
			emit serviceAdded(svr);
		}
		d->m_duringResolve.remove(it);
		queryFinished();
	}
}

void ServiceBrowser::startBrowse()
{
	if (d->m_running) return;
	d->m_running=true;
	if (isAvailable()!=Working) return;
	if (d->m_domains->isRunning()) {
		TQStringList::const_iterator itEnd  = d->m_domains->domains().end();
		for ( TQStringList::const_iterator it = d->m_domains->domains().begin(); it != itEnd; ++it )
			addDomain(*it);
	} else d->m_domains->startBrowse();
}

void ServiceBrowser::gotNewService(RemoteService::Ptr svr)
{
	if (findDuplicate(svr)==(d->m_services.end()))  {
		if (d->m_flags & AutoResolve) {
			connect(svr,TQ_SIGNAL(resolved(bool )),this,TQ_SLOT(serviceResolved(bool )));
			d->m_duringResolve+=svr;
			svr->resolveAsync();
		} else	{
			d->m_services+=svr;
			emit serviceAdded(svr);
		}
	}
}

void ServiceBrowser::gotRemoveService(RemoteService::Ptr svr)
{
	TQValueList<RemoteService::Ptr>::Iterator it = findDuplicate(svr);
	if (it!=(d->m_services.end())) {
		emit serviceRemoved(*it);
		d->m_services.remove(it);
	}
}


void ServiceBrowser::removeDomain(const TQString& domain)
{
	while (d->resolvers[domain]) d->resolvers.remove(domain);
	TQValueList<RemoteService::Ptr>::Iterator it = d->m_services.begin();
	while (it!=d->m_services.end())
		// use section to skip possible trailing dot
		if ((*it)->domain().section('.',0) == domain.section('.',0)) {
			emit serviceRemoved(*it);
			it = d->m_services.remove(it);
		} else ++it;
}

void ServiceBrowser::addDomain(const TQString& domain)
{
	if (!d->m_running) return;
	if (!(d->resolvers[domain])) {
		TQStringList::ConstIterator itEnd = d->m_types.end();
		for (TQStringList::ConstIterator it=d->m_types.begin(); it!=itEnd; ++it) {
			Query* b = new Query((*it),domain);
			connect(b,TQ_SIGNAL(serviceAdded(DNSSD::RemoteService::Ptr)),this,
				TQ_SLOT(gotNewService(DNSSD::RemoteService::Ptr)));
			connect(b,TQ_SIGNAL(serviceRemoved(DNSSD::RemoteService::Ptr )),this,
				TQ_SLOT(gotRemoveService(DNSSD::RemoteService::Ptr)));
			connect(b,TQ_SIGNAL(finished()),this,TQ_SLOT(queryFinished()));
			b->startQuery();
			d->resolvers.insert(domain,b);
		}
	}
}

void ServiceBrowser::queryFinished()
{
	if (allFinished()) emit finished();
}

bool ServiceBrowser::allFinished()
{
	if  (d->m_duringResolve.count()) return false;
	bool all = true;
	TQDictIterator<Query> it(d->resolvers);
	for ( ; it.current(); ++it) all&=(*it)->isFinished();
	return all;
}

const TQValueList<RemoteService::Ptr>& ServiceBrowser::services() const
{
	return d->m_services;
}

void ServiceBrowser::virtual_hook(int, void*)
{}

TQValueList<RemoteService::Ptr>::Iterator ServiceBrowser::findDuplicate(RemoteService::Ptr src)
{
	TQValueList<RemoteService::Ptr>::Iterator itEnd = d->m_services.end();
	for (TQValueList<RemoteService::Ptr>::Iterator it = d->m_services.begin(); it!=itEnd; ++it)
		if ((src->type()==(*it)->type()) && (src->serviceName()==(*it)->serviceName()) &&
				   (src->domain() == (*it)->domain())) return it;
	return itEnd;
}


}

#include "servicebrowser.moc"
