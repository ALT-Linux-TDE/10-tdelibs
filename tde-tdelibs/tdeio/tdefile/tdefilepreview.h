/*
    This file is part of the KDE libraries
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

#ifndef _TDEFILEPREVIEW_H
#define _TDEFILEPREVIEW_H

#include <tqsplitter.h>
#include <tqwidget.h>
#include <tqstring.h>

#include <kurl.h>
#include <tdefileitem.h>
#include <tdefileiconview.h>
#include <tdefiledetailview.h>
#include <tdefile.h>

/*!
 * This KFileView is an empbedded preview for some file types.
 */
class TDEIO_EXPORT KFilePreview : public TQSplitter, public KFileView
{
    TQ_OBJECT

public:
    KFilePreview(TQWidget *parent, const char *name);
    KFilePreview(KFileView *view, TQWidget *parent, const char *name);
    virtual ~KFilePreview();

    virtual TQWidget *widget() { return this; }
    virtual void clearView();

    /**
     * Delets the current view and sets the view to the given @p view.
     * The view is reparented to have this as parent, if necessary.
     */
    void setFileView(KFileView *view);

    /**
     * @returns the current fileview
     */
    KFileView* fileView() const { return left; }

    virtual void updateView( bool );
    virtual void updateView(const KFileItem*);
    virtual void removeItem(const KFileItem*);
    virtual void listingCompleted();

    virtual void setSelectionMode( KFile::SelectionMode sm );

    virtual void setSelected(const KFileItem *, bool);
    virtual bool isSelected( const KFileItem * ) const;
    virtual void clearSelection();
    virtual void selectAll();
    virtual void invertSelection();

    virtual void insertItem(KFileItem *);
    virtual void clear();

    virtual void setCurrentItem( const KFileItem * );
    virtual KFileItem * currentFileItem() const;
    virtual KFileItem * firstFileItem() const;
    virtual KFileItem * nextItem( const KFileItem * ) const;
    virtual KFileItem * prevItem( const KFileItem * ) const;

    virtual void setSorting( TQDir::SortSpec sort );

    virtual void readConfig( TDEConfig *, const TQString& group = TQString::null );
    virtual void writeConfig( TDEConfig *, const TQString& group = TQString::null);

    /**
     * This overrides KFileView::actionCollection() by returning
     * the actionCollection() of the KFileView (member left) it contains.
     * This means that KFilePreview will never create a TDEActionCollection
     * object of its own.
     */
    virtual TDEActionCollection * actionCollection() const;

    void ensureItemVisible(const KFileItem *);

    void setPreviewWidget(const TQWidget *w, const KURL &u);

protected slots:
    virtual void slotHighlighted( const KFileItem * );

signals:
    void showPreview(const KURL &);
    void clearPreview();

private:
    void init( KFileView *view );

    KFileView *left;
    TQWidget *preview;
    TQString viewname;

protected:
    /** \internal */
    virtual void virtual_hook( int id, void* data );
private:
    class KFilePreviewPrivate;
    KFilePreviewPrivate *d;
};
#endif
