/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>
    Copyright (C) 2004 Frans Englich <frans.englich@telia.com>

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

#ifndef TDECMODULEPROXY_H
#define TDECMODULEPROXY_H

#include <tqwidget.h>
#include <tqstringlist.h>

#include <kservice.h>
#include <tdelibs_export.h>

class TDEAboutData;
class TDECModule;
class TDECModuleInfo;
class TDEInstance;
class TDEProcess;

/**
 * @ingroup tdecmodule
 *
 * @brief Encapsulates a TDECModule for embedding.
 *
 * TDECModuleProxy is a wrapper for TDECModule intended for cases where
 * modules are to be displayed. It ensures layout is consistent, handles
 * root/administrator modules and in general takes care of the details
 * needed for making a module available in an interface. A TDECModuleProxy
 * can be treated as a TQWidget, without worrying about the details specific
 * for modules such as library loading. TDECModuleProxy is not a sub class of TDECModule
 * but its API closely resembles TDECModule's.\n
 * Usually, an instance is created by passing one of the constructors a KService::Ptr,
 * TDECModuleInfo or simply the name of the module and then added to the layout as any
 * other widget. \n
 * When the user have changed the module, changed( bool ) as well as changed ( TDECModuleProxy * )
 * is emitted. TDECModuleProxy does not take care of prompting for saving - if the object is deleted while
 * changes is not saved the changes will be lost. changed() returns true if changes are unsaved. \n
 * \n
 * TDECModuleProxy does not take care of authorization of TDECModules. \n
 * TDECModuleProxy do lazy loading, meaning the library will not be loaded or
 * any other initialization done before its show() function is called. This means
 * modules will only be loaded when they are actually needed as well as it is possible to
 * load many TDECModuleProxy without any speed penalty.
 *
 * TDECModuleProxy should be used in all cases where modules are embedded in order to
 * promote code efficiency and usability consistency.
 *
 * @author Frans Englich <frans.englich@telia.com>
 * @author Matthias Kretz <kretz@kde.org>
 *
 */
class TDEUTILS_EXPORT TDECModuleProxy : public TQWidget
{
TQ_OBJECT


	friend class TDECModuleProxyRootCommunicatorImpl;

public:

	/**
	 * Constructs a TDECModuleProxy from a TDECModuleInfo class.
	 *
	 * @param info The TDECModuleInfo to construct the module from.
	 * @param withFallback If set to true and loading of the module fails,
	 * a alternative will be tried, resulting in the module appearing in its
	 * own window, if at all.
	 * The embedded module will be load()ed.
	 * @param parent the parent TQWidget.
	 * @param name the module's name.
	 * @param args This is used in the implementation and is internal.
	 * Use the default.
	 */
	TDECModuleProxy( const TDECModuleInfo & info, bool withFallback = true,
			TQWidget * parent = 0, const char * name = 0,
			const TQStringList & args = TQStringList() );

	/**
	 * Constructs a TDECModuleProxy from a module's service name, which is
	 * equivalent to the desktop file for the kcm without the ".desktop" part.
	 * Otherwise equal to the one above.
	 *
	 * @param serviceName The module's service name to construct from.
	 * @param withFallback If set to true and loading of the module fails,
	 * a alternative will be tried, resulting in the module appearing in its
	 * own window, if at all.
	 * The embedded module will be load()ed.
	 * @param parent the parent TQWidget.
	 * @param name the module's name.
	 * @param args This is used in the implementation and is internal.
	 * Use the default.
	 */
	TDECModuleProxy( const TQString& serviceName, bool withFallback = true,
			TQWidget * parent = 0, const char * name = 0,
			const TQStringList & args = TQStringList() );

	/**
	 * Constructs a TDECModuleProxy from KService. Otherwise equal to the one above.
	 *
	 * @param service The KService to construct from.
	 * @param withFallback If set to true and loading of the module fails,
	 * a alternative will be tried, resulting in the module appearing in its
	 * own window, if at all.
	 * The embedded module will be load()ed.
	 * @param parent the parent TQWidget.
	 * @param name the module's name.
	 * @param args This is used in the implementation and is internal.
	 * Use the default.
	 */
	TDECModuleProxy( const KService::Ptr& service, bool withFallback = true,
			TQWidget  * parent = 0, const char * name = 0,
			const TQStringList & args = TQStringList() );

	/**
	 * Default destructor
	 */
	~TDECModuleProxy();

	/**
	 * Calling it will cause the contained module to
	 * run its load() routine.
	 */
	void load();

	/**
	 * Calling it will cause the contained module to
	 * run its save() routine.
	 *
	 * If the module was not modified, it will not be asked
	 * to save.
	 */
	void save();

	/**
	 * @return the module's quickHelp();
	 */
	TQString quickHelp() const;

	/**
	 * @return the module's aboutData()
	 */
	const TDEAboutData * aboutData() const;

