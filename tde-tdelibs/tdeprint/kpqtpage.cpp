/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
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

#include "kpqtpage.h"
#include "kprinter.h"
#include "kxmlcommand.h"
#include "driver.h"
#include "util.h"

#include <tqcombobox.h>
#include <tqbuttongroup.h>
#include <tqradiobutton.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>

#include <kiconloader.h>
#include <tdelocale.h>
#include <kdebug.h>

#define ORIENT_PORTRAIT_ID	0
#define ORIENT_LANDSCAPE_ID	1

#define COLORMODE_COLOR_ID	0
#define COLORMODE_GRAYSCALE_ID	1

#define NUP_1		0
#define NUP_2		1
#define NUP_4		2
#define NUP_OTHER	3

//*****************************************************************************************************

KPQtPage::KPQtPage(TQWidget *parent, const char *name)
: KPrintDialogPage(parent,name)
{
	init();
}

KPQtPage::KPQtPage(DrMain *driver, TQWidget *parent, const char *name)
: KPrintDialogPage(0, (driver && driver->findOption("PageSize") ? driver : 0), parent, name)
{
	init();
}

KPQtPage::~KPQtPage()
{
}

void KPQtPage::init()
{
        //WhatsThis strings.... (added by pfeifle@kde.org)
	TQString whatsThisColorModeOtPageLabel = i18n( " <qt> "
			" <b>Selection of color mode:</b> You can choose between 2 options: "
			" <ul><li><b>Color</b> and</li> "
                        " <li><b>Grayscale</b></li></ul> "
			" <b>Note:</b> This selection field may be grayed out and made inactive. "
			" This happens if TDEPrint can not retrieve "
			" enough information about your print file. In this case the embedded color- or grayscale information "
			" of your printfile, and the default handling of the printer take precedence. "
			" </qt>" );
	TQString whatsThisPageSizeOtPageLabel = i18n( " <qt> "
			" <b>Selection of page size:</b> Select paper size to be printed on from "
			" the drop-down menu. "
			" <p>The exact list of choices depends on the printer driver (\"PPD\") you have installed. "
			" </qt>" );
	TQString whatsThisPagesPerSheetOtPageLabel = i18n( " <qt> "
			" <b>Selection of pages per sheet:</b> "
                        " You can choose to print more than one page onto each sheet of paper. "
                        " This is sometimes useful to save paper. "
                        " <p><b>Note 1:</b> the page images get scaled down accordingly to print 2 or 4 pages per sheet. "
                        " The page image does not get scaled if you print 1 page per sheet (the default setting.). "
                        " <p><b>Note 2:</b> If you select multiple pages per sheet here, the scaling and re-arranging is done "
                        " by your printing system. "
			" <p><b>Note 3, regarding \"Other\":</b> You cannot really select <em>Other</em> as the number of "
			" pages to print on one sheet.\"Other\" is checkmarked here for information purposes only. "
			" <p>To select 8, 9, 16 or other numbers of "
			" pages per sheet: "
			" <ul> "
			" <li> go to the tab headlined \"Filter\"</li>"
			" <li> enable the <em>Multiple Pages per Sheet</em> filter </li>"
			" <li> and configure it (bottom-most button on the right of the \"Filters\" tab). </li>"
			" </ul>"
			" </qt>" );
	TQString whatsThisOrientationOtPageLabel = i18n( " <qt> "
			" <b>Selection of image orientation:</b> Orientation of the printed "
                        " pageimage on your paper is controlled by the radio buttons. By default, "
                        " the orientation is <em>Portrait</em> "
                        " <p>You can select 2 alternatives: "
                        " <ul> "
                        " <li> <b>Portrait.</b>.Portrait is the default setting. </li> "
                        " <li> <b>Landscape.</b> </li> "
                        " </ul> "
                        " The icon changes according to your selection."
			" </qt>" );
	setTitle(i18n("Print Format"));

	// widget creation
	m_pagesize = new TQComboBox(this);
          TQWhatsThis::add(m_pagesize, whatsThisPageSizeOtPageLabel);
	TQLabel	*m_pagesizelabel = new TQLabel(i18n("Page s&ize:"), this);
	m_pagesizelabel->setAlignment(TQt::AlignVCenter|TQt::AlignRight);
	m_pagesizelabel->setBuddy(m_pagesize);
	m_orientbox = new TQButtonGroup(0, TQt::Vertical, i18n("Orientation"), this);
          TQWhatsThis::add(m_orientbox, whatsThisOrientationOtPageLabel);
	m_colorbox = new TQButtonGroup(0, TQt::Vertical, i18n("Color Mode"), this);
          TQWhatsThis::add(m_colorbox, whatsThisColorModeOtPageLabel);
	TQRadioButton	*m_portrait = new TQRadioButton(i18n("&Portrait"), m_orientbox);
          TQWhatsThis::add(m_portrait, whatsThisOrientationOtPageLabel);

	TQRadioButton	*m_landscape = new TQRadioButton(i18n("&Landscape"), m_orientbox);
          TQWhatsThis::add(m_landscape, whatsThisOrientationOtPageLabel);

	m_orientpix = new TQLabel(m_orientbox);
	m_orientpix->setAlignment(TQt::AlignCenter);
          TQWhatsThis::add(m_orientpix, whatsThisOrientationOtPageLabel);

	TQRadioButton	*m_color = new TQRadioButton(i18n("Colo&r"), m_colorbox);
          TQWhatsThis::add(m_color, whatsThisColorModeOtPageLabel);

	TQRadioButton	*m_grayscale = new TQRadioButton(i18n("&Grayscale"), m_colorbox);
	m_colorpix = new TQLabel(m_colorbox);
	m_colorpix->setAlignment(TQt::AlignCenter);
          TQWhatsThis::add(m_colorpix, whatsThisColorModeOtPageLabel);

	m_nupbox = new TQButtonGroup(0, TQt::Vertical, i18n("Pages per Sheet"), this);
        //  TQWhatsThis::add(m_nupbox, whatsThisPagesPerSheetOtPageLabel);
	TQRadioButton	*m_nup1 = new TQRadioButton("&1", m_nupbox);
          TQWhatsThis::add(m_nup1, whatsThisPagesPerSheetOtPageLabel);
	TQRadioButton	*m_nup2 = new TQRadioButton("&2", m_nupbox);
          TQWhatsThis::add(m_nup2, whatsThisPagesPerSheetOtPageLabel);
	TQRadioButton	*m_nup4 = new TQRadioButton("&4", m_nupbox);
          TQWhatsThis::add(m_nup4, whatsThisPagesPerSheetOtPageLabel);
	TQRadioButton	*m_nupother = new TQRadioButton(i18n("Ot&her"), m_nupbox);
          TQWhatsThis::add(m_nupother, whatsThisPagesPerSheetOtPageLabel);

	m_nuppix = new TQLabel(m_nupbox);
	m_nuppix->setAlignment(TQt::AlignCenter);
          TQWhatsThis::add(m_nuppix, whatsThisPagesPerSheetOtPageLabel);

	// layout creation
	TQGridLayout	*lay0 = new TQGridLayout(this, 3, 2, 0, 10);
	lay0->setRowStretch(1,1);
	lay0->setRowStretch(2,1);
	lay0->addWidget(m_pagesizelabel,0,0);
	lay0->addWidget(m_pagesize,0,1);
	lay0->addWidget(m_orientbox,1,0);
	lay0->addWidget(m_colorbox,1,1);
	lay0->addWidget(m_nupbox,2,0);
	TQGridLayout	*lay1 = new TQGridLayout(m_orientbox->layout(), 2, 2, 10);
	lay1->addWidget(m_portrait,0,0);
	lay1->addWidget(m_landscape,1,0);
	lay1->addMultiCellWidget(m_orientpix,0,1,1,1);
	TQGridLayout	*lay2 = new TQGridLayout(m_colorbox->layout(), 2, 2, 10);
	lay2->addWidget(m_color,0,0);
	lay2->addWidget(m_grayscale,1,0);
	lay2->addMultiCellWidget(m_colorpix,0,1,1,1);
	TQGridLayout	*lay3 = new TQGridLayout(m_nupbox->layout(), 4, 2, 5);
	lay3->addWidget(m_nup1,0,0);
	lay3->addWidget(m_nup2,1,0);
	lay3->addWidget(m_nup4,2,0);
	lay3->addWidget(m_nupother,3,0);
	lay3->addMultiCellWidget(m_nuppix,0,3,1,1);

	// initialization
	m_portrait->setChecked(true);
	slotOrientationChanged(0);
	m_color->setChecked(true);
	slotColorModeChanged(0);
	m_nup1->setChecked(true);
	slotNupChanged(0);

	if (!KXmlCommandManager::self()->checkCommand("psnup"))
		m_nupbox->setEnabled(false);
	if (KPrinter::applicationType() != KPrinter::Dialog
			&& KPrinter::applicationType() >= 0 )
	{
		m_orientbox->setEnabled(false);
		m_colorbox->setEnabled(false);
		m_pagesize->setEnabled(driver());
		m_pagesizelabel->setEnabled(driver());
	}

	if (!driver())
	{
		for (int i=0; i<KPrinter::NPageSize-1; i++)
			m_pagesize->insertItem(i18n(page_sizes[i].text));
		// default page size to locale settings
		m_pagesize->setCurrentItem(findIndex((KPrinter::PageSize)(TDEGlobal::locale()->pageSize())));
	}
	else
	{
		DrListOption	*lopt = static_cast<DrListOption*>(driver()->findOption("PageSize"));
		TQPtrListIterator<DrBase>	it(*(lopt->choices()));
		for (; it.current(); ++it)
		{
			m_pagesize->insertItem(it.current()->get("text"));
			if (it.current() == lopt->currentChoice())
				m_pagesize->setCurrentItem(m_pagesize->count()-1);
		}
	}

	// connections
	connect(m_orientbox,TQ_SIGNAL(clicked(int)),TQ_SLOT(slotOrientationChanged(int)));
	connect(m_colorbox,TQ_SIGNAL(clicked(int)),TQ_SLOT(slotColorModeChanged(int)));
	connect(m_nupbox,TQ_SIGNAL(clicked(int)),TQ_SLOT(slotNupChanged(int)));
}

