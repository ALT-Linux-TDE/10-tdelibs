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

#ifndef __tdeactioncollection_h__
#define __tdeactioncollection_h__

#include <tdeaction.h>

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
class KXMLGUIClient;

typedef TQValueList<TDEAction *> TDEActionPtrList;

/**
 * A managed set of TDEAction objects.
 * 
 * If you set the tooltips on TDEActions and want the tooltip to show in statusbar
 * (recommended) then you will need to connect a couple of the actionclass signals
 * to the toolbar.
 * The easiest way of doing this is in your TDEMainWindow subclass, where you create
 * a statusbar, do:
 *
 * \code
 * actionCollection()->setHighlightingEnabled(true);  
 * connect(actionCollection(), TQ_SIGNAL( actionStatusText( const TQString & ) ),
 *           statusBar(), TQ_SLOT( message( const TQString & ) ) );
 * connect(actionCollection(), TQ_SIGNAL( clearStatusText() ),
 *           statusBar(), TQ_SLOT( clear() ) );
 * \endcode
 */
class TDEUI_EXPORT TDEActionCollection : public TQObject
{
  friend class TDEAction;
  friend class KXMLGUIClient;

  TQ_OBJECT

public:
  TDEActionCollection( TQWidget *parent, const char *name = 0, TDEInstance *instance = 0 );
  /**
   * Use this constructor if you want the collection's actions to restrict
   * their accelerator keys to @p watch rather than the @p parent.  If
   * you don't require shortcuts, you can pass a null to the @p watch parameter.
   */
  TDEActionCollection( TQWidget *watch, TQObject* parent, const char *name = 0, TDEInstance *instance = 0 );
#ifndef KDE_NO_COMPAT
  TDEActionCollection( const TDEActionCollection &copy );
#endif
  virtual ~TDEActionCollection();

  /**
   * This sets the widget to which the keyboard shortcuts should be attached.
   * You only need to call this if a null pointer was passed in the constructor.
   */
  virtual void setWidget( TQWidget *widget );

  /**
   * This indicates whether new actions which are created in this collection
   * should have their keyboard shortcuts automatically connected on
   * construction.  Set to 'false' if you will be loading XML-based settings.
   * This is automatically done by KParts.  The default is 'true'.
   * @see isAutoConnectShortcuts()
   */
  void setAutoConnectShortcuts( bool );

  /**
   * This indicates whether new actions which are created in this collection
   * have their keyboard shortcuts automatically connected on
   * construction.
   * @see setAutoConnectShortcuts()
   */
  bool isAutoConnectShortcuts();

  /**
   * This sets the default shortcut scope for new actions created in this
   * collection.  The default is ScopeUnspecified.  Ideally the default
   * would have been ScopeWidget, but that would cause some backwards
   * compatibility problems.
   */
  //void setDefaultScope( TDEAction::Scope );

  /**
   * Doc/View model.  This lets you add the action collection of a document
   * to a view's action collection.
   */
  bool addDocCollection( TDEActionCollection* pDoc );

  /** Returns the number of widgets which this collection is associated with. */
  //uint widgetCount() const;

  /**
   * Returns true if the collection has its own TDEAccel object.  This will be
   * the case if it was constructed with a valid widget ptr or if setWidget()
   * was called.
   */
  //bool ownsTDEAccel() const;

  /** @deprecated  Deprecated because of ambiguous name.  Use tdeaccel() */
  virtual TDEAccel* accel() TDE_DEPRECATED;
  /** @deprecated  Deprecated because of ambiguous name.  Use tdeaccel() */
  virtual const TDEAccel* accel() const TDE_DEPRECATED;

  /** Returns the TDEAccel object of the most recently set widget. */
  TDEAccel* tdeaccel();
  /** Returns the TDEAccel object of the most recently set widget. Const version for convenience. */
  const TDEAccel* tdeaccel() const;

