/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (c) 1998, 1999 KDE Team

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

#ifndef _TDEAPP_H
#define _TDEAPP_H

// Version macros. Never put this further down.
#include "tdeversion.h"
#include "tdelibs_export.h"

class TDEConfig;
class KCharsets;
class DCOPClient;
class DCOPObject;

typedef unsigned long Atom;
#if !defined(TQ_WS_X11)
typedef void Display;
#endif

#include <tqapplication.h>
#include <tqpixmap.h>
#include <kinstance.h>

struct _IceConn;
class TQPopupMenu;
class TQStrList;
class KSessionManaged;
class TDEStyle;
class KURL;

#define kapp TDEApplication::kApplication()

class TDEApplicationPrivate;

/**
* Controls and provides information to all KDE applications.
*
* Only one object of this class can be instantiated in a single app.
* This instance is always accessible via the 'kapp' global variable.
* See cut() for an example.
*
* This class provides the following services to all KDE applications.
*
* @li It controls the event queue (see TQApplication ).
* @li It provides the application with KDE resources such as
* accelerators, common menu entries, a TDEConfig object. session
* management events, help invocation etc.
* @li Installs a signal handler for the SIGCHLD signal in order to
* avoid zombie children. If you want to catch this signal yourself or
* don't want it to be caught at all, you have set a new signal handler
* (or SIG_IGN) after TDEApplication's constructor has run.
* @li Installs an empty signal handler for the SIGPIPE signal using
* installSigpipeHandler(). If you want to catch this signal
* yourself, you have set a new signal handler after TDEApplication's
* constructor has run.
* @li It can start new services
*
*
* The way a service gets started depends on the 'X-DCOP-ServiceType'
* entry in the desktop file of the service:
*
* There are three possibilities:
* @li X-DCOP-ServiceType=None (default)
*    Always start a new service,
*    don't wait till the service registers with dcop.
* @li X-DCOP-ServiceType=Multi
*    Always start a new service,
*    wait until the service has registered with dcop.
* @li X-DCOP-ServiceType=Unique
*    Only start the service if it isn't already running,
*    wait until the service has registered with dcop.
*
* @short Controls and provides information to all KDE applications.
* @author Matthias Kalle Dalheimer <kalle@kde.org>
*/
class TDECORE_EXPORT TDEApplication : public TQApplication, public TDEInstance
{

  TQ_OBJECT
public:
  /** Position of the caption (presumably in the application window's
  *   title bar). This enum appears to be unused.
  *
  * @todo Find out if this is used anywhere.
  */
  enum CaptionLayout {
    CaptionAppLast=1 /**< Display the application name last (before document name). */, 
    CaptionAppFirst /**< Display the application name first. */ , 
    CaptionNoApp  /**< Do not display application name at all. */
  };

  /**
   * This constructor takes aboutData and command line
   *  arguments from TDECmdLineArgs.
   *
   * If ARGB (transparent) widgets are to be used in your application,
   * please use
   * TDEApplication app(TDEApplication::openX11RGBADisplay());
   * or
   * TDEApplication app(TDEApplication::openX11RGBADisplay(), useStyles);
   *
   * @param allowStyles Set to false to disable the loading on plugin based
   * styles. This is only useful to applications that do not display a GUI
   * normally. If you do create an application with @p allowStyles set to false
   * it normally runs in the background but under special circumstances
   * displays widgets.  Call enableStyles() before displaying any widgets.
   *
   * @param GUIenabled Set to false to disable all GUI stuff. This implies
   * no styles either.
   */
  TDEApplication( bool allowStyles=true, bool GUIenabled=true, bool SMenabled=true);

#ifdef Q_QDOC
#else // Q_QDOC
#ifdef TDEAPPLICATION_BINARY_COMPAT_HACK
  // FIXME
  // FOR BINARY COMPATIBILITY ONLY
  // REMOVE WHEN PRACTICAL!
  TDEApplication( bool allowStyles=true, bool GUIenabled=true);
#endif // TDEAPPLICATION_BINARY_COMPAT_HACK
#endif // Q_QDOC

#ifdef TQ_WS_X11
  /**
   * Constructor. Parses command-line arguments. Use this constructor when you
   * you want ARGB support to be automatically detected and enabled.
   *
   * @param display Will be passed to Qt as the X display. The display must be
   * valid and already opened.
   *
   * @param allowStyles Set to false to disable the loading on plugin based
   * styles. This is only useful to applications that do not display a GUI
   * normally. If you do create an application with @p allowStyles set to false
   * that normally runs in the background but under special circumstances
   * displays widgets call enableStyles() before displaying any widgets.
   *
   * @since KDE 3.5
   * 
   * @see RGBADisplay()
   */
  TDEApplication(Display *display, bool allowStyles);

  /**
   * Constructor. Parses command-line arguments. Use this constructor when you
   * you want ARGB support to be automatically detected and enabled.
   *
   * @param display Will be passed to Qt as the X display. The display must be
   * valid and already opened.
   *
   * @param disable_argb Set to true to disable ARGB visuals in this application.
   *
   * @param display Will be passed to Qt as the X display. The display must be
   * valid and already opened.
   *
   * @param visual A pointer to the X11 visual that should be used by the
   * appliction. Note that only TrueColor visuals are supported on depths
   * greater than 8 bpp. If this parameter is NULL, the default visual will
   * be used instead.
   *
   * @param allowStyles Set to false to disable the loading on plugin based
   * styles. This is only useful to applications that do not display a GUI
   * normally. If you do create an application with @p allowStyles set to false
   * that normally runs in the background but under special circumstances
   * displays widgets call enableStyles() before displaying any widgets.
   *
   * @since KDE 3.5
   * 
   * @see RGBADisplay()
   */
  TDEApplication(Display *display, bool disable_argb, TQt::HANDLE visual, TQt::HANDLE colormap, bool allowStyles);

