/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>
		 2002 Klaas Freitag <freitag@suse.de>

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

#ifndef TDEFILE_TREE_VIEW_H
#define TDEFILE_TREE_VIEW_H

#include <tqmap.h>
#include <tqpoint.h>
#include <tqpixmap.h>
#include <tqstrlist.h>
#include <tqtooltip.h>

#include <tdelistview.h>
#include <kdirnotify.h>
#include <tdeio/job.h>
#include <tdefiletreeviewitem.h>
#include <tdefiletreebranch.h>

class TQTimer;



class TDEIO_EXPORT KFileTreeViewToolTip : public TQToolTip
{
public:
    KFileTreeViewToolTip( TQListView *view ) : TQToolTip( view ), m_view( view ) {}

protected:
    virtual void maybeTip( const TQPoint & );

private:
    TQListView *m_view;
};


/**
 * The filetreeview offers a treeview on the file system which behaves like
 * a QTreeView showing files and/or directories in the file system.
 *
 * KFileTreeView is able to handle more than one URL, represented by
 * KFileTreeBranch.
 *
 * Typical usage:
 * 1. create a KFileTreeView fitting in your layout and add columns to it
 * 2. call addBranch to create one or more branches
 * 3. retrieve the root item with KFileTreeBranch::root() and set it open
 *    if desired. That starts the listing.
 */
class TDEIO_EXPORT KFileTreeView : public TDEListView
{
    TQ_OBJECT
public:
    KFileTreeView( TQWidget *parent, const char *name = 0 );
    virtual ~KFileTreeView();

    /**
     * @return the current (i.e. selected) item
     */
    KFileTreeViewItem * currentKFileTreeViewItem() const;

   /**
    * @return the URL of the current selected item.
    */
    KURL currentURL() const;

   /**
    *  Adds a branch to the treeview item.
    *
    *  This high-level function creates the branch, adds it to the treeview and
    *  connects some signals. Note that directory listing does not start until
    *  a branch is expanded either by opening the root item by user or by setOpen
    *  on the root item.
    *
    *  @returns a pointer to the new branch or zero
    *  @param path is the base url of the branch
    *  @param name is the name of the branch, which will be the text for column 0
    *  @param showHidden says if hidden files and directories should be visible
    */
   KFileTreeBranch* addBranch( const KURL &path, const TQString& name, bool showHidden = false );

   /**
    *  same as the function above but with a pixmap to set for the branch.
    */
   virtual KFileTreeBranch* addBranch( const KURL &path, const TQString& name ,
				       const TQPixmap& pix, bool showHidden = false  );

   /**
    *  same as the function above but letting the user create the branch.
    */
   virtual KFileTreeBranch* addBranch( KFileTreeBranch * );

   /**
    *  removes the branch from the treeview.
    *  @param branch is a pointer to the branch
    *  @returns true on success.
    */
   virtual bool removeBranch( KFileTreeBranch *branch );

   /**
    *  @returns a pointer to the KFileTreeBranch in the KFileTreeView or zero on failure.
    *  @param searchName is the name of a branch
    */
   KFileTreeBranch *branch( const TQString& searchName );


   /**
    *  @returns a list of pointers to all existing branches in the treeview.
    **/
   KFileTreeBranchList& branches();

   /**
    *  set the directory mode for branches. If true is passed, only directories will be loaded.
    *  @param branch is a pointer to a KFileTreeBranch
    */
   virtual void setDirOnlyMode( KFileTreeBranch *branch, bool );

   /**
    * searches a branch for a KFileTreeViewItem identified by the relative url given as
    * second parameter. The method adds the branches base url to the relative path and finds
    * the item.
    * @returns a pointer to the item or zero if the item does not exist.
    * @param brnch  is a pointer to the branch to search in
    * @param relUrl is the branch relativ url
    */
   KFileTreeViewItem *findItem( KFileTreeBranch* brnch, const TQString& relUrl );

   /**
    * see method above, differs only in the first parameter. Finds the branch by its name.
    */
   KFileTreeViewItem *findItem( const TQString& branchName, const TQString& relUrl );

   /**
    * @returns a flag indicating if extended folder pixmaps are displayed or not.
    */
   bool showFolderOpenPixmap() const { return m_wantOpenFolderPixmaps; };

public slots:

   /**
    * set the flag to show 'extended' folder icons on or off. If switched on, folders will
    * have an open folder pixmap displayed if their children are visible, and the standard
    * closed folder pixmap (from mimetype folder) if they are closed.
    * If switched off, the plain mime pixmap is displayed.
    * @param showIt = false displays mime type pixmap only
    */
   virtual void setShowFolderOpenPixmap( bool showIt = true )
      { m_wantOpenFolderPixmaps = showIt; }

protected:
   /**
    * @returns true if we can decode the drag and support the action
    */

   virtual bool acceptDrag(TQDropEvent* event) const;
    virtual TQDragObject * dragObject();

    virtual void startAnimation( KFileTreeViewItem* item, const char * iconBaseName = "kde", uint iconCount = 6 );
    virtual void stopAnimation( KFileTreeViewItem* item );
    virtual void contentsDragEnterEvent( TQDragEnterEvent *e );
    virtual void contentsDragMoveEvent( TQDragMoveEvent *e );
    virtual void contentsDragLeaveEvent( TQDragLeaveEvent *e );
    virtual void contentsDropEvent( TQDropEvent *ev );

protected slots:
    virtual void slotNewTreeViewItems( KFileTreeBranch*,
				       const KFileTreeViewItemList& );

    virtual void slotSetNextUrlToSelect( const KURL &url )
      { m_nextUrlToSelect = url; }

    virtual TQPixmap itemIcon( KFileTreeViewItem*, int gap = 0 ) const;

private slots:
    void slotExecuted( TQListViewItem * );
    void slotExpanded( TQListViewItem * );
    void slotCollapsed( TQListViewItem *item );

    void slotSelectionChanged();

    void slotAnimation();

    void slotAutoOpenFolder();

    void slotOnItem( TQListViewItem * );
    void slotItemRenamed(TQListViewItem*, const TQString &, int);

   void slotPopulateFinished( KFileTreeViewItem* );


signals:

   void onItem( const TQString& );
   /* New signals if you like it ? */
   void dropped( TQWidget*, TQDropEvent* );
   void dropped( TQWidget*, TQDropEvent*, KURL::List& );
   void dropped( KURL::List&, KURL& );
   // The drop event allows to differentiate between move and copy
   void dropped( TQWidget*, TQDropEvent*, KURL::List&, KURL& );

   void dropped( TQDropEvent *e, TQListViewItem * after);
   void dropped(KFileTreeView *, TQDropEvent *, TQListViewItem *);
   void dropped(TQDropEvent *e, TQListViewItem * parent, TQListViewItem * after);
   void dropped(KFileTreeView *, TQDropEvent *, TQListViewItem *, TQListViewItem *);

protected:
   KURL m_nextUrlToSelect;


private:
    // Returns whether item is still a valid item in the tree
    bool isValidItem( TQListViewItem *item);
    void clearTree();


   /* List that holds the branches */
    KFileTreeBranchList m_branches;


    struct AnimationInfo
    {
        AnimationInfo( const char * _iconBaseName, uint _iconCount, const TQPixmap & _originalPixmap )
            : iconBaseName(_iconBaseName), iconCount(_iconCount), iconNumber(1), originalPixmap(_originalPixmap) {}
        AnimationInfo() : iconCount(0) {}
        TQCString iconBaseName;
        uint iconCount;
        uint iconNumber;
        TQPixmap originalPixmap;
    };
    typedef TQMap<KFileTreeViewItem *, AnimationInfo> MapCurrentOpeningFolders;
    MapCurrentOpeningFolders m_mapCurrentOpeningFolders;


    TQTimer *m_animationTimer;

    TQPoint m_dragPos;
    bool m_bDrag;

   bool m_wantOpenFolderPixmaps; // Flag weather the folder should have open-folder pixmaps

    TQListViewItem *m_currentBeforeDropItem; // The item that was current before the drag-enter event happened
    TQListViewItem *m_dropItem; // The item we are moving the mouse over (during a drag)
    TQStrList m_lstDropFormats;
   TQPixmap  m_openFolderPixmap;
    TQTimer *m_autoOpenTimer;

    KFileTreeViewToolTip m_toolTip;


protected:
   virtual void virtual_hook( int id, void* data );
private:
   class KFileTreeViewPrivate;
   KFileTreeViewPrivate *d;
};

#endif
