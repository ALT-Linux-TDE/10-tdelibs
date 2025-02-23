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

#ifndef LPRHANDLER_H
#define LPRHANDLER_H

#if !defined( _TDEPRINT_COMPILE ) && defined( __GNUC__ )
#warning internal header, do not use except if you are a TDEPrint developer
#endif

#include <tqstring.h>

class PrintcapEntry;
class KMPrinter;
class DrMain;
class KMManager;
class KPrinter;

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class LprHandler
{
public:
	LprHandler(const TQString& name, KMManager *mgr = 0);
	virtual ~LprHandler();

	virtual bool validate(PrintcapEntry*);
	virtual KMPrinter* createPrinter(PrintcapEntry*);
	virtual bool completePrinter(KMPrinter*, PrintcapEntry*, bool shortmode = true);
	virtual DrMain* loadDriver(KMPrinter*, PrintcapEntry*, bool = false);
	virtual DrMain* loadDbDriver(const TQString&);
	virtual bool savePrinterDriver(KMPrinter*, PrintcapEntry*, DrMain*, bool* = 0);
	virtual PrintcapEntry* createEntry(KMPrinter*);
	virtual bool removePrinter(KMPrinter*, PrintcapEntry*);
	virtual TQString printOptions(KPrinter*);
	virtual void reset();

	TQString name() const;
	KMManager* manager() const;
	TQString driverDirectory();

protected:
	DrMain* loadToolDriver(const TQString&);
	TQString locateDir(const TQString& dirname, const TQString& paths);
	TQString cachedDriverDir() const;
	void setCachedDriverDir(const TQString&);
	virtual TQString driverDirInternal();

protected:
	TQString	m_name;
	KMManager	*m_manager;
	TQString	m_cacheddriverdir;
};

inline TQString LprHandler::name() const
{ return m_name; }

inline KMManager* LprHandler::manager() const
{ return m_manager; }

inline TQString LprHandler::cachedDriverDir() const
{ return m_cacheddriverdir; }

inline void LprHandler::setCachedDriverDir(const TQString& s)
{ m_cacheddriverdir = s; }

#endif
