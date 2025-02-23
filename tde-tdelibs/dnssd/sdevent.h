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

#ifndef DNSSDSDEVENT_H
#define DNSSDSDEVENT_H

#include <tqevent.h>
#include <tqstring.h>
#include <tqmap.h>

namespace DNSSD
{

enum Operation { SD_ERROR = 101,SD_ADDREMOVE, SD_PUBLISH, SD_RESOLVE};

class ErrorEvent : public TQCustomEvent
{
public:
	ErrorEvent() : TQCustomEvent(TQEvent::User+SD_ERROR) 
	{}
};
class AddRemoveEvent : public TQCustomEvent
{
public:
	enum Operation { Add, Remove };
	AddRemoveEvent(Operation op,const TQString& name,const TQString& type,
		const TQString& domain) : TQCustomEvent(TQEvent::User+SD_ADDREMOVE),
	m_op(op), m_name(name), m_type(type), m_domain(domain) 
	{}

	const Operation m_op;
	const TQString m_name;
	const TQString m_type;
	const TQString m_domain;
};

class PublishEvent : public TQCustomEvent
{
public:
	PublishEvent(bool ok) : TQCustomEvent(TQEvent::User+SD_PUBLISH), m_ok(ok)
	{}

	bool m_ok;
};

class ResolveEvent : public TQCustomEvent
{
public:
	ResolveEvent(const TQString& hostname, unsigned short port,
		     const TQMap<TQString,TQString>& txtdata) 
		: TQCustomEvent(TQEvent::User+SD_RESOLVE), m_hostname(hostname),
		  m_port(port), m_txtdata(txtdata)
	{}

	const TQString m_hostname;
	const unsigned short m_port;
	const TQMap<TQString,TQString> m_txtdata;
};


}

#endif
