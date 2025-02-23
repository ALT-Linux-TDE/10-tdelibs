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

#include "cupsdbrowsingpage.h"
#include "cupsdconf.h"
#include "editlist.h"
#include "browsedialog.h"

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqwhatsthis.h>

#include <tdelocale.h>
#include <knuminput.h>

CupsdBrowsingPage::CupsdBrowsingPage(TQWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	setPageLabel(i18n("Browsing"));
	setHeader(i18n("Browsing Settings"));
	setPixmap("tdeprint_printer_remote");

	browseinterval_ = new KIntNumInput(this);
	browseport_ = new KIntNumInput(this);
	browsetimeout_ = new KIntNumInput(this);
	browsing_ = new TQCheckBox(i18n("Use browsing"), this);
	cups_ = new TQCheckBox("CUPS", this);
	slp_ = new TQCheckBox("SLP", this);
	browseaddresses_ = new EditList(this);
	browseorder_ = new TQComboBox(this);
	useimplicitclasses_ = new TQCheckBox(i18n("Implicit classes"), this);
	hideimplicitmembers_ = new TQCheckBox(i18n("Hide implicit members"), this);
	useshortnames_ = new TQCheckBox(i18n("Use short names"), this);
	useanyclasses_ = new TQCheckBox(i18n("Use \"any\" classes"), this);

	browseorder_->insertItem(i18n("Allow, Deny"));
	browseorder_->insertItem(i18n("Deny, Allow"));

	browseport_->setRange(0, 9999, 1, true);
	browseport_->setSteps(1, 5);
	browseinterval_->setRange(0, 10000, 1, true);
	browseinterval_->setSteps(1, 10);
	browseinterval_->setSuffix(i18n(" sec"));
	browsetimeout_->setRange(0, 10000, 1, true);
	browsetimeout_->setSteps(1, 10);
	browsetimeout_->setSuffix(i18n(" sec"));

	TQLabel *l1 = new TQLabel(i18n("Browse port:"), this);
	TQLabel *l2 = new TQLabel(i18n("Browse interval:"), this);
	TQLabel *l3 = new TQLabel(i18n("Browse timeout:"), this);
	TQLabel *l4 = new TQLabel(i18n("Browse addresses:"), this);
	TQLabel *l5 = new TQLabel(i18n("Browse order:"), this);
	TQLabel *l6 = new TQLabel(i18n("Browse options:"), this);

	TQGridLayout	*m1 = new TQGridLayout(this, 8, 2, 10, 7);
	m1->setRowStretch(7, 1);
	m1->setColStretch(1, 1);
	TQHBoxLayout	*m2 = new TQHBoxLayout(0, 0, 10);
	m1->addMultiCellLayout(m2, 0, 0, 0, 1);
	m2->addWidget(browsing_);
	m2->addWidget(cups_);
	m2->addWidget(slp_);
	m2->addStretch(1);
	m1->addWidget(l1, 1, 0, TQt::AlignRight);
	m1->addWidget(l2, 2, 0, TQt::AlignRight);
	m1->addWidget(l3, 3, 0, TQt::AlignRight);
	m1->addWidget(l4, 4, 0, TQt::AlignRight|TQt::AlignTop);
	m1->addWidget(l5, 5, 0, TQt::AlignRight);
	m1->addWidget(l6, 6, 0, TQt::AlignRight|TQt::AlignTop);	
	m1->addWidget(browseport_, 1, 1);
	m1->addWidget(browseinterval_, 2, 1);
	m1->addWidget(browsetimeout_, 3, 1);
	m1->addWidget(browseaddresses_, 4, 1);
	m1->addWidget(browseorder_, 5, 1);
	TQGridLayout	*m3 = new TQGridLayout(0, 2, 2, 0, 5);
	m1->addLayout(m3, 6, 1);
	m3->addWidget(useimplicitclasses_, 0, 0);
	m3->addWidget(useanyclasses_, 0, 1);
	m3->addWidget(hideimplicitmembers_, 1, 0);
	m3->addWidget(useshortnames_, 1, 1);

	connect(browsing_, TQ_SIGNAL(toggled(bool)), cups_, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), slp_, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), browseport_, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), browseinterval_, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), browsetimeout_, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), browseaddresses_, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), browseorder_, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), useimplicitclasses_, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), useanyclasses_, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), hideimplicitmembers_, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), useshortnames_, TQ_SLOT(setEnabled(bool)));

	connect(browsing_, TQ_SIGNAL(toggled(bool)), l1, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), l2, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), l3, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), l4, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), l5, TQ_SLOT(setEnabled(bool)));
	connect(browsing_, TQ_SIGNAL(toggled(bool)), l6, TQ_SLOT(setEnabled(bool)));

	connect(browseaddresses_, TQ_SIGNAL(add()), TQ_SLOT(slotAdd()));
	connect(browseaddresses_, TQ_SIGNAL(edit(int)), TQ_SLOT(slotEdit(int)));
	connect(browseaddresses_, TQ_SIGNAL(defaultList()), TQ_SLOT(slotDefaultList()));
	connect(browseinterval_, TQ_SIGNAL(valueChanged(int)), TQ_SLOT(intervalChanged(int)));
	browsing_->setChecked(true);
}

