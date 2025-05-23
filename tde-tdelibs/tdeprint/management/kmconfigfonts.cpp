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

#include "kmconfigfonts.h"

#include <tqgroupbox.h>
#include <kpushbutton.h>
#include <tqlayout.h>
#include <tqheader.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqsettings.h>
#include <tqwhatsthis.h>

#include <tdelocale.h>
#include <tdeconfig.h>
#include <kiconloader.h>
#include <kurlrequester.h>
#include <tdefile.h>
#include <tdelistview.h>
#include <kdialog.h>

KMConfigFonts::KMConfigFonts(TQWidget *parent, const char *name)
: KMConfigPage(parent, name)
{
	setPageName(i18n("Fonts"));
	setPageHeader(i18n("Font Settings"));
	setPagePixmap("fonts");

	TQGroupBox	*box = new TQGroupBox(0, TQt::Vertical, i18n("Fonts Embedding"), this);
	TQGroupBox	*box2 = new TQGroupBox(0, TQt::Vertical, i18n("Fonts Path"), this);

	m_embedfonts = new TQCheckBox(i18n("&Embed fonts in PostScript data when printing"), box);
	m_fontpath = new TDEListView(box2);
	m_fontpath->addColumn("");
	m_fontpath->header()->setStretchEnabled(true, 0);
	m_fontpath->header()->hide();
	m_fontpath->setSorting(-1);
	m_addpath = new KURLRequester(box2);
	m_addpath->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
	m_up = new KPushButton(KGuiItem(i18n("&Up"), "go-up"), box2);
	m_down = new KPushButton(KGuiItem(i18n("&Down"), "go-down"), box2);
	m_add = new KPushButton(KGuiItem(i18n("&Add"), "add"), box2);
	m_remove = new KPushButton(KGuiItem(i18n("&Remove"), "edit-delete"), box2);
	TQLabel	*lab0 = new TQLabel(i18n("Additional director&y:"), box2);
	lab0->setBuddy(m_addpath);

	TQVBoxLayout	*l0 = new TQVBoxLayout(box->layout(), KDialog::spacingHint());
	l0->addWidget(m_embedfonts);
	TQVBoxLayout	*l1 = new TQVBoxLayout(box2->layout(), KDialog::spacingHint());
	l1->addWidget(m_fontpath);
	TQHBoxLayout	*l2 = new TQHBoxLayout(0, 0, KDialog::spacingHint());
	l1->addLayout(l2);
	l2->addWidget(m_up);
	l2->addWidget(m_down);
	l2->addWidget(m_remove);
	l1->addSpacing(10);
	l1->addWidget(lab0);
	l1->addWidget(m_addpath);
	TQHBoxLayout	*l3 = new TQHBoxLayout(0, 0, KDialog::spacingHint());
	l1->addLayout(l3);
	l3->addStretch(1);
	l3->addWidget(m_add);
	TQVBoxLayout	*l4 = new TQVBoxLayout(this, 0, KDialog::spacingHint());
	l4->addWidget(box);
	l4->addWidget(box2);

	TQWhatsThis::add(m_embedfonts,
			i18n("These options will automatically put fonts in the PostScript file "
                             "which are not present on the printer. Font embedding usually produces better print results "
			     "(closer to what you see on the screen), but larger print data as well."));
	TQWhatsThis::add(m_fontpath, 
			i18n("When using font embedding you can select additional directories where "
			     "TDE should search for embeddable font files. By default, the X server "
			     "font path is used, so adding those directories is not needed. The default "
			     "search path should be sufficient in most cases."));

	connect(m_remove, TQ_SIGNAL(clicked()), TQ_SLOT(slotRemove()));
	connect(m_add, TQ_SIGNAL(clicked()), TQ_SLOT(slotAdd()));
	connect(m_up, TQ_SIGNAL(clicked()), TQ_SLOT(slotUp()));
	connect(m_down, TQ_SIGNAL(clicked()), TQ_SLOT(slotDown()));
	connect(m_fontpath, TQ_SIGNAL(selectionChanged()), TQ_SLOT(slotSelected()));
	connect(m_addpath, TQ_SIGNAL(textChanged(const TQString&)), TQ_SLOT(slotTextChanged(const TQString&)));
	m_add->setEnabled(false);
	m_remove->setEnabled(false);
	m_up->setEnabled(false);
	m_down->setEnabled(false);
}

void KMConfigFonts::loadConfig(TDEConfig *)
{
	TQSettings	settings;
	m_embedfonts->setChecked(settings.readBoolEntry("/qt/embedFonts", true));
	TQStringList	paths = settings.readListEntry("/qt/fontPath", ':');
	TQListViewItem	*item(0);
	for (TQStringList::ConstIterator it=paths.begin(); it!=paths.end(); ++it)
		item = new TQListViewItem(m_fontpath, item, *it);
}

void KMConfigFonts::saveConfig(TDEConfig *)
{
	TQSettings	settings;
	settings.writeEntry("/qt/embedFonts", m_embedfonts->isChecked());
	TQStringList	l;
	TQListViewItem	*item = m_fontpath->firstChild();
	while (item)
	{
		l << item->text(0);
		item = item->nextSibling();
	}
	settings.writeEntry("/qt/fontPath", l, ':');
}

void KMConfigFonts::slotSelected()
{
	TQListViewItem	*item = m_fontpath->selectedItem();
	m_remove->setEnabled(item);
	m_up->setEnabled(item && item->itemAbove());
	m_down->setEnabled(item && item->itemBelow());
}

void KMConfigFonts::slotAdd()
{
	if (m_addpath->url().isEmpty())
		return;
	TQListViewItem	*lastItem(m_fontpath->firstChild());
	while (lastItem && lastItem->nextSibling())
		lastItem = lastItem->nextSibling();
	TQListViewItem	*item = new TQListViewItem(m_fontpath, lastItem, m_addpath->url());
	m_fontpath->setSelected(item, true);
}

void KMConfigFonts::slotRemove()
{
	delete m_fontpath->selectedItem();
	if (m_fontpath->currentItem())
		m_fontpath->setSelected(m_fontpath->currentItem(), true);
	slotSelected();
}

void KMConfigFonts::slotUp()
{
	TQListViewItem	*citem = m_fontpath->selectedItem(), *nitem = 0;
	if (!citem || !citem->itemAbove())
		return;
	nitem = new TQListViewItem(m_fontpath, citem->itemAbove()->itemAbove(), citem->text(0));
	delete citem;
	m_fontpath->setSelected(nitem, true);
}

void KMConfigFonts::slotDown()
{
	TQListViewItem	*citem = m_fontpath->selectedItem(), *nitem = 0;
	if (!citem || !citem->itemBelow())
		return;
	nitem = new TQListViewItem(m_fontpath, citem->itemBelow(), citem->text(0));
	delete citem;
	m_fontpath->setSelected(nitem, true);
}

void KMConfigFonts::slotTextChanged(const TQString& t)
{
	m_add->setEnabled(!t.isEmpty());
}

#include "kmconfigfonts.moc"
