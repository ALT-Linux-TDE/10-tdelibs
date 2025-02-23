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

#ifndef TREECOMBOBOX_H
#define TREECOMBOBOX_H

#include <tqlistbox.h>
#include <tqcombobox.h>
#include <tqstringlist.h>

/**
 * Class that represents a single object in the tree
 */
class TreeListBoxItem : public TQListBoxPixmap
{
public:
	TreeListBoxItem(TQListBox *lb, const TQPixmap& pix, const TQString& txt, bool oneBlock = false);

	virtual int width(const TQListBox *lb) const;

protected:
	virtual void paint(TQPainter *p);
	int stepSize() const { return 16; }

private:
	TQStringList	m_path;
	int		m_depth;
	TreeListBoxItem	*m_child, *m_next, *m_parent;
};

/**
 * Class for the listbox shown on popup
 */
class TreeListBox : public TQListBox
{
	friend class TreeListBoxItem;
public:
	TreeListBox(TQWidget *parent = 0, const char *name = 0);

protected:
	virtual void paintCell(TQPainter *p, int row, int col);

private:
	bool	m_painting;
};

/**
 * Main class.
 */
class TreeComboBox : public TQComboBox
{
public:
	TreeComboBox(TQWidget *parent = 0, const char *name = 0);
	void insertItem(const TQPixmap& pix, const TQString& txt, bool oneBlock = false);

private:
	TQListBox	*m_listbox;
};

#endif
