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

#include <tqapplication.h>
#include <tqheader.h>
#include <tqtimer.h>
#include <kdebug.h>
#include <kdirnotify_stub.h>
#include <tdeglobalsettings.h>
#include <tdefileitem.h>
#include <tdefileview.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <stdlib.h>
#include <assert.h>
#include <tdeio/job.h>
#include <tdeio/global.h>
#include <kurldrag.h>
#include <kiconloader.h>


#include "tdefiletreeview.h"
#include "tdefiletreebranch.h"
#include "tdefiletreeviewitem.h"

KFileTreeView::KFileTreeView( TQWidget *parent, const char *name )
    : TDEListView( parent, name ),
      m_wantOpenFolderPixmaps( true ),
      m_toolTip( this )
{
    setDragEnabled(true);
    setSelectionModeExt( TDEListView::Single );

    m_animationTimer = new TQTimer( this );
    connect( m_animationTimer, TQ_SIGNAL( timeout() ),
             this, TQ_SLOT( slotAnimation() ) );

    m_currentBeforeDropItem = 0;
    m_dropItem = 0;

    m_autoOpenTimer = new TQTimer( this );
    connect( m_autoOpenTimer, TQ_SIGNAL( timeout() ),
             this, TQ_SLOT( slotAutoOpenFolder() ) );

    /* The executed-Slot only opens  a path, while the expanded-Slot populates it */
    connect( this, TQ_SIGNAL( executed( TQListViewItem * ) ),
             this, TQ_SLOT( slotExecuted( TQListViewItem * ) ) );
    connect( this, TQ_SIGNAL( expanded ( TQListViewItem *) ),
    	     this, TQ_SLOT( slotExpanded( TQListViewItem *) ));
    connect( this, TQ_SIGNAL( collapsed( TQListViewItem *) ),
	     this, TQ_SLOT( slotCollapsed( TQListViewItem* )));


    /* connections from the konqtree widget */
    connect( this, TQ_SIGNAL( selectionChanged() ),
             this, TQ_SLOT( slotSelectionChanged() ) );
    connect( this, TQ_SIGNAL( onItem( TQListViewItem * )),
	     this, TQ_SLOT( slotOnItem( TQListViewItem * ) ) );
    connect( this, TQ_SIGNAL(itemRenamed(TQListViewItem*, const TQString &, int)),
             this, TQ_SLOT(slotItemRenamed(TQListViewItem*, const TQString &, int)));


    m_bDrag = false;
    m_branches.setAutoDelete( true );

    m_openFolderPixmap = DesktopIcon( "folder_open",TDEIcon::SizeSmall,TDEIcon::ActiveState );
}

KFileTreeView::~KFileTreeView()
{
   // we must make sure that the KFileTreeViewItems are deleted _before_ the
   // branches are deleted. Otherwise, the KFileItems would be destroyed
   // and the KFileTreeViewItems had dangling pointers to them.
   hide();
   clear();
   m_branches.clear(); // finally delete the branches and KFileItems
}


bool KFileTreeView::isValidItem( TQListViewItem *item)
{
   if (!item)
      return false;
   TQPtrList<TQListViewItem> lst;
   TQListViewItemIterator it( this );
   while ( it.current() )
   {
      if ( it.current() == item )
         return true;
      ++it;
   }
   return false;
}

void KFileTreeView::contentsDragEnterEvent( TQDragEnterEvent *ev )
{
   if ( ! acceptDrag( ev ) )
   {
      ev->ignore();
      return;
   }
   ev->acceptAction();
   m_currentBeforeDropItem = selectedItem();

   TQListViewItem *item = itemAt( contentsToViewport( ev->pos() ) );
   if( item )
   {
      m_dropItem = item;
      m_autoOpenTimer->start( KFileView::autoOpenDelay() );
   }
   else
   {
   m_dropItem = 0;
}
}

