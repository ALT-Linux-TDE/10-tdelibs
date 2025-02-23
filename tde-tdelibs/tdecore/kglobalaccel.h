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

#ifndef _KGLOBALACCEL_H_
#define _KGLOBALACCEL_H_

#include <tqobject.h>
#include <tdeshortcut.h>

class TQPopupMenu;
class TQWidget;
class TDEAccelAction;
class TDEAccelActions;
class TDEConfigBase;

class TDEGlobalAccelPrivate;

/**
* TDEGlobalAccel allows you to have global accelerators that are independent of
* the focused window. Unlike TDEAccel it does not matter which window is 
* currently active.
*
* @see TDEAccel
* @see TDEAccelShortcutList
* @see KKeyChooser
* @see KKeyDialog
* @short Configurable global shortcut support
*/
class TDECORE_EXPORT TDEGlobalAccel : public TQObject
{
	TQ_OBJECT
 public:
	/**
	 * Creates a new TDEGlobalAccel object with the given pParent and
	 * psName.
	 * @param pParent the parent of the TQObject
	 * @param psName the name of the TQObject
	 */
	TDEGlobalAccel( TQObject* pParent, const char* psName = 0 );
	virtual ~TDEGlobalAccel();

	/**
	 * Checks whether the accelerators are enabled.
	 * @return true if the TDEGlobalAccel is enabled
	 */
	bool isEnabled();
	
	/**
	 * Enables or disables the TDEGlobalAccel
	 * @param bEnabled true if the TDEGlobalAccel should be enabled, false if it
	 *  should be disabled.
	 */
	void setEnabled( bool bEnabled );

	/**
	 * Create an accelerator action.
	 *
	 * Usage:
	 *\code
	 * insert( "Do Something", i18n("Do Something"),
	 *   i18n("This action allows you to do something really great with this program to "
	 *        "the currently open document."),
	 *   ALT+CTRL+Key_Q, KKey::QtWIN+CTRL+Key_Q, this, TQ_SLOT(slotDoSomething()) );
	 *\endcode
	 *
	 * @param sAction The internal name of the action.
	 * @param sLabel An i18n'ized short description of the action displayed when
	 *  using KKeyChooser to reconfigure the shortcuts.
	 * @param sWhatsThis An extended description of the action.
	 * @param cutDef3 The default 3 modifier scheme shortcut.
	 * @param cutDef4 The default 4 modifier scheme shortcut.
	 * @param pObjSlot Pointer to the slot object.
	 * @param psMethodSlot Pointer to the slot method.
	 * @param bConfigurable Allow the user to change this shortcut if set to 'true'.
	 * @param bEnabled The action will be activated by the shortcut if set to 'true'.
	 */
	TDEAccelAction* insert( const TQString& sAction, const TQString& sLabel, const TQString& sWhatsThis,
	                 const TDEShortcut& cutDef3, const TDEShortcut& cutDef4,
	                 const TQObject* pObjSlot, const char* psMethodSlot,
	                 bool bConfigurable = true, bool bEnabled = true );

	/**
	 * Removes the accelerator action identified by the name.
	 * Remember to also call updateConnections().
	 * @param sAction the name of the action to remove
	 * @since 3.1
	 */
	bool remove( const TQString& sAction );

	/**
	 * Use this to insert a label into the action list.  This will be
	 * displayed when the user configures shortcuts.
	 * @param sName of the of the action to insert
	 * @param sLabel a user-readable (i18n!) name for the action
	 * @return the TDEAccelAction of the action
	 */
	TDEAccelAction* insert( const TQString& sName, const TQString& sLabel );

	/**
	 * Updates the connections of the accelerations after changing them. 
	 * @return true if successful, false otherwise
	 */
	bool updateConnections();

	/**
	 * Return the shortcut associated with the action named by @p sAction.
	 * @param sAction the name of the action
	 * @return the shortcut. If the action does not exist a null shortcut will be returned.
	 */
	const TDEShortcut& shortcut( const TQString& sAction ) const;
	/**
	 * Set the shortcut to be associated with the action named by @p sAction.
	 * @param sAction the name of the action
	 * @param shortcut the shortcut for the action
	 * @return true if successful, false otherwise
	 */
	bool setShortcut( const TQString& sAction, const TDEShortcut &shortcut );
	/**
	 * Set the slot to be called when the shortcut of the action named
	 * by @p sAction is pressed.
	 * @param sAction the name of the action
	 * @param pObjSlot the receiver of the signal
	 * @param psMethodSlot the slot to receive the signal
	 * @return true if successful, false otherwise
	 */
	bool setSlot( const TQString& sAction, const TQObject* pObjSlot, const char* psMethodSlot );

        /**
         * Enables or disables action @p sAction.
         * @since 3.4
         */ 
	bool setActionEnabled( const TQString& sAction, bool bEnable );
	/**
	 * Return the label (i18n'ized short description) associated with the action named by @p sAction.
	 * @param sAction the name of the action
	 * @return the label
	 * @since 3.3
	 */
	TQString label( const TQString& sAction ) const;

	/**
	 * Returns the configuration group that is used to save the accelerators.
	 * @return the configuration group
	 * @see TDEConfig
	 */
	const TQString& configGroup() const;

	/**
	 * Sets the configuration group that is used to save the accelerators.
	 * @param cg the configuration group
	 * @see TDEConfig
	 */
	void setConfigGroup( const TQString &cg );

	/**
	 * Read all shortcuts from @p pConfig, or (if @p pConfig
	 * is zero) from the application's configuration file
	 * TDEGlobal::config().
	 * @param pConfig the configuration file to read from, or 0 for the application
	 *                 configuration file
	 * @return true if successful, false otherwise
	 */
	bool readSettings( TDEConfigBase* pConfig = 0 );

	/**
	 * Write the current shortcuts to @p pConfig,
	 * or (if @p pConfig is zero) to the application's
	 * configuration file.
	 * @param pConfig the configuration file to read from, or 0 for the application
	 *                 configuration file
	 * @return true if successful, false otherwise
	 * @since 3.1
	 */
	bool writeSettings( TDEConfigBase* pConfig = 0 ) const;
	// BCI: merge these two writeSettings methods in KDE 4.0
	/**
	 * Write the current shortcuts to @p pConfig,
	 * or (if @p pConfig is zero) to the application's
	 * configuration file.  Alternatively, if bGlobal is true, then write
	 * to kdeglobals.
	 * @param pConfig the configuration file to read from, or 0 for the application
	 *                 configuration file
	 * @param bGlobal if true write the configuration to the kde global settings
	 * @return true if successful, false otherwise
	 */
	bool writeSettings( TDEConfigBase* pConfig, bool bGlobal ) const;

	/**
	 * @internal -- this a wrapper function to
	 * TDEAccelActions::useFourModifierKeys().
	 */
	static bool useFourModifierKeys();
        
        /**
         * @internal
         */
        static void blockShortcuts( bool block );
        /**
         * @internal
         */
        void disableBlocking( bool disable );
        
        /**
         * @internal
         */
        // like setEnabled(), but doesn't ungrab (see in TDEGlobalAccelPrivate)
        void suspend( bool s );

private:

	TDEAccelActions& actions();
	const TDEAccelActions& actions() const;

	friend class TDEGlobalAccelPrivate;
	friend class TDEAccelShortcutList;
protected:
	/** \internal */
	virtual void virtual_hook( int id, void* data );
private:
	class TDEGlobalAccelPrivate* d;
};

#endif // _KGLOBALACCEL_H_
