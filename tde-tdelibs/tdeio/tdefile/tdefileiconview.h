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

#ifndef TDEFILEICONVIEW_H
#define TDEFILEICONVIEW_H

#include <tqt.h>

#include <kiconview.h>
#include <kiconloader.h>
#include <tdefileview.h>
#include <kmimetyperesolver.h>
#include <tdefile.h>

class KFileItem;
class TQWidget;
class TQLabel;

/**
 * An item for the iconview, that has a reference to its corresponding
 * KFileItem.
 */
class TDEIO_EXPORT KFileIconViewItem : public TDEIconViewItem
{
public:
    KFileIconViewItem( TQIconView *parent, const TQString &text,
		       const TQPixmap &pixmap,
		       KFileItem *fi )
	: TDEIconViewItem( parent, text, pixmap ), inf( fi ) {}
    /**
     * @since 3.1
     */
    KFileIconViewItem( TQIconView *parent, KFileItem *fi )
	: TDEIconViewItem( parent ), inf( fi ) {}

    virtual ~KFileIconViewItem();

    /**
     * @returns the corresponding KFileItem
     */
    KFileItem *fileInfo() const {
	return inf;
    }

private:
    KFileItem *inf;

private:
    class KFileIconViewItemPrivate;
    KFileIconViewItemPrivate *d;

};

namespace TDEIO {
    class Job;
}

/**
 * An icon-view capable of showing KFileItem's. Used in the filedialog
 * for example. Most of the documentation is in KFileView class.
 *
 * @see KDirOperator
 * @see KCombiView
 * @see KFileDetailView
 */
class TDEIO_EXPORT KFileIconView : public TDEIconView, public KFileView
{
    TQ_OBJECT

public:
    KFileIconView(TQWidget *parent, const char *name);
    virtual ~KFileIconView();

    virtual TQWidget *widget() { return this; }
    virtual void clearView();
    virtual void setAutoUpdate( bool ) {} // ### unused. remove in KDE4

    virtual void updateView( bool );
    virtual void updateView(const KFileItem*);
    virtual void removeItem(const KFileItem*);

    virtual void listingCompleted();

    virtual void insertItem( KFileItem *i );
    virtual void setSelectionMode( KFile::SelectionMode sm );

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

    /**
     * Sets the size of the icons to show. Defaults to TDEIcon::SizeSmall.
     */
    void setIconSize( int size );

    /**
     * Sets the size of the previews. Defaults to TDEIcon::SizeLarge.
     */
    void setPreviewSize( int size );

    /**
     * Disables the "Maximum file size" configuration option for previews
     *
     * Set this before calling showPreviews()
     *
     * @since 3.4
     **/
    void setIgnoreMaximumSize(bool ignoreSize=true);

    /**
     * @returns the current size used for icons.
     */
    int iconSize() const { return myIconSize; }

    void ensureItemVisible( const KFileItem * );

    virtual void setSorting(TQDir::SortSpec sort);

    virtual void readConfig( TDEConfig *, const TQString& group = TQString::null );
    virtual void writeConfig( TDEConfig *, const TQString& group = TQString::null);

    // for KMimeTypeResolver
    void mimeTypeDeterminationFinished();
    void determineIcon( KFileIconViewItem *item );
    TQScrollView *scrollWidget() const { return (TQScrollView*) this; }
    void setAcceptDrops(bool b) 
    {  
      TDEIconView::setAcceptDrops(b); 
      viewport()->setAcceptDrops(b);
    }

public slots:
    /**
     * Starts loading previews for all files shown and shows them. Switches
     * into 'large rows' mode, if that isn't the current mode yet.
     * 
     * @sa setIgnoreMaximumSize
     */
    void showPreviews();

    void zoomIn();
    
    void zoomOut();
    
    /**
     * Reimplemented for performance reasons.
     * @since 3.1
     */
    virtual void arrangeItemsInGrid( bool updated = true );

protected:
    /**
     * Reimplemented to not let TQIconView eat return-key events
     */
    virtual void keyPressEvent( TQKeyEvent * );

    /**
     * Reimplemented to remove an eventual tooltip
     */
    virtual void hideEvent( TQHideEvent * );

    // ### workaround for Qt3 bug (see #35080)
    virtual void showEvent( TQShowEvent * );

    virtual bool eventFilter( TQObject *o, TQEvent *e );

    // DND support
    virtual TQDragObject *dragObject();
    virtual void contentsDragEnterEvent( TQDragEnterEvent *e );
    virtual void contentsDragMoveEvent( TQDragMoveEvent *e );
    virtual void contentsDragLeaveEvent( TQDragLeaveEvent *e );
    virtual void contentsDropEvent( TQDropEvent *ev );

    // KDE4: Make virtual
    bool acceptDrag(TQDropEvent* e ) const;

private slots:
    void selected( TQIconViewItem *item );
    void slotActivate( TQIconViewItem * );
    void highlighted( TQIconViewItem *item );
    void showToolTip( TQIconViewItem *item );
    void removeToolTip();
    void slotActivateMenu( TQIconViewItem *, const TQPoint& );
    void slotSelectionChanged();

    void slotSmallColumns();
    void slotLargeRows();
    void slotPreviewsToggled( bool );

    void slotPreviewResult( TDEIO::Job * );
    void gotPreview( const KFileItem *item, const TQPixmap& pix );
    void slotAutoOpen();

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

private:
    KMimeTypeResolver<KFileIconViewItem,KFileIconView> *m_resolver;

    TQLabel *toolTip;
    int th;
    int myIconSize;

    virtual void insertItem(TQIconViewItem *a, TQIconViewItem *b) { TDEIconView::insertItem(a, b); }
    virtual void setSelectionMode(TQIconView::SelectionMode m) { TDEIconView::setSelectionMode(m); }
    virtual void setSelected(TQIconViewItem *i, bool a, bool b) { TDEIconView::setSelected(i, a, b); }

    bool canPreview( const KFileItem * ) const;
    void stopPreview();

    void updateIcons();

    inline KFileIconViewItem * viewItem( const KFileItem *item ) const {
        if ( item )
            return (KFileIconViewItem *) item->extraData( this );
        return 0L;
    }

    void initItem(KFileIconViewItem *item, const KFileItem *i,
                  bool updateTextAndPixmap );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KFileIconViewPrivate;
    KFileIconViewPrivate *d;
};

#endif // TDEFILESIMPLEVIEW_H
