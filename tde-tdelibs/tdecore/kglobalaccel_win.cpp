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

#include "config.h"

#include <tqwindowdefs.h>
#ifdef TQ_WS_WIN

#include "kglobalaccel_win.h"
#include "kglobalaccel.h"
#include "kkeyserver_x11.h"

#include <tqpopupmenu.h>
#include <tqregexp.h>
#include <tqwidget.h>
#include <tqmetaobject.h>
#include <private/qucomextra_p.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <kkeynative.h>

//----------------------------------------------------

static TQValueList< TDEGlobalAccelPrivate* >* all_accels = 0;

TDEGlobalAccelPrivate::TDEGlobalAccelPrivate()
: TDEAccelBase( TDEAccelBase::NATIVE_KEYS )
, m_blocked( false )
, m_blockingDisabled( false )
{
        if( all_accels == NULL )
            all_accels = new TQValueList< TDEGlobalAccelPrivate* >;
        all_accels->append( this );
	m_sConfigGroup = "Global Shortcuts";
//	kapp->installX11EventFilter( this );
}

TDEGlobalAccelPrivate::~TDEGlobalAccelPrivate()
{
	// TODO: Need to release all grabbed keys if the main window is not shutting down.
	//for( CodeModMap::ConstIterator it = m_rgCodeModToAction.begin(); it != m_rgCodeModToAction.end(); ++it ) {
	//	const CodeMod& codemod = it.key();
	//}
        all_accels->remove( this );
        if( all_accels->count() == 0 ) {
            delete all_accels;
            all_accels = NULL;
        }
}

void TDEGlobalAccelPrivate::setEnabled( bool bEnable )
{
	m_bEnabled = bEnable;
	//updateConnections();
}

void TDEGlobalAccelPrivate::blockShortcuts( bool block )
{
        if( all_accels == NULL )
            return;
        for( TQValueList< TDEGlobalAccelPrivate* >::ConstIterator it = all_accels->begin();
             it != all_accels->end();
             ++it ) {
            if( (*it)->m_blockingDisabled )
                continue;
            (*it)->m_blocked = block;
            (*it)->updateConnections();
        }
}

void TDEGlobalAccelPrivate::disableBlocking( bool block )
{
        m_blockingDisabled = block;
}

bool TDEGlobalAccelPrivate::isEnabledInternal() const
{
        return TDEAccelBase::isEnabled() && !m_blocked;
}

bool TDEGlobalAccelPrivate::emitSignal( Signal )
{
	return false;
}

bool TDEGlobalAccelPrivate::connectKey( TDEAccelAction& action, const KKeyServer::Key& key )
	{ return grabKey( key, true, &action ); }
bool TDEGlobalAccelPrivate::connectKey( const KKeyServer::Key& key )
	{ return grabKey( key, true, 0 ); }
bool TDEGlobalAccelPrivate::disconnectKey( TDEAccelAction& action, const KKeyServer::Key& key )
	{ return grabKey( key, false, &action ); }
bool TDEGlobalAccelPrivate::disconnectKey( const KKeyServer::Key& key )
	{ return grabKey( key, false, 0 ); }

