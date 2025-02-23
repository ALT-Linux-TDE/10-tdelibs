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

#ifndef CJANUSWIDGET_H
#define CJANUSWIDGET_H

#include <tqwidget.h>
#include <tqpixmap.h>
#include <tqptrlist.h>

class TQWidgetStack;
class TQLabel;
class TQListBoxItem;

class CJanusWidget : public TQWidget
{
	TQ_OBJECT
public:
	CJanusWidget(TQWidget *parent, const char *name);
	~CJanusWidget();

	void addPage(TQWidget *w, const TQString& text, const TQString& header, const TQPixmap& pix);
	void enablePage(TQWidget *w);
	void disablePage(TQWidget *w);
	void clearPages();

protected slots:
	void slotSelected(TQListBoxItem*);

public:
	class CPage;
	class CListBox;
	class CListBoxItem;

private:
	CPage* findPage(TQWidget *w);
	CPage* findPage(TQListBoxItem *i);
	TQListBoxItem* findPrevItem(CPage*);

private:
	TQPtrList<CPage>	m_pages;
	CListBox		*m_iconlist;
	TQLabel			*m_header;
	TQWidgetStack		*m_stack;
	TQWidget		*m_empty;
};

#endif