  /**
   * Constructor. Parses command-line arguments. Use this constructor when you
   * you need to use a non-default visual or colormap. 
   *
   * @param display Will be passed to Qt as the X display. The display must be
   * valid and already opened.
   *
   * @param visual A pointer to the X11 visual that should be used by the
   * appliction. Note that only TrueColor visuals are supported on depths
   * greater than 8 bpp. If this parameter is NULL, the default visual will
   * be used instead.
   *
   * @param colormap The colormap that should be used by the application. If
   * this parameter is 0, the default colormap will be used instead.
   *
   * @param allowStyles Set to false to disable the loading on plugin based
   * styles. This is only useful to applications that do not display a GUI
   * normally. If you do create an application with @p allowStyles set to false
   * that normally runs in the background but under special circumstances
   * displays widgets call enableStyles() before displaying any widgets.
   *
   * @since KDE 3.3
   */
  TDEApplication(Display *display, TQt::HANDLE visual = 0, TQt::HANDLE colormap = 0,
               bool allowStyles=true);

  /**
   * Constructor. Parses command-line arguments. Use this constructor to use TDEApplication
   * in a Motif or Xt program.
   *
   * @param display Will be passed to Qt as the X display. The display must be valid and already
   * opened.
   *
   * @param argc command line argument count
   *
   * @param argv command line argument value(s)
   *
   * @param rAppName application name. Will be used for finding the
   * associated message files and icon files, and as the default
   * registration name for DCOP. This is a mandatory parameter.
   *
   * @param allowStyles Set to false to disable the loading on plugin based
   * styles. This is only useful to applications that do not display a GUI
   * normally. If you do create an application with @p allowStyles set to false
   * that normally runs in the background but under special circumstances
   * displays widgets call enableStyles() before displaying any widgets.
   *
   * @param GUIenabled Set to false to disable all GUI stuff. This implies
   * no styles either.
   */
  TDEApplication(Display *display, int& argc, char** argv, const TQCString& rAppName,
               bool allowStyles=true, bool GUIenabled=true);
#endif

  /**
   * @deprecated do not use it at all, it will make your application crash, use TDECmdLineArgs
   *
   * Constructor. Parses command-line arguments.
   *
   * @param argc command line argument count
   *
   * @param argv command line argument value(s)
   *
   * @param rAppName application name. Will be used for finding the
   * associated message files and icon files, and as the default
   * registration name for DCOP. This is a mandatory parameter.
   *
   * @param allowStyles Set to false to disable the loading on plugin based
   * styles. This is only useful to applications that do not display a GUI
   * normally. If you do create an application with @p allowStyles set to false
   * that normally runs in the background but under special circumstances
   * displays widgets call enableStyles() before displaying any widgets.
   *
   * @param GUIenabled Set to false to disable all GUI stuff. This implies
   * no styles either.
   */
  // REMOVE FOR KDE 4.0 - using it only gives crashing applications because
  // TDECmdLineArgs::init isn't called
  TDEApplication(int& argc, char** argv,
              const TQCString& rAppName, bool allowStyles=true, bool GUIenabled=true, bool SMenabled=true) TDE_DEPRECATED;

#ifdef Q_QDOC
#else // Q_QDOC
#ifdef TDEAPPLICATION_BINARY_COMPAT_HACK
  // FIXME
  // FOR BINARY COMPATIBILITY ONLY
  // REMOVE WHEN PRACTICAL!
  TDEApplication(int& argc, char** argv,
              const TQCString& rAppName, bool allowStyles, bool GUIenabled) TDE_DEPRECATED;
#endif // TDEAPPLICATION_BINARY_COMPAT_HACK
#endif // Q_QDOC

  /**
    * Add Qt and KDE command line options to TDECmdLineArgs.
    */
  static void addCmdLineOptions();

  virtual ~TDEApplication();

  /**
   * Returns the current application object.
   *
   * This is similar to the global TQApplication pointer tqApp. It
   * allows access to the single global TDEApplication object, since
   * more than one cannot be created in the same application. It
   * saves you the trouble of having to pass the pointer explicitly
   * to every function that may require it.
   * @return the current application object
   */
  static TDEApplication* kApplication() { return KApp; }

  /**
   * Returns the application session config object.
   *
   * @return A pointer to the application's instance specific
   * TDEConfig object.
   * @see TDEConfig
   */
  TDEConfig* sessionConfig();

  /**
   * Is the application restored from the session manager?
   *
   * @return If true, this application was restored by the session manager.
   *    Note that this may mean the config object returned by
   * sessionConfig() contains data saved by a session closedown.
   * @see sessionConfig()
   */
  bool isRestored() const { return TQApplication::isSessionRestored(); }

  /**
   * Disables session management for this application.
   *
   * Useful in case  your application is started by the
   * initial "starttde" script.
   */
  void disableSessionManagement();

  /**
   * Enables again session management for this application, formerly
   * disabled by calling disableSessionManagement(). You usually
   * shouldn't call this function, as the session management is enabled
   * by default.
   */
  void enableSessionManagement();

  /**
   * The possible values for the @p confirm parameter of requestShutDown().
   */
  enum ShutdownConfirm {
    /**
     * Obey the user's confirmation setting.
     */
    ShutdownConfirmDefault = -1,
    /**
     * Don't confirm, shutdown without asking.
     */
    ShutdownConfirmNo = 0,
    /**
     * Always confirm, ask even if the user turned it off.
     */
    ShutdownConfirmYes = 1
  };

  /**
   * The possible values for the @p sdtype parameter of requestShutDown().
   */
  enum ShutdownType {
    /**
     * Select previous action or the default if it's the first time.
     */
    ShutdownTypeDefault = -1,
    /**
     * Only log out.
     */
    ShutdownTypeNone = 0,
    /**
     * Log out and reboot the machine.
     */
    ShutdownTypeReboot = 1,
    /**
     * Log out and halt the machine.
     */
    ShutdownTypeHalt = 2
  };

