/*
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

#include "tdeaccel.h"

#include <tqaccel.h>
#include <tqguardedptr.h>
#include <tqpopupmenu.h>
#include <tqregexp.h>
#include <tqstring.h>
#include <tqtimer.h>

#include "tdeaccelbase.h"
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdeshortcut.h>

#include "tdeaccelprivate.h"

#ifdef TQ_WS_X11
#	include <X11/Xlib.h>
#	ifdef KeyPress // needed for --enable-final
		// defined by X11 headers
		const int XKeyPress = KeyPress;
#		undef KeyPress
#	endif
#endif

// TODO: Put in tdeaccelbase.cpp
//---------------------------------------------------------------------
// TDEAccelEventHandler
//---------------------------------------------------------------------
//
// In TDEAccelEventHandler::x11Event we do our own X11 keyboard event handling
// This allows us to map the Win key to TQt::MetaButton, Qt does not know about
// the Win key.
//
// TDEAccelEventHandler::x11Event will generate an AccelOverride event. The
// AccelOverride event is abused a bit to ensure that TDEAccelPrivate::eventFilter
// (as an event filter on the toplevel widget) will get the key event first
// (in the form of AccelOverride) before any of the intermediate widgets are
// able to process it.
//
// Qt normally sends an AccelOverride, Accel and then a KeyPress event.
// A widget can accept the AccelOverride event in which case the Accel event will be
// skipped and the KeyPress is followed immediately.
// If the Accel event is accepted, no KeyPress event will follow.
//
// TDEAccelEventHandler::x11Event converts a X11 keyboard event into an AccelOverride
// event, there are now two possibilities:
//
// 1) If TDEAccel intercepts the AccelOverride we are done and can consider the X11
// keyboard event as handled.
// 2) If another widget accepts the AccelOverride, it will expect to get a normal
// Qt generated KeyPress event afterwards. So we let Qt handle the X11 keyboard event
// again. However, this will first generate an AccelOverride event, and we already
// had send that one. To compnesate for this, the global event filter in TDEApplication
// is instructed to eat the next AccelOveride event. Qt will then send a normal KeyPress
// event and from then on everything is normal again.
//
// kde_g_bKillAccelOverride is used to tell TDEApplication::notify to eat the next
// AccelOverride event.

bool kde_g_bKillAccelOverride = false;

class TDEAccelEventHandler : public TQWidget
{
 public:
	static TDEAccelEventHandler* self()
	{
		if( !g_pSelf )
			g_pSelf = new TDEAccelEventHandler;
		return g_pSelf;
	}

	static void accelActivated( bool b ) { g_bAccelActivated = b; }

 private:
	TDEAccelEventHandler();

#	ifdef TQ_WS_X11
	bool x11Event( XEvent* pEvent );
#	endif

	static TDEAccelEventHandler* g_pSelf;
	static bool g_bAccelActivated;
};

TDEAccelEventHandler* TDEAccelEventHandler::g_pSelf = 0;
bool TDEAccelEventHandler::g_bAccelActivated = false;

TDEAccelEventHandler::TDEAccelEventHandler()
    : TQWidget( 0, "TDEAccelEventHandler" )
{
#	ifdef TQ_WS_X11
	if ( kapp )
		kapp->installX11EventFilter( this );
#	endif
}

#ifdef TQ_WS_X11
bool	tqt_try_modal( TQWidget *, XEvent * );

bool TDEAccelEventHandler::x11Event( XEvent* pEvent )
{
	if( TQWidget::keyboardGrabber() || !kapp->focusWidget() )
		return false;

	if ( !tqt_try_modal(kapp->focusWidget(), pEvent) )
	        return false;

	if( pEvent->type == XKeyPress ) {
		unsigned int tmp = pEvent->xkey.state;
		pEvent->xkey.state &= ~0x2000;
		KKeyNative keyNative( pEvent );
		pEvent->xkey.state = tmp;
		KKey key( keyNative );
		key.simplify();
		int keyCodeQt = key.keyCodeQt();
		int state = 0;
		if( key.modFlags() & KKey::SHIFT ) state |= TQt::ShiftButton;
		if( key.modFlags() & KKey::CTRL )  state |= TQt::ControlButton;
		if( key.modFlags() & KKey::ALT )   state |= TQt::AltButton;
		if( key.modFlags() & KKey::WIN )   state |= TQt::MetaButton;

		TQKeyEvent ke( TQEvent::AccelOverride, keyCodeQt, 0,  state );
		ke.ignore();

		g_bAccelActivated = false;
		kapp->sendEvent( kapp->focusWidget(), &ke );

		// If the Override event was accepted from a non-TDEAccel widget,
		//  then kill the next AccelOverride in TDEApplication::notify.
		if( ke.isAccepted() && !g_bAccelActivated )
			kde_g_bKillAccelOverride = true;

		// Stop event processing if a KDE accelerator was activated.
		return g_bAccelActivated;
	}

	return false;
}
#endif // TQ_WS_X11

//---------------------------------------------------------------------
// TDEAccelPrivate
//---------------------------------------------------------------------

TDEAccelPrivate::TDEAccelPrivate( TDEAccel* pParent, TQWidget* pWatch )
: TDEAccelBase( TDEAccelBase::QT_KEYS )
{
	//kdDebug(125) << "TDEAccelPrivate::TDEAccelPrivate( pParent = " << pParent << " ): this = " << this << endl;
	m_pAccel = pParent;
	m_pWatch = pWatch;
	m_bAutoUpdate = true;
	connect( (TQAccel*)m_pAccel, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotKeyPressed(int)) );

#ifdef TQ_WS_X11 //only makes sense if TDEAccelEventHandler is working
	if( m_pWatch )
		m_pWatch->installEventFilter( this );
#endif
	TDEAccelEventHandler::self();
}

void TDEAccelPrivate::setEnabled( bool bEnabled )
{
	m_bEnabled = bEnabled;
	((TQAccel*)m_pAccel)->setEnabled( bEnabled );
}

bool TDEAccelPrivate::setEnabled( const TQString& sAction, bool bEnable )
{
	kdDebug(125) << "TDEAccelPrivate::setEnabled( \"" << sAction << "\", " << bEnable << " ): this = " << this << endl;
	TDEAccelAction* pAction = actionPtr( sAction );
	if( !pAction )
		return false;
	if( pAction->isEnabled() == bEnable )
		return true;

	pAction->setEnabled( bEnable );

	TQMap<int, TDEAccelAction*>::const_iterator it = m_mapIDToAction.begin();
	for( ; it != m_mapIDToAction.end(); ++it ) {
		if( *it == pAction )
			((TQAccel*)m_pAccel)->setItemEnabled( it.key(), bEnable );
	}
	return true;
}

bool TDEAccelPrivate::removeAction( const TQString& sAction )
{
	// FIXME: getID() doesn't contains any useful
	//  information!  Use mapIDToAction. --ellis, 2/May/2002
	//  Or maybe TDEAccelBase::remove() takes care of TQAccel indirectly...
	TDEAccelAction* pAction = actions().actionPtr( sAction );
	if( pAction ) {
		int nID = pAction->getID();
		//bool b = actions().removeAction( sAction );
		bool b = TDEAccelBase::remove( sAction );
		((TQAccel*)m_pAccel)->removeItem( nID );
		return b;
	} else
		return false;
}

bool TDEAccelPrivate::emitSignal( TDEAccelBase::Signal signal )
{
	if( signal == TDEAccelBase::KEYCODE_CHANGED ) {
		m_pAccel->emitKeycodeChanged();
		return true;
	}
	return false;
}

bool TDEAccelPrivate::connectKey( TDEAccelAction& action, const KKeyServer::Key& key )
{
	uint keyQt = key.keyCodeQt();
	int nID = ((TQAccel*)m_pAccel)->insertItem( keyQt );
	m_mapIDToAction[nID] = &action;
	m_mapIDToKey[nID] = keyQt;

	if( action.objSlotPtr() && action.methodSlotPtr() ) {
#ifdef TQ_WS_WIN /** @todo TEMP: new implementation (commit #424926) didn't work */
		((TQAccel*)m_pAccel)->connectItem( nID, action.objSlotPtr(), action.methodSlotPtr() );
