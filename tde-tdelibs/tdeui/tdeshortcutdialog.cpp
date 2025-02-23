/* This file is part of the KDE libraries
    Copyright (C) 2002,2003 Ellis Whitehead <ellis@kde.org>

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

#include "tdeshortcutdialog.h"

#include <tqvariant.h>

#ifdef TQ_WS_X11
	#define XK_XKB_KEYS
	#define XK_MISCELLANY
	#include <X11/Xlib.h>	// For x11Event()
	#include <X11/keysymdef.h> // For XK_...

	#ifdef KeyPress
		const int XKeyPress = KeyPress;
		const int XKeyRelease = KeyRelease;
		const int XFocusOut = FocusOut;
		const int XFocusIn = FocusIn;
		#undef KeyRelease
		#undef KeyPress
		#undef FocusOut
		#undef FocusIn
	#endif
#elif defined(TQ_WS_WIN)
# include <kkeyserver.h>
#endif

#include <tdeshortcutdialog_simple.h>
#include <tdeshortcutdialog_advanced.h>

#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqframe.h>
#include <tqlayout.h>
#include <tqradiobutton.h>
#include <tqtimer.h>
#include <tqvbox.h>

#include <tdeapplication.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <kiconloader.h>
#include <kkeynative.h>
#include <tdelocale.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>

bool TDEShortcutDialog::s_showMore = false;

TDEShortcutDialog::TDEShortcutDialog( const TDEShortcut& shortcut, bool bQtShortcut, TQWidget* parent, const char* name )
: KDialogBase( parent, name, true, i18n("Configure Shortcut"),
               KDialogBase::Details|KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Cancel, true )
{
        setButtonText(Details, i18n("Advanced"));
        m_stack = new TQVBox(this);
        m_stack->setMinimumWidth(360);
        m_stack->setSpacing(0);
        m_stack->setMargin(0);
        setMainWidget(m_stack);
        
        m_simple = new TDEShortcutDialogSimple(m_stack);

        m_adv = new TDEShortcutDialogAdvanced(m_stack);
        m_adv->hide();
        
	m_bQtShortcut = bQtShortcut;

	m_iSeq = 0;
	m_iKey = 0;
	m_ptxtCurrent = 0;
	m_bRecording = false;
	m_mod = 0;

	m_simple->m_btnClearShortcut->setPixmap( SmallIcon( "locationbar_erase" ) );
	m_adv->m_btnClearPrimary->setPixmap( SmallIcon( "locationbar_erase" ) );
	m_adv->m_btnClearAlternate->setPixmap( SmallIcon( "locationbar_erase" ) );
	connect(m_simple->m_btnClearShortcut, TQ_SIGNAL(clicked()),
	        this, TQ_SLOT(slotClearShortcut()));
	connect(m_adv->m_btnClearPrimary, TQ_SIGNAL(clicked()),
	        this, TQ_SLOT(slotClearPrimary()));
	connect(m_adv->m_btnClearAlternate, TQ_SIGNAL(clicked()),
	        this, TQ_SLOT(slotClearAlternate()));

	connect(m_adv->m_txtPrimary, TQ_SIGNAL(clicked()),
		m_adv->m_btnPrimary, TQ_SLOT(animateClick()));
	connect(m_adv->m_txtAlternate, TQ_SIGNAL(clicked()),
		m_adv->m_btnAlternate, TQ_SLOT(animateClick()));
	connect(m_adv->m_btnPrimary, TQ_SIGNAL(clicked()),
		this, TQ_SLOT(slotSelectPrimary()));
	connect(m_adv->m_btnAlternate, TQ_SIGNAL(clicked()),
		this, TQ_SLOT(slotSelectAlternate()));

	KGuiItem ok = KStdGuiItem::ok();
	ok.setText( i18n( "OK" ) );
	setButtonOK( ok );

	KGuiItem cancel = KStdGuiItem::cancel();
	cancel.setText( i18n( "Cancel" ) );
	setButtonCancel( cancel );

	setShortcut( shortcut );
	resize( 0, 0 );

	s_showMore = TDEConfigGroup(TDEGlobal::config(), "General").readBoolEntry("ShowAlternativeShortcutConfig", s_showMore);
	updateDetails();

	#ifdef TQ_WS_X11
	kapp->installX11EventFilter( this );	// Allow button to capture X Key Events.
	#endif
}

TDEShortcutDialog::~TDEShortcutDialog()
{
	TDEConfigGroup group(TDEGlobal::config(), "General");
	group.writeEntry("ShowAlternativeShortcutConfig", s_showMore);
}

void TDEShortcutDialog::setShortcut( const TDEShortcut & shortcut )
{
	m_shortcut = shortcut;
	updateShortcutDisplay();
}

void TDEShortcutDialog::updateShortcutDisplay()
{
	TQString s[2] = { m_shortcut.seq(0).toString(), m_shortcut.seq(1).toString() };

	if( m_bRecording ) {
		m_ptxtCurrent->setDefault( true );
		m_ptxtCurrent->setFocus();

		// Display modifiers for the first key in the KKeySequence
		if( m_iKey == 0 ) {
			if( m_mod ) {
				TQString keyModStr;
				if( m_mod & KKey::WIN )   keyModStr += KKey::modFlagLabel(KKey::WIN) + "+";
				if( m_mod & KKey::ALT )   keyModStr += KKey::modFlagLabel(KKey::ALT) + "+";
				if( m_mod & KKey::CTRL )  keyModStr += KKey::modFlagLabel(KKey::CTRL) + "+";
				if( m_mod & KKey::SHIFT ) keyModStr += KKey::modFlagLabel(KKey::SHIFT) + "+";
				s[m_iSeq] = keyModStr;
	}
		}
		// When in the middle of entering multi-key shortcuts,
		//  add a "," to the end of the displayed shortcut.
		else
			s[m_iSeq] += ",";
	}
	else {
		m_adv->m_txtPrimary->setDefault( false );
		m_adv->m_txtAlternate->setDefault( false );
		this->setFocus();
	}
	
	s[0].replace('&', TQString::fromLatin1("&&"));
	s[1].replace('&', TQString::fromLatin1("&&"));

	m_simple->m_txtShortcut->setText( s[0] );
	m_adv->m_txtPrimary->setText( s[0] );
	m_adv->m_txtAlternate->setText( s[1] );

	// Determine the enable state of the 'Less' button
	bool bLessOk;
	// If there is no shortcut defined,
	if( m_shortcut.count() == 0 )
		bLessOk = true;
	// If there is a single shortcut defined, and it is not a multi-key shortcut,
	else if( m_shortcut.count() == 1 && m_shortcut.seq(0).count() <= 1 )
		bLessOk = true;
	// Otherwise, we have an alternate shortcut or multi-key shortcut(s).
	else
		bLessOk = false;
	enableButton(Details, bLessOk);
}

void TDEShortcutDialog::slotDetails()
{
	s_showMore = (m_adv->isHidden());
	updateDetails();
}

void TDEShortcutDialog::updateDetails()
{
	bool showAdvanced = s_showMore || (m_shortcut.count() > 1);
	setDetails(showAdvanced);
	m_bRecording = false;
	m_iSeq = 0;
	m_iKey = 0;

	if (showAdvanced)
	{
		m_simple->hide();
		m_adv->show();
		m_adv->m_btnPrimary->setChecked( true );
		slotSelectPrimary();
	}
	else
	{
		m_ptxtCurrent = m_simple->m_txtShortcut;
		m_adv->hide();
		m_simple->show();
		m_simple->m_txtShortcut->setDefault( true );
		m_simple->m_txtShortcut->setFocus();
		m_adv->m_btnMultiKey->setChecked( false );
	}
	kapp->processEvents();
	adjustSize();
}

void TDEShortcutDialog::slotSelectPrimary()
{
	m_bRecording = false;
	m_iSeq = 0;
	m_iKey = 0;
	m_ptxtCurrent = m_adv->m_txtPrimary;
	m_ptxtCurrent->setDefault(true);
	m_ptxtCurrent->setFocus();
	updateShortcutDisplay();
}

void TDEShortcutDialog::slotSelectAlternate()
{
	m_bRecording = false;
	m_iSeq = 1;
	m_iKey = 0;
	m_ptxtCurrent = m_adv->m_txtAlternate;
	m_ptxtCurrent->setDefault(true);
	m_ptxtCurrent->setFocus();
	updateShortcutDisplay();
}

void TDEShortcutDialog::slotClearShortcut()
{
	m_shortcut.setSeq( 0, KKeySequence() );
	updateShortcutDisplay();
}

void TDEShortcutDialog::slotClearPrimary()
{
	m_shortcut.setSeq( 0, KKeySequence() );
	m_adv->m_btnPrimary->setChecked( true );
	slotSelectPrimary();
}

void TDEShortcutDialog::slotClearAlternate()
{
	if( m_shortcut.count() == 2 )
		m_shortcut.init( m_shortcut.seq(0) );
	m_adv->m_btnAlternate->setChecked( true );
	slotSelectAlternate();
}

void TDEShortcutDialog::slotMultiKeyMode( bool bOn )
{
	// If turning off multi-key mode during a recording,
	if( !bOn && m_bRecording ) {
		m_bRecording = false;
	m_iKey = 0;
		updateShortcutDisplay();
	}
}

#ifdef TQ_WS_X11
/* we don't use the generic Qt code on X11 because it allows us 
 to grab the keyboard so that all keypresses are seen
 */
