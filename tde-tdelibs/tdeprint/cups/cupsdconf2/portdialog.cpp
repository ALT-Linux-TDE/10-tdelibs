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

#include "portdialog.h"
#include "cupsdconf.h"

#include <tqlineedit.h>
#include <tqspinbox.h>
#include <tqcheckbox.h>
#include <tqpushbutton.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>

#include <tdelocale.h>

PortDialog::PortDialog(TQWidget *parent, const char *name)
	: KDialogBase(parent, name, true, TQString::null, Ok|Cancel, Ok, true)
{
	TQWidget	*dummy = new TQWidget(this);
	setMainWidget(dummy);
	address_ = new TQLineEdit(dummy);
	port_ = new TQSpinBox(0, 9999, 1, dummy);
	port_->setValue(631);
	usessl_ = new TQCheckBox(i18n("Use SSL encryption"), dummy);

	TQLabel	*l1 = new TQLabel(i18n("Address:"), dummy);
	TQLabel	*l2 = new TQLabel(i18n("Port:"), dummy);

	TQVBoxLayout	*m1 = new TQVBoxLayout(dummy, 0, 10);
	TQGridLayout	*m2 = new TQGridLayout(0, 3, 2, 0, 5);
	m1->addLayout(m2);
	m2->addWidget(l1, 0, 0, TQt::AlignRight);
	m2->addWidget(l2, 1, 0, TQt::AlignRight);
	m2->addMultiCellWidget(usessl_, 2, 2, 0, 1);
	m2->addWidget(address_, 0, 1);
	m2->addWidget(port_, 1, 1);

	setCaption(i18n("Listen To"));
	resize(250, 100);
}

TQString PortDialog::listenString()
{
	TQString s;
	if (usessl_->isChecked())
		s.append("SSLListen ");
	else
		s.append("Listen ");
	if (!address_->text().isEmpty())
		s.append(address_->text());
	else
		s.append("*");
	s.append(":").append(port_->text());
	return s;
}

void PortDialog::setInfos(CupsdConf *conf)
{
	TQWhatsThis::add(address_, conf->comments_.toolTip("address"));
	TQWhatsThis::add(port_, conf->comments_.toolTip("port"));
	TQWhatsThis::add(usessl_, conf->comments_.toolTip("usessl"));
}

TQString PortDialog::newListen(TQWidget *parent, CupsdConf *conf)
{
	PortDialog	dlg(parent);
	dlg.setInfos(conf);
	if (dlg.exec())
	{
		return dlg.listenString();
	}
	return TQString::null;
}

TQString PortDialog::editListen(const TQString& s, TQWidget *parent, CupsdConf *conf)
{
	PortDialog	dlg(parent);
	dlg.setInfos(conf);
	int	p = s.find(' ');
	if (p != -1)
	{
		dlg.usessl_->setChecked(s.left(p).startsWith("SSL"));
		TQString	addr = s.mid(p+1).stripWhiteSpace();
		int p1 = addr.find(':');
		if (p1 == -1)
		{
			dlg.address_->setText(addr);
			dlg.port_->setValue(631);
		}
		else
		{
			dlg.address_->setText(addr.left(p1));
			dlg.port_->setValue(addr.mid(p1+1).toInt());
		}
	}
	if (dlg.exec())
	{
		return dlg.listenString();
	}
	return TQString::null;
}
