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

#include <config.h>

#include "kpmarginpage.h"
#include "kprinter.h"
#include "driver.h"
#include "marginwidget.h"

#include <tqgroupbox.h>
#include <tqlayout.h>
#include <tqprinter.h>
#include <tqpaintdevicemetrics.h>

#include <kiconloader.h>
#include <tdelocale.h>
#include <kdebug.h>
#include <tdeglobal.h>

KPMarginPage::KPMarginPage(KPrinter *prt, DrMain *driver, TQWidget *parent, const char *name)
: KPrintDialogPage(0, driver, parent, name)
{
	m_printer = prt;
	setTitle(i18n("Margins"));
	m_usedriver = true;

	TQGroupBox	*box = new TQGroupBox(1, TQt::Vertical, i18n("Margins"), this);
	m_margin = new MarginWidget(box, "MarginWidget", (m_printer != 0));
	//m_margin->setSymetricMargins(true);
	//if (m_printer)
	//	m_margin->setResolution(m_printer->resolution());

	TQVBoxLayout	*l0 = new TQVBoxLayout(this, 0, 10);
	l0->addWidget(box);
	l0->addStretch(1);
}

KPMarginPage::~KPMarginPage()
{
}

void KPMarginPage::initPageSize(const TQString& ps, bool landscape)
{
	// first retrieve the Qt values for page size and margins
	TQPrinter	prt(TQPrinter::PrinterResolution);
	prt.setFullPage(true);
	prt.setPageSize((TQPrinter::PageSize)(ps.isEmpty() ? TDEGlobal::locale()->pageSize() : ps.toInt()));
	TQPaintDeviceMetrics	metrics(&prt);
	float	w = metrics.width();
	float	h = metrics.height();
	unsigned int	it, il, ib, ir;
	prt.margins( &it, &il, &ib, &ir );
	float	mt = it;
	float	ml = il;
	float	mb = ib;
	float	mr = ir;

	if (driver() && m_usedriver )
	{
		TQString	pageSize(ps);

		if (pageSize.isEmpty())
		{
			DrListOption	*o = (DrListOption*)driver()->findOption("PageSize");
			if (o)
				pageSize = o->get("default");
		}
		if (!pageSize.isEmpty())
		{
			DrPageSize	*dps = driver()->findPageSize(pageSize);
			if (dps)
			{
				w = dps->pageWidth();
				h = dps->pageHeight();
				mt = TQMAX( mt, dps->topMargin() );
				ml = TQMAX( ml, dps->leftMargin() );
				mb = TQMAX( mb, dps->bottomMargin() );
				mr = TQMAX( mr, dps->rightMargin() );
			}
		}
	}
	m_margin->setPageSize(w, h);
	m_margin->setOrientation(landscape ? KPrinter::Landscape : KPrinter::Portrait);
	m_margin->setDefaultMargins( mt, mb, ml, mr );
	m_margin->setCustomEnabled(false);
}

void KPMarginPage::setOptions(const TQMap<TQString,TQString>& opts)
{
	TQString	orient = opts["orientation-requested"];
	bool 	land = (orient.isEmpty()? opts["kde-orientation"] == "Landscape" : orient == "4" || orient == "5");
	TQString ps = opts[ "kde-printsize" ];
	if ( ps.isEmpty() )
	{
		m_usedriver = true;
		ps = opts[ "PageSize" ];
		if (ps.isEmpty())
			ps = opts["kde-pagesize"];
	}
	else
		m_usedriver = false;
	initPageSize(ps, land);

	bool	marginset(false);
	TQString	value;
	if (!(value=opts["kde-margin-top"]).isEmpty() && value.toFloat() != m_margin->top())
	{
		marginset = true;
		m_margin->setTop(value.toFloat());
	}
	if (!(value=opts["kde-margin-left"]).isEmpty() && value.toFloat() != m_margin->left())
	{
		marginset = true;
		m_margin->setLeft(value.toFloat());
	}
	if (!(value=opts["kde-margin-bottom"]).isEmpty() && value.toFloat() != m_margin->bottom())
	{
		marginset = true;
		m_margin->setBottom(value.toFloat());
	}
	if (!(value=opts["kde-margin-right"]).isEmpty() && value.toFloat() != m_margin->right())
	{
		marginset = true;
		m_margin->setRight(value.toFloat());
	}
	m_margin->setCustomEnabled(marginset);
}

void KPMarginPage::getOptions(TQMap<TQString,TQString>& opts, bool /* incldef */)
{
	if (m_margin->isCustomEnabled() /*|| incldef*/)
	{
		opts["kde-margin-top"] = TQString::number(m_margin->top());
		opts["kde-margin-left"] = TQString::number(m_margin->left());
		opts["kde-margin-bottom"] = TQString::number(m_margin->bottom());
		opts["kde-margin-right"] = TQString::number(m_margin->right());
	}
	else
	{
		opts.remove("kde-margin-top");
		opts.remove("kde-margin-left");
		opts.remove("kde-margin-bottom");
		opts.remove("kde-margin-right");
	}
}
