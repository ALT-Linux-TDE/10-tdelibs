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

#ifndef KMSPECIALMANAGER_H
#define KMSPECIALMANAGER_H

#include <tqobject.h>
#include <tqmap.h>

class KMPrinter;
class KMManager;
class KXmlCommand;
class DrMain;

class KMSpecialManager : public TQObject
{
public:
	KMSpecialManager(KMManager *parent, const char *name = 0);

	bool loadPrinters();
	bool savePrinters();
	void refresh();
	KXmlCommand* loadCommand(KMPrinter*);
	KXmlCommand* loadCommand(const TQString& cmd);
	DrMain* loadDriver(KMPrinter*);
	TQString setupCommand(const TQString& cmd, const TQMap<TQString,TQString>& opts);

protected:
	bool loadDesktopFile(const TQString&);

private:
	KMManager	*m_mgr;
	bool		m_loaded;
};

#endif
