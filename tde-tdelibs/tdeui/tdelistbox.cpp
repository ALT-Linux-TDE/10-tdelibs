/* This file is part of the KDE libraries
   Copyright (C) 2000 Reginald Stadlbauer <reggie@kde.org>

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
#include "config.h"

#include <tqtimer.h>

#include <tdeglobalsettings.h>
#include <kcursor.h>
#include <tdeapplication.h>
#include <kipc.h> 
#include <kdebug.h>

#include "tdelistbox.h"

TDEListBox::TDEListBox( TQWidget *parent, const char *name, WFlags f )
    : TQListBox( parent, name, f ), d(0)
{
    connect( this, TQ_SIGNAL( onViewport() ),
	     this, TQ_SLOT( slotOnViewport() ) );
    connect( this, TQ_SIGNAL( onItem( TQListBoxItem * ) ),
	     this, TQ_SLOT( slotOnItem( TQListBoxItem * ) ) );
    slotSettingsChanged(TDEApplication::SETTINGS_MOUSE);
    if (kapp)
    {
        connect( kapp, TQ_SIGNAL( settingsChanged(int) ), TQ_SLOT( slotSettingsChanged(int) ) );
        kapp->addKipcEventMask( KIPC::SettingsChanged );
    }

    m_pCurrentItem = 0L;

    m_pAutoSelect = new TQTimer( this );
    connect( m_pAutoSelect, TQ_SIGNAL( timeout() ),
    	     this, TQ_SLOT( slotAutoSelect() ) );
}

void TDEListBox::slotOnItem( TQListBoxItem *item )
{
    if ( item && m_bChangeCursorOverItem && m_bUseSingle )
        viewport()->setCursor( KCursor().handCursor() );

    if ( item && (m_autoSelectDelay > -1) && m_bUseSingle ) {
      m_pAutoSelect->start( m_autoSelectDelay, true );
      m_pCurrentItem = item;
    }
}

void TDEListBox::slotOnViewport()
{
    if ( m_bChangeCursorOverItem )
        viewport()->unsetCursor();

    m_pAutoSelect->stop();
    m_pCurrentItem = 0L;
}


void TDEListBox::slotSettingsChanged(int category)
{
    if (category != TDEApplication::SETTINGS_MOUSE)
        return;
    m_bUseSingle = TDEGlobalSettings::singleClick();

    disconnect( this, TQ_SIGNAL( mouseButtonClicked( int, TQListBoxItem *,
						  const TQPoint & ) ),
		this, TQ_SLOT( slotMouseButtonClicked( int, TQListBoxItem *,
						    const TQPoint & ) ) );
//         disconnect( this, TQ_SIGNAL( doubleClicked( TQListBoxItem *, 
// 						 const TQPoint & ) ),
// 		    this, TQ_SLOT( slotExecute( TQListBoxItem *, 
// 					     const TQPoint & ) ) );

    if( m_bUseSingle )
    {
      connect( this, TQ_SIGNAL( mouseButtonClicked( int, TQListBoxItem *, 
						 const TQPoint & ) ),
	       this, TQ_SLOT( slotMouseButtonClicked( int, TQListBoxItem *,
						   const TQPoint & ) ) );
    }
    else
    {
//         connect( this, TQ_SIGNAL( doubleClicked( TQListBoxItem *, 
// 					      const TQPoint & ) ),
//                  this, TQ_SLOT( slotExecute( TQListBoxItem *, 
// 					  const TQPoint & ) ) );
    }

    m_bChangeCursorOverItem = TDEGlobalSettings::changeCursorOverIcon();
    m_autoSelectDelay = TDEGlobalSettings::autoSelectDelay();

    if( !m_bUseSingle || !m_bChangeCursorOverItem )
        viewport()->unsetCursor();
}

void TDEListBox::slotAutoSelect()
{
  // check that the item still exists
  if( index( m_pCurrentItem ) == -1 )
    return;

  //Give this widget the keyboard focus.
  if( !hasFocus() )
    setFocus();

  ButtonState keybstate = TDEApplication::keyboardMouseState();

  TQListBoxItem* previousItem = item( currentItem() ); 
  setCurrentItem( m_pCurrentItem );

  if( m_pCurrentItem ) {
    //Shift pressed?
    if( (keybstate & ShiftButton) ) {
      bool block = signalsBlocked();
      blockSignals( true );

      //No Ctrl? Then clear before!
      if( !(keybstate & ControlButton) )  
	clearSelection(); 

      bool select = !m_pCurrentItem->isSelected();
      bool update = viewport()->isUpdatesEnabled();
      viewport()->setUpdatesEnabled( false );

      bool down = index( previousItem ) < index( m_pCurrentItem );
      TQListBoxItem* it = down ? previousItem : m_pCurrentItem;
      for (;it ; it = it->next() ) {
	if ( down && it == m_pCurrentItem ) {
	  setSelected( m_pCurrentItem, select );
	  break;
	}
	if ( !down && it == previousItem ) {
	  setSelected( previousItem, select );
	  break;
	}
	setSelected( it, select );
      }
      
      blockSignals( block );
      viewport()->setUpdatesEnabled( update );
      triggerUpdate( false );

      emit selectionChanged();

      if( selectionMode() == TQListBox::Single )
	emit selectionChanged( m_pCurrentItem );
    }
    else if( (keybstate & ControlButton) )
      setSelected( m_pCurrentItem, !m_pCurrentItem->isSelected() );
    else {
      bool block = signalsBlocked();
      blockSignals( true );

      if( !m_pCurrentItem->isSelected() )
	clearSelection(); 

      blockSignals( block );

      setSelected( m_pCurrentItem, true );
    }
  }
  else
    kdDebug() << "That´s not supposed to happen!!!!" << endl;
}

void TDEListBox::emitExecute( TQListBoxItem *item, const TQPoint &pos )
{
  ButtonState keybstate = TDEApplication::keyboardMouseState();
    
  m_pAutoSelect->stop();
  
  //Don´t emit executed if in SC mode and Shift or Ctrl are pressed
  if( !( m_bUseSingle && ((keybstate & ShiftButton) || (keybstate & ControlButton)) ) ) {
    emit executed( item );
    emit executed( item, pos );
  }
}

//
// 2000-16-01 Espen Sand
// This widget is used in dialogs. It should ignore
// F1 (and combinations) and Escape since these are used
// to start help or close the dialog. This functionality
// should be done in TQListView but it is not (at least now)
//
void TDEListBox::keyPressEvent(TQKeyEvent *e)
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
    TQListBox::keyPressEvent(e);
  }
}

void TDEListBox::focusOutEvent( TQFocusEvent *fe )
{
  m_pAutoSelect->stop();

  TQListBox::focusOutEvent( fe );
}

void TDEListBox::leaveEvent( TQEvent *e ) 
{
  m_pAutoSelect->stop();

  TQListBox::leaveEvent( e );
}

void TDEListBox::contentsMousePressEvent( TQMouseEvent *e )
{
  if( (selectionMode() == Extended) && (e->state() & ShiftButton) && !(e->state() & ControlButton) ) {
    bool block = signalsBlocked();
    blockSignals( true );

    clearSelection();

    blockSignals( block );
  }

  TQListBox::contentsMousePressEvent( e );
}

void TDEListBox::contentsMouseDoubleClickEvent ( TQMouseEvent * e )
{
  TQListBox::contentsMouseDoubleClickEvent( e );

  TQListBoxItem* item = itemAt( contentsToViewport( e->pos() ) );

  if( item ) {
    emit doubleClicked( item, e->globalPos() );

    if( (e->button() == TQt::LeftButton) && !m_bUseSingle )
      emitExecute( item, e->globalPos() );
  }
}

void TDEListBox::slotMouseButtonClicked( int btn, TQListBoxItem *item, const TQPoint &pos )
{
  if( (btn == TQt::LeftButton) && item )
    emitExecute( item, pos );
}

void TDEListBox::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "tdelistbox.moc"
