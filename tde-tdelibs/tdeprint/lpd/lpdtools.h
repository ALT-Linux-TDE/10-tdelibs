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
#ifndef LPDTOOLS_H
#define LPDTOOLS_H

#include <tqstring.h>
#include <tqmap.h>
#include <tqptrlist.h>
#include <tqtextstream.h>

class KMLpdManager;
class DrMain;
class KMPrinter;

class PrintcapEntry
{
friend class KMLpdManager;
public:
	bool readLine(const TQString& line);
	void writeEntry(TQTextStream&);
	KMPrinter* createPrinter();
	TQString arg(const TQString& key) const 	{ return m_args[key]; }
	TQString comment(int i);
private:
	TQString			m_name;
	TQString			m_comment;
	TQMap<TQString,TQString>	m_args;
};

//*****************************************************************************************************

struct Resolution
{
	int 	xdpi, ydpi;
	TQString	comment;
};

struct BitsPerPixel
{
	TQString	bpp;
	TQString	comment;
};

class PrinttoolEntry
{
friend class KMLpdManager;
public:
	bool readEntry(TQTextStream& t);
	DrMain* createDriver();
private:
	TQString			m_name, m_gsdriver, m_description, m_about;
	TQPtrList<Resolution>	m_resolutions;
	TQPtrList<BitsPerPixel>	m_depths;
};

//*****************************************************************************************************

TQString getPrintcapLine(TQTextStream& t, TQString *lastcomment = NULL);

#endif // LPDTOOLS_H
