/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
                      David Faure <faure@kde.org>

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

#ifndef __tdeparts_browserextension_h__
#define __tdeparts_browserextension_h__

#include <sys/types.h>

#include <tqpoint.h>
#include <tqptrlist.h>
#include <tqdatastream.h>
#include <tqstringlist.h>
#include <tqpair.h>

#include <tdeparts/part.h>
#include <tdeparts/event.h>

class KFileItem;
typedef TQPtrList<KFileItem> KFileItemList;
class TQString;

namespace KParts {

class BrowserInterface;

struct URLArgsPrivate;

/**
 * URLArgs is a set of arguments bundled into a structure,
 * to allow specifying how a URL should be opened by openURL().
 * In other words, this is like arguments to openURL(), but without
 * have to change the signature of openURL() (since openURL is a
 * generic KParts method).
 * The parts (with a browser extension) who care about urlargs will
 * use those arguments, others will ignore them.
 *
 * This can also be used the other way round, when a part asks
 * for a URL to be opened (with openURLRequest or createNewWindow).
 */
struct TDEPARTS_EXPORT URLArgs
{
  URLArgs();
  URLArgs( const URLArgs &args );
  URLArgs &operator=( const URLArgs &args);

  URLArgs( bool reload, int xOffset, int yOffset, const TQString &serviceType = TQString::null );
  virtual ~URLArgs();

  /**
   * This buffer can be used by the part to save and restore its contents.
   * See TDEHTMLPart for instance.
   */
  TQStringList docState;

  /**
   * @p reload is set when the cache shouldn't be used (forced reload).
   */
  bool reload;
  /**
   * @p xOffset is the horizontal scrolling of the part's widget
   * (in case it's a scrollview). This is saved into the history
   * and restored when going back in the history.
   */
  int xOffset;
  /**
   * @p yOffset vertical scrolling position, xOffset.
   */
  int yOffset;
  /**
   * The servicetype (usually mimetype) to use when opening the next URL.
   */
  TQString serviceType;

  /**
   * TDEHTML-specific field, contents of the HTTP POST data.
   */
  TQByteArray postData;

  /**
   * TDEHTML-specific field, header defining the type of the POST data.
   */
  void setContentType( const TQString & contentType );
  /**
   * TDEHTML-specific field, header defining the type of the POST data.
   */
  TQString contentType() const;
  /**
   * TDEHTML-specific field, whether to do a POST instead of a GET,
   * for the next openURL.
   */
  void setDoPost( bool enable );

  /**
   * TDEHTML-specific field, whether to do a POST instead of a GET,
   * for the next openURL.
   */
  bool doPost() const;

  /**
   * Whether to lock the history when opening the next URL.
   * This is used during e.g. a redirection, to avoid a new entry
   * in the history.
   */
  void setLockHistory( bool lock );
  bool lockHistory() const;

  /**
   * Whether the URL should be opened in a new tab instead in a new window.
   */
  void setNewTab( bool newTab );
  bool newTab() const;

  /**
   * Meta-data to associate with the next TDEIO operation
   * @see TDEIO::TransferJob etc.
   */
  TQMap<TQString, TQString> &metaData();

  /**
   * The frame in which to open the URL. TDEHTML/Konqueror-specific.
   */
  TQString frameName;

  /**
   * If true, the part who asks for a URL to be opened can be 'trusted'
   * to execute applications. For instance, the directory views can be
   * 'trusted' whereas HTML pages are not trusted in that respect.
   */
  bool trustedSource;

  /**
   * @return true if the request was a result of a META refresh/redirect request or
   * HTTP redirect.
   */
  bool redirectedRequest () const;

  /**
   * Set the redirect flag to indicate URL is a result of either a META redirect
   * or HTTP redirect.
   *
   * @param redirected
   */
  void setRedirectedRequest(bool redirected);

  /**
   * Set whether the URL specifies to be opened in a new window
   * @since 3.4
   */
  void setForcesNewWindow( bool forcesNewWindow );

  /**
   * Whether the URL specifies to be opened in a new window
   * @since 3.4
   */
  bool forcesNewWindow() const;