void KFileTreeView::contentsDragMoveEvent( TQDragMoveEvent *e )
{
   if( ! acceptDrag( e ) )
   {
      e->ignore();
      return;
   }
   e->acceptAction();


   TQListViewItem *afterme;
   TQListViewItem *parent;

   findDrop( e->pos(), parent, afterme );

   // "afterme" is 0 when aiming at a directory itself
   TQListViewItem *item = afterme ? afterme : parent;

   if( item && item->isSelectable() )
   {
      setSelected( item, true );
      if( item != m_dropItem ) {
	 m_autoOpenTimer->stop();
	 m_dropItem = item;
	 m_autoOpenTimer->start( KFileView::autoOpenDelay() );
      }
   }
   else
   {
      m_autoOpenTimer->stop();
      m_dropItem = 0;
   }
}

void KFileTreeView::contentsDragLeaveEvent( TQDragLeaveEvent * )
{
   // Restore the current item to what it was before the dragging (#17070)
   if ( isValidItem(m_currentBeforeDropItem) )
   {
      setSelected( m_currentBeforeDropItem, true );
      ensureItemVisible( m_currentBeforeDropItem );
   }
   else if ( isValidItem(m_dropItem) )
      setSelected( m_dropItem, false ); // no item selected
   m_currentBeforeDropItem = 0;
   m_dropItem = 0;

}

void KFileTreeView::contentsDropEvent( TQDropEvent *e )
{

    m_autoOpenTimer->stop();
    m_dropItem = 0;

    kdDebug(250) << "contentsDropEvent !" << endl;
    if( ! acceptDrag( e ) ) {
       e->ignore();
       return;
    }

    e->acceptAction();
    TQListViewItem *afterme;
    TQListViewItem *parent;
    findDrop(e->pos(), parent, afterme);

    //kdDebug(250) << " parent=" << (parent?parent->text(0):TQString::null)
    //             << " afterme=" << (afterme?afterme->text(0):TQString::null) << endl;

    if (e->source() == viewport() && itemsMovable())
        movableDropEvent(parent, afterme);
    else
    {
       emit dropped(e, afterme);
       emit dropped(this, e, afterme);
       emit dropped(e, parent, afterme);
       emit dropped(this, e, parent, afterme);

       KURL::List urls;
       KURLDrag::decode( e, urls );
       emit dropped( this, e, urls );

       KURL parentURL;
       if( parent )
           parentURL = static_cast<KFileTreeViewItem*>(parent)->url();
       else
           // can happen when dropping above the root item
           // Should we choose the first branch in such a case ??
           return;

       emit dropped( urls, parentURL );
       emit dropped( this , e, urls, parentURL );
    }
}

bool KFileTreeView::acceptDrag(TQDropEvent* e ) const
{

   bool ancestOK= acceptDrops();
   // kdDebug(250) << "Do accept drops: " << ancestOK << endl;
   ancestOK = ancestOK && itemsMovable();
   // kdDebug(250) << "acceptDrag: " << ancestOK << endl;
   // kdDebug(250) << "canDecode: " << KURLDrag::canDecode(e) << endl;
   // kdDebug(250) << "action: " << e->action() << endl;

   /*  TDEListView::acceptDrag(e);  */
   /* this is what TDEListView does:
    * acceptDrops() && itemsMovable() && (e->source()==viewport());
    * ask acceptDrops and itemsMovable, but not the third
    */
   return ancestOK && KURLDrag::canDecode( e ) &&
       // Why this test? All DnDs are one of those AFAIK (DF)
      ( e->action() == TQDropEvent::Copy
	|| e->action() == TQDropEvent::Move
	|| e->action() == TQDropEvent::Link );
}



TQDragObject * KFileTreeView::dragObject()
{

   KURL::List urls;
   const TQPtrList<TQListViewItem> fileList = selectedItems();
   TQPtrListIterator<TQListViewItem> it( fileList );
   for ( ; it.current(); ++it )
   {
      urls.append( static_cast<KFileTreeViewItem*>(it.current())->url() );
   }
   TQPoint hotspot;
   TQPixmap pixmap;
   if( urls.count() > 1 ){
      pixmap = DesktopIcon( "application-vnd.tde.tdemultiple", 16 );
   }
   if( pixmap.isNull() )
      pixmap = currentKFileTreeViewItem()->fileItem()->pixmap( 16 );
   hotspot.setX( pixmap.width() / 2 );
   hotspot.setY( pixmap.height() / 2 );
   TQDragObject* dragObject = new KURLDrag( urls, this );
   if( dragObject )
      dragObject->setPixmap( pixmap, hotspot );
   return dragObject;
}



