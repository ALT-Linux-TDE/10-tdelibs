/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

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

#ifndef _TDEACCELACTION_H
#define _TDEACCELACTION_H

#include <tqmap.h>
#include <tqptrvector.h>
#include <tqstring.h>
#include <tqvaluevector.h>

#include <tdeshortcut.h>

class TDEAccelBase;

class TQObject;
class TDEConfig;
class TDEConfigBase;

/**
 * @internal
 * A TDEAccelAction prepresents an action that can be executed using 
 * an accelerator key. Each TDEAccelAction has a name, a label, a 
 * "What's this" string and a TDEShortcut. The user can configure and 
 * enable/disable them using KKeyDialog. 
 *
 * \code
 *	1) TDEAccelAction = "Run Command"
 *		Default3 = "Alt+F2"
 *		Default4 = "Meta+Enter;Alt+F2"
 *		1) TDEShortcut = "Meta+Enter"
 *			1) KKeySequence = "Meta+Enter"
 *				1) KKey = "Meta+Enter"
 *					1) Meta+Enter
 *					2) Meta+Keypad_Enter
 *		2) TDEShortcut = "Alt+F2"
 *			1) KKeySequence = "Alt+F2"
 *				1) Alt+F2
 *	2) TDEAccelAction = "Something"
 *		Default3 = ""
 *		Default4 = ""
 *		1) TDEShortcut = "Meta+X,Asterisk"
 *			1) KKeySequence = "Meta+X,Asterisk"
 *				1) KKey = "Meta+X"
 *					1) Meta+X
 *				2) KKey = "Asterisk"
 *					1) Shift+8 (English layout)
 *					2) Keypad_Asterisk
 * \endcode
 * @short An accelerator action
 * @see TDEAccel
 * @see TDEGlobalAccel
 * @see KKeyChooser
 * @see KKeyDialog
 */
class TDECORE_EXPORT TDEAccelAction
{
 public:
        /**
	 * Creates an empty TDEAccelAction.
	 * @see clear()
	 */
	TDEAccelAction();

	/**
	 * Copy constructor.
	 */
	TDEAccelAction( const TDEAccelAction& );

	/**
	 * Creates a new TDEAccelAction.
	 * @param sName the name of the accelerator
	 * @param sLabel the label of the accelerator (i18n!)
	 * @param sWhatsThis the What's This text (18n!)
	 * @param cutDef3 the default shortcut for 3 modifier systems
	 * @param cutDef4 the default shortcut for 4 modifier systems
	 * @param pObjSlot the receiver of a signal when the key has been 
	 *                 pressed
	 * @param psMethodSlot the slot to connect for key presses. Receives
	 *                     an int, as set by setID(), as only argument
	 * @param bConfigurable if true the user can configure the shortcut
	 * @param bEnabled true if the accelerator should be enabled
	 */
	TDEAccelAction( const TQString& sName, const TQString& sLabel, const TQString& sWhatsThis,
			const TDEShortcut& cutDef3, const TDEShortcut& cutDef4,
			const TQObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled );
	~TDEAccelAction();

	/**
	 * Clears the accelerator.
	 */
	void clear();

	/**
	 * Re-initialized the TDEAccelAction.
	 * @param sName the name of the accelerator
	 * @param sLabel the label of the accelerator (i18n!)
	 * @param sWhatsThis the What's This text (18n!)
	 * @param cutDef3 the default shortcut for 3 modifier systems
	 * @param cutDef4 the default shortcut for 4 modifier systems
	 * @param pObjSlot the receiver of a signal when the key has been 
	 *                 pressed
	 * @param psMethodSlot the slot to connect for key presses. Receives
	 *                     an int, as set by setID(), as only argument
	 * @param bConfigurable if true the user can configure the shortcut
	 * @param bEnabled true if the accelerator should be enabled
	 * @return true if successful, false otherwise
	 */
	bool init( const TQString& sName, const TQString& sLabel, const TQString& sWhatsThis,
			const TDEShortcut& cutDef3, const TDEShortcut& cutDef4,
			const TQObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled );

	/**
	 * Copies this TDEAccelAction.
	 */
	TDEAccelAction& operator=( const TDEAccelAction& );

	/**
	 * Returns the name of the accelerator action.
	 * @return the name of the accelerator action, can be null if not 
	 *         set
	 */
	const TQString& name() const                { return m_sName; }

	/**
	 * Returns the label of the accelerator action.
	 * @return the label of the accelerator action, can be null if
	 *         not set
	 */
	const TQString& label() const               { return m_sLabel; }

