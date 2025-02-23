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

#include "cupsdjobspage.h"
#include "cupsdconf.h"

#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>

#include <tdelocale.h>
#include <knuminput.h>

CupsdJobsPage::CupsdJobsPage(TQWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	setPageLabel(i18n("Jobs"));
	setHeader(i18n("Print Jobs Settings"));
	setPixmap("document-print");

	keepjobhistory_ = new TQCheckBox(i18n("Preserve job history"), this);
	keepjobfiles_ = new TQCheckBox(i18n("Preserve job files"), this);
	autopurgejobs_ = new TQCheckBox(i18n("Auto purge jobs"), this);
	maxjobs_ = new KIntNumInput(this);
	maxjobsperprinter_ = new KIntNumInput(this);
	maxjobsperuser_ = new KIntNumInput(this);

	maxjobs_->setRange(0, 1000, 1, true);
	maxjobs_->setSteps(1, 10);
	maxjobs_->setSpecialValueText(i18n("Unlimited"));
	maxjobsperprinter_->setRange(0, 1000, 1, true);
	maxjobsperprinter_->setSpecialValueText(i18n("Unlimited"));
	maxjobsperprinter_->setSteps(1, 10);
	maxjobsperuser_->setRange(0, 1000, 1, true);
	maxjobsperuser_->setSpecialValueText(i18n("Unlimited"));
	maxjobsperuser_->setSteps(1, 10);

	TQLabel *l1 = new TQLabel(i18n("Max jobs:"), this);
	TQLabel *l2 = new TQLabel(i18n("Max jobs per printer:"), this);
	TQLabel *l3 = new TQLabel(i18n("Max jobs per user:"), this);

	TQGridLayout	*m1 = new TQGridLayout(this, 7, 2, 10, 7);
	m1->setRowStretch(6, 1);
	m1->setColStretch(1, 1);
	m1->addWidget(keepjobhistory_, 0, 1);
	m1->addWidget(keepjobfiles_, 1, 1);
	m1->addWidget(autopurgejobs_, 2, 1);
	m1->addWidget(l1, 3, 0, TQt::AlignRight);
	m1->addWidget(l2, 4, 0, TQt::AlignRight);
	m1->addWidget(l3, 5, 0, TQt::AlignRight);
	m1->addWidget(maxjobs_, 3, 1);
	m1->addWidget(maxjobsperprinter_, 4, 1);
	m1->addWidget(maxjobsperuser_, 5, 1);

	connect(keepjobhistory_, TQ_SIGNAL(toggled(bool)), TQ_SLOT(historyChanged(bool)));
	keepjobhistory_->setChecked(true);
}

bool CupsdJobsPage::loadConfig(CupsdConf *conf, TQString&)
{
	conf_ = conf;
	keepjobhistory_->setChecked(conf_->keepjobhistory_);
	if (conf_->keepjobhistory_)
	{
		keepjobfiles_->setChecked(conf_->keepjobfiles_);
		autopurgejobs_->setChecked(conf_->autopurgejobs_);
	}
	maxjobs_->setValue(conf_->maxjobs_);
	maxjobsperprinter_->setValue(conf_->maxjobsperprinter_);
	maxjobsperuser_->setValue(conf_->maxjobsperuser_);

	return true;
}

bool CupsdJobsPage::saveConfig(CupsdConf *conf, TQString&)
{
	conf->keepjobhistory_ = keepjobhistory_->isChecked();
	if (conf->keepjobhistory_)
	{
		conf->keepjobfiles_ = keepjobfiles_->isChecked();
		conf->autopurgejobs_ = autopurgejobs_->isChecked();
	}
	conf->maxjobs_ = maxjobs_->value();
	conf->maxjobsperprinter_ = maxjobsperprinter_->value();
	conf->maxjobsperuser_ = maxjobsperuser_->value();

	return true;
}

void CupsdJobsPage::setInfos(CupsdConf *conf)
{
	TQWhatsThis::add(keepjobhistory_, conf->comments_.toolTip("preservejobhistory"));
	TQWhatsThis::add(keepjobfiles_, conf->comments_.toolTip("preservejobfiles"));
	TQWhatsThis::add(autopurgejobs_, conf->comments_.toolTip("autopurgejobs"));
	TQWhatsThis::add(maxjobs_, conf->comments_.toolTip("maxjobs"));
	TQWhatsThis::add(maxjobsperprinter_, conf->comments_.toolTip("maxjobsperprinter"));
	TQWhatsThis::add(maxjobsperuser_, conf->comments_.toolTip("maxjobsperuser"));
}

void CupsdJobsPage::historyChanged(bool on)
{
	keepjobfiles_->setEnabled(on);
	autopurgejobs_->setEnabled(on);
}

#include "cupsdjobspage.moc"