  /**
   * The possible values for the @p sdmode parameter of requestShutDown().
   */
  enum ShutdownMode {
    /**
     * Select previous mode or the default if it's the first time.
     */
    ShutdownModeDefault = -1,
    /**
     * Schedule a shutdown (halt or reboot) for the time all active sessions
     * have exited.
     */
    ShutdownModeSchedule = 0,
    /**
     * Shut down, if no sessions are active. Otherwise do nothing.
     */
    ShutdownModeTryNow = 1,
    /**
     * Force shutdown. Kill any possibly active sessions.
     */
    ShutdownModeForceNow = 2,
    /**
     * Pop up a dialog asking the user what to do if sessions are still active.
     */
    ShutdownModeInteractive = 3
  };

  /**
   * Asks the session manager to shut the session down.
   *
   * Using @p confirm == ShutdownConfirmYes or @p sdtype != ShutdownTypeDefault or
   * @p sdmode != ShutdownModeDefault causes the use of ksmserver's DCOP
   * interface. The remaining two combinations use the standard XSMP and
   * will work with any session manager compliant with it.
   *
   * @param confirm Whether to ask the user if he really wants to log out.
   * ShutdownConfirm
   * @param sdtype The action to take after logging out. ShutdownType
   * @param sdmode If/When the action should be taken. ShutdownMode
   * @return true on success, false if the session manager could not be
   * contacted.
   */
  bool requestShutDown( ShutdownConfirm confirm = ShutdownConfirmDefault,
                        ShutdownType sdtype = ShutdownTypeDefault,
			ShutdownMode sdmode = ShutdownModeDefault );

  /**
   * Propagates the network address of the session manager in the
   * SESSION_MANAGER environment variable so that child processes can
   * pick it up.
   *
   * If SESSION_MANAGER isn't defined yet, the address is searched in
   * $HOME/.KSMserver.
   *
   * This function is called by clients that are started outside the
   * session ( i.e. before ksmserver is started), but want to launch
   * other processes that should participate in the session.  Examples
   * are kdesktop or kicker.
   */
    void propagateSessionManager();

    /**
     * Reimplemented for internal purposes, mainly the highlevel
     *  handling of session management with KSessionManaged.
     * @internal
     */
  void commitData( TQSessionManager& sm );

    /**
     * Reimplemented for internal purposes, mainly the highlevel
     *  handling of session management with KSessionManaged.
     * @internal
     */
  void saveState( TQSessionManager& sm );

  /**
   * Returns true if the application is currently saving its session
   * data (most probably before KDE logout). This is intended for use
   * mainly in TDEMainWindow::queryClose() and TDEMainWindow::queryExit().
   *
   * @see TDEMainWindow::queryClose
   * @see TDEMainWindow::queryExit
   * @since 3.1.1
   */
  bool sessionSaving() const;

  /**
   * Returns a pointer to a DCOPClient for the application.
   * If a client does not exist yet, it is created when this
   * function is called.
   * @return the DCOPClient for the application
   */
  static DCOPClient *dcopClient();

  /**
   * Disable automatic dcop registration
   * Must be called before creating a TDEApplication instance to have an effect.
   */
  static void disableAutoDcopRegistration();

  /**
   * Returns a TQPixmap with the application icon.
   * @return the application icon
   */
  TQPixmap icon() const;

  /**
   * Returns the name of the application icon.
   * @return the icon's name
   */
  TQString iconName() const;

  /**
   * Returns the mini-icon for the application as a TQPixmap.
   * @return the application's mini icon
   */
  TQPixmap miniIcon() const;

  /**
   * Returns the name of the mini-icon for the application.
   * @return the mini icon's name
   */
  TQString miniIconName() const;

  /**
   *  Sets the top widget of the application.
   *  This means basically applying the right window caption and
   *  icon. An application may have several top widgets. You don't
   *  need to call this function manually when using TDEMainWindow.
   *
   *  @param topWidget A top widget of the application.
   *
   *  @see icon(), caption()
   **/
  void setTopWidget( TQWidget *topWidget );

  /**
   * Invokes the KHelpCenter HTML help viewer from docbook sources.
   *
   * @param anchor      This has to be a defined anchor in your
   *                    docbook sources. If empty the main index
   *                    is loaded
   * @param appname     This allows you to show the help of another
   *                    application. If empty the current name() is
   *                    used
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   */
  void invokeHelp( const TQString& anchor,
                   const TQString& appname,
                   const TQCString& startup_id ) const;

  // KDE4 merge with above with startup_id = ""
  void invokeHelp( const TQString& anchor = TQString::null,
                   const TQString& appname = TQString::null ) const;

  /**
   * @deprecated
   * Invoke the khelpcenter HTML help viewer from HTML sources.
   * Please use invokeHelp() instead.
   *
   * @param aFilename  The filename that is to be loaded. Its
   *                   location is computed automatically
   *                   according to the KFSSTND.  If @p aFilename
   *                   is empty, the logical appname with .html
   *                   appended to it is used.
   * @param aTopic     This allows context-sensitive help. Its
   *                   value will be appended to the filename,
   *                   prefixed with a "#" (hash) character.
   */
  void invokeHTMLHelp( const TQString& aFilename, const TQString& aTopic = TQString::null ) const TDE_DEPRECATED;

  /**
   * Convenience method; invokes the standard email application.
   *
   * @param address The destination address
   * @param subject Subject string. Can be TQString::null.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   */
  void invokeMailer( const TQString &address, const TQString &subject, const TQCString& startup_id );
  // KDE4 merge with above with startup_id = ""
  void invokeMailer( const TQString &address, const TQString &subject );

  /**
   * Invokes the standard email application.
   *
   * @param mailtoURL A mailto URL.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param allowAttachments whether attachments specified in mailtoURL should be honoured.
               The default is false; do not honour requests for attachments.
   */
  void invokeMailer( const KURL &mailtoURL, const TQCString& startup_id, bool allowAttachments );
  // KDE4 merge with above with allowAttachments = false
  void invokeMailer( const KURL &mailtoURL, const TQCString& startup_id );
  // KDE4 merge with above with startup_id = ""
  void invokeMailer( const KURL &mailtoURL );

