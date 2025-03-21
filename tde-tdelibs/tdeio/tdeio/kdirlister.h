/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>
                 2001, 2002, 2004, 2005 Michael Brade <brade@kde.org>

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

#ifndef kdirlister_h
#define kdirlister_h

#include "tdefileitem.h"
#include "kdirnotify.h"

#include <tqstring.h>
#include <tqstringlist.h>

#include <kurl.h>

namespace TDEIO { class Job; class LocalURLJob; class ListJob; }

/**
 * The dir lister deals with the tdeiojob used to list and update a directory
 * and has signals for the user of this class (e.g. konqueror view or
 * kdesktop) to create/destroy its items when asked.
 *
 * This class is independent from the graphical representation of the dir
 * (icon container, tree view, ...) and it stores the items (as KFileItems).
 *
 * Typical usage :
 * @li Create an instance.
 * @li Connect to at least update, clear, newItem, and deleteItem.
 * @li Call openURL - the signals will be called.
 * @li Reuse the instance when opening a new url (openURL).
 * @li Destroy the instance when not needed anymore (usually destructor).
 *
 * Advanced usage : call openURL with _keep = true to list directories
 * without forgetting the ones previously read (e.g. for a tree view)
 *
 * @short Helper class for the tdeiojob used to list and update a directory.
 * @author Michael Brade <brade@kde.org>
 */
class TDEIO_EXPORT KDirLister : public TQObject
{
  class KDirListerPrivate;
  friend class KDirListerPrivate;
  friend class KDirListerCache;

  TQ_OBJECT
  TQ_PROPERTY( bool autoUpdate READ autoUpdate WRITE setAutoUpdate )
  TQ_PROPERTY( bool showingDotFiles READ showingDotFiles WRITE setShowingDotFiles )
  TQ_PROPERTY( bool dirOnlyMode READ dirOnlyMode WRITE setDirOnlyMode )
  TQ_PROPERTY( bool autoErrorHandlingEnabled READ autoErrorHandlingEnabled )
  TQ_PROPERTY( TQString nameFilter READ nameFilter WRITE setNameFilter )
  TQ_PROPERTY( TQStringList mimeFilter READ mimeFilters WRITE setMimeFilter RESET clearMimeFilter )

public:
  /**
   * Create a directory lister.
   * @param _delayedMimeTypes if true, mime types will be fetched on demand. If false,
   *                          they will always be fetched immediately
   */
  KDirLister( bool _delayedMimeTypes = false );

  /**
   * Destroy the directory lister.
   */
  virtual ~KDirLister();

  /**
   * Run the directory lister on the given url.
   *
   * This method causes KDirLister to emit _all_ the items of @p _url, in any case.
   * Depending on @p _keep either clear() or clear(const KURL &) will be
   * emitted first.
   *
   * The newItems() signal may be emitted more than once to supply you
   * with KFileItems, up until the signal completed() is emitted
   * (and isFinished() returns true).
   *
   * @param _url     the directory URL.
   * @param _keep    if true the previous directories aren't forgotten
   *                 (they are still watched by kdirwatch and their items
   *                 are kept for this KDirLister). This is useful for e.g.
   *                 a treeview.
   * @param _reload  indicates wether to use the cache (false) or to reread the
   *                 directory from the disk.
   *                 Use only when opening a dir not yet listed by this lister
   *                 without using the cache. Otherwise use updateDirectory.
   * @return true    if successful, 
   *         false   otherwise (e.g. unable to communicate with tdeio slave)
   */
  virtual bool openURL( const KURL& _url, bool _keep = false, bool _reload = false );

  /**
   * Stop listing all directories currently being listed.
   *
   * Emits canceled() if there was at least one job running.
   * Emits canceled( const KURL& ) for each stopped job if
   * there are at least two dirctories being watched by KDirLister.
   */
  virtual void stop();