bool TDEGlobalAccelPrivate::grabKey( const KKeyServer::Key& key, bool bGrab, TDEAccelAction* pAction )
{
	/*
	if( !key.code() ) {
		kdWarning(125) << "TDEGlobalAccelPrivate::grabKey( " << key.key().toStringInternal() << ", " << bGrab << ", \"" << (pAction ? pAction->name().latin1() : "(null)") << "\" ): Tried to grab key with null code." << endl;
		return false;
	}

	// Make sure that grab masks have been initialized.
	if( g_keyModMaskXOnOrOff == 0 )
		calculateGrabMasks();

	uchar keyCodeX = key.code();
	uint keyModX = key.mod() & g_keyModMaskXAccel; // Get rid of any non-relevant bits in mod
	// HACK: make Alt+Print work
	if( key.sym() == XK_Sys_Req ) {
	    keyModX |= KKeyServer::modXAlt();
	    keyCodeX = 111;
	}

	kdDebug(125) << TQString( "grabKey( key: '%1', bGrab: %2 ): keyCodeX: %3 keyModX: %4\n" )
		.arg( key.key().toStringInternal() ).arg( bGrab )
		.arg( keyCodeX, 0, 16 ).arg( keyModX, 0, 16 );
	if( !keyCodeX )
		return false;

	// We'll have to grab 8 key modifier combinations in order to cover all
	//  combinations of CapsLock, NumLock, ScrollLock.
	// Does anyone with more X-savvy know how to set a mask on tqt_xrootwin so that
	//  the irrelevant bits are always ignored and we can just make one XGrabKey
	//  call per accelerator? -- ellis
#ifndef NDEBUG
	TQString sDebug = TQString("\tcode: 0x%1 state: 0x%2 | ").arg(keyCodeX,0,16).arg(keyModX,0,16);
#endif
	uint keyModMaskX = ~g_keyModMaskXOnOrOff;
	for( uint irrelevantBitsMask = 0; irrelevantBitsMask <= 0xff; irrelevantBitsMask++ ) {
		if( (irrelevantBitsMask & keyModMaskX) == 0 ) {
#ifndef NDEBUG
			sDebug += TQString("0x%3, ").arg(irrelevantBitsMask, 0, 16);
#endif
			if( bGrab )
				XGrabKey( tqt_xdisplay(), keyCodeX, keyModX | irrelevantBitsMask,
					tqt_xrootwin(), True, GrabModeAsync, GrabModeSync );
			else
				XUngrabKey( tqt_xdisplay(), keyCodeX, keyModX | irrelevantBitsMask, tqt_xrootwin() );
		}
	}
#ifndef NDEBUG
	kdDebug(125) << sDebug << endl;
#endif

        bool failed = false;
        if( bGrab ) {
#ifdef TQ_WS_X11
        	failed = handler.error( true ); // sync now
#endif
        	// If grab failed, then ungrab any grabs that could possibly succeed
		if( failed ) {
			kdDebug(125) << "grab failed!\n";
			for( uint m = 0; m <= 0xff; m++ ) {
				if( m & keyModMaskX == 0 )
					XUngrabKey( tqt_xdisplay(), keyCodeX, keyModX | m, tqt_xrootwin() );
				}
                }
	}
        if( !failed )
        {
		CodeMod codemod;
		codemod.code = keyCodeX;
		codemod.mod = keyModX;
		if( key.mod() & KKeyServer::MODE_SWITCH )
			codemod.mod |= KKeyServer::MODE_SWITCH;

		if( bGrab )
			m_rgCodeModToAction.insert( codemod, pAction );
		else
			m_rgCodeModToAction.remove( codemod );
	}
	return !failed;*/
	return false;
}

/*bool TDEGlobalAccelPrivate::x11Event( XEvent* pEvent )
{
	//kdDebug(125) << "x11EventFilter( type = " << pEvent->type << " )" << endl;
	switch( pEvent->type ) {
	 case MappingNotify:
	        XRefreshKeyboardMapping( &pEvent->xmapping );
		x11MappingNotify();
		return false;
	 case XKeyPress:
		if( x11KeyPress( pEvent ) )
			return true;
	 default:
		return TQWidget::x11Event( pEvent );
	}
}

void TDEGlobalAccelPrivate::x11MappingNotify()
{
	kdDebug(125) << "TDEGlobalAccelPrivate::x11MappingNotify()" << endl;
	if( m_bEnabled ) {
		// Maybe the X modifier map has been changed.
		KKeyServer::initializeMods();
		calculateGrabMasks();
		// Do new XGrabKey()s.
		updateConnections();
	}
}

bool TDEGlobalAccelPrivate::x11KeyPress( const XEvent *pEvent )
{
	// do not change this line unless you really really know what you are doing (Matthias)
	if ( !TQWidget::keyboardGrabber() && !TQApplication::activePopupWidget() ) {
		XUngrabKeyboard( tqt_xdisplay(), pEvent->xkey.time );
                XFlush( tqt_xdisplay()); // avoid X(?) bug
        }

	if( !m_bEnabled )
		return false;

	CodeMod codemod;
	codemod.code = pEvent->xkey.keycode;
	codemod.mod = pEvent->xkey.state & (g_keyModMaskXAccel | KKeyServer::MODE_SWITCH);

	// If numlock is active and a keypad key is pressed, XOR the SHIFT state.
	//  e.g., KP_4 => Shift+KP_Left, and Shift+KP_4 => KP_Left.
	if( pEvent->xkey.state & KKeyServer::modXNumLock() ) {
		// TODO: what's the xor operator in c++?
		uint sym = XkbKeycodeToKeysym( tqt_xdisplay(), codemod.code, 0, 0 );
		// If this is a keypad key,
		if( sym >= XK_KP_Space && sym <= XK_KP_9 ) {
			switch( sym ) {
				// Leave the following keys unaltered
				// FIXME: The proper solution is to see which keysyms don't change when shifted.
				case XK_KP_Multiply:
				case XK_KP_Add:
				case XK_KP_Subtract:
				case XK_KP_Divide:
					break;
				default:
					if( codemod.mod & KKeyServer::modXShift() )
						codemod.mod &= ~KKeyServer::modXShift();
					else
						codemod.mod |= KKeyServer::modXShift();
			}
		}
	}

	KKeyNative keyNative( pEvent );
	KKey key = keyNative;

	kdDebug(125) << "x11KeyPress: seek " << key.toStringInternal()
		<< TQString( " keyCodeX: %1 state: %2 keyModX: %3" )
			.arg( codemod.code, 0, 16 ).arg( pEvent->xkey.state, 0, 16 ).arg( codemod.mod, 0, 16 ) << endl;

	// Search for which accelerator activated this event:
	if( !m_rgCodeModToAction.contains( codemod ) ) {
#ifndef NDEBUG
		for( CodeModMap::ConstIterator it = m_rgCodeModToAction.begin(); it != m_rgCodeModToAction.end(); ++it ) {
			TDEAccelAction* pAction = *it;
			kdDebug(125) << "\tcode: " << TQString::number(it.key().code, 16) << " mod: " << TQString::number(it.key().mod, 16)
				<< (pAction ? TQString(" name: \"%1\" shortcut: %2").arg(pAction->name()).arg(pAction->shortcut().toStringInternal()) : TQString::null)
				<< endl;
		}
#endif
		return false;
	}
	TDEAccelAction* pAction = m_rgCodeModToAction[codemod];

	if( !pAction ) {
                static bool recursion_block = false;
                if( !recursion_block ) {
                        recursion_block = true;
		        TQPopupMenu* pMenu = createPopupMenu( 0, KKeySequence(key) );
		        connect( pMenu, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotActivated(int)) );
		        pMenu->exec( TQPoint( 0, 0 ) );
		        disconnect( pMenu, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotActivated(int)));
		        delete pMenu;
                        recursion_block = false;
                }
	} else if( !pAction->objSlotPtr() || !pAction->isEnabled() )
		return false;
	else
		activate( pAction, KKeySequence(key) );

	return true;
}*/

