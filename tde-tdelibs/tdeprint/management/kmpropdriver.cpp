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

#include "kmpropdriver.h"
#include "kmprinter.h"
#include "kmwizard.h"

#include <tqlabel.h>
#include <tqlayout.h>
#include <tdelocale.h>

KMPropDriver::KMPropDriver(TQWidget *parent, const char *name)
: KMPropWidget(parent,name)
{
	m_manufacturer = new TQLabel("",this);
	m_model = new TQLabel("",this);
	m_driverinfo = new TQLabel("",this);
	m_driverinfo->setTextFormat(TQt::RichText);

	TQLabel	*l1 = new TQLabel(i18n("Manufacturer:"), this);
	TQLabel	*l2 = new TQLabel(i18n("Printer model:"), this);
	TQLabel	*l3 = new TQLabel(i18n("Driver info:"), this);

	// layout
	TQGridLayout	*main_ = new TQGridLayout(this, 4, 2, 10, 7);
	main_->setColStretch(0,0);
	main_->setColStretch(1,1);
	main_->setRowStretch(3,1);
	main_->addWidget(l1,0,0);
	main_->addWidget(l2,1,0);
	main_->addWidget(l3,2,0,TQt::AlignTop|TQt::AlignLeft);
	main_->addWidget(m_manufacturer,0,1);
	main_->addWidget(m_model,1,1);
	main_->addWidget(m_driverinfo,2,1);

	m_pixmap = "gear";
	m_title = i18n("Driver");
	m_header = i18n("Driver Settings");
}

KMPropDriver::~KMPropDriver()
{
}

void KMPropDriver::setPrinter(KMPrinter *p)
{
	if (p && p->isPrinter())
	{
		m_manufacturer->setText(p->manufacturer());
		m_model->setText(p->model());
		m_driverinfo->setText(p->driverInfo());
		emit enable(true);
		emit enableChange(p->isLocal());
	}
	else
	{
		emit enable(false);
		m_manufacturer->setText("");
		m_model->setText("");
		m_driverinfo->setText("");
	}
}

void KMPropDriver::configureWizard(KMWizard *w)
{
	w->configure(KMWizard::Driver,KMWizard::DriverTest,true);
}