	/**
	 * Returns the What's This text of the accelerator action.
	 * @return the What's This text of the accelerator action, can be
	 *         null if not set
	 */
	const TQString& whatsThis() const           { return m_sWhatsThis; }
	
	/**
	 * The shortcut that is actually used (may be used configured).
	 * @return the shortcut of the TDEAccelAction, can be null if not set
	 * @see shortcutDefault()
	 */
	const TDEShortcut& shortcut() const          { return m_cut; }

	/**
	 * The default shortcut for this system.
	 * @return the default shortcut on this system, can be null if not set
	 * @see shortcut()
	 * @see shortcutDefault3()
	 * @see shortcutDefault4()
	 */
	const TDEShortcut& shortcutDefault() const;

	/**
	 * The default shortcut for 3 modifier systems.
	 * @return the default shortcut for 3 modifier systems, can be null
	 *           if not set
	 * @see shortcutDefault()
	 * @see shortcutDefault4()
	 * @see useFourModifierKeys()
	 */
	const TDEShortcut& shortcutDefault3() const  { return m_cutDefault3; }

	/**
	 * The default shortcut for 4 modifier systems.
	 * @return the default shortcut for 4 modifier systems, can be null 
	 *         if not set
	 * @see shortcutDefault()
	 * @see shortcutDefault3()
	 * @see useFourModifierKeys()
	 */
	const TDEShortcut& shortcutDefault4() const  { return m_cutDefault4; }

	/**
	 * Returns the receiver of signals.
	 * @return the receiver of signals (can be 0 if not set)
	 */
	const TQObject* objSlotPtr() const          { return m_pObjSlot; }

	/**
	 * Returns the slot for the signal.
	 * @return the slot for the signal
	 */
	const char* methodSlotPtr() const          { return m_psMethodSlot; }

	/**
	 * Checks whether the user can configure the action.
	 * @return true if configurable, false otherwise
	 */
	bool isConfigurable() const                { return m_bConfigurable; }

	/**
	 * Checks whether the action is enabled.
	 * @return true if enabled, false otherwise
	 */
	bool isEnabled() const                     { return m_bEnabled; }

	/**
	 * Sets the name of the accelerator action.
	 * @param name the new name
	 */
	void setName( const TQString& name );

	/**
	 * Sets the user-readable label of the accelerator action.
	 * @param label the new label (i18n!)
	 */
	void setLabel( const TQString& label );

	/**
	 * Sets the What's This text for the accelerator action.
	 * @param whatsThis the new What's This text (i18n!)
	 */
	void setWhatsThis( const TQString& whatsThis );

	/**
	 * Sets the new shortcut of the accelerator action.
	 * @param rgCuts the shortcut to set
	 * @return true if successful, false otherwise
	 */
	bool setShortcut( const TDEShortcut& rgCuts );

	/**
	 * Sets the slot of the accelerator action.
	 * @param pObjSlot the receiver object of the signal
	 * @param psMethodSlot the slot for the signal
	 */
	void setSlot( const TQObject* pObjSlot, const char* psMethodSlot );

	/**
	 * Enables or disabled configuring the action.
	 * @param configurable true to enable configurability, false to disable
	 */
	void setConfigurable( bool configurable );

	/**
	 * Enables or disabled the action.
	 * @param enable true to enable the action, false to disable
	 */
	void setEnabled( bool enable );

	/**
	 * Retrieves the id set using setID.
	 * @return the id of the accelerator action
	 */
	int getID() const   { return m_nIDAccel; }

	/**
	 * Allows you to set an id that will be used as the action 
	 * signal's argument.
	 *
	 * @param n the new id
	 * @see getID()
	 */
	void setID( int n ) { m_nIDAccel = n; }

	/**
	 * Checkes whether the action is connected (emits signals).
	 * @return true if connected, false otherwise
	 */
	bool isConnected() const;

	/**
	 * Sets a key sequence of the action's shortcut.
	 * @param i the position of the sequence
	 * @param keySeq the new new sequence
	 * @return true if successful, false otherwise
	 * @see TDEShortcut::setSeq()
	 */
	bool setKeySequence( uint i, const KKeySequence &keySeq );
	
	/**
	 * Clears the action's shortcut. It will not contain any sequences after
	 * calling this method.
	 * @see TDEShortcut::clear()
	 */
	void clearShortcut();
	