void KPQtPage::slotOrientationChanged(int ID)
{
	m_orientpix->setPixmap(UserIcon((ID == ORIENT_PORTRAIT_ID ? "tdeprint_portrait" : "tdeprint_landscape")));
}

void KPQtPage::slotColorModeChanged(int ID)
{
	m_colorpix->setPixmap(UserIcon((ID == COLORMODE_COLOR_ID ? "tdeprint_color" : "tdeprint_grayscale")));
}

void KPQtPage::slotNupChanged(int ID)
{
	TQString	pixstr;
	switch (ID)
	{
		case NUP_1: pixstr = "tdeprint_nup1"; break;
		case NUP_2: pixstr = "tdeprint_nup2"; break;
		case NUP_4: pixstr = "tdeprint_nup4"; break;
		case NUP_OTHER: pixstr = "tdeprint_nupother"; break;
	}
	m_nuppix->setPixmap(UserIcon(pixstr));
}

void KPQtPage::setOptions(const TQMap<TQString,TQString>& opts)
{
	int 	ID = (opts["kde-orientation"] == "Landscape" ? ORIENT_LANDSCAPE_ID : ORIENT_PORTRAIT_ID);
	m_orientbox->setButton(ID);
	slotOrientationChanged(ID);
	ID = (opts["kde-colormode"] == "GrayScale" ? COLORMODE_GRAYSCALE_ID : COLORMODE_COLOR_ID);
	m_colorbox->setButton(ID);
	slotColorModeChanged(ID);
	if (driver())
	{
		TQString	val = opts["PageSize"];
		if (!val.isEmpty())
		{
			DrListOption	*opt = static_cast<DrListOption*>(driver()->findOption("PageSize"));
			DrBase	*ch = opt->findChoice(val);
			if (ch)
				m_pagesize->setCurrentItem(opt->choices()->findRef(ch));
		}
	}
	else if (!opts["kde-pagesize"].isEmpty())
		m_pagesize->setCurrentItem(findIndex(opts["kde-pagesize"].toInt()));
	ID = NUP_1;
	if (opts["_kde-filters"].find("psnup") != -1)
	{
               if (opts.contains("_kde-psnup-nup")) {
                       ID = opts["_kde-psnup-nup"].toInt();
                       if (ID == 1 || ID == 2 || ID == 4)
                       {
                               if (ID == 4) ID = 3;
                               ID--;
                       }
                       else
                       {
                               ID = NUP_OTHER;
                       }
               }
               else
               {
                       ID = NUP_1;
               }
	}
	m_nupbox->setButton(ID);
	slotNupChanged(ID);

	if ( m_orientbox->isEnabled() )
		m_orientbox->setDisabled( opts[ "kde-orientation-fixed" ] == "1" );
	if ( m_pagesize->isEnabled() )
		m_pagesize->setDisabled( opts[ "kde-pagesize-fixed" ] == "1" );
}

