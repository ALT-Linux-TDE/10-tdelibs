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

#include "cupsdsecuritypage.h"
#include "cupsdconf.h"
#include "qdirlineedit.h"
#include "editlist.h"
#include "locationdialog.h"

#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>

#include <tdelocale.h>
#include <kiconloader.h>
#include <tdemessagebox.h>

CupsdSecurityPage::CupsdSecurityPage(TQWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	setPageLabel(i18n("Security"));
	setHeader(i18n("Security Settings"));
	setPixmap("password");
	locs_.setAutoDelete(true);

	remoteroot_ = new TQLineEdit(this);
	systemgroup_ = new TQLineEdit(this);
	encryptcert_ = new QDirLineEdit(true, this);
	encryptkey_ = new QDirLineEdit(true, this);
	locations_ = new EditList(this);

	TQLabel *l1 = new TQLabel(i18n("Remote root user:"), this);
	TQLabel *l2 = new TQLabel(i18n("System group:"), this);
	TQLabel *l3 = new TQLabel(i18n("Encryption certificate:"), this);
	TQLabel *l4 = new TQLabel(i18n("Encryption key:"), this);
	TQLabel *l5 = new TQLabel(i18n("Locations:"), this);

	TQGridLayout	*m1 = new TQGridLayout(this, 6, 2, 10, 7);
	m1->setRowStretch(5, 1);
	m1->setColStretch(1, 1);
	m1->addWidget(l1, 0, 0, TQt::AlignRight);
	m1->addWidget(l2, 1, 0, TQt::AlignRight);
	m1->addWidget(l3, 2, 0, TQt::AlignRight);
	m1->addWidget(l4, 3, 0, TQt::AlignRight);
	m1->addWidget(l5, 4, 0, TQt::AlignRight|TQt::AlignTop);
	m1->addWidget(remoteroot_, 0, 1);
	m1->addWidget(systemgroup_, 1, 1);
	m1->addWidget(encryptcert_, 2, 1);
	m1->addWidget(encryptkey_, 3, 1);
	m1->addWidget(locations_, 4, 1);

	connect(locations_, TQ_SIGNAL(add()), TQ_SLOT(slotAdd()));
	connect(locations_, TQ_SIGNAL(edit(int)), TQ_SLOT(slotEdit(int)));
	connect(locations_, TQ_SIGNAL(defaultList()), TQ_SLOT(slotDefaultList()));
	connect(locations_, TQ_SIGNAL(deleted(int)), TQ_SLOT(slotDeleted(int)));
}

bool CupsdSecurityPage::loadConfig(CupsdConf *conf, TQString&)
{
	conf_ = conf;
	remoteroot_->setText(conf_->remoteroot_);
	systemgroup_->setText(conf_->systemgroup_);
	encryptcert_->setURL(conf_->encryptcert_);
	encryptkey_->setURL(conf_->encryptkey_);
	locs_.clear();
	TQPtrListIterator<CupsLocation>	it(conf_->locations_);
	for (;it.current();++it)
	{
		locs_.append(new CupsLocation(*(it.current())));
		if (it.current()->resource_)
			locations_->insertItem(SmallIcon(CupsResource::typeToIconName(it.current()->resource_->type_)), it.current()->resource_->text_);
		else
			locations_->insertItem(it.current()->resourcename_);
	}

	return true;
}

bool CupsdSecurityPage::saveConfig(CupsdConf *conf, TQString&)
{
	conf->remoteroot_ = remoteroot_->text();
	conf->systemgroup_ = systemgroup_->text();
	conf->encryptcert_ = encryptcert_->url();
	conf->encryptkey_ = encryptkey_->url();
	conf->locations_.clear();
	TQPtrListIterator<CupsLocation>	it(locs_);
	for (;it.current();++it)
		conf->locations_.append(new CupsLocation(*(it.current())));

	return true;
}

void CupsdSecurityPage::setInfos(CupsdConf *conf)
{
	TQWhatsThis::add(remoteroot_, conf->comments_.toolTip("remoteroot"));
	TQWhatsThis::add(systemgroup_, conf->comments_.toolTip("systemgroup"));
	TQWhatsThis::add(encryptcert_, conf->comments_.toolTip("servercertificate"));
	TQWhatsThis::add(encryptkey_, conf->comments_.toolTip("serverkey"));
	TQWhatsThis::add(locations_, conf->comments_.toolTip("locationsshort"));
}

void CupsdSecurityPage::slotAdd()
{
	CupsLocation	*loc = new CupsLocation;
	if (LocationDialog::newLocation(loc, this, conf_))
	{
		int index(-1);
		for (locs_.first(); locs_.current(); locs_.next())
			if (locs_.current()->resource_ == loc->resource_)
			{
				if (KMessageBox::warningContinueCancel(this, i18n("This location is already defined. Do you want to replace the existing one?"),TQString::null,i18n("Replace")) == KMessageBox::Continue)
				{
					index = locs_.at();
					locs_.remove();
					break;
				}
				else
				{
					delete loc;
					return;
				}
			}
				
		if (index == -1)
			index = locs_.count();
		locs_.insert(index, loc);
		locations_->insertItem(SmallIcon(loc->resource_->typeToIconName(loc->resource_->type_)), loc->resource_->text_);
	}
	else
		delete loc;
}

void CupsdSecurityPage::slotEdit(int index)
{
	CupsLocation *loc = locs_.at(index);
	LocationDialog::editLocation(loc, this, conf_);
}

void CupsdSecurityPage::slotDefaultList()
{
	locs_.clear();
	locations_->clear();
}

void CupsdSecurityPage::slotDeleted(int index)
{
	if (index >= 0 && index < (int)(locs_.count()))
		locs_.remove(index);
}

#include "cupsdsecuritypage.moc"
