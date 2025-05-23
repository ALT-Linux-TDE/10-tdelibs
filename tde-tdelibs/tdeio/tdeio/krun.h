/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2006 David Faure <faure@kde.org>

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

#ifndef __k_run_h__
#define __k_run_h__

#include <sys/stat.h>
#include <sys/types.h>

#include <tqobject.h>
#include <tqtimer.h>
#include <tqstring.h>
#include <kurl.h>
#include <tdestartupinfo.h>

class TDEProcess;
class KService;
namespace TDEIO {
   class Job;
   class StatJob;
}

/**
 * To open files with their associated applications in KDE, use KRun.
 *
 * It can execute any desktop entry, as well as any file, using
 * the default application or another application "bound" to the file type
 * (or URL protocol).
 *
 * In that example, the mimetype of the file is not known by the application,
 * so a KRun instance must be created. It will determine the mimetype by itself.
 * If the mimetype is known, or if you even know the service (application) to
 * use for this file, use one of the static methods.
 *
 * By default KRun uses auto deletion. It causes the KRun instance to delete
 * itself when the it finished its task. If you allocate the KRun
 * object on the stack you must disable auto deletion, otherwise it will crash.
 *
 * @short Opens files with their associated applications in KDE
 */
class TDEIO_EXPORT KRun : public TQObject
{
  TQ_OBJECT
public:
  /**
   * Create a KRun object to run the preferred application for a file/URL.
   * KRun will first determine the type of the file, and will then
   * run the associated application.
   *
   * @param url the URL of the file or directory to 'run'
   *
   * @param mode The @p st_mode field of <tt>struct stat</tt>. If
   *        you don't know this set it to 0.
   *
   * @param isLocalFile
   *        If this parameter is set to @p false then @p url is
   *        examined to find out whether it is a local URL or
   *        not. This flag is just used to improve speed, since the
   *        function KURL::isLocalFile is a bit slow.
   *
   * @param showProgressInfo
   *        Whether to show progress information when determining the
   *        type of the file (i.e. when using TDEIO::stat and TDEIO::mimetype)
   *        Before you set this to false to avoid a dialog box, think about
   *        a very slow FTP server...
   *        It is always better to provide progress info in such cases.
   */
  KRun( const KURL& url, mode_t mode = 0,
	bool isLocalFile = false, bool showProgressInfo = true );

  /**
   * BIC: Combine with the above ctor for KDE 4.0.
   * @param window
   *        The top-level widget of the app that invoked this object.
   *        It is used to make sure private information like passwords
   *        are properly handled per application.
   * @param url the URL of the file or directory to 'run'
   *
   * @param mode The @p st_mode field of <tt>struct stat</tt>. If
   *        you don't know this set it to 0.
   *
   * @param isLocalFile
   *        If this parameter is set to @p false then @p url is
   *        examined to find out whether it is a local URL or
   *        not. This flag is just used to improve speed, since the
   *        function KURL::isLocalFile is a bit slow.
   *
   * @param showProgressInfo
   *        Whether to show progress information when determining the
   *        type of the file (i.e. when using TDEIO::stat and TDEIO::mimetype)
   *        Before you set this to false to avoid a dialog box, think about
   *        a very slow FTP server...
   *        It is always better to provide progress info in such cases.
   */
  KRun( const KURL& url, TQWidget* window, mode_t mode = 0,
	bool isLocalFile = false, bool showProgressInfo = true );
  KRun( const KURL& url, TQWidget* window, const TQCString& asn, mode_t mode = 0,
	bool isLocalFile = false, bool showProgressInfo = true );

  /**
   * Destructor. Don't call it yourself, since a KRun object auto-deletes
   * itself.
   */
  virtual ~KRun();

  /**
   * Abort this KRun. This kills any jobs launched by it,
   * and leads to deletion if auto-deletion is on.
   * This is much safer than deleting the KRun (in case it's
   * currently showing an error dialog box, for instance)
   */
  void abort();

  /**
   * Returns true if the KRun instance has an error.
   * @return true when an error occurred
   * @see error()
   */
  bool hasError() const { return m_bFault; }

  /**
   * Returns true if the KRun instance has finished.
   * @return true if the KRun instance has finished
   * @see finished()
   */
  bool hasFinished() const { return m_bFinished; }

