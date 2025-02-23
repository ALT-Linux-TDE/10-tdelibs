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

#include "kmconfigcupsdir.h"

#include <tqcheckbox.h>
#include <kurlrequester.h>
#include <tdelocale.h>
#include <kdialog.h>
#include <tdeconfig.h>
#include <tqgroupbox.h>
#include <tqlayout.h>
#include <kcursor.h>

KMConfigCupsDir::KMConfigCupsDir(TQWidget *parent)
: KMConfigPage(parent,"ConfigCupsDir")
{
	setPageName(i18n("Folder"));
	setPageHeader(i18n("CUPS Folder Settings"));
	setPagePixmap("folder");

	TQGroupBox *m_dirbox = new TQGroupBox(0, TQt::Vertical, i18n("Installation Folder"), this);
	m_installdir = new KURLRequester(m_dirbox);
	m_installdir->setMode((KFile::Mode)(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly));
	m_stddir = new TQCheckBox(i18n("Standard installation (/)"), m_dirbox);
	m_stddir->setCursor(KCursor::handCursor());

	TQVBoxLayout *lay0 = new TQVBoxLayout(this, 0, KDialog::spacingHint());
	lay0->addWidget(m_dirbox);
	lay0->addStretch(1);
	TQVBoxLayout *lay1 = new TQVBoxLayout(m_dirbox->layout(), 10);
	lay1->addWidget(m_stddir);
	lay1->addWidget(m_installdir);

	connect(m_stddir,TQ_SIGNAL(toggled(bool)),m_installdir,TQ_SLOT(setDisabled(bool)));
	m_stddir->setChecked(true);
}

void KMConfigCupsDir::loadConfig(TDEConfig *conf)
{
	conf->setGroup("CUPS");
	TQString	dir = conf->readPathEntry("InstallDir");
	m_stddir->setChecked(dir.isEmpty());
	m_installdir->setURL(dir);
}

void KMConfigCupsDir::saveConfig(TDEConfig *conf)
{
	conf->setGroup("CUPS");
	conf->writePathEntry("InstallDir",(m_stddir->isChecked() ? TQString::null : m_installdir->url()));
}
