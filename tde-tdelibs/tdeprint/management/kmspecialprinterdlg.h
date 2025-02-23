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

#ifndef KMSPECIALPRINTERDLG_H
#define KMSPECIALPRINTERDLG_H

#include <kdialogbase.h>

class KMPrinter;
class TQLineEdit;
class TQCheckBox;
class TQComboBox;
class TDEIconButton;
class KXmlCommandSelector;

class KMSpecialPrinterDlg : public KDialogBase
{
	TQ_OBJECT
public:
	KMSpecialPrinterDlg(TQWidget *parent = 0, const char *name = 0);

	void setPrinter(KMPrinter*);
	KMPrinter* printer();

protected:
	bool checkSettings();

protected slots:
	void slotOk();
	void slotTextChanged(const TQString &);

private:
	TQLineEdit	*m_name, *m_description, *m_location, *m_extension;
	TQComboBox	*m_mimetype;
	TQCheckBox	*m_usefile;
	TQStringList	m_mimelist;
	TDEIconButton	*m_icon;
	KXmlCommandSelector	*m_command;
};

#endif
