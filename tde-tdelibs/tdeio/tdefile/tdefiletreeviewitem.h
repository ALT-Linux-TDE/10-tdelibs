/* This file is part of the KDE project
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

#ifndef TDEFILE_TREE_VIEW_ITEM_H
#define TDEFILE_TREE_VIEW_ITEM_H

#include <tqptrlist.h>
#include <tdelistview.h>

#include <tdefileitem.h>
#include <tdeio/global.h>
#include <kdirlister.h>
#include <tdeio/job.h>

class KURL;
class KFileTreeView;
class KFileTreeBranch;
class KFileTreeItem;


/**
 * An item for a KFileTreeView that knows about its own KFileItem.
 */
class TDEIO_EXPORT KFileTreeViewItem : public TDEListViewItem
{
public:
   KFileTreeViewItem( KFileTreeViewItem*, KFileItem*, KFileTreeBranch * );
   KFileTreeViewItem( KFileTreeView*, KFileItem*, KFileTreeBranch * );
   ~KFileTreeViewItem();

   /**
    * @return the KFileTreeBranch the item is sorted in.
    */
   KFileTreeBranch* branch() const     { return m_branch; }

   /**
    * @return the KFileItem the viewitem is representing.
    */
   KFileItem *fileItem() const         { return m_tdefileitem; }

   /**
    * @return the path of the item.
    */
   TQString path() const;

   /**
    * @return the items KURL
    */
   KURL    url() const;

   /**
    * @return if the item represents a directory
    */
   bool    isDir() const;

   /**
    * @return if this directory was already seen by a KDirLister.
    */
   bool    alreadyListed() const;

   /**
    * set the flag if the directory was already listed.
    */
   void    setListed( bool wasListed );

protected:

private:

   KFileItem *m_tdefileitem;
   KFileTreeBranch *m_branch;
   bool  m_wasListed;
   class KFileTreeViewItemPrivate;
   KFileTreeViewItemPrivate *d;
};


/**
 * List of KFileTreeViewItems
 */
typedef TQPtrList<KFileTreeViewItem> KFileTreeViewItemList;

/**
 * Iterator for KFileTreeViewItemList
 */
typedef TQPtrListIterator<KFileTreeViewItem> KFileTreeViewItemListIterator;


#endif

