/*
 * Copyright (C) 2004 Frans Englich <frans.englich@telia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
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

#include <tqcstring.h>
#include <tqdatastream.h>

#include <dcopclient.h>

#include <tdeapplication.h>
#include <tdecmoduleproxy.h>
#include <kdebug.h>

#include "tdecmoduleproxyIfaceImpl.h"


#include <tqmessagebox.h>

TDECModuleProxyIfaceImpl::TDECModuleProxyIfaceImpl( const TQCString& name, 
		TDECModuleProxy* const client )
	: DCOPObject( name ), TQObject( 0, name ),
		p( const_cast<TDECModuleProxy *>( client ))
{ 
	connect( p, TQ_SIGNAL( changed(bool)), 
			TQ_SLOT( changedRelay(bool)));
	connect( p, TQ_SIGNAL( quickHelpChanged()), 
			TQ_SLOT( quickHelpRelay()));
}

void TDECModuleProxyIfaceImpl::save()
{
	kdDebug(711) << k_funcinfo << endl;
	p->save();
}

void TDECModuleProxyIfaceImpl::load()
{
	kdDebug(711) << k_funcinfo << endl;
	p->load();
}

void TDECModuleProxyIfaceImpl::defaults()
{
	kdDebug(711) << k_funcinfo << endl;
	p->defaults();
}

TQString TDECModuleProxyIfaceImpl::applicationName()
{
	return kapp->caption();
}

TQString TDECModuleProxyIfaceImpl::quickHelp()
{
	return p->quickHelp();
}

bool TDECModuleProxyIfaceImpl::changed()
{
	return p->changed();
}

void TDECModuleProxyIfaceImpl::changedRelay( bool c )
{
	TQByteArray data;
	TQDataStream stream(data, IO_WriteOnly);
	stream << c;
	emitDCOPSignal( "changed(bool)", data );
}

void TDECModuleProxyIfaceImpl::quickHelpRelay()
{
	TQByteArray data;
	emitDCOPSignal( "quickHelpChanged()", data );
}

/***************************************************************/




/***************************************************************/
TDECModuleProxyRootCommunicatorImpl::TDECModuleProxyRootCommunicatorImpl
		( const TQCString& name, TDECModuleProxy* const client )
	: DCOPObject( name ), TQObject( 0, name ), 
		p( const_cast<TDECModuleProxy *>( client ))
{ 
	/*
	 * Connect tdecmshell's TDECModuleProxy's change signal 
	 * to us, such that we act as a proxy for 
	 * TDECModuleProxy's API.
	 */

	/* Note, we don't use TDECModuleProxy::d->dcopClient */
	kapp->dcopClient()->connectDCOPSignal( 0, p->dcopName(), 
			"changed(bool)", objId(), "changed(bool)", false );

	kapp->dcopClient()->connectDCOPSignal( 0, p->dcopName(), 
			"quickHelpChanged()", objId(), "quickHelpChanged()", false );
}

/* Reimplementations of DCOP members */
void TDECModuleProxyRootCommunicatorImpl::changed( bool c )
{
	kdDebug(711) << k_funcinfo << endl;
	p->moduleChanged( c );
}

void TDECModuleProxyRootCommunicatorImpl::quickHelpChanged()
{
	kdDebug(711) << k_funcinfo << endl;
	p->emitQuickHelpChanged();
}

#include "tdecmoduleproxyIfaceImpl.moc"
