/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <tdeprint@swing.be>
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

#ifndef QDIRMULTILINEEDIT_H
#define QDIRMULTILINEEDIT_H

#include <tqwidget.h>

class TDEListView;
class TQListViewItem;
class TQPushButton;

class QDirMultiLineEdit : public TQWidget
{
	TQ_OBJECT

public:
	QDirMultiLineEdit(TQWidget *parent = 0, const char *name = 0);
	~QDirMultiLineEdit();

	void setURLs(const TQStringList&);
	TQStringList urls();

protected:
	void addURL(const TQString&);

private slots:
	void slotAddClicked();
	void slotRemoveClicked();
	void slotSelected(TQListViewItem*);

private:
	TDEListView	*m_view;
	TQPushButton	*m_add, *m_remove;
};

#endif