  /**
   * Checks whether auto delete is activated.
   * Auto-deletion causes the KRun instance to delete itself
   * when it finished its task.
   * By default auto deletion is on.
   * @return true if auto deletion is on, false otherwise
   */
  bool autoDelete() const { return m_bAutoDelete; }

  /**
   * Enables or disabled auto deletion.
   * Auto deletion causes the KRun instance to delete itself
   * when it finished its task. If you allocate the KRun
   * object on the stack you must disable auto deletion.
   * By default auto deletion is on.
   * @param b true to enable auto deletion, false to disable
   */
  void setAutoDelete(bool b) { m_bAutoDelete = b; }

  /**
   * Set the preferred service for opening this URL, after
   * its mimetype will have been found by KRun. IMPORTANT: the service is
   * only used if its configuration says it can handle this mimetype.
   * This is used for instance for the X-TDE-LastOpenedWith key, for
   * the recent documents list.
   * @param desktopEntryName the desktopEntryName of the service, e.g. "kate".
   */
  void setPreferredService( const TQString& desktopEntryName );

  /**
   * Sets whether executables, .desktop files or shell scripts should
   * be run by KRun. This is enabled by default.
   * @param b whether to run executable files or not.
   * @see isExecutable()
   * @since 3.2
   */
  void setRunExecutables(bool b);

  /**
   * Sets whether the external webbrowser setting should be honoured.
   * This is enabled by default.
   * This should only be disabled in webbrowser applications.
   * @param b whether to enable the external browser or not.
   * @since 3.4
   */
  void setEnableExternalBrowser(bool b);

  /**
   * Sets the file name to use in the case of downloading the file to a tempfile
   * in order to give to a non-url-aware application. Some apps rely on the extension
   * to determine the mimetype of the file. Usually the file name comes from the URL,
   * but in the case of the HTTP Content-Disposition header, we need to override the
   * file name.
   * @since 3.5.3
   */
  void setSuggestedFileName( const TQString& fileName );

  /**
   * Open a list of URLs with a certain service (application).
   *
   * @param _service the service to run
   * @param _urls the list of URLs, can be empty (app launched
   *        without argument)
   * @param window The top-level widget of the app that invoked this object.
   * @param tempFiles if true and _urls are local files, they will be deleted
   *        when the application exits.
   * @return the process id, or 0 on error
   * @since 3.5.2
   */
  static pid_t run( const KService& _service, const KURL::List& _urls, TQWidget* window, bool tempFiles = false );
  static pid_t run( const KService& _service, const KURL::List& _urls, TQWidget* window,
      const TQCString& asn, bool tempFiles = false );
  /**
   * Open a list of URLs with a certain service (application).
   *
   * @param _service the service to run
   * @param _urls the list of URLs, can be empty (app launched
   *        without argument)
   * @param tempFiles if true and _urls are local files, they will be deleted
   *        when the application exits.
   * @return the process id, or 0 on error
   */
  // BIC merge second overload with first one, using tempFiles=false
  static pid_t run( const KService& _service, const KURL::List& _urls, bool tempFiles );
  static pid_t run( const KService& _service, const KURL::List& _urls );
  /// @since 3.5.3
  /// @internal
  static pid_t run( const KService& _service, const KURL::List& _urls, TQWidget* window, bool tempFiles, const TQString& suggestedFileName );
  static pid_t run( const KService& _service, const KURL::List& _urls, TQWidget* window,
      const TQCString& asn, bool tempFiles, const TQString& suggestedFileName );

  /**
   * Open a list of URLs with.
   *
   * @param _exec the name of the executable, for example
   *        "/usr/bin/netscape".
   * @param _urls  the list of URLs to open, can be empty (app launched without argument)
   * @param _name the logical name of the application, for example
   *        "Netscape 4.06".
   * @param _icon the icon which should be used by the application.
   * @param _obsolete1 Do not use!
   * @param _obsolete2 Do not use!
   * @return the process id, or 0 on error
   */
  static pid_t run( const TQString& _exec, const KURL::List& _urls,
		   const TQString& _name = TQString::null,
		   const TQString& _icon = TQString::null,
		   const TQString& _obsolete1 = TQString::null,
		   const TQString& _obsolete2 = TQString::null );