bool CupsdBrowsingPage::loadConfig(CupsdConf *conf, TQString&)
{
	conf_ = conf;
	browsing_->setChecked(conf_->browsing_);
	cups_->setChecked(conf_->browseprotocols_.findIndex("CUPS") != -1);
	slp_->setChecked(conf_->browseprotocols_.findIndex("SLP") != -1);
	browseport_->setValue(conf_->browseport_);
	browseinterval_->setValue(conf_->browseinterval_);
	browsetimeout_->setValue(conf_->browsetimeout_);
	browseaddresses_->insertItems(conf_->browseaddresses_);
	browseorder_->setCurrentItem(conf_->browseorder_);
	useimplicitclasses_->setChecked(conf_->useimplicitclasses_);
	useanyclasses_->setChecked(conf_->useanyclasses_);
	hideimplicitmembers_->setChecked(conf_->hideimplicitmembers_);
	useshortnames_->setChecked(conf_->useshortnames_);

	return true;
}

bool CupsdBrowsingPage::saveConfig(CupsdConf *conf, TQString&)
{
	conf->browsing_ = browsing_->isChecked();
	TQStringList	l;
	if (cups_->isChecked()) l << "CUPS";
	if (slp_->isChecked()) l << "SLP";
	conf->browseprotocols_ = l;
	conf->browseport_ = browseport_->value();
	conf->browseinterval_ = browseinterval_->value();
	conf->browsetimeout_ = browsetimeout_->value();
	conf->browseaddresses_ = browseaddresses_->items();
	conf->browseorder_ = browseorder_->currentItem();
	conf->useimplicitclasses_ = useimplicitclasses_->isChecked();
	conf->useanyclasses_ = useanyclasses_->isChecked();
	conf->hideimplicitmembers_ = hideimplicitmembers_->isChecked();
	conf->useshortnames_ = useshortnames_->isChecked();

	return true;
}

void CupsdBrowsingPage::setInfos(CupsdConf *conf)
{
	TQWhatsThis::add(browsing_, conf->comments_.toolTip("browsing"));
	TQWhatsThis::add(cups_, conf->comments_.toolTip("browseprotocols"));
	TQWhatsThis::add(slp_, conf->comments_.toolTip("browseprotocols"));
	TQWhatsThis::add(browseinterval_, conf->comments_.toolTip("browseinterval"));
	TQWhatsThis::add(browseport_, conf->comments_.toolTip("browseport"));
	TQWhatsThis::add(browsetimeout_, conf->comments_.toolTip("browsetimeout"));
	TQWhatsThis::add(browseaddresses_, conf->comments_.toolTip("browseaddresses"));
	TQWhatsThis::add(browseorder_, conf->comments_.toolTip("browseorder"));
	TQWhatsThis::add(useimplicitclasses_, conf->comments_.toolTip("implicitclasses"));
	TQWhatsThis::add(useanyclasses_, conf->comments_.toolTip("implicitanyclasses"));
	TQWhatsThis::add(hideimplicitmembers_, conf->comments_.toolTip("hideimplicitmembers"));
	TQWhatsThis::add(useshortnames_, conf->comments_.toolTip("browseshortnames"));
}

void CupsdBrowsingPage::slotAdd()
{
	TQString s = BrowseDialog::newAddress(this, conf_);
	if (!s.isEmpty())
		browseaddresses_->insertItem(s);
}

void CupsdBrowsingPage::slotEdit(int index)
{
	TQString s = browseaddresses_->text(index);
	s = BrowseDialog::editAddress(s, this, conf_);
	if (!s.isEmpty())
		browseaddresses_->setText(index, s);
}

void CupsdBrowsingPage::slotDefaultList()
{
	browseaddresses_->clear();
	TQStringList	l;
	l << "Send 255.255.255.255";
	browseaddresses_->insertItems(l);
}

void CupsdBrowsingPage::intervalChanged(int val)
{
	browsetimeout_->setRange(val, 10000, 1, true);
	browsetimeout_->setSteps(1, 10);
}

#include "cupsdbrowsingpage.moc"