  /**
   * Convenience method; invokes the standard email application.
   *
   * All parameters are optional.
   *
   * @param to          The destination address.
   * @param cc          The Cc field
   * @param bcc         The Bcc field
   * @param subject     Subject string
   * @param body        A string containing the body of the mail (exclusive with messageFile)
   * @param messageFile A file (URL) containing the body of the mail (exclusive with body) - currently unsupported
   * @param attachURLs  List of URLs to be attached to the mail.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   */
  void invokeMailer(const TQString &to, const TQString &cc, const TQString &bcc,
                    const TQString &subject, const TQString &body,
                    const TQString &messageFile, const TQStringList &attachURLs,
                    const TQCString& startup_id );
  // KDE4 merge with above with startup_id = ""
  void invokeMailer(const TQString &to, const TQString &cc, const TQString &bcc,
                    const TQString &subject, const TQString &body,
                    const TQString &messageFile = TQString::null, const TQStringList &attachURLs = TQStringList());

public slots:
  /**
   * Invokes the standard browser.
   * Note that you should only do this when you know for sure that the browser can
   * handle the URL (i.e. its mimetype). In doubt, if the URL can point to an image
   * or anything else than directory or HTML, prefer to use new KRun( url ).
   *
   * @param url The destination address
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   */
  void invokeBrowser( const TQString &url, const TQCString& startup_id );
  // KDE4 merge with above with startup_id = ""
  /**
  * Invoke the standard browser. Uses a @p startup_id of "" (empty)
  * and is otherwise the same as the above function.
  */
  void invokeBrowser( const TQString &url );

  /**
   * If the widget with focus provides a cut() slot, call that slot.  Thus for a
   * simple application cut can be implemented as:
   * \code
   * KStdAction::cut( kapp, TQ_SLOT( cut() ), actionCollection() );
   * \endcode
   */
  void cut();

  /**
   * If the widget with focus provides a copy() slot, call that slot.  Thus for a
   * simple application copy can be implemented as:
   * \code
   * KStdAction::copy( kapp, TQ_SLOT( copy() ), actionCollection() );
   * \endcode
   */
  void copy();

  /**
   * If the widget with focus provides a paste() slot, call that slot.  Thus for a
   * simple application copy can be implemented as:
   * \code
   * KStdAction::paste( kapp, TQ_SLOT( paste() ), actionCollection() );
   * \endcode
   */
  void paste();

  /**
   * If the widget with focus provides a clear() slot, call that slot.  Thus for a
   * simple application clear() can be implemented as:
   * \code
   * new TDEAction( i18n( "Clear" ), "edit-clear", 0, kapp, TQ_SLOT( clear() ), actionCollection(), "clear" );
   * \endcode
   *
   * Note that for some widgets, this may not provide the intended bahavior.  For
   * example if you make use of the code above and a TDEListView has the focus, clear()
   * will clear all of the items in the list.  If this is not the intened behavior
   * and you want to make use of this slot, you can subclass TDEListView and reimplement
   * this slot.  For example the following code would implement a TDEListView without this
   * behavior:
   *
   * \code
   * class MyListView : public TDEListView {
   *   TQ_OBJECT
   * public:
   *   MyListView( TQWidget * parent = 0, const char * name = 0, WFlags f = 0 ) : TDEListView( parent, name, f ) {}
   *   virtual ~MyListView() {}
   * public slots:
   *   virtual void clear() {}
   * };
   * \endcode
   */
  void clear();

  /**
   * If the widget with focus provides a selectAll() slot, call that slot.  Thus for a
   * simple application select all can be implemented as:
   * \code
   * KStdAction::selectAll( kapp, TQ_SLOT( selectAll() ), actionCollection() );
   * \endcode
   */
  void selectAll();

  /**
   * Broadcast a received keycode to all listening KDE applications
   * The primary use for this feature is to connect hotkeys such as
   * XF86Display to their respective TDEGlobalAccel functions while
   * the screen is locked by kdesktop_lock.
   */
  void broadcastKeyCode(unsigned int keyCode);

public:
  /**
   * Returns the DCOP name of the service launcher. This will be something like
   * klaucher_$host_$uid.
   * @return the name of the service launcher
   */
  static TQCString launcher();