bool TDEShortcutDialog::x11Event( XEvent *pEvent )
{
	switch( pEvent->type ) {
		case XKeyPress:
			x11KeyPressEvent( pEvent );
			return true;
		case XKeyRelease:
			x11KeyReleaseEvent( pEvent );
				return true;
		case XFocusIn:
			{
				XFocusInEvent *fie = (XFocusInEvent*)pEvent;
				if (fie->mode != NotifyGrab && fie->mode != NotifyUngrab) {
					grabKeyboard();
				}
			}
			break;
		case XFocusOut:
			{
				XFocusOutEvent *foe = (XFocusOutEvent*)pEvent;
				if (foe->mode != NotifyGrab && foe->mode != NotifyUngrab) {
					releaseKeyboard();
				}
			}
			break;
		default:
			//kdDebug(125) << "x11Event->type = " << pEvent->type << endl;
			break;
	}
	return KDialogBase::x11Event( pEvent );
}

static uint getModsFromModX( uint keyModX )
{
	uint mod = 0;
	if( keyModX & KKeyNative::modX(KKey::SHIFT) ) mod += KKey::SHIFT;
	if( keyModX & KKeyNative::modX(KKey::CTRL) )  mod += KKey::CTRL;
	if( keyModX & KKeyNative::modX(KKey::ALT) )   mod += KKey::ALT;
	if( keyModX & KKeyNative::modX(KKey::WIN) )   mod += KKey::WIN;
	return mod;
}

