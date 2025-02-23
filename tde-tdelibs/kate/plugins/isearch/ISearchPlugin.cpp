 /* This file is part of the KDE libraries
    Copyright (C) 2002 by John Firebaugh <jfirebaugh@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */

#include <tqlabel.h>
#include <tqregexp.h>
#include <tqstyle.h>
#include <tqpopupmenu.h>
#include <kgenericfactory.h>
#include <tdelocale.h>
#include <tdeaction.h>
#include <kcombobox.h>
#include <tdeconfig.h>
#include <kdebug.h>

#include "ISearchPlugin.h"
#include "ISearchPlugin.moc"

K_EXPORT_COMPONENT_FACTORY( tdetexteditor_isearch, KGenericFactory<ISearchPlugin>( "tdetexteditor_isearch" ) )

ISearchPluginView::ISearchPluginView( KTextEditor::View *view )
	: TQObject ( view ), KXMLGUIClient (view)
	, m_view( 0L )
	, m_doc( 0L )
	, m_searchIF( 0L )
	, m_cursorIF( 0L )
	, m_selectIF( 0L )
//	, m_toolBarAction( 0L )
	, m_searchForwardAction( 0L )
	, m_searchBackwardAction( 0L )
	, m_label( 0L )
	, m_combo( 0L )
	, m_lastString( "" )
	, m_searchBackward( false )
	, m_caseSensitive( false )
	, m_fromBeginning( false )
	, m_regExp( false )
	, m_autoWrap( false )
	, m_wrapped( false )
	, m_startLine( 0 )
	, m_startCol( 0 )
	, m_searchLine( 0 )
	, m_searchCol( 0 )
	, m_foundLine( 0 )
	, m_foundCol( 0 )
	, m_matchLen( 0 )
	, m_toolBarWasHidden( false )
{
	view->insertChildClient (this);

	setInstance( KGenericFactory<ISearchPlugin>::instance() );

	m_searchForwardAction = new TDEAction(
		i18n("Search Incrementally"), CTRL+ALT+Key_F,
		this, TQ_SLOT(slotSearchForwardAction()),
		actionCollection(), "edit_isearch" );
	m_searchBackwardAction = new TDEAction(
		i18n("Search Incrementally Backwards"), CTRL+ALT+SHIFT+Key_F,
		this, TQ_SLOT(slotSearchBackwardAction()),
		actionCollection(), "edit_isearch_reverse" );

	m_label = new TQLabel( i18n("I-Search:"), 0L, "tde toolbar widget" );
	KWidgetAction* labelAction = new KWidgetAction(
		m_label,
		i18n("I-Search:"), 0, 0, 0,
		actionCollection(), "isearch_label" );
	labelAction->setShortcutConfigurable( false );

	m_combo = new KHistoryCombo();
	m_combo->setDuplicatesEnabled( false );
	m_combo->setMaximumWidth( 300 );
	m_combo->lineEdit()->installEventFilter( this );
	connect( m_combo, TQ_SIGNAL(textChanged(const TQString&)),
	         this, TQ_SLOT(slotTextChanged(const TQString&)) );
	connect( m_combo, TQ_SIGNAL(returnPressed(const TQString&)),
	         this, TQ_SLOT(slotReturnPressed(const TQString&)) );
	connect( m_combo, TQ_SIGNAL(aboutToShowContextMenu(TQPopupMenu*)),
		 this, TQ_SLOT(slotAddContextMenuItems(TQPopupMenu*)) );
	m_comboAction = new KWidgetAction(
		m_combo,
		i18n("Search"), 0, 0, 0,
		actionCollection(), "isearch_combo" );
	m_comboAction->setAutoSized( true );
	m_comboAction->setShortcutConfigurable( false );

	TDEActionMenu* optionMenu = new TDEActionMenu(
		i18n("Search Options"), "configure",
		actionCollection(), "isearch_options" );
	optionMenu->setDelayed( false );

	TDEToggleAction* action = new TDEToggleAction(
		i18n("Case Sensitive"), TDEShortcut(),
		actionCollection(), "isearch_case_sensitive" );
	action->setShortcutConfigurable( false );
	connect( action, TQ_SIGNAL(toggled(bool)),
	         this, TQ_SLOT(setCaseSensitive(bool)) );
	action->setChecked( m_caseSensitive );
	optionMenu->insert( action );

	action = new TDEToggleAction(
		i18n("From Beginning"), TDEShortcut(),
		actionCollection(), "isearch_from_beginning" );
	action->setShortcutConfigurable( false );
	connect( action, TQ_SIGNAL(toggled(bool)),
	         this, TQ_SLOT(setFromBeginning(bool)) );
	action->setChecked( m_fromBeginning );
	optionMenu->insert( action );

	action = new TDEToggleAction(
		i18n("Regular Expression"), TDEShortcut(),
		actionCollection(), "isearch_reg_exp" );
	action->setShortcutConfigurable( false );
	connect( action, TQ_SIGNAL(toggled(bool)),
	         this, TQ_SLOT(setRegExp(bool)) );
	action->setChecked( m_regExp );
	optionMenu->insert( action );

// 	optionMenu->insert( new TDEActionSeparator() );
//
// 	action = new TDEToggleAction(
// 		i18n("Auto-Wrap Search"), TDEShortcut(),
// 		actionCollection(), "isearch_auto_wrap" );
// 	connect( action, TQ_SIGNAL(toggled(bool)),
// 	         this, TQ_SLOT(setAutoWrap(bool)) );
// 	action->setChecked( m_autoWrap );
// 	optionMenu->insert( action );

	setXMLFile( "tdetexteditor_isearchui.rc" );
}

