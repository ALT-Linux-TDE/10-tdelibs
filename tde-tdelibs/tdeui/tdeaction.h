/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>

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
//$Id$

#ifndef __tdeaction_h__
#define __tdeaction_h__

#include <tqkeysequence.h>
#include <tqobject.h>
#include <tqvaluelist.h>
#include <tqguardedptr.h>
#include <kguiitem.h>
#include <tdeshortcut.h>
#include <kstdaction.h>
#include <kicontheme.h>

class TQMenuBar;
class TQPopupMenu;
class TQComboBox;
class TQPoint;
class TQIconSet;
class TQString;
class TDEToolBar;

class TDEAccel;
class TDEAccelActions;
class TDEConfig;
class TDEConfigBase;
class KURL;
class TDEInstance;
class TDEToolBar;
class TDEActionCollection;
class TDEPopupMenu;
class TDEMainWindow;

/**
 * @short Class to encapsulate user-driven action or event
 *
 * The TDEAction class (and derived and super classes) provides a way to
 * easily encapsulate a "real" user-selected action or event in your
 * program.
 *
 * For instance, a user may want to @p paste the contents of
 * the clipboard or @p scroll @p down a document or @p quit the
 * application.  These are all @p actions -- events that the
 * user causes to happen.  The TDEAction class allows the developer to
 * deal with these actions in an easy and intuitive manner.
 *
 * Specifically, the TDEAction class encapsulated the various attributes
 * to an event/action.  For instance, an action might have an icon
 * that goes along with it (a clipboard for a "paste" action or
 * scissors for a "cut" action).  The action might have some text to
 * describe the action.  It will certainly have a method or function
 * that actually @p executes the action!  All these attributes
 * are contained within the TDEAction object.
 *
 * The advantage of dealing with Actions is that you can manipulate
 * the Action without regard to the GUI representation of it.  For
 * instance, in the "normal" way of dealing with actions like "cut",
 * you would manually insert a item for Cut into a menu and a button
 * into a toolbar.  If you want to disable the cut action for a moment
 * (maybe nothing is selected), you would have to hunt down the pointer
 * to the menu item and the toolbar button and disable both
 * individually.  Setting the menu item and toolbar item up uses very
 * similar code - but has to be done twice!
 *
 * With the Action concept, you simply "plug" the Action into whatever
 * GUI element you want.  The TDEAction class will then take care of
 * correctly defining the menu item (with icons, accelerators, text,
 * etc) or toolbar button.. or whatever.  From then on, if you
 * manipulate the Action at all, the effect will propogate through all
 * GUI representations of it.  Back to the "cut" example: if you want
 * to disable the Cut Action, you would simply do
 * 'cutAction->setEnabled(false)' and the menuitem and button would
 * instantly be disabled!
 *
 * This is the biggest advantage to the Action concept -- there is a
 * one-to-one relationship between the "real" action and @p all
 * GUI representations of it.
 *
 * TDEAction emits the activated() signal if the user activated the
 * corresponding GUI element ( menu item, toolbar button, etc. )
 *
 * If you are in the situation of wanting to map the activated()
 * signal of multiple action objects to one slot, with a special
 * argument bound to each action, then you might consider using
 * TQSignalMapper . A tiny example:
 *
 * \code
 * TQSignalMapper *desktopNumberMapper = new TQSignalMapper( this );
 * connect( desktopNumberMapper, TQ_SIGNAL( mapped( int ) ),
 *          this, TQ_SLOT( moveWindowToDesktop( int ) ) );
 *
 * for ( uint i = 0; i < numberOfDesktops; ++i ) {
 *     TDEAction *desktopAction = new TDEAction( i18n( "Move Window to Desktop %i" ).arg( i ), ... );
 *     connect( desktopAction, TQ_SIGNAL( activated() ), desktopNumberMapper, TQ_SLOT( map() ) );
 *     desktopNumberMapper->setMapping( desktopAction, i );
 * }
 * \endcode
 *
 * <b>General Usage:</b>\n
 *
 * The steps to using actions are roughly as follows
 *
 * @li Decide which attributes you want to associate with a given
 *     action (icons, text, keyboard shortcut, etc)
 * @li Create the action using TDEAction (or derived or super class).
 * @li "Plug" the Action into whatever GUI element you want.  Typically,
 *      this will be a menu or toolbar.
 *
 * <b>Detailed Example:</b>\n
 *
 * Here is an example of enabling a "New [document]" action
 * \code
 * TDEAction *newAct = new TDEAction(i18n("&New"), "document-new",
 *                               TDEStdAccel::shortcut(TDEStdAccel::New),
 *                               this, TQ_SLOT(fileNew()),
 *                               actionCollection(), "new");
 * \endcode
 * This line creates our action.  It says that wherever this action is
 * displayed, it will use "&New" as the text, the standard icon, and
 * the standard shortcut.  It further says that whenever this action
 * is invoked, it will use the fileNew() slot to execute it.
 *
 * \code
 * TQPopupMenu *file = new TQPopupMenu;
 * newAct->plug(file);
 * \endcode
 * That just inserted the action into the File menu.  The point is, it's not
 * important in which menu it is: all manipulation of the item is
 * done through the newAct object.
 *
 * \code
 * newAct->plug(toolBar());
 * \endcode
 * And this inserted the Action into the main toolbar as a button.
 *
 * That's it!
 *
 * If you want to disable that action sometime later, you can do so
 * with
 * \code
 * newAct->setEnabled(false)
 * \endcode
 * and both the menuitem in File and the toolbar button will instantly
 * be disabled.
 *
 * Do not delete a TDEAction object without unplugging it from all its
 * containers. The simplest way to do that is to use the unplugAll()
 * as in the following example:
 * \code
 * newAct->unplugAll();
 * delete newAct;
 * \endcode
 * Normally you will not need to do this as TDEActionCollection manages
 * everything for you.
 *
 * Note: if you are using a "standard" action like "new", "paste",
 * "quit", or any other action described in the KDE UI Standards,
 * please use the methods in the KStdAction class rather than
 * defining your own.
 *
 * <b>Usage Within the XML Framework:</b>\n
 *
 * If you are using TDEAction within the context of the XML menu and
 * toolbar building framework, then there are a few tiny changes.  The
 * first is that you must insert your new action into an action
 * collection.  The action collection (a TDEActionCollection) is,
 * logically enough, a central collection of all of the actions
 * defined in your application.  The XML UI framework code in KXMLGUI
 * classes needs access to this collection in order to build up the
 * GUI (it's how the builder code knows which actions are valid and
 * which aren't).
 *
 * Also, if you use the XML builder framework, then you do not ever
 * have to plug your actions into containers manually.  The framework
 * does that for you.
 *
 * @see KStdAction
 */
