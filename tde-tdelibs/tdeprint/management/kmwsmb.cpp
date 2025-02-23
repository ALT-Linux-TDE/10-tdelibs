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

#include "kmwsmb.h"
#include "kmwizard.h"
#include "smbview.h"
#include "kmprinter.h"
#include "util.h"

#include <tdelocale.h>
#include <kpushbutton.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqlabel.h>

KMWSmb::KMWSmb(TQWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_title = i18n("SMB Printer Settings");
	m_ID = KMWizard::SMB;
	m_nextpage = KMWizard::Driver;

	m_view = new SmbView(this,"SmbView");
	m_loginlabel = new TQLabel( this );
	TQPushButton	*m_scan = new KPushButton(KGuiItem(i18n("Scan"), "viewmag"), this);
	TQPushButton	*m_abort = new KPushButton(KGuiItem(i18n("Abort"), "process-stop"), this);
	m_abort->setEnabled(false);
	TQLabel		*m_worklabel = new TQLabel(i18n("Workgroup:"), this);
	TQLabel		*m_serverlabel = new TQLabel(i18n("Server:"), this);
	TQLabel		*m_printerlabel = new TQLabel(i18n("Printer:"), this);
	m_work = new TQLineEdit(this);
	m_server = new TQLineEdit(this);
	m_printer = new TQLineEdit(this);

	TQVBoxLayout	*lay0 = new TQVBoxLayout(this, 0, 10);
	TQGridLayout	*lay1 = new TQGridLayout(0, 3, 2, 0, 10);
	TQHBoxLayout	*lay3 = new TQHBoxLayout(0, 0, 10);
	lay0->addLayout(lay1,0);
	lay0->addWidget(m_view,1);
	lay0->addLayout(lay3,0);
	lay0->addSpacing(10);
	lay1->setColStretch(1,1);
	lay1->addWidget(m_worklabel,0,0);
	lay1->addWidget(m_serverlabel,1,0);
	lay1->addWidget(m_printerlabel,2,0);
	lay1->addWidget(m_work,0,1);
	lay1->addWidget(m_server,1,1);
	lay1->addWidget(m_printer,2,1);
	lay3->addWidget( m_loginlabel );
	lay3->addStretch(1);
	lay3->addWidget(m_scan);
	lay3->addWidget(m_abort);

	connect(m_scan,TQ_SIGNAL(clicked()),TQ_SLOT(slotScan()));
	connect(m_abort,TQ_SIGNAL(clicked()),TQ_SLOT(slotAbort()));
	connect(m_view,TQ_SIGNAL(printerSelected(const TQString&,const TQString&,const TQString&)),TQ_SLOT(slotPrinterSelected(const TQString&,const TQString&,const TQString&)));
	connect(m_view,TQ_SIGNAL(running(bool)),m_abort,TQ_SLOT(setEnabled(bool)));
}

bool KMWSmb::isValid(TQString& msg)
{
	if (m_server->text().isEmpty())
		msg = i18n("Empty server name.");
	else if (m_printer->text().isEmpty())
		msg = i18n("Empty printer name.");
	else
		return true;
	return false;
}

void KMWSmb::updatePrinter(KMPrinter *printer)
{
	TQString uri = buildSmbURI( m_work->text(), m_server->text(), m_printer->text(), printer->option( "kde-login" ), printer->option( "kde-password" ) );
	printer->setDevice( uri );
}

void KMWSmb::initPrinter(KMPrinter *printer)
{
	if (printer)
	{
		TQString login = printer->option( "kde-login" );
		m_view->setLoginInfos(login,printer->option("kde-password"));
		m_loginlabel->setText( i18n( "Login: %1" ).arg( login.isEmpty() ? i18n( "<anonymous>" ) : login ) );
	}
}

void KMWSmb::slotScan()
{
	m_view->init();
}

void KMWSmb::slotAbort()
{
	m_view->abort();
}

void KMWSmb::slotPrinterSelected(const TQString& work, const TQString& server, const TQString& printer)
{
	m_work->setText(work);
	m_server->setText(server);
	m_printer->setText(printer);
}
#include "kmwsmb.moc"