void KFileTreeView::slotCollapsed( TQListViewItem *item )
{
   KFileTreeViewItem *kftvi = static_cast<KFileTreeViewItem*>(item);
   kdDebug(250) << "hit slotCollapsed" << endl;
   if( kftvi && kftvi->isDir())
   {
      item->setPixmap( 0, itemIcon(kftvi));
   }
}

void KFileTreeView::slotExpanded( TQListViewItem *item )
{
   kdDebug(250) << "slotExpanded here !" << endl;

   if( ! item ) return;

   KFileTreeViewItem *it = static_cast<KFileTreeViewItem*>(item);
   KFileTreeBranch *branch = it->branch();

   /* Start the animation for the branch object */
   if( it->isDir() && branch && item->childCount() == 0 )
   {
      /* check here if the branch really needs to be populated again */
      kdDebug(250 ) << "starting to open " << it->url().prettyURL() << endl;
      startAnimation( it );
      bool branchAnswer = branch->populate( it->url(), it );
      kdDebug(250) << "Branches answer: " << branchAnswer << endl;
      if( ! branchAnswer )
      {
	 kdDebug(250) << "ERR: Could not populate!" << endl;
	 stopAnimation( it );
      }
   }

   /* set a pixmap 'open folder' */
   if( it->isDir() && isOpen( item ) )
   {
      kdDebug(250)<< "Setting open Pixmap" << endl;
      item->setPixmap( 0, itemIcon( it )); // 0, m_openFolderPixmap );
   }
}



void KFileTreeView::slotExecuted( TQListViewItem *item )
{
    if ( !item )
        return;
    /* This opens the dir and causes the Expanded-slot to be called,
     * which strolls through the children.
     */
    if( static_cast<KFileTreeViewItem*>(item)->isDir())
    {
       item->setOpen( !item->isOpen() );
    }
}


void KFileTreeView::slotAutoOpenFolder()
{
   m_autoOpenTimer->stop();

   if ( !isValidItem(m_dropItem) || m_dropItem->isOpen() )
      return;

   m_dropItem->setOpen( true );
   m_dropItem->repaint();
}


void KFileTreeView::slotSelectionChanged()
{
   if ( !m_dropItem ) // don't do this while the dragmove thing
   {
   }
}


KFileTreeBranch* KFileTreeView::addBranch( const KURL &path, const TQString& name,
                              bool showHidden )
{
    const TQPixmap& folderPix = KMimeType::mimeType("inode/directory")->pixmap( TDEIcon::Desktop,TDEIcon::SizeSmall );

    return addBranch( path, name, folderPix, showHidden);
}

KFileTreeBranch* KFileTreeView::addBranch( const KURL &path, const TQString& name,
                              const TQPixmap& pix, bool showHidden )
{
   kdDebug(250) << "adding another root " << path.prettyURL() << endl;

   /* Open a new branch */
   KFileTreeBranch *newBranch = new KFileTreeBranch( this, path, name, pix,
                                                     showHidden );
   return addBranch(newBranch);
}

KFileTreeBranch *KFileTreeView::addBranch(KFileTreeBranch *newBranch)
{
   connect( newBranch, TQ_SIGNAL(populateFinished( KFileTreeViewItem* )),
            this, TQ_SLOT( slotPopulateFinished( KFileTreeViewItem* )));

   connect( newBranch, TQ_SIGNAL( newTreeViewItems( KFileTreeBranch*,
                               const KFileTreeViewItemList& )),
            this, TQ_SLOT( slotNewTreeViewItems( KFileTreeBranch*,
                        const KFileTreeViewItemList& )));

   m_branches.append( newBranch );
   return( newBranch );
}

KFileTreeBranch *KFileTreeView::branch( const TQString& searchName )
{
   KFileTreeBranch *branch = 0;
   TQPtrListIterator<KFileTreeBranch> it( m_branches );

   while ( (branch = it.current()) != 0 ) {
      ++it;
      TQString bname = branch->name();
      kdDebug(250) << "This is the branches name: " << bname << endl;
      if( bname == searchName )
      {
	 kdDebug(250) << "Found branch " << bname << " and return ptr" << endl;
	 return( branch );
      }
   }
   return ( 0L );
}