ISearchPluginView::~ISearchPluginView()
{
	writeConfig();
	m_combo->lineEdit()->removeEventFilter( this );
	delete m_combo;
	delete m_label;
}

void ISearchPluginView::setView( KTextEditor::View* view )
{
	m_view = view;
	m_doc  = m_view->document();
	m_searchIF = KTextEditor::searchInterface ( m_doc );
	m_cursorIF = KTextEditor::viewCursorInterface ( m_view );
	m_selectIF = KTextEditor::selectionInterface ( m_doc );
	if( !m_doc || !m_cursorIF || !m_selectIF ) {
		m_view = 0L;
		m_doc = 0L;
		m_searchIF = 0L;
		m_cursorIF = 0L;
		m_selectIF = 0L;
	}

	readConfig();
}

void ISearchPluginView::readConfig()
{
    // TDEConfig* config = instance()->config();
}

void ISearchPluginView::writeConfig()
{
    // TDEConfig* config = instance()->config();
}

void ISearchPluginView::setCaseSensitive( bool caseSensitive )
{
	m_caseSensitive = caseSensitive;
}

void ISearchPluginView::setFromBeginning( bool fromBeginning )
{
	m_fromBeginning = fromBeginning;

	if( m_fromBeginning ) {
		m_searchLine = m_searchCol = 0;
	}
}

void ISearchPluginView::setRegExp( bool regExp )
{
	m_regExp = regExp;
}

void ISearchPluginView::setAutoWrap( bool autoWrap )
{
	m_autoWrap = autoWrap;
}

bool ISearchPluginView::eventFilter( TQObject* o, TQEvent* e )
{
	if( o != m_combo->lineEdit() )
		return false;

	if( e->type() == TQEvent::FocusIn ) {
		TQFocusEvent* focusEvent = (TQFocusEvent*)e;
		if( focusEvent->reason() == TQFocusEvent::ActiveWindow ||
		    focusEvent->reason() == TQFocusEvent::Popup )
			return false;
		startSearch();
	}

	if( e->type() == TQEvent::FocusOut ) {
		TQFocusEvent* focusEvent = (TQFocusEvent*)e;
		if( focusEvent->reason() == TQFocusEvent::ActiveWindow ||
		    focusEvent->reason() == TQFocusEvent::Popup )
			return false;
		endSearch();
	}

	if( e->type() == TQEvent::KeyPress ) {
		TQKeyEvent *keyEvent = (TQKeyEvent*)e;
		if( keyEvent->key() == TQt::Key_Escape )
			quitToView( TQString::null );
	}

	return false;
}