  /**
   * Open the given URL.
   *
   * This function is used after the mime type
   * is found out. It will search for all services which can handle
   * the mime type and call run() afterwards.
   * @param _url the URL to open
   * @param _mimetype the mime type of the resource
   * @param tempFile if true and _url is a local file, it will be deleted
   *        when the launched application exits.
   * @param runExecutables if false then local .desktop files,
   *        executables and shell scripts will not be run.
   *        See also isExecutable().
   * @return the process id, or 0 on error
   */
  // BIC Merge second overload with first one using runExecutables=true, and
  // merge third overload with first one as well using tempFiles=false and
  // runExecutables=true
  static pid_t runURL( const KURL& _url, const TQString& _mimetype, bool tempFile, bool runExecutables);
  static pid_t runURL( const KURL& _url, const TQString& _mimetype, bool tempFile);
  static pid_t runURL( const KURL& _url, const TQString& _mimetype );
  /// @since 3.5.3
  /// @internal
  static pid_t runURL( const KURL& _url, const TQString& _mimetype, TQWidget* window, const TQCString& asn, bool tempFile, bool runExecutables, const TQString& suggestedFileName );
  static pid_t runURL( const KURL& _url, const TQString& _mimetype, bool tempFile, bool runExecutables, const TQString& suggestedFileName );

  /**
   * Run the given shell command and notifies kicker of the starting
   * of the application. If the program to be called doesn't exist,
   * an error box will be displayed.
   *
   * Use only when you know the full command line. Otherwise use the other
   * static methods, or KRun's constructor.
   *
   * @p _cmd must be a shell command. You must not append "&"
   * to it, since the function will do that for you.
   *
   * @return PID of running command, 0 if it could not be started, 0 - (PID
   * of running command) if command was unsafe for map notification.
   */
  static pid_t runCommand( TQString cmd );

  /**
   * Same as the other runCommand(), but it also takes the name of the
   * binary, to display an error message in case it couldn't find it.
   *
   * @param cmd must be a shell command. You must not append "&"
   * to it, since the function will do that for you.
   * @param execName the name of the executable
   * @param icon icon for app starting notification
   * @return PID of running command, 0 if it could not be started, 0 - (PID
   * of running command) if command was unsafe for map notification.
   */
  static pid_t runCommand( const TQString& cmd, const TQString & execName, const TQString & icon );
  static pid_t runCommand( const TQString& cmd, const TQString & execName, const TQString & icon,
      TQWidget* window, const TQCString& asn );

  /**
   * Display the Open-With dialog for those URLs, and run the chosen application.
   * @param lst the list of applications to run
   * @param tempFiles if true and lst are local files, they will be deleted
   *        when the application exits.
   * @return false if the dialog was canceled
   */
  // BIC merge second overload with first one, using tempFiles=false
  static bool displayOpenWithDialog( const KURL::List& lst, bool tempFiles );
  static bool displayOpenWithDialog( const KURL::List& lst );
  /// @since 3.5.3
  /// @internal
  static bool displayOpenWithDialog( const KURL::List& lst, bool tempFiles, const TQString& suggestedFileName );

  /**
   * Quotes a string for the shell.
   * @param _str the string to quote. The quoted string will be written here
   */
  static void shellQuote( TQString &_str );

  /**
   * Processes a Exec= line as found in .desktop files.
   * @param _service the service to extract information from.
   * @param _urls The urls the service should open.
   * @param has_shell If true, the arguments are going to be fed into a
   *        shell e.g by using system().
   *        If false, the arguments are going to be fed into a exec() kind
   *        call.
   *        If the arguments are intended for an exec() kind of call and
   *        the Exec line contains shell commands then "/bin/sh -c" is added.
   * @param tempFiles if true and _urls are local files, they will be deleted
   *        when the application exits.
   * @return a list of arguments suitable for either system() or exec().
   */
  static TQStringList processDesktopExec(const KService &_service, const KURL::List &_urls, bool has_shell, bool tempFiles);
  static TQStringList processDesktopExec(const KService &_service, const KURL::List &_urls, bool has_shell);
  /// @since 3.5.3
  /// @internal
  static TQStringList processDesktopExec(const KService &_service, const KURL::List &_urls, bool has_shell, bool tempFiles, const TQString& suggestedFileName);

  /**
   * Given a full command line (e.g. the Exec= line from a .desktop file),
   * extract the name of the binary being run.
   * @param execLine the full command line
   * @param removePath if true, remove a (relative or absolute) path. E.g. /usr/bin/ls becomes ls.
   * @return the name of the binary to run
   * @since 3.1
   */
  static TQString binaryName( const TQString & execLine, bool removePath );

