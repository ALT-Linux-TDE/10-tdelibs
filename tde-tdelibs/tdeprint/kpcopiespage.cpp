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

#include "kpcopiespage.h"
#include "kmfactory.h"
#include "kmuimanager.h"
#include "kprinter.h"
#include "kxmlcommand.h"

#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqspinbox.h>
#include <tqbuttongroup.h>
#include <tqradiobutton.h>
#include <tqlineedit.h>
#include <tqcheckbox.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqlayout.h>

#include <tdeapplication.h>
#include <tdelocale.h>
#include <kiconloader.h>
#include <kseparator.h>

KPCopiesPage::KPCopiesPage(KPrinter *prt, TQWidget *parent, const char *name)
: KPrintDialogPage(parent,name)
{
	//WhatsThis strings.... (added by pfeifle@kde.org)
	TQString whatsThisPageSelectionLabel = i18n(  " <qt><p><b>Page Selection</b></p> "
						" <p>Here you can control if you print a certain selection only out of all"
						" the pages from the complete document."
						" </p>"
						" </qt>" );
	TQString whatsThisAllPagesLabel = i18n(  " <qt><b>All Pages:</b> Select \"All\" to print the complete document."
						" Since this is the default, it is pre-selected."
						" </p>"
						" </qt>" );
	TQString whatsThisCurrentPagesLabel = i18n(  " <qt><b>Current Page:</b> Select <em>\"Current\"</em> if you want "
						" to print the page currently visible in your TDE application.</p>"
						" <p><b>Note:</b> this field is disabled if you print from"
						" non-TDE applications like Firefox, PaleMoon, SeaMonkey or OpenOffice and"
						" LibreOffice, since here TDEPrint has no means to determine which document page"
						" you are currently viewing.</p></qt>" );
	TQString whatsThisPageRangeLabel = i18n(  " <qt><b>Page Range:</b> Choose a \"Page Range\" to select a subset of the"
						" complete document pages"
						" to be printed. The format is <em>\"n,m,o-p,q,r,s-t, u\"</em>.</p>"
						" <p><b>Example:</b> <em>\"4,6,10-13,17,20,23-25\"</em> will print"
						" the pages 4, 6, 10, 11, 12, 13, 17, 20, 23, 24, 25 of your document.</p>"
						" <br> "
						" <hr> "
						" <p><em><b>Additional hint for power users:</b> This TDEPrint GUI element matches "
						" with the CUPS commandline job option parameter:</em> "
						" <pre>"
						"    -o page-ranges=...     # example: \"4,6,10-13,17,20,23-25\" "
						" </pre>"
						" </p> "
						"</qt>" );
	TQString whatsThisPageSetLabel = i18n(  " <qt><b>Page Set:</b>"
						" <p>Choose <em>\"All Pages\"</em>, <em>\"Even Pages\"</em> or"
						" <em>\"Odd Pages\"</em>"
						" if you want to print a page selection matching one of these terms. The default"
						" is <em>\"All Pages\"</em>.</p>"
						" <p><b>Note:</b> If you combine a selection of a <em>\"Page Range\"</em> with a"
						" <em>\"Page Set\"</em> of <em>\"Odd\"</em> or <em>\"Even\"</em>, you will only get the"
						" odd or even pages from the originally selected page range. This is useful if you"
						" odd or even pages from the originally selected page range. This is useful if you"
						" want to print a page range in duplex on a simplex-only printer. In this case you"
						" can feed the paper to the printer twice; in the first pass, select \"Odd\" or"
						" \"Even\" (depending on your printer model), in second pass select the other"
						" option. You may need to <em>\"Reverse\"</em> the output in one of the passes (depending"
						" on your printer model).</p> "
						" <br> "
						" <hr> "
						" <p><em><b>Additional hint for power users:</b> This TDEPrint GUI element matches "
						" with the CUPS commandline job option parameter:</em> "
						" <pre>"
						"    -o page-set=...        # example: \"odd\" or \"even\" "
						" </pre>"
						" </p> "
						" </qt>" );
	TQString whatsThisCopiesLabel = i18n(  " <qt><b>Output Settings:</b>"
						" Here you can determine the number of copies, the output order and the collate"
						" mode for the pages of your printjob. (Note, that the maximum number of copies "
						" allowed to print may be restricted by your print subsystem.)</p>"
						" <p>The 'Copies' setting defaults to 1. </p> "
						" <br> "
						" <hr> "
						" <p><em><b>Additional hint for power users:</b> This TDEPrint GUI element matches "
						" with the CUPS commandline job option parameter:</em> "
						" <pre>"
						"     -o copies=...            # examples: \"5\" or \"42\" "
						" <br> "
						"    -o outputorder=...       # example:  \"reverse\""
						" <br> "
						"    -o Collate=...           # example:  \"true\" or \"false\" "
						" </pre>"
						" </p> "
						".</qt>" );
	TQString whatsThisNumberOfCopiesLabel = i18n(  " <qt><b>Number of Copies:</b> Determine the number of requested copies here."
						" You can increase or decrease"
						" the number of printed copies by clicking on the up and down arrows. You can also type the figure"
						" directly into the box. </p>"
						" <br> "
						" <hr> "
						" <p><em><b>Additional hint for power users:</b> This TDEPrint GUI element matches "
						" with the CUPS commandline job option parameter:</em> "
						" <pre>"
						"     -o copies=...            # examples: \"5\" or \"42\" "
						" </pre>"
						" </p> "
						" </qt>" );
	TQString whatsThisCollateLabel = i18n(  " <qt><b>Collate Copies</b>"
						" <p>If the <em>\"Collate\"</em> checkbox is enabled (default), the output order for"
						" multiple copies of a multi-page document will be \"1-2-3-..., 1-2-3-..., 1-2-3-...\".</p>"
						" <p>If the <em>\"Collate\"</em> checkbox is disabled, the output order for"
						" multiple copies of a multi-page document will be \"1-1-1-..., 2-2-2-..., 3-3-3-...\".</p>"
						" <br> "
						" <hr> "
						" <p><em><b>Additional hint for power users:</b> This TDEPrint GUI element matches "
						" with the CUPS commandline job option parameter:</em> "
						" <pre>"
						"    -o Collate=...           # example:  \"true\" or \"false\" "
						" </pre>"
						" </p> "
						" </qt>" );
	TQString whatsThisReverseLabel = i18n(  " <qt><b>Reverse Order</b>"
						" <p> If the <em>\"Reverse\"</em> checkbox is enabled, the output order for"
						" multiple copies of a multi-page document will be \"...-3-2-1, ...-3-2-1, ...-3-2-1\", if you"
						" also have <em>en</em>abled the <em>\"Collate\"</em> checkbox at the same time"
						" (the usual usecase).</p>"
						" <p>If the <em>\"Reverse\"</em> checkbox is enabled, the output order for"
						" multiple copies of a multi-page document will be \"...-3-3-3, ...-2-2-2, ...-1-1-1\", if you"
						" have <em>dis</em>abled the <em>\"Collate\"</em> checkbox at the same time. </p>"
						" <br> "
						" <hr> "
						" <p><em><b>Additional hint for power users:</b> This TDEPrint GUI element matches "
						" with the CUPS commandline job option parameter:</em> "
						" <pre>"
						"    -o outputorder=...       # example:  \"reverse\""
						" "
						" </pre>"
						" </p> "
						" </qt>" );
	m_printer = prt;
	m_useplugin = true;

	setTitle(i18n("C&opies"));
	setId(KPrinter::CopiesPage);

	// widget creation
	TQButtonGroup	*m_pagebox = new TQButtonGroup(0, TQt::Vertical, i18n("Page Selection"), this);
	TQWhatsThis::add(m_pagebox, whatsThisPageSelectionLabel);
	m_all = new TQRadioButton(i18n("&All"), m_pagebox);
	TQWhatsThis::add(m_all, whatsThisAllPagesLabel);
	m_current = new TQRadioButton(i18n("Cu&rrent"), m_pagebox);
	TQWhatsThis::add(m_current, whatsThisCurrentPagesLabel);
	m_range = new TQRadioButton(i18n("Ran&ge"), m_pagebox);
	TQWhatsThis::add(m_range, whatsThisPageRangeLabel);
	m_rangeedit = new TQLineEdit(m_pagebox);
	TQWhatsThis::add(m_rangeedit, whatsThisPageRangeLabel);
	connect(m_range, TQ_SIGNAL(clicked()), m_rangeedit, TQ_SLOT(setFocus()));
	TQToolTip::add(m_rangeedit, i18n("<p>Enter pages or group of pages to print separated by commas (1,2-5,8).</p>"));
//	TQWhatsThis::add(m_rangeedit, i18n("<p>Enter pages or group of pages to print separated by commas (1,2-5,8).</p>"));
	//TQLabel	*m_rangeexpl = new TQLabel(m_pagebox);
	//m_rangeexpl->setText(i18n("<p>Enter pages or group of pages to print separated by commas (1,2-5,8).</p>"));
	TQGroupBox	*m_copybox = new TQGroupBox(0, TQt::Vertical, i18n("Output Settings"), this);
	TQWhatsThis::add(m_copybox, whatsThisCopiesLabel);
	m_collate = new TQCheckBox(i18n("Co&llate"), m_copybox);
	TQWhatsThis::add(m_collate, whatsThisCollateLabel);
	m_order = new TQCheckBox(i18n("Re&verse"), m_copybox);
	TQWhatsThis::add(m_order, whatsThisReverseLabel);
	m_collatepix = new TQLabel(m_copybox);
	m_collatepix->setAlignment(TQt::AlignCenter);
	m_collatepix->setMinimumHeight(70);
	TQLabel	*m_copieslabel = new TQLabel(i18n("Cop&ies:"), m_copybox);
	m_copies = new TQSpinBox(m_copybox);
	m_copies->setRange(1,999);
	TQWhatsThis::add(m_copies, whatsThisNumberOfCopiesLabel);
	m_copieslabel->setBuddy(m_copies);
	TQWhatsThis::add(m_copieslabel, whatsThisNumberOfCopiesLabel);
	m_pageset = new TQComboBox(m_pagebox);
	m_pageset->insertItem(i18n("All Pages"));
	m_pageset->insertItem(i18n("Odd Pages"));
	m_pageset->insertItem(i18n("Even Pages"));
	TQWhatsThis::add(m_pageset, whatsThisPageSetLabel);
	TQLabel	*m_pagesetlabel = new TQLabel(i18n("Page &set:"), m_pagebox);
	m_pagesetlabel->setBuddy(m_pageset);
	TQWhatsThis::add(m_pagesetlabel, whatsThisPageSetLabel);
	KSeparator	*sepline = new KSeparator(TQt::Horizontal, m_pagebox);
	sepline->setMinimumHeight(10);

	TQWidget::setTabOrder( m_all, m_current );
	TQWidget::setTabOrder( m_current, m_range );
	TQWidget::setTabOrder( m_range, m_rangeedit );
	TQWidget::setTabOrder( m_rangeedit, m_pageset );
	TQWidget::setTabOrder( m_pageset, m_copies );
	TQWidget::setTabOrder( m_copies, m_collate );
	TQWidget::setTabOrder( m_collate, m_order );

	// layout creation
	TQGridLayout	*l1 = new TQGridLayout(this, 2, 2, 0, 5);
	l1->setRowStretch(1,1);
	l1->setColStretch(0,1);
	l1->setColStretch(1,1);
	l1->addWidget(m_pagebox,0,0);
	l1->addWidget(m_copybox,0,1);
	TQVBoxLayout	*l3 = new TQVBoxLayout(m_pagebox->layout(), 5);
	l3->addWidget(m_all);
	l3->addWidget(m_current);
	TQHBoxLayout	*l4 = new TQHBoxLayout(0, 0, 5);
	l3->addLayout(l4);
	l4->addWidget(m_range,0);
	l4->addWidget(m_rangeedit,1);
	//l3->addWidget(m_rangeexpl);
	l3->addWidget(sepline);
	TQHBoxLayout	*l2 = new TQHBoxLayout(0, 0, 5);
	l3->addLayout(l2);
	l2->addWidget(m_pagesetlabel,0);
	l2->addWidget(m_pageset,1);
	TQGridLayout	*l5 = new TQGridLayout(m_copybox->layout(), 4, 2, 10);
	l5->setRowStretch(4,1);
	l5->addWidget(m_copieslabel,0,0);
	l5->addWidget(m_copies,0,1);
	l5->addMultiCellWidget(m_collatepix,1,2,0,0);
	l5->addWidget(m_collate,1,1);
	l5->addWidget(m_order,2,1);

	// some initialization
	m_all->setChecked(true);
	m_copies->setValue(1);
	initialize(m_useplugin);
	slotCollateClicked();

	// connections
	connect(m_rangeedit,TQ_SIGNAL(textChanged(const TQString&)),TQ_SLOT(slotRangeEntered()));
	connect(m_collate,TQ_SIGNAL(clicked()),TQ_SLOT(slotCollateClicked()));
	connect(m_order,TQ_SIGNAL(clicked()),TQ_SLOT(slotCollateClicked()));

	if (!kapp->authorize("print/copies"))
	{
		setTitle(i18n("Pages"));
		m_copybox->hide();
	}
}

