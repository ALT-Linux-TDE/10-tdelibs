/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kglobalaccel.h"
#ifdef TQ_WS_X11
#include "kglobalaccel_x11.h"
#elif defined(TQ_WS_WIN)
#include "kglobalaccel_win.h"
#elif defined(TQ_WS_MACX)
#include "kglobalaccel_mac.h"
#else
#include "kglobalaccel_emb.h"
#endif

#include <tqstring.h>
#include "tdeaccelbase.h"
#include <kdebug.h>
#include <tdeshortcut.h>
#include <tdelocale.h>

//----------------------------------------------------

TDEGlobalAccel::TDEGlobalAccel( TQObject* pParent, const char* psName )
: TQObject( pParent, psName )
{
	kdDebug(125) << "TDEGlobalAccel(): this = " << this << endl;
	d = new TDEGlobalAccelPrivate();
}

TDEGlobalAccel::~TDEGlobalAccel()
{
	kdDebug(125) << "~TDEGlobalAccel(): this = " << this << endl;
	delete d;
}

/*
void TDEGlobalAccel::clear()
	{ d->clearActions(); }
*/
TDEAccelActions& TDEGlobalAccel::actions()
	{ return d->TDEAccelBase::actions(); }

const TDEAccelActions& TDEGlobalAccel::actions() const
	{ return d->TDEAccelBase::actions(); }

bool TDEGlobalAccel::isEnabled()
	{ return ((TDEAccelBase*)d)->isEnabled(); }

void TDEGlobalAccel::setEnabled( bool bEnabled )
	{ d->setEnabled( bEnabled ); }

void TDEGlobalAccel::suspend( bool s )
	{ d->suspend( s ); }

void TDEGlobalAccel::blockShortcuts( bool block )
        { TDEGlobalAccelPrivate::blockShortcuts( block ); }

void TDEGlobalAccel::disableBlocking( bool disable )
        { d->disableBlocking( disable ); }

TDEAccelAction* TDEGlobalAccel::insert( const TQString& sAction, const TQString& sDesc, const TQString& sHelp,
		const TDEShortcut& cutDef3, const TDEShortcut& cutDef4,
		const TQObject* pObjSlot, const char* psMethodSlot,
		bool bConfigurable, bool bEnabled )
{
	return d->insert( sAction, sDesc, sHelp,
		cutDef3, cutDef4,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
}

TDEAccelAction* TDEGlobalAccel::insert( const TQString& sName, const TQString& sDesc )
	{ return d->insert( sName, sDesc ); }
bool TDEGlobalAccel::updateConnections()
	{ return d->updateConnections(); }

bool TDEGlobalAccel::remove( const TQString& sAction )
        { return d->remove( sAction ); }

const TDEShortcut& TDEGlobalAccel::shortcut( const TQString& sAction ) const
{
	const TDEAccelAction* pAction = d->TDEAccelBase::actions().actionPtr( sAction );
	return (pAction) ? pAction->shortcut() : TDEShortcut::null();
}

bool TDEGlobalAccel::setShortcut( const TQString& sAction, const TDEShortcut& cut )
	{ return d->setShortcut( sAction, cut ); }
bool TDEGlobalAccel::setSlot( const TQString& sAction, const TQObject* pObjSlot, const char* psMethodSlot )
	{ return d->setActionSlot( sAction, pObjSlot, psMethodSlot ); }
TQString TDEGlobalAccel::label( const TQString& sAction ) const
{
	const TDEAccelAction* pAction = d->TDEAccelBase::actions().actionPtr( sAction );
	return (pAction) ? pAction->label() : TQString();
}
bool TDEGlobalAccel::setActionEnabled( const TQString& sAction, bool bEnable )
{
        return d->setActionEnabled( sAction, bEnable );
}

const TQString& TDEGlobalAccel::configGroup() const
	{ return d->configGroup(); }
// for tdemultimedia/kmix
void TDEGlobalAccel::setConfigGroup( const TQString& s )
	{ d->setConfigGroup( s ); }

bool TDEGlobalAccel::readSettings( TDEConfigBase* pConfig )
	{ d->readSettings( pConfig ); return true; }
bool TDEGlobalAccel::writeSettings( TDEConfigBase* pConfig ) const
	{ d->writeSettings( pConfig ); return true; }
bool TDEGlobalAccel::writeSettings( TDEConfigBase* pConfig, bool bGlobal ) const
{
	d->setConfigGlobal( bGlobal );
	d->writeSettings( pConfig );
	return true;
}

bool TDEGlobalAccel::useFourModifierKeys()
	{ return TDEAccelAction::useFourModifierKeys(); }

void TDEGlobalAccel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kglobalaccel.moc"
