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

#ifndef EDITENTRYDIALOG_H
#define EDITENTRYDIALOG_H

#include <kdialogbase.h>
#include "printcapentry.h"

class TQLineEdit;
class TQCheckBox;
class TQSpinBox;
class TQComboBox;
class TQListView;
class TQListviewItem;
class TQWidgetStack;

class EditEntryDialog : public KDialogBase
{
	TQ_OBJECT
public:
	EditEntryDialog(PrintcapEntry *entry, TQWidget *parent = 0, const char *name = 0);

	void fillEntry(PrintcapEntry *entry);

protected slots:
	void slotItemSelected(TQListViewItem*);
	void slotChanged();
	void slotTypeChanged(int);

protected:
	Field createField();

private:
	TQMap<TQString,Field>	m_fields;
	TQLineEdit	*m_name, *m_string, *m_aliases;
	TQCheckBox	*m_boolean;
	TQComboBox	*m_type;
	TQSpinBox	*m_number;
	TQListView	*m_view;
	TQWidgetStack	*m_stack;
	TQString		m_current;
	bool		m_block;
};

#endif
