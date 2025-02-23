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

#ifndef KICONSELECTACTION_H
#define KICONSELECTACTION_H

#include <tdeaction.h>

class TDEIconSelectActionPrivate;

class TDEIconSelectAction : public TDESelectAction
{
	TQ_OBJECT
public:
	TDEIconSelectAction(const TQString& text, int accel = 0, TQObject* parent = 0, const char* name = 0);
	virtual ~TDEIconSelectAction();

	virtual int plug(TQWidget* widget, int index = -1);

public slots:
	virtual void setItems(const TQStringList& lst, const TQStringList& iconlst);
	virtual void setCurrentItem(int index);

protected:
	void createPopupMenu();
	void updateIcons();
	virtual void updateCurrentItem(int id);

private:
	TDEIconSelectActionPrivate*	d;
};

#endif