  URLArgsPrivate *d;
};

struct WindowArgsPrivate;

/**
 * The WindowArgs are used to specify arguments to the "create new window"
 * call (see the createNewWindow variant that uses WindowArgs).
 * The primary reason for this is the javascript window.open function.
 */
struct TDEPARTS_EXPORT WindowArgs
{
    WindowArgs();
    ~WindowArgs();
    WindowArgs( const WindowArgs &args );
    WindowArgs &operator=( const WindowArgs &args );
    WindowArgs( const TQRect &_geometry, bool _fullscreen, bool _menuBarVisible,
                bool _toolBarsVisible, bool _statusBarVisible, bool _resizable );
    WindowArgs( int _x, int _y, int _width, int _height, bool _fullscreen,
                bool _menuBarVisible, bool _toolBarsVisible,
                bool _statusBarVisible, bool _resizable );

    // Position
    int x;
    int y;
    // Size
    int width;
    int height;
    bool fullscreen; //defaults to false
    bool menuBarVisible; //defaults to true
    bool toolBarsVisible; //defaults to true
    bool statusBarVisible; //defaults to true
    bool resizable; //defaults to true

    bool lowerWindow; //defaults to false
    bool scrollBarsVisible; //defaults to true

    WindowArgsPrivate *d; // don't use before KDE4, many KDE-3.x didn't have an explicit destructor
};

/**
 * The KParts::OpenURLEvent event informs that a given part has opened a given URL.
 * Applications can use this event to send this information to interested plugins.
 *
 * The event should be sent before opening the URL in the part, so that the plugins
 * can use part()->url() to get the old URL.
 */
class TDEPARTS_EXPORT OpenURLEvent : public Event
{
public:
  OpenURLEvent( ReadOnlyPart *part, const KURL &url, const URLArgs &args = URLArgs() );
  virtual ~OpenURLEvent();

  ReadOnlyPart *part() const { return m_part; }
  KURL url() const { return m_url; }
  URLArgs args() const { return m_args; }

  static bool test( const TQEvent *event ) { return Event::test( event, s_strOpenURLEvent ); }

private:
  static const char *s_strOpenURLEvent;
  ReadOnlyPart *m_part;
  KURL m_url;
  URLArgs m_args;

  class OpenURLEventPrivate;
  OpenURLEventPrivate *d;
};

class BrowserExtensionPrivate;

 /**
  * The Browser Extension is an extension (yes, no kidding) to
  * KParts::ReadOnlyPart, which allows a better integration of parts
  * with browsers (in particular Konqueror).
  * Remember that ReadOnlyPart only has openURL(KURL), with no other settings.
  * For full-fledged browsing, we need much more than that, including
  * many arguments about how to open this URL (see URLArgs), allowing
  * parts to save and restore their data into the back/forward history,
  * allowing parts to control the location bar URL, to requests URLs
  * to be opened by the hosting browser, etc.
  *
  * The part developer needs to define its own class derived from BrowserExtension,
  * to implement the virtual methods [and the standard-actions slots, see below].
  *
  * The way to associate the BrowserExtension with the part is to simply
  * create the BrowserExtension as a child of the part (in TQObject's terms).
  * The hosting application will look for it automatically.
  *
  * Another aspect of the browser integration is that a set of standard
  * actions are provided by the browser, but implemented by the part
  * (for the actions it supports).
  *
  * The following standard actions are defined by the host of the view :
  *
  * [selection-dependent actions]
  * @li @p cut : Copy selected items to clipboard and store 'not cut' in clipboard.
  * @li @p copy : Copy selected items to clipboard and store 'cut' in clipboard.
  * @li @p paste : Paste clipboard into view URL.
  * @li @p pasteTo(const KURL &) : Paste clipboard into given URL.
  * @li @p rename : Rename item in place.
  * @li @p trash : Move selected items to trash.
  * @li @p del : Delete selected items (couldn't call it delete!).
  * @li @p shred : Shred selected items (secure deletion) - DEPRECATED.
  * @li @p properties : Show file/document properties.
  * @li @p editMimeType : show file/document's mimetype properties.
  * @li @p searchProvider : Lookup selected text at default search provider
  *
  * [normal actions]
  * @li @p print : Print :-)
  * @li @p reparseConfiguration : Re-read configuration and apply it.
  * @li @p refreshMimeTypes : If the view uses mimetypes it should re-determine them.
  *
  *
  * The view defines a slot with the name of the action in order to implement the action.
  * The browser will detect the slot automatically and connect its action to it when
  * appropriate (i.e. when the view is active).
  *
  *
  * The selection-dependent actions are disabled by default and the view should
  * enable them when the selection changes, emitting enableAction().
  *
  * The normal actions do not depend on the selection.
  * You need to enable 'print' when printing is possible - you can even do that
  * in the constructor.
  *
  * A special case is the configuration slots, not connected to any action directly,
  * and having parameters.
  *
  * [configuration slot]
  * @li @p setSaveViewPropertiesLocally( bool ): If @p true, view properties are saved into .directory
  *                                       otherwise, they are saved globally.
  * @li @p disableScrolling: no scrollbars
  */
class TDEPARTS_EXPORT BrowserExtension : public TQObject
{
  TQ_OBJECT
  TQ_PROPERTY( bool urlDropHandling READ isURLDropHandlingEnabled WRITE setURLDropHandlingEnabled )
public:
  /**
   * Constructor
   *
   * @param parent The KParts::ReadOnlyPart that this extension ... "extends" :)
   * @param name An optional name for the extension.
   */
  BrowserExtension( KParts::ReadOnlyPart *parent,
                    const char *name = 0L );


