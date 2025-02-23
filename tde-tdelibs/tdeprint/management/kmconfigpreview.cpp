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

#include "kmconfigpreview.h"

#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqgroupbox.h>
#include <tqlabel.h>

#include <tdelocale.h>
#include <kurlrequester.h>
#include <tdeconfig.h>
#include <kdialog.h>

KMConfigPreview::KMConfigPreview(TQWidget *parent, const char *name)
: KMConfigPage(parent, name)
{
	setPageName(i18n("Preview"));
	setPageHeader(i18n("Preview Settings"));
	setPagePixmap("filefind");

	TQGroupBox *box = new TQGroupBox(0, TQt::Vertical, i18n("Preview Program"), this);

	m_useext = new TQCheckBox(i18n("&Use external preview program"), box);
	m_program = new KURLRequester(box);
	TQLabel	*lab = new TQLabel(box);
	lab->setText(i18n("You can use an external preview program (PS viewer) instead of the "
					  "TDE built-in preview system. Note that if the TDE default PS viewer "
					  "(KGhostView) cannot be found, TDE tries automatically to find another "
					  "external PostScript viewer"));
	lab->setTextFormat(TQt::RichText);

	TQVBoxLayout	*l0 = new TQVBoxLayout(this, 0, KDialog::spacingHint());
	l0->addWidget(box);
	l0->addStretch(1);
	TQVBoxLayout	*l1 = new TQVBoxLayout(box->layout(), KDialog::spacingHint());
	l1->addWidget(lab);
	l1->addWidget(m_useext);
	l1->addWidget(m_program);

	connect(m_useext, TQ_SIGNAL(toggled(bool)), m_program, TQ_SLOT(setEnabled(bool)));
	m_program->setEnabled(false);
}

void KMConfigPreview::loadConfig(TDEConfig *conf)
{
	conf->setGroup("General");
	m_useext->setChecked(conf->readBoolEntry("ExternalPreview", false));
	m_program->setURL(conf->readPathEntry("PreviewCommand", "gv"));
}

void KMConfigPreview::saveConfig(TDEConfig *conf)
{
	conf->setGroup("General");
	conf->writeEntry("ExternalPreview", m_useext->isChecked());
	conf->writePathEntry("PreviewCommand", m_program->url());
}
