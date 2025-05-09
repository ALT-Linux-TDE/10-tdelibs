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

#include "kmconfigjobs.h"

#include <tqgroupbox.h>
#include <tqlayout.h>

#include <knuminput.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <kdialog.h>

KMConfigJobs::KMConfigJobs(TQWidget *parent, const char *name)
: KMConfigPage(parent, name)
{
	setPageName(i18n("Jobs"));
	setPageHeader(i18n("Print Job Settings"));
	setPagePixmap("application-x-executable");

	TQGroupBox	*box = new TQGroupBox(0, TQt::Vertical, i18n("Jobs Shown"), this);

	m_limit = new KIntNumInput(box);
	m_limit->setRange(0, 9999, 1, true);
	m_limit->setSpecialValueText(i18n("Unlimited"));
	m_limit->setLabel(i18n("Maximum number of jobs shown:"));

	TQVBoxLayout	*l0 = new TQVBoxLayout(this, 0, KDialog::spacingHint());
	l0->addWidget(box, 0);
	l0->addStretch(1);
	TQVBoxLayout	*l1 = new TQVBoxLayout(box->layout(), KDialog::spacingHint());
	l1->addWidget(m_limit);
}

void KMConfigJobs::loadConfig(TDEConfig *conf)
{
	conf->setGroup("Jobs");
	m_limit->setValue(conf->readNumEntry("Limit", 0));
}

void KMConfigJobs::saveConfig(TDEConfig *conf)
{
	conf->setGroup("Jobs");
	conf->writeEntry("Limit", m_limit->value());
}

#include "kmconfigjobs.moc"