  virtual ~BrowserExtension();

  typedef uint PopupFlags;

  /**
   * Set of flags passed via the popupMenu signal, to ask for some items in the popup menu.
   * DefaultPopupItems: default value, no additional menu item
   * ShowNavigationItems: show "back" and "forward" (usually done when clicking the background of the view, but not an item)
   * ShowUp: show "up" (same thing, but not over e.g. HTTP). Requires ShowNavigationItems.
   * ShowReload: show "reload" (usually done when clicking the background of the view, but not an item)
   * ShowBookmark: show "add to bookmarks" (usually not done on the local filesystem)
   * ShowCreateDirectory: show "create directory" (usually only done on the background of the view, or
   *                      in hierarchical views like directory trees, where the new dir would be visible)
   * ShowTextSelectionItems: set when selecting text, for a popup that only contains text-related items.
   * NoDeletion: deletion, trashing and renaming not allowed (e.g. parent dir not writeable).
   *            (this is only needed if the protocol itself supports deletion, unlike e.g. HTTP)
   *
   * KDE4 TODO: add IsLink flag, for "Bookmark This Link" and linkactions merging group.
   *                    [currently it depends on which signal is emitted]
   *            add ShowURLOperation flags for copy,cut,paste,rename,trash,del [same thing]
   */
  enum { DefaultPopupItems=0x0000, ShowNavigationItems=0x0001,
         ShowUp=0x0002, ShowReload=0x0004, ShowBookmark=0x0008,
         ShowCreateDirectory=0x0010, ShowTextSelectionItems=0x0020,
         NoDeletion=0x0040 ///< @since 3.4
       };


  /**
   * Set the parameters to use for opening the next URL.
   * This is called by the "hosting" application, to pass parameters to the part.
   * @see URLArgs
   */
  virtual void setURLArgs( const URLArgs &args );

  /**
   * Retrieve the set of parameters to use for opening the URL
   * (this must be called from openURL() in the part).
   * @see URLArgs
   */
  URLArgs urlArgs() const;

  /**
   * Returns the current x offset.
   *
   * For a scrollview, implement this using contentsX().
   */
  virtual int xOffset();
  /**
   * Returns the current y offset.
   *
   * For a scrollview, implement this using contentsY().
   */
  virtual int yOffset();

  /**
   * Used by the browser to save the current state of the view
   * (in order to restore it if going back in navigation).
   *
   * If you want to save additional properties, reimplement it
   * but don't forget to call the parent method (probably first).
   */
  virtual void saveState( TQDataStream &stream );

  /**
   * Used by the browser to restore the view in the state
   * it was when we left it.
   *
   * If you saved additional properties, reimplement it
   * but don't forget to call the parent method (probably first).
   */
  virtual void restoreState( TQDataStream &stream );

  /**
   * Returns whether url drop handling is enabled.
   * See setURLDropHandlingEnabled for more information about this
   * property.
   */
  bool isURLDropHandlingEnabled() const;

  /**
   * Enables or disables url drop handling. URL drop handling is a property
   * describing whether the hosting shell component is allowed to install an
   * event filter on the part's widget, to listen for URI drop events.
   * Set it to true if you are exporting a BrowserExtension implementation and
   * do not provide any special URI drop handling. If set to false you can be
   * sure to receive all those URI drop events unfiltered. Also note that the
   * implementation as of Konqueror installs the event filter only on the part's
   * widget itself, not on child widgets.
   */
  void setURLDropHandlingEnabled( bool enable );

  void setBrowserInterface( BrowserInterface *impl );
  BrowserInterface *browserInterface() const;

