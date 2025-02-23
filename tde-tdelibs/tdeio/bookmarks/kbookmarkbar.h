/* This file is part of the KDE project
   Copyright (C) 1999 Kurt Granroth <granroth@kde.org>

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
#ifndef KBOOKMARKBAR_H
#define KBOOKMARKBAR_H

#include <tqobject.h>
#include <tqguardedptr.h>
#include <tqptrlist.h>
#include <kbookmark.h>
#include <tdeaction.h>

class TDEToolBar;
class KBookmarkMenu;
class KBookmarkOwner;
class TDEActionCollection;
class TDEAction;
class TQPopupMenu;

/**
 * This class provides a bookmark toolbar.  Using this class is nearly
 * identical to using KBookmarkMenu so follow the directions
 * there.
 */
class TDEIO_EXPORT KBookmarkBar : public TQObject
{
    TQ_OBJECT
    
    friend class RMB;
public:
    /**
     * Fills a bookmark toolbar
     *
     * @param manager the bookmark manager
     * @param owner implementation of the KBookmarkOwner interface (callbacks)
     * @param toolBar toolbar to fill
     * 
     * The TDEActionCollection pointer argument is now obsolete.
     *
     * @param parent the parent widget for the bookmark toolbar
     * @param name the internal name for the bookmark toolbar
     */
    KBookmarkBar( KBookmarkManager* manager,
                  KBookmarkOwner *owner, TDEToolBar *toolBar,
                  TDEActionCollection *,
                  TQObject *parent = 0L, const char *name = 0L);

    virtual ~KBookmarkBar();

    /**
     * @since 3.2
     */
    bool isReadOnly() const;

    /**
     * @since 3.2
     */
    void setReadOnly(bool);

    /**
     * @since 3.2
     */
    TQString parentAddress();

signals:
    /**
     * @since 3.2
     */
    void aboutToShowContextMenu( const KBookmark &, TQPopupMenu * );
    /**
     * @since 3.4
     */
    void openBookmark( const TQString& url, TQt::ButtonState state );

public slots:
    void clear();

    void slotBookmarksChanged( const TQString & );
    void slotBookmarkSelected();

    /**
     * @since 3.4
     */
    void slotBookmarkSelected( TDEAction::ActivationReason reason, TQt::ButtonState state );
    
    /// @since 3.2
    void slotRMBActionRemove( int );
    /// @since 3.2
    void slotRMBActionInsert( int );
    /// @since 3.2
    void slotRMBActionCopyLocation( int );
    /// @since 3.2
    void slotRMBActionEditAt( int );
    /// @since 3.2
    void slotRMBActionProperties( int );

protected:
    void fillBookmarkBar( KBookmarkGroup & parent );
    virtual bool eventFilter( TQObject *o, TQEvent *e );

private:
    KBookmarkGroup getToolbar();

    KBookmarkOwner *m_pOwner;
    TQGuardedPtr<TDEToolBar> m_toolBar;
    TDEActionCollection *m_actionCollection;
    KBookmarkManager *m_pManager;
    TQPtrList<KBookmarkMenu> m_lstSubMenus;

private:
    class KBookmarkBarPrivate* dptr() const;
};

#endif // KBOOKMARKBAR_H
