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

#ifndef LOCATIONDIALOG_H
#define LOCATIONDIALOG_H

#include <kdialogbase.h>

class TQComboBox;
class TQLineEdit;
class EditList;
struct CupsdConf;
struct CupsLocation;

class LocationDialog : public KDialogBase
{
	TQ_OBJECT
public:
	LocationDialog(TQWidget *parent = 0, const char *name = 0);

	void setInfos(CupsdConf*);
	void fillLocation(CupsLocation*);
	void setLocation(CupsLocation*);

	static bool newLocation(CupsLocation*, TQWidget *parent = 0, CupsdConf *conf = 0);
	static bool editLocation(CupsLocation*, TQWidget *parent = 0, CupsdConf *conf = 0);

protected slots:
	void slotTypeChanged(int);
	void slotClassChanged(int);
	void slotAdd();
	void slotEdit(int);
	void slotDefaultList();

private:
	TQComboBox	*resource_, *authtype_, *authclass_, *encryption_, *satisfy_, *order_;
	TQLineEdit	*authname_;
	EditList	*addresses_;
	CupsdConf	*conf_;
};

#endif