  /**
   * @return the status (enabled/disabled) of an action.
   * When the enableAction signal is emitted, the browserextension
   * stores the status of the action internally, so that it's possible
   * to query later for the status of the action, using this method.
   */
  bool isActionEnabled( const char * name ) const;

  /**
   * @return the text of an action, if it was set explicitely by the part.
   * When the setActionText signal is emitted, the browserextension
   * stores the text of the action internally, so that it's possible
   * to query later for the text of the action, using this method.
   * @since 3.5
   */
  TQString actionText( const char * name ) const;

  typedef TQMap<TQCString,TQCString> ActionSlotMap;
  /**
   * Returns a map containing the action names as keys and corresponding
   * TQ_SLOT()'ified method names as data entries.
   *
   * This is very useful for
   * the host component, when connecting the own signals with the
   * extension's slots.
   * Basically you iterate over the map, check if the extension implements
   * the slot and connect to the slot using the data value of your map
   * iterator.
   * Checking if the extension implements a certain slot can be done like this:
   *
   * \code
   *   extension->metaObject()->slotNames().contains( actionName + "()" )
   * \endcode
   *
   * (note that @p actionName is the iterator's key value if already
   *  iterating over the action slot map, returned by this method)
   *
   * Connecting to the slot can be done like this:
   *
   * \code
   *   connect( yourObject, TQ_SIGNAL( yourSignal() ),
   *            extension, mapIterator.data() )
   * \endcode
   *
   * (where "mapIterator" is your TQMap<TQCString,TQCString> iterator)
   */
  static ActionSlotMap actionSlotMap();

  /**
   * @return a pointer to the static action-slot map. Preferred method to get it.
   * The map is created if it doesn't exist yet
   */
  static ActionSlotMap * actionSlotMapPtr();

  /**
   * Queries @p obj for a child object which inherits from this
   * BrowserExtension class. Convenience method.
   */
  static BrowserExtension *childObject( TQObject *obj );

  /**
   * Asks the hosting browser to perform a paste (using openURLRequestDelayed)
   * @since 3.2
   */
  void pasteRequest();

// KDE invents support for public signals...
#undef signals
#define signals public
signals:
#undef signals
#define signals protected
  /**
   * Enables or disable a standard action held by the browser.
   *
   * See class documentation for the list of standard actions.
   */
  void enableAction( const char * name, bool enabled );

  /**
   * Change the text of a standard action held by the browser.
   * This can be used to change "Paste" into "Paste Image" for instance.
   *
   * See class documentation for the list of standard actions.
   * @since 3.5
   */
  void setActionText( const char * name, const TQString& text );

  /**
   * Asks the host (browser) to open @p url.
   * To set a reload, the x and y offsets, the service type etc., fill in the
   * appropriate fields in the @p args structure.
   * Hosts should not connect to this signal but to openURLRequestDelayed.
   */
  void openURLRequest( const KURL &url, const KParts::URLArgs &args = KParts::URLArgs() );

  /**
   * This signal is emitted when openURLRequest is called, after a 0-seconds timer.
   * This allows the caller to terminate what it's doing first, before (usually)
   * being destroyed. Parts should never use this signal, hosts should only connect
   * to this signal.
   */
  void openURLRequestDelayed( const KURL &url, const KParts::URLArgs &args = KParts::URLArgs() );

  /**
   * Tells the hosting browser that the part opened a new URL (which can be
   * queried via KParts::Part::url().
   *
   * This helps the browser to update/create an entry in the history.
   * The part may @em not emit this signal together with openURLRequest().
   * Emit openURLRequest() if you want the browser to handle a URL the user
   * asked to open (from within your part/document). This signal however is
   * useful if you want to handle URLs all yourself internally, while still
   * telling the hosting browser about new opened URLs, in order to provide
   * a proper history functionality to the user.
   * An example of usage is a html rendering component which wants to emit
   * this signal when a child frame document changed its URL.
   * Conclusion: you probably want to use openURLRequest() instead.
   */
  void openURLNotify();

  /**
   * Updates the URL shown in the browser's location bar to @p url.
   */
  void setLocationBarURL( const TQString &url );

  /**
   * Sets the URL of an icon for the currently displayed page.
   */
  void setIconURL( const KURL &url );

  /**
   * Asks the hosting browser to open a new window for the given @p url.
   *
   * The @p args argument is optional additional information for the
   * browser,
   * @see KParts::URLArgs
   */
  void createNewWindow( const KURL &url, const KParts::URLArgs &args = KParts::URLArgs() );

