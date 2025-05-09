
/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>
                 2001 Klaas Freitag <freitag@suse.de>

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

#ifndef TDEFILE_TREE_BRANCH_H
#define TDEFILE_TREE_BRANCH_H

#include <tqdict.h>
#include <tqlistview.h>

#include <tdefileitem.h>
#include <tdeio/global.h>
#include <kdirlister.h>
#include <tdeio/job.h>
#include <tdefiletreeviewitem.h>

class KURL;
class KFileTreeView;


/**
 * This is the branch class of the KFileTreeView, which represents one
 * branch in the treeview. Every branch has a root which is an url. The branch
 * lists the files under the root. Every branch uses its own dirlister and can
 * have its own filter etc.
 *
 * @short Branch object for KFileTreeView object.
 *
 */

class TDEIO_EXPORT KFileTreeBranch : public KDirLister
{
   TQ_OBJECT
public:
   /**
    * constructs a branch for KFileTreeView. Does not yet start to list it.
    * @param url start url of the branch.
    * @param name the name of the branch, which is displayed in the first column of the treeview.
    * @param pix is a pixmap to display as an icon of the branch.
    * @param showHidden flag to make hidden files visible or not.
    * @param branchRoot is the KFileTreeViewItem to use as the root of the
    *        branch, with the default 0 meaning to let KFileTreeBranch create
    *        it for you.
    */
   KFileTreeBranch( KFileTreeView*, const KURL& url, const TQString& name,
                    const TQPixmap& pix, bool showHidden = false,
		    KFileTreeViewItem *branchRoot = 0 );

   /**
    * @returns the root url of the branch.
    */
   KURL 	rootUrl() const { return( m_startURL ); }

   /**
    * sets a KFileTreeViewItem as root widget for the branch.
    * That must be created outside of the branch. All KFileTreeViewItems
    * the branch is allocating will become children of that object.
    * @param r the KFileTreeViewItem to become the root item.
    */
   virtual void 	setRoot( KFileTreeViewItem *r ){ m_root = r; };

   /**
    * @returns the root item.
    */
   KFileTreeViewItem *root( ) { return( m_root );}

   /**
    * @returns the name of the branch.
    */
   TQString      name() const { return( m_name ); }

   /**
    * sets the name of the branch.
    */
   virtual void         setName( const TQString n ) { m_name = n; };

   /*
    * returns the current root item pixmap set in the constructor. The root
    * item pixmap defaults to the icon for directories.
    * @see openPixmap()
    */
   const TQPixmap& pixmap(){ return(m_rootIcon); }

   /*
    * returns the current root item pixmap set by setOpenPixmap()
    * which is displayed if the branch is expanded.
    * The root item pixmap defaults to the icon for directories.
    * @see pixmap()
    * Note that it depends on KFileTreeView::showFolderOpenPximap weather
    * open pixmap are displayed or not.
    */
   const TQPixmap& openPixmap() { return(m_openRootIcon); }

   /**
    * @returns whether the items in the branch show their file extensions in the
    * tree or not. See setShowExtensions for more information.
    */
   bool showExtensions( ) const;

   /**
    * sets the root of the branch open or closed.
    */
   void setOpen( bool setopen = true )
      { if( root() ) root()->setOpen( setopen ); }

   /**
    * sets if children recursion is wanted or not. If this is switched off, the
    * child directories of a just opened directory are not listed internally.
    * That means that it can not be determined if the sub directories are
    * expandable or not. If this is switched off there will be no call to
    * setExpandable.
    * @param t set to true to switch on child recursion
    */
   void setChildRecurse( bool t=true );

   /**
    * @returns if child recursion is on or off.
    * @see setChildRecurse
    */
   bool childRecurse()
      { return m_recurseChildren; }

public slots:
   /**
    * populates a branch. This method must be called after a branch was added
    * to  a KFileTreeView using method addBranch.
    * @param url is the url of the root item where the branch starts.
    * @param currItem is the current parent.
    */
   virtual bool populate( const KURL &url, KFileTreeViewItem* currItem );

   /**
    * sets printing of the file extensions on or off. If you pass false to this
    * slot, all items of this branch will not show their file extensions in the
    * tree.
    * @param visible flags if the extensions should be visible or not.
    */
   virtual void setShowExtensions( bool visible = true );

   void setOpenPixmap( const TQPixmap& pix );

protected:
   /**
    * allocates a KFileTreeViewItem for the branch
    * for new items.
    */
   virtual KFileTreeViewItem *createTreeViewItem( KFileTreeViewItem *parent,
						  KFileItem *fileItem );

public:
   /**
    * find the according KFileTreeViewItem by an url
    */
   virtual KFileTreeViewItem *findTVIByURL( const KURL& );

signals:
   /**
    * emitted with the item of a directory which was finished to populate
    */
   void populateFinished( KFileTreeViewItem * );

   /**
    * emitted with a list of new or updated KFileTreeViewItem which were
    * found in a branch. Note that this signal is emitted very often and may slow
    * down the performance of the treeview !
    */
   void newTreeViewItems( KFileTreeBranch*, const KFileTreeViewItemList& );

   /**
    * emitted with the exact count of children for a directory.
    */
   void directoryChildCount( KFileTreeViewItem* item, int count );

private slots:
   void slotRefreshItems( const KFileItemList& );
   void addItems( const KFileItemList& );
   void slCompleted( const KURL& );
   void slotCanceled( const KURL& );
   void slotListerStarted( const KURL& );
   void slotDeleteItem( KFileItem* );
   void slotDirlisterClear();
   void slotDirlisterClearURL( const KURL& url );
   void slotRedirect( const KURL& oldUrl, const KURL&newUrl );

private:
   KFileTreeViewItem    *parentKFTVItem( KFileItem *item );
   static void           deleteChildrenOf( TQListViewItem *parent );

   KFileTreeViewItem 	*m_root;
   KURL 		m_startURL;
   TQString 		m_name;
   TQPixmap 		m_rootIcon;
   TQPixmap              m_openRootIcon;

   /* this list holds the url's which children are opened. */
   KURL::List           m_openChildrenURLs;


   /* The next two members are used for caching purposes in findTVIByURL. */
   KURL                 m_lastFoundURL;
   KFileTreeViewItem   *m_lastFoundItem;

   bool                 m_recurseChildren :1;
   bool                 m_showExtensions  :1;

protected:
   virtual void virtual_hook( int id, void* data );
private:
   class KFileTreeBranchPrivate;
   KFileTreeBranchPrivate *d;
};


/**
 * List of KFileTreeBranches
 */
typedef TQPtrList<KFileTreeBranch> KFileTreeBranchList;

/**
 * Iterator for KFileTreeBranchLists
 */
typedef TQPtrListIterator<KFileTreeBranch> KFileTreeBranchIterator;

#endif