	/**
	 * Checks whether the action's shortcut contains the given key sequence.
	 * @param keySeq the key sequence to check
	 * @return true if the shortcut contains the given sequence
	 * @see TDEShortcut::contains()
	 */
	bool contains( const KKeySequence &keySeq );

	/**
	 * Returns the string representation of the action's shortcut.
	 * @return the string representation of the action's shortcut.
	 * @see TDEShortcut::toString()
	 */
	TQString toString() const;

	/**
	 * @internal
	 */
	TQString toStringInternal() const;

	/**
	 * Returns true if four modifier keys will be used.
	 * @return true if four modifier keys will be used.
	 */
	static bool useFourModifierKeys();

	/**
	 * Selects 3 or 4 modifier default shortcuts.
	 * @param use true to use 4 modifier shortcuts, false to use
	 *            3 modifier shortcuts
	 */
	static void useFourModifierKeys( bool use );

 protected:
	TQString m_sName /**< Name of accel. @sa setName() */,
	        m_sLabel /**< Label of accel. User-visible. */,
	        m_sWhatsThis /**< WhatsThis help for accel. User-visible. */;
	TDEShortcut m_cut /**< Shortcut actually assigned. */;
	TDEShortcut m_cutDefault3 /**< Default shortcut in 3-modifier layout */, 
		  m_cutDefault4 /**< Default shortcur in 4-modifier layout */;
	const TQObject* m_pObjSlot /**< Object we will send signals to. */;
	const char* m_psMethodSlot /**< Slot we send signals to, in m_pObjSlot */;
	bool m_bConfigurable /**< Can this accel be configured by the user? */,
	     m_bEnabled /**< Is this accel enabled? */;
	int m_nIDAccel /**< Id of this accel, from the list of IDs */;
	uint m_nConnections /**< Number of connections to this accel. */ ;

	/** @internal Increment the number of connections to this accel. */
	void incConnections();
	/** @internal Decrement the number of connections to this accel (bouded by zero). */
	void decConnections();

 private:
	static int g_bUseFourModifierKeys;
	class TDEAccelActionPrivate* d;

	friend class TDEAccelActions;
	friend class TDEAccelBase;
};

//---------------------------------------------------------------------
// TDEAccelActions
//---------------------------------------------------------------------

/**
 * @internal
 * This class represents a collection of TDEAccelAction objects.
 *
 * @short A collection of accelerator actions
 * @see TDEAccelAction
 */
class TDECORE_EXPORT TDEAccelActions
{
 public:
       /**
	* Creates a new, empty TDEAccelActions object.
	*/
	TDEAccelActions();

	/**
	 * Copy constructor (deep copy).
	 */
	TDEAccelActions( const TDEAccelActions& );
	virtual ~TDEAccelActions();

	/**
	 * Removes all items from this collection.
	 */
	void clear();

	/**
	 * Initializes this object with the given actions.
	 * It will make a deep copy of all actions.
	 * @param actions the actions to copy
	 * @return true if successful, false otherwise
	 */
	bool init( const TDEAccelActions &actions );

	/**
	 * Loads the actions from the given configuration file.
	 *
	 * @param config the configuration file to load from
	 * @param sGroup the group in the configuration file
	 * @return true if successful, false otherwise
	 */
	bool init( TDEConfigBase& config, const TQString& sGroup );

	/**
	 * Updates the shortcuts of all actions in this object
	 * with the shortcuts from the given object.
	 * @param shortcuts the collection that contains the new
	 *        shortcuts
	 */
	void updateShortcuts( TDEAccelActions &shortcuts );

	/**
	 * Retrieves the index of the action with the given name.
	 * @param sAction the action to search
	 * @return the index of the action, or -1 if not found
	 */
	int actionIndex( const TQString& sAction ) const;

	/**
	 * Returns the action with the given @p index.
	 * @param index the index of an action. You must not
	 *         use an index that is too high.
	 * @return the TDEAccelAction with the given index
	 * @see count()
	 */
	TDEAccelAction* actionPtr( uint index );

	/**
	 * Returns the action with the given @p index.
	 * @param index the index of an action. You must not
	 *         use an index that is too high.
	 * @return the TDEAccelAction with the given index
	 * @see count()
	 */
	const TDEAccelAction* actionPtr( uint index ) const;

	/**
	 * Returns the action with the given name.
	 * @param sAction the name of the action to search
	 * @return the TDEAccelAction with the given name, or 0
	 *          if not found
	 */
	TDEAccelAction* actionPtr( const TQString& sAction );

