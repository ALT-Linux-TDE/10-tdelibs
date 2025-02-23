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

#ifndef	KMWIPPPRINTER_H
#define	KMWIPPPRINTER_H

#include "kmwizardpage.h"

class TDEListView;
class TQListViewItem;
class TQLineEdit;
class TQTextView;
class TQPushButton;
class NetworkScanner;

class KMWIppPrinter : public KMWizardPage
{
	TQ_OBJECT
public:
	KMWIppPrinter(TQWidget *parent = 0, const char *name = 0);
	~KMWIppPrinter();

	bool isValid(TQString&);
	void updatePrinter(KMPrinter*);

protected slots:
	void slotScanStarted();
	void slotScanFinished();
	void slotPrinterSelected(TQListViewItem*);
	void slotIppReport();

private:
	TDEListView	*m_list;
	NetworkScanner *m_scanner;
	TQLineEdit	*m_uri;
	TQTextView	*m_info;
	TQPushButton	*m_ippreport;
};

#endif
