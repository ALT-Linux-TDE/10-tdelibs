/* This file is part of the KDE libraries
   Copyright (C) 1997 Stephan Kulow <coolo@kde.org>
                 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.	If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <tqevent.h>
#include <tqkeycode.h>
#include <tqheader.h>
#include <tqpainter.h>
#include <tqpixmap.h>

#include <tdeapplication.h>
#include <tdefileitem.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <tdelocale.h>
#include <kdebug.h>
#include <kurldrag.h>

#include "tdefiledetailview.h"
#include "config-tdefile.h"

#define COL_NAME 0
#define COL_SIZE 1
#define COL_DATE 2
#define COL_PERM 3
#define COL_OWNER 4
#define COL_GROUP 5

class KFileDetailView::KFileDetailViewPrivate
{
public:
   KFileDetailViewPrivate() : dropItem(0)
   { }

   KFileListViewItem *dropItem;
   TQTimer autoOpenTimer;
};

KFileDetailView::KFileDetailView(TQWidget *parent, const char *name)
    : TDEListView(parent, name), KFileView(), d(new KFileDetailViewPrivate())
{
    // this is always the static section, not the index depending on column order
    m_sortingCol = COL_NAME; 
    m_blockSortingSignal = false;
    setViewName( i18n("Detailed View") );

    addColumn( i18n( "Name" ) );
    addColumn( i18n( "Size" ) );
    addColumn( i18n( "Date" ) );
    addColumn( i18n( "Permissions" ) );
    addColumn( i18n( "Owner" ) );
    addColumn( i18n( "Group" ) );
    setShowSortIndicator( true );
    setAllColumnsShowFocus( true );
    setDragEnabled(true);

    connect( header(), TQ_SIGNAL( clicked(int)),
             TQ_SLOT(slotSortingChanged(int) ));


    connect( this, TQ_SIGNAL( returnPressed(TQListViewItem *) ),
	     TQ_SLOT( slotActivate( TQListViewItem *) ) );

    connect( this, TQ_SIGNAL( clicked(TQListViewItem *, const TQPoint&, int)),
	     TQ_SLOT( selected( TQListViewItem *) ) );
    connect( this, TQ_SIGNAL( doubleClicked(TQListViewItem *, const TQPoint&, int)),
	     TQ_SLOT( slotActivate( TQListViewItem *) ) );

    connect( this, TQ_SIGNAL(contextMenuRequested( TQListViewItem *,
                                                const TQPoint &, int )),
	     this, TQ_SLOT( slotActivateMenu( TQListViewItem *, const TQPoint& )));

    KFile::SelectionMode sm = KFileView::selectionMode();
    switch ( sm ) {
    case KFile::Multi:
	TQListView::setSelectionMode( TQListView::Multi );
	break;
    case KFile::Extended:
	TQListView::setSelectionMode( TQListView::Extended );
	break;
    case KFile::NoSelection:
	TQListView::setSelectionMode( TQListView::NoSelection );
	break;
    default: // fall through
    case KFile::Single:
	TQListView::setSelectionMode( TQListView::Single );
	break;
    }

    // for highlighting
    if ( sm == KFile::Multi || sm == KFile::Extended )
	connect( this, TQ_SIGNAL( selectionChanged() ),
		 TQ_SLOT( slotSelectionChanged() ));
    else
	connect( this, TQ_SIGNAL( selectionChanged( TQListViewItem * ) ),
		 TQ_SLOT( highlighted( TQListViewItem * ) ));
		
    // DND
    connect( &(d->autoOpenTimer), TQ_SIGNAL( timeout() ),
             this, TQ_SLOT( slotAutoOpen() ));

    setSorting( sorting() );

    m_resolver =
        new KMimeTypeResolver<KFileListViewItem,KFileDetailView>( this );
}

KFileDetailView::~KFileDetailView()
{
    delete m_resolver;
    delete d;
}

void KFileDetailView::readConfig( TDEConfig *config, const TQString& group )
{
    restoreLayout( config, group );
}

void KFileDetailView::writeConfig( TDEConfig *config, const TQString& group )
{
    saveLayout( config, group );
}

void KFileDetailView::setSelected( const KFileItem *info, bool enable )
{
    if ( !info )
	return;

    // we can only hope that this casts works
    KFileListViewItem *item = (KFileListViewItem*)info->extraData( this );

    if ( item )
	TDEListView::setSelected( item, enable );
}

void KFileDetailView::setCurrentItem( const KFileItem *item )
{
    if ( !item )
        return;
    KFileListViewItem *it = (KFileListViewItem*) item->extraData( this );
    if ( it )
        TDEListView::setCurrentItem( it );
}

KFileItem * KFileDetailView::currentFileItem() const
{
    KFileListViewItem *current = static_cast<KFileListViewItem*>( currentItem() );
    if ( current )
        return current->fileInfo();

    return 0L;
}

void KFileDetailView::clearSelection()
{
    TDEListView::clearSelection();
}

void KFileDetailView::selectAll()
{
    if (KFileView::selectionMode() == KFile::NoSelection ||
        KFileView::selectionMode() == KFile::Single)
	return;

    TDEListView::selectAll( true );
}

void KFileDetailView::invertSelection()
{
    TDEListView::invertSelection();
}

void KFileDetailView::slotActivateMenu (TQListViewItem *item,const TQPoint& pos )
{
    if ( !item ) {
        sig->activateMenu( 0, pos );
        return;
    }
    KFileListViewItem *i = (KFileListViewItem*) item;
    sig->activateMenu( i->fileInfo(), pos );
}

void KFileDetailView::clearView()
{
    m_resolver->m_lstPendingMimeIconItems.clear();
    TDEListView::clear();
}

void KFileDetailView::insertItem( KFileItem *i )
{
    KFileView::insertItem( i );

    KFileListViewItem *item = new KFileListViewItem( (TQListView*) this, i );

    setSortingKey( item, i );

    i->setExtraData( this, item );

    if ( !i->isMimeTypeKnown() )
        m_resolver->m_lstPendingMimeIconItems.append( item );
}

void KFileDetailView::slotActivate( TQListViewItem *item )
{
    if ( !item )
        return;

    const KFileItem *fi = ( (KFileListViewItem*)item )->fileInfo();
    if ( fi )
        sig->activate( fi );
}

void KFileDetailView::selected( TQListViewItem *item )
{
    if ( !item )
        return;

    if ( TDEGlobalSettings::singleClick() ) {
        const KFileItem *fi = ( (KFileListViewItem*)item )->fileInfo();
        if ( fi && (fi->isDir() || !onlyDoubleClickSelectsFiles()) )
            sig->activate( fi );
    }
}

void KFileDetailView::highlighted( TQListViewItem *item )
{
    if ( !item )
        return;

    const KFileItem *fi = ( (KFileListViewItem*)item )->fileInfo();
    if ( fi )
        sig->highlightFile( fi );
}


void KFileDetailView::setSelectionMode( KFile::SelectionMode sm )
{
    disconnect( this, TQ_SIGNAL( selectionChanged() ));
    disconnect( this, TQ_SIGNAL( selectionChanged( TQListViewItem * ) ));

    KFileView::setSelectionMode( sm );

    switch ( KFileView::selectionMode() ) {
    case KFile::Multi:
        TQListView::setSelectionMode( TQListView::Multi );
        break;
    case KFile::Extended:
        TQListView::setSelectionMode( TQListView::Extended );
        break;
    case KFile::NoSelection:
        TQListView::setSelectionMode( TQListView::NoSelection );
        break;
    default: // fall through
    case KFile::Single:
        TQListView::setSelectionMode( TQListView::Single );
        break;
    }

    if ( sm == KFile::Multi || sm == KFile::Extended )
        connect( this, TQ_SIGNAL( selectionChanged() ),
                 TQ_SLOT( slotSelectionChanged() ));
    else
        connect( this, TQ_SIGNAL( selectionChanged( TQListViewItem * )),
                 TQ_SLOT( highlighted( TQListViewItem * )));
}

bool KFileDetailView::isSelected( const KFileItem *i ) const
{
    if ( !i )
        return false;

    KFileListViewItem *item = (KFileListViewItem*) i->extraData( this );
    return (item && item->isSelected());
}


void KFileDetailView::updateView( bool b )
{
    if ( !b )
        return;

    TQListViewItemIterator it( (TQListView*)this );
    for ( ; it.current(); ++it ) {
        KFileListViewItem *item=static_cast<KFileListViewItem *>(it.current());
        item->setPixmap( 0, item->fileInfo()->pixmap(TDEIcon::SizeSmall) );
    }
}

void KFileDetailView::updateView( const KFileItem *i )
{
    if ( !i )
        return;

    KFileListViewItem *item = (KFileListViewItem*) i->extraData( this );
    if ( !item )
        return;

    item->init();
    setSortingKey( item, i );

    //item->repaint(); // only repaints if visible
}

void KFileDetailView::setSortingKey( KFileListViewItem *item,
                                     const KFileItem *i )
{
    // see also setSorting()
    TQDir::SortSpec spec = KFileView::sorting();

    if ( spec & TQDir::Time )
        item->setKey( sortingKey( i->time( TDEIO::UDS_MODIFICATION_TIME ),
                                  i->isDir(), spec ));
    else if ( spec & TQDir::Size )
        item->setKey( sortingKey( i->size(), i->isDir(), spec ));

    else // Name or Unsorted
        item->setKey( sortingKey( i->text(), i->isDir(), spec ));
}


void KFileDetailView::removeItem( const KFileItem *i )
{
    if ( !i )
        return;

    KFileListViewItem *item = (KFileListViewItem*) i->extraData( this );
    m_resolver->m_lstPendingMimeIconItems.remove( item );
    delete item;

    KFileView::removeItem( i );
}

void KFileDetailView::slotSortingChanged( int col )
{
    // col is the section here, not the index!
    
    TQDir::SortSpec sort = sorting();
    int sortSpec = -1;
    bool reversed = (col == m_sortingCol) && (sort & TQDir::Reversed) == 0;
    m_sortingCol = col;

    switch( col ) {
        case COL_NAME:
            sortSpec = ((sort & ~TQDir::SortByMask) | TQDir::Name);
            break;
        case COL_SIZE:
            sortSpec = ((sort & ~TQDir::SortByMask) | TQDir::Size);
            break;
        case COL_DATE:
            sortSpec = ((sort & ~TQDir::SortByMask) | TQDir::Time);
            break;

        // the following columns have no equivalent in TQDir, so we set it
        // to TQDir::Unsorted and remember the column (m_sortingCol)
        case COL_OWNER:
        case COL_GROUP:
        case COL_PERM:
            // grmbl, TQDir::Unsorted == SortByMask.
            sortSpec = (sort & ~TQDir::SortByMask);// | TQDir::Unsorted;
            break;
        default:
            break;
    }

    if ( reversed )
        sortSpec |= TQDir::Reversed;
    else
        sortSpec &= ~TQDir::Reversed;

    if ( sort & TQDir::IgnoreCase )
        sortSpec |= TQDir::IgnoreCase;
    else
        sortSpec &= ~TQDir::IgnoreCase;


    KFileView::setSorting( static_cast<TQDir::SortSpec>( sortSpec ) );

    KFileItem *item;
    KFileItemListIterator it( *items() );

    if ( sortSpec & TQDir::Time ) {
        for ( ; (item = it.current()); ++it )
            viewItem(item)->setKey( sortingKey( item->time( TDEIO::UDS_MODIFICATION_TIME ), item->isDir(), sortSpec ));
    }

    else if ( sortSpec & TQDir::Size ) {
        for ( ; (item = it.current()); ++it )
            viewItem(item)->setKey( sortingKey( item->size(), item->isDir(),
                                                sortSpec ));
    }
    else { // Name or Unsorted -> use column text
        for ( ; (item = it.current()); ++it ) {
            KFileListViewItem *i = viewItem( item );
            i->setKey( sortingKey( i->text(m_sortingCol), item->isDir(),
                                   sortSpec ));
        }
    }

    TDEListView::setSorting( m_sortingCol, !reversed );
    TDEListView::sort();

    if ( !m_blockSortingSignal )
        sig->changeSorting( static_cast<TQDir::SortSpec>( sortSpec ) );
}


void KFileDetailView::setSorting( TQDir::SortSpec spec )
{
    int col = 0;
    if ( spec & TQDir::Time )
        col = COL_DATE;
    else if ( spec & TQDir::Size )
        col = COL_SIZE;
    else if ( spec & TQDir::Unsorted )
        col = m_sortingCol;
    else
        col = COL_NAME;

    // inversed, because slotSortingChanged will reverse it
    if ( spec & TQDir::Reversed )
        spec = (TQDir::SortSpec) (spec & ~TQDir::Reversed);
    else
        spec = (TQDir::SortSpec) (spec | TQDir::Reversed);

    m_sortingCol = col;
    KFileView::setSorting( (TQDir::SortSpec) spec );


    // don't emit sortingChanged() when called via setSorting()
    m_blockSortingSignal = true; // can't use blockSignals()
    slotSortingChanged( col );
    m_blockSortingSignal = false;
}

void KFileDetailView::ensureItemVisible( const KFileItem *i )
{
    if ( !i )
        return;

    KFileListViewItem *item = (KFileListViewItem*) i->extraData( this );

    if ( item )
        TDEListView::ensureItemVisible( item );
}

// we're in multiselection mode
void KFileDetailView::slotSelectionChanged()
{
    sig->highlightFile( 0L );
}

KFileItem * KFileDetailView::firstFileItem() const
{
    KFileListViewItem *item = static_cast<KFileListViewItem*>( firstChild() );
    if ( item )
        return item->fileInfo();
    return 0L;
}

KFileItem * KFileDetailView::nextItem( const KFileItem *fileItem ) const
{
    if ( fileItem ) {
        KFileListViewItem *item = viewItem( fileItem );
        if ( item && item->itemBelow() )
            return ((KFileListViewItem*) item->itemBelow())->fileInfo();
        else
            return 0L;
    }
    else
        return firstFileItem();
}

KFileItem * KFileDetailView::prevItem( const KFileItem *fileItem ) const
{
    if ( fileItem ) {
        KFileListViewItem *item = viewItem( fileItem );
        if ( item && item->itemAbove() )
            return ((KFileListViewItem*) item->itemAbove())->fileInfo();
        else
            return 0L;
    }
    else
        return firstFileItem();
}

void KFileDetailView::keyPressEvent( TQKeyEvent *e )
{
    TDEListView::keyPressEvent( e );

    if ( e->key() == Key_Return || e->key() == Key_Enter ) {
        if ( e->state() & ControlButton )
            e->ignore();
        else
            e->accept();
    }
}

//
// mimetype determination on demand
//
void KFileDetailView::mimeTypeDeterminationFinished()
{
    // anything to do?
}

void KFileDetailView::determineIcon( KFileListViewItem *item )
{
    (void) item->fileInfo()->determineMimeType();
    updateView( item->fileInfo() );
}

void KFileDetailView::listingCompleted()
{
    m_resolver->start();
}

TQDragObject *KFileDetailView::dragObject()
{
    // create a list of the URL:s that we want to drag
    KURL::List urls;
    KFileItemListIterator it( * KFileView::selectedItems() );
    for ( ; it.current(); ++it ){
        urls.append( (*it)->url() );
    }
    TQPixmap pixmap;
    if( urls.count() > 1 )
        pixmap = DesktopIcon( "application-vnd.tde.tdemultiple", TDEIcon::SizeSmall );
    if( pixmap.isNull() )
        pixmap = currentFileItem()->pixmap( TDEIcon::SizeSmall );

    TQPoint hotspot;
    hotspot.setX( pixmap.width() / 2 );
    hotspot.setY( pixmap.height() / 2 );
    TQDragObject* myDragObject = new KURLDrag( urls, widget() );
    myDragObject->setPixmap( pixmap, hotspot );
    return myDragObject;
}

void KFileDetailView::slotAutoOpen()
{
    d->autoOpenTimer.stop();
    if( !d->dropItem )
        return;

    KFileItem *fileItem = d->dropItem->fileInfo();
    if (!fileItem)
        return;

    if( fileItem->isFile() )
        return;

    if ( fileItem->isDir() || fileItem->isLink())
        sig->activate( fileItem );
}

bool KFileDetailView::acceptDrag(TQDropEvent* e) const
{
   return KURLDrag::canDecode( e ) &&
       (e->source()!= const_cast<KFileDetailView*>(this)) &&
       ( e->action() == TQDropEvent::Copy
      || e->action() == TQDropEvent::Move
      || e->action() == TQDropEvent::Link );
}

void KFileDetailView::contentsDragEnterEvent( TQDragEnterEvent *e )
{
    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptAction();     // Yes

    if ((dropOptions() & AutoOpenDirs) == 0)
       return;

    KFileListViewItem *item = dynamic_cast<KFileListViewItem*>(itemAt( contentsToViewport( e->pos() ) ));
    if ( item ) {  // are we over an item ?
       d->dropItem = item;
       d->autoOpenTimer.start( autoOpenDelay() ); // restart timer
    }
    else
    {
       d->dropItem = 0;
       d->autoOpenTimer.stop();
    }
}

void KFileDetailView::contentsDragMoveEvent( TQDragMoveEvent *e )
{
    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptAction();     // Yes

    if ((dropOptions() & AutoOpenDirs) == 0)
       return;

    KFileListViewItem *item = dynamic_cast<KFileListViewItem*>(itemAt( contentsToViewport( e->pos() ) ));
    if ( item ) {  // are we over an item ?
       if (d->dropItem != item)
       {
           d->dropItem = item;
           d->autoOpenTimer.start( autoOpenDelay() ); // restart timer
       }
    }
    else
    {
       d->dropItem = 0;
       d->autoOpenTimer.stop();
    }
}

void KFileDetailView::contentsDragLeaveEvent( TQDragLeaveEvent * )
{
    d->dropItem = 0;
    d->autoOpenTimer.stop();
}

void KFileDetailView::contentsDropEvent( TQDropEvent *e )
{
    d->dropItem = 0;
    d->autoOpenTimer.stop();

    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptAction();     // Yes

    KFileListViewItem *item = dynamic_cast<KFileListViewItem*>(itemAt( contentsToViewport( e->pos() ) ));
    KFileItem * fileItem = 0;
    if (item)
        fileItem = item->fileInfo();

    emit dropped(e, fileItem);

    KURL::List urls;
    if (KURLDrag::decode( e, urls ) && !urls.isEmpty())
    {
        emit dropped(e, urls, fileItem ? fileItem->url() : KURL());
        sig->dropURLs(fileItem, e, urls);
    }
}


/////////////////////////////////////////////////////////////////


void KFileListViewItem::init()
{
    KFileListViewItem::setPixmap( COL_NAME, inf->pixmap(TDEIcon::SizeSmall));

    setText( COL_NAME, inf->text() );
    setText( COL_SIZE, TDEGlobal::locale()->formatNumber( inf->size(), 0));
    setText( COL_DATE,  inf->timeString() );
    setText( COL_PERM,  inf->permissionsString() );
    setText( COL_OWNER, inf->user() );
    setText( COL_GROUP, inf->group() );
}


void KFileDetailView::virtual_hook( int id, void* data )
{ TDEListView::virtual_hook( id, data );
  KFileView::virtual_hook( id, data ); }

#include "tdefiledetailview.moc"