  /**
   * Starts a service based on the (translated) name of the service.
   * E.g. "Web Browser"
   *
   * @param _name the name of the service
   * @param URL if not empty this URL is passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByName( const TQString& _name, const TQString &URL,
                TQString *error=0, TQCString *dcopService=0, int *pid=0, const TQCString &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the (translated) name of the service.
   * E.g. "Web Browser"
   *
   * @param _name the name of the service
   * @param URLs if not empty these URLs will be passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByName( const TQString& _name, const TQStringList &URLs=TQStringList(),
                TQString *error=0, TQCString *dcopService=0, int *pid=0, const TQCString &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the desktop path of the service.
   * E.g. "Applications/konqueror.desktop" or "/home/user/bla/myfile.desktop"
   *
   * @param _name the path of the desktop file
   * @param URL if not empty this URL is passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByDesktopPath( const TQString& _name, const TQString &URL,
                TQString *error=0, TQCString *dcopService=0, int *pid = 0, const TQCString &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the desktop path of the service.
   * E.g. "Applications/konqueror.desktop" or "/home/user/bla/myfile.desktop"
   *
   * @param _name the path of the desktop file
   * @param URLs if not empty these URLs will be passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByDesktopPath( const TQString& _name, const TQStringList &URLs=TQStringList(),
                TQString *error=0, TQCString *dcopService=0, int *pid = 0, const TQCString &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the desktop name of the service.
   * E.g. "konqueror"
   *
   * @param _name the desktop name of the service
   * @param URL if not empty this URL is passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByDesktopName( const TQString& _name, const TQString &URL,
                TQString *error=0, TQCString *dcopService=0, int *pid = 0, const TQCString &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the desktop name of the service.
   * E.g. "konqueror"
   *
   * @param _name the desktop name of the service
   * @param URLs if not empty these URLs will be passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByDesktopName( const TQString& _name, const TQStringList &URLs=TQStringList(),
                TQString *error=0, TQCString *dcopService=0, int *pid = 0, const TQCString &startup_id = "", bool noWait = false );

  /**
   * Starts a program via tdeinit.
   *
   * program name and arguments are converted to according to the
   * local encoding and passed as is to tdeinit.
   *
   * @param name Name of the program to start
   * @param args Arguments to pass to the program
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int tdeinitExec( const TQString& name, const TQStringList &args,
                TQString *error, int *pid, const TQCString& startup_id );
  // KDE4 merge with above with startup_id = ""
  static int tdeinitExec( const TQString& name, const TQStringList &args=TQStringList(),
                TQString *error=0, int *pid = 0 );

  /**
   * Starts a program via tdeinit and wait for it to finish.
   *
   * Like tdeinitExec(), but it waits till the program is finished.
   * As such it behaves similar to the system(...) function.
   *
   * @param name Name of the program to start
   * @param args Arguments to pass to the program
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int tdeinitExecWait( const TQString& name, const TQStringList &args,
                TQString *error, int *pid, const TQCString& startup_id );
  // KDE4 merge with above with startup_id = ""
  static int tdeinitExecWait( const TQString& name, const TQStringList &args=TQStringList(),
                TQString *error=0, int *pid = 0 );

  /**
   * Returns a text for the window caption.
   *
   * This may be set by
   * "-caption", otherwise it will be equivalent to the name of the
   * executable.
   * @return the text for the window caption
   */
  TQString caption() const;

  /**
   * @deprecated
   */
  TDE_DEPRECATED TDEStyle* tdestyle() const { return 0; }

  /**
   * Builds a caption that contains the application name along with the
   * userCaption using a standard layout.
   *
   * To make a compliant caption
   * for your window, simply do: @p setCaption(kapp->makeStdCaption(yourCaption));
   *
   * @param userCaption The caption string you want to display in the
   * window caption area. Do not include the application name!
   * @param withAppName Indicates that the method shall include or ignore
   * the application name when making the caption string. You are not
   * compliant if you set this to @p false.
   * @param modified If true, a 'modified' sign will be included in the
   * returned string. This is useful when indicating that a file is
   * modified, i.e., it contains data that has not been saved.
   * @return the created caption
   */
  TQString makeStdCaption( const TQString &userCaption,
                          bool withAppName=true, bool modified=false ) const;

  /**
   * Get a file name in order to make a temporary copy of your document.
   *
   * @param pFilename The full path to the current file of your
   * document.
   * @return A new filename for auto-saving.
   */
  TQString tempSaveName( const TQString& pFilename ) const;

  /**
   * Check whether  an auto-save file exists for the document you want to
   * open.
   *
   * @param pFilename The full path to the document you want to open.
   * @param bRecover  This gets set to true if there was a recover
   * file.
   * @return The full path of the file to open.
   */
  TQString checkRecoverFile( const TQString& pFilename, bool& bRecover ) const;

#if defined(TQ_WS_X11)
  /**
   * @internal
   * Get the X11 display
   * @return the X11 Display
   */
   Display *getDisplay() { return display; }
#endif

  /**
   * @internal
   * Gets X11 composition information
   */
   void getX11RGBAInformation(Display *dpy);

  /**
   * Gets the availability of a composition manager such as kompmgr
   * Note that at least one application must have called detectCompositionManagerAvailable
   * while the current X display was active in order for this method to return valid results.
   * @see detectCompositionManagerAvailable()
   * @return whether the composition manager is enabled
   */
   static bool isCompositionManagerAvailable();

  /**
   * Detects the availability of a composition manager such as kompmgr
   * Note that calling this method will probably cause the screen to flicker.
   * @see isCompositionManagerAvailable()
   * @param force_available If set, force TDE to assume a composition manager is available
   * @param available Whether or not the composition manager is available (only used if force_available is TRUE)
   * @return whether the composition manager is enabled
   */
   bool detectCompositionManagerAvailable(bool force_available=false, bool available=true);

  /**
   * @internal
   * Opens the display
   * This can be used to initialize a TDEApplication with RGBA support like this:
   * TDEApplication app(TDEApplication::openX11RGBADisplay());
   * or
   * TDEApplication app(TDEApplication::openX11RGBADisplay(), useStyles);
   */
   static Display* openX11RGBADisplay();

  /**
   * Returns the X11 display visual
   *
   * @return A pointer to the X11 display visual
   */
   TQt::HANDLE getX11RGBAVisual(Display *dpy);

  /**
   * Returns the X11 display colormap
   *
   * @return An X11 display colormap object
   */
   TQt::HANDLE getX11RGBAColormap(Display *dpy);

  /**
   * Returns whether or not X11 composition is available
   * 
   * You must first call getX11RGBAInformation()
   * 
   * Note that getX11RGBAInformation() has already
   * been called if you used the default TDEApplication
   * constructor.
   *
   * Additionally, at least one application must have called
   * detectCompositionManagerAvailable while the current X 
   * display was active in order for this method to return
   * valid results.
   *
   * @return true if composition is available
   */
   bool isX11CompositionAvailable();

  /**
   * Enables style plugins.
   *
   * This is useful only to applications that normally
   * do not display a GUI and create the TDEApplication with
   * allowStyles set to false.
   */
  void enableStyles();

  /**
   * Disables style plugins.
   *
   * Current style plugins do not get unloaded.
   *
   * This is only useful when used in combination with enableStyles().
   */
  void disableStyles();

  /**
   *  Installs widget filter as global X11 event filter.
   *
   * The widget
   *  filter receives XEvents in its standard TQWidget::x11Event() function.
   *
   *  Warning: Only do this when absolutely necessary. An installed X11 filter
   *  can slow things down.
   **/
  void installX11EventFilter( TQWidget* filter );

