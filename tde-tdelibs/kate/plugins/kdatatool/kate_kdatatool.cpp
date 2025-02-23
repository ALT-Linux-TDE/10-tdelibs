/* This file is part of the KDE libraries
   Copyright (C) 2002 Joseph Wenninger <jowenn@jowenn.at> and Daniel Naber <daniel.naber@t-online.de>
   
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

//BEGIN includes
#include "kate_kdatatool.h"
#include "kate_kdatatool.moc"
#include <kgenericfactory.h>
#include <tdeaction.h>
#include <tdetexteditor/view.h>
#include <kdebug.h>
#include <kdatatool.h>
#include <tdetexteditor/document.h>
#include <tdetexteditor/selectioninterface.h>
#include <tdepopupmenu.h>
#include <tdetexteditor/viewcursorinterface.h>
#include <tdetexteditor/editinterface.h>
#include <tdemessagebox.h>
//END includes


K_EXPORT_COMPONENT_FACTORY( tdetexteditor_kdatatool, KGenericFactory<KTextEditor::KDataToolPlugin>( "tdetexteditor_kdatatool" ) )

namespace KTextEditor {

KDataToolPlugin::KDataToolPlugin( TQObject *parent, const char* name, const TQStringList& )
	: KTextEditor::Plugin ( (KTextEditor::Document*) parent, name )
{
}


KDataToolPlugin::~KDataToolPlugin ()
{
}

void KDataToolPlugin::addView(KTextEditor::View *view)
{
	KDataToolPluginView *nview = new KDataToolPluginView (view);
	nview->setView (view);
	m_views.append (nview);
}

void KDataToolPlugin::removeView(KTextEditor::View *view)
{
	for (uint z=0; z < m_views.count(); z++)
        {
		if (m_views.at(z)->parentClient() == view)
		{
			KDataToolPluginView *nview = m_views.at(z);
			m_views.remove (nview);
			delete nview;
		}
	}
}


KDataToolPluginView::KDataToolPluginView( KTextEditor::View *view )
	:m_menu(0),m_notAvailable(0)
{

	view->insertChildClient (this);
	setInstance( KGenericFactory<KDataToolPlugin>::instance() );

	m_menu = new TDEActionMenu(i18n("Data Tools"), actionCollection(), "popup_dataTool");
	connect(m_menu->popupMenu(), TQ_SIGNAL(aboutToShow()), this, TQ_SLOT(aboutToShow()));
	setXMLFile("tdetexteditor_kdatatoolui.rc");

	m_view = view;
}

KDataToolPluginView::~KDataToolPluginView()
{
        m_view->removeChildClient (this);
	delete m_menu;
}

void KDataToolPluginView::aboutToShow()
{
	kdDebug()<<"KTextEditor::KDataToolPluginView::aboutToShow"<<endl;
	TQString word;
	m_singleWord = false;
	m_wordUnderCursor = TQString::null;

	// unplug old actions, if any:
	TDEAction *ac;
	for ( ac = m_actionList.first(); ac; ac = m_actionList.next() ) {
		m_menu->remove(ac);
	}
	if (m_notAvailable) {
		m_menu->remove(m_notAvailable);
		delete m_notAvailable;
		m_notAvailable=0;
	}
	if ( selectionInterface(m_view->document())->hasSelection() )
	{
		word = selectionInterface(m_view->document())->selection();
		if ( word.find(' ') == -1 && word.find('\t') == -1 && word.find('\n') == -1 )
			m_singleWord = true;
		else
			m_singleWord = false;
	} else {
		// No selection -> use word under cursor
		KTextEditor::EditInterface *ei;
		KTextEditor::ViewCursorInterface *ci;
		KTextEditor::View *v = (KTextEditor::View*)m_view; 
		ei = KTextEditor::editInterface(v->document());
		ci = KTextEditor::viewCursorInterface(v);
		uint line, col;
		ci->cursorPositionReal(&line, &col);
		TQString tmp_line = ei->textLine(line);
		m_wordUnderCursor = "";
		// find begin of word:
		m_singleWord_start = 0;
		for(int i = col; i >= 0; i--) {
			TQChar ch = tmp_line.at(i);
			if( ! (ch.isLetter() || ch == '-' || ch == '\'') )
			{
				m_singleWord_start = i+1;
				break;
			}
			m_wordUnderCursor = ch + m_wordUnderCursor;
		}
		// find end of word:
		m_singleWord_end = tmp_line.length();
		for(uint i = col+1; i < tmp_line.length(); i++) {
			TQChar ch = tmp_line.at(i);
			if( ! (ch.isLetter() || ch == '-' || ch == '\'') )
			{
				m_singleWord_end = i;
				break;
			}
			m_wordUnderCursor += ch;
		}
		if( ! m_wordUnderCursor.isEmpty() )
		{
			m_singleWord = true;
			m_singleWord_line = line;
		} else {
			m_notAvailable = new TDEAction(i18n("(not available)"), TQString::null, 0, this, 
					TQ_SLOT(slotNotAvailable()), actionCollection(),"dt_n_av");
			m_menu->insert(m_notAvailable);
			return;
		}
	}

	TDEInstance *inst=instance();

	TQValueList<KDataToolInfo> tools;
	tools += KDataToolInfo::query( "TQString", "text/plain", inst );
	if( m_singleWord )
		tools += KDataToolInfo::query( "TQString", "application/x-singleword", inst );

	m_actionList = KDataToolAction::dataToolActionList( tools, this,
		TQ_SLOT( slotToolActivated( const KDataToolInfo &, const TQString & ) ) );

	for ( ac = m_actionList.first(); ac; ac = m_actionList.next() ) {
		m_menu->insert(ac);
	}

	if( m_actionList.isEmpty() ) {
		m_notAvailable = new TDEAction(i18n("(not available)"), TQString::null, 0, this,
			TQ_SLOT(slotNotAvailable()), actionCollection(),"dt_n_av");
		m_menu->insert(m_notAvailable);
	}
}

void KDataToolPluginView::slotNotAvailable()
{
	KMessageBox::sorry(0, i18n("Data tools are only available when text is selected, "
		"or when the right mouse button is clicked over a word. If no data tools are offered "
		"even when text is selected, you need to install them. Some data tools are part "
		"of the KOffice package."));
}

void KDataToolPluginView::slotToolActivated( const KDataToolInfo &info, const TQString &command )
{

	KDataTool* tool = info.createTool( );
	if ( !tool )
	{
		kdWarning() << "Could not create Tool !" << endl;
		return;
	}

	TQString text;
	if ( selectionInterface(m_view->document())->hasSelection() )
		text = selectionInterface(m_view->document())->selection();
	else
		text = m_wordUnderCursor;

	TQString mimetype = "text/plain";
	TQString datatype = "TQString";

	// If unsupported (and if we have a single word indeed), try application/x-singleword
	if ( !info.mimeTypes().contains( mimetype ) && m_singleWord )
		mimetype = "application/x-singleword";
	
	kdDebug() << "Running tool with datatype=" << datatype << " mimetype=" << mimetype << endl;

	TQString origText = text;

	if ( tool->run( command, &text, datatype, mimetype) )
	{
		kdDebug() << "Tool ran. Text is now " << text << endl;
		if ( origText != text )
		{
			uint line, col;
			viewCursorInterface(m_view)->cursorPositionReal(&line, &col);
			if ( ! selectionInterface(m_view->document())->hasSelection() )
			{
				KTextEditor::SelectionInterface *si;
				si = KTextEditor::selectionInterface(m_view->document());
				si->setSelection(m_singleWord_line, m_singleWord_start, m_singleWord_line, m_singleWord_end);
			}
		
			// replace selection with 'text'
			selectionInterface(m_view->document())->removeSelectedText();
			viewCursorInterface(m_view)->cursorPositionReal(&line, &col);
			editInterface(m_view->document())->insertText(line, col, text);
			 // fixme: place cursor at the end:
			 /* No idea yet (Joseph Wenninger)
			 for ( uint i = 0; i < text.length(); i++ ) {
				viewCursorInterface(m_view)->cursorRight();
			 } */
		}
	}

	delete tool;
}


}
