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

#include "kmconfiglpr.h"
#include "lprsettings.h"

#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqgroupbox.h>
#include <tdelocale.h>
#include <tdeconfig.h>

KMConfigLpr::KMConfigLpr(TQWidget *parent, const char *name)
: KMConfigPage(parent, name)
{
	setPageName(i18n("Spooler"));
	setPageHeader(i18n("Spooler Settings"));
	setPagePixmap("gear");

	TQGroupBox	*m_modebox = new TQGroupBox(1, TQt::Vertical, i18n("Spooler"), this);

	m_mode = new TQComboBox(m_modebox);
	m_mode->insertItem("LPR (BSD compatible)");
	m_mode->insertItem("LPRng");

	TQVBoxLayout	*l0 = new TQVBoxLayout(this, 5, 10);
	l0->addWidget(m_modebox);
	l0->addStretch(1);
}

void KMConfigLpr::loadConfig(TDEConfig*)
{
	m_mode->setCurrentItem(LprSettings::self()->mode());
}

void KMConfigLpr::saveConfig(TDEConfig *conf)
{
	LprSettings::self()->setMode((LprSettings::Mode)(m_mode->currentItem()));

	TQString	modestr;
	switch (m_mode->currentItem())
	{
		default:
		case 0: modestr = "LPR"; break;
		case 1: modestr = "LPRng"; break;
	}
	conf->setGroup("LPR");
	conf->writeEntry("Mode", modestr);
}