  /**
   * Removes global X11 event filter previously installed by
   * installX11EventFilter().
   */
  void removeX11EventFilter( const TQWidget* filter );

  /**
   * Generates a uniform random number.
   * @return A truly unpredictable number in the range [0, RAND_MAX)
   */
  static int random();

  /**
   * Generates a random string.  It operates in the range [A-Za-z0-9]
   * @param length Generate a string of this length.
   * @return the random string
   */
  static TQString randomString(int length);

  /**
   * Adds a message type to the KIPC event mask. You can only add "system
   * messages" to the event mask. These are the messages with id < 32.
   * Messages with id >= 32 are user messages.
   * @param id The message id. See KIPC::Message.
   * @see KIPC
   * @see removeKipcEventMask()
   * @see kipcMessage()
   */
  void addKipcEventMask(int id);

  /**
   * Removes a message type from the KIPC event mask. This message will
   * not be handled anymore.
   * @param id The message id.
   * @see KIPC
   * @see addKipcEventMask()
   * @see kipcMessage()
   */
  void removeKipcEventMask(int id);

  /**
   * Returns the app startup notification identifier for this running
   * application.
   * @return the startup notification identifier
   */
  TQCString startupId() const;

  /**
   * @internal
   * Sets a new value for the application startup notification window property for newly
   * created toplevel windows. 
   * @param startup_id the startup notification identifier
   * @see TDEStartupInfo::setNewStartupId
   */
  void setStartupId( const TQCString& startup_id );

  /**
   * Updates the last user action timestamp to the given time, or to the current time,
   * if 0 is given. Do not use unless you're really sure what you're doing.
   * Consult focus stealing prevention section in tdebase/twin/README.
   * @since 3.2
   */
  void updateUserTimestamp( unsigned long time = 0 );
  
  /**
   * Returns the last user action timestamp or 0 if no user activity has taken place yet.
   * @since 3.2.3
   * @see updateuserTimestamp
   */
  unsigned long userTimestamp() const;

  /**
   * Updates the last user action timestamp in the application registered to DCOP with dcopId
   * to the given time, or to this application's user time, if 0 is given.
   * Use before causing user interaction in the remote application, e.g. invoking a dialog
   * in the application using a DCOP call.
   * Consult focus stealing prevention section in tdebase/twin/README.
   * @since 3.3
   */
  void updateRemoteUserTimestamp( const TQCString& dcopId, unsigned long time = 0 );
  
    /**
    * Returns the argument to --geometry if any, so the geometry can be set
    * wherever necessary
    * @return the geometry argument, or TQString::null if there is none
    */
  TQString geometryArgument() const;

  /**
   * Install a Qt SQL property map with entries for all KDE widgets
   * Call this in any application using KDE widgets in TQSqlForm or TQDataView.
   */
  void installKDEPropertyMap();

  /**
   * Returns whether a certain action is authorized
   * @param genericAction The name of a generic  action
   * @return true if the action is authorized
   */
  bool authorize(const TQString &genericAction);

  /**
   * Returns whether a certain TDEAction is authorized.
   *
   * @param action The name of a TDEAction action. The name is prepended
   * with "action/" before being passed to authorize()
   * @return true if the TDEAction is authorized
   */
  bool authorizeTDEAction(const char *action);

  /**
   * Returns whether a certain URL related action is authorized.
   *
   * @param action The name of the action. Known actions are
   * list (may be listed (e.g. in file selection dialog)),
   * link (may be linked to),
   * open (may open) and
   * redirect (may be redirected to)
   * @param baseURL The url where the action originates from
   * @param destURL The object of the action
   * @return true when the action is authorized, false otherwise.
   * @since 3.1
   */
  bool authorizeURLAction(const TQString &action, const KURL &baseURL, const KURL &destURL);

  /**
   * Allow a certain URL action. This can be useful if your application
   * needs to ensure access to an application specific directory that may 
   * otherwise be subject to KIOSK restrictions.
   * @param action The name of the action.
   * @param _baseURL The url where the action originates from
   * @param _destURL The object of the action
   * @since 3.2
   */
  void allowURLAction(const TQString &action, const KURL &_baseURL, const KURL &_destURL);

  /**
   * Returns whether access to a certain control module is authorized.
   *
   * @param menuId identifying the control module, e.g. tde-mouse.desktop
   * @return true if access to the module is authorized, false otherwise.
   * @since 3.2
   */
  bool authorizeControlModule(const TQString &menuId);
  
  /**
   * Returns whether access to a certain control modules is authorized.
   *
   * @param menuIds list of menu-ids of control module, 
   * an example of a menu-id is tde-mouse.desktop.
   * @return Those control modules for which access has been authorized.
   * @since 3.2
   */
  TQStringList authorizeControlModules(const TQStringList &menuIds);

  /**
   * Returns the state of the currently pressed keyboard modifiers (e.g. shift, control, etc.)
   * and mouse buttons, similarly to TQKeyEvent::state() and TQMouseEvent::state().
   * You usually should simply use the information provided by TQKeyEvent and TQMouseEvent,
   * but it can be useful to query for the status of the modifiers at another moment
   * (e.g. some KDE apps do that upon a drop event).
   * @return the keyboard modifiers and mouse buttons state
   * @since 3.4
   */
  static ButtonState keyboardMouseState();

  // Same values as ShiftMask etc. in X.h
  enum { ShiftModifier = 1<<0,
         LockModifier = 1<<1,
         ControlModifier = 1<<2,
         Modifier1 = 1<<3,
         Modifier2 = 1<<4,
         Modifier3 = 1<<5,
         Modifier4 = 1<<6,
         Modifier5 = 1<<7 };
  /**
   * @deprecated Use keyboardMouseState()
   * @since 3.1
   */
  static uint keyboardModifiers() TDE_DEPRECATED;

