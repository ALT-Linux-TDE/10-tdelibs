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

#ifndef KMLRMANAGER_H
#define KMLRMANAGER_H

#include "kmmanager.h"

#include <tqdict.h>
#include <tqptrlist.h>
#include <tqdatetime.h>
#include <kurl.h>

class LprHandler;
class PrintcapEntry;
class LpcHelper;
class KPrinter;

class KMLprManager : public KMManager
{
	TQ_OBJECT
public:
	KMLprManager(TQObject *parent, const char *name, const TQStringList & /*args*/);

	bool completePrinter(KMPrinter*);
	bool completePrinterShort(KMPrinter*);
	bool enablePrinter(KMPrinter*, bool);
	bool startPrinter(KMPrinter*, bool);
	bool savePrinterDriver(KMPrinter*, DrMain*);
	DrMain* loadPrinterDriver(KMPrinter*, bool = false);
	DrMain* loadFileDriver(const TQString&);
	bool createPrinter(KMPrinter*);
	bool removePrinter(KMPrinter*);

	TQString driverDbCreationProgram();
	TQString driverDirectory();

	LpcHelper* lpcHelper()	{ return m_lpchelper; }
	TQString printOptions(KPrinter*);

	void createPluginActions(TDEActionCollection*);
	void validatePluginActions(TDEActionCollection*, KMPrinter*);
	TQString stateInformation();

protected slots:
	void slotEditPrintcap();

protected:
	void listPrinters();
	void initHandlers();
	void insertHandler(LprHandler*);
	PrintcapEntry* findEntry(KMPrinter*);
	LprHandler* findHandler(KMPrinter*);
	void checkPrinterState(KMPrinter*);
	bool savePrintcapFile();

private:
	TQDict<LprHandler>	m_handlers;
	TQPtrList<LprHandler>    m_handlerlist;
	TQDict<PrintcapEntry>	m_entries;
	TQDateTime		m_updtime;
	LpcHelper		*m_lpchelper;
	KMPrinter		*m_currentprinter;
};

#endif
