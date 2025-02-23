/*
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997-2000 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (c) 2001,2002 Ellis Whitehead <ellis@kde.org>

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

#include "tdeaccelaction.h"
#include "tdeaccelbase.h"   // for TDEAccelBase::slotRemoveAction() & emitSignal()

#include <tqkeycode.h>

#include <tdeconfig.h>
#include "kckey.h"
#include <kdebug.h>
#include <tdeglobal.h>
#include <kkeynative.h>
#include <tdelocale.h>
#include <tdeshortcutlist.h>

//---------------------------------------------------------------------
// TDEAccelAction
//---------------------------------------------------------------------

class TDEAccelActionPrivate
{
 public:
	uint m_nConnections;
};

TDEAccelAction::TDEAccelAction()
{
	//kdDebug(125) << "TDEAccelAction(): this = " << this << endl;
	d = new TDEAccelActionPrivate;
	m_pObjSlot = 0;
	m_psMethodSlot = 0;
	m_bConfigurable = true;
	m_bEnabled = true;
	m_nIDAccel = 0;
	d->m_nConnections = 0;
}

TDEAccelAction::TDEAccelAction( const TDEAccelAction& action )
{
	//kdDebug(125) << "TDEAccelAction( copy from \"" << action.m_sName << "\" ): this = " << this << endl;
	d = new TDEAccelActionPrivate;
	*this = action;
}

TDEAccelAction::TDEAccelAction( const TQString& sName, const TQString& sLabel, const TQString& sWhatsThis,
			const TDEShortcut& cutDef3, const TDEShortcut& cutDef4,
			const TQObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled )
{
	//kdDebug(125) << "TDEAccelAction( \"" << sName << "\" ): this = " << this << endl;
	d = new TDEAccelActionPrivate;
	init( sName, sLabel, sWhatsThis,
		cutDef3, cutDef4,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
}

TDEAccelAction::~TDEAccelAction()
{
	//kdDebug(125) << "\t\t\tTDEAccelAction::~TDEAccelAction( \"" << m_sName << "\" ): this = " << this << endl;
	delete d;
}

void TDEAccelAction::clear()
{
	m_cut.clear();
	m_pObjSlot = 0;
	m_psMethodSlot = 0;
	m_bConfigurable = true;
	m_bEnabled = true;
	m_nIDAccel = 0;
	d->m_nConnections = 0;
}

bool TDEAccelAction::init( const TQString& sName, const TQString& sLabel, const TQString& sWhatsThis,
			const TDEShortcut& rgCutDefaults3, const TDEShortcut& rgCutDefaults4,
			const TQObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled )
{
	m_sName = sName;
	m_sLabel = sLabel;
	m_sWhatsThis = sWhatsThis;
	m_cutDefault3 = rgCutDefaults3;
	m_cutDefault4 = rgCutDefaults4;
	m_pObjSlot = pObjSlot;
	m_psMethodSlot = psMethodSlot;
	m_bConfigurable = bConfigurable;
	m_bEnabled = bEnabled;
	m_nIDAccel = 0;
	m_cut = shortcutDefault();
	d->m_nConnections = 0;
	if( !m_bEnabled )
		kdDebug(125) << "TDEAccelAction::init( \"" << sName << "\" ): created with enabled = false" << endl;
	return true;
}

TDEAccelAction& TDEAccelAction::operator =( const TDEAccelAction& action )
{
	m_sName          = action.m_sName;
	m_sLabel         = action.m_sLabel;
	m_sWhatsThis     = action.m_sWhatsThis;
	m_cutDefault3    = action.m_cutDefault3;
	m_cutDefault4    = action.m_cutDefault4;
	m_pObjSlot       = action.m_pObjSlot;
	m_psMethodSlot   = action.m_psMethodSlot;
	m_bConfigurable  = action.m_bConfigurable;
	m_bEnabled       = action.m_bEnabled;
	m_nIDAccel       = action.m_nIDAccel;
	m_cut            = action.m_cut;
	d->m_nConnections = action.d->m_nConnections;

	return *this;
}

void TDEAccelAction::setName( const TQString& s )
	{ m_sName = s; }
void TDEAccelAction::setLabel( const TQString& s )
	{ m_sLabel = s; }
void TDEAccelAction::setWhatsThis( const TQString& s )
	{ m_sWhatsThis = s; }

bool TDEAccelAction::setShortcut( const TDEShortcut& cut )
{
	m_cut = cut;
	return true;
}

void TDEAccelAction::setSlot( const TQObject* pObjSlot, const char* psMethodSlot )
{
	m_pObjSlot = pObjSlot;
	m_psMethodSlot = psMethodSlot;
}

void TDEAccelAction::setConfigurable( bool b )
	{ m_bConfigurable = b; }
void TDEAccelAction::setEnabled( bool b )
	{ m_bEnabled = b; }

TQString TDEAccelAction::toString() const
	{ return m_cut.toString(); }

TQString TDEAccelAction::toStringInternal() const
	{ return m_cut.toStringInternal( &shortcutDefault() ); }

bool TDEAccelAction::setKeySequence( uint i, const KKeySequence& seq )
{
	if( i < m_cut.count() ) {
		m_cut.setSeq( i, seq );
		return true;
	} else if( i == m_cut.count() )
		return m_cut.append( seq );
	return false;
}

void TDEAccelAction::clearShortcut()
{
	m_cut.clear();
}

bool TDEAccelAction::contains( const KKeySequence& seq )
{
	return m_cut.contains( seq );
	for( uint i = 0; i < m_cut.count(); i++ ) {
		if( m_cut.seq(i) == seq )
			return true;
	}
	return false;
}

const TDEShortcut& TDEAccelAction::shortcutDefault() const
	{ return (useFourModifierKeys()) ? m_cutDefault4 : m_cutDefault3; }
bool TDEAccelAction::isConnected() const
	{ return d->m_nConnections; }
void TDEAccelAction::incConnections()
	{ d->m_nConnections++; }
void TDEAccelAction::decConnections()
	{ if( d->m_nConnections > 0 ) d->m_nConnections--; }

// Indicate whether to default to the 3- or 4- modifier keyboard schemes
int TDEAccelAction::g_bUseFourModifierKeys = -1;

bool TDEAccelAction::useFourModifierKeys()
{
	if( TDEAccelAction::g_bUseFourModifierKeys == -1 ) {
		// Read in whether to use 4 modifier keys
		TDEConfigGroupSaver cgs( TDEGlobal::config(), "Keyboard" );
		bool b = TDEGlobal::config()->readBoolEntry( "Use Four Modifier Keys",  false );
		TDEAccelAction::g_bUseFourModifierKeys = b && KKeyNative::keyboardHasWinKey();
	}
	return TDEAccelAction::g_bUseFourModifierKeys == 1;
}

void TDEAccelAction::useFourModifierKeys( bool b )
{
	if( TDEAccelAction::g_bUseFourModifierKeys != (int)b ) {
		TDEAccelAction::g_bUseFourModifierKeys = b && KKeyNative::keyboardHasWinKey();
		// If we're 'turning off' the meta key or, if we're turning it on,
		//  the keyboard must actually have a meta key.
		if( b && !KKeyNative::keyboardHasWinKey() )
			kdDebug(125) << "Tried to use four modifier keys on a keyboard layout without a Meta key.\n";
	}
	TDEConfigGroupSaver cgs( TDEGlobal::config(), "Keyboard" );
	TDEGlobal::config()->writeEntry( "Use Four Modifier Keys", TDEAccelAction::g_bUseFourModifierKeys, true, true);

	kdDebug(125) << "bUseFourModifierKeys = " << TDEAccelAction::g_bUseFourModifierKeys << endl;
}

//---------------------------------------------------------------------
// TDEAccelActions
//---------------------------------------------------------------------

class TDEAccelActionsPrivate
{
 public:
};

TDEAccelActions::TDEAccelActions()
{
	kdDebug(125) << "TDEAccelActions(): this = " << this << endl;
	initPrivate( 0 );
}

TDEAccelActions::TDEAccelActions( const TDEAccelActions& actions )
{
	kdDebug(125) << "TDEAccelActions( actions = " << &actions << " ): this = " << this << endl;
	initPrivate( 0 );
	init( actions );
}

TDEAccelActions::TDEAccelActions( TDEAccelBase* pTDEAccelBase )
{
	kdDebug(125) << "TDEAccelActions( TDEAccelBase = " << pTDEAccelBase << " ): this = " << this << endl;
	initPrivate( pTDEAccelBase );
}

TDEAccelActions::~TDEAccelActions()
{
	//kdDebug(125) << "TDEAccelActions::~TDEAccelActions(): this = " << this << endl;
	clear();
	//delete d;
}

void TDEAccelActions::initPrivate( TDEAccelBase* pTDEAccelBase )
{
	m_pTDEAccelBase = pTDEAccelBase;
	m_nSizeAllocated = m_nSize = 0;
	m_prgActions = 0;
	//d = new TDEAccelActionsPrivate;
}

void TDEAccelActions::clear()
{
	kdDebug(125) << "\tTDEAccelActions::clear()" << endl;
	for( uint i = 0; i < m_nSize; i++ )
		delete m_prgActions[i];
	delete[] m_prgActions;

	m_nSizeAllocated = m_nSize = 0;
	m_prgActions = 0;
}

bool TDEAccelActions::init( const TDEAccelActions& actions )
{
	clear();
	resize( actions.count() );
	for( uint i = 0; i < m_nSize; i++ ) {
		TDEAccelAction* pAction = actions.m_prgActions[i];
		if( pAction )
			m_prgActions[i] = new TDEAccelAction( *pAction );
		else
			m_prgActions[i] = 0;
	}

	return true;
}

bool TDEAccelActions::init( TDEConfigBase& config, const TQString& sGroup )
{
	kdDebug(125) << "TDEAccelActions::init( " << sGroup << " )" << endl;
	TQMap<TQString, TQString> mapEntry = config.entryMap( sGroup );
	resize( mapEntry.count() );

	TQMap<TQString, TQString>::Iterator it( mapEntry.begin() );
	for( uint i = 0; it != mapEntry.end(); ++it, i++ ) {
		TQString sShortcuts = *it;
		TDEShortcut cuts;

		kdDebug(125) << it.key() << " = " << sShortcuts << endl;
		if( !sShortcuts.isEmpty() && sShortcuts != "none" )
			cuts.init( sShortcuts );

		m_prgActions[i] = new TDEAccelAction( it.key(), it.key(), it.key(),
			cuts, cuts,
			0, 0,          // pObjSlot, psMethodSlot,
			true, false ); // bConfigurable, bEnabled
	}

	return true;
}

void TDEAccelActions::resize( uint nSize )
{
	if( nSize > m_nSizeAllocated ) {
		uint nSizeAllocated = ((nSize/10) + 1) * 10;
		TDEAccelAction** prgActions = new TDEAccelAction* [nSizeAllocated];

		// Copy pointers over to new array
		for( uint i = 0; i < m_nSizeAllocated; i++ )
			prgActions[i] = m_prgActions[i];

		// Null out new pointers
		for( uint i = m_nSizeAllocated; i < nSizeAllocated; i++ )
			prgActions[i] = 0;

		delete[] m_prgActions;
		m_prgActions = prgActions;
		m_nSizeAllocated = nSizeAllocated;
	}

	m_nSize = nSize;
}

void TDEAccelActions::insertPtr( TDEAccelAction* pAction )
{
	resize( m_nSize + 1 );
	m_prgActions[m_nSize-1] = pAction;
}

void TDEAccelActions::updateShortcuts( TDEAccelActions& actions2 )
{
	kdDebug(125) << "TDEAccelActions::updateShortcuts()" << endl;
	bool bChanged = false;

	for( uint i = 0; i < m_nSize; i++ ) {
		TDEAccelAction* pAction = m_prgActions[i];
		if( pAction && pAction->m_bConfigurable ) {
			TDEAccelAction* pAction2 = actions2.actionPtr( pAction->m_sName );
			if( pAction2 ) {
				TQString sOld = pAction->m_cut.toStringInternal();
				pAction->m_cut = pAction2->m_cut;
				kdDebug(125) << "\t" << pAction->m_sName
					<< " found: " << sOld
					<< " => " << pAction2->m_cut.toStringInternal()
					<< " = " << pAction->m_cut.toStringInternal() << endl;
				bChanged = true;
			}
		}
	}

	if( bChanged )
		emitKeycodeChanged();
}

int TDEAccelActions::actionIndex( const TQString& sAction ) const
{
	for( uint i = 0; i < m_nSize; i++ ) {
		if( m_prgActions[i] == 0 )
			kdWarning(125) << "TDEAccelActions::actionPtr( " << sAction << " ): encountered null pointer at m_prgActions[" << i << "]" << endl;
		else if( m_prgActions[i]->m_sName == sAction )
			return (int) i;
	}
	return -1;
}

TDEAccelAction* TDEAccelActions::actionPtr( uint i )
{
	return m_prgActions[i];
}

const TDEAccelAction* TDEAccelActions::actionPtr( uint i ) const
{
	return m_prgActions[i];
}

TDEAccelAction* TDEAccelActions::actionPtr( const TQString& sAction )
{
	int i = actionIndex( sAction );
	return (i >= 0) ? m_prgActions[i] : 0;
}

const TDEAccelAction* TDEAccelActions::actionPtr( const TQString& sAction ) const
{
	int i = actionIndex( sAction );
	return (i >= 0) ? m_prgActions[i] : 0;
}

TDEAccelAction* TDEAccelActions::actionPtr( KKeySequence cut )
{
	for( uint i = 0; i < m_nSize; i++ ) {
		if( m_prgActions[i] == 0 )
			kdWarning(125) << "TDEAccelActions::actionPtr( " << cut.toStringInternal() << " ): encountered null pointer at m_prgActions[" << i << "]" << endl;
		else if( m_prgActions[i]->contains( cut ) )
			return m_prgActions[i];
	}
	return 0;
}

TDEAccelAction& TDEAccelActions::operator []( uint i )
{
	return *actionPtr( i );
}

const TDEAccelAction& TDEAccelActions::operator []( uint i ) const
{
	return *actionPtr( i );
}

TDEAccelAction* TDEAccelActions::insert( const TQString& sName, const TQString& sLabel )
{
	if( actionPtr( sName ) ) {
		kdWarning(125) << "TDEAccelActions::insertLabel( " << sName << ", " << sLabel << " ): action with same name already present." << endl;
		return 0;
	}

	TDEAccelAction* pAction = new TDEAccelAction;
	pAction->m_sName = sName;
	pAction->m_sLabel = sLabel;
	pAction->m_bConfigurable = false;
	pAction->m_bEnabled = false;

	insertPtr( pAction );
	return pAction;
}

TDEAccelAction* TDEAccelActions::insert( const TQString& sAction, const TQString& sLabel, const TQString& sWhatsThis,
			const TDEShortcut& rgCutDefaults3, const TDEShortcut& rgCutDefaults4,
			const TQObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled )
{
	//kdDebug(125) << "TDEAccelActions::insert()2 begin" << endl;
	if( actionPtr( sAction ) ) {
		kdWarning(125) << "TDEAccelActions::insert( " << sAction << " ): action with same name already present." << endl;
		return 0;
	}

	TDEAccelAction* pAction = new TDEAccelAction(
		sAction, sLabel, sWhatsThis,
		rgCutDefaults3, rgCutDefaults4,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
	insertPtr( pAction );

	//kdDebug(125) << "TDEAccelActions::insert()2 end" << endl;
	return pAction;
}

bool TDEAccelActions::remove( const TQString& sAction )
{
	kdDebug(125) << "TDEAccelActions::remove( \"" << sAction << "\" ): this = " << this << " m_pTDEAccelBase = " << m_pTDEAccelBase << endl;

	int iAction = actionIndex( sAction );
	if( iAction < 0 )
		return false;

	if( m_pTDEAccelBase )
		m_pTDEAccelBase->slotRemoveAction( m_prgActions[iAction] );
	delete m_prgActions[iAction];

	for( uint i = iAction; i < m_nSize - 1; i++ )
		m_prgActions[i] = m_prgActions[i+1];
	m_nSize--;

	return true;
}

bool TDEAccelActions::readActions( const TQString& sConfigGroup, TDEConfigBase* pConfig )
{
	TDEAccelShortcutList accelList(*this, false);
	return accelList.readSettings( sConfigGroup, pConfig );
}

/*
	1) TDEAccelAction = "Something"
		1) KKeySequence = "Meta+X,Asterisk"
			1) TDEAccelSequence = "Meta+X"
				1) KKeySequence = Meta+X
			2) TDEAccelSequence = "Asterisk"
				1) KKeySequence = Shift+8 (English layout)
				2) KKeySequence = Keypad_Asterisk
		2) KKeySequence = "Alt+F2"
			1) TDEAccelSequence = "Alt+F2"
				1) KKeySequence = Alt+F2
	-> "Something=Meta+X,Asterisk;Alt+F2"
*/
bool TDEAccelActions::writeActions( const TQString &sGroup, TDEConfigBase* pConfig,
			bool bWriteAll, bool bGlobal ) const
{
	kdDebug(125) << "TDEAccelActions::writeActions( " << sGroup << ", " << pConfig << ", " << bWriteAll << ", " << bGlobal << " )" << endl;
	if( !pConfig )
		pConfig = TDEGlobal::config();
	TDEConfigGroupSaver cs( pConfig, sGroup );

	for( uint i = 0; i < m_nSize; i++ ) {
		if( m_prgActions[i] == 0 ) {
			kdWarning(125) << "TDEAccelActions::writeActions(): encountered null pointer at m_prgActions[" << i << "]" << endl;
			continue;
		}
		const TDEAccelAction& action = *m_prgActions[i];

		TQString s;
		bool bConfigHasAction = !pConfig->readEntry( action.m_sName ).isEmpty();
		bool bSameAsDefault = true;
		bool bWriteAction = false;

		if( action.m_bConfigurable ) {
			s = action.toStringInternal();
			bSameAsDefault = (action.m_cut == action.shortcutDefault());

			//if( bWriteAll && s.isEmpty() )
			if( s.isEmpty() )
				s = "none";

			// If we're using a global config or this setting
			//  differs from the default, then we want to write.
			if( bWriteAll || !bSameAsDefault )
				bWriteAction = true;

			if( bWriteAction ) {
				kdDebug(125) << "\twriting " << action.m_sName << " = " << s << endl;
				// Is passing bGlobal irrelevant, since if it's true,
				//  then we're using the global config anyway? --ellis
				pConfig->writeEntry( action.m_sName, s, true, bGlobal );
			}
			// Otherwise, this key is the same as default
			//  but exists in config file.  Remove it.
			else if( bConfigHasAction ) {
				kdDebug(125) << "\tremoving " << action.m_sName << " because == default" << endl;
				pConfig->deleteEntry( action.m_sName, bGlobal );
			}

		}
	}

	pConfig->sync();
	return true;
}

void TDEAccelActions::emitKeycodeChanged()
{
	if( m_pTDEAccelBase )
		m_pTDEAccelBase->emitSignal( TDEAccelBase::KEYCODE_CHANGED );
}

uint TDEAccelActions::count() const
	{ return m_nSize; }
