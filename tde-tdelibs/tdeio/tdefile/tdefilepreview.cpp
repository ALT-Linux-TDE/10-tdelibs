/* This file is part of the KDE libraries
    Copyright (C) 1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  2000 Werner Trobin <wtrobin@carinthia.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tdeaction.h>
#include <tdefilepreview.h>
#include <tdefilepreview.moc>
#include <tdelocale.h>

#include <tqlabel.h>

#include "config-tdefile.h"

KFilePreview::KFilePreview(KFileView *view, TQWidget *parent, const char *name)
    : TQSplitter(parent, name), KFileView()
{
    if ( view )
        init( view );
    else
        init( new KFileIconView( (TQSplitter*) this, "left" ));
}


KFilePreview::KFilePreview(TQWidget *parent, const char *name) :
                           TQSplitter(parent, name), KFileView()
{
    init( new KFileIconView((TQSplitter*)this, "left") );
}

KFilePreview::~KFilePreview()
{
    // Why copy the actions in the first place? --ellis, 13 Jan 02.
    //// don't delete the view's actions (inserted into our collection)!
    //for ( uint i = 0; i < left->actionCollection()->count(); i++ )
    //    actionCollection()->take( left->actionCollection()->action( i ));

    // don't delete the preview, we can reuse it
    // (it will get deleted by ~KDirOperator)
    if ( preview && preview->parentWidget() == this ) {
        preview->reparent(0L, 0, TQPoint(0, 0), false);
    }
}

void KFilePreview::init( KFileView *view )
{
    setViewName( i18n("Preview") );

    left = 0L;
    setFileView( view );

    preview = new TQWidget((TQSplitter*)this, "preview");
    TQString tmp = i18n("No preview available.");
    TQLabel *l = new TQLabel(tmp, preview);
    l->setMinimumSize(l->sizeHint());
    l->move(10, 5);
    preview->setMinimumWidth(l->sizeHint().width()+20);
    setResizeMode(preview, TQSplitter::KeepSize);

    // Why copy the actions? --ellis, 13 Jan 02.
    //for ( uint i = 0; i < view->actionCollection()->count(); i++ )
    //    actionCollection()->insert( view->actionCollection()->action( i ));
}

void KFilePreview::setFileView( KFileView *view )
{
    Q_ASSERT( view );

    // Why copy the actions? --ellis, 13 Jan 02.
    //if ( left ) { // remove any previous actions
    //    for ( uint i = 0; i < left->actionCollection()->count(); i++ )
    //        actionCollection()->take( left->actionCollection()->action( i ));
    //}

    delete left;
    view->widget()->reparent( this, TQPoint(0,0) );
    view->KFileView::setViewMode(All);
    view->setParentView(this);
    view->setSorting( sorting() );
    left = view;

    connect( left->signaler(), TQ_SIGNAL( fileHighlighted(const KFileItem*) ),
             TQ_SLOT( slotHighlighted( const KFileItem * )));

    // Why copy the actions? --ellis, 13 Jan 02.
    //for ( uint i = 0; i < view->actionCollection()->count(); i++ )
    //    actionCollection()->insert( view->actionCollection()->action( i ));
}

// this url parameter is useless... it's the url of the current directory.
// what for?
void KFilePreview::setPreviewWidget(const TQWidget *w, const KURL &)
{
    left->setOnlyDoubleClickSelectsFiles( onlyDoubleClickSelectsFiles() );

    if (w) {
        connect(this, TQ_SIGNAL( showPreview(const KURL &) ),
                w, TQ_SLOT( showPreview(const KURL &) ));
        connect( this, TQ_SIGNAL( clearPreview() ),
                w, TQ_SLOT( clearPreview() ));
    }
    else {
        preview->hide();
        return;
    }

    delete preview;
    preview = const_cast<TQWidget*>(w);
    preview->reparent((TQSplitter*)this, 0, TQPoint(0, 0), true);
    preview->resize(preview->sizeHint());
    preview->show();
}

void KFilePreview::insertItem(KFileItem *item)
{
    KFileView::insertItem( item );
    left->insertItem(item);
}

void KFilePreview::setSorting( TQDir::SortSpec sort )
{
    left->setSorting( sort );
    KFileView::setSorting( left->sorting() );
}

void KFilePreview::clearView()
{
    left->clearView();
    emit clearPreview();
}

void KFilePreview::updateView(bool b)
{
    left->updateView(b);
    if(preview)
        preview->repaint(b);
}

void KFilePreview::updateView(const KFileItem *i)
{
    left->updateView(i);
}

void KFilePreview::removeItem(const KFileItem *i)
{
    if ( left->isSelected( i ) )
        emit clearPreview();

    left->removeItem(i);
    KFileView::removeItem( i );
}

void KFilePreview::listingCompleted()
{
    left->listingCompleted();
}

void KFilePreview::clear()
{
    KFileView::clear();
    left->KFileView::clear();
}

void KFilePreview::clearSelection()
{
    left->clearSelection();
    emit clearPreview();
}

void KFilePreview::selectAll()
{
    left->selectAll();
}

void KFilePreview::invertSelection()
{
    left->invertSelection();
}

bool KFilePreview::isSelected( const KFileItem *i ) const
{
    return left->isSelected( i );
}

void KFilePreview::setSelectionMode(KFile::SelectionMode sm) {
    left->setSelectionMode( sm );
}

void KFilePreview::setSelected(const KFileItem *item, bool enable) {
    left->setSelected( item, enable );
}

void KFilePreview::setCurrentItem( const KFileItem *item )
{
    left->setCurrentItem( item );
}

KFileItem * KFilePreview::currentFileItem() const
{
    return left->currentFileItem();
}

void KFilePreview::slotHighlighted(const KFileItem* item)
{
    if ( item )
        emit showPreview( item->url() );

    else { // item = 0 -> multiselection mode
        const KFileItemList *items = selectedItems();
        if ( items->count() == 1 )
            emit showPreview( items->getFirst()->url() );
        else
            emit clearPreview();
    }

    // the preview widget appears and takes some space of the left view,
    // so we may have to scroll to make the current item visible
    left->ensureItemVisible(item);
 }

void KFilePreview::ensureItemVisible(const KFileItem *item)
{
    left->ensureItemVisible(item);
}

KFileItem * KFilePreview::firstFileItem() const
{
    return left->firstFileItem();
}

KFileItem * KFilePreview::nextItem( const KFileItem *item ) const
{
    return left->nextItem( item );
}

KFileItem * KFilePreview::prevItem( const KFileItem *item ) const
{
    return left->prevItem( item );
}

TDEActionCollection * KFilePreview::actionCollection() const
{
    if ( left )
        return left->actionCollection();
    else {
        kdWarning() << "KFilePreview::actionCollection(): called before setFileView()." << endl; //ellis
        return KFileView::actionCollection();
    }
}

void KFilePreview::readConfig( TDEConfig *config, const TQString& group )
{
    left->readConfig( config, group );
}

void KFilePreview::writeConfig( TDEConfig *config, const TQString& group )
{
    left->writeConfig( config, group );
}

void KFilePreview::virtual_hook( int id, void* data )
{ KFileView::virtual_hook( id, data ); }

