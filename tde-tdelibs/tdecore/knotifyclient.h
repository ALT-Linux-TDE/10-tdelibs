/* This file is part of the KDE libraries
   Copyright (C) 2000 Charles Samuels <charles@kde.org>

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
#ifndef _KNOTIFY_CLIENT
#define _KNOTIFY_CLIENT
#include <tqstring.h>
#include "tdelibs_export.h"

class TDEInstance;
#undef None // X11 headers...

/**
 * This namespace provides a method for issuing events to a KNotifyServer
 * call KNotifyClient::event("eventname"); to issue it.
 * On installation, there should be a file called
 * $TDEDIR/share/apps/appname/eventsrc which contains the events.
 *
 * The file looks like this:
 * \code
 * [!Global!]
 * IconName=Filename (e.g. kdesktop, without any extension)
 * Comment=FriendlyNameOfApp
 *
 * [eventname]
 * Name=FriendlyNameOfEvent
 * Comment=Description Of Event
 * default_sound=filetoplay.wav
 * default_logfile=logfile.txt
 * default_commandline=command
 * default_presentation=1
 *  ...
 * \endcode
 * default_presentation contains these ORed events:
 *	None=0, Sound=1, Messagebox=2, Logfile=4, Stderr=8, PassivePopup=16,
 *      Execute=32, Taskbar=64
 *
 * KNotify will search for sound files given with a relative path first in
 * the application's sound directory (share/apps/Application Name/sounds), then in
 * the KDE global sound directory (share/sounds).
 *
 * You can also use the "nopresentation" key, with any the presentations
 * ORed.  Those that are in that field will not appear in the kcontrol
 * module.  This was intended for software like KWin to not allow a window-opening
 * that opens a window (e.g., allowing to disable KMessageBoxes from appearing)
 * If the user edits the eventsrc file manually, it will appear.  This only
 * affects the KcmNotify.
 *
 * You can also use the following events, which are system controlled
 * and do not need to be placed in your eventsrc:
 *
 *<ul>
 * <li>cannotopenfile
 * <li>notification
 * <li>warning
 * <li>fatalerror
 * <li>catastrophe
 *</ul>
 *
 * The events can be configured in an application using KNotifyDialog, which is part of TDEIO.
 * 
 * @author Charles Samuels <charles@kde.org>
 */


namespace KNotifyClient
{
    struct InstancePrivate;
	class InstanceStack;

    /**
     * Makes it possible to use KNotifyClient with a TDEInstance
     * that is not the application.
     *
     * Use like this:
     * \code
     * KNotifyClient::Instance(myInstance);
     * KNotifyClient::event("MyEvent");
     * \endcode
     *
     * @short Enables KNotifyClient to use a different TDEInstance
     */
    class TDECORE_EXPORT Instance
    {
    public:
        /**
         * Constructs a KNotifyClient::Instance to make KNotifyClient use
         * the specified TDEInstance for the event configuration.
	 * @param instance the instance for the event configuration
         */
        Instance(TDEInstance *instance);
        /**
         * Destructs the KNotifyClient::Instance and resets KNotifyClient
         * to the previously used TDEInstance.
         */
        ~Instance();
	/**
	 * Checks whether the system bell should be used.
	 * @returns true if this instance should use the System bell instead
	 * of KNotify.
	 */
	bool useSystemBell() const;
        /**
         * Returns the currently active TDEInstance.
	 * @return the active TDEInstance
         */
        static TDEInstance *current();

	/**
	 * Returns the current KNotifyClient::Instance (not the TDEInstance).
	 * @return the active Instance
	 */
	static Instance *currentInstance();
	
    private:
		static InstanceStack *instances();
		InstancePrivate *d;
		static InstanceStack *s_instances;
    };


    /**
     * Describes the notification method.
     */
	enum {
		Default = -1,
		None = 0,
		Sound = 1,
		Messagebox = 2,
		Logfile = 4,
		Stderr = 8,
		PassivePopup = 16, ///< @since 3.1
		Execute = 32,      ///< @since 3.1
		Taskbar = 64       ///< @since 3.2
	};

	/**
	 * Describes the level of the error.
	 */
	enum {
		Notification=1,
		Warning=2,
		Error=4,
		Catastrophe=8
	};

	/**
	 * default events you can use
	 */
	enum StandardEvent {
		cannotOpenFile,
		notification,
		warning,
		fatalError,
		catastrophe
	};

	/**
	 * This starts the KNotify Daemon, if it's not already started.
	 * This will be useful for games that use sound effects. Run this
	 * at the start of the program, and there won't be a pause when it is
	 * first triggered.
	 * @return true if daemon is running (always true at the moment)
	 **/
	TDECORE_EXPORT bool startDaemon();

//#ifndef KDE_NO_COMPAT
	/**
	 * @deprecated
	 * @param message The name of the event
	 * @param text The text to put in a dialog box.  This won't be shown if
	 *             the user connected the event to sound, only. Can be TQString::null.
	 * @return a value > 0, unique for this event if successful, 0 otherwise
	 */
	TDECORE_EXPORT int event(const TQString &message, const TQString &text=TQString::null) TDE_DEPRECATED;

