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

#ifndef KMPRINTERVIEW_H
#define KMPRINTERVIEW_H

#include <tqwidgetstack.h>
#include <tqptrlist.h>

class KMIconView;
class KMListView;
class KMPrinter;
class TQIconViewItem;

class KMPrinterView : public TQWidgetStack
{
	TQ_OBJECT
public:
	enum ViewType { Icons = 0, List, Tree };

	KMPrinterView(TQWidget *parent = 0, const char *name = 0);
	~KMPrinterView();

	void setPrinterList(TQPtrList<KMPrinter> *list);
	void setPrinter( KMPrinter* );
	void setViewType(ViewType t);
	ViewType viewType() const 	{ return m_type; }

	TQSize minimumSizeHint() const;

signals:
	void printerSelected(const TQString&);
	void rightButtonClicked(const TQString&, const TQPoint&);

protected slots:
	void slotPrinterSelected(const TQString&);

private:
	KMIconView	*m_iconview;
	KMListView	*m_listview;
	ViewType	m_type;
	TQString		m_current;
	bool m_listset;
};

#endif