  /**
   * Stop listing the given directory.
   *
   * Emits canceled() if the killed job was the last running one.
   * Emits canceled( const KURL& ) for the killed job if
   * there are at least two directories being watched by KDirLister.
   * No signal is emitted if there was no job running for @p _url.
   * @param _url the directory URL
   */
  virtual void stop( const KURL& _url );

  /**
   * Checks whether KDirWatch will automatically update directories. This is
   * enabled by default.
   * @return true if KDirWatch is used to automatically update directories.
   */
  bool autoUpdate() const;

  /**
   * Enable/disable automatic directory updating, when a directory changes
   * (using KDirWatch).
   * @param enable true to enable, false to disable
   */
  virtual void setAutoUpdate( bool enable );

  /**
   * Check whether auto error handling is enabled.
   * If enabled, it will show an error dialog to the user when an
   * error occurs. It is turned on by default.
   * @return true if auto error handling is enabled, false otherwise
   * @see setAutoErrorHandlingEnabled()
   */
  bool autoErrorHandlingEnabled() const;

  /**
   * Enable or disable auto error handling is enabled.
   * If enabled, it will show an error dialog to the user when an
   * error occurs. It is turned on by default.
   * @param enable true to enable auto error handling, false to disable
   * @param parent the parent widget for the error dialogs, can be 0 for
   *               top-level
   * @see autoErrorHandlingEnabled()
   */
  void setAutoErrorHandlingEnabled( bool enable, TQWidget *parent );

  /**
   * Checks whether hidden files (files beginning with a dot) will be
   * shown.
   * By default this option is disabled (hidden files will be not shown).
   * @return true if dot files are shown, false otherwise
   * @see setShowingDotFiles()
   */
  bool showingDotFiles() const;

  /**
   * Changes the "is viewing dot files" setting.
   * Calls updateDirectory() if setting changed.
   * By default this option is disabled (hidden files will not be shown).
   * @param _showDotFiles true to enable showing hidden files, false to
   *        disable
   * @see showingDotFiles()
   */
  virtual void setShowingDotFiles( bool _showDotFiles );

  /**
   * Checks whether the KDirLister only lists directories or all
   * files.
   * By default this option is disabled (all files will be shown).
   * @return true if setDirOnlyMode(true) was called
   */
  bool dirOnlyMode() const;

  /**
   * Call this to list only directories.
   * By default this option is disabled (all files will be shown).
   * @param dirsOnly true to list only directories
   */
  virtual void setDirOnlyMode( bool dirsOnly );

  /**
   * Returns the top level URL that is listed by this KDirLister.
   * It might be different from the one given with openURL() if there was a 
   * redirection. If you called openURL() with @p _keep == true this is the
   * first url opened (e.g. in a treeview this is the root).
   *
   * @return the url used by this instance to list the files.
   */
  const KURL& url() const;

  /**
   * Returns all URLs that are listed by this KDirLister. This is only
   * useful if you called openURL() with @p _keep == true, as it happens in a
   * treeview, for example. (Note that the base url is included in the list
   * as well, of course.)
   * 
   * @return the list of all listed URLs
   * @since 3.4
   */
  const KURL::List& directories() const;

  /**
   * Actually emit the changes made with setShowingDotFiles, setDirOnlyMode,
   * setNameFilter and setMimeFilter.
   */
  virtual void emitChanges();

  /**
   * Update the directory @p _dir. This method causes KDirLister to _only_ emit
   * the items of @p _dir that actually changed compared to the current state in the
   * cache and updates the cache.
   *
   * The current implementation calls updateDirectory automatically for
   * local files, using KDirWatch (if autoUpdate() is true), but it might be
   * useful to force an update manually.
   *
   * @param _dir the directory URL
   */
  virtual void updateDirectory( const KURL& _dir );

  /**
   * Returns true if no io operation is currently in progress.
   * @return true if finished, false otherwise
   */
  bool isFinished() const;

  /**
   * Returns the file item of the URL.
   * @return the file item for url() itself (".")
   */
  KFileItem *rootItem() const;