  /** @deprecated Same values as Button1Mask etc. in X.h */
  enum { Button1Pressed = 1<<8,
         Button2Pressed = 1<<9,
         Button3Pressed = 1<<10,
         Button4Pressed = 1<<11,
         Button5Pressed = 1<<12 };
  /**
   * @deprecated Use keyboardMouseState()
   * @since 3.1
   */
  static uint mouseState() TDE_DEPRECATED;

   /**
   * Returns the VT that the current X server is running on, or -1 if this information is unavailable.
   *
   * @since 14.0.0
   */
  static int currentX11VT();


public slots:
  /**
   * Tells TDEApplication about one more operation that should be finished
   * before the application exits. The standard behavior is to exit on the
   * "last window closed" event, but some events should outlive the last window closed
   * (e.g. a file copy for a file manager, or 'compacting folders on exit' for a mail client).
   */
  void ref();

  /**
   * Tells TDEApplication that one operation such as those described in ref() just finished.
   * The application exits if the counter is back to 0.
   */
  void deref();

protected:
  /**
   * @internal Used by KUniqueApplication
   */
  TDEApplication( bool allowStyles, bool GUIenabled, TDEInstance* _instance );

#ifdef TQ_WS_X11
  /**
   * @internal Used by KUniqueApplication
   */
  TDEApplication( Display *display, TQt::HANDLE visual, TQt::HANDLE colormap,
		  bool allowStyles, TDEInstance* _instance );

  /**
   * Used to catch X11 events
   */
  bool x11EventFilter( XEvent * );

  Display *display;
#endif
  Atom kipcCommAtom;
  int kipcEventMask;

  /// Current application object.
  static TDEApplication *KApp;
  int pArgc;

  /**
   * This method is used internally to determine which edit slots are implemented
   * by the widget that has the focus, and to invoke those slots if available.
   *
   * @param slot is the slot as returned using the TQ_SLOT() macro, for example TQ_SLOT( cut() )
   *
   * This method can be used in TDEApplication subclasses to implement application wide
   * edit actions not supported by the TDEApplication class.  For example (in your subclass):
   *
   * \code
   * void MyApplication::deselect()
   * {
   *   invokeEditSlot( TQ_SLOT( deselect() ) );
   * }
   * \endcode
   *
   * Now in your application calls to MyApplication::deselect() will call this slot on the
   * focused widget if it provides this slot.  You can combine this with TDEAction with:
   *
   * \code
   * KStdAction::deselect( static_cast<MyApplication *>( kapp ), TQ_SLOT( cut() ), actionCollection() );
   * \endcode
   *
   * @see cut()
   * @see copy()
   * @see paste()
   * @see clear()
   * @see selectAll()
   *
   * @since 3.2
   */
  void invokeEditSlot( const char *slot );

private slots:
  void dcopFailure(const TQString &);
  void dcopBlockUserInput( bool );
  void x11FilterDestroyed();
  void checkAppStartedSlot();

private:
  TQString sessionConfigName() const;
  TDEConfig* pSessionConfig; //instance specific application config object
  static DCOPClient *s_DCOPClient; // app specific application communication client
  static bool s_dcopClientNeedsPostInit;
  TQString aCaption; // the name for the window title
  bool bSessionManagement;
  struct oldPixmapType { TQPixmap a, b; };
  mutable union {
    struct {
      TQPixmap *icon, *miniIcon;
    } pm;
    char unused[sizeof(oldPixmapType)];
  } aIconPixmap; // KDE4: remove me
  TQString aIconName;
  TQString aMiniIconName;
  bool useStyles;
  TQWidget *smw;

  void init( bool GUIenabled );

  void parseCommandLine( ); // Handle KDE arguments (Using TDECmdLineArgs)

  void read_app_startup_id();

  void dcopAutoRegistration();
  void dcopClientPostInit();
  void initUrlActionRestrictions();

  bool argb_visual;
#if defined(TQ_WS_X11)
  TQt::HANDLE argb_x11_visual;
  TQt::HANDLE argb_x11_colormap;
#endif

public:
  /**
   * @internal
   */
  bool notify(TQObject *receiver, TQEvent *event);

  /**
      @internal
    */
  int xErrhandler( Display*, void* );

  /**
      @internal
    */
  int xioErrhandler( Display* );

  /**
   * @internal
   */
  void iceIOErrorHandler( _IceConn *conn );

  /**
   * @internal
   */
  static bool loadedByKdeinit;

  /**
   * @internal
   */
  static void startKdeinit();

  /**
   * Valid values for the settingsChanged signal
   */
  enum SettingsCategory { SETTINGS_MOUSE, SETTINGS_COMPLETION, SETTINGS_PATHS,
         SETTINGS_POPUPMENU, SETTINGS_QT, SETTINGS_SHORTCUTS };

  /**
   * Used to obtain the TQPalette that will be used to set the application palette.
   *
   * This is only useful for configuration modules such as krdb and should not be
   * used in normal circumstances.
   * @return the QPalette
   * @since 3.1
   */
  static TQPalette createApplicationPalette();

  /**
   * @internal
   * Raw access for use by TDM.
   */
  static TQPalette createApplicationPalette( TDEConfig *config, int contrast );

  /**
   * Installs a handler for the SIGPIPE signal. It is thrown when you write to
   * a pipe or socket that has been closed.
   * The handler is installed automatically in the constructor, but you may
   * need it if your application or component does not have a TDEApplication
   * instance.
   */
  static void installSigpipeHandler();

  /**
   * @internal
   * Whether widgets can be used. 
   *
   * @since 3.2
   */
  static bool guiEnabled();

signals:
  /**
   * Emitted when TDEApplication has changed its palette due to a KControl request.
   *
   * Normally, widgets will update their palette automatically, but you
   * should connect to this to program special behavior.
   */
  void tdedisplayPaletteChanged();

  /**
   * Emitted when TDEApplication has changed its GUI style in response to a KControl request.
   *
   * Normally, widgets will update their styles automatically (as they would
   * respond to an explicit setGUIStyle() call), but you should connect to
   * this to program special behavior.
   */
  void tdedisplayStyleChanged();

