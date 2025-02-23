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

#include "cupsddirpage.h"
#include "cupsdconf.h"
#include "qdirlineedit.h"
#include "qdirmultilineedit.h"

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>

#include <tdelocale.h>

CupsdDirPage::CupsdDirPage(TQWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	setPageLabel(i18n("Folders"));
	setHeader(i18n("Folders Settings"));
	setPixmap("folder");

	datadir_ = new QDirLineEdit(false, this);
	documentdir_ = new QDirLineEdit(false, this);
	fontpath_ = new QDirMultiLineEdit(this);
	requestdir_ = new QDirLineEdit(false, this);
	serverbin_ = new QDirLineEdit(false, this);
	serverfiles_ = new QDirLineEdit(false, this);
	tmpfiles_ = new QDirLineEdit(false, this);

	TQLabel *l1 = new TQLabel(i18n("Data folder:"), this);
	TQLabel *l2 = new TQLabel(i18n("Document folder:"), this);
	TQLabel *l3 = new TQLabel(i18n("Font path:"), this);
	TQLabel *l4 = new TQLabel(i18n("Request folder:"), this);
	TQLabel *l5 = new TQLabel(i18n("Server binaries:"), this);
	TQLabel *l6 = new TQLabel(i18n("Server files:"), this);
	TQLabel *l7 = new TQLabel(i18n("Temporary files:"), this);

	TQGridLayout	*m1 = new TQGridLayout(this, 8, 2, 10, 7);
	m1->setRowStretch(7, 1);
	m1->setColStretch(1, 1);
	m1->addWidget(l1, 0, 0, TQt::AlignRight);
	m1->addWidget(l2, 1, 0, TQt::AlignRight);
	m1->addWidget(l3, 2, 0, TQt::AlignRight|TQt::AlignTop);
	m1->addWidget(l4, 3, 0, TQt::AlignRight);
	m1->addWidget(l5, 4, 0, TQt::AlignRight);
	m1->addWidget(l6, 5, 0, TQt::AlignRight);
	m1->addWidget(l7, 6, 0, TQt::AlignRight);
	m1->addWidget(datadir_, 0, 1);
	m1->addWidget(documentdir_, 1, 1);
	m1->addWidget(fontpath_, 2, 1);
	m1->addWidget(requestdir_, 3, 1);
	m1->addWidget(serverbin_, 4, 1);
	m1->addWidget(serverfiles_, 5, 1);
	m1->addWidget(tmpfiles_, 6, 1);
}

bool CupsdDirPage::loadConfig(CupsdConf *conf, TQString&)
{
	conf_ = conf;
	datadir_->setURL(conf_->datadir_);
	documentdir_->setURL(conf_->documentdir_);
	fontpath_->setURLs(conf_->fontpath_);
	requestdir_->setURL(conf_->requestdir_);
	serverbin_->setURL(conf_->serverbin_);
	serverfiles_->setURL(conf_->serverfiles_);
	tmpfiles_->setURL(conf_->tmpfiles_);

	return true;
}

bool CupsdDirPage::saveConfig(CupsdConf *conf, TQString&)
{
	conf->datadir_ = datadir_->url();
	conf->documentdir_ = documentdir_->url();
	conf->fontpath_ = fontpath_->urls();
	conf->requestdir_ = requestdir_->url();
	conf->serverbin_ = serverbin_->url();
	conf->serverfiles_ = serverfiles_->url();
	conf->tmpfiles_ = tmpfiles_->url();

	return true;
}

void CupsdDirPage::setInfos(CupsdConf *conf)
{
	TQWhatsThis::add(datadir_, conf->comments_.toolTip("datadir"));
	TQWhatsThis::add(documentdir_, conf->comments_.toolTip("documentroot"));
	TQWhatsThis::add(fontpath_, conf->comments_.toolTip("fontpath"));
	TQWhatsThis::add(requestdir_, conf->comments_.toolTip("requestroot"));
	TQWhatsThis::add(serverbin_, conf->comments_.toolTip("serverbin"));
	TQWhatsThis::add(serverfiles_, conf->comments_.toolTip("serverroot"));
	TQWhatsThis::add(tmpfiles_, conf->comments_.toolTip("tempdir"));
}
