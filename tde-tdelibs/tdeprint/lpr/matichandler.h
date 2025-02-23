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

#ifndef MATICHANDLER_H
#define MATICHANDLER_H

#include "lprhandler.h"

#include <kurl.h>

class MaticBlock;

class MaticHandler : public LprHandler
{
public:
	MaticHandler(KMManager *mgr = 0);

	bool validate(PrintcapEntry*);
	KMPrinter* createPrinter(PrintcapEntry*);
	bool completePrinter(KMPrinter*, PrintcapEntry*, bool = true);
	DrMain* loadDriver(KMPrinter*, PrintcapEntry*, bool = false);
	DrMain* loadDbDriver(const TQString&);
	bool savePrinterDriver(KMPrinter*, PrintcapEntry*, DrMain*, bool* = 0);
	PrintcapEntry* createEntry(KMPrinter*);
	bool removePrinter(KMPrinter*, PrintcapEntry*);
	TQString printOptions(KPrinter*);

protected:
	TQString driverDirInternal();

private:
	TQString parsePostpipe(const TQString&);
	TQString createPostpipe(const TQString&);
	bool savePpdFile(DrMain*, const TQString&);

private:
	TQString	m_exematicpath;
	TQString	m_ncpath, m_smbpath, m_rlprpath;
};

#endif
