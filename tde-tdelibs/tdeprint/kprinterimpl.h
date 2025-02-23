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

#ifndef KPRINTERIMPL_H
#define KPRINTERIMPL_H

#include <tqobject.h>
#include <tqstringlist.h>
#include <tqmap.h>
#include <tqptrlist.h>

#include <tdelibs_export.h>

class KPrinter;
class KMPrinter;

class TDEPRINT_EXPORT KPrinterImpl : public TQObject
{
	TQ_OBJECT
public:
	KPrinterImpl(TQObject *parent = 0, const char *name = 0);
	virtual ~KPrinterImpl();

	virtual bool setupCommand(TQString& cmd, KPrinter*);
	virtual void preparePrinting(KPrinter*);
	virtual void broadcastOption(const TQString& key, const TQString& value);

	bool printFiles(KPrinter*, const TQStringList&, bool removeflag = false);
	// result:
	//	-1	->	error
	//	0	->	nothing happened
	//	1	->	files filterd
	int filterFiles(KPrinter*, TQStringList&, bool removeflag = false);
	int autoConvertFiles(KPrinter*, TQStringList&, bool removeflag = false);
	void saveOptions(const TQMap<TQString,TQString>& opts);
	const TQMap<TQString,TQString>& loadOptions() const 	{ return m_options; }
	TQString tempFile();
	TQString quote(const TQString&);
	void statusMessage(const TQString&, KPrinter* = 0);

protected:
	bool startPrinting(const TQString& cmd, KPrinter *printer, const TQStringList& files, bool removeflag = false);
	int dcopPrint(const TQString& cmd, const TQStringList& files, bool removeflag = false);
	bool setupSpecialCommand(TQString&, KPrinter*, const TQStringList&);
	int doFilterFiles(KPrinter* pr, TQStringList& files, const TQStringList& flist, const TQMap<TQString,TQString>& opts, bool removeflag = false);
	void loadAppOptions();
	void saveAppOptions();

protected:
	TQMap<TQString,TQString>	m_options;	// use to save current options
};

#endif
