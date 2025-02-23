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

#ifndef KMWIZARDPAGE_H
#define KMWIZARDPAGE_H

#include <tqwidget.h>

#include <tdelibs_export.h>

class KMPrinter;

class TDEPRINT_EXPORT KMWizardPage : public TQWidget
{
	TQ_OBJECT
public:
	KMWizardPage(TQWidget *parent = 0, const char *name = 0);
	virtual ~KMWizardPage();

	int id() const 			{ return m_ID; }
	const TQString& title() const 	{ return m_title; }
	int nextPage() const 		{ return m_nextpage; }
	void setNextPage(int p)		{ m_nextpage = p; }
	virtual bool isValid(TQString&);
	virtual void initPrinter(KMPrinter*);
	virtual void updatePrinter(KMPrinter*);
        bool needsInitOnBack()          { return m_needsinitonback; }

protected:
	TQString	m_title;
	int	m_ID;
	int	m_nextpage;
        bool    m_needsinitonback;
};

#endif
