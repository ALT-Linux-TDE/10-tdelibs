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

#ifndef KMDRIVERDBWIDGET_H
#define KMDRIVERDBWIDGET_H

#include <tqwidget.h>
#include "kmdbentry.h"

class TQListBox;
class TQCheckBox;
class TQPushButton;

class KMDriverDbWidget : public TQWidget
{
	TQ_OBJECT
public:
	KMDriverDbWidget(TQWidget *parent = 0, const char *name = 0);
	~KMDriverDbWidget();

	void init();
	void setHaveRaw(bool on);
	void setHaveOther(bool on);
	void setDriver(const TQString& manu, const TQString& model);

	TQString manufacturer();
	TQString model();
	TQString description()		{ return m_desc; }
	KMDBEntryList* drivers();
	TQString driverFile();
	bool isRaw();
	bool isExternal();

protected slots:
	void slotDbLoaded(bool reloaded);
	void slotManufacturerSelected(const TQString& name);
	void slotPostscriptToggled(bool);
	void slotOtherClicked();
	void slotError(const TQString&);

private:
	TQListBox	*m_manu;
	TQListBox	*m_model;
	TQCheckBox	*m_postscript;
	TQCheckBox	*m_raw;
	TQPushButton	*m_other;
	TQString		m_external;
	TQString		m_desc;
	bool		m_valid;
};

inline TQString KMDriverDbWidget::driverFile()
{ return m_external; }

inline bool KMDriverDbWidget::isExternal()
{ return !(m_external.isEmpty()); }

#endif
