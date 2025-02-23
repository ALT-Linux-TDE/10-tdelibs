/* This file is part of the KDE libraries
   Copyright (C) 2002, 2003, 2004 Anders Lund <anders.lund@lund.tdcadsl.dk>
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

#include "katebookmarks.h"
#include "katebookmarks.moc"

#include "katedocument.h"
#include "kateview.h"

#include <tdelocale.h>
#include <tdeaction.h>
#include <tdepopupmenu.h>
#include <kstringhandler.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>

#include <tqregexp.h>
#include <tqmemarray.h>
#include <tqevent.h>

/**
   Utility: selection sort
   sort a TQMemArray<uint> in ascending order.
   max it the largest (zerobased) index to sort.
   To sort the entire array: ssort( *array, array.size() -1 );
   This is only efficient if ran only once.
*/
static void ssort( TQMemArray<uint> &a, int max )
{
  uint tmp, j, maxpos;
  for ( uint h = max; h >= 1; h-- )
  {
    maxpos = 0;
    for ( j = 0; j <= h; j++ )
      maxpos = a[j] > a[maxpos] ? j : maxpos;
    tmp = a[maxpos];
    a[maxpos] = a[h];
    a[h] = tmp;
  }
}

// TODO add a insort() or bubble_sort - more efficient for aboutToShow() ?

KateBookmarks::KateBookmarks( KateView* view, Sorting sort )
  : TQObject( view, "kate bookmarks" )
  , m_view( view )
  , m_sorting( sort )
{
  connect (view->getDoc(), TQ_SIGNAL(marksChanged()), this, TQ_SLOT(marksChanged()));
  _tries=0;
  m_bookmarksMenu = 0L;
}

KateBookmarks::~KateBookmarks()
{
}

void KateBookmarks::createActions( TDEActionCollection* ac )
{
  m_bookmarkToggle = new TDEToggleAction(
    i18n("Set &Bookmark"), "bookmark", CTRL+Key_B,
    this, TQ_SLOT(toggleBookmark()),
    ac, "bookmarks_toggle" );
  m_bookmarkToggle->setWhatsThis(i18n("If a line has no bookmark then add one, otherwise remove it."));
  m_bookmarkToggle->setCheckedState( i18n("Clear &Bookmark") );

  m_bookmarkClear = new TDEAction(
    i18n("Clear &All Bookmarks"), 0,
    this, TQ_SLOT(clearBookmarks()),
    ac, "bookmarks_clear");
  m_bookmarkClear->setWhatsThis(i18n("Remove all bookmarks of the current document."));

  m_goNext = new TDEAction(
    i18n("Next Bookmark"), "go-next", ALT + Key_PageDown,
    this, TQ_SLOT(goNext()),
    ac, "bookmarks_next");
  m_goNext->setWhatsThis(i18n("Go to the next bookmark."));

  m_goPrevious = new TDEAction(
    i18n("Previous Bookmark"), "go-previous", ALT + Key_PageUp,
    this, TQ_SLOT(goPrevious()),
    ac, "bookmarks_previous");
  m_goPrevious->setWhatsThis(i18n("Go to the previous bookmark."));

  m_bookmarksMenu = (new TDEActionMenu(i18n("&Bookmarks"), ac, "bookmarks"))->popupMenu();

  //connect the aboutToShow() and aboutToHide() signals with
  //the bookmarkMenuAboutToShow() and bookmarkMenuAboutToHide() slots
  connect( m_bookmarksMenu, TQ_SIGNAL(aboutToShow()), this, TQ_SLOT(bookmarkMenuAboutToShow()));
  connect( m_bookmarksMenu, TQ_SIGNAL(aboutToHide()), this, TQ_SLOT(bookmarkMenuAboutToHide()) );

  marksChanged ();
  bookmarkMenuAboutToHide();

  connect( m_view, TQ_SIGNAL( gotFocus( Kate::View * ) ), this, TQ_SLOT( slotViewGotFocus( Kate::View * ) ) );
  connect( m_view, TQ_SIGNAL( lostFocus( Kate::View * ) ), this, TQ_SLOT( slotViewLostFocus( Kate::View * ) ) );
}

void KateBookmarks::toggleBookmark ()
{
  uint mark = m_view->getDoc()->mark( m_view->cursorLine() );
  if( mark & KTextEditor::MarkInterface::markType01 )
    m_view->getDoc()->removeMark( m_view->cursorLine(),
        KTextEditor::MarkInterface::markType01 );
  else
    m_view->getDoc()->addMark( m_view->cursorLine(),
        KTextEditor::MarkInterface::markType01 );
}

void KateBookmarks::clearBookmarks ()
{

  TQPtrList<KTextEditor::Mark> m = m_view->getDoc()->marks();
  for (uint i=0; i < m.count(); i++)
    m_view->getDoc()->removeMark( m.at(i)->line, KTextEditor::MarkInterface::markType01 );

  // just to be sure ;)
  marksChanged ();
}

