/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef KPFILTERPAGE_H
#define KPFILTERPAGE_H

#include "kprintdialogpage.h"

#include <tqdict.h>
#include <tqstringlist.h>

class TDEListView;
class KXmlCommand;
class TQListViewItem;
class TQToolButton;
class TQTextBrowser;

class KPFilterPage : public KPrintDialogPage
{
	TQ_OBJECT
public:
	KPFilterPage(TQWidget *parent = 0, const char *name = 0);
	~KPFilterPage();

	void setOptions(const TQMap<TQString,TQString>& opts);
	void getOptions(TQMap<TQString,TQString>& opts, bool incldef = false);
	bool isValid(TQString& msg);

protected slots:
	void slotAddClicked();
	void slotRemoveClicked();
	void slotUpClicked();
	void slotDownClicked();
	void slotConfigureClicked();
	void slotItemSelected(TQListViewItem*);

protected:
	KXmlCommand* currentFilter();
	void checkFilterChain();
	void updateInfo();
	TQStringList activeList();
    void updateButton();
private:
	TDEListView		*m_view;
	TQStringList		m_filters;	// <idname,description> pairs
	TQDict<KXmlCommand>	m_activefilters;
	TQToolButton		*m_add, *m_remove, *m_up, *m_down, *m_configure;
	bool			m_valid;
	TQTextBrowser		*m_info;
};

#endif
