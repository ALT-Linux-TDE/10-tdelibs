/* This file is part of the KDE libraries
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>

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

#ifndef _TDEACCELBASE_H
#define _TDEACCELBASE_H

#include <tqmap.h>
#include <tqptrvector.h>
#include <tqstring.h>
#include <tqvaluevector.h>
#include <tqvaluelist.h>

#include "tdeaccelaction.h"
#include "kkeyserver.h"

class TQPopupMenu;
class TQWidget;

//----------------------------------------------------

/**
 * @internal
 * Handle keyboard accelerators.
 *
 * Allow an user to configure
 * key bindings through application configuration files or through the
 * KKeyChooser GUI.
 *
 * A TDEAccel contains a list of accelerator items. Each accelerator item
 * consists of an action name and a keyboard code combined with modifiers
 * (Shift, Ctrl and Alt.)
 *
 * For example, "Ctrl+P" could be a shortcut for printing a document. The key
 * codes are listed in ckey.h. "Print" could be the action name for printing.
 * The action name identifies the key binding in configuration files and the
 * KKeyChooser GUI.
 *
 * When pressed, an accelerator key calls the slot to which it has been
 * connected. Accelerator items can be connected so that a key will activate
 * two different slots.
 *
 * A TDEAccel object handles key events sent to its parent widget and to all
 * children of this parent widget.
 *
 * Key binding reconfiguration during run time can be prevented by specifying
 * that an accelerator item is not configurable when it is inserted. A special
 * group of non-configurable key bindings are known as the
 * standard accelerators.
 *
 * The standard accelerators appear repeatedly in applications for
 * standard document actions such as printing and saving. Convenience methods are
 * available to insert and connect these accelerators which are configurable on
 * a desktop-wide basis.
 *
 * It is possible for a user to choose to have no key associated with
 * an action.
 *
 * The translated first argument for insertItem() is used only
 * in the configuration dialog.
 *\code
 * TDEAccel *a = new TDEAccel( myWindow );
 * // Insert an action "Scroll Up" which is associated with the "Up" key:
 * a->insertItem( i18n("Scroll Up"), "Scroll Up", "Up" );
 * // Insert an action "Scroll Down" which is not associated with any key:
 * a->insertItem( i18n("Scroll Down"), "Scroll Down", 0);
 * a->connectItem( "Scroll up", myWindow, TQ_SLOT( scrollUp() ) );
 * // a->insertStdItem( TDEStdAccel::Print ); //not necessary, since it
 *	// is done automatially with the
 *	// connect below!
 * a->connectItem(TDEStdAccel::Print, myWindow, TQ_SLOT( printDoc() ) );
 *
 * a->readSettings();
 *\endcode
 *
 * If a shortcut has a menu entry as well, you could insert them like
 * this. The example is again the TDEStdAccel::Print from above.
 *
 * \code
 * int id;
 * id = popup->insertItem("&Print",this, TQ_SLOT(printDoc()));
 * a->changeMenuAccel(popup, id, TDEStdAccel::Print );
 * \endcode
 *
 * If you want a somewhat "exotic" name for your standard print action, like
 *   id = popup->insertItem(i18n("Print &Document"),this, TQ_SLOT(printDoc()));
 * it might be a good idea to insert the standard action before as
 *          a->insertStdItem( TDEStdAccel::Print, i18n("Print Document") )
 * as well, so that the user can easily find the corresponding function.
 *
 * This technique works for other actions as well.  Your "scroll up" function
 * in a menu could be done with
 *
 * \code
 *    id = popup->insertItem(i18n"Scroll &up",this, TQ_SLOT(scrollUp()));
 *    a->changeMenuAccel(popup, id, "Scroll Up" );
 * \endcode
 *
 * Please keep the order right:  First insert all functions in the
 * acceleratior, then call a -> readSettings() and @em then build your
 * menu structure.
 *
 * @short Configurable key binding support.
 */

class TDECORE_EXPORT TDEAccelBase
{
 public:
	/** Initialization mode of the TDEAccelBase, used in constructor. */
	enum Init { QT_KEYS = 0x00, NATIVE_KEYS = 0x01 };

	/** Enum for kinds of signals which may be emitted. */
	enum Signal { KEYCODE_CHANGED };

	/** Constructor. @p fInitCode should be a bitwise OR of
	*   values from the Init enum.
	*/
	TDEAccelBase( int fInitCode );
	virtual ~TDEAccelBase();

	/** Returns number of actions in this handler. */
	uint actionCount() const;
	/** Returns a list of all the actions in this handler. */
	TDEAccelActions& actions();
	/** Returns whether this accelerator handler is enabled or not. */
	bool isEnabled() const;

	/** Returns a pointer to the TDEAccelAction named @p sAction. */
	TDEAccelAction* actionPtr( const TQString& sAction );
	/** Const version of the above. */
	const TDEAccelAction* actionPtr( const TQString& sAction ) const;
	/** Returns a pointer to the TDEAccelAction associated with
	*   the key @p key. This function takes into account the
	*   key mapping defined in the constructor.
	*
	*   May return 0 if no (or more than one)
	*   action is associated with the key.
	*/
	TDEAccelAction* actionPtr( const KKey& key );
	/** Basically the same as above, except a KKeyServer::Key
	*   already has a key mapping defined (either NATIVE_KEYS or not).
	*/
	TDEAccelAction* actionPtr( const KKeyServer::Key& key );

