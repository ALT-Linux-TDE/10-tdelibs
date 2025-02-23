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

#include "kmpropertypage.h"
#include "kmpropwidget.h"
#include "kmpropcontainer.h"
#include "kmprinter.h"
#include "kmfactory.h"
#include "kmuimanager.h"

#include "kmpropgeneral.h"

#include <tqvbox.h>
#include <kiconloader.h>

KMPropertyPage::KMPropertyPage(TQWidget *parent, const char *name)
: CJanusWidget(parent,name)
{
	m_widgets.setAutoDelete(false);

	initialize();
}

KMPropertyPage::~KMPropertyPage()
{
}

void KMPropertyPage::setPrinter(KMPrinter *p)
{
	TQPtrListIterator<KMPropWidget>	it(m_widgets);
	for (;it.current();++it)
		it.current()->setPrinterBase(p);
}

void KMPropertyPage::addPropPage(KMPropWidget *w)
{
	if (w)
	{
		m_widgets.append(w);
		KMPropContainer	*ctn = new KMPropContainer(this,"Container");
		ctn->setWidget(w);
		connect(ctn,TQ_SIGNAL(enable(bool)),TQ_SLOT(slotEnable(bool)));

		TQPixmap icon = TDEGlobal::instance()->iconLoader()->loadIcon(
		                                                           w->pixmap(),
		                                                           TDEIcon::NoGroup,
		                                                           TDEIcon::SizeMedium
 		                                                          );
		addPage(ctn,w->title(),w->header(),icon);
	}
}

void KMPropertyPage::slotEnable(bool on)
{
	TQWidget	*w = (TQWidget*)(sender());
	if (on)
		enablePage(w);
	else
		disablePage(w);
}

void KMPropertyPage::initialize()
{
	// add General page
	addPropPage(new KMPropGeneral(this, "General"));
	// add plugin specific pages
	KMFactory::self()->uiManager()->setupPropertyPages(this);
}

void KMPropertyPage::reload()
{
	clearPages();
	m_widgets.clear();
	initialize();
	setPrinter(0);
}

#include "kmpropertypage.moc"
