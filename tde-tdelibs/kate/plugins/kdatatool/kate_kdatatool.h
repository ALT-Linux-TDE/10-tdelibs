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

// $Id$

#ifndef _KATE_KDATATOOL_
#define _KATE_KDATATOOL_

#include <tdetexteditor/plugin.h>
#include <tqstringlist.h>
#include <kxmlguiclient.h>
#include <tqguardedptr.h>

class TDEActionMenu;
class KDataToolInfo;

namespace KTextEditor
{

class View;

class KDataToolPlugin : public KTextEditor::Plugin, public KTextEditor::PluginViewInterface
{
	TQ_OBJECT

public:
	KDataToolPlugin( TQObject *parent = 0, const char* name = 0, const TQStringList &args = TQStringList() );
	virtual ~KDataToolPlugin();
	void addView (KTextEditor::View *view);
	void removeView (KTextEditor::View *view);

  private:
	TQPtrList<class KDataToolPluginView> m_views;
};


class KDataToolPluginView : public TQObject, public KXMLGUIClient
{
	TQ_OBJECT

public:
	KDataToolPluginView( KTextEditor::View *view );
	virtual ~KDataToolPluginView();
	void setView( KTextEditor::View* ){;}
private:
	View *m_view;
	bool m_singleWord;
	int m_singleWord_line, m_singleWord_start, m_singleWord_end;
	TQString m_wordUnderCursor;
	TQPtrList<TDEAction> m_actionList;
	TQGuardedPtr<TDEActionMenu> m_menu;
	TDEAction *m_notAvailable;
protected slots:
	void aboutToShow();
	void slotToolActivated( const KDataToolInfo &datatoolinfo, const TQString &string );
	void slotNotAvailable();
};

}

#endif
