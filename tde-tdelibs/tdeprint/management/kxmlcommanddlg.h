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

#ifndef KXMLCOMMANDDLG_H
#define KXMLCOMMANDDLG_H

#include <tqwidget.h>
#include <tqmap.h>
#include <tqstringlist.h>
#include <kdialogbase.h>

class TDEListView;
class TQListViewItem;
class TQLineEdit;
class TQComboBox;
class TQWidgetStack;
class TQToolButton;
class TDEListBox;
class TQListBoxItem;
class TQTextEdit;
class TQCheckBox;

class DrGroup;
class DrBase;
class KXmlCommand;

class KXmlCommandAdvancedDlg : public TQWidget
{
	TQ_OBJECT
public:
	KXmlCommandAdvancedDlg(TQWidget *parent = 0, const char *name = 0);
	~KXmlCommandAdvancedDlg();

	void setCommand(KXmlCommand*);
	static bool editCommand(KXmlCommand *xmlcmd, TQWidget *parent = 0);

protected:
	void parseGroupItem(DrGroup*, TQListViewItem*);
	void parseXmlCommand(KXmlCommand*);
	void viewItem(TQListViewItem*);
	void removeItem(TQListViewItem*);
	void recreateGroup(TQListViewItem*, DrGroup*);

protected slots:
	void slotSelectionChanged(TQListViewItem*);
	void slotTypeChanged(int);
	void slotAddValue();
	void slotRemoveValue();
	void slotApplyChanges();
	void slotAddGroup();
	void slotAddOption();
	void slotRemoveItem();
	void slotMoveUp();
	void slotMoveDown();
	void slotCommandChanged(const TQString&);
	void slotValueSelected(TQListViewItem*);
	void slotOptionRenamed(TQListViewItem*, int);
	void slotChanged();

private:
	TDEListView	*m_view;
	TQLineEdit	*m_name, *m_desc, *m_format, *m_default, *m_command;
	TQComboBox	*m_type;
	TQWidget		*m_dummy;
	TDEListView	*m_values;
	TQLineEdit	*m_edit1, *m_edit2;
	TQWidgetStack	*m_stack;
	TQToolButton	*m_apply, *m_addgrp, *m_addopt, *m_delopt, *m_up, *m_down;
	TQLineEdit	*m_inputfile, *m_inputpipe, *m_outputfile, *m_outputpipe;
	TQToolButton	*m_addval, *m_delval;
	TQTextEdit *m_comment;
	TQCheckBox *m_persistent;

	KXmlCommand	*m_xmlcmd;
	TQMap<TQString, DrBase*>	m_opts;
};

class KXmlCommandDlg : public KDialogBase
{
	TQ_OBJECT
public:
	KXmlCommandDlg(TQWidget *parent = 0, const char *name = 0);

	void setCommand(KXmlCommand*);
	static bool editCommand(KXmlCommand*, TQWidget *parent = 0);

protected slots:
	void slotAddMime();
	void slotRemoveMime();
	void slotEditCommand();
	void slotAddReq();
	void slotRemoveReq();
	void slotReqSelected(TQListViewItem*);
	void slotAvailableSelected(TQListBoxItem*);
	void slotSelectedSelected(TQListBoxItem*);
	void slotOk();

private:
	TQLineEdit	*m_description;
	TQLabel		*m_idname;
	TQComboBox	*m_mimetype;
	TDEListBox	*m_availablemime, *m_selectedmime;
	TQToolButton	*m_addmime, *m_removemime;
	TDEListView	*m_requirements;
	TQToolButton	*m_removereq, *m_addreq;

	TQStringList	m_mimelist;
	KXmlCommand	*m_cmd;
};

#endif
