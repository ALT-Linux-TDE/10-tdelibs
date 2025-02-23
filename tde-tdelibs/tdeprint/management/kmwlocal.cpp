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

#include "kmwlocal.h"
#include "kmwizard.h"
#include "kmprinter.h"
#include "kmfactory.h"
#include "kmmanager.h"

#include <tdelocale.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqlabel.h>
#include <tqheader.h>
#include <tdelistview.h>
#include <tdemessagebox.h>
#include <kiconloader.h>

KMWLocal::KMWLocal(TQWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_title = i18n("Local Port Selection");
	m_ID = KMWizard::Local;
	m_nextpage = KMWizard::Driver;
	m_initialized = false;
	m_block = false;

	m_ports = new TDEListView(this);
	m_ports->setFrameStyle(TQFrame::WinPanel|TQFrame::Sunken);
	m_ports->setLineWidth(1);
	m_ports->header()->hide();
	m_ports->addColumn("");
	m_ports->setSorting(-1);
	TQListViewItem	*root = new TQListViewItem(m_ports, i18n("Local System"));
	root->setPixmap(0, SmallIcon("tdeprint_computer"));
	root->setOpen(true);
	connect(m_ports, TQ_SIGNAL(selectionChanged(TQListViewItem*)), TQ_SLOT(slotPortSelected(TQListViewItem*)));
	TQLabel	*l1 = new TQLabel(i18n("URI:"), this);
	m_localuri = new TQLineEdit(this);
	connect( m_localuri, TQ_SIGNAL( textChanged( const TQString& ) ), TQ_SLOT( slotTextChanged( const TQString& ) ) );
	m_parents[0] = new TQListViewItem(root, i18n("Parallel"));
	m_parents[1] = new TQListViewItem(root, m_parents[0], i18n("Serial"));
	m_parents[2] = new TQListViewItem(root, m_parents[1], i18n("USB"));
	m_parents[3] = new TQListViewItem(root, m_parents[2], i18n("Others"));
	for (int i=0;i<4;i++)
		m_parents[i]->setPixmap(0, SmallIcon("preferences-desktop-peripherals"));
	TQLabel	*l2 = new TQLabel(i18n("<p>Select a valid detected port, or enter directly the corresponding URI in the bottom edit field.</p>"), this);

	TQVBoxLayout	*lay0 = new TQVBoxLayout(this, 0, 10);
	TQHBoxLayout	*lay1 = new TQHBoxLayout(0, 0, 10);
	lay0->addWidget(l2, 0);
	lay0->addWidget(m_ports, 1);
	lay0->addLayout(lay1, 0);
	lay1->addWidget(l1, 0);
	lay1->addWidget(m_localuri, 1);
}

bool KMWLocal::isValid(TQString& msg)
{
	if (m_localuri->text().isEmpty())
	{
		msg = i18n("The URI is empty","Empty URI.");
		return false;
	}
	else if (m_uris.findIndex(m_localuri->text()) == -1)
	{
		if (KMessageBox::warningContinueCancel(this, i18n("The local URI doesn't correspond to a detected port. Continue?")) == KMessageBox::Cancel)
		{
			msg = i18n("Select a valid port.");
			return false;
		}
	}
	return true;
}

void KMWLocal::slotPortSelected(TQListViewItem *item)
{
	if ( m_block )
		return;

	TQString uri;
	if (!item || item->depth() <= 1 || item->depth() > 3)
		uri = TQString::null;
	else if (item->depth() == 3)
		uri = item->parent()->text( 1 );
	else
		uri = item->text( 1 );
	m_block = true;
	m_localuri->setText( uri );
	m_block = false;
}

void KMWLocal::updatePrinter(KMPrinter *printer)
{
	TQListViewItem *item = m_ports->selectedItem();
	if ( item && item->depth() == 3 )
		printer->setOption( "kde-autodetect", item->text( 0 ) );
	printer->setDevice(m_localuri->text());
}

void KMWLocal::initPrinter(KMPrinter *printer)
{
	if (!m_initialized)
		initialize();

	if (printer)
	{
		m_localuri->setText(printer->device());
	}
}

TQListViewItem* KMWLocal::lookForItem( const TQString& uri )
{
	for ( int i=0; i<4; i++ )
	{
		TQListViewItem *item = m_parents[ i ]->firstChild();
		while ( item )
			if ( item->text( 1 ) == uri )
				if ( item->firstChild() )
					return item->firstChild();
				else
					return item;
			else
				item = item->nextSibling();
	}
	return 0;
}

void KMWLocal::slotTextChanged( const TQString& txt )
{
	if ( m_block )
		return;

	TQListViewItem *item = lookForItem( txt );
	if ( item )
	{
		m_block = true;
		m_ports->setSelected( item, true );
		m_block = false;
	}
	else
		m_ports->clearSelection();
}

void KMWLocal::initialize()
{
	TQStringList	list = KMFactory::self()->manager()->detectLocalPrinters();
	if (list.isEmpty() || (list.count() % 4) != 0)
	{
		KMessageBox::error(this, i18n("Unable to detect local ports."));
		return;
	}
	TQListViewItem	*last[4] = {0, 0, 0, 0};
	for (TQStringList::Iterator it=list.begin(); it!=list.end(); ++it)
	{
		TQString cl = *it;
		++it;

		TQString	uri = *it;
		int p = uri.find( ':' );
		TQString	desc = *(++it), prot = ( p != -1 ? uri.left( p ) : TQString::null );
		TQString	printer = *(++it);
		int	index(-1);
		if (desc.isEmpty())
			desc = uri;
		if (prot == "parallel" || prot == "file")
			index = 0;
		else if (prot == "serial")
			index = 1;
		else if (prot == "usb")
			index = 2;
		else if (cl == "direct")
			index = 3;
		else
			continue;
		last[index] = new TQListViewItem(m_parents[index], last[index], desc, uri);
		last[index]->setPixmap(0, SmallIcon("blockdevice"));
		m_parents[index]->setOpen(true);
		m_uris << uri;
		if (!printer.isEmpty())
		{
			TQListViewItem	*pItem = new TQListViewItem(last[index], printer);
			last[index]->setOpen(true);
			pItem->setPixmap(0, SmallIcon("tdeprint_printer"));
		}
	}
	m_initialized = true;
}

#include "kmwlocal.moc"
