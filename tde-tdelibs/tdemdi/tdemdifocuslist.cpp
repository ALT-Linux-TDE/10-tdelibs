/* This file is part of the KDE project
  Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>

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

#include "tdemdifocuslist.h"
#include "tdemdifocuslist.moc"
#include <tqobjectlist.h>
#include <kdebug.h>

KMdiFocusList::KMdiFocusList( TQObject *parent ) : TQObject( parent )
{}

KMdiFocusList::~KMdiFocusList()
{}

void KMdiFocusList::addWidgetTree( TQWidget* w )
{
	//this method should never be called twice on the same hierarchy
	m_list.insert( w, w->focusPolicy() );
	w->setFocusPolicy( TQWidget::ClickFocus );
	kdDebug( 760 ) << "KMdiFocusList::addWidgetTree: adding toplevel" << endl;
	connect( w, TQ_SIGNAL( destroyed( TQObject * ) ), this, TQ_SLOT( objectHasBeenDestroyed( TQObject* ) ) );
	TQObjectList *l = w->queryList( "TQWidget" );
	TQObjectListIt it( *l );
	TQObject *obj;
	while ( ( obj = it.current() ) != 0 )
	{
		TQWidget * wid = ( TQWidget* ) obj;
		m_list.insert( wid, wid->focusPolicy() );
		wid->setFocusPolicy( TQWidget::ClickFocus );
		kdDebug( 760 ) << "KMdiFocusList::addWidgetTree: adding widget" << endl;
		connect( wid, TQ_SIGNAL( destroyed( TQObject * ) ), this, TQ_SLOT( objectHasBeenDestroyed( TQObject* ) ) );
		++it;
	}
	delete l;
}

void KMdiFocusList::restore()
{
	for ( TQMap<TQWidget*, TQWidget::FocusPolicy>::const_iterator it = m_list.constBegin();it != m_list.constEnd();++it )
	{
		it.key() ->setFocusPolicy( it.data() );
	}
	m_list.clear();
}


void KMdiFocusList::objectHasBeenDestroyed( TQObject * o )
{
	if ( !o || !o->isWidgetType() )
		return ;
	TQWidget *w = ( TQWidget* ) o;
	m_list.remove( w );
}
