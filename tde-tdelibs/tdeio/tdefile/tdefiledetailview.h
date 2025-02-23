/* This file is part of the KDE libraries
    Copyright (C) 1997 Stephan Kulow <coolo@kde.org>

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

#ifndef TDEFILEDETAILVIEW_H
#define TDEFILEDETAILVIEW_H

class KFileItem;
class TQWidget;
class TQKeyEvent;

#include <tdelistview.h>
#include <kmimetyperesolver.h>

#include "tdefileview.h"

/**
 * An item for the listiew, that has a reference to its corresponding
 * KFileItem.
 */
class TDEIO_EXPORT KFileListViewItem : public TDEListViewItem
{
public:
    KFileListViewItem( TQListView *parent, const TQString &text,
		       const TQPixmap &icon, KFileItem *fi )
	: TDEListViewItem( parent, text ), inf( fi ) {
        setPixmap( 0, icon );
        setText( 0, text );
    }

    /**
     * @since 3.1
     */
    KFileListViewItem( TQListView *parent, KFileItem *fi )
        : TDEListViewItem( parent ), inf( fi ) {
        init();
    }

    KFileListViewItem( TQListView *parent, const TQString &text,
		       const TQPixmap &icon, KFileItem *fi,
		       TQListViewItem *after)
	: TDEListViewItem( parent, after ), inf( fi ) {
        setPixmap( 0, icon );
        setText( 0, text );
    }
    ~KFileListViewItem() {
        inf->removeExtraData( listView() );
    }

    /**
     * @returns the corresponding KFileItem
     */
    KFileItem *fileInfo() const {
	return inf;
    }

    virtual TQString key( int /*column*/, bool /*ascending*/ ) const {
        return m_key;
    }

    void setKey( const TQString& key ) { m_key = key; }

    TQRect rect() const
    {
        TQRect r = listView()->itemRect(this);
        return TQRect( listView()->viewportToContents( r.topLeft() ),
                      TQSize( r.width(), r.height() ) );
    }

    /**
     * @since 3.1
     */
    void init();

private:
    KFileItem *inf;
    TQString m_key;

private:
    class KFileListViewItemPrivate;
    KFileListViewItemPrivate *d;

};

/**
 * A list-view capable of showing KFileItem'. Used in the filedialog
 * for example. Most of the documentation is in KFileView class.
 *
 * @see KDirOperator
 * @see KCombiView
 * @see KFileIconView
 */
class TDEIO_EXPORT KFileDetailView : public TDEListView, public KFileView
{
    TQ_OBJECT

public:
    KFileDetailView(TQWidget *parent, const char *name);
    virtual ~KFileDetailView();

    virtual TQWidget *widget() { return this; }
    virtual void clearView();
    virtual void setAutoUpdate( bool ) {} // ### unused. remove in KDE4

    virtual void setSelectionMode( KFile::SelectionMode sm );

    virtual void updateView( bool );
    virtual void updateView(const KFileItem*);
    virtual void removeItem( const KFileItem *);
    virtual void listingCompleted();

    virtual void setSelected(const KFileItem *, bool);
    virtual bool isSelected(const KFileItem *i) const;
    virtual void clearSelection();
    virtual void selectAll();
    virtual void invertSelection();

    virtual void setCurrentItem( const KFileItem * );
    virtual KFileItem * currentFileItem() const;
    virtual KFileItem * firstFileItem() const;
    virtual KFileItem * nextItem( const KFileItem * ) const;
    virtual KFileItem * prevItem( const KFileItem * ) const;

    virtual void insertItem( KFileItem *i );

    // implemented to get noticed about sorting changes (for sortingIndicator)
    virtual void setSorting( TQDir::SortSpec );

    void ensureItemVisible( const KFileItem * );

    // for KMimeTypeResolver
    void mimeTypeDeterminationFinished();
    void determineIcon( KFileListViewItem *item );
    TQScrollView *scrollWidget() const { return (TQScrollView*) this; }

    virtual void readConfig( TDEConfig *, const TQString& group = TQString::null );
    virtual void writeConfig( TDEConfig *, const TQString& group = TQString::null);

signals:
    /**
     * The user dropped something.
     * @p fileItem points to the item dropped on or can be 0 if the
     * user dropped on empty space.
     * @since 3.2
     */
    void dropped(TQDropEvent *event, KFileItem *fileItem);
    /**
     * The user dropped the URLs @p urls.
     * @p url points to the item dropped on or can be empty if the
     * user dropped on empty space.
     * @since 3.2
     */
    void dropped(TQDropEvent *event, const KURL::List &urls, const KURL &url);

protected:
    virtual void keyPressEvent( TQKeyEvent * );

    // DND support
    virtual TQDragObject *dragObject();
    virtual void contentsDragEnterEvent( TQDragEnterEvent *e );
    virtual void contentsDragMoveEvent( TQDragMoveEvent *e );
    virtual void contentsDragLeaveEvent( TQDragLeaveEvent *e );
    virtual void contentsDropEvent( TQDropEvent *ev );
    virtual bool acceptDrag(TQDropEvent* e ) const;

    int m_sortingCol;

protected slots:
    void slotSelectionChanged();

private slots:
    void slotSortingChanged( int );
    void selected( TQListViewItem *item );
    void slotActivate( TQListViewItem *item );
    void highlighted( TQListViewItem *item );
    void slotActivateMenu ( TQListViewItem *item, const TQPoint& pos );
    void slotAutoOpen();

private:
    virtual void insertItem(TQListViewItem *i) { TDEListView::insertItem(i); }
    virtual void setSorting(int i, bool b) { TDEListView::setSorting(i, b); }
    virtual void setSelected(TQListViewItem *i, bool b) { TDEListView::setSelected(i, b); }

    inline KFileListViewItem * viewItem( const KFileItem *item ) const {
        if ( item )
            return (KFileListViewItem *) item->extraData( this );
        return 0L;
    }

    void setSortingKey( KFileListViewItem *item, const KFileItem *i );


    bool m_blockSortingSignal;
    KMimeTypeResolver<KFileListViewItem,KFileDetailView> *m_resolver;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KFileDetailViewPrivate;
    KFileDetailViewPrivate *d;
};

#endif // TDEFILEDETAILVIEW_H
