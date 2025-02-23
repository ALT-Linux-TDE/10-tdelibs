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

#include "addressdialog.h"

#include <tqcombobox.h>
#include <tqlineedit.h>
#include <tqlabel.h>
#include <tqlayout.h>

#include <tdelocale.h>

AddressDialog::AddressDialog(TQWidget *parent, const char *name)
	: KDialogBase(Swallow, i18n("ACL Address"), Ok|Cancel, Ok, parent, name, true, true)
{
	TQWidget *w = new TQWidget(this);
	type_ = new TQComboBox(w);
	address_ = new TQLineEdit(w);

	type_->insertItem(i18n("Allow"));
	type_->insertItem(i18n("Deny"));

	TQLabel	*l1 = new TQLabel(i18n("Type:"), w);
	TQLabel	*l2 = new TQLabel(i18n("Address:"), w);

	TQGridLayout	*m1 = new TQGridLayout(w, 2, 2, 0, 5);
	m1->setColStretch(1, 1);
	m1->addWidget(l1, 0, 0, TQt::AlignRight);
	m1->addWidget(l2, 1, 0, TQt::AlignRight);
	m1->addWidget(type_, 0, 1);
	m1->addWidget(address_, 1, 1);

	setMainWidget(w);
	resize(300, 100);
}

TQString AddressDialog::addressString()
{
	TQString s;
	if (type_->currentItem() == 0)
		s.append("Allow ");
	else
		s.append("Deny ");
	if (address_->text().isEmpty())
		s.append("All");
	else
		s.append(address_->text());
	return s;
}

TQString AddressDialog::newAddress(TQWidget *parent)
{
	AddressDialog	dlg(parent);
	if (dlg.exec())
		return dlg.addressString();
	else
		return TQString::null;
}

TQString AddressDialog::editAddress(const TQString& addr, TQWidget *parent)
{
	AddressDialog	dlg(parent);
	int p = addr.find(' ');
	if (p != -1)
	{
		dlg.type_->setCurrentItem(addr.left(p).lower() == "deny" ? 1 : 0);
		dlg.address_->setText(addr.mid(p+1));
	}
	if (dlg.exec())
		return dlg.addressString();
	else
		return TQString::null;
}