static bool convertSymXToMod( uint keySymX, uint* pmod )
{
	switch( keySymX ) {
		// Don't allow setting a modifier key as an accelerator.
		// Also, don't release the focus yet.  We'll wait until
		//  we get a 'normal' key.
		case XK_Shift_L:   case XK_Shift_R:   *pmod = KKey::SHIFT; break;
		case XK_Control_L: case XK_Control_R: *pmod = KKey::CTRL; break;
		case XK_Alt_L:     case XK_Alt_R:     *pmod = KKey::ALT; break;
		// FIXME: check whether the Meta or Super key are for the Win modifier
		case XK_Meta_L:    case XK_Meta_R:
		case XK_Super_L:   case XK_Super_R:   *pmod = KKey::WIN; break;
		case XK_Hyper_L:   case XK_Hyper_R:
		case XK_Mode_switch:
		case XK_Num_Lock:
		case XK_Caps_Lock:
			break;
		default:
			return false;
				}
	return true;
}

void TDEShortcutDialog::x11KeyPressEvent( XEvent* pEvent )
{
	KKeyNative keyNative( pEvent );
	uint keyModX = keyNative.mod();
	uint keySymX = keyNative.sym();

	m_mod = getModsFromModX( keyModX );

	if( keySymX ) {
		m_bRecording = true;

		uint mod = 0;
		if( convertSymXToMod( keySymX, &mod ) ) {
			if( mod )
				m_mod |= mod;
		}
		else
			keyPressed( KKey(keyNative) );
	}
	updateShortcutDisplay();
}

