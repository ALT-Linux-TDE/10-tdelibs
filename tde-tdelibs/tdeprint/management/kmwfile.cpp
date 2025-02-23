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

#include "kmwfile.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <tqlabel.h>
#include <tqlayout.h>

#include <kurlrequester.h>
#include <tdelocale.h>
#include <tdefiledialog.h>

KMWFile::KMWFile(TQWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::File;
	m_title = i18n("File Selection");
	m_nextpage = KMWizard::Driver;

	m_url = new KURLRequester(this);
	m_url->setMode((KFile::Mode)(KFile::File|KFile::LocalOnly));
	TQLabel	*l1 = new TQLabel(this);
	l1->setText(i18n("<p>The printing will be redirected to a file. Enter here the path "
			 "of the file you want to use for redirection. Use an absolute path or "
			 "the browse button for graphical selection.</p>"));
	TQLabel	*l2 = new TQLabel(i18n("Print to file:"), this);

	TQVBoxLayout	*lay1 = new TQVBoxLayout(this, 0, 30);
	TQVBoxLayout	*lay2 = new TQVBoxLayout(0, 0, 5);
	lay1->addWidget(l1);
	lay1->addLayout(lay2);
	lay1->addStretch(1);
	lay2->addWidget(l2);
	lay2->addWidget(m_url);
}

bool KMWFile::isValid(TQString& msg)
{
	TQFileInfo	fi(m_url->url());
	if (fi.fileName().isEmpty())
	{
		msg = i18n("Empty file name.");
		return false;
	}

	if (!fi.dir().exists())
	{
		msg = i18n("Directory does not exist.");
		return false;
	}

	return true;
}

void KMWFile::updatePrinter(KMPrinter *p)
{
	TQString	dev = TQString::fromLatin1("file:%1").arg(m_url->url());
	p->setDevice(dev);
}
