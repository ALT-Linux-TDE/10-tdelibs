/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999 Matthias Ettrich <ettrich@kde.org>
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
 *
 */
#include <tqwidget.h>
#include <tqpopupmenu.h>
#include "kcmenumngr.h"
#include "tdeglobal.h"
#include "tdeconfig.h"
#include "tdeshortcut.h"

#undef KeyPress
#undef None

template class TQPtrDict<TQPopupMenu>;

KContextMenuManager* KContextMenuManager::manager = 0;

KContextMenuManager::KContextMenuManager( TQObject* parent, const char* name )
    : TQObject( parent, name)
{
    TDEConfigGroupSaver saver ( TDEGlobal::config(), TQString::fromLatin1("Shortcuts") ) ;
    menuKey = TDEShortcut( saver.config()->readEntry(TQString::fromLatin1("PopupContextMenu"), TQString::fromLatin1("Menu") ) ).keyCodeQt();
    saver.config()->setGroup( TQString::fromLatin1("ContextMenus") ) ;
    showOnPress = saver.config()->readBoolEntry(TQString::fromLatin1("ShowOnPress"), true );
}

KContextMenuManager::~KContextMenuManager()
{
}


bool KContextMenuManager::showOnButtonPress( void )
{
  if ( !manager )
	manager = new KContextMenuManager;
  return manager->showOnPress;
}


void KContextMenuManager::insert( TQWidget* widget, TQPopupMenu* popup )
{
    if ( !manager )
	manager = new KContextMenuManager;
    
    manager->connect( widget, TQ_SIGNAL( destroyed() ), manager, TQ_SLOT( widgetDestroyed() ) );
    manager->menus.insert( widget, popup );
    widget->installEventFilter( manager );
}

bool KContextMenuManager::eventFilter( TQObject *o, TQEvent * e)
{
    TQPopupMenu* popup = 0;
    TQPoint pos;
    switch ( e->type() ) {
    case TQEvent::MouseButtonPress:
	if (((TQMouseEvent*) e )->button() != TQt::RightButton )
	    break;
	if ( !showOnPress )
	    return true; // eat event for safety
	popup = menus[o];
	pos = ((TQMouseEvent*) e )->globalPos();
	break;
    case TQEvent::MouseButtonRelease:
	if ( showOnPress  || ((TQMouseEvent*) e )->button() != TQt::RightButton )
	    break;
	popup = menus[o];	
	pos = ((TQMouseEvent*) e )->globalPos();
	break;
    case TQEvent::KeyPress:
	{
	    if ( !o->isWidgetType() )
		break;
	    TQKeyEvent *k = (TQKeyEvent *)e;
	    int key = k->key();
	    if ( k->state() & ShiftButton )
		key |= SHIFT;
	    if ( k->state() & ControlButton )
		key |= CTRL;
	    if ( k->state() & AltButton )
		key |= ALT;
	    if ( key != menuKey )
		break;
	    popup = menus[o];
	    if ( popup ) {
		TQWidget* w = (TQWidget*) o ;
	    
		// ### workaround
		pos = w->mapToGlobal( w->rect().center() );
		// with later Qt snapshot 
		// pos = w->mapToGlobal( w->microFocusHint().center() );
	    }
	}
	break;
    default: 
	break;
    }
    
    if ( popup ) {
	popup->popup( pos );
	return true;
    }
	
    return false;
}

void KContextMenuManager::widgetDestroyed()
{
    if ( menus.find( (TQObject*)sender() ) )
	menus.remove( (TQObject*)sender() );
}

#include "kcmenumngr.moc"
