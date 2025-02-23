//
/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef __open_with_p_h__
#define __open_with_p_h__

#include <kurl.h>
#include <tdelistview.h>

class KURLRequester;

class TQWidget;
class TQCheckBox;
class TQPushButton;
class TQLabel;
class TQStringList;


/* ------------------------------------------------------------------------- */

/**
 * @internal
 */
class KAppTreeListItem : public TQListViewItem
{
    bool parsed;
    bool directory;
    TQString path;
    TQString exec;
    TQString desktopPath;

protected:
	int compare(TQListViewItem *i, int col, bool ascending ) const;
    TQString key(int column, bool ascending) const;

    void init(const TQPixmap& pixmap, bool parse, bool dir, 
    					const TQString &_path, const TQString &exec, const TQString &_desktopPath);

public:
    KAppTreeListItem( TDEListView* parent, const TQString & name, const TQPixmap& pixmap,
                      bool parse, bool dir, const TQString &p, const TQString &c, const TQString &dp );
    KAppTreeListItem( TQListViewItem* parent, const TQString & name, const TQPixmap& pixmap,
                      bool parse, bool dir, const TQString &p, const TQString &c, const TQString &dp );
    bool isDirectory();

protected:
    virtual void activate();
    virtual void setOpen( bool o );

    friend class TDEApplicationTree;
};

/* ------------------------------------------------------------------------- */

/**
 * @internal
 */
class TDEApplicationTree : public TDEListView
{
    TQ_OBJECT
public:
    TDEApplicationTree( TQWidget *parent );

    /**
     * Add a group of .desktop/.kdelnk entries
     */
    void addDesktopGroup( const TQString &relPath, KAppTreeListItem *item = 0 );

    bool isDirSel();

protected:
    void resizeEvent( TQResizeEvent *_ev );
    KAppTreeListItem* currentitem;
	void cleanupTree();

public slots:
    void slotItemHighlighted(TQListViewItem* i);
    void slotSelectionChanged(TQListViewItem* i);

signals:
    void selected( const TQString& _name, const TQString& _exec, const  TQString& _desktopPath );
    void highlighted( const TQString& _name, const  TQString& _exec, const  TQString& _desktopPath );
};

/* ------------------------------------------------------------------------- */

#endif
