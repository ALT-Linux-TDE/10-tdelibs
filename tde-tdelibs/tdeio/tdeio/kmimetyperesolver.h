/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2000 Rik Hemsley <rik@kde.org>
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef __kmimetyperesolver_h
#define __kmimetyperesolver_h

#include <tqscrollview.h>
#include <tqptrlist.h>
#include <tqtimer.h>
#include <kdebug.h>

/**
 * @internal
 * A baseclass for KMimeTypeResolver, with the interface,
 * KMimeTypeResolverHelper uses.
 */
class TDEIO_EXPORT KMimeTypeResolverBase
{
public:
    virtual void slotViewportAdjusted() = 0;
    virtual void slotProcessMimeIcons() = 0;
protected:
    virtual void virtual_hook( int, void* ) {}
};

/**
 * @internal
 * This class is used by KMimeTypeResolver, because it can't be a TQObject
 * itself. So an object of this class is used to handle signals, slots etc.
 * and forwards them to the KMimeTypeResolver instance.
 */
class TDEIO_EXPORT KMimeTypeResolverHelper : public TQObject
{
    TQ_OBJECT

public:
    KMimeTypeResolverHelper( KMimeTypeResolverBase *resolver,
                             TQScrollView *view )
        : m_resolver( resolver ),
          m_timer( new TQTimer( this ) )
    {
        connect( m_timer, TQ_SIGNAL( timeout() ), TQ_SLOT( slotProcessMimeIcons() ));

        connect( view->horizontalScrollBar(), TQ_SIGNAL( sliderMoved(int) ),
                 TQ_SLOT( slotAdjust() ) );
        connect( view->verticalScrollBar(), TQ_SIGNAL( sliderMoved(int) ),
                 TQ_SLOT( slotAdjust() ) );

        view->viewport()->installEventFilter( this );
    }

    void start( int delay, bool singleShot )
    {
        m_timer->start( delay, singleShot );
    }

protected:
    virtual bool eventFilter( TQObject *o, TQEvent *e )
    {
        bool ret = TQObject::eventFilter( o, e );

        if ( e->type() == TQEvent::Resize )
            m_resolver->slotViewportAdjusted();

        return ret;
    }

private slots:
    void slotProcessMimeIcons()
    {
        m_resolver->slotProcessMimeIcons();
    }

    void slotAdjust()
    {
        m_resolver->slotViewportAdjusted();
    }

private:
    KMimeTypeResolverBase *m_resolver;
    TQTimer *m_timer;
};

/**
 * This class implements the "delayed-mimetype-determination" feature,
 * for konqueror's directory views (and KFileDialog's :).
 *
 * It determines the mimetypes of the icons in the background, but giving
 * preferrence to the visible icons.
 *
 * It is implemented as a template, so that it can work with both QPtrListViewItem
 * and TQIconViewItem, without requiring hacks such as void * or TQPtrDict lookups.
 *
 * Here's what the parent must implement :
 * @li void mimeTypeDeterminationFinished();
 * @li TQScrollView * scrollWidget();
 * @li void determineIcon( IconItem * item ), which should call
 * @li KFileItem::determineMimeType on the fileItem, and update the icon, etc.
*/
template<class IconItem, class Parent>
class KMimeTypeResolver : public KMimeTypeResolverBase // if only this could be a TQObject....
{
public:
  /**
   * Creates a new KMimeTypeResolver with the given parent.
   * @param parent the parent's resolver
   */
    KMimeTypeResolver( Parent * parent )
        : m_parent(parent),
          m_helper( new KMimeTypeResolverHelper(this, parent->scrollWidget())),
          m_delayNonVisibleIcons(10)
    {}

    virtual ~KMimeTypeResolver() {
        delete m_helper;
    }

    /**
     * Start the mimetype-determination. Call this when the listing is completed.
     * @param delayNonVisibleIcons the delay to use between icons not on screen.
     * Usually 10, but should be set to 0 when the image preview feature is
     * activated, because image preview can only start once we know the mimetypes
     */
    void start( uint delayNonVisibleIcons = 10 )
    {
        m_helper->start( 0, true /* single shot */ );
        m_delayNonVisibleIcons = delayNonVisibleIcons;
    }

    /**
     * The list of items to process. The view is free to
     * clear it, insert new items into it, remove items, etc.
     * @return the list of items to process
     */
    TQPtrList<IconItem> m_lstPendingMimeIconItems;

    /**
     * "Connected" to the viewportAdjusted signal of the scrollview
     */
    virtual void slotViewportAdjusted();

    /**
     * "Connected" to the timer
     */
    virtual void slotProcessMimeIcons();

private:
    /**
     * Find a visible icon and determine its mimetype.
     * KonqDirPart will call this method repeatedly until it returns 0L
     * (no more visible icon to process).
     * @return the file item that was just processed.
     */
    IconItem * findVisibleIcon();

    Parent * m_parent;
    KMimeTypeResolverHelper *m_helper;
    uint m_delayNonVisibleIcons;
};

// The main slot
template<class IconItem, class Parent>
inline void KMimeTypeResolver<IconItem, Parent>::slotProcessMimeIcons()
{
    //kdDebug(1203) << "KMimeTypeResolver::slotProcessMimeIcons() "
    //              << m_lstPendingMimeIconItems.count() << endl;
    IconItem * item = 0L;
    int nextDelay = 0;

    if ( m_lstPendingMimeIconItems.count() > 0 )
    {
        // We only find mimetypes for icons that are visible. When more
        // of our viewport is exposed, we'll get a signal and then get
        // the mimetypes for the newly visible icons. (Rikkus)
        item = findVisibleIcon();
    }

    // No more visible items.
    if (0 == item)
    {
        // Do the unvisible ones, then, but with a bigger delay, if so configured
        if ( m_lstPendingMimeIconItems.count() > 0 )
        {
            item = m_lstPendingMimeIconItems.first();
            nextDelay = m_delayNonVisibleIcons;
        }
        else
        {
            m_parent->mimeTypeDeterminationFinished();
            return;
        }
    }

    m_parent->determineIcon(item);
    m_lstPendingMimeIconItems.remove(item);
    m_helper->start( nextDelay, true /* single shot */ );
}

template<class IconItem, class Parent>
inline void KMimeTypeResolver<IconItem, Parent>::slotViewportAdjusted()
{
    if (m_lstPendingMimeIconItems.isEmpty()) return;
    IconItem * item = findVisibleIcon();
    if (item)
    {
        m_parent->determineIcon( item );
        m_lstPendingMimeIconItems.remove(item);
        m_helper->start( 0, true /* single shot */ );
    }
}

template<class IconItem, class Parent>
inline IconItem * KMimeTypeResolver<IconItem, Parent>::findVisibleIcon()
{
    // Find an icon that's visible and whose mimetype we don't know.

    TQPtrListIterator<IconItem> it(m_lstPendingMimeIconItems);
    if ( m_lstPendingMimeIconItems.count()<20) // for few items, it's faster to not bother
        return m_lstPendingMimeIconItems.first();

    TQScrollView * view = m_parent->scrollWidget();
    TQRect visibleContentsRect
        (
            view->viewportToContents(TQPoint(0, 0)),
            view->viewportToContents
            (
                TQPoint(view->visibleWidth(), view->visibleHeight())
                )
            );

    for (; it.current(); ++it)
        if (visibleContentsRect.intersects(it.current()->rect()))
            return it.current();

    return 0L;
}

#endif