// Sigh... i18n hell.
void ISearchPluginView::updateLabelText(
	bool failing /* = false */, bool reverse /* = false */,
	bool wrapped /* = false */, bool overwrapped /* = false */ )
{
	TQString text;
	// Reverse binary:
	// 0000
	if( !failing && !reverse && !wrapped && !overwrapped ) {
		text = i18n("Incremental Search", "I-Search:");
	// 1000
	} else if ( failing && !reverse && !wrapped && !overwrapped ) {
		text = i18n("Incremental Search found no match", "Failing I-Search:");
	// 0100
	} else if ( !failing && reverse && !wrapped && !overwrapped ) {
		text = i18n("Incremental Search in the reverse direction", "I-Search Backward:");
	// 1100
	} else if ( failing && reverse && !wrapped && !overwrapped ) {
		text = i18n("Failing I-Search Backward:");
	// 0010
	} else if ( !failing && !reverse && wrapped  && !overwrapped ) {
		text = i18n("Incremental Search has passed the end of the document", "Wrapped I-Search:");
	// 1010
	} else if ( failing && !reverse && wrapped && !overwrapped ) {
		text = i18n("Failing Wrapped I-Search:");
	// 0110
	} else if ( !failing && reverse && wrapped && !overwrapped ) {
		text = i18n("Wrapped I-Search Backward:");
	// 1110
	} else if ( failing && reverse && wrapped && !overwrapped ) {
		text = i18n("Failing Wrapped I-Search Backward:");
	// 0011
	} else if ( !failing && !reverse && overwrapped ) {
		text = i18n("Incremental Search has passed both the end of the document "
		            "and the original starting position", "Overwrapped I-Search:");
	// 1011
	} else if ( failing && !reverse && overwrapped ) {
		text = i18n("Failing Overwrapped I-Search:");
	// 0111
	} else if ( !failing && reverse && overwrapped ) {
		text = i18n("Overwrapped I-Search Backwards:");
	// 1111
	} else if ( failing && reverse && overwrapped ) {
		text = i18n("Failing Overwrapped I-Search Backward:");
	} else {
		text = i18n("Error: unknown i-search state!");
	}
	m_label->setText( text );
}

void ISearchPluginView::slotSearchForwardAction()
{
	slotSearchAction( false );
}

void ISearchPluginView::slotSearchBackwardAction()
{
	slotSearchAction( true );
}

void ISearchPluginView::slotSearchAction( bool reverse )
{
	if( !m_combo->hasFocus() ) {
		if( m_comboAction->container(0) && m_comboAction->container(0)->isHidden() ) {
			m_toolBarWasHidden = true;
			m_comboAction->container(0)->setHidden( false );
		} else {
			m_toolBarWasHidden = false;
		}
		m_combo->setFocus(); // Will call startSearch()
	} else {
		nextMatch( reverse );
	}
}

void ISearchPluginView::nextMatch( bool reverse )
{
	TQString text = m_combo->currentText();
	if( text.isEmpty() )
		return;
	if( state != MatchSearch ) {
		// Last search was performed by typing, start from that match.
		if( !reverse ) {
			m_searchLine = m_foundLine;
			m_searchCol = m_foundCol + m_matchLen;
		} else {
			m_searchLine = m_foundLine;
			m_searchCol = m_foundCol;
		}
		state = MatchSearch;
	}
        
	bool found = iSearch( m_searchLine, m_searchCol, text, reverse, m_autoWrap );
	if( found ) {
		m_searchLine = m_foundLine;
		m_searchCol = m_foundCol + m_matchLen;
	} else {
		m_wrapped = true;
		m_searchLine = m_searchCol = 0;
	}
}

void ISearchPluginView::startSearch()
{
	if( !m_view ) return;

	m_searchForwardAction->setText( i18n("Next Incremental Search Match") );
	m_searchBackwardAction->setText( i18n("Previous Incremental Search Match") );

	m_wrapped = false;

	if( m_fromBeginning ) {
		m_startLine = m_startCol = 0;
	} else {
		m_cursorIF->cursorPositionReal( &m_startLine, &m_startCol );
	}
	m_searchLine = m_startLine;
	m_searchCol = m_startCol;

	updateLabelText( false, m_searchBackward );

	m_combo->blockSignals( true );

	TQString text = m_selectIF->selection();
	if( text.isEmpty() )
		text = m_lastString;
	m_combo->setCurrentText( text );

	m_combo->blockSignals( false );
	m_combo->lineEdit()->selectAll();

//	kdDebug() << "Starting search at " << m_startLine << ", " << m_startCol << endl;
}

