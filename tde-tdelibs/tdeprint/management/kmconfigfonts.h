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

#ifndef KMCONFIGFONTS_H
#define KMCONFIGFONTS_H

#include "kmconfigpage.h"

class TDEListView;
class KURLRequester;
class TQPushButton;
class TQCheckBox;

class KMConfigFonts : public KMConfigPage
{
	TQ_OBJECT
public:
	KMConfigFonts(TQWidget *parent = 0, const char *name = 0);

	void loadConfig(TDEConfig*);
	void saveConfig(TDEConfig*);

protected slots:
	void slotUp();
	void slotDown();
	void slotRemove();
	void slotAdd();
	void slotSelected();
	void slotTextChanged(const TQString&);

private:
	TQCheckBox	*m_embedfonts;
	TDEListView	*m_fontpath;
	KURLRequester	*m_addpath;
	TQPushButton	*m_up, *m_down, *m_add, *m_remove;
};

#endif