  /**
   * Asks the hosting browser to open a new window for the given @p url
   * and return a reference to the content part.
   * The request for a reference to the part is only fullfilled/processed
   * if the serviceType is set in the @p args . (otherwise the request cannot be
   * processed synchroniously.
   */
  void createNewWindow( const KURL &url, const KParts::URLArgs &args,
                        const KParts::WindowArgs &windowArgs, KParts::ReadOnlyPart *&part );

  /**
   * Since the part emits the jobid in the started() signal,
   * progress information is automatically displayed.
   *
   * However, if you don't use a TDEIO::Job in the part,
   * you can use loadingProgress() and speedProgress()
   * to display progress information.
   */
  void loadingProgress( int percent );
  /**
   * @see loadingProgress
   */
  void speedProgress( int bytesPerSecond );

  void infoMessage( const TQString & );

  /**
   * Emit this to make the browser show a standard popup menu
   * at the point @p global for the files @p items.
   */
  void popupMenu( const TQPoint &global, const KFileItemList &items );

  /**
   * Emit this to make the browser show a standard popup menu
   * at the point @p global for the files @p items.
   *
   * The GUI described by @p client is being merged with the popupmenu of the host
   */
  void popupMenu( KXMLGUIClient *client, const TQPoint &global, const KFileItemList &items );

  void popupMenu( KXMLGUIClient *client, const TQPoint &global, const KFileItemList &items, const KParts::URLArgs &args, KParts::BrowserExtension::PopupFlags i );

  /**
   * Emit this to make the browser show a standard popup menu
   * at the point @p global for the given @p url.
   *
   * Give as much information
   * about this URL as possible, like the @p mimeType and the file type
   * (@p mode: S_IFREG, S_IFDIR...)
   */
  void popupMenu( const TQPoint &global, const KURL &url,
                  const TQString &mimeType, mode_t mode = (mode_t)-1 );

  /**
   * Emit this to make the browser show a standard popup menu
   * at the point @p global for the given @p url.
   *
   * Give as much information
   * about this URL as possible, like the @p mimeType and the file type
   * (@p mode: S_IFREG, S_IFDIR...)
   * The GUI described by @p client is being merged with the popupmenu of the host
   */
  void popupMenu( KXMLGUIClient *client,
                  const TQPoint &global, const KURL &url,
                  const TQString &mimeType, mode_t mode = (mode_t)-1 );

  /**
   * Emit this to make the browser show a standard popup menu
   * at the point @p global for the given @p url.
   *
   * Give as much information
   * about this URL as possible, like @p args.mimeType and the file type
   * (@p mode: S_IFREG, S_IFDIR...)
   * The GUI described by @p client is being merged with the popupmenu of the host
   */
  void popupMenu( KXMLGUIClient *client,
                  const TQPoint &global, const KURL &url,
                  const KParts::URLArgs &args, KParts::BrowserExtension::PopupFlags i, mode_t mode = (mode_t)-1 );

  /**
   * Inform the hosting application about the current selection.
   * Used when a set of files/URLs is selected (with full information
   * about those URLs, including size, permissions etc.)
   */
  void selectionInfo( const KFileItemList &items );
  /**
   * Inform the hosting application about the current selection.
   * Used when some text is selected.
   */
  void selectionInfo( const TQString &text );
  /**
   * Inform the hosting application about the current selection.
   * Used when a set of URLs is selected.
   */
  void selectionInfo( const KURL::List &urls );

  /**
   * Inform the hosting application that the user moved the mouse over an item.
   * Used when the mouse is on an URL.
   */
  void mouseOverInfo( const KFileItem* item );

  /**
   * Ask the hosting application to add a new HTML (aka Mozilla/Netscape)
   * SideBar entry.
   */
  void addWebSideBar(const KURL &url, const TQString& name);

  /**
   * Ask the hosting application to move the top level widget.
   */
  void moveTopLevelWidget( int x, int y );

  /**
   * Ask the hosting application to resize the top level widget.
   */
  void resizeTopLevelWidget( int w, int h );

  /**
   * Ask the hosting application to focus @p part.
   * @since 3.4
   */
  void requestFocus(KParts::ReadOnlyPart *part);

