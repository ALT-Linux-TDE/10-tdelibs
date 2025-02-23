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

#ifndef _TDESHORTCUTDIALOG_H_
#define _TDESHORTCUTDIALOG_H_

#include "kdialogbase.h"
#include "tdeshortcut.h"

class TQVBox;
class KPushButton;
class TDEShortcutDialogSimple;
class TDEShortcutDialogAdvanced;

/**
 * @short Dialog for configuring a shortcut.
 *
 * This dialog allows configuring a single TDEShortcut. KKeyDialog
 * should be usually used instead.
 *
 * @internal
 * @see KKeyDialog
 * @since 3.4
 */
class TDEUI_EXPORT TDEShortcutDialog : public KDialogBase
{
	TQ_OBJECT
public:
	TDEShortcutDialog( const TDEShortcut& shortcut, bool bQtShortcut, TQWidget* parent = 0, const char* name = 0 );
	~TDEShortcutDialog();

	void setShortcut( const TDEShortcut & shortcut );
	const TDEShortcut& shortcut() const { return m_shortcut; }

private:
	// true if qt shortcut, false if native shortcut
	bool m_bQtShortcut;

	TDEShortcut m_shortcut;
	KPushButton* m_ptxtCurrent;
	uint m_iSeq;
	uint m_iKey;
	bool m_bRecording;
	uint m_mod;
	TDEShortcutDialogSimple *m_simple;
	TDEShortcutDialogAdvanced *m_adv;
	TQVBox *m_stack;
	
	void updateShortcutDisplay();
	//void displayMods();
	void keyPressed( KKey key );
	void updateDetails();

	#ifdef TQ_WS_X11
	virtual bool x11Event( XEvent *pEvent );
	//void x11EventKeyPress( XEvent *pEvent );
	void x11KeyPressEvent( XEvent* pEvent );
	void x11KeyReleaseEvent( XEvent* pEvent );
	#endif
	#ifdef TQ_WS_WIN
	virtual void keyPressEvent( TQKeyEvent * e );
	virtual bool event(TQEvent * e);
	#endif

protected slots:
	void slotDetails();
	void slotSelectPrimary();
	void slotSelectAlternate();
	void slotClearShortcut();
	void slotClearPrimary();
	void slotClearAlternate();
	void slotMultiKeyMode( bool bOn );

private:
        class TDEShortcutDialogPrivate* d;
        static bool s_showMore;
};

#endif // _TDESHORTCUTDIALOG_H_
