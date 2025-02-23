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

#include "kmconfigfilter.h"
#include "kmmanager.h"
#include "kmfactory.h"

#include <tqgroupbox.h>
#include <tqlineedit.h>
#include <tqtoolbutton.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqapplication.h>

#include <tdelocale.h>
#include <tdeconfig.h>
#include <kiconloader.h>
#include <tdelistbox.h>
#include <kdialog.h>

KMConfigFilter::KMConfigFilter(TQWidget *parent, const char *name)
: KMConfigPage(parent, name)
{
	setPageName(i18n("Filter"));
	setPageHeader(i18n("Printer Filtering Settings"));
	setPagePixmap("filter");

	TQGroupBox	*box = new TQGroupBox(0, TQt::Vertical, i18n("Printer Filter"), this);

	m_list1 = new TDEListBox(box);
	m_list1->setSelectionMode(TDEListBox::Extended);
	m_list2 = new TDEListBox(box);
	m_list2->setSelectionMode(TDEListBox::Extended);
	m_add = new TQToolButton( box );
	m_add->setIconSet(TQApplication::reverseLayout() ? SmallIconSet( "back" ) : SmallIconSet( "forward" ));
	m_remove = new TQToolButton( box );
	m_remove->setIconSet(TQApplication::reverseLayout() ? SmallIconSet( "forward" ) : SmallIconSet( "back" ));
	m_locationre = new TQLineEdit(box);
	TQLabel	*lab = new TQLabel(box);
	lab->setText(i18n("The printer filtering allows you to view only a specific set of "
	                  "printers instead of all of them. This may be useful when there are a "
			  "lot of printers available but you only use a few ones. Select the "
			  "printers you want to see from the list on the left or enter a <b>Location</b> "
			  "filter (ex: Group_1*). Both are cumulative and ignored if empty."));
	lab->setTextFormat(TQt::RichText);
	TQLabel	*lab1 = new TQLabel(i18n("Location filter:"), box);

	TQVBoxLayout	*l0 = new TQVBoxLayout(this, 0, KDialog::spacingHint());
	l0->addWidget(box, 1);
	TQVBoxLayout	*l1 = new TQVBoxLayout(box->layout(), KDialog::spacingHint());
	l1->addWidget(lab);
	TQGridLayout	*l2 = new TQGridLayout(0, 4, 3, 0, KDialog::spacingHint());
	l1->addLayout(l2);
	l2->setRowStretch(0, 1);
	l2->setRowStretch(3, 1);
	l2->setColStretch(0, 1);
	l2->setColStretch(2, 1);
	l2->addMultiCellWidget(m_list1, 0, 3, 0, 0);
	l2->addMultiCellWidget(m_list2, 0, 3, 2, 2);
	l2->addWidget(m_add, 1, 1);
	l2->addWidget(m_remove, 2, 1);
	TQHBoxLayout	*l3 = new TQHBoxLayout(0, 0, KDialog::spacingHint());
	l1->addLayout(l3, 0);
	l3->addWidget(lab1, 0);
	l3->addWidget(m_locationre, 1);

	connect(m_add, TQ_SIGNAL(clicked()), TQ_SLOT(slotAddClicked()));
	connect(m_remove, TQ_SIGNAL(clicked()), TQ_SLOT(slotRemoveClicked()));
	connect(m_list1, TQ_SIGNAL(selectionChanged()), TQ_SLOT(slotSelectionChanged()));
	connect(m_list2, TQ_SIGNAL(selectionChanged()), TQ_SLOT(slotSelectionChanged()));
	m_add->setEnabled(false);
	m_remove->setEnabled(false);
}

void KMConfigFilter::loadConfig(TDEConfig *conf)
{
	conf->setGroup("Filter");
	TQStringList	m_plist = conf->readListEntry("Printers");
	TQPtrListIterator<KMPrinter>	it(*(KMManager::self()->printerListComplete(false)));
	for (; it.current(); ++it)
	{
		if (!it.current()->isSpecial() && !it.current()->isVirtual())
		{
			TDEListBox	*lb = (m_plist.find(it.current()->printerName()) == m_plist.end() ? m_list1 : m_list2);
			lb->insertItem(SmallIcon(it.current()->pixmap()), it.current()->printerName());
		}
	}
	m_list1->sort();
	m_list2->sort();
	m_locationre->setText(conf->readEntry("LocationRe"));
}

void KMConfigFilter::saveConfig(TDEConfig *conf)
{
	conf->setGroup("Filter");
	TQStringList	plist;
	for (uint i=0; i<m_list2->count(); i++)
		plist << m_list2->text(i);
	conf->writeEntry("Printers", plist);
	conf->writeEntry("LocationRe", m_locationre->text());
}

void KMConfigFilter::transfer(TDEListBox *from, TDEListBox *to)
{
	for (uint i=0; i<from->count();)
	{
		if (from->isSelected(i))
		{
			to->insertItem(*(from->pixmap(i)), from->text(i));
			from->removeItem(i);
		}
		else
			i++;
	}
	to->sort();
}

void KMConfigFilter::slotAddClicked()
{
	transfer(m_list1, m_list2);
}

void KMConfigFilter::slotRemoveClicked()
{
	transfer(m_list2, m_list1);
}

void KMConfigFilter::slotSelectionChanged()
{
	const TDEListBox	*lb = static_cast<const TDEListBox*>(sender());
	if (!lb)
		return;
	TQToolButton	*pb = (lb == m_list1 ? m_add : m_remove);
	for (uint i=0; i<lb->count(); i++)
		if (lb->isSelected(i))
		{
			pb->setEnabled(true);
			return;
		}
	pb->setEnabled(false);
}

#include "kmconfigfilter.moc"
