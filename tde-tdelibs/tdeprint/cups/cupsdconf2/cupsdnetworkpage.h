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

#ifndef CUPSDNETWORKPAGE_H
#define CUPSDNETWORKPAGE_H

#include "cupsdpage.h"

class KIntNumInput;
class TQCheckBox;
class TQComboBox;
class EditList;
class SizeWidget;

class CupsdNetworkPage : public CupsdPage
{
	TQ_OBJECT

public:
	CupsdNetworkPage(TQWidget *parent = 0, const char *name = 0);

	bool loadConfig(CupsdConf*, TQString&);
	bool saveConfig(CupsdConf*, TQString&);
	void setInfos(CupsdConf*);

protected slots:
	void slotAdd();
	void slotEdit(int);
	void slotDefaultList();

private:
	KIntNumInput	*keepalivetimeout_, *maxclients_, *clienttimeout_;
	TQComboBox	*hostnamelookup_;
	TQCheckBox	*keepalive_;
	EditList	*listen_;
	SizeWidget *maxrequestsize_;
};

#endif