void KateBookmarks::slotViewGotFocus( Kate::View *v )
{
  if ( v == (Kate::View*)m_view )
    bookmarkMenuAboutToHide();
}

void KateBookmarks::slotViewLostFocus( Kate::View *v )
{
  if ( v == (Kate::View*)m_view )
    m_bookmarksMenu->clear();
}

void KateBookmarks::insertBookmarks( TQPopupMenu& menu )
{
  uint line = m_view->cursorLine();
  const TQRegExp re("&(?!&)");
  int idx( -1 );
  int old_menu_count = menu.count();
  KTextEditor::Mark *next = 0;
  KTextEditor::Mark *prev = 0;

  TQPtrList<KTextEditor::Mark> m = m_view->getDoc()->marks();
  TQMemArray<uint> sortArray( m.count() );
  TQPtrListIterator<KTextEditor::Mark> it( m );

  if ( it.count() > 0 )
    menu.insertSeparator();

  for( int i = 0; *it; ++it, ++i )
  {
    if( (*it)->type & KTextEditor::MarkInterface::markType01 )
    {
      TQString bText = KStringHandler::rEmSqueeze
                      ( m_view->getDoc()->textLine( (*it)->line ),
                        menu.fontMetrics(), 32 );
      bText.replace(re, "&&"); // kill undesired accellerators!
      bText.replace('\t', ' '); // kill tabs, as they are interpreted as shortcuts

      if ( m_sorting == Position )
      {
        sortArray[i] = (*it)->line;
        ssort( sortArray, i );
        idx = sortArray.find( (*it)->line ) + 3;
      }

      menu.insertItem(
          TQString("%1 - \"%2\"").arg( (*it)->line+1 ).arg( bText ),
          m_view, TQ_SLOT(gotoLineNumber(int)), 0, (*it)->line, idx );

      if ( (*it)->line < line )
      {
        if ( ! prev || prev->line < (*it)->line )
          prev = (*it);
      }

      else if ( (*it)->line > line )
      {
        if ( ! next || next->line > (*it)->line )
          next = (*it);
      }
    }
  }

  idx = ++old_menu_count;
  if ( next )
  {
    m_goNext->setText( i18n("&Next: %1 - \"%2\"").arg( next->line + 1 )
        .arg( KStringHandler::rsqueeze( m_view->getDoc()->textLine( next->line ), 24 ) ) );
    m_goNext->plug( &menu, idx );
    idx++;
  }
  if ( prev )
  {
    m_goPrevious->setText( i18n("&Previous: %1 - \"%2\"").arg(prev->line + 1 )
        .arg( KStringHandler::rsqueeze( m_view->getDoc()->textLine( prev->line ), 24 ) ) );
    m_goPrevious->plug( &menu, idx );
    idx++;
  }
  if ( next || prev )
    menu.insertSeparator( idx );

}

void KateBookmarks::bookmarkMenuAboutToShow()
{

  TQPtrList<KTextEditor::Mark> m = m_view->getDoc()->marks();

  m_bookmarksMenu->clear();
  m_bookmarkToggle->setChecked( m_view->getDoc()->mark( m_view->cursorLine() )
                                & KTextEditor::MarkInterface::markType01 );
  m_bookmarkToggle->plug( m_bookmarksMenu );
  m_bookmarkClear->plug( m_bookmarksMenu );


  insertBookmarks(*m_bookmarksMenu);
}

/*
   Make sure next/prev actions are plugged, and have a clean text
*/
void KateBookmarks::bookmarkMenuAboutToHide()
{
  m_bookmarkToggle->plug( m_bookmarksMenu );
  m_bookmarkClear->plug( m_bookmarksMenu );
  m_goNext->setText( i18n("Next Bookmark") );
  m_goNext->plug( m_bookmarksMenu );
  m_goPrevious->setText( i18n("Previous Bookmark") );
  m_goPrevious->plug( m_bookmarksMenu );
}

void KateBookmarks::goNext()
{
  TQPtrList<KTextEditor::Mark> m = m_view->getDoc()->marks();
  if (m.isEmpty())
    return;

  uint line = m_view->cursorLine();
  int found = -1;

  for (uint z=0; z < m.count(); z++)
    if ( (m.at(z)->line > line) && ((found == -1) || (uint(found) > m.at(z)->line)) )
      found = m.at(z)->line;

  if (found != -1)
    m_view->gotoLineNumber ( found );
}

void KateBookmarks::goPrevious()
{
  TQPtrList<KTextEditor::Mark> m = m_view->getDoc()->marks();
  if (m.isEmpty())
    return;

  uint line = m_view->cursorLine();
  int found = -1;

  for (uint z=0; z < m.count(); z++)
    if ((m.at(z)->line < line) && ((found == -1) || (uint(found) < m.at(z)->line)))
      found = m.at(z)->line;

  if (found != -1)
    m_view->gotoLineNumber ( found );
}

void KateBookmarks::marksChanged ()
{
  m_bookmarkClear->setEnabled( !m_view->getDoc()->marks().isEmpty() );
}
