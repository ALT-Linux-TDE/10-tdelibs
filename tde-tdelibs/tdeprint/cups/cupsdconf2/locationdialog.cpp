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

#include "locationdialog.h"
#include "cupsdconf.h"
#include "editlist.h"
#include "addressdialog.h"

#include <tqlineedit.h>
#include <tqcombobox.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqwhatsthis.h>

#include <tdelocale.h>
#include <kiconloader.h>

LocationDialog::LocationDialog(TQWidget *parent, const char *name)
	: KDialogBase(parent, name, true, TQString::null, Ok|Cancel, Ok, true)
{
	TQWidget	*dummy = new TQWidget(this);
	setMainWidget(dummy);
	resource_ = new TQComboBox(dummy);
	authtype_ = new TQComboBox(dummy);
	authclass_ = new TQComboBox(dummy);
	authname_ = new TQLineEdit(dummy);
	encryption_ = new TQComboBox(dummy);
	satisfy_ = new TQComboBox(dummy);
	order_ = new TQComboBox(dummy);
	addresses_ = new EditList(dummy);

	authtype_->insertItem(i18n("None"));
	authtype_->insertItem(i18n("Basic"));
	authtype_->insertItem(i18n("Digest"));

	authclass_->insertItem(i18n("None"));
	authclass_->insertItem(i18n("User"));
	authclass_->insertItem(i18n("System"));
	authclass_->insertItem(i18n("Group"));

	encryption_->insertItem(i18n("Always"));
	encryption_->insertItem(i18n("Never"));
	encryption_->insertItem(i18n("Required"));
	encryption_->insertItem(i18n("If Requested"));

	satisfy_->insertItem(i18n("All"));
	satisfy_->insertItem(i18n("Any"));

	order_->insertItem(i18n("Allow, Deny"));
	order_->insertItem(i18n("Deny, Allow"));

	connect(authclass_, TQ_SIGNAL(activated(int)), TQ_SLOT(slotClassChanged(int)));
	connect(authtype_, TQ_SIGNAL(activated(int)), TQ_SLOT(slotTypeChanged(int)));

	TQLabel	*l1 = new TQLabel(i18n("Resource:"), dummy);
	TQLabel	*l2 = new TQLabel(i18n("Authentication:"), dummy);
	TQLabel	*l3 = new TQLabel(i18n("Class:"), dummy);
	TQLabel	*l4 = new TQLabel(i18n("Names:"), dummy);
	TQLabel	*l5 = new TQLabel(i18n("Encryption:"), dummy);
	TQLabel	*l6 = new TQLabel(i18n("Satisfy:"), dummy);
	TQLabel	*l7 = new TQLabel(i18n("ACL order:"), dummy);
	TQLabel	*l8 = new TQLabel(i18n("ACL addresses:"),dummy);

	TQGridLayout	*m1 = new TQGridLayout(dummy, 8, 2, 0, 5);
	m1->setColStretch(1, 1);
	m1->addWidget(l1, 0, 0, TQt::AlignRight);
	m1->addWidget(l2, 1, 0, TQt::AlignRight);
	m1->addWidget(l3, 2, 0, TQt::AlignRight);
	m1->addWidget(l4, 3, 0, TQt::AlignRight);
	m1->addWidget(l5, 4, 0, TQt::AlignRight);
	m1->addWidget(l6, 5, 0, TQt::AlignRight);
	m1->addWidget(l7, 6, 0, TQt::AlignRight);
	m1->addWidget(l8, 7, 0, TQt::AlignRight|TQt::AlignTop);
	m1->addWidget(resource_, 0, 1);
	m1->addWidget(authtype_, 1, 1);
	m1->addWidget(authclass_, 2, 1);
	m1->addWidget(authname_, 3, 1);
	m1->addWidget(encryption_, 4, 1);
	m1->addWidget(satisfy_, 5, 1);
	m1->addWidget(order_, 6, 1);
	m1->addWidget(addresses_, 7, 1);

	setCaption(i18n("Location"));
	resize(400, 100);

	slotTypeChanged(AUTHTYPE_NONE);
	slotClassChanged(AUTHCLASS_ANONYMOUS);
	encryption_->setCurrentItem(ENCRYPT_IFREQUESTED);

	connect(addresses_, TQ_SIGNAL(add()), TQ_SLOT(slotAdd()));
	connect(addresses_, TQ_SIGNAL(edit(int)), TQ_SLOT(slotEdit(int)));
	connect(addresses_, TQ_SIGNAL(defaultList()), TQ_SLOT(slotDefaultList()));
}