class TDEUI_EXPORT TDEAction : public TQObject
{
  friend class TDEActionCollection;
  TQ_OBJECT
  TQ_PROPERTY( int containerCount READ containerCount )
  TQ_PROPERTY( TQString plainText READ plainText )
  TQ_PROPERTY( TQString text READ text WRITE setText )
  TQ_PROPERTY( TQString shortcut READ shortcutText WRITE setShortcutText )
  TQ_PROPERTY( bool enabled READ isEnabled WRITE setEnabled )
  TQ_PROPERTY( TQString group READ group WRITE setGroup )
  TQ_PROPERTY( TQString whatsThis READ whatsThis WRITE setWhatsThis )
  TQ_PROPERTY( TQString toolTip READ toolTip WRITE setToolTip )
  TQ_PROPERTY( TQString icon READ icon WRITE setIcon )
public:
    /**
     * Constructs an action with text, potential keyboard
     * shortcut, and a slot to call when this action is invoked by
     * the user.
     *
     * If you do not want or have a keyboard shortcut,
     * set the @p cut param to 0.
     *
     * This is the most common TDEAction used when you do not have a
     * corresponding icon (note that it won't appear in the current version
     * of the "Edit ToolBar" dialog, because an action needs an icon to be
     * plugged in a toolbar...).
     *
     * @param text The text that will be displayed.
     * @param cut The corresponding keyboard shortcut.
     * @param receiver The slot's parent.
     * @param slot The slot to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    TDEAction( const TQString& text, const TDEShortcut& cut,
             const TQObject* receiver, const char* slot,
             TDEActionCollection* parent, const char* name );

    /**
     * Constructs an action with text, icon, potential keyboard
     * shortcut, and a slot to call when this action is invoked by
     * the user.
     *
     * If you do not want or have a keyboard shortcut, set the
     * @p cut param to 0.
     *
     * This is the other common TDEAction used.  Use it when you
     * @p do have a corresponding icon.
     *
     * @param text The text that will be displayed.
     * @param pix The icon to display.
     * @param cut The corresponding keyboard shortcut.
     * @param receiver The slot's parent.
     * @param slot The slot to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    TDEAction( const TQString& text, const TQIconSet& pix, const TDEShortcut& cut,
             const TQObject* receiver, const char* slot,
             TDEActionCollection* parent, const char* name );

    /**
     * Constructs an action with text, icon, potential keyboard
     * shortcut, and a slot to call when this action is invoked by
     * the user.  The icon is loaded on demand later based on where it
     * is plugged in.
     *
     * If you do not want or have a keyboard shortcut, set the
     * @p cut param to 0.
     *
     * This is the other common TDEAction used.  Use it when you
     * @p do have a corresponding icon.
     *
     * @param text The text that will be displayed.
     * @param pix The icon to display.
     * @param cut The corresponding keyboard shortcut (shortcut).
     * @param receiver The slot's parent.
     * @param slot The slot to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    TDEAction( const TQString& text, const TQString& pix, const TDEShortcut& cut,
             const TQObject* receiver, const char* slot,
             TDEActionCollection* parent, const char* name );

    /**
     * The same as the above constructor, but with a KGuiItem providing
     * the text and icon.
     *
     * @param item The KGuiItem with the label and (optional) icon.
     * @param cut The corresponding keyboard shortcut (shortcut).
     * @param receiver The slot's parent.
     * @param slot The slot to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    TDEAction( const KGuiItem& item, const TDEShortcut& cut,
             const TQObject* receiver, const char* slot,
             TDEActionCollection* parent, const char* name );

	/**
	 * @obsolete
	 */
	TDEAction( const TQString& text, const TDEShortcut& cut = TDEShortcut(), TQObject* parent = 0, const char* name = 0 );
	/**
	 * @obsolete
	 */
	TDEAction( const TQString& text, const TDEShortcut& cut,
		const TQObject* receiver, const char* slot, TQObject* parent, const char* name = 0 );
	/**
	 * @obsolete
	 */
	TDEAction( const TQString& text, const TQIconSet& pix, const TDEShortcut& cut = TDEShortcut(),
		TQObject* parent = 0, const char* name = 0 );
	/**
	 * @obsolete
	 */
	TDEAction( const TQString& text, const TQString& pix, const TDEShortcut& cut = TDEShortcut(),
		TQObject* parent = 0, const char* name = 0 );
	/**
	 * @obsolete
	 */
	TDEAction( const TQString& text, const TQIconSet& pix, const TDEShortcut& cut,
		const TQObject* receiver, const char* slot, TQObject* parent, const char* name = 0 );
	/**
	 * @obsolete
	 */
	TDEAction( const TQString& text, const TQString& pix, const TDEShortcut& cut,
		const TQObject* receiver, const char* slot, TQObject* parent,
		const char* name = 0 );
	/**
	 * @obsolete
	 */
	TDEAction( TQObject* parent = 0, const char* name = 0 );

