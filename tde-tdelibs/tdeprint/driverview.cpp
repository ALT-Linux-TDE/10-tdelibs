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

#include "driverview.h"
#include "droptionview.h"
#include "driveritem.h"
#include "driver.h"

#include <tqlistview.h>
#include <tqsplitter.h>
#include <tqheader.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>
#include <tdelocale.h>

DrListView::DrListView(TQWidget *parent, const char *name)
: TDEListView(parent,name)
{
	addColumn("");
	header()->hide();
	setFrameStyle(TQFrame::WinPanel|TQFrame::Sunken);
	setSorting(-1);
}

//****************************************************************************************************

DriverView::DriverView(TQWidget *parent, const char *name)
: TQWidget(parent,name)
{
	//WhatsThis strings.... (added by pfeifle@kde.org)
	TQString whatsThisPPDOptionsDriverPage = i18n( " <qt> "
			" <b>List of Driver Options (from PPD)</b>. "
			" <p>The upper pane of this dialog page contains all printjob options as laid "
			" down in the printer's description file (PostScript Printer Description == 'PPD') </p>"
			" <p>Click on any item in the list and watch the lower pane of this dialog page "
			" display the available values. </p> "
			" <p>Set the values as needed. Then use one of the pushbuttons below to proceed:</p> "
			" <ul> "
			" <li><em>'Save'</em> your settings if you want to re-use "
			" them in your next job(s) too. <em>'Save'</em> will store your settings permanently until "
			" you change them again. </li>."
			" <li>Click <em>'OK'</em> (without a prior click on <em>'Save'</em>, if you want to use "
			" your selected settings just once, for the next print job. <em>'OK'</em> "
			" will forget your current settings when kprinter is closed again, and will start next time "
			" with the previously saved defaults. </li>"
			" <li><em>'Cancel'</em> will not change anything. If you proceed to print after clicking "
			" <em>'Cancel'</em>, the job will print with the default settings of this queue. "
			" </ul>"
			" <p><b>Note.</b> The number of available job options depends strongly on the actual "
			" driver used for your print queue. <em>'Raw'</em> queues do not have a driver or a  "
			" PPD. For raw queues this tab page is not loaded by TDEPrint, and thus is not present "
			" in the kprinter dialog.</p> "
			" </qt>" );

	TQString whatsThisOptionSettingsDriverPage = i18n( " <qt> "
			" <b>List of Possible Values for given Option (from PPD)</b>. "
			" <p>The lower pane of this dialog page contains all possible values of the printoption "
			" highlighted above, as laid "
			" down in the printer's description file (PostScript Printer Description == 'PPD') </p>"
			" <p>Select the value you want and proceed. </p> "
			" <p>Then use one of the pushbuttons below to leave this dialog:</p> "
			" <ul> "
			" <li><em>'Save'</em> your settings if you want to re-use "
			" them in your next job(s) too. <em>'Save'</em> will store your settings permanently until "
			" you change them again. </li>."
			" <li>Click <em>'OK'</em> if you want to use your selected settings just once, for the "
			" next print job. <em>'OK'</em> "
			" will forget your current settings when kprinter is closed again, and will start next time "
			" with your previous defaults. </li>"
			" <li><em>'Cancel'</em> will not change anything. If you proceed to print after clicking "
			" <em>'Cancel'</em>, the job will print with the default settings of this queue. "
			" </ul>"
			" <p><b>Note.</b> The number of available job options depends strongly on the actual "
			" driver used for your print queue. <em>'Raw'</em> queues do not have a driver or a  "
			" PPD. For raw queues this tab page is not loaded by TDEPrint, and thus is not present "
			" in the kprinter dialog.</p> "
			" </qt>" );

	m_driver = 0;

	TQSplitter	*splitter = new TQSplitter(this);
	splitter->setOrientation(TQt::Vertical);

	TQVBoxLayout     *vbox = new TQVBoxLayout(this, 0, 10);
	vbox->addWidget(splitter);

	m_view = new DrListView(splitter);
	  TQWhatsThis::add(m_view, whatsThisPPDOptionsDriverPage);
	m_optview = new DrOptionView(splitter);
	  TQWhatsThis::add(m_optview, whatsThisOptionSettingsDriverPage);

	connect(m_view,TQ_SIGNAL(selectionChanged(TQListViewItem*)),m_optview,TQ_SLOT(slotItemSelected(TQListViewItem*)));
	connect(m_optview,TQ_SIGNAL(changed()),TQ_SLOT(slotChanged()));
}

DriverView::~DriverView()
{
}

void DriverView::setDriver(DrMain *driver)
{
	m_driver = driver;
	if (m_driver)
	{
		m_view->clear();
		m_driver->createTreeView(m_view);
		slotChanged();
	}
}

void DriverView::slotChanged()
{
	if (m_driver)
	{
		m_conflict = m_driver->checkConstraints();
		((DriverItem*)m_view->firstChild())->updateConflict();
	}
}

void DriverView::setOptions(const TQMap<TQString,TQString>& opts)
{
	if (m_driver)
	{
		m_driver->setOptions(opts);
		static_cast<DriverItem*>( m_view->firstChild() )->updateTextRecursive();
		slotChanged();
		m_optview->slotItemSelected(m_view->currentItem());
	}
}

void DriverView::getOptions(TQMap<TQString,TQString>& opts, bool incldef)
{
	if (m_driver)
		m_driver->getOptions(opts,incldef);
}

void DriverView::setAllowFixed(bool on)
{
	m_optview->setAllowFixed(on);
}
#include "driverview.moc"