void LocationDialog::setInfos(CupsdConf *conf)
{
	conf_ = conf;

	TQPtrListIterator<CupsResource>	it(conf->resources_);
	for (; it.current(); ++it)
		resource_->insertItem(SmallIcon(it.current()->typeToIconName(it.current()->type_)), it.current()->text_);

	TQWhatsThis::add(encryption_, conf_->comments_.toolTip("encryption"));
	TQWhatsThis::add(order_, conf_->comments_.toolTip("order"));
	TQWhatsThis::add(authclass_, conf_->comments_.toolTip("authclass"));
	TQWhatsThis::add(authtype_, conf_->comments_.toolTip("authtype"));
	TQWhatsThis::add(authname_, conf_->comments_.toolTip("authname"));
	TQWhatsThis::add(satisfy_, conf_->comments_.toolTip("satisfy"));
	TQWhatsThis::add(addresses_, conf_->comments_.toolTip("allowdeny"));
}

void LocationDialog::fillLocation(CupsLocation *loc)
{
	loc->resource_ = conf_->resources_.at(resource_->currentItem());
	loc->resourcename_ = loc->resource_->path_;
	loc->authtype_ = authtype_->currentItem();
	loc->authclass_ = (loc->authtype_ == AUTHTYPE_NONE ? AUTHCLASS_ANONYMOUS : authclass_->currentItem());
	loc->authname_ = (loc->authclass_ == AUTHCLASS_USER || loc->authclass_ == AUTHCLASS_GROUP ? authname_->text() : TQString::null);
	loc->encryption_ = encryption_->currentItem();
	loc->satisfy_ = satisfy_->currentItem();
	loc->order_ = order_->currentItem();
	loc->addresses_ = addresses_->items();
}

void LocationDialog::setLocation(CupsLocation *loc)
{
	int	index = conf_->resources_.findRef(loc->resource_);
	resource_->setCurrentItem(index);
	authtype_->setCurrentItem(loc->authtype_);
	authclass_->setCurrentItem(loc->authclass_);
	authname_->setText(loc->authname_);
	encryption_->setCurrentItem(loc->encryption_);
	satisfy_->setCurrentItem(loc->satisfy_);
	order_->setCurrentItem(loc->order_);
	addresses_->insertItems(loc->addresses_);

	slotTypeChanged(loc->authtype_);
	slotClassChanged(loc->authclass_);
}

void LocationDialog::slotTypeChanged(int index)
{
	authclass_->setEnabled(index != AUTHTYPE_NONE);
	if (index != AUTHTYPE_NONE)
		slotClassChanged(authclass_->currentItem());
	else
		authname_->setEnabled(false);
}

void LocationDialog::slotClassChanged(int index)
{
	authname_->setEnabled((index == AUTHCLASS_USER || index == AUTHCLASS_GROUP));
}

bool LocationDialog::newLocation(CupsLocation *loc, TQWidget *parent, CupsdConf *conf)
{
	LocationDialog	dlg(parent);
	if (conf)
		dlg.setInfos(conf);
	if (dlg.exec())
	{
		dlg.fillLocation(loc);
		return true;
	}
	else
		return false;
}

bool LocationDialog::editLocation(CupsLocation *loc, TQWidget *parent, CupsdConf *conf)
{
	LocationDialog	dlg(parent);
	if (conf)
		dlg.setInfos(conf);
	dlg.setLocation(loc);
	dlg.resource_->setEnabled(false);
	if (dlg.exec())
	{
		dlg.fillLocation(loc);
		return true;
	}
	else
		return false;
}

void LocationDialog::slotAdd()
{
	TQString addr = AddressDialog::newAddress(this);
	if (!addr.isEmpty())
		addresses_->insertItem(addr);
}

void LocationDialog::slotEdit(int index)
{
	TQString addr = addresses_->text(index);
	addr = AddressDialog::editAddress(addr, this);
	if (!addr.isEmpty())
		addresses_->insertItem(addr);
}

void LocationDialog::slotDefaultList()
{
	addresses_->clear();
}

#include "locationdialog.moc"