#else
		((TQAccel*)m_pAccel)->connectItem( nID, this, TQ_SLOT(slotKeyPressed(int)));
#endif
		if( !action.isEnabled() )
			((TQAccel*)m_pAccel)->setItemEnabled( nID, false );
	}

	kdDebug(125) << "TDEAccelPrivate::connectKey( \"" << action.name() << "\", " << key.key().toStringInternal() << " = 0x" << TQString::number(keyQt,16) << " ): id = " << nID << " m_pObjSlot = " << action.objSlotPtr() << endl;
	//kdDebug(125) << "m_pAccel = " << m_pAccel << endl;
	return nID != 0;
}

bool TDEAccelPrivate::connectKey( const KKeyServer::Key& key )
{
	uint keyQt = key.keyCodeQt();
	int nID = ((TQAccel*)m_pAccel)->insertItem( keyQt );

	m_mapIDToKey[nID] = keyQt;

	kdDebug(125) << "TDEAccelPrivate::connectKey( " << key.key().toStringInternal() << " = 0x" << TQString::number(keyQt,16) << " ): id = " << nID << endl;
	return nID != 0;
}

bool TDEAccelPrivate::disconnectKey( TDEAccelAction& action, const KKeyServer::Key& key )
{
	int keyQt = key.keyCodeQt();
	TQMap<int, int>::iterator it = m_mapIDToKey.begin();
	for( ; it != m_mapIDToKey.end(); ++it ) {
		//kdDebug(125) << "m_mapIDToKey[" << it.key() << "] = " << TQString::number(*it,16) << " == " << TQString::number(keyQt,16) << endl;
		if( *it == keyQt ) {
			int nID = it.key();
			kdDebug(125) << "TDEAccelPrivate::disconnectKey( \"" << action.name() << "\", 0x" << TQString::number(keyQt,16) << " ) : id = " << nID << " m_pObjSlot = " << action.objSlotPtr() << endl;
			((TQAccel*)m_pAccel)->removeItem( nID );
			m_mapIDToAction.remove( nID );
			m_mapIDToKey.remove( it );
			return true;
		}
	}
	//kdWarning(125) << kdBacktrace() << endl;
	kdWarning(125) << "Didn't find key in m_mapIDToKey." << endl;
	return false;
}

