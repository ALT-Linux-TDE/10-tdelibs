/* This file is part of the KDEproject
   Copyright (C) 2000 David Faure <faure@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <kdebug.h>
#include <tdefileitem.h>
#include <kicontheme.h>

#include "tdefiletreeviewitem.h"

/* --- KFileTreeViewItem --- */
/*
 */
KFileTreeViewItem::KFileTreeViewItem( KFileTreeViewItem *parent,
				      KFileItem* item,
				      KFileTreeBranch *brnch )
   : TDEListViewItem( parent ),
     m_tdefileitem( item ),
     m_branch( brnch ),
     m_wasListed(false)
{
   setPixmap(0, item->pixmap( TDEIcon::SizeSmall ));
   setText( 0, item->text());

}

KFileTreeViewItem::KFileTreeViewItem( KFileTreeView* parent,
				      KFileItem* item,
				      KFileTreeBranch *brnch )
   :TDEListViewItem( (TQListView*)parent ),
    m_tdefileitem(item ),
    m_branch( brnch ),
    m_wasListed(false)
{
   setPixmap(0, item->pixmap( TDEIcon::SizeSmall ));
   setText( 0, item->text());
}

KFileTreeViewItem::~KFileTreeViewItem()
{
    if ( m_tdefileitem )
        m_tdefileitem->removeExtraData( m_branch );
}

bool KFileTreeViewItem::alreadyListed() const
{
   return m_wasListed;
}

void KFileTreeViewItem::setListed( bool wasListed )
{
   m_wasListed = wasListed;
}

KURL KFileTreeViewItem::url() const
{
    return m_tdefileitem ? m_tdefileitem->url() : KURL();
}

TQString KFileTreeViewItem::path()  const
{
    return m_tdefileitem ? m_tdefileitem->url().path() : TQString::null;
}

bool KFileTreeViewItem::isDir() const
{
    return m_tdefileitem ? m_tdefileitem->isDir() : false;
}