	/**
	 * @return the module's handbookDocPath()
	 */
	TQString handbookDocPath() const;

	/**
	 * @return the module's handbookSection()
	 */
	TQString handbookSection() const;

	/**
	 * @return what buttons the module
	 * needs
	 */
	int buttons() const;

	/**
	 * @return The module's custom root
	 * message, if it has one
	 * @deprecated
	 */
	TQString rootOnlyMsg() const;
	//KDE4 remove. There's a limit for convenience functions,
	// this one's available via moduleInfo()-> and realModule()->

	/**
	 * @return If the module is a root module.
	 * @deprecated
	 */
	bool useRootOnlyMsg() const;
	//KDE4 remove. There's a limit for convenience functions,
	// this one's available via moduleInfo()-> and realModule()->

	/**
	 * Returns the embedded TDECModule's TDEInstance.
	 * @return The module's TDEInstance.
	 * @deprecated
	 */
	TDEInstance * instance() const;
	//KDE4 remove. There's a limit for convenience functions,
	// this one's available via realModule()

	/**
	 * @return true if the module is modified
	 * and needs to be saved.
	 */
	bool changed() const;

	/**
	 * Returns whether the module is running in root mode. A module is in root mode
	 * when runAsRoot() has been called. A session under root user will never reach
	 * root mode.
	 *
	 * @note realModule() will return null when the module is running in root mode.
	 *
	 * @return true if the module is running with root privileges
	 * @since 3.4
	 */
	bool rootMode() const;

	/**
	 * Access to the actual module. However, if the module is
	 * running in root mode, see rootMode(), this function returns
	 * a NULL pointer, since the module is in another process. It may also
	 * return NULL if anything goes wrong.
	 *
	 * @return the encapsulated module.
	 */
	TDECModule* realModule() const;

	/**
	 * @return a TDECModuleInfo for the encapsulated
	 * module
	 */
	const TDECModuleInfo& moduleInfo() const;

	/**
	 * Returns the DCOP the module's DCOPClient
	 * and DCOPObject has(they are identical).
	 *
	 * @since 3.4
	 */
	TQCString dcopName() const;

public slots:

	/**
	 * Calling this will cause the module to be run in
	 * "administrator mode".
	 *
	 * @since 3.4
	 */
	void runAsRoot();

	/**
	 * Calling it will cause the contained module to
	 * load its default values.
	 */
	void defaults();

	/**
	 * Calling this, results in deleting the contained
	 * module, and unregistering from DCOP. A similar result is achieved
	 * by deleting the TDECModuleProxy itself.
	 *
	 * @since 3.4
	 */
	void deleteClient();

signals:

	/*
	 * This signal is emitted when the contained module is changed.
	 */
	void changed( bool state );

	/**
	 * This is emitted in the same situations as in the one above. Practical
	 * when several TDECModuleProxys are loaded.
	 *
	 * @since 3.4
	 */
	void changed( TDECModuleProxy* mod );

	/**
	 * When a module running with root privileges and exits, returns to normal mode, the
	 * childClosed() signal is emitted.
	 *
	 * @since 3.4
	 */
	void childClosed();

	/*
	 * This signal is relayed from the encapsulated module, and
	 * is equivalent to the module's own quickHelpChanged() signal.
	 *
	 * @since 3.4
	 */
	void quickHelpChanged();

protected:

	/**
	 * Reimplemented for internal purposes. Makes sure the encapsulated
	 * module is loaded before the show event is taken care of.
	 */
	void showEvent( TQShowEvent * );

	/**
	 * Internal intialization function, called by the constructors.
	 *
	 * @internal
	 * @since 3.4
	 */
	void init( const TDECModuleInfo& info );


	/**
	 * Emits the quickHelpChanged signal.
	 * @since 3.4
	 */
	void emitQuickHelpChanged();

private slots:

   /**
	* Calls the function @p function of the root module's TDECModuleProxy
	* DCOP interface.
	*
	* @param function the function signature of the function to call.
	* @since 3.4
	*/
	void callRootModule( const TQCString& function );

	/**
	 * This is called when the module exits from root mode. It zeroes
	 * pointers, deletes the embed window, and so forth.
	 *
	 * @since 3.4
	 */
	void rootExited();

	/**
	 * Makes sure the proper variables is set and signals are emitted.
	 */
	void moduleChanged( bool );

	/**
	 * Zeroes d->kcm
	 */
	void moduleDestroyed();

	/**
	 * Gets called by DCOP when an application closes.
	 * Is used to (try to) reload a KCM which previously
	 * was loaded.
	 *
	 * @since 3.4
	 */
	void applicationRemoved( const TQCString& app );

private:

	class TDECModuleProxyPrivate;
	TDECModuleProxyPrivate * d;
};

#endif // TDECMODULEPROXY_H