KFileTreeBranchList& KFileTreeView::branches()
{
   return( m_branches );
}


bool KFileTreeView::removeBranch( KFileTreeBranch *branch )
{
   if(m_branches.contains(branch))
   {
      delete (branch->root());
      m_branches.remove( branch );
      return true;
   }
   else
   {
      return false;
   }
}

void KFileTreeView::setDirOnlyMode( KFileTreeBranch* branch, bool bom )
{
   if( branch )
   {
      branch->setDirOnlyMode( bom );
   }
}


void KFileTreeView::slotPopulateFinished( KFileTreeViewItem *it )
{
   if( it && it->isDir())
    stopAnimation( it );
}

void KFileTreeView::slotNewTreeViewItems( KFileTreeBranch* branch, const KFileTreeViewItemList& itemList )
{
   if( ! branch ) return;
   kdDebug(250) << "hitting slotNewTreeViewItems" << endl;

   /* Sometimes it happens that new items should become selected, i.e. if the user
    * creates a new dir, he probably wants it to be selected. This can not be done
    * right after creating the directory or file, because it takes some time until
    * the item appears here in the treeview. Thus, the creation code sets the member
    * m_neUrlToSelect to the required url. If this url appears here, the item becomes
    * selected and the member nextUrlToSelect will be cleared.
    */
   if( ! m_nextUrlToSelect.isEmpty() )
   {
      KFileTreeViewItemListIterator it( itemList );

      bool end = false;
      for( ; !end && it.current(); ++it )
      {
	 KURL url = (*it)->url();

	 if( m_nextUrlToSelect.equals(url, true ))   // ignore trailing / on dirs
	 {
	    setCurrentItem( static_cast<TQListViewItem*>(*it) );
	    m_nextUrlToSelect = KURL();
	    end = true;
	 }
      }
   }
}

TQPixmap KFileTreeView::itemIcon( KFileTreeViewItem *item, int gap ) const
{
   TQPixmap pix;
   kdDebug(250) << "Setting icon for column " << gap << endl;

   if( item )
   {
      /* Check if it is a branch root */
      KFileTreeBranch *brnch = item->branch();
      if( item == brnch->root() )
      {
	 pix = brnch->pixmap();
	 if( m_wantOpenFolderPixmaps && brnch->root()->isOpen() )
	 {
	    pix = brnch->openPixmap();
	 }
      }
      else
      {
         // TODO: different modes, user Pixmaps ?
         pix = item->fileItem()->pixmap( TDEIcon::SizeSmall ); // , TDEIcon::DefaultState);

         /* Only if it is a dir and the user wants open dir pixmap and it is open,
          * change the fileitem's pixmap to the open folder pixmap. */
         if( item->isDir() && m_wantOpenFolderPixmaps )
         {
            if( isOpen( static_cast<TQListViewItem*>(item)))
               pix = m_openFolderPixmap;
         }
      }
   }

   return pix;
}


void KFileTreeView::slotAnimation()
{
   MapCurrentOpeningFolders::Iterator it = m_mapCurrentOpeningFolders.begin();
   MapCurrentOpeningFolders::Iterator end = m_mapCurrentOpeningFolders.end();
   for (; it != end;)
   {
      KFileTreeViewItem *item = it.key();
      if (!isValidItem(item))
      {
         ++it;
         m_mapCurrentOpeningFolders.remove(item);
         continue;
      }
         
      uint & iconNumber = it.data().iconNumber;
      TQString icon = TQString::fromLatin1( it.data().iconBaseName ).append( TQString::number( iconNumber ) );
      // kdDebug(250) << "Loading icon " << icon << endl;
      item->setPixmap( 0, DesktopIcon( icon,TDEIcon::SizeSmall,TDEIcon::ActiveState )); // KFileTreeViewFactory::instance() ) );

      iconNumber++;
      if ( iconNumber > it.data().iconCount )
	 iconNumber = 1;

      ++it;
   }
}