	/**
	 * @deprecated
	 * Allows to easily emit standard events.
	 * @param event The event you want to raise.
	 * @param text The text explaining the event you raise. Can be TQString::null.
	 * @return a value > 0, unique for this event if successful, 0 otherwise
	 */
	TDECORE_EXPORT int event( StandardEvent event, const TQString& text=TQString::null ) TDE_DEPRECATED;

	/**
	 * @deprecated
	 * Will fire an event that's not registered.
	 * @param text The error message text, if applicable
	 * @param present The presentation method(s) of the event
	 * @param level The error message level, defaulting to "Default"
	 * @param sound The sound file to play if selected with @p present
	 * @param file The log file to append the message to if selected with @p present
	 * @return a value > 0, unique for this event if successful, 0 otherwise
	 */
	TDECORE_EXPORT int userEvent(const TQString &text=TQString::null, int present=Default, int level=Default,
	                             const TQString &sound=TQString::null, const TQString &file=TQString::null) TDE_DEPRECATED;
	
//#endif

	/**
	 * This should be the most used method in here.
	 * Pass the origin-widget's winId() here so that a PassivePopup can be
	 * placed appropriately.
	 *
	 * Call it by KNotifyClient::event(widget->winId(), "EventName");
	 * It will use TDEApplication::kApplication->dcopClient() to communicate to
	 * the server
	 * @param winId The winId() of the widget where the event originates
	 * @param message The name of the event
	 * @param text The text to put in a dialog box.  This won't be shown if
	 *             the user connected the event to sound, only. Can be TQString::null.
	 * @return a value > 0, unique for this event if successful, 0 otherwise
	 * @since 3.1.1
	 */
	// KDE4: use WId instead of int
	TDECORE_EXPORT int event( int winId, const TQString& message,
                              const TQString& text = TQString::null );

	/**
	 * You should
	 * pass the origin-widget's winId() here so that a PassivePopup can be
	 * placed appropriately.
	 * @param winId The winId() of the widget where the event originates
	 * @param event The event you want to raise
	 * @param text The text to put in a dialog box.  This won't be shown if
	 *             the user connected the event to sound, only. Can be TQString::null.
	 * @return a value > 0, unique for this event if successful, 0 otherwise
	 * @since 3.1.1
	 */
	// KDE4: use WId instead of int
	TDECORE_EXPORT int event( int winId, StandardEvent event,
                              const TQString& text = TQString::null );

	/**
	 * Will fire an event that's not registered.
	 * You should
	 * pass the origin-widget's winId() here so that a PassivePopup can be
	 * placed appropriately.
	 * @param winId The winId() of the originating widget
	 * @param text The error message text, if applicable
	 * @param present The presentation method(s) of the event
	 * @param level The error message level, defaulting to "Default"
	 * @param sound The sound file to play if selected with @p present
	 * @param file The log file to append the message to if selected with @p present
	 * @return a value > 0, unique for this event if successful, 0 otherwise
	 * @since 3.1.1
	 */
	// KDE4: use WId instead of int
	TDECORE_EXPORT int userEvent(int winId, const TQString &text=TQString::null, int present=Default, int level=Default,
	                      const TQString &sound=TQString::null, const TQString &file=TQString::null);
	
	/**
	 * This is a simple substitution for TQApplication::beep().
	 * It simply calls
	 * \code
	 * KNotifyClient::event( KNotifyClient::notification, reason );
	 * \endcode
	 * @param reason the reason, can be TQString::null.
	 */
	TDECORE_EXPORT void beep(const TQString& reason=TQString::null);

	/**
	 * Gets the presentation associated with a certain event name
	 * Remeber that they may be ORed:
	 * \code
	 * if (present & KNotifyClient::Sound) { [Yes, sound is a default] }	
	 * \endcode
	 * @param eventname the event name to check
	 * @return the presentation methods
	 */
	TDECORE_EXPORT int getPresentation(const TQString &eventname);
	
	/**
	 * Gets the default file associated with a certain event name
	 * The control panel module will list all the event names
	 * This has the potential for being slow.
	 * @param eventname the name of the event
	 * @param present the presentation method
	 * @return the associated file. Can be TQString::null if not found.
	 */
	TDECORE_EXPORT TQString getFile(const TQString &eventname, int present);
	
	/**
	 * Gets the default presentation for the event of this program.
	 * Remember that the Presentation may be ORed.  Try this:
	 * \code
	 * if (present & KNotifyClient::Sound) { [Yes, sound is a default] }
	 * \endcode
	 * @return the presentation methods
	 */
	TDECORE_EXPORT int getDefaultPresentation(const TQString &eventname);
	
	/**
	 * Gets the default File for the event of this program.
	 * It gets it in relation to present.
	 * Some events don't apply to this function ("Message Box")
	 * Some do (Sound)
	 * @param eventname the name of the event
	 * @param present the presentation method
	 * @return the default file. Can be TQString::null if not found.
	 */
	TDECORE_EXPORT TQString getDefaultFile(const TQString &eventname, int present);

	/**
	 * Shortcut to KNotifyClient::Instance::current() :)
	 * @returns the current TDEInstance.
	 */
	TDECORE_EXPORT TDEInstance * instance();
}

#endif
