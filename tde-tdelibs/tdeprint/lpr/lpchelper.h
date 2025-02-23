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

#ifndef LPCHELPER_H
#define LPCHELPER_H

#include <tqobject.h>
#include <tqmap.h>
#include <tqtextstream.h>
#include "kmprinter.h"

class KMJob;

class LpcHelper : public TQObject
{
public:
	LpcHelper(TQObject *parent = 0, const char *name = 0);
	~LpcHelper();

	KMPrinter::PrinterState state(const TQString&) const;
	KMPrinter::PrinterState state(KMPrinter*) const;
	void updateStates();

	bool enable(KMPrinter*, bool, TQString&);
	bool start(KMPrinter*, bool, TQString&);
	bool removeJob(KMJob*, TQString&);
	bool changeJobState(KMJob*, int, TQString&);

	bool restart(TQString&);

protected:
	bool changeState(const TQString&, const TQString&, TQString&);
	void parsetStatusLPR(TQTextStream&);
	void parsetStatusLPRng(TQTextStream&);
	int parseStateChangeLPR(const TQString&, const TQString&);
	int parseStateChangeLPRng(const TQString&, const TQString&);

private:
	TQMap<TQString, KMPrinter::PrinterState>	m_state;
	TQString	m_exepath, m_lprmpath, m_checkpcpath;
};

#endif