void KFileTreeView::startAnimation( KFileTreeViewItem * item, const char * iconBaseName, uint iconCount )
{
   /* TODO: allow specific icons */
   if( ! item )
   {
      kdDebug(250) << " startAnimation Got called without valid item !" << endl;
      return;
   }

   m_mapCurrentOpeningFolders.insert( item,
                                      AnimationInfo( iconBaseName,
                                                     iconCount,
                                                     itemIcon(item, 0) ) );
   if ( !m_animationTimer->isActive() )
      m_animationTimer->start( 50 );
}

void KFileTreeView::stopAnimation( KFileTreeViewItem * item )
{
   if( ! item ) return;

   kdDebug(250) << "Stoping Animation !" << endl;

   MapCurrentOpeningFolders::Iterator it = m_mapCurrentOpeningFolders.find(item);
   if ( it != m_mapCurrentOpeningFolders.end() )
   {
      if( item->isDir() && isOpen( item) )
      {
	 kdDebug(250) << "Setting folder open pixmap !" << endl;
	 item->setPixmap( 0, itemIcon( item ));
      }
      else
      {
	 item->setPixmap( 0, it.data().originalPixmap );
      }
      m_mapCurrentOpeningFolders.remove( item );
   }
   else
   {
      if( item )
	 kdDebug(250)<< "StopAnimation - could not find item " << item->url().prettyURL()<< endl;
      else
	 kdDebug(250)<< "StopAnimation - item is zero !" << endl;
   }
   if (m_mapCurrentOpeningFolders.isEmpty())
      m_animationTimer->stop();
}

KFileTreeViewItem * KFileTreeView::currentKFileTreeViewItem() const
{
   return static_cast<KFileTreeViewItem *>( selectedItem() );
}

KURL KFileTreeView::currentURL() const
{
    KFileTreeViewItem *item = currentKFileTreeViewItem();
    if ( item )
        return currentKFileTreeViewItem()->url();
    else
        return KURL();
}

void KFileTreeView::slotOnItem( TQListViewItem *item )
{
    KFileTreeViewItem *i = static_cast<KFileTreeViewItem *>( item );
    if( i )
    {
       const KURL url = i->url();
       if ( url.isLocalFile() )
	  emit onItem( url.path() );
       else
	  emit onItem( url.prettyURL() );
    }
}

void KFileTreeView::slotItemRenamed(TQListViewItem* item, const TQString &name, int col)
{
   (void) item;
   kdDebug(250) << "Do not bother: " << name << col << endl;
}

KFileTreeViewItem *KFileTreeView::findItem( const TQString& branchName, const TQString& relUrl )
{
   KFileTreeBranch *br = branch( branchName );
   return( findItem( br, relUrl ));
}

KFileTreeViewItem *KFileTreeView::findItem( KFileTreeBranch* brnch, const TQString& relUrl )
{
   KFileTreeViewItem *ret = 0;
   if( brnch )
   {
      KURL url = brnch->rootUrl();

      if( ! relUrl.isEmpty() && TQDir::isRelativePath(relUrl) )
      {
         TQString partUrl( relUrl );

         if( partUrl.endsWith("/"))
            partUrl.truncate( relUrl.length()-1 );

         url.addPath( partUrl );

         kdDebug(250) << "assembled complete dir string " << url.prettyURL() << endl;

         KFileItem *fi = brnch->findByURL( url );
         if( fi )
         {
            ret = static_cast<KFileTreeViewItem*>( fi->extraData( brnch ));
            kdDebug(250) << "Found item !" <<ret << endl;
         }
      }
      else
      {
         ret = brnch->root();
      }
   }
   return( ret );
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


void KFileTreeViewToolTip::maybeTip( const TQPoint & )
{
#if 0
    TQListViewItem *item = m_view->itemAt( point );
    if ( item ) {
	TQString text = static_cast<KFileViewItem*>( item )->toolTipText();
	if ( !text.isEmpty() )
	    tip ( m_view->itemRect( item ), text );
    }
#endif
}

void KFileTreeView::virtual_hook( int id, void* data )
{ TDEListView::virtual_hook( id, data ); }

#include "tdefiletreeview.moc"
