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

#ifndef ADDRESSDIALOG_H
#define ADDRESSDIALOG_H

#include <kdialogbase.h>

class TQComboBox;
class TQLineEdit;

class AddressDialog : public KDialogBase
{
public:
	AddressDialog(TQWidget *parent = 0, const char *name = 0);

	TQString addressString();
	static TQString newAddress(TQWidget *parent = 0);
	static TQString editAddress(const TQString& s, TQWidget *parent = 0);

private:
	TQComboBox	*type_;
	TQLineEdit	*address_;
};

#endif