    /**
     * Standard destructor
     */
    virtual ~TDEAction();

    /**
     * "Plug" or insert this action into a given widget.
     *
     * This will
     * typically be a menu or a toolbar.  From this point on, you will
     * never need to directly manipulate the item in the menu or
     * toolbar.  You do all enabling/disabling/manipulation directly
     * with your TDEAction object.
     *
     * @param widget The GUI element to display this action
     * @param index The position into which the action is plugged. If
     * this is negative, the action is inserted at the end.
     */
    virtual int plug( TQWidget *widget, int index = -1 );

    /**
     * @deprecated.  Shouldn't be used.  No substitute available.
     *
     * "Plug" or insert this action into a given TDEAccel.
     *
     * @param accel The TDEAccel collection which holds this accel
     * @param configurable If the shortcut is configurable via
     * the TDEAccel configuration dialog (this is somehow deprecated since
     * there is now a TDEAction key configuration dialog).
     */
    virtual void plugAccel(TDEAccel *accel, bool configurable = true) TDE_DEPRECATED;

    /**
     * "Unplug" or remove this action from a given widget.
     *
     * This will typically be a menu or a toolbar.  This is rarely
     * used in "normal" application.  Typically, it would be used if
     * your application has several views or modes, each with a
     * completely different menu structure.  If you simply want to
     * disable an action for a given period, use setEnabled()
     * instead.
     *
     * @param w Remove the action from this GUI element.
     */
    virtual void unplug( TQWidget *w );

