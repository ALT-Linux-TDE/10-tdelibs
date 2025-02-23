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

#ifndef _ISearchPlugin_H_
#define _ISearchPlugin_H_

#include <tdetexteditor/plugin.h>
#include <tdetexteditor/view.h>
#include <tdetexteditor/document.h>
#include <tdetexteditor/searchinterface.h>
#include <tdetexteditor/viewcursorinterface.h>
#include <tdetexteditor/selectioninterface.h>        

#include <kxmlguiclient.h>
#include <tqobject.h>
#include <tqguardedptr.h>

class TQLabel;

class ISearchPlugin : public KTextEditor::Plugin, public KTextEditor::PluginViewInterface
{
	TQ_OBJECT
	
public:
	ISearchPlugin( TQObject *parent = 0, const char* name = 0, const TQStringList &args = TQStringList() );
	virtual ~ISearchPlugin();
	
	void addView (KTextEditor::View *view);
	void removeView (KTextEditor::View *view);
	
private:
	TQPtrList<class ISearchPluginView> m_views;
};

class ISearchPluginView : public TQObject, public KXMLGUIClient
{
	TQ_OBJECT
	
public:
	ISearchPluginView( KTextEditor::View *view );
	virtual ~ISearchPluginView();
	
	virtual bool eventFilter( TQObject*, TQEvent* );
	
	void setView( KTextEditor::View* view );   
	
public slots:
	void setCaseSensitive( bool );
	void setFromBeginning( bool );
	void setRegExp( bool );
	void setAutoWrap( bool );
	
private slots:
	void slotSearchForwardAction();
	void slotSearchBackwardAction();
	void slotSearchAction( bool reverse );
	void slotTextChanged( const TQString& text );
	void slotReturnPressed( const TQString& text );
	void slotAddContextMenuItems( TQPopupMenu *menu);
	
private:
	void readConfig();
	void writeConfig();
	
	void updateLabelText( bool failing = false, bool reverse = false,
	                      bool wrapped = false, bool overwrapped = false );
	void startSearch();
	void endSearch();
	void quitToView( const TQString &text );

	void nextMatch( bool reverse );
	bool iSearch( uint startLine, uint startCol,
	              const TQString& text, bool reverse, bool autoWrap );
	
	KTextEditor::View*     m_view;
	KTextEditor::Document* m_doc;
	KTextEditor::SearchInterface* m_searchIF;
	KTextEditor::ViewCursorInterface* m_cursorIF;
	KTextEditor::SelectionInterface* m_selectIF;
	TDEAction*               m_searchForwardAction;
	TDEAction*               m_searchBackwardAction;
	KWidgetAction*         m_comboAction;
	TQGuardedPtr<TQLabel>    m_label;
	TQGuardedPtr<KHistoryCombo> m_combo;
	TQString        m_lastString;
	bool           m_searchBackward;
	bool           m_caseSensitive;
	bool           m_fromBeginning;
	bool           m_regExp;
	bool           m_autoWrap;
	bool           m_wrapped;
	uint           m_startLine, m_startCol;
	uint           m_searchLine, m_searchCol;
	uint           m_foundLine, m_foundCol, m_matchLen;
	bool           m_toolBarWasHidden;
	enum { NoSearch, TextSearch, MatchSearch } state;
};

#endif // _ISearchPlugin_H_