bool TDEAccelPrivate::disconnectKey( const KKeyServer::Key& key )
{
	int keyQt = key.keyCodeQt();
	kdDebug(125) << "TDEAccelPrivate::disconnectKey( 0x" << TQString::number(keyQt,16) << " )" << endl;
	TQMap<int, int>::iterator it = m_mapIDToKey.begin();
	for( ; it != m_mapIDToKey.end(); ++it ) {
		if( *it == keyQt ) {
			((TQAccel*)m_pAccel)->removeItem( it.key() );
			m_mapIDToKey.remove( it );
			return true;
		}
	}
	//kdWarning(125) << kdBacktrace() << endl;
	kdWarning(125) << "Didn't find key in m_mapIDTokey." << endl;
	return false;
}

void TDEAccelPrivate::slotKeyPressed( int id )
{
	kdDebug(125) << "TDEAccelPrivate::slotKeyPressed( " << id << " )" << endl;

	if( m_mapIDToKey.contains( id ) ) {
		KKey key = m_mapIDToKey[id];
		KKeySequence seq( key );
		TQPopupMenu* pMenu = createPopupMenu( m_pWatch, seq );

		// If there was only one action mapped to this key,
		//  and that action is not a multi-key shortcut,
		//  then activated it without popping up the menu.
		// This is needed for when there are multiple actions
		//  with the same shortcut where all but one is disabled.
		// pMenu->count() also counts the menu title, so one shortcut will give count = 2.
		if( pMenu->count() == 2 && pMenu->accel(1).isEmpty() ) {
			int iAction = pMenu->idAt(1);
			slotMenuActivated( iAction );
		} else {
			connect( pMenu, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotMenuActivated(int)) );
			pMenu->exec( m_pWatch->mapToGlobal( TQPoint( 0, 0 ) ) );
			disconnect( pMenu, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotMenuActivated(int)) );
		}
		delete pMenu;
	}
}

void TDEAccelPrivate::slotShowMenu()
{
}