    /**
     * @deprecated.  Complement method to plugAccel().
     * Disconnect this action from the TDEAccel.
     */
    virtual void unplugAccel() TDE_DEPRECATED;

    /**
     * returns whether the action is plugged into any container widget or not.
     * @since 3.1
     */
    virtual bool isPlugged() const;

    /**
     * returns whether the action is plugged into the given container
     */
    bool isPlugged( const TQWidget *container ) const;

    /**
     * returns whether the action is plugged into the given container with the given, container specific, id (often
     * menu or toolbar id ) .
     */
    virtual bool isPlugged( const TQWidget *container, int id ) const;

    /**
     * returns whether the action is plugged into the given container with the given, container specific, representative
     * container widget item.
     */
    virtual bool isPlugged( const TQWidget *container, const TQWidget *_representative ) const;

    TQWidget* container( int index ) const;
    int itemId( int index ) const;
    TQWidget* representative( int index ) const;
    int containerCount() const;
    /// @since 3.1
    uint tdeaccelCount() const;

    virtual bool hasIcon() const;
#ifndef KDE_NO_COMPAT
    bool hasIconSet() const { return hasIcon(); }
#endif
    virtual TQString plainText() const;

    /**
     * Get the text associated with this action.
     */
    virtual TQString text() const;

    /**
     * Get the keyboard shortcut associated with this action.
     */
    virtual const TDEShortcut& shortcut() const;
    /**
     * Get the default shortcut for this action.
     */
    virtual const TDEShortcut& shortcutDefault() const;

    // These two methods are for TQ_PROPERTY
    TQString shortcutText() const;
    void setShortcutText( const TQString& );

    /**
     * Returns true if this action is enabled.
     */
    virtual bool isEnabled() const;

    /**
     * Returns true if this action's shortcut is configurable.
     */
    virtual bool isShortcutConfigurable() const;

    virtual TQString group() const;

    /**
     * Get the What's this text for the action.
     */
    virtual TQString whatsThis() const;

    /**
     * Get the tooltip text for the action.
     */
    virtual TQString toolTip() const;

    /**
     * Get the TQIconSet from which the icons used to display this action will
     * be chosen.
     *
     * In KDE4 set group default to TDEIcon::Small while removing the other
     * iconSet() function.
     */
    virtual TQIconSet iconSet( TDEIcon::Group group, int size=0 ) const;
    /**
     * Remove in KDE4
     */
    TQIconSet iconSet() const { return iconSet( TDEIcon::Small ); }

    virtual TQString icon() const;

    TDEActionCollection *parentCollection() const;

    /**
     * @internal
     * Generate a toolbar button id. Made public for reimplementations.
     */
    static int getToolButtonID();


    void unplugAll();

    /**
    * @since 3.4
    */
    enum ActivationReason { UnknownActivation, EmulatedActivation, AccelActivation, PopupMenuActivation, ToolBarActivation };

public slots:
    /**
     * Sets the text associated with this action. The text is used for menu
     * and toolbar labels etc.
     */
    virtual void setText(const TQString &text);

    /**
     * Sets the keyboard shortcut associated with this action.
     */
    virtual bool setShortcut( const TDEShortcut& );

    virtual void setGroup( const TQString& );

    /**
     * Sets the What's this text for the action. This text will be displayed when
     * a widget that has been created by plugging this action into a container
     * is clicked on in What's this mode.
     *
     * The What's this text can include QML markup as well as raw text.
     */
    virtual void setWhatsThis( const TQString& text );

    /**
     * Sets the tooltip text for the action.
     * This will be used as a tooltip for a toolbar button, as a
     * statusbar help-text for a menu item, and it also appears
     * in the toolbar editor, to describe the action.
     *
     * For the tooltip to show up on the statusbar you will need to connect
     * a couple of the actionclass signals to the toolbar.
     * The easiest way of doing this is in your main window class, when you create
     * a statusbar.  See the TDEActionCollection class for more details.
     *
     * @see TDEActionCollection
     *
     */
    virtual void setToolTip( const TQString& );

