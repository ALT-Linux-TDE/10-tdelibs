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

#include "kmpropbanners.h"
#include "kmprinter.h"
#include "kmwizard.h"
#include "kmwbanners.h"

#include <tqlabel.h>
#include <tqlayout.h>
#include <tdelocale.h>

KMPropBanners::KMPropBanners(TQWidget *parent, const char *name)
: KMPropWidget(parent,name)
{
	m_startbanner = new TQLabel(this);
	m_stopbanner = new TQLabel(this);

	TQLabel	*l1 = new TQLabel(i18n("&Starting banner:"), this);
	TQLabel	*l2 = new TQLabel(i18n("&Ending banner:"), this);

	l1->setBuddy(m_startbanner);
	l2->setBuddy(m_stopbanner);

	TQGridLayout	*main_ = new TQGridLayout(this, 3, 2, 10, 10);
	main_->setColStretch(1,1);
	main_->setRowStretch(2,1);
	main_->addWidget(l1,0,0);
	main_->addWidget(l2,1,0);
	main_->addWidget(m_startbanner,0,1);
	main_->addWidget(m_stopbanner,1,1);

	m_title = i18n("Banners");
	m_header = i18n("Banner Settings");
	m_pixmap = "edit-copy";
}

KMPropBanners::~KMPropBanners()
{
}

void KMPropBanners::setPrinter(KMPrinter *p)
{
	if (p && p->isPrinter())
	{
		TQStringList	l = TQStringList::split(',',p->option("kde-banners"),false);
		while ( l.count() < 2 )
			l.append( "none" );
		m_startbanner->setText(i18n(mapBanner(l[0]).utf8()));
		m_stopbanner->setText(i18n(mapBanner(l[1]).utf8()));
		emit enable(true);
		emit enableChange(p->isLocal());
	}
	else
	{
		emit enable(false);
		m_startbanner->setText("");
		m_stopbanner->setText("");
	}
}

void KMPropBanners::configureWizard(KMWizard *w)
{
	w->configure(KMWizard::Banners,KMWizard::Banners,true);
}
