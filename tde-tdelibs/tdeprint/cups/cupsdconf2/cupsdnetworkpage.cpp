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

#include "cupsdnetworkpage.h"
#include "cupsdconf.h"
#include "editlist.h"
#include "portdialog.h"
#include "sizewidget.h"

#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>

#include <tdelocale.h>
#include <knuminput.h>

CupsdNetworkPage::CupsdNetworkPage(TQWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	setPageLabel(i18n("Network"));
	setHeader(i18n("Network Settings"));
	setPixmap("network");

	keepalive_ = new TQCheckBox(i18n("Keep alive"), this);
	keepalivetimeout_ = new KIntNumInput(this);
	maxclients_ = new KIntNumInput(this);
	maxrequestsize_ = new SizeWidget(this);
	clienttimeout_ = new KIntNumInput(this);
	hostnamelookup_ = new TQComboBox(this);
	listen_ = new EditList(this);

	keepalivetimeout_->setRange(0, 10000, 1, true);
	keepalivetimeout_->setSteps(1, 10);
	keepalivetimeout_->setSpecialValueText(i18n("Unlimited"));
	keepalivetimeout_->setSuffix(i18n(" sec"));

	maxclients_->setRange(1, 1000, 1, true);
	maxclients_->setSteps(1, 10);

	clienttimeout_->setRange(0, 10000, 1, true);
	clienttimeout_->setSteps(1, 10);
	clienttimeout_->setSpecialValueText(i18n("Unlimited"));
	clienttimeout_->setSuffix(i18n(" sec"));

	hostnamelookup_->insertItem(i18n("Off"));
	hostnamelookup_->insertItem(i18n("On"));
	hostnamelookup_->insertItem(i18n("Double"));

	TQLabel *l1 = new TQLabel(i18n("Hostname lookups:"), this);
	TQLabel *l2 = new TQLabel(i18n("Keep-alive timeout:"), this);
	TQLabel *l3 = new TQLabel(i18n("Max clients:"), this);
	TQLabel *l4 = new TQLabel(i18n("Max request size:"), this);
	TQLabel *l5 = new TQLabel(i18n("Client timeout:"), this);
	TQLabel *l6 = new TQLabel(i18n("Listen to:"), this);

	TQGridLayout	*m1 = new TQGridLayout(this, 8, 2, 10, 7);
	m1->setRowStretch(7, 1);
	m1->setColStretch(1, 1);
	m1->addWidget(l1, 0, 0, TQt::AlignRight);
	m1->addWidget(l2, 2, 0, TQt::AlignRight);
	m1->addWidget(l3, 3, 0, TQt::AlignRight);
	m1->addWidget(l4, 4, 0, TQt::AlignRight);
	m1->addWidget(l5, 5, 0, TQt::AlignRight);
	m1->addWidget(l6, 6, 0, TQt::AlignTop|TQt::AlignRight);
	m1->addWidget(keepalive_, 1, 1);
	m1->addWidget(hostnamelookup_, 0, 1);
	m1->addWidget(keepalivetimeout_, 2, 1);
	m1->addWidget(maxclients_, 3, 1);
	m1->addWidget(maxrequestsize_, 4, 1);
	m1->addWidget(clienttimeout_, 5, 1);
	m1->addWidget(listen_, 6, 1);

	connect(listen_, TQ_SIGNAL(add()), TQ_SLOT(slotAdd()));
	connect(listen_, TQ_SIGNAL(edit(int)), TQ_SLOT(slotEdit(int)));
	connect(listen_, TQ_SIGNAL(defaultList()), TQ_SLOT(slotDefaultList()));
	connect(keepalive_, TQ_SIGNAL(toggled(bool)), keepalivetimeout_, TQ_SLOT(setEnabled(bool)));
	keepalive_->setChecked(true);
}

bool CupsdNetworkPage::loadConfig(CupsdConf *conf, TQString&)
{
	conf_ = conf;
	hostnamelookup_->setCurrentItem(conf_->hostnamelookup_);
	keepalive_->setChecked(conf_->keepalive_);
	keepalivetimeout_->setValue(conf_->keepalivetimeout_);
	maxclients_->setValue(conf_->maxclients_);
	maxrequestsize_->setSizeString(conf_->maxrequestsize_);
	clienttimeout_->setValue(conf_->clienttimeout_);
	listen_->insertItems(conf_->listenaddresses_);

	return true;
}

bool CupsdNetworkPage::saveConfig(CupsdConf *conf, TQString&)
{
	conf->hostnamelookup_ = hostnamelookup_->currentItem();
	conf->keepalive_ = keepalive_->isChecked();
	conf->keepalivetimeout_ = keepalivetimeout_->value();
	conf->maxclients_ = maxclients_->value();
	conf->maxrequestsize_ = maxrequestsize_->sizeString();
	conf->clienttimeout_ = clienttimeout_->value();
	conf->listenaddresses_ = listen_->items();

	return true;
}

void CupsdNetworkPage::setInfos(CupsdConf *conf)
{
	TQWhatsThis::add(hostnamelookup_, conf->comments_.toolTip("hostnamelookups"));
	TQWhatsThis::add(keepalive_, conf->comments_.toolTip("keepalive"));
	TQWhatsThis::add(keepalivetimeout_, conf->comments_.toolTip("keepalivetimeout"));
	TQWhatsThis::add(maxclients_, conf->comments_.toolTip("maxclients"));
	TQWhatsThis::add(maxrequestsize_, conf->comments_.toolTip("maxrequestsize"));
	TQWhatsThis::add(clienttimeout_, conf->comments_.toolTip("timeout"));
	TQWhatsThis::add(listen_, conf->comments_.toolTip("listen"));
}

void CupsdNetworkPage::slotAdd()
{
	TQString	s = PortDialog::newListen(this, conf_);
	if (!s.isEmpty())
		listen_->insertItem(s);
}

void CupsdNetworkPage::slotEdit(int index)
{
	TQString s = listen_->text(index);
	s = PortDialog::editListen(s, this, conf_);
	if (!s.isEmpty())
		listen_->setText(index, s);
}

void CupsdNetworkPage::slotDefaultList()
{
	listen_->clear();
	TQStringList	l;
	l << "Listen *:631";
	listen_->insertItems(l);
}

#include "cupsdnetworkpage.moc"
