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

#ifndef KPRINTERPROPERTYDIALOG_H
#define KPRINTERPROPERTYDIALOG_H

#include <kdialogbase.h>
#include <tqptrlist.h>
#include <tqmap.h>

class KMPrinter;
class KPrintDialogPage;
class DrMain;
class TQTabWidget;

class TDEPRINT_EXPORT KPrinterPropertyDialog : public KDialogBase
{
	TQ_OBJECT
public:
	KPrinterPropertyDialog(KMPrinter *printer, TQWidget *parent = 0, const char *name = 0);
	~KPrinterPropertyDialog();

	KMPrinter* printer() 		{ return m_printer; }
	DrMain* driver()		{ return m_driver; }
	void setDriver(DrMain* d) 	{ if (!m_driver) m_driver = d; }
	void addPage(KPrintDialogPage*);
	void setOptions(const TQMap<TQString,TQString>&);
	void getOptions(TQMap<TQString,TQString>&, bool = false);
	void enableSaveButton(bool);

	static void setupPrinter(KMPrinter *printer, TQWidget *parent);

protected:
	bool synchronize();
	void collectOptions(TQMap<TQString,TQString>& opts, bool incldef = false);

protected slots:
	void slotCurrentChanged(TQWidget*);
	void slotUser1();
	void slotOk();

protected:
	KMPrinter		*m_printer;
	DrMain			*m_driver;
	TQPtrList<KPrintDialogPage>	m_pages;
	KPrintDialogPage	*m_current;
	TQMap<TQString,TQString>	m_options;
	TQTabWidget		*m_tw;
	TQPushButton		*m_save;
};

#endif
