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

#ifndef PORTDIALOG_H
#define PORTDIALOG_H

#include <kdialogbase.h>

class TQLineEdit;
class TQSpinBox;
class TQCheckBox;
struct CupsdConf;

class PortDialog : public KDialogBase
{
public:
	PortDialog(TQWidget *parent = 0, const char *name = 0);

	TQString listenString();
	void setInfos(CupsdConf*);
	static TQString newListen(TQWidget *parent = 0, CupsdConf *conf = 0);
	static TQString editListen(const TQString& s, TQWidget *parent = 0, CupsdConf *conf = 0);

private:
	TQLineEdit	*address_;
	TQSpinBox	*port_;
	TQCheckBox	*usessl_;
};

#endif
