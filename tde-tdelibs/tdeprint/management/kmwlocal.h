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

#ifndef KMWLOCAL_H
#define KMWLOCAL_H

#include "kmwizardpage.h"

#include <tqstringlist.h>

class TQLineEdit;
class TDEListView;
class TQListViewItem;

class KMWLocal : public KMWizardPage
{
	TQ_OBJECT
public:
	KMWLocal(TQWidget *parent = 0, const char *name = 0);

	bool isValid(TQString&);
	void updatePrinter(KMPrinter*);
	void initPrinter(KMPrinter*);

protected slots:
	void slotPortSelected(TQListViewItem*);
	void slotTextChanged( const TQString& );

protected:
	void initialize();
	TQListViewItem* lookForItem( const TQString& );

protected:
	TDEListView	*m_ports;
	TQLineEdit	*m_localuri;
	TQStringList	m_uris;
	TQListViewItem	*m_parents[4];
	bool		m_initialized;
	bool m_block;
};

#endif
