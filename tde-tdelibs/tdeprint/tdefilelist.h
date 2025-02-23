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

#ifndef TDEFILELIST_H
#define TDEFILELIST_H

#include <tqwidget.h>
#include <tqptrlist.h>
#include <kurl.h>

class TDEListView;
class TQToolButton;
class TQListViewItem;

class KFileList : public TQWidget
{
	TQ_OBJECT
public:
	KFileList(TQWidget *parent = 0, const char *name = 0);
	virtual ~KFileList();

	void setFileList(const TQStringList&);
	TQStringList fileList() const;
	TQSize sizeHint() const;

protected slots:
	void slotAddFile();
	void slotRemoveFile();
	void slotOpenFile();
	void slotSelectionChanged();
	void slotUp();
	void slotDown();

protected:
	void dragEnterEvent(TQDragEnterEvent*);
	void dropEvent(TQDropEvent*);
	void addFiles(const KURL::List&);
	void selection(TQPtrList<TQListViewItem>&);

private:
	TDEListView	*m_files;
	TQToolButton	*m_add, *m_remove, *m_open, *m_up, *m_down;
	bool		m_block;
};

#endif
