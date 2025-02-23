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

#ifndef KMCONFIGFILTER_H
#define KMCONFIGFILTER_H

#include "kmconfigpage.h"

class TDEListBox;
class TQToolButton;
class TQLineEdit;

class KMConfigFilter : public KMConfigPage
{
	TQ_OBJECT
public:
	KMConfigFilter(TQWidget *parent = 0, const char *name = 0);

	void loadConfig(TDEConfig*);
	void saveConfig(TDEConfig*);

protected slots:
	void slotSelectionChanged();
	void slotAddClicked();
	void slotRemoveClicked();

protected:
	void transfer(TDEListBox *from, TDEListBox *to);

private:
	TDEListBox	*m_list1, *m_list2;
	TQToolButton	*m_add, *m_remove;
	TQLineEdit	*m_locationre;
};

#endif