  /**
   * Tell the host (browser) about security state of current page
   * enum PageSecurity { NotCrypted, Encrypted, Mixed };
   * @since 3.4
   */
  void setPageSecurity( int );

#define TDEPARTS_BROWSEREXTENSION_HAS_ITEMS_REMOVED
  /**
   * Inform the host about items that have been removed.
   * @since 3.5.5
   */
  void itemsRemoved( const KFileItemList &items );

private slots:
  void slotCompleted();
  void slotOpenURLRequest( const KURL &url, const KParts::URLArgs &args );
  void slotEmitOpenURLRequestDelayed();
  void slotEnableAction( const char *, bool );
  void slotSetActionText( const char*, const TQString& );

private:
  KParts::ReadOnlyPart *m_part;
  URLArgs m_args;
public:
  typedef TQMap<TQCString,int> ActionNumberMap;

private:
  static ActionNumberMap * s_actionNumberMap;
  static ActionSlotMap * s_actionSlotMap;
  static void createActionSlotMap();
protected:
  virtual void virtual_hook( int id, void* data );
private:
  BrowserExtensionPrivate *d;
};

/**
 * An extension class for container parts, i.e. parts that contain
 * other parts.
 * For instance a TDEHTMLPart hosts one part per frame.
 */
class TDEPARTS_EXPORT BrowserHostExtension : public TQObject
{
  TQ_OBJECT
public:
  BrowserHostExtension( KParts::ReadOnlyPart *parent,
                        const char *name = 0L );

  virtual ~BrowserHostExtension();

  /**
   * Returns a list of the names of all hosted child objects.
   *
   * Note that this method does not query the child objects recursively.
   */
  virtual TQStringList frameNames() const;

  /**
   * Returns a list of pointers to all hosted child objects.
   *
   * Note that this method does not query the child objects recursively.
   */
  virtual const TQPtrList<KParts::ReadOnlyPart> frames() const;

  /**
   * Returns the part that contains @p frame and that may be accessed
   * by @p callingPart
   * @since 3.3
   */
  BrowserHostExtension *findFrameParent(KParts::ReadOnlyPart *callingPart, const TQString &frame);

  /**
   * Opens the given url in a hosted child frame. The frame name is specified in the
   * frameName variable in the urlArgs argument structure (see KParts::URLArgs ) .
   */
  virtual bool openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs );

  /**
   * Queries @p obj for a child object which inherits from this
   * BrowserHostExtension class. Convenience method.
   */
  static BrowserHostExtension *childObject( TQObject *obj );

protected:
  /** This 'enum' along with the structure below is NOT part of the public API.
   * It's going to disappear in KDE 4.0 and is likely to change inbetween.
   *
   * @internal
   */
  enum { VIRTUAL_FIND_FRAME_PARENT = 0x10 };
  struct FindFrameParentParams
  {
      BrowserHostExtension *parent;
      KParts::ReadOnlyPart *callingPart;
      TQString frame;
  };

  virtual void virtual_hook( int id, void* data );
private:
  class BrowserHostExtensionPrivate;
  BrowserHostExtensionPrivate *d;
};

/**
 * An extension class for LiveConnect, i.e\. a call from JavaScript
 * from a HTML page which embeds this part.
 * A part can have an object hierarchie by using objid as a reference
 * to an object.
 */
class TDEPARTS_EXPORT LiveConnectExtension : public TQObject
{
  TQ_OBJECT
public:
  enum Type {
      TypeVoid=0, TypeBool, TypeFunction, TypeNumber, TypeObject, TypeString
  };
  typedef TQValueList<TQPair<Type, TQString> > ArgList;

  LiveConnectExtension( KParts::ReadOnlyPart *parent, const char *name = 0L );

  virtual ~LiveConnectExtension() {}
  /**
   * get a field value from objid, return true on success
   */
  virtual bool get( const unsigned long objid, const TQString & field, Type & type, unsigned long & retobjid, TQString & value );
  /**
   * put a field value in objid, return true on success
   */
  virtual bool put( const unsigned long objid, const TQString & field, const TQString & value );
  /**
   * calls a function of objid, return true on success
   */
  virtual bool call( const unsigned long objid, const TQString & func, const TQStringList & args, Type & type, unsigned long & retobjid, TQString & value );
  /**
   * notifies the part that there is no reference anymore to objid
   */
  virtual void unregister( const unsigned long objid );

  static LiveConnectExtension *childObject( TQObject *obj );
signals:
  /**
   * notify an event from the part of object objid
   */
  virtual void partEvent( const unsigned long objid, const TQString & event, const ArgList & args );
};

}

#endif

