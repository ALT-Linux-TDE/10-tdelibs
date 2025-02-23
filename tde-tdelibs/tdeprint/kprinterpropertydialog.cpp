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

#include "kprinterpropertydialog.h"
#include "kprintdialogpage.h"
#include "kmfactory.h"
#include "kmuimanager.h"
#include "kmvirtualmanager.h"
#include "kmprinter.h"
#include "driver.h"

#include <tdemessagebox.h>
#include <tqtabwidget.h>
#include <tdelocale.h>
#include <kpushbutton.h>
#include <kguiitem.h>

KPrinterPropertyDialog::KPrinterPropertyDialog(KMPrinter *p, TQWidget *parent, const char *name)
: KDialogBase(parent, name, true, TQString::null, KDialogBase::Ok|KDialogBase::Cancel|KDialogBase::User1, KDialogBase::Ok, false, KStdGuiItem::save()),
  m_printer(p), m_driver(0), m_current(0)
{
	m_pages.setAutoDelete(false);

	// set a margin
	m_tw = new TQTabWidget(this);
	m_tw->setMargin(10);
	connect(m_tw,TQ_SIGNAL(currentChanged(TQWidget*)),TQ_SLOT(slotCurrentChanged(TQWidget*)));
	setMainWidget(m_tw);

	if (m_printer)
		m_options = (m_printer->isEdited() ? m_printer->editedOptions() : m_printer->defaultOptions());
}

KPrinterPropertyDialog::~KPrinterPropertyDialog()
{
	delete m_driver;
}

void KPrinterPropertyDialog::slotCurrentChanged(TQWidget *w)
{
	if (m_current) m_current->getOptions(m_options,true);
	m_current = (KPrintDialogPage*)w;
	if (m_current) m_current->setOptions(m_options);
}

void KPrinterPropertyDialog::addPage(KPrintDialogPage *page)
{
	m_tw->addTab(page,page->title());
	m_pages.append(page);
}

bool KPrinterPropertyDialog::synchronize()
{
	if (m_current) m_current->getOptions(m_options,true);
	TQString	msg;
	TQPtrListIterator<KPrintDialogPage>	it(m_pages);
	for (;it.current();++it)
	{
		it.current()->setOptions(m_options);
		if (!it.current()->isValid(msg))
		{
			KMessageBox::error(this, msg.prepend("<qt>").append("</qt>"), i18n("Printer Configuration"));
			return false;
		}
	}
	return true;
}

void KPrinterPropertyDialog::setOptions(const TQMap<TQString,TQString>& opts)
{
	// merge the 2 options sets
	for (TQMap<TQString,TQString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
		m_options[it.key()] = it.data();
	// update all existing pages
	TQPtrListIterator<KPrintDialogPage>	it(m_pages);
	for (; it.current(); ++it)
		it.current()->setOptions(m_options);
}

void KPrinterPropertyDialog::getOptions(TQMap<TQString,TQString>& opts, bool incldef)
{
	collectOptions(opts, incldef);
}

void KPrinterPropertyDialog::collectOptions(TQMap<TQString,TQString>& opts, bool incldef)
{
	TQPtrListIterator<KPrintDialogPage>	it(m_pages);
	for (;it.current();++it)
		it.current()->getOptions(opts,incldef);
}

void KPrinterPropertyDialog::slotOk()
{
	if (!synchronize())
		return;
	KDialogBase::slotOk();
}

void KPrinterPropertyDialog::slotUser1()
{
	if (m_printer && synchronize())
	{
		TQMap<TQString,TQString>	opts;
		collectOptions(opts, false);
		m_printer->setDefaultOptions(opts);
		m_printer->setEditedOptions(TQMap<TQString,TQString>());
		m_printer->setEdited(false);
		KMFactory::self()->virtualManager()->triggerSave();
	}
}

void KPrinterPropertyDialog::enableSaveButton(bool state)
{
	showButton(KDialogBase::User1, state);
}

void KPrinterPropertyDialog::setupPrinter(KMPrinter *pr, TQWidget *parent)
{
	KPrinterPropertyDialog	dlg(pr,parent,"PropertyDialog");
	KMFactory::self()->uiManager()->setupPropertyDialog(&dlg);
	if (dlg.m_pages.count() == 0)
		KMessageBox::information(parent,i18n("No configurable options for that printer."),i18n("Printer Configuration"));
	else if (dlg.exec())
	{
		TQMap<TQString,TQString>	opts;
		dlg.collectOptions(opts, false);
		pr->setEditedOptions(opts);
		pr->setEdited(true);
	}
}
#include "kprinterpropertydialog.moc"