  /**
   * Find an item by its URL.
   * @param _url the item URL
   * @return the pointer to the KFileItem
   */
  virtual KFileItem *findByURL( const KURL& _url ) const;
#ifndef KDE_NO_COMPAT
  KFileItem *find( const KURL& _url ) const;
#endif

  /**
   * Find an item by its name.
   * @param name the item name
   * @return the pointer to the KFileItem
   */
  virtual KFileItem *findByName( const TQString& name ) const;

  /**
   * Set a name filter to only list items matching this name, e.g. "*.cpp".
   *
   * You can set more than one filter by separating them with whitespace, e.g
   * "*.cpp *.h".
   * Note: the direcory is not automatically reloaded.
   *
   * @param filter the new filter, TQString::null to disable filtering
   * @see matchesFilter
   */
  virtual void setNameFilter( const TQString &filter );

  /**
   * Returns the current name filter, as set via setNameFilter()
   * @return the current name filter, can be TQString::null if filtering
   *         is turned off
   */
  const TQString& nameFilter() const;

  /**
   * Set mime-based filter to only list items matching the given mimetypes.
   *
   * NOTE: setting the filter does not automatically reload direcory.
   * Also calling this function will not affect any named filter already set.
   *
   * @param mimeList a list of mime-types.
   *
   * @see clearMimeFilter
   * @see matchesMimeFilter
   */
  virtual void setMimeFilter( const TQStringList &mimeList );

  /**
   * Filtering should be done with KFileFilter. This will be implemented in a later
   * revision of KDirLister. This method may be removed then.
   *
   * Set mime-based exclude filter to only list items not matching the given mimetypes
   *
   * NOTE: setting the filter does not automatically reload direcory.
   * Also calling this function will not affect any named filter already set.
   *
   * @param mimeList a list of mime-types.
   * @see clearMimeFilter
   * @see matchesMimeFilter
   * @since 3.1
   * @internal
   */
  void setMimeExcludeFilter(const TQStringList &mimeList );


  /**
   * Clears the mime based filter.
   *
   * @see setMimeFilter
   */
  virtual void clearMimeFilter();

  /**
   * Returns the list of mime based filters, as set via setMimeFilter().
   * @return the list of mime based filters. Empty, when no mime filter is set.
   */
  const TQStringList& mimeFilters() const;

  /**
   * Checks whether @p name matches a filter in the list of name filters.
   * @return true if @p name matches a filter in the list,
   * otherwise false.
   * @see setNameFilter
   */
  bool matchesFilter( const TQString& name ) const;

  /**
   * Checks whether @p mime matches a filter in the list of mime types
   * @param mime the mimetype to find in the filter list.
   * @return true if @p name matches a filter in the list,
   * otherwise false.
   * @see setMimeFilter.
   */
  bool matchesMimeFilter( const TQString& mime ) const;

  /**
   * Pass the main window this object is associated with
   * this is used for caching authentication data
   * @param window the window to associate with, 0 to disassociate
   * @since 3.1
   */
  void setMainWindow( TQWidget *window );

  /**
   * Returns the main window associated with this object.
   * @return the associated main window, or 0 if there is none
   * @since 3.1
   */
  TQWidget *mainWindow();

  /**
   * Used by items() and itemsForDir() to specify whether you want
   * all items for a directory or just the filtered ones.
   */
  enum WhichItems
  {
      AllItems = 0,
      FilteredItems = 1
  };

  /**
   * Returns the items listed for the current url().
   * This method will NOT start listing a directory, you should only call
   * this when receiving the finished() signal.
   *
   * The items in the KFileItemList are references to the items used
   * by KDirLister, so e.g. an item gets destroyed when the deleteItem()
   * signal is emitted.
   *
   * @param which specifies whether the returned list will contain all entries
   *              or only the ones that passed the nameFilter(), mimeFilter(),
   *              etc. Note that the latter causes iteration over all the 
   *              items, filtering them. If this is too slow for you, use the
   *              newItems() signal, sending out filtered items in chunks.
   * @return the items listed for the current url().
   * @since 3.1
   */
  KFileItemList items( WhichItems which = FilteredItems ) const;

