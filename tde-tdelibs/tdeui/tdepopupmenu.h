/* This file is part of the KDE libraries
   Copyright (C) 2000 Daniel M. Duley <mosfet@kde.org>

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
#ifndef _TDEPOPUP_H
#define _TDEPOPUP_H "$Id$"

#define INCLUDE_MENUITEM_DEF

#include <tqpopupmenu.h>
#include <kpixmapeffect.h>
#include <kpixmap.h>
#include <tdelibs_export.h>
/**
 * @short TDEPopupMenu title widget.
 *
 * Title widget for use in TDEPopupMenu.
 *
 * You usually don't have to create this manually since
 * TDEPopupMenu::insertTitle will do it for you, but it is allowed if
 * you wish to customize it's look.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class TDEUI_EXPORT TDEPopupTitle : public TQWidget
{
    TQ_OBJECT

public:
    /**
     * Constructs a title widget with the user specified gradient, pixmap,
     * and colors.
     */
    TDEPopupTitle(TQWidget *parent=0, const char *name=0);
    /**
     * @deprecated
     * Constructs a title widget with the specified gradient and colors.
     */
    TDEPopupTitle(KPixmapEffect::GradientType gradient, const TQColor &color,
                const TQColor &textColor, TQWidget *parent=0,
                const char *name=0) TDE_DEPRECATED;
    /**
     * @deprecated
     * Constructs a title widget with the specified pixmap and colors.
     */
    TDEPopupTitle(const KPixmap &background, const TQColor &color,
                const TQColor &textColor, TQWidget *parent=0,
                const char *name=0) TDE_DEPRECATED;
    /**
     * Sets the title string and optional icon for the title widget.
     *
     * You will want to call this before inserting into a menu.
     */
    void setTitle(const TQString &text, const TQPixmap *icon=0);
    /**
     * Returns the current title.
     */
    TQString title() const { return titleStr; }
    /**
     * Returns the current icon.
     */
    TQPixmap icon() const { return miniicon; }

    TQSize sizeHint() const;

public slots:
    /// @since 3.1
    void setText( const TQString &text );
    /// @since 3.1
    void setIcon( const TQPixmap &pix );

protected:
    void calcSize();
    void paintEvent(TQPaintEvent *ev);

    // Remove in KDE4
    KPixmapEffect::GradientType grType;
    TQString titleStr;
    // Remove in KDE4
    KPixmap fill;
    TQPixmap miniicon;
    TQColor fgColor, bgColor, grHigh, grLow;
    bool useGradient;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class TDEPopupTitlePrivate;
    TDEPopupTitlePrivate *d;
};

/**
 * @short A menu with title items.
 *
 * TDEPopupMenu is a class for menus with standard title items and keyboard
 * accessibility for popups with many options and/or varying options. It acts
 * identically to TQPopupMenu, with the addition of insertTitle(),
 * changeTitle(), setKeyboardShortcutsEnabled() and
 * setKeyboardShortcutsExecute() methods.
 *
 * The titles support a text string, an icon, plus user defined gradients,
 * colors, and background pixmaps.
 *
 * The keyboard search algorithm is incremental with additional underlining
 * for user feedback.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 * @author Hamish Rodda <rodda@kde.org>
 */
class TDEUI_EXPORT TDEPopupMenu : public TQPopupMenu {
    TQ_OBJECT
    
public:
    /**
     * Constructs a TDEPopupMenu.
     */
    TDEPopupMenu(TQWidget *parent=0, const char *name=0);

    /**
     * Destructs the object
     */
    ~TDEPopupMenu();

