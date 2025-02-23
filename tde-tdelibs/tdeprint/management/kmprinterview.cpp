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

#include "kmprinterview.h"
#include "kmprinter.h"
#include "kmiconview.h"
#include "kmlistview.h"
#include "kmtimer.h"
#include "kmmanager.h"

#include <tqlayout.h>
#include <tqpopupmenu.h>
#include <tdeaction.h>
#include <tdelocale.h>

KMPrinterView::KMPrinterView(TQWidget *parent, const char *name)
: TQWidgetStack(parent,name), m_type(KMPrinterView::Icons)
{
	m_iconview = new KMIconView(this);
	addWidget(m_iconview,0);
	m_listview = new KMListView(this);
	addWidget(m_listview,1);
	m_current = TQString();
	m_listset = false;

	connect(m_iconview,TQ_SIGNAL(rightButtonClicked(const TQString&,const TQPoint&)),TQ_SIGNAL(rightButtonClicked(const TQString&,const TQPoint&)));
	connect(m_listview,TQ_SIGNAL(rightButtonClicked(const TQString&,const TQPoint&)),TQ_SIGNAL(rightButtonClicked(const TQString&,const TQPoint&)));
	connect(m_iconview,TQ_SIGNAL(printerSelected(const TQString&)),TQ_SIGNAL(printerSelected(const TQString&)));
	connect(m_listview,TQ_SIGNAL(printerSelected(const TQString&)),TQ_SIGNAL(printerSelected(const TQString&)));
	connect(m_iconview,TQ_SIGNAL(printerSelected(const TQString&)),TQ_SLOT(slotPrinterSelected(const TQString&)));
	connect(m_listview,TQ_SIGNAL(printerSelected(const TQString&)),TQ_SLOT(slotPrinterSelected(const TQString&)));

	setViewType(m_type);
	setSizePolicy( TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Expanding ) );
}

KMPrinterView::~KMPrinterView()
{
}

void KMPrinterView::setPrinterList(TQPtrList<KMPrinter> *list)
{
	if (m_type != KMPrinterView::Tree || list == 0)
		m_iconview->setPrinterList(list);
	if (m_type == KMPrinterView::Tree || list == 0)
		m_listview->setPrinterList(list);
	m_listset = ( list != 0 );
}

void KMPrinterView::setPrinter( KMPrinter *p )
{
	if ( m_type == KMPrinterView::Tree )
		m_listview->setPrinter( p );
	else
		m_iconview->setPrinter( p );
}

void KMPrinterView::setViewType(ViewType t)
{
	m_type = t;
	switch (m_type)
	{
		case KMPrinterView::Icons:
			m_iconview->setViewMode(KMIconView::Big);
			break;
		case KMPrinterView::List:
			m_iconview->setViewMode(KMIconView::Small);
			break;
		default:
			break;
	}
	TQString	oldcurrent = m_current;
	if ( m_listset )
		setPrinterList(KMManager::self()->printerList(false));
	if (m_type == KMPrinterView::Tree)
	{
		raiseWidget(m_listview);
		m_listview->setPrinter(oldcurrent);
	}
	else
	{
		raiseWidget(m_iconview);
		m_iconview->setPrinter(oldcurrent);
	}
}

void KMPrinterView::slotPrinterSelected(const TQString& p)
{
	m_current = p;
}

TQSize KMPrinterView::minimumSizeHint() const
{
	return TQWidgetStack::minimumSizeHint();
}

#include "kmprinterview.moc"
