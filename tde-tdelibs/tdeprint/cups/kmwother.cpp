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

#include "kmwother.h"
#include "kmwizard.h"
#include "kmprinter.h"
#include "kmmanager.h"
#include "cupsinfos.h"

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqheader.h>
#include <tqdict.h>

#include <tdelocale.h>
#include <tdelistview.h>
#include <kiconloader.h>
#include <kdebug.h>

KMWOther::KMWOther(TQWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::Custom+5;
	m_title = i18n("URI Selection");
	m_nextpage = KMWizard::Driver;

	m_uri = new TQLineEdit(this);
	TQLabel	*l1 = new TQLabel(this);
	l1->setText(i18n("<p>Enter the URI corresponding to the printer to be installed. "
	                 "Examples:</p><ul>"
					 "<li>smb://[login[:passwd]@]server/printer</li>"
					 "<li>lpd://server/queue</li>"
					 "<li>parallel:/dev/lp0</li></ul>"));
	TQLabel	*l2 = new TQLabel(i18n("URI:"), this);
	m_uriview = new TDEListView( this );
	m_uriview->addColumn( "" );
	m_uriview->header()->hide();
	m_uriview->setSorting( -1 );
	connect( m_uriview, TQ_SIGNAL( pressed( TQListViewItem* ) ), TQ_SLOT( slotPressed( TQListViewItem* ) ) );

	TQVBoxLayout	*lay1 = new TQVBoxLayout(this, 0, 15);
	TQVBoxLayout	*lay2 = new TQVBoxLayout(0, 0, 5);
	lay1->addWidget(l1);
	lay1->addLayout(lay2);
	lay1->addWidget( m_uriview );
	lay2->addWidget(l2);
	lay2->addWidget(m_uri);
}

void KMWOther::initPrinter(KMPrinter *p)
{
	m_uri->setText(p->device());

	if ( m_uriview->childCount() == 0 )
	{
		TQStringList l = KMManager::self()->detectLocalPrinters();
		if ( l.isEmpty() || l.count() % 4 != 0 )
			return;

		TQListViewItem *item = 0, *lastparent = 0, *root;
		root = new TQListViewItem( m_uriview, i18n( "CUPS Server %1:%2" ).arg( CupsInfos::self()->host() ).arg( CupsInfos::self()->port() ) );
		root->setPixmap( 0, SmallIcon( "gear" ) );
		root->setOpen( true );
		TQDict<TQListViewItem> parents, last;
		parents.setAutoDelete( false );
		last.setAutoDelete( false );
		for ( TQStringList::Iterator it=l.begin(); it!=l.end(); ++it )
		{
			TQString cl = *it;
			TQString uri = *( ++it );
			TQString desc = *( ++it );
			TQString prt = *( ++it );
			if ( !prt.isEmpty() )
				desc.append( " [" + prt + "]" );
			TQListViewItem *parent = parents.find( cl );
			if ( !parent )
			{
				parent = new TQListViewItem( root, lastparent, cl );
				parent->setOpen( true );
				if ( cl == "network" )
					parent->setPixmap( 0, SmallIcon( "network" ) );
				else if ( cl == "direct" )
					parent->setPixmap( 0, SmallIcon( "tdeprint_computer" ) );
				else if ( cl == "serial" )
					parent->setPixmap( 0, SmallIcon( "usb" ) );
				else
					parent->setPixmap( 0, SmallIcon( "package" ) );
				lastparent = parent;
				parents.insert( cl, parent );
			}
			item = new TQListViewItem( parent, last.find( cl ), desc, uri);
			last.insert( cl, item );
		}
	}
}

void KMWOther::updatePrinter(KMPrinter *p)
{
	p->setDevice( m_uri->text() );
}

void KMWOther::slotPressed( TQListViewItem *item )
{
	if ( item && !item->text( 1 ).isEmpty() )
		m_uri->setText( item->text( 1 ) );
}

#include "kmwother.moc"
