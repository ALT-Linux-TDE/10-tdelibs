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

#include "cupsdsplash.h"

#include <tqlabel.h>
#include <tqpixmap.h>
#include <tqlayout.h>
#include <tdelocale.h>
#include <kstandarddirs.h>

CupsdSplash::CupsdSplash(TQWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	setHeader(i18n("Welcome to the CUPS Server Configuration Tool"));
	setPageLabel(i18n("Welcome"));
	setPixmap("go");

	TQVBoxLayout	*main_ = new TQVBoxLayout(this, 10, 10);
	TQHBoxLayout	*sub_ = new TQHBoxLayout(0, 0, 10);
	main_->addLayout(sub_);

	TQLabel	*cupslogo_ = new TQLabel(this);
	TQString	logopath = locate("data", TQString("tdeprint/cups_logo.png"));
	cupslogo_->setPixmap(logopath.isEmpty() ? TQPixmap() : TQPixmap(logopath));
	cupslogo_->setAlignment(TQt::AlignCenter);
	TQLabel	*kupslogo_ = new TQLabel(this);
	logopath = locate("data", TQString("tdeprint/kde_logo.png"));
	kupslogo_->setPixmap(logopath.isEmpty() ? TQPixmap() : TQPixmap(logopath));
	kupslogo_->setAlignment(TQt::AlignCenter);

	TQLabel	*helptxt_ = new TQLabel(this);
	helptxt_->setText(i18n( "<p>This tool will help you to configure graphically the server of the CUPS printing system. "
				"The available options are grouped into sets of related topics and can be accessed "
				"quickly through the icon view located on the left. Each option has a default value that is "
				"shown if it has not been previously set. This default value should be OK in most cases.</p><br>"
				"<p>You can access a short help message for each option using either the '?' button in the "
				"the title bar, or the button at the bottom of this dialog.</p>"));

	sub_->addWidget(cupslogo_);
	sub_->addWidget(kupslogo_);
	main_->addWidget(helptxt_, 1);
}

CupsdSplash::~CupsdSplash()
{
}

bool CupsdSplash::loadConfig(CupsdConf*, TQString&)
{
	return true;
}

bool CupsdSplash::saveConfig(CupsdConf*, TQString&)
{
	return true;
}