    /**
     * Sets the TQIconSet from which the icons used to display this action will
     * be chosen.
     */
    virtual void setIconSet( const TQIconSet &iconSet );

    virtual void setIcon( const TQString& icon );

    /**
     * Enables or disables this action. All uses of this action (eg. in menus
     * or toolbars) will be updated to reflect the state of the action.
     */
    virtual void setEnabled(bool enable);

    /**
     * Calls setEnabled( !disable ).
     * @since 3.5
     */    
    void setDisabled(bool disable) { return setEnabled(!disable); }

    /**
     * Indicate whether the user may configure the action's shortcut.
     */
    virtual void setShortcutConfigurable( bool );

    /**
     * Emulate user's interaction programmatically, by activating the action.
     * The implementation simply emits activated().
     */
    virtual void activate();

protected slots:
    virtual void slotDestroyed();
    virtual void slotKeycodeChanged();
    virtual void slotActivated();
    /// @since 3.4
    void slotPopupActivated(); // KDE4: make virtual
    /// @since 3.4
    void slotButtonClicked( int, TQt::ButtonState state ); // KDE4: make virtual

protected:
    TDEToolBar* toolBar( int index ) const;
    TQPopupMenu* popupMenu( int index ) const;
    void removeContainer( int index );
    int findContainer( const TQWidget* widget ) const;
    int findContainer( int id ) const;
    void plugMainWindowAccel( TQWidget *w );

    void addContainer( TQWidget* parent, int id );
    void addContainer( TQWidget* parent, TQWidget* representative );

    virtual void updateShortcut( int i );
    virtual void updateShortcut( TQPopupMenu* menu, int id );
    virtual void updateGroup( int id );
    virtual void updateText(int i );
    virtual void updateEnabled(int i);
    virtual void updateIconSet(int i);
    virtual void updateIcon( int i);
    virtual void updateToolTip( int id );
    virtual void updateWhatsThis( int i );

    TDEActionCollection *m_parentCollection;
    TQString whatsThisWithIcon() const;
    /**
     * Return the underlying KGuiItem
     * @since 3.3
     */
    const KGuiItem& guiItem() const;

signals:
    /**
     * Emitted when this action is activated
     */
    void activated();
    /**
     * This signal allows to know the reason why an action was activated:
     * whether it was due to a toolbar button, popupmenu, keyboard accel, or programmatically.
     * In the first two cases, it also allows to know which mouse button was
     * used (Left or Middle), and whether keyboard modifiers were pressed (e.g. CTRL).
     *
     * Note that this signal is emitted before the normal activated() signal.
     * Yes, BOTH signals are always emitted, so that connecting to activated() still works.
     * Applications which care about reason and state can either ignore the activated()
     * signal for a given action and react to this one instead, or store the
     * reason and state until the activated() signal is emitted.
     *
     * @since 3.4
     */
    void activated( TDEAction::ActivationReason reason, TQt::ButtonState state );
    void enabled( bool );

private:
    void initPrivate( const TQString& text, const TDEShortcut& cut,
                  const TQObject* receiver, const char* slot );
    TDEAccel* tdeaccelCurrent();
    bool initShortcut( const TDEShortcut& );
    void plugShortcut();
    bool updateTDEAccelShortcut( TDEAccel* tdeaccel );
    void insertTDEAccel( TDEAccel* );
    /** @internal To be used exclusively by TDEActionCollection::removeWidget(). */
    void removeTDEAccel( TDEAccel* );

#ifndef KDE_NO_COMPAT
public:
    /**
     * @deprecated.  Use shortcut().
     * Get the keyboard accelerator associated with this action.
     */
    int accel() const TDE_DEPRECATED;

    TQString statusText() const
        { return toolTip(); }

    /**
     * @deprecated.  Use setShortcut().
     * Sets the keyboard accelerator associated with this action.
     */
    void setAccel( int key ) TDE_DEPRECATED;

    /**
     * @deprecated. Use setToolTip instead (they do the same thing now).
     */
    void setStatusText( const TQString &text )
         { setToolTip( text ); }

    /**
     * @deprecated. for backwards compatibility. Use itemId()
     */
    int menuId( int i ) { return itemId( i ); }
#endif // !KDE_NO_COMPAT

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class TDEActionPrivate;
    TDEActionPrivate* const d;
};

#include <tdeactioncollection.h>
#include <tdeactionclasses.h>

#endif
