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

#ifndef KMCUPSCONFIGWIDGET_H
#define KMCUPSCONFIGWIDGET_H

#include <tqwidget.h>

class TQLineEdit;
class TQCheckBox;
class TDEConfig;

class KMCupsConfigWidget : public TQWidget
{
public:
	KMCupsConfigWidget(TQWidget *parent = 0, const char *name = 0);

	void load();
	void save(bool sync = true);
	void saveConfig(TDEConfig*);

protected:
	TQLineEdit	*m_host, *m_port, *m_login, *m_password;
	TQCheckBox	*m_anonymous, *m_savepwd;
};

#endif