void TDEAccelPrivate::slotMenuActivated( int iAction )
{
	kdDebug(125) << "TDEAccelPrivate::slotMenuActivated( " << iAction << " )" << endl;
	TDEAccelAction* pAction = actions().actionPtr( iAction );
#ifdef TQ_WS_WIN /** @todo TEMP: new implementation (commit #424926) didn't work */
	if( pAction ) {
		connect( this, TQ_SIGNAL(menuItemActivated()), pAction->objSlotPtr(), pAction->methodSlotPtr() );
		emit menuItemActivated();
		disconnect( this, TQ_SIGNAL(menuItemActivated()), pAction->objSlotPtr(), pAction->methodSlotPtr() );
	}
#else
	emitActivatedSignal( pAction );
#endif
}

bool TDEAccelPrivate::eventFilter( TQObject* /*pWatched*/, TQEvent* pEvent )
{
	if( pEvent->type() == TQEvent::AccelOverride && m_bEnabled ) {
		TQKeyEvent* pKeyEvent = (TQKeyEvent*) pEvent;
		KKey key( pKeyEvent );
		kdDebug(125) << "TDEAccelPrivate::eventFilter( AccelOverride ): this = " << this << ", key = " << key.toStringInternal() << endl;
		int keyCodeQt = key.keyCodeQt();
		TQMap<int, int>::iterator it = m_mapIDToKey.begin();
		for( ; it != m_mapIDToKey.end(); ++it ) {
			if( (*it) == keyCodeQt ) {
				int nID = it.key();
				kdDebug(125) << "shortcut found!" << endl;
				if( m_mapIDToAction.contains( nID ) ) {
					// TODO: reduce duplication between here and slotMenuActivated
					TDEAccelAction* pAction = m_mapIDToAction[nID];
					if( !pAction->isEnabled() )
						continue;
#ifdef TQ_WS_WIN /** @todo TEMP: new implementation (commit #424926) didn't work */
					TQGuardedPtr<TDEAccelPrivate> me = this;
					connect( this, TQ_SIGNAL(menuItemActivated()), pAction->objSlotPtr(), pAction->methodSlotPtr() );
					emit menuItemActivated();
					if (me) {
						disconnect( me, TQ_SIGNAL(menuItemActivated()), pAction->objSlotPtr(), pAction->methodSlotPtr() );
					}
#else
					emitActivatedSignal( pAction );
#endif
				} else
					slotKeyPressed( nID );

				pKeyEvent->accept();
				TDEAccelEventHandler::accelActivated( true );
				return true;
			}
		}
	}
	return false;
}

#ifndef TQ_WS_WIN /** @todo TEMP: new implementation (commit #424926) didn't work */
void TDEAccelPrivate::emitActivatedSignal( TDEAccelAction* pAction )
{
	if( pAction ) {
		TQGuardedPtr<TDEAccelPrivate> me = this;
		TQRegExp reg( "([ ]*TDEAccelAction.*)" );
		if( reg.search( pAction->methodSlotPtr()) >= 0 ) {
			connect( this, TQ_SIGNAL(menuItemActivated(TDEAccelAction*)),
				pAction->objSlotPtr(), pAction->methodSlotPtr() );
			emit menuItemActivated( pAction );
			if (me)
				disconnect( me, TQ_SIGNAL(menuItemActivated(TDEAccelAction*)),
					pAction->objSlotPtr(), pAction->methodSlotPtr() );
		} else {
			connect( this, TQ_SIGNAL(menuItemActivated()),
				pAction->objSlotPtr(), pAction->methodSlotPtr() );
			emit menuItemActivated();
			if (me)
				disconnect( me, TQ_SIGNAL(menuItemActivated()),
					pAction->objSlotPtr(), pAction->methodSlotPtr() );

		}
	}
}
#endif

//---------------------------------------------------------------------
// TDEAccel
//---------------------------------------------------------------------

TDEAccel::TDEAccel( TQWidget* pParent, const char* psName )
: TQAccel( pParent, (psName) ? psName : "TDEAccel-TQAccel" )
{
	kdDebug(125) << "TDEAccel( pParent = " << pParent << ", psName = " << psName << " ): this = " << this << endl;
	d = new TDEAccelPrivate( this, pParent );
}

