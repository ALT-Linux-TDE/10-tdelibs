/* This file is part of the KDE libraries
   Copyright (C) 2000 Charles Samuels <charles@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef TDELISTVIEWLINEEDIT_H
#define TDELISTVIEWLINEEDIT_H

#include <klineedit.h>
#include <tdelistview.h>

/**
 * the editor for a TDEListView.  please don't use this.
 * @internal
 **/
class TDEUI_EXPORT TDEListViewLineEdit : public KLineEdit
{
TQ_OBJECT
public:
	TDEListViewLineEdit(TDEListView *parent);
	~TDEListViewLineEdit();

	TQListViewItem *currentItem() const;
  void setRenameSettings(const TDEListViewRenameSettings &renSett) { m_renSett = renSett; };

signals:
	void done(TQListViewItem*, int);
  
  /**
   * This signal is emitted when item renaming is completed by a TAB.
   * It signals the receiver that the sender would like to start renaming the next item.
   * This is not hardcoded in TDEListView because the next item is application depended
   * (for example it could be the next column or the next row or something completely different)
   *
   * @param item is the renamed item.
   * @param col is the renamed column.
   *
   * @since 14.0
   */
  void renameNext(TQListViewItem* item, int col);

  /**
   * This signal is emitted when item renaming is completed by a Shift+TAB.
   * It signals the receiver that the sender would like to start renaming the previous item.
   * This is not hardcoded in TDEListView because the next item is application depended
   * (for example it could be the next column or the next row or something completely different)
   *
   * @param item is the renamed item.
   * @param col is the renamed column.
   *
   * @since 14.0
   */
  void renamePrev(TQListViewItem* item, int col);

public slots:
	void terminate();
	void load(TQListViewItem *i, int c);

protected:
	virtual void focusOutEvent(TQFocusEvent *);
	virtual void keyPressEvent(TQKeyEvent *e);
	virtual void paintEvent(TQPaintEvent *e);
	virtual bool event (TQEvent *pe);

	/// @since 3.1
	void selectNextCell (TQListViewItem *pi, int column, bool forward);
	void terminate(bool commit);
	TQListViewItem *item;
	int col;
	TDEListView* const p;
  TDEListViewRenameSettings m_renSett;

protected slots:
	void slotSelectionChanged();

	/// @since 3.5.4
	void slotItemRemoved(TQListViewItem *i);

};

#endif