void KPQtPage::getOptions(TQMap<TQString,TQString>& opts, bool incldef)
{
	opts["kde-orientation"] = (m_orientbox->id(m_orientbox->selected()) == ORIENT_LANDSCAPE_ID ? "Landscape" : "Portrait");
	opts["kde-colormode"] = (m_colorbox->id(m_colorbox->selected()) == COLORMODE_GRAYSCALE_ID ? "GrayScale" : "Color");
	if (driver())
	{
		DrListOption	*opt = static_cast<DrListOption*>(driver()->findOption("PageSize"));
		if (opt)
		{
			DrBase	*ch = opt->choices()->at(m_pagesize->currentItem());
			if (ch && (incldef || ch->name() != opt->get("default")))
			{
				opts["PageSize"] = ch->name();
			}
		}
	}
	else
		opts["kde-pagesize"] = TQString::number(page_sizes[m_pagesize->currentItem()].ID);
	int	ID = m_nupbox->id(m_nupbox->selected());
	TQString	s = opts["_kde-filters"];
	if (ID == NUP_1)
	{
		opts.remove("_kde-psnup-nup");
	}
	else if (ID != NUP_OTHER)
	{
		int	nup(ID == NUP_2 ? 2 : 4);
		if (s.find("psnup") == -1)
		{
			TQStringList	fl = TQStringList::split(',', s, false);
			KXmlCommandManager::self()->insertCommand(fl, "psnup");
			s = fl.join(",");
		}
		opts["_kde-psnup-nup"] = TQString::number(nup);
	}
	opts["_kde-filters"] = s;
}
#include "kpqtpage.moc"