  /**
   * Emitted when TDEApplication has changed its font in response to a KControl request.
   *
   * Normally widgets will update their fonts automatically, but you should
   * connect to this to monitor global font changes, especially if you are
   * using explicit fonts.
   *
   * Note: If you derive from a TQWidget-based class, a faster method is to
   *       reimplement TQWidget::fontChange(). This is the preferred way
   *       to get informed about font updates.
   */
  void tdedisplayFontChanged();

  /**
   * Emitted when TDEApplication has changed either its GUI style, its font or its palette
   * in response to a tdedisplay request. Normally, widgets will update their styles
   * automatically, but you should connect to this to program special
   * behavior. */
  void appearanceChanged();

  /**
   * Emitted when the settings for toolbars have been changed. TDEToolBar will know what to do.
   */
  void toolbarAppearanceChanged(int);

  /**
   * Emitted when the desktop background has been changed by @p kcmdisplay.
   *
   * @param desk The desktop whose background has changed.
   */
  void backgroundChanged(int desk);

  /**
   * Emitted when the global settings have been changed - see TDEGlobalSettings
   * TDEApplication takes care of calling reparseConfiguration on TDEGlobal::config()
   * so that applications/classes using this only have to re-read the configuration
   * @param category the category among the enum above
   */
  void settingsChanged(int category);

  /**
   * Emitted when the global icon settings have been changed.
   * @param group the new group
   */
  void iconChanged(int group);

  /**
   * Emitted when a KIPC user message has been received.
   * @param id the message id
   * @param data the data
   * @see KIPC
   * @see KIPC::Message
   * @see addKipcEventMask
   * @see removeKipcEventMask
   */
  void kipcMessage(int id, int data);

  /**
      Session management asks you to save the state of your application.

     This signal is provided for compatibility only. For new
     applications, simply use TDEMainWindow. By reimplementing 
     TDEMainWindow::queryClose(), TDEMainWindow::saveProperties() and
 TDEMainWindow::readProperties() you can simply handle session
     management for applications with multiple toplevel windows.

     For purposes without TDEMainWindow, create an instance of
     KSessionManaged and reimplement the functions 
     KSessionManaged::commitData() and/or 
     KSessionManaged::saveState()

     If you still want to use this signal, here is what you should do:

     Connect to this signal in order to save your data. Do NOT
     manipulate the UI in that slot, it is blocked by the session
     manager.

     Use the sessionConfig() TDEConfig object to store all your
     instance specific data.

     Do not do any closing at this point! The user may still select
     Cancel  wanting to continue working with your
     application. Cleanups could be done after shutDown() (see
     the following).

  */
  void saveYourself();

  /** Your application is killed. Either by your program itself,
      @p xkill or (the usual case) by KDE's logout.

      The signal is particularly useful if your application has to do some
      last-second cleanups. Note that no user interaction is possible at
      this state.
   */
  void shutDown();

  /**
   * @internal
   * Used to notify TDEIconLoader objects that they need to reload.
   */
  void updateIconLoaders();

  /**
   * @internal
   * Used to send TDEGlobalAccel objects a new keypress from physical hotkeys.
   */
  void coreFakeKeyPress(unsigned int keyCode);

private:
  void propagateSettings(SettingsCategory category);
  void tdedisplaySetPalette();
  void tdedisplaySetStyle();
  void tdedisplaySetFont();
  void applyGUIStyle();
  static void sigpipeHandler(int);

  int captionLayout;

  TDEApplication(const TDEApplication&);
  TDEApplication& operator=(const TDEApplication&);
protected:
  virtual void virtual_hook( int id, void* data );
private:
  TDEApplicationPrivate* d;
};


/**
 * \relates TDEGlobal
 * Check, if a file may be accessed in a given mode.
 * This is a wrapper around the access() system call.
 * checkAccess() calls access() with the given parameters.
 * If this is OK, checkAccess() returns true. If not, and W_OK
 * is part of mode, it is checked if there is write access to
 * the directory. If yes, checkAccess() returns true.
 * In all other cases checkAccess() returns false.
 *
 * Other than access() this function EXPLICITLY ignores non-existant
 * files if checking for write access.
 *
 * @param pathname The full path of the file you want to test
 * @param mode     The access mode, as in the access() system call.
 * @return Whether the access is allowed, true = Access allowed
 */
TDECORE_EXPORT bool checkAccess(const TQString& pathname, int mode);

class KSessionManagedPrivate;

/**
   Provides highlevel access to session management on a per-object
   base.

   KSessionManaged makes it possible to provide implementations for
 TQApplication::commitData() and TQApplication::saveState(), without
   subclassing TDEApplication. TDEMainWindow internally makes use of this.

   You don't need to do anything with this class when using
   TDEMainWindow. Instead, use TDEMainWindow::saveProperties(),
 TDEMainWindow::readProperties(), TDEMainWindow::queryClose(),
 TDEMainWindow::queryExit() and friends.

  @short Highlevel access to session management.
  @author Matthias Ettrich <ettrich@kde.org>
 */
class TDECORE_EXPORT KSessionManaged
{
public:
  KSessionManaged();
  virtual ~KSessionManaged();

    /**
       See TQApplication::saveState() for documentation.

       This function is just a convenience version to avoid subclassing TDEApplication.

       Return true to indicate a successful state save or false to
       indicate a problem and to halt the shutdown process (will
       implicitly call sm.cancel() ).
     */
  virtual bool saveState( TQSessionManager& sm );
    /**
       See TQApplication::commitData() for documentation.

       This function is just a convenience version to avoid subclassing TDEApplication.

       Return true to indicate a successful commit of data or false to
       indicate a problem and to halt the shutdown process (will
       implicitly call sm.cancel() ).
     */
  virtual bool commitData( TQSessionManager& sm );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KSessionManagedPrivate *d;
};


#endif

