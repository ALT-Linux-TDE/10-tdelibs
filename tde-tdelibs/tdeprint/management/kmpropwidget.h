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

#ifndef KMPROPWIDGET_H
#define KMPROPWIDGET_H

#include <tqwidget.h>

#include <tdelibs_export.h>

class KMPrinter;
class KMWizard;

class TDEPRINT_EXPORT KMPropWidget : public TQWidget
{
	TQ_OBJECT
public:
	KMPropWidget(TQWidget *parent = 0, const char *name = 0);
	virtual ~KMPropWidget();

	virtual void setPrinter(KMPrinter*);
	void setPrinterBase(KMPrinter*);
	TQString pixmap() const 	{ return m_pixmap; }
	TQString title() const 	{ return m_title; }
	TQString header() const 	{ return m_header; }
	bool canChange() const 	{ return m_canchange; }

signals:
	void enable(bool);
	void enableChange(bool);

public slots:
	void slotChange();

protected:
	virtual int requestChange();
	virtual void configureWizard(KMWizard*);

protected:
	TQString		m_pixmap;
	TQString		m_title;
	TQString		m_header;
	KMPrinter	*m_printer;
	bool 		m_canchange;
};

#endif