void TDEShortcutDialog::x11KeyReleaseEvent( XEvent* pEvent )
{
	// We're only interested in the release of modifier keys,
	//  and then only when it's for the first key in a sequence.
	if( m_bRecording && m_iKey == 0 ) {
		KKeyNative keyNative( pEvent );
		uint keyModX = keyNative.mod();
		uint keySymX = keyNative.sym();

		m_mod = getModsFromModX( keyModX );

		uint mod = 0;
		if( convertSymXToMod( keySymX, &mod ) && mod ) {
			m_mod &= ~mod;
			if( !m_mod )
				m_bRecording = false;
		}
		updateShortcutDisplay();
	}
}
#elif defined(TQ_WS_WIN)
void TDEShortcutDialog::keyPressEvent( TQKeyEvent * e )
{
	kdDebug() << e->text() << " " << (int)e->text()[0].latin1()<<  " " << (int)e->ascii() << endl;
	//if key is a letter, it must be stored as lowercase
	int keyQt = TQChar( e->key() & 0xff ).isLetter() ? 
		(TQChar( e->key() & 0xff ).lower().latin1() | (e->key() & 0xffff00) )
		: e->key();
	int modQt = KKeyServer::qtButtonStateToMod( e->state() );
	KKeyNative keyNative( KKey(keyQt, modQt) );
	m_mod = keyNative.mod();
	uint keySym = keyNative.sym();

	switch( keySym ) {
		case Key_Shift: 
			m_mod |= KKey::SHIFT;
			m_bRecording = true;
			break;
		case Key_Control:
			m_mod |= KKey::CTRL;
			m_bRecording = true;
			break;
		case Key_Alt:
			m_mod |= KKey::ALT;
			m_bRecording = true;
			break;
		case Key_Menu:
		case Key_Meta: //unused
			break;
		default:
			if( keyNative.sym() == Key_Return && m_iKey > 0 ) {
				accept();
				return;
			}
			//accept
			if (keyNative.sym()) {
				KKey key = keyNative;
				key.simplify();
				KKeySequence seq;
				if( m_iKey == 0 )
					seq = key;
				else {
					seq = m_shortcut.seq( m_iSeq );
					seq.setKey( m_iKey, key );
				}
				m_shortcut.setSeq( m_iSeq, seq );

				if(m_adv->m_btnMultiKey->isChecked())
					m_iKey++;

				m_bRecording = true;

				updateShortcutDisplay();

				if( !m_adv->m_btnMultiKey->isChecked() )
					TQTimer::singleShot(500, this, TQ_SLOT(accept()));
			}
			return;
	}

	// If we are editing the first key in the sequence,
	//  display modifier keys which are held down
	if( m_iKey == 0 ) {
		updateShortcutDisplay();
	}
}

bool TDEShortcutDialog::event ( TQEvent * e )
{
	if (e->type()==TQEvent::KeyRelease) {
		int modQt = KKeyServer::qtButtonStateToMod( static_cast<TQKeyEvent*>(e)->state() );
		KKeyNative keyNative( KKey(static_cast<TQKeyEvent*>(e)->key(), modQt) );
		uint keySym = keyNative.sym();

		bool change = true;
		switch( keySym ) {
		case Key_Shift: 
			if (m_mod & KKey::SHIFT)
				m_mod ^= KKey::SHIFT;
			break;
		case Key_Control:
			if (m_mod & KKey::CTRL)
				m_mod ^= KKey::CTRL;
			break;
		case Key_Alt:
			if (m_mod & KKey::ALT)
				m_mod ^= KKey::ALT;
			break;
		default:
			change = false;
		}
		if (change)
			updateShortcutDisplay();
	}
	return KDialogBase::event(e);
}
#endif

void TDEShortcutDialog::keyPressed( KKey key )
{
	kdDebug(125) << "keyPressed: " << key.toString() << endl;

	key.simplify();
	if( m_bQtShortcut ) {
		key = key.keyCodeQt();
		if( key.isNull() ) {
			// TODO: message box about key not able to be used as application shortcut
		}
	}

	KKeySequence seq;
	if( m_iKey == 0 )
		seq = key;
	else {
		// Remove modifiers
		key.init( key.sym(), 0 );
		seq = m_shortcut.seq( m_iSeq );
		seq.setKey( m_iKey, key );
	}

	m_shortcut.setSeq( m_iSeq, seq );

	m_mod = 0;
	if( m_adv->m_btnMultiKey->isChecked() && m_iKey < KKeySequence::MAX_KEYS - 1 )
		m_iKey++;
	else {
		m_iKey = 0;
		m_bRecording = false;
	}

	updateShortcutDisplay();

	if( !m_adv->m_btnMultiKey->isChecked() )
		TQTimer::singleShot(500, this, TQ_SLOT(accept()));
}

#include "tdeshortcutdialog.moc"