void TDEGlobalAccelPrivate::activate( TDEAccelAction* pAction, const KKeySequence& seq )
{
	kdDebug(125) << "TDEGlobalAccelPrivate::activate( \"" << pAction->name() << "\" ) " << endl;

	TQRegExp rexPassIndex( "([ ]*int[ ]*)" );
	TQRegExp rexPassInfo( " TQString" );
	TQRegExp rexIndex( " ([0-9]+)$" );

	// If the slot to be called accepts an integer index
	//  and an index is present at the end of the action's name,
	//  then send the slot the given index #.
	if( rexPassIndex.search( pAction->methodSlotPtr() ) >= 0 && rexIndex.search( pAction->name() ) >= 0 ) {
		int n = rexIndex.cap(1).toInt();
		kdDebug(125) << "Calling " << pAction->methodSlotPtr() << " int = " << n << endl;
                int slot_id = pAction->objSlotPtr()->metaObject()->findSlot( normalizeSignalSlot( pAction->methodSlotPtr() ).data() + 1, true );
                if( slot_id >= 0 ) {
                    QUObject o[2];
                    static_QUType_int.set(o+1,n);
                    const_cast< TQObject* >( pAction->objSlotPtr())->tqt_invoke( slot_id, o );
                }
	} else if( rexPassInfo.search( pAction->methodSlotPtr() ) ) {
                int slot_id = pAction->objSlotPtr()->metaObject()->findSlot( normalizeSignalSlot( pAction->methodSlotPtr() ).data() + 1, true );
                if( slot_id >= 0 ) {
                    QUObject o[4];
                    static_QUType_QString.set(o+1,pAction->name());
                    static_QUType_QString.set(o+2,pAction->label());
                    static_QUType_ptr.set(o+3,&seq);
                    const_cast< TQObject* >( pAction->objSlotPtr())->tqt_invoke( slot_id, o );
                }
	} else {
                int slot_id = pAction->objSlotPtr()->metaObject()->findSlot( normalizeSignalSlot( pAction->methodSlotPtr() ).data() + 1, true );
                if( slot_id >= 0 )
                    const_cast< TQObject* >( pAction->objSlotPtr())->tqt_invoke( slot_id, 0 );
	}
}

void TDEGlobalAccelPrivate::slotActivated( int iAction )
{
	TDEAccelAction* pAction = actions().actionPtr( iAction );
	if( pAction )
		activate( pAction, KKeySequence() );
}

#include "kglobalaccel_win.moc"

#endif // !TQ_WS_WIN