  /**
   * Returns whether @p serviceType refers to an executable program instead
   * of a data file.
   * @since 3.2
   */
  static bool isExecutable( const TQString& serviceType );

  /**
   * Returns wether the @p url of @p mimetype is executable.
   * To be executable the file must pass the following rules:
   * -# Must reside on the local filesystem.
   * -# Must be marked as executable for the user by the filesystem.
   * -# The mime type must inherit application/x-executable or application/x-executable-script.
   * To allow a script to run when the above rules are satisfied add the entry
   * @code
   * X-TDE-IsAlso=application/x-executable-script
   * @endcode
   * to the mimetype's desktop file.
   * @since 3.3
   */
   static bool isExecutableFile( const KURL& url, const TQString &mimetype );

   /**
    * @internal
    * @since 3.4
    */
    static bool checkStartupNotify( const TQString& binName, const KService* service, bool* silent_arg, TQCString* wmclass_arg );

signals:
  /**
   * Emitted when the operation finished.
   * @see hasFinished()
   */
  void finished();
  /**
   * Emitted when the operation had an error.
   * @see hasError()
   */
  void error();

protected slots:
  void slotTimeout();
  void slotScanFinished( TDEIO::Job * );
  void slotScanMimeType( TDEIO::Job *, const TQString &type );
  virtual void slotStatResult( TDEIO::Job * );

protected:
  virtual void init();

  virtual void scanFile();

  /**
   * Called if the mimetype has been detected. The function checks
   * whether the document and appends the gzip protocol to the
   * URL. Otherwise runURL is called to finish the job.
   */
  virtual void foundMimeType( const TQString& _type );

  virtual void killJob();

  KURL m_strURL;
  bool m_bFault;
  bool m_bAutoDelete;
  bool m_bProgressInfo;
  bool m_bFinished;
  TDEIO::Job * m_job;
  TQTimer m_timer;

  /**
   * Used to indicate that the next action is to scan the file.
   * This action is invoked from slotTimeout.
   */
  bool m_bScanFile;
  bool m_bIsDirectory;

  /**
   * USed to indicate that the next action is to initialize.
   * This action is invoked from slotTimeout
   */
  bool m_bInit;

  bool m_bIsLocalFile;
  mode_t m_mode;

protected:
  virtual void virtual_hook( int id, void* data );

private:
  void init (const KURL& url, TQWidget* window, const TQCString& asn, mode_t mode,
             bool isLocalFile, bool showProgressInfo);
private:
  class KRunPrivate;
  KRunPrivate *d;
};

#ifndef KDE_NO_COMPAT
/**
 * @deprecated. Kept for source compatibility, does nothing nowadays.
 * Do not use in new source.
 * KRun can open the openwith dialog directly now.
 * Use KRun::displayOpenWithDialog() if you were using KOpenWithHandler directly.
 */
class TDEIO_EXPORT_DEPRECATED KOpenWithHandler
{
public:
  KOpenWithHandler() {}
  static bool exists() { return true; }
};
#endif

/**
 * @internal
 * This class watches a process launched by KRun.
 * It sends a notification when the process exits (for the taskbar)
 * and it will show an error message if necessary (e.g. "program not found").
 */
class TDEIO_EXPORT TDEProcessRunner : public TQObject
{
  TQ_OBJECT

  public:

    static pid_t run(TDEProcess *, const TQString & binName);
#ifdef TQ_WS_X11 // We don't have TDEStartupInfo in Qt/Embedded
    static pid_t run(TDEProcess *, const TQString & binName, const TDEStartupInfoId& id );
#endif

    virtual ~TDEProcessRunner();

    pid_t pid() const;

  protected slots:

    void slotProcessExited(TDEProcess *);

  private:

    TDEProcessRunner(TDEProcess *, const TQString & binName);
#ifdef TQ_WS_X11 // We don't have TDEStartupInfo in Qt/Embedded
    TDEProcessRunner(TDEProcess *, const TQString & binName, const TDEStartupInfoId& id );
#endif
    TDEProcessRunner();

    TDEProcess * process_;
    TQString binName;
#ifdef TQ_WS_X11 // We don't have TDEStartupInfo in Qt/Embedded
    TDEStartupInfoId id_;
#endif
};

#endif