	/**
	 * Returns the action with the given name.
	 * @param sAction the name of the action to search
	 * @return the TDEAccelAction with the given name, or 0
	 *          if not found
	 */
	const TDEAccelAction* actionPtr( const TQString& sAction ) const;

	/**
	 * Returns the action with the given key sequence.
	 * @param cut the sequence to search for
	 * @return the TDEAccelAction with the given sequence, or 0
	 *          if not found
	 */
	TDEAccelAction* actionPtr( KKeySequence cut );

	/**
	 * Returns the action with the given @p index.
	 * @param index the index of an action. You must not
	 *         use an index that is too high.
	 * @return the TDEAccelAction with the given index
	 * @see actionPtr()
	 * @see count()
	 */
	TDEAccelAction& operator []( uint index );

	/**
	 * Returns the action with the given @p index.
	 * @param index the index of an action. You must not
	 *         use an index that is too high.
	 * @return the TDEAccelAction with the given index
	 * @see actionPtr()
	 * @see count()
	 */
	const TDEAccelAction& operator []( uint index ) const;

	/**
	 * Inserts an action into the collection.
	 * @param sAction        the name of the accelerator
	 * @param sLabel         the label of the accelerator (i18n!)
	 * @param sWhatsThis     the What's This text (18n!)
	 * @param rgCutDefaults3 the default shortcut for 3 modifier systems
	 * @param rgCutDefaults4 the default shortcut for 4 modifier systems
	 * @param pObjSlot       the receiver of a signal when the key has been 
	 *                       pressed
	 * @param psMethodSlot   the slot to connect for key presses. Receives
	 *                       an int, as set by setID(), as only argument
	 * @param bConfigurable  if true the user can configure the shortcut
	 * @param bEnabled       if true the accelerator should be enabled
	 * @return the new action
	 */
	TDEAccelAction* insert( const TQString& sAction, const TQString& sLabel, const TQString& sWhatsThis,
			const TDEShortcut& rgCutDefaults3, const TDEShortcut& rgCutDefaults4,
			const TQObject* pObjSlot = 0, const char* psMethodSlot = 0,
			bool bConfigurable = true, bool bEnabled = true );

	/**
	 * Inserts an action into the collection.
	 * @param sName the name of the accelerator
	 * @param sLabel the label of the accelerator (i18n!)
	 * @return the new action
	 */
	TDEAccelAction* insert( const TQString& sName, const TQString& sLabel );

	/**
	 * Removes the given action.
	 * @param sAction the name of the action.
	 * @return true if successful, false otherwise
	 */
	bool remove( const TQString& sAction );

	/**
	 * Loads the actions from the given configuration file.
	 *
	 * @param sConfigGroup the group in the configuration file
	 * @param pConfig the configuration file to load from
	 * @return true if successful, false otherwise
	 */
	bool readActions( const TQString& sConfigGroup = "Shortcuts", TDEConfigBase* pConfig = 0 );

	/**
	 * Writes the actions to the given configuration file.
	 *
	 * @param sConfigGroup the group in the configuration file
	 * @param pConfig the configuration file to save to
	 * @param bWriteAll true to write all actions
	 * @param bGlobal true to write to the global configuration file
	 * @return true if successful, false otherwise
	 */
	bool writeActions( const TQString& sConfigGroup = "Shortcuts", TDEConfigBase* pConfig = 0,
			bool bWriteAll = false, bool bGlobal = false ) const;

	/**
	 * Emit a keycodeChanged signal.
	 */
	void emitKeycodeChanged();

	/**
	 * Returns the number of actions in the collection.
	 * @return the number of actions
	 */
	uint count() const;

 protected:
	/** Base object that proxies signals from us. */
	TDEAccelBase* m_pTDEAccelBase;
	/** Array of actions we're hanging on to. */
	TDEAccelAction** m_prgActions;
	uint m_nSizeAllocated /**< Allocated size of the array. */, 
	     m_nSize /**< Amount in use. */ ;

	/** 
	 * Resize the list to the given number @p new_size of entries. 
	 * @todo Can you make it smaller?
	 * @todo Implementation seems to break m_nSize.
	 */
	void resize( uint new_size );
	/** Add a action to this collection. @todo Document ownership. */
	void insertPtr( TDEAccelAction* );

 private:
	class TDEAccelActionsPrivate* d;

	TDEAccelActions( TDEAccelBase* );
	void initPrivate( TDEAccelBase* );
	TDEAccelActions& operator =( TDEAccelActions& );

	friend class TDEAccelBase;
};

#endif // _TDEACCELACTION_H
