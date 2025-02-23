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

#ifndef KMCONFIGPAGE_H
#define KMCONFIGPAGE_H

#include <tqwidget.h>

#include <tdelibs_export.h>

class TDEConfig;

class TDEPRINT_EXPORT KMConfigPage : public TQWidget
{
	TQ_OBJECT
public:
	KMConfigPage(TQWidget *parent = 0, const char *name = 0);

	virtual void loadConfig(TDEConfig*);
	virtual void saveConfig(TDEConfig*);

	TQString pageName() const 	{ return m_name; }
	TQString pageHeader() const 	{ return m_header; }
	TQString pagePixmap() const 	{ return m_pixmap; }

protected:
	void setPageName(const TQString& s)	{ m_name = s; }
	void setPageHeader(const TQString& s)	{ m_header = s; }
	void setPagePixmap(const TQString& s)	{ m_pixmap = s; }

protected:
	TQString	m_name;
	TQString	m_header;
	TQString	m_pixmap;
};

#endif