  /**
   * Returns the items listed for the given @p dir.
   * This method will NOT start listing @p dir, you should only call
   * this when receiving the finished() signal.
   *
   * The items in the KFileItemList are references to the items used
   * by KDirLister, so e.g. an item gets destroyed when the deleteItem()
   * signal is emitted.
   *
   * @param dir specifies the url for which the items should be returned. This
   *            is only useful if you use KDirLister with multiple URLs
   *            i.e. using bool keep = true in openURL().
   * @param which specifies whether the returned list will contain all entries
   *              or only the ones that passed the nameFilter, mimeFilter, etc.
   *              Note that the latter causes iteration over all the items,
   *              filtering them. If this is too slow for you, use the
   * newItems() signal, sending out filtered items in chunks.
   * @return the items listed for @p dir.
   * @since 3.1
   */
  KFileItemList itemsForDir( const KURL& dir,
                             WhichItems which = FilteredItems ) const;

signals:
  /**
   * Tell the view that we started to list @p _url. NOTE: this does _not_ imply that there
   * is really a job running! I.e. KDirLister::jobs() may return an empty list. In this case
   * the items are taken from the cache.
   *
   * The view knows that openURL should start it, so it might seem useless,
   * but the view also needs to know when an automatic update happens.
   * @param _url the URL to list
   */
  void started( const KURL& _url );

  /**
   * Tell the view that listing is finished. There are no jobs running anymore.
   */
  void completed();

  /**
   * Tell the view that the listing of the directory @p _url is finished.
   * There might be other running jobs left.
   * @param _url the directory URL
   */
  void completed( const KURL& _url );

  /**
   * Tell the view that the user canceled the listing. No running jobs are left.
   */
  void canceled();

  /**
   * Tell the view that the listing of the directory @p _url was canceled.
   * There might be other running jobs left.
   * @param _url the directory URL
   */
  void canceled( const KURL& _url );

  /**
   * Signal a redirection.
   * Only emitted if there's just one directory to list, i.e. most
   * probably openURL() has been called with @p _keep == @p false.
   * @param _url the new URL
   */
  void redirection( const KURL& _url );

  /**
   * Signal a redirection.
   * @param oldUrl the original URL
   * @param newUrl the new URL
   */
  void redirection( const KURL& oldUrl, const KURL& newUrl );

  /**
   * Signal to clear all items.
   * It must always be connected to this signal to avoid doubled items!
   */
  void clear();

  /**
   * Signal to empty the directory @p _url.
   * It is only emitted if the lister is holding more than one directory.
   * @param _url the directory that will be emptied
   */
  void clear( const KURL& _url );

  /**
   * Signal new items.
   * @param items a list of new items
   */
  void newItems( const KFileItemList& items );

  /**
   * Send a list of items filtered-out by mime-type.
   * @param items the list of filtered items
   */
  void itemsFilteredByMime( const KFileItemList& items );

  /**
   * Signal an item to remove.
   *
   * ATTENTION: if @p _fileItem == rootItem() the directory this lister
   *            is holding was deleted and you HAVE to release especially the
   *            rootItem() of this lister, otherwise your app will CRASH!!
   *            The clear() signals have been emitted already.
   * @param _fileItem the fileItem to delete
   */
  void deleteItem( KFileItem *_fileItem );

  /**
   * Signal an item to refresh (its mimetype/icon/name has changed).
   * Note: KFileItem::refresh has already been called on those items.
   * @param items the items to refresh
   */
  void refreshItems( const KFileItemList& items );

  /**
   * Emitted to display information about running jobs.
   * Examples of message are "Resolving host", "Connecting to host...", etc.
   * @param msg the info message
   */
  void infoMessage( const TQString& msg );

  /**
   * Progress signal showing the overall progress of the KDirLister.
   * This allows using a progress bar very easily. (see KProgress)
   * @param percent the progress in percent
   */
  void percent( int percent );