	/** Returns the name of the configuration group these
	*   accelerators are stored in. The default is "Shortcuts".
	*/
	const TQString& configGroup() const { return m_sConfigGroup; }
	/** Set the group (in the configuration file) for storing
	*   accelerators.
	*/
	void setConfigGroup( const TQString& group );
	void setConfigGlobal( bool global );
	/** Enables or disables the accelerator.
	 * @param bEnabled determines whether the accelerator should be enabled or
	 * disabled.
	 */
	virtual void setEnabled( bool bEnabled ) = 0;
	/** Returns whether autoupdate is enabled for these accelerators. */
	bool getAutoUpdate() { return m_bAutoUpdate; }
	/** Enables (or disables) autoupdate for these accelerators.
	*   @return the value of autoupdate before the call.
	*/
	bool setAutoUpdate( bool bAuto );

// Procedures for manipulating Actions.
	//void clearActions();

	TDEAccelAction* insert( const TQString& sName, const TQString& sDesc );
	TDEAccelAction* insert(
	                 const TQString& sAction, const TQString& sDesc, const TQString& sHelp,
	                 const TDEShortcut& rgCutDefaults3, const TDEShortcut& rgCutDefaults4,
	                 const TQObject* pObjSlot, const char* psMethodSlot,
			 bool bConfigurable = true, bool bEnabled = true );
	bool remove( const TQString& sAction );
	bool setActionSlot( const TQString& sAction, const TQObject* pObjSlot, const char* psMethodSlot );

	bool updateConnections();

	bool setShortcut( const TQString& sAction, const TDEShortcut& cut );

// Modify individual Action sub-items
	bool setActionEnabled( const TQString& sAction, bool bEnable );

	/**
	 * Read all key associations from @p config, or (if @p config
	 * is zero) from the application's configuration file
	 * TDEGlobal::config().
	 *
	 * The group in which the configuration is stored can be
	 * set with setConfigGroup().
	 */
	void readSettings( TDEConfigBase* pConfig = 0 );

	/**
	 * Write the current configurable associations to @p config,
         * or (if @p config is zero) to the application's
	 * configuration file.
	 */
	void writeSettings( TDEConfigBase* pConfig = 0 ) const;

	TQPopupMenu* createPopupMenu( TQWidget* pParent, const KKeySequence& );

 // Protected methods
 protected:
	void slotRemoveAction( TDEAccelAction* );

	struct X;

	/** Constructs a list of keys to be connected, sorted highest priority first.
	 * @param rgKeys constructed list of keys
	 */
	void createKeyList( TQValueVector<struct X>& rgKeys );
	bool insertConnection( TDEAccelAction* );
	bool removeConnection( TDEAccelAction* );

	/** Emits a signal.
	 * @param signal signal to be emitted
	 */
	virtual bool emitSignal( Signal signal ) = 0;
	/** Defines a key which activates the accelerator and executes the action
	 * @param action action to be executed when key is pressed
	 * @param key key which causes the action to be executed
	 */
	virtual bool connectKey( TDEAccelAction& action, const KKeyServer::Key& key ) = 0;
	/** Defines a key which activates the accelerator
	 * @param key key which causes the action to be executed
	 */
	virtual bool connectKey( const KKeyServer::Key& key) = 0;
	/** Removes the key from accelerator so it no longer executes the action
	 */
	virtual bool disconnectKey( TDEAccelAction&, const KKeyServer::Key& ) = 0;
	/** Removes the key from accelerator
	 */
	virtual bool disconnectKey( const KKeyServer::Key& ) = 0;

 protected:
        virtual bool isEnabledInternal() const;
	struct ActionInfo
	{
		TDEAccelAction* pAction;
		uint iSeq, iVariation;
		//ActionInfo* pInfoNext; // nil if only one action uses this key.

		ActionInfo() { pAction = 0; iSeq = 0xffff; iVariation = 0xffff; }
		ActionInfo( TDEAccelAction* _pAction, uint _iSeq, uint _iVariation )
			{ pAction = _pAction; iSeq = _iSeq; iVariation = _iVariation; }
	};
	typedef TQMap<KKeyServer::Key, ActionInfo> KKeyToActionMap;

	TDEAccelActions m_rgActions;
	KKeyToActionMap m_mapKeyToAction;
	TQValueList<TDEAccelAction*> m_rgActionsNonUnique;
	bool m_bNativeKeys; // Use native key codes instead of Qt codes
	bool m_bEnabled;
	bool m_bConfigIsGlobal;
	TQString m_sConfigGroup;
	bool m_bAutoUpdate;
	TDEAccelAction* mtemp_pActionRemoving;

 private:
	TDEAccelBase& operator =( const TDEAccelBase& );

	friend class TDEAccelActions;
};

#endif // _TDEACCELBASE_H
