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

#include "browsedialog.h"
#include "cupsdconf.h"

#include <tqlineedit.h>
#include <tqpushbutton.h>
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>
#include <tqregexp.h>

#include <tdelocale.h>

BrowseDialog::BrowseDialog(TQWidget *parent, const char *name)
	: KDialogBase(parent, name, true, TQString::null, Ok|Cancel, Ok, true)
{
	TQWidget	*dummy = new TQWidget(this);
	setMainWidget(dummy);
	type_ = new TQComboBox(dummy);
	from_ = new TQLineEdit(dummy);
	to_ = new TQLineEdit(dummy);
	type_->insertItem(i18n("Send"));
	type_->insertItem(i18n("Allow"));
	type_->insertItem(i18n("Deny"));
	type_->insertItem(i18n("Relay"));
	type_->insertItem(i18n("Poll"));

	TQLabel	*l1 = new TQLabel(i18n("Type:"), dummy);
	TQLabel	*l2 = new TQLabel(i18n("From:"), dummy);
	TQLabel	*l3 = new TQLabel(i18n("To:"), dummy);

	TQGridLayout	*m1 = new TQGridLayout(dummy, 3, 2, 0, 5);
	m1->addWidget(l1, 0, 0, TQt::AlignRight);
	m1->addWidget(l2, 1, 0, TQt::AlignRight);
	m1->addWidget(l3, 2, 0, TQt::AlignRight);
	m1->addWidget(type_, 0, 1);
	m1->addWidget(from_, 1, 1);
	m1->addWidget(to_, 2, 1);

	connect(type_, TQ_SIGNAL(activated(int)), TQ_SLOT(slotTypeChanged(int)));
	slotTypeChanged(type_->currentItem());

	setCaption(i18n("Browse Address"));
	resize(250, 100);
}

TQString BrowseDialog::addressString()
{
	TQString s;
	switch (type_->currentItem())
	{
		case 0:
			s.append("Send");
			break;
		case 1:
			s.append("Allow");
			break;
		case 2:
			s.append("Deny");
			break;
		case 3:
			s.append("Relay");
			break;
		case 4:
			s.append("Poll");
			break;
	}
	if (from_->isEnabled())
		s.append(" ").append(from_->text());
	if (to_->isEnabled())
		s.append(" ").append(to_->text());
	return s;
}

void BrowseDialog::setInfos(CupsdConf *conf)
{
	TQWhatsThis::add(type_, conf->comments_.toolTip("browsetype"));
}

TQString BrowseDialog::newAddress(TQWidget *parent, CupsdConf *conf)
{
	BrowseDialog	dlg(parent);
	dlg.setInfos(conf);
	if (dlg.exec())
	{
		return dlg.addressString();
	}
	return TQString::null;
}

TQString BrowseDialog::editAddress(const TQString& s, TQWidget *parent, CupsdConf *conf)
{
	BrowseDialog	dlg(parent);
	dlg.setInfos(conf);
	TQStringList	l = TQStringList::split(TQRegExp("\\s"), s, false);
	if (l.count() > 1)
	{
		if (l[0] == "Send") dlg.type_->setCurrentItem(0);
		else if (l[0] == "Allow") dlg.type_->setCurrentItem(1);
		else if (l[0] == "Deny") dlg.type_->setCurrentItem(2);
		else if (l[0] == "Relay") dlg.type_->setCurrentItem(3);
		else if (l[0] == "Poll") dlg.type_->setCurrentItem(4);
		dlg.slotTypeChanged(dlg.type_->currentItem());
		int	index(1);
		if (dlg.from_->isEnabled())
			dlg.from_->setText(l[index++]);
		if (dlg.to_->isEnabled())
			dlg.to_->setText(l[index]);
	}
	if (dlg.exec())
	{
		return dlg.addressString();
	}
	return TQString::null;
}

void BrowseDialog::slotTypeChanged(int index)
{
	bool	useFrom(true), useTo(true);
	switch (index)
	{
		case 0: useFrom = false; break;
		case 1:
		case 4:
		case 2: useTo = false; break;
	}
	from_->setEnabled(useFrom);
	to_->setEnabled(useTo);
}

#include "browsedialog.moc"