    /**
     * Inserts a title item with no icon.
     */
    int insertTitle(const TQString &text, int id=-1, int index=-1);
    /**
     * Inserts a title item with the given icon and title.
     */
    int insertTitle(const TQPixmap &icon, const TQString &text, int id=-1,
                    int index=-1);
    /**
     * Changes the title of the item at the specified id. If a icon was
     * previously set it is cleared.
     */
    void changeTitle(int id, const TQString &text);
    /**
     * Changes the title and icon of the title item at the specified id.
     */
    void changeTitle(int id, const TQPixmap &icon, const TQString &text);
    /**
     * Returns the title of the title item at the specified id. The default
     * id of -1 is for backwards compatibility only, you should always specify
     * the id.
     */
    TQString title(int id=-1) const;
    /**
     * Returns the icon of the title item at the specified id.
     */
    TQPixmap titlePixmap(int id) const;

    /**
     * Enables keyboard navigation by searching for the entered key sequence.
     * Also underlines the currently selected item, providing feedback on the search.
     *
     * Defaults to off.
     *
     * WARNING: calls to text() of currently keyboard-selected items will
     * contain additional ampersand characters.
     *
     * WARNING: though pre-existing keyboard shortcuts will not interfere with the
     * operation of this feature, they may be confusing to the user as the existing
     * shortcuts will not work.
     * @since 3.1
     */
    void setKeyboardShortcutsEnabled(bool enable);

    /**
     * Enables execution of the menu item once it is uniquely specified.
     * Defaults to off.
     * @since 3.1
     */
    void setKeyboardShortcutsExecute(bool enable);

    /**
     * @deprecated
     * Obsolete method provided for backwards compatibility only. Use the
     * normal constructor and insertTitle instead.
     */
    TDEPopupMenu(const TQString &title, TQWidget *parent=0, const char *name=0) TDE_DEPRECATED;

    /**
     * @deprecated
     * Obsolete method provided for backwards compatibility only. Use
     * insertTitle and changeTitle instead.
     */
    void setTitle(const TQString &title) TDE_DEPRECATED;

    /**
     * Returns the context menu associated with this menu
     * @since 3.2
     */
    TQPopupMenu* contextMenu();

    /**
     * Returns the context menu associated with this menu
     * @since 3.2
     */
    const TQPopupMenu* contextMenu() const;

    /**
     * Hides the context menu if shown
     * @since 3.2
     */
    void hideContextMenu();

    /**
     * Returns the TDEPopupMenu associated with the current context menu
     * @since 3.2
     */
    static TDEPopupMenu* contextMenuFocus();

    /**
     * returns the ID of the menuitem associated with the current context menu
     * @since 3.2
     */
    static int contextMenuFocusItem();

    /**
     * Reimplemented for internal purposes
     * @since 3.4
     */
    virtual void activateItemAt(int index);
    /**
     * Return the state of the mouse button and keyboard modifiers
     * when the last menuitem was activated.
     * @since 3.4
     */
    TQt::ButtonState state() const;

signals:
    /**
     * connect to this signal to be notified when a context menu is about to be shown
     * @param menu The menu that the context menu is about to be shown for
     * @param menuItem The menu item that the context menu is currently on
     * @param ctxMenu The context menu itself
     * @since 3.2
     */
    void aboutToShowContextMenu(TDEPopupMenu* menu, int menuItem, TQPopupMenu* ctxMenu);

protected:
    virtual void closeEvent(TQCloseEvent *);
    virtual void keyPressEvent(TQKeyEvent* e);
    /// @since 3.4
    virtual void mouseReleaseEvent(TQMouseEvent* e);
    virtual void mousePressEvent(TQMouseEvent* e);
    virtual bool focusNextPrevChild( bool next );
    virtual void contextMenuEvent(TQContextMenuEvent *e);
    virtual void hideEvent(TQHideEvent*);

    virtual void virtual_hook( int id, void* data );

protected slots:
    /// @since 3.1
    TQString underlineText(const TQString& text, uint length);
    /// @since 3.1
    void resetKeyboardVars(bool noMatches = false);
    void itemHighlighted(int whichItem);
    void showCtxMenu(TQPoint pos);
    void ctxMenuHiding();
    void ctxMenuHideShowingMenu();

private:
    class TDEPopupMenuPrivate;
    TDEPopupMenuPrivate *d;
};

#endif