  /** @internal, for TDEAction::tdeaccelCurrent() */
  TDEAccel* builderTDEAccel() const;
  /** Returns the TDEAccel object associated with widget #. */
  //TDEAccel* widgetTDEAccel( uint i );
  //const TDEAccel* widgetTDEAccel( uint i ) const;

  /** Returns the number of actions in the collection */
  virtual uint count() const;
  bool isEmpty() const { return (count() == 0); }
  /**
   * Return the TDEAction* at position "index" in the action collection.
   * @see count()
   */
  virtual TDEAction* action( int index ) const;
  /**
   * Find an action (optionally, of a given subclass of TDEAction) in the action collection.
   * @param name Name of the TDEAction.
   * @param classname Name of the TDEAction subclass.
   * @return A pointer to the first TDEAction in the collection which matches the parameters or
   * null if nothing matches.
   */
  virtual TDEAction* action( const char* name, const char* classname = 0 ) const;

  /** Returns a list of all the groups of all the TDEActions in this action collection.
   * @see TDEAction::group()
   * @see TDEAction::setGroup()
   */
  virtual TQStringList groups() const;
  /**
   * Returns the list of actions in a particular group managed by this action collection.
   * @param group The name of the group.
   */
  virtual TDEActionPtrList actions( const TQString& group ) const;
  /** Returns the list of actions managed by this action collection. */
  virtual TDEActionPtrList actions() const;

  /**
   * Used for reading shortcut configuration from a non-XML rc file.
   */
  bool readShortcutSettings( const TQString& sConfigGroup = TQString::null, TDEConfigBase* pConfig = 0 );
  /**
   * Used for writing shortcut configuration to a non-XML rc file.
   */
  bool writeShortcutSettings( const TQString& sConfigGroup = TQString::null, TDEConfigBase* pConfig = 0 ) const;

  void setInstance( TDEInstance *instance );
  /** The instance with which this class is associated. */
  TDEInstance *instance() const;

  /**
   * @deprecated
   */
  void setXMLFile( const TQString& );
  /**
   * @deprecated
   */
  const TQString& xmlFile() const;

  //TODO FOR KDE4 make this default true
  /**
   * Enable highlighting notification for specific TDEActions.
   * This is false by default, so, by default, the highlighting
   * signals will not be emitted.
   * 
   * @see connectHighlight()
   * @see disconnectHighlight()
   * @see actionHighlighted()
   * @see actionHighlighted()
   * @see highlightingEnabled()
   */
  void setHighlightingEnabled( bool enable );
  /**
   * Return whether highlighting notifications are enabled.
   * @see connectHighlight()
   * @see disconnectHighlight()
   * @see actionHighlighted()
   * @see setHighlightingEnabled()
   * @see actionHighlighted()
   */
  bool highlightingEnabled() const;

  /**
   * Call this function if you want to receive a signal whenever a TDEAction is highlighted in a menu or a toolbar.
   * This is only needed if you do not add this action to this container.
   * You will generally not need to call this function.
   * 
   * @param container A container in which the TDEAction is plugged (must inherit TQPopupMenu or TDEToolBar)
   * @param action The action you are interested in
   * @see disconnectHighlight()
   * @see actionHighlighted()
   * @see setHighlightingEnabled()
   * @see highlightingEnabled()
   * @see actionHighlighted()
   */
  void connectHighlight( TQWidget *container, TDEAction *action );
  /**
   * Disconnect highlight notifications for a particular pair of contianer and action.
   * This is only needed if you do not add this action to this container.
   * You will generally not need to call this function.
   * 
   * @param container A container in which the TDEAction is plugged (must inherit TQPopupMenu or TDEToolBar)
   * @param action The action you are interested in
   * @see connectHighlight()
   * @see actionHighlighted()
   * @see setHighlightingEnabled()
   * @see highlightingEnabled()
   * @see actionHighlighted()
   */
  void disconnectHighlight( TQWidget *container, TDEAction *action );

  /**
   * The parent KXMLGUIClient, return 0L if not available.
   */
  const KXMLGUIClient *parentGUIClient() const;

signals:
  void inserted( TDEAction* );
  void removed( TDEAction* );