  /**
   * Emitted when we know the size of the jobs.
   * @param size the total size in bytes
   */
  void totalSize( TDEIO::filesize_t size );

  /**
   * Regularly emitted to show the progress of this KDirLister.
   * @param size the processed size in bytes
   */
  void processedSize( TDEIO::filesize_t size );

  /**
   * Emitted to display information about the speed of the jobs.
   * @param bytes_per_second the speed in bytes/s
   */
  void speed( int bytes_per_second );

protected:
  enum Changes {
    NONE=0, NAME_FILTER=1, MIME_FILTER=2, DOT_FILES=4, DIR_ONLY_MODE=8
  };

  /**
   * Called for every new item before emitting newItems().
   * You may reimplement this method in a subclass to implement your own
   * filtering.
   * The default implementation filters out ".." and everything not matching
   * the name filter(s)
   * @return true if the item is "ok".
   *         false if the item shall not be shown in a view, e.g.
   * files not matching a pattern *.cpp ( KFileItem::isHidden())
   * @see matchesFilter
   * @see setNameFilter
   */
  virtual bool matchesFilter( const KFileItem * ) const;

  /**
   * Called for every new item before emitting newItems().
   * You may reimplement this method in a subclass to implement your own
   * filtering.
   * The default implementation filters out ".." and everything not matching
   * the name filter(s)
   * @return true if the item is "ok".
   *         false if the item shall not be shown in a view, e.g.
   * files not matching a pattern *.cpp ( KFileItem::isHidden())
   * @see matchesMimeFilter
   * @see setMimeFilter
   */
  virtual bool matchesMimeFilter( const KFileItem * ) const;

  /**
   * Called by the public matchesFilter() to do the
   * actual filtering. Those methods may be reimplemented to customize
   * filtering.
   * @param name the name to filter
   * @param filters a list of regular expressions for filtering
   */
  virtual bool doNameFilter( const TQString& name, const TQPtrList<TQRegExp>& filters ) const;

  /**
   * Called by the public matchesMimeFilter() to do the
   * actual filtering. Those methods may be reimplemented to customize
   * filtering.
   * @param mime the mime type to filter
   * @param filters the list of mime types to filter
   */
  virtual bool doMimeFilter( const TQString& mime, const TQStringList& filters ) const;

  /**
   * @internal
   */
  bool doMimeExcludeFilter( const TQString& mimeExclude, const TQStringList& filters ) const;

  /**
   * Checks if an url is malformed or not and displays an error message
   * if it is and autoErrorHandling is set to true.
   * @return true if url is valid, otherwise false.
   */
  virtual bool validURL( const KURL& ) const;

  /** Reimplement to customize error handling */
  virtual void handleError( TDEIO::Job * );

protected:
  virtual void virtual_hook( int id, void *data );

private slots:
  void slotInfoMessage( TDEIO::Job *, const TQString& );
  void slotPercent( TDEIO::Job *, unsigned long );
  void slotTotalSize( TDEIO::Job *, TDEIO::filesize_t );
  void slotProcessedSize( TDEIO::Job *, TDEIO::filesize_t );
  void slotSpeed( TDEIO::Job *, unsigned long );
  void slotOpenURLGotLocalURL( TDEIO::LocalURLJob*, const KURL&, bool );
  void slotLocalURLKIODestroyed( );

private:
  void jobStarted( TDEIO::ListJob * );
  void connectJob( TDEIO::ListJob * );
  void jobDone( TDEIO::ListJob * );

  uint numJobs();

public:
  void emitCompleted( const KURL& _url );

private:
  virtual void addNewItem( const KFileItem *item );
  virtual void addNewItems( const KFileItemList& items );
  /*virtual*/ void aboutToRefreshItem( const KFileItem *item );  // TODO: KDE 4.0 make virtual
  virtual void addRefreshItem( const KFileItem *item );
  virtual void emitItems();
  virtual void emitDeleteItem( KFileItem *item );

  KDirListerPrivate *d;
};

#endif

