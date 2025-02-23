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

#ifndef KMICONVIEW_H
#define KMICONVIEW_H

#include <kiconview.h>
#include <tqptrlist.h>

#include "kmobject.h"

class KMPrinter;

class KMIconViewItem : public TQIconViewItem, public KMObject
{
public:
	KMIconViewItem(TQIconView *parent, KMPrinter *p);
	void updatePrinter(KMPrinter *printer = 0, int mode = TQIconView::Bottom);
	bool isClass() const	{ return m_isclass; }

protected:
	virtual void paintItem(TQPainter*, const TQColorGroup&);
	virtual void calcRect(const TQString& text_ = TQString::null);

private:
	int		m_mode;
	TQString		m_pixmap;
	char		m_state;
	bool		m_isclass;
};

class KMIconView : public TDEIconView
{
	TQ_OBJECT
public:
	enum ViewMode { Big, Small };

	KMIconView(TQWidget *parent = 0, const char *name = 0);
	~KMIconView();

	void setPrinterList(TQPtrList<KMPrinter> *list);
	void setPrinter(const TQString&);
	void setPrinter(KMPrinter*);
	void setViewMode(ViewMode);

signals:
	void rightButtonClicked(const TQString&, const TQPoint&);
	void printerSelected(const TQString&);

protected slots:
	void slotRightButtonClicked(TQIconViewItem*, const TQPoint&);
	void slotSelectionChanged();

private:
	KMIconViewItem* findItem(KMPrinter *p);

private:
	TQPtrList<KMIconViewItem>	m_items;
	ViewMode		m_mode;
};

#endif