  /** Emitted when @p action is highlighted.
   *  This is only emitted if you have setHighlightingEnabled()
   * @see connectHighlight()
   * @see disconnectHighlight()
   * @see actionHighlighted()
   * @see setHighlightingEnabled()
   * @see highlightingEnabled()
   */
  void actionHighlighted( TDEAction *action );
  /** Emitted when @p action is highlighed or loses highlighting.
   *  This is only emitted if you have setHighlightingEnabled()
   * @see connectHighlight()
   * @see disconnectHighlight()
   * @see actionHighlighted()
   * @see setHighlightingEnabled()
   * @see highlightingEnabled()
   */
  void actionHighlighted( TDEAction *action, bool highlight );
  /** Emitted when an action is highlighted, with text
   *  being the tooltip for the action.
   *  This is only emitted if you have setHighlightingEnabled()
   *  
   *  This is useful to connect to KStatusBar::message().  See
   *  this class overview for more information.
   *  
   * @see setHighlightingEnabled()
   */
  void actionStatusText( const TQString &text );
  /** Emitted when an action loses highlighting.
   *  This is only emitted if you have setHighlightingEnabled()
   *  
   * @see setHighlightingEnabled()
   */
  void clearStatusText();

private:
  /**
   * @internal Only to be called by KXMLGUIFactory::addClient().
   * When actions are being connected, TDEAction needs to know what
   * widget it should connect widget-scope actions to, and what
   * main window it should connect
   */
  void beginXMLPlug( TQWidget *widget );
  void endXMLPlug();
  /** @internal.  Only to be called by KXMLGUIFactory::removeClient() */
  void prepareXMLUnplug();
  void unplugShortcuts( TDEAccel* tdeaccel );

  void _clear();
  void _insert( TDEAction* );
  void _remove( TDEAction* );
  TDEAction* _take( TDEAction* );

private slots:
   void slotMenuItemHighlighted( int id );
   void slotToolBarButtonHighlighted( int id, bool highlight );
   void slotMenuAboutToHide();
   void slotDestroyed();

private:
   TDEAction *findAction( TQWidget *container, int id );

#ifndef KDE_NO_COMPAT
public:
  TDEActionCollection( TQObject *parent, const char *name = 0, TDEInstance *instance = 0 );
#endif

public:
  /**
   * Add an action to the collection.
   * Generally you don't have to call this. The action inserts itself automatically
   * into its parent collection. This can be useful however for a short-lived
   * collection (e.g. for a popupmenu, where the signals from the collection are needed too).
   * (don't forget that in the simple case, a list of actions should be a simple TDEActionPtrList).
   * If you manually insert actions into a 2nd collection, don't forget to take them out
   * again before destroying the collection.
   * @param action The TDEAction to add.
   */
  void insert( TDEAction* action);

  /**
   * Removes an action from the collection and deletes it.
   * Since the TDEAction destructor removes the action from the collection, you generally
   * don't have to call this.
   * @param action The TDEAction to remove.
   */
  void remove( TDEAction* action );

  /**
   * Removes an action from the collection.
   * Since the TDEAction destructor removes the action from the collection, you generally
   * don't have to call this.
   * @return NULL if not found else returns action.
   * @param action the TDEAction to remove.
   */
  TDEAction* take( TDEAction* action );

#ifndef KDE_NO_COMPAT
  TDEActionCollection operator+ ( const TDEActionCollection& ) const;
  TDEActionCollection& operator= ( const TDEActionCollection& );
  TDEActionCollection& operator+= ( const TDEActionCollection& );
#endif // !KDE_NO_COMPAT

  // KDE4: clear() doesn't need to be a slot
public slots:
  /**
   * Clears the entire actionCollection, deleting all actions.
   * @see remove
   */
  void clear();

protected:
    virtual void virtual_hook( int id, void* data );
private:
    TDEActionCollection( const char* name, const KXMLGUIClient* parent );
    class TDEActionCollectionPrivate;
    TDEActionCollectionPrivate *d;
};

#endif