TDEAccel::TDEAccel( TQWidget* watch, TQObject* pParent, const char* psName )
: TQAccel( watch, pParent, (psName) ? psName : "TDEAccel-TQAccel" )
{
	kdDebug(125) << "TDEAccel( watch = " << watch << ", pParent = " << pParent << ", psName = " << psName << " ): this = " << this << endl;
	if( !watch )
		kdDebug(125) << kdBacktrace() << endl;
	d = new TDEAccelPrivate( this, watch );
}

TDEAccel::~TDEAccel()
{
	kdDebug(125) << "~TDEAccel(): this = " << this << endl;
	delete d;
}

TDEAccelActions& TDEAccel::actions()             { return d->actions(); }
const TDEAccelActions& TDEAccel::actions() const { return d->actions(); }
bool TDEAccel::isEnabled()                     { return d->isEnabled(); }
void TDEAccel::setEnabled( bool bEnabled )     { d->setEnabled( bEnabled ); }
bool TDEAccel::setAutoUpdate( bool bAuto )     { return d->setAutoUpdate( bAuto ); }

TDEAccelAction* TDEAccel::insert( const TQString& sAction, const TQString& sLabel, const TQString& sWhatsThis,
		const TDEShortcut& cutDef,
		const TQObject* pObjSlot, const char* psMethodSlot,
		bool bConfigurable, bool bEnabled )
{
	return d->insert( sAction, sLabel, sWhatsThis,
		cutDef, cutDef,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
}

TDEAccelAction* TDEAccel::insert( const TQString& sAction, const TQString& sLabel, const TQString& sWhatsThis,
		const TDEShortcut& cutDef3, const TDEShortcut& cutDef4,
		const TQObject* pObjSlot, const char* psMethodSlot,
		bool bConfigurable, bool bEnabled )
{
	return d->insert( sAction, sLabel, sWhatsThis,
		cutDef3, cutDef4,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
}

TDEAccelAction* TDEAccel::insert( const char* psAction, const TDEShortcut& cutDef,
		const TQObject* pObjSlot, const char* psMethodSlot,
		bool bConfigurable, bool bEnabled )
{
	return d->insert( psAction, i18n(psAction), TQString::null,
		cutDef, cutDef,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
}

TDEAccelAction* TDEAccel::insert( TDEStdAccel::StdAccel id,
		const TQObject* pObjSlot, const char* psMethodSlot,
		bool bConfigurable, bool bEnabled )
{
	TQString sAction = TDEStdAccel::name( id );
	if( sAction.isEmpty() )
		return 0;

	TDEAccelAction* pAction = d->insert( sAction, TDEStdAccel::label( id ), TDEStdAccel::whatsThis( id ),
		TDEStdAccel::shortcutDefault3( id ), TDEStdAccel::shortcutDefault4( id ),
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
	if( pAction )
		pAction->setShortcut( TDEStdAccel::shortcut( id ) );

	return pAction;
}

bool TDEAccel::remove( const TQString& sAction )
	{ return d->removeAction( sAction ); }
bool TDEAccel::updateConnections()
	{ return d->updateConnections(); }

const TDEShortcut& TDEAccel::shortcut( const TQString& sAction ) const
{
	const TDEAccelAction* pAction = actions().actionPtr( sAction );
	return (pAction) ? pAction->shortcut() : TDEShortcut::null();
}

bool TDEAccel::setSlot( const TQString& sAction, const TQObject* pObjSlot, const char* psMethodSlot )
	{ return d->setActionSlot( sAction, pObjSlot, psMethodSlot ); }

bool TDEAccel::setEnabled( const TQString& sAction, bool bEnable )
	{ return d->setEnabled( sAction, bEnable ); }

bool TDEAccel::setShortcut( const TQString& sAction, const TDEShortcut& cut )
{
	kdDebug(125) << "TDEAccel::setShortcut( \"" << sAction << "\", " << cut.toStringInternal() << " )" << endl;
	TDEAccelAction* pAction = actions().actionPtr( sAction );
	if( pAction ) {
		if( pAction->shortcut() != cut )
			return d->setShortcut( sAction, cut );
		return true;
	}
	return false;
}

const TQString& TDEAccel::configGroup() const
	{ return d->configGroup(); }
// for tdegames/ksirtet
void TDEAccel::setConfigGroup( const TQString& s )
	{ d->setConfigGroup( s ); }

bool TDEAccel::readSettings( TDEConfigBase* pConfig )
{
	d->readSettings( pConfig );
	return true;
}

bool TDEAccel::writeSettings( TDEConfigBase* pConfig ) const
	{ d->writeSettings( pConfig ); return true; }

void TDEAccel::emitKeycodeChanged()
{
	kdDebug(125) << "TDEAccel::emitKeycodeChanged()" << endl;
	emit keycodeChanged();
}

#ifndef KDE_NO_COMPAT
//------------------------------------------------------------
// Obsolete methods -- for backward compatibility
//------------------------------------------------------------

bool TDEAccel::insertItem( const TQString& sLabel, const TQString& sAction,
		const char* cutsDef,
		int /*nIDMenu*/, TQPopupMenu *, bool bConfigurable )
{
	TDEShortcut cut( cutsDef );
	bool b = d->insert( sAction, sLabel, TQString::null,
		cut, cut,
		0, 0,
		bConfigurable ) != 0;
	return b;
}

bool TDEAccel::insertItem( const TQString& sLabel, const TQString& sAction,
		int key,
		int /*nIDMenu*/, TQPopupMenu*, bool bConfigurable )
{
	TDEShortcut cut;
	cut.init( TQKeySequence(key) );
	TDEAccelAction* pAction = d->insert( sAction, sLabel, TQString::null,
		cut, cut,
		0, 0,
		bConfigurable );
	return pAction != 0;
}

// Used in tdeutils/kjots
bool TDEAccel::insertStdItem( TDEStdAccel::StdAccel id, const TQString& sLabel )
{
	TDEAccelAction* pAction = d->insert( TDEStdAccel::name( id ), sLabel, TQString::null,
		TDEStdAccel::shortcutDefault3( id ), TDEStdAccel::shortcutDefault4( id ),
		0, 0 );
	if( pAction )
		pAction->setShortcut( TDEStdAccel::shortcut( id ) );

	return true;
}

bool TDEAccel::connectItem( const TQString& sAction, const TQObject* pObjSlot, const char* psMethodSlot, bool bActivate )
{
	kdDebug(125) << "TDEAccel::connectItem( " << sAction << ", " << pObjSlot << ", " << psMethodSlot << " )" << endl;
	if( bActivate == false )
		d->setActionEnabled( sAction, false );
	bool b = setSlot( sAction, pObjSlot, psMethodSlot );
	if( bActivate == true )
		d->setActionEnabled( sAction, true );
	return b;
}

bool TDEAccel::removeItem( const TQString& sAction )
	{ return d->removeAction( sAction ); }

bool TDEAccel::setItemEnabled( const TQString& sAction, bool bEnable )
	{ return setEnabled( sAction, bEnable ); }

void TDEAccel::changeMenuAccel( TQPopupMenu *menu, int id, const TQString& action )
{
	TDEAccelAction* pAction = actions().actionPtr( action );
	TQString s = menu->text( id );
	if( !pAction || s.isEmpty() )
		return;

	int i = s.find( '\t' );

	TQString k = pAction->shortcut().seq(0).toString();
	if( k.isEmpty() )
		return;

	if ( i >= 0 )
		s.replace( i+1, s.length()-i, k );
	else {
		s += '\t';
		s += k;
	}

	TQPixmap *pp = menu->pixmap(id);
	if( pp && !pp->isNull() )
		menu->changeItem( *pp, s, id );
	else
		menu->changeItem( s, id );
}

void TDEAccel::changeMenuAccel( TQPopupMenu *menu, int id, TDEStdAccel::StdAccel accel )
{
	changeMenuAccel( menu, id, TDEStdAccel::name( accel ) );
}

int TDEAccel::stringToKey( const TQString& sKey )
{
	return KKey( sKey ).keyCodeQt();
}

int TDEAccel::currentKey( const TQString& sAction ) const
{
	TDEAccelAction* pAction = d->actionPtr( sAction );
	if( pAction )
		return pAction->shortcut().keyCodeQt();
	return 0;
}

TQString TDEAccel::findKey( int key ) const
{
	TDEAccelAction* pAction = d->actionPtr( KKey(key) );
	if( pAction )
		return pAction->name();
	else
		return TQString::null;
}
#endif // !KDE_NO_COMPAT

void TDEAccel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "tdeaccel.moc"
#include "tdeaccelprivate.moc"