KPCopiesPage::~KPCopiesPage()
{
}

void KPCopiesPage::slotRangeEntered()
{
	m_range->setChecked(true);
}

void KPCopiesPage::slotCollateClicked()
{
	TQString	s("tdeprint_");
	s.append((m_collate->isChecked() ? "collate" : "uncollate"));
	if (m_order->isChecked()) s.append("_reverse");
	m_collatepix->setPixmap(UserIcon(s));
}

void KPCopiesPage::initialize(bool usePlugin)
{
	m_useplugin = usePlugin;
	int	f = KMFactory::self()->uiManager()->copyFlags(m_printer, m_useplugin);

	m_current->setEnabled((f & KMUiManager::Current));
	m_range->setEnabled((f & KMUiManager::Range));
	m_rangeedit->setEnabled((f & KMUiManager::Range));
	m_collate->setEnabled((f & KMUiManager::Collate));
	m_order->setEnabled((f & KMUiManager::Order));
	m_pageset->setEnabled((f & KMUiManager::PageSet));

	// by default, if collate disabled, set it to true
	m_collate->setChecked(!(f & KMUiManager::NoAutoCollate));
	slotCollateClicked();
}

void KPCopiesPage::setOptions(const TQMap<TQString,TQString>& options)
{
	TQString	value;
	// copies
	value = options["kde-copies"];
	if (!value.isEmpty()) m_copies->setValue(value.toInt());
	// output order
	value = options["kde-pageorder"];
	if (m_order->isEnabled()) m_order->setChecked(value == "Reverse");
	// collate
	value = options["kde-collate"];
	if (m_collate->isEnabled()) m_collate->setChecked(!(value == "Uncollate"));
	// update pixmap
	slotCollateClicked();
	// page ranges
	value = options["kde-range"];
	if (!value.isEmpty() && m_range->isEnabled() && value != "1-")
	{
		m_rangeedit->setText(value);
		m_range->setChecked(true);
	}
	else if (options["kde-current"] == "1")
		m_current->setChecked(true);
	else
		m_all->setChecked(true);
	// page set
	value = options["kde-pageset"];
	if (!value.isEmpty() && m_pageset->isEnabled())
		m_pageset->setCurrentItem(value.toInt());
	else
		m_pageset->setCurrentItem(0);
}

void KPCopiesPage::getOptions(TQMap<TQString,TQString>& options, bool incldef)
{
	// copies
	options["kde-copies"] = m_copies->text();
	// output order
	options["kde-pageorder"] = (m_order->isChecked() ? "Reverse" : "Forward");
	// collate
	options["kde-collate"] = (m_collate->isChecked() ? "Collate" : "Uncollate");
	// ranges
	options["kde-current"] = (m_current->isChecked() ? "1" : "0");
	options["kde-range"] = (m_range->isChecked() ? m_rangeedit->text() : (incldef ? TQString::fromLatin1("1-") : TQString::fromLatin1("")));
	// page set
	options["kde-pageset"] = TQString::number(m_pageset->currentItem());
}

void KPCopiesPage::reload()
{
	initialize(m_useplugin);
}

#include "kpcopiespage.moc"
