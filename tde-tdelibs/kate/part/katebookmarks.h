/* This file is part of the KDE libraries
   Copyright (C) 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>

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

#ifndef __KATE_BOOKMARKS_H__
#define __KATE_BOOKMARKS_H__

#include <tqobject.h>
#include <tqptrlist.h>

class KateView;

namespace KTextEditor { class Mark; }

namespace Kate { class View; }

class TDEAction;
class TDEToggleAction;
class TDEActionCollection;
class TQPopupMenu;
class TQMenuData;

class KateBookmarks : public TQObject
{
  TQ_OBJECT

  public:
    enum Sorting { Position, Creation };
    KateBookmarks( KateView* parent, Sorting sort=Position );
    virtual ~KateBookmarks();

    void createActions( TDEActionCollection* );

    KateBookmarks::Sorting sorting() { return m_sorting; };
    void setSorting( Sorting s ) { m_sorting = s; };

  protected:
    void insertBookmarks( TQPopupMenu& menu);

  private slots:
    void toggleBookmark();
    void clearBookmarks();

    void slotViewGotFocus( Kate::View * );
    void slotViewLostFocus( Kate::View * );

    void bookmarkMenuAboutToShow();
    void bookmarkMenuAboutToHide();

    void goNext();
    void goPrevious();

    void marksChanged ();

  private:
    KateView*                    m_view;
    TDEToggleAction*               m_bookmarkToggle;
    TDEAction*                     m_bookmarkClear;
    TDEAction*                     m_goNext;
    TDEAction*                     m_goPrevious;

    Sorting                      m_sorting;
    TQPopupMenu*          m_bookmarksMenu;

    uint _tries;
};

#endif
