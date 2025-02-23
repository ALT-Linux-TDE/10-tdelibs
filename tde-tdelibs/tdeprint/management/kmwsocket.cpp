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

#include "kmwsocket.h"
#include "networkscanner.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <tdelistview.h>
#include <tqheader.h>
#include <tqlineedit.h>
#include <tqlabel.h>
#include <tdemessagebox.h>
#include <tqlayout.h>
#include <tdelocale.h>
#include <kiconloader.h>
#include <kseparator.h>

KMWSocket::KMWSocket(TQWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_title = i18n("Network Printer Information");
	m_ID = KMWizard::TCP;
	m_nextpage = KMWizard::Driver;

	m_list = new TDEListView(this);
	m_list->addColumn("");
	m_list->header()->hide();
	m_list->setFrameStyle(TQFrame::WinPanel|TQFrame::Sunken);
	m_list->setLineWidth(1);

	TQLabel	*l1 = new TQLabel(i18n("&Printer address:"),this);
	TQLabel	*l2 = new TQLabel(i18n("P&ort:"),this);

	m_printer = new TQLineEdit(this);
	m_port = new TQLineEdit(this);
	m_port->setText(TQString("9100"));

	l1->setBuddy(m_printer);
	l2->setBuddy(m_port);

	m_scanner = new NetworkScanner( 9100, this );

	KSeparator* sep = new KSeparator( KSeparator::HLine, this);
	sep->setFixedHeight(40);

	connect(m_list,TQ_SIGNAL(selectionChanged(TQListViewItem*)),TQ_SLOT(slotPrinterSelected(TQListViewItem*)));
	connect( m_scanner, TQ_SIGNAL( scanStarted() ), TQ_SLOT( slotScanStarted() ) );
	connect( m_scanner, TQ_SIGNAL( scanFinished() ), TQ_SLOT( slotScanFinished() ) );
	connect( m_scanner, TQ_SIGNAL( scanStarted() ), parent, TQ_SLOT( disableWizard() ) );
	connect( m_scanner, TQ_SIGNAL( scanFinished() ), parent, TQ_SLOT( enableWizard() ) );

	// layout
	TQHBoxLayout	*lay3 = new TQHBoxLayout(this, 0, 10);
	TQVBoxLayout	*lay2 = new TQVBoxLayout(0, 0, 0);

	lay3->addWidget(m_list,1);
	lay3->addLayout(lay2,1);
	lay2->addWidget(l1);
	lay2->addWidget(m_printer);
	lay2->addSpacing(10);
	lay2->addWidget(l2);
	lay2->addWidget(m_port);
	lay2->addWidget(sep);
	lay2->addWidget( m_scanner );
	lay2->addStretch(1);
}

KMWSocket::~KMWSocket()
{
}

void KMWSocket::updatePrinter(KMPrinter *p)
{
	TQString	dev = TQString::fromLatin1("socket://%1:%2").arg(m_printer->text()).arg(m_port->text());
	p->setDevice(dev);
}

bool KMWSocket::isValid(TQString& msg)
{
	if (m_printer->text().isEmpty())
	{
		msg = i18n("You must enter a printer address.");
		return false;
	}
	TQString	port(m_port->text());
	int	p(9100);
	if (!port.isEmpty())
	{
		bool	ok;
		p = port.toInt(&ok);
		if (!ok)
		{
			msg = i18n("Wrong port number.");
			return false;
		}
	}

	if (!m_scanner->checkPrinter(m_printer->text(),p))
	{
		msg = i18n("No printer found at this address/port.");
		return false;
	}
	return true;
}

void KMWSocket::slotScanStarted()
{
	m_list->clear();
}

void KMWSocket::slotScanFinished()
{
	const TQPtrList<NetworkScanner::SocketInfo>	*list = m_scanner->printerList();
	TQPtrListIterator<NetworkScanner::SocketInfo>	it(*list);
	for (;it.current();++it)
	{
		TQString	name;
		if (it.current()->Name.isEmpty())
			name = i18n("Unknown host - 1 is the IP", "<Unknown> (%1)").arg(it.current()->IP);
		else
			name = it.current()->Name;
		TQListViewItem	*item = new TQListViewItem(m_list,name,it.current()->IP,TQString::number(it.current()->Port));
		item->setPixmap(0,SmallIcon("tdeprint_printer"));
	}
}

void KMWSocket::slotPrinterSelected(TQListViewItem *item)
{
	if (!item) return;
	m_printer->setText(item->text(1));
	m_port->setText(item->text(2));
}

#include "kmwsocket.moc"
