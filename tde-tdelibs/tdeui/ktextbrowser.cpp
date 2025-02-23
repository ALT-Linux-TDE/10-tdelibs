/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2000 Espen Sand (espen@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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
 */


#include <tqpopupmenu.h>
#include <tdeapplication.h>
#include <tdeglobalsettings.h>
#include <ktextbrowser.h>
#include <kcursor.h>
#include <kurl.h>
#include <kiconloader.h>

KTextBrowser::KTextBrowser( TQWidget *parent, const char *name,
			    bool notifyClick )
  : TQTextBrowser( parent, name ), mNotifyClick(notifyClick)
{
  //
  //1999-10-04 Espen Sand: Not required anymore ?
  //connect( this, TQ_SIGNAL(highlighted(const TQString &)),
  //   this, TQ_SLOT(refChanged(const TQString &)));
}

KTextBrowser::~KTextBrowser( void )
{
}


void KTextBrowser::setNotifyClick( bool notifyClick )
{
  mNotifyClick = notifyClick;
}


bool KTextBrowser::isNotifyClick() const
{
  return mNotifyClick;
}


void KTextBrowser::setSource( const TQString& name )
{
  if( name.isNull() )
  {
    return;
  }

  if( name.find('@') > -1 )
  {
    if( !mNotifyClick )
    {
      kapp->invokeMailer( KURL( name ) );
    }
    else
    {
      emit mailClick( TQString::null, name );
    }
  }
  else
  {
    if( !mNotifyClick )
    {
      kapp->invokeBrowser( name );
    }
    else
    {
      emit urlClick( name );
    }
  }
}


void KTextBrowser::keyPressEvent(TQKeyEvent *e)
{
  if( e->key() == Key_Escape )
  {
    e->ignore();
  }
  else if( e->key() == Key_F1 )
  {
    e->ignore();
  }
  else
  {
    TQTextBrowser::keyPressEvent(e);
  }
}

void KTextBrowser::viewportMouseMoveEvent( TQMouseEvent* e)
{
  // do this first so we get the right type of cursor
  TQTextBrowser::viewportMouseMoveEvent(e);

  if ( viewport()->cursor().shape() == PointingHandCursor )
    viewport()->setCursor( KCursor::handCursor() );
}

void KTextBrowser::contentsWheelEvent( TQWheelEvent *e )
{
    if ( TDEGlobalSettings::wheelMouseZooms() )
        TQTextBrowser::contentsWheelEvent( e );
    else // thanks, we don't want to zoom, so skip QTextEdit's impl.
        TQScrollView::contentsWheelEvent( e );
}

TQPopupMenu *KTextBrowser::createPopupMenu( const TQPoint & pos )
{
    enum { IdUndo, IdRedo, IdSep1, IdCut, IdCopy, IdPaste, IdClear, IdSep2, IdSelectAll };

    TQPopupMenu *popup = TQTextBrowser::createPopupMenu( pos );

    if ( isReadOnly() )
      popup->changeItem( popup->idAt(0), SmallIconSet("edit-copy"), popup->text( popup->idAt(0) ) );
    else {
      int id = popup->idAt(0);
      popup->changeItem( id - IdUndo, SmallIconSet("edit-undo"), popup->text( id - IdUndo) );
      popup->changeItem( id - IdRedo, SmallIconSet("edit-redo"), popup->text( id - IdRedo) );
      popup->changeItem( id - IdCut, SmallIconSet("edit-cut"), popup->text( id - IdCut) );
      popup->changeItem( id - IdCopy, SmallIconSet("edit-copy"), popup->text( id - IdCopy) );
      popup->changeItem( id - IdPaste, SmallIconSet("edit-paste"), popup->text( id - IdPaste) );
      popup->changeItem( id - IdClear, SmallIconSet("edit-clear"), popup->text( id - IdClear) );
    }

    return popup;
}

void KTextBrowser::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ktextbrowser.moc"