void ISearchPluginView::endSearch()
{
	m_searchForwardAction->setText( i18n("Search Incrementally") );
	m_searchBackwardAction->setText( i18n("Search Incrementally Backwards") );

	updateLabelText();

	if( m_toolBarWasHidden && m_comboAction->containerCount() > 0 ) {
		m_comboAction->container(0)->setHidden( true );
	}
}

void ISearchPluginView::quitToView( const TQString &text )
{
	if( !text.isNull() && !text.isEmpty() ) {
		m_combo->addToHistory( text );
		m_lastString = text;
	}

	if( m_view ) {
		m_view->setFocus(); // Will call endSearch()
	}
}

void ISearchPluginView::slotTextChanged( const TQString& text )
{
	state = TextSearch;

	if( text.isEmpty() )
		return;

	iSearch( m_searchLine, m_searchCol, text, m_searchBackward, m_autoWrap );
}

void ISearchPluginView::slotReturnPressed( const TQString& text )
{
	quitToView( text );
}

void ISearchPluginView::slotAddContextMenuItems( TQPopupMenu *menu )
{
	if( menu ) {
		menu->insertSeparator();
		menu->insertItem( i18n("Case Sensitive"), this,
				  TQ_SLOT(setCaseSensitive(bool)));
		menu->insertItem( i18n("From Beginning"), this,
				  TQ_SLOT(setFromBeginning(bool)));
		menu->insertItem( i18n("Regular Expression"), this,
				  TQ_SLOT(setRegExp(bool)));
		//menu->insertItem( i18n("Auto-Wrap Search"), this,
		//		  TQ_SLOT(setAutoWrap(bool)));
	}
}

bool ISearchPluginView::iSearch(
	uint startLine, uint startCol,
	const TQString& text, bool reverse,
	bool autoWrap )
{
	if( !m_view ) return false;

//	kdDebug() << "Searching for " << text << " at " << startLine << ", " << startCol << endl;
	bool found = false;
	if( !m_regExp ) {
		found = m_searchIF->searchText( startLine,
			           startCol,
			           text,
			           &m_foundLine,
			           &m_foundCol,
			           &m_matchLen,
			           m_caseSensitive,
			           reverse );
	} else {
		found = m_searchIF->searchText( startLine,
			           startCol,
			           TQRegExp( text ),
			           &m_foundLine,
			           &m_foundCol,
			           &m_matchLen,
			           reverse );
	}
	if( found ) {
//		kdDebug() << "Found '" << text << "' at " << m_foundLine << ", " << m_foundCol << endl;
//		v->gotoLineNumber( m_foundLine );
		m_cursorIF->setCursorPositionReal( m_foundLine, m_foundCol + m_matchLen );
		m_selectIF->setSelection( m_foundLine, m_foundCol, m_foundLine, m_foundCol + m_matchLen );
	} else if ( autoWrap ) {
		m_wrapped = true;
		found = iSearch( 0, 0, text, reverse, false );
	}
	// FIXME
	bool overwrapped = ( m_wrapped &&
		((m_foundLine > m_startLine ) ||
		 (m_foundLine == m_startLine && m_foundCol >= m_startCol)) );
//	kdDebug() << "Overwrap = " << overwrapped << ". Start was " << m_startLine << ", " << m_startCol << endl;
	updateLabelText( !found, reverse, m_wrapped, overwrapped );
	return found;
}

ISearchPlugin::ISearchPlugin( TQObject *parent, const char* name, const TQStringList& )
	: KTextEditor::Plugin ( (KTextEditor::Document*) parent, name )
{
}

ISearchPlugin::~ISearchPlugin()
{
}

void ISearchPlugin::addView(KTextEditor::View *view)
{
	ISearchPluginView *nview = new ISearchPluginView (view);
	nview->setView (view);
	m_views.append (nview);
}

void ISearchPlugin::removeView(KTextEditor::View *view)
{
	for (uint z=0; z < m_views.count(); z++)
        {
		if (m_views.at(z)->parentClient() == view)
		{
			ISearchPluginView *nview = m_views.at(z);
			m_views.remove (nview);
			delete nview;
		}
	}
}
