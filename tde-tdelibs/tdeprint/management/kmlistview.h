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

#ifndef KMLISTVIEW_H
#define KMLISTVIEW_H

#include <tqlistview.h>
#include <tqptrlist.h>

class KMListViewItem;
class KMPrinter;

class KMListView : public TQListView
{
	TQ_OBJECT
public:
	KMListView(TQWidget *parent = 0, const char *name = 0);
	~KMListView();

	void setPrinterList(TQPtrList<KMPrinter> *list);
	void setPrinter(const TQString&);
	void setPrinter(KMPrinter*);

signals:
	void rightButtonClicked(const TQString&, const TQPoint&);
	void printerSelected(const TQString&);

protected slots:
	void slotRightButtonClicked(TQListViewItem*, const TQPoint&, int);
	void slotSelectionChanged();
	void slotOnItem(TQListViewItem*);
	void slotOnViewport();

protected:
	KMListViewItem* findItem(KMPrinter*);
	KMListViewItem* findItem(const TQString&);

private:
	TQPtrList<KMListViewItem>	m_items;
	KMListViewItem		*m_root, *m_classes, *m_printers, *m_specials;
};

#endif
