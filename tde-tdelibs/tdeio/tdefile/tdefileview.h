/* This file is part of the KDE libraries
    Copyright (C) 1997 Stephan Kulow <coolo@kde.org>
    Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef TDEFILEVIEW_H
#define TDEFILEVIEW_H

class TQPoint;
class TDEActionCollection;

#include <tqwidget.h>

#include "tdefileitem.h"
#include "tdefile.h"

/**
 * internal class to make easier to use signals possible
 * @internal
 **/
class TDEIO_EXPORT KFileViewSignaler : public TQObject
{
    TQ_OBJECT

public:
    /**
      * Call this method when an item is selected (depends on single click /
      * double click configuration). Emits the appropriate signal.
      **/
    void activate( const KFileItem *item ) {
        if ( item->isDir() )
            emit dirActivated( item );
        else
            emit fileSelected( item );
    }
    /**
     * emits the highlighted signal for item. Call this in your view class
     * whenever the selection changes.
     */
    void highlightFile(const KFileItem *i) { emit fileHighlighted(i); }

    void activateMenu( const KFileItem *i, const TQPoint& pos ) {
        emit activatedMenu( i, pos );
    }

    void changeSorting( TQDir::SortSpec sorting ) {
        emit sortingChanged( sorting );
    }
    
    void dropURLs(const KFileItem *i, TQDropEvent*e, const KURL::List&urls) {
        emit dropped(i, e, urls);
    }

signals:
    void dirActivated(const KFileItem*);

    void sortingChanged( TQDir::SortSpec );

    /**
     * the item maybe be 0L, indicating that we're in multiselection mode and
     * the selection has changed.
     */
    void fileHighlighted(const KFileItem*);
    void fileSelected(const KFileItem*);
    void activatedMenu( const KFileItem *i, const TQPoint& );
    void dropped(const KFileItem *, TQDropEvent*, const KURL::List&);
};

/**
  * This class defines an interface to all file views. Its intent is
  * to allow to switch the view of the files in the selector very easily.
  * It defines some pure virtual functions, that must be implemented to
  * make a file view working.
  *
  * Since this class is not a widget, but it's meant to be added to other
  * widgets, its most important function is widget. This should return
  * a pointer to the implemented widget.
  *
  * @short A base class for views of the KDE file selector
  * @author Stephan Kulow <coolo@kde.org>
  **/
class TDEIO_EXPORT KFileView {

public:
    KFileView();

    /**
     * Destructor
     */
    virtual ~KFileView();

    /**
     * inserts a list of items.
     **/
    void addItemList(const KFileItemList &list);

    /**
      * a pure virtual function to get a TQWidget, that can be added to
      * other widgets. This function is needed to make it possible for
      * derived classes to derive from other widgets.
      **/
    virtual TQWidget *widget() = 0;

    /**
     * ### As const-method, to be fixed in 3.0
     */
    TQWidget *widget() const { return const_cast<KFileView*>(this)->widget(); }

    /**
     * Sets @p filename the current item in the view, if available.
     */
    void setCurrentItem( const TQString &filename );

    /**
     * Reimplement this to set @p item the current item in the view, e.g.
     * the item having focus.
     */
    virtual void setCurrentItem( const KFileItem *item ) = 0;

    /**
     * @returns the "current" KFileItem, e.g. where the cursor is.
     * Returns 0L when there is no current item (e.g. in an empty view).
     * Subclasses have to implement this.
     */
    virtual KFileItem *currentFileItem() const = 0;

    /**
     * Clears the view and all item lists.
     */
    virtual void clear();

    /**
      * does a repaint of the view.
      *
      * The default implementation calls
      * \code
      * widget()->repaint(f)
      * \endcode
      **/
    virtual void updateView(bool f = true);

    virtual void updateView(const KFileItem*);

    /**
     * Removes an item from the list; has to be implemented by the view.
     * Call KFileView::removeItem( item ) after removing it.
     */
    virtual void removeItem(const KFileItem *item);

    /**
     * This hook is called when all items of the currently listed directory
     * are listed and inserted into the view, i.e. there won't come any new
     * items anymore.
     */
    virtual void listingCompleted();

    /**
      * Returns the sorting order of the internal list. Newly added files
      * are added through this sorting.
      */
    TQDir::SortSpec sorting() const { return m_sorting; }

    /**
      * Sets the sorting order of the view.
      *
      * Default is TQDir::Name | TQDir::IgnoreCase | TQDir::DirsFirst
      * Override this in your subclass and sort accordingly (usually by
      * setting the sorting-key for every item and telling QIconView
      * or TQListView to sort.
      *
      * A view may choose to use a different sorting than TQDir::Name, Time
      * or Size. E.g. to sort by mimetype or any possible string. Set the
      * sorting to TQDir::Unsorted for that and do the rest internally.
      *
      * @see sortingKey
      */
    virtual void setSorting(TQDir::SortSpec sort);

    /**
     * Tells whether the current items are in reversed order (shortcut to
     * sorting() & TQDir::Reversed).
     */
    bool isReversed() const { return (m_sorting & TQDir::Reversed); }

    void sortReversed();

    /**
      * @returns the number of dirs and files
      **/
    uint count() const { return filesNumber + dirsNumber; }

    /**
      * @returns the number of files.
      **/
    uint numFiles() const { return filesNumber; }

    /**
      * @returns the number of directories
      **/
    uint numDirs() const { return dirsNumber; }

    virtual void setSelectionMode( KFile::SelectionMode sm );
    virtual KFile::SelectionMode selectionMode() const;

    enum ViewMode {
	Files       = 1,
	Directories = 2,
	All = Files | Directories
    };
    virtual void setViewMode( ViewMode vm );
    virtual ViewMode viewMode() const { return view_mode; }

    /**
     * @returns the localized name of the view, which could be displayed
     * somewhere, e.g. in a menu, where the user can choose between views.
     * @see setViewName
     */
    TQString viewName() const { return m_viewName; }

    /**
     * Sets the name of the view, which could be displayed somewhere.
     * E.g. "Image Preview".
     */
    void setViewName( const TQString& name ) { m_viewName = name; }

    virtual void setParentView(KFileView *parent);

    /**
     * The derived view must implement this function to add
     * the file in the widget.
     *
     * Make sure to call this implementation, i.e.
     * KFileView::insertItem( i );
     *
     */
    virtual void insertItem( KFileItem *i);

    /**
     * pure virtual function, that should be implemented to clear
     * the view. At this moment the list is already empty
     **/
    virtual void clearView() = 0;

    /**
     * pure virtual function, that should be implemented to make item i
     * visible, i.e. by scrolling the view appropriately.
     */
    virtual void ensureItemVisible( const KFileItem *i ) = 0;

    /**
     * Clears any selection, unhighlights everything. Must be implemented by
     * the view.
     */
    virtual void clearSelection() = 0;

    /**
     * Selects all items. You may want to override this, if you can implement
     * it more efficiently than calling setSelected() with every item.
     * This works only in Multiselection mode of course.
     */
    virtual void selectAll();

    /**
     * Inverts the current selection, i.e. selects all items, that were up to
     * now not selected and deselects the other.
     */
    virtual void invertSelection();

    /**
     * Tells the view that it should highlight the item.
     * This function must be implemented by the view.
     **/
    virtual void setSelected(const KFileItem *, bool enable) = 0;

    /**
     * @returns whether the given item is currently selected.
     * Must be implemented by the view.
     */
    virtual bool isSelected( const KFileItem * ) const = 0;

    /**
     * @returns all currently highlighted items.
     */
    const KFileItemList * selectedItems() const;

    /**
     * @returns all items currently available in the current sort-order
     */
    const KFileItemList * items() const;

    virtual KFileItem * firstFileItem() const = 0;
    virtual KFileItem * nextItem( const KFileItem * ) const = 0;
    virtual KFileItem * prevItem( const KFileItem * ) const = 0;

    /**
     * This is a KFileDialog specific hack: we want to select directories with
     * single click, but not files. But as a generic class, we have to be able
     * to select files on single click as well.
     *
     * This gives us the opportunity to do both.
     *
     * Every view has to decide when to call select( item ) when a file was
     * single-clicked, based on onlyDoubleClickSelectsFiles().
     */
    void setOnlyDoubleClickSelectsFiles( bool enable ) {
	myOnlyDoubleClickSelectsFiles = enable;
    }

    /**
     * @returns whether files (not directories) should only be select()ed by
     * double-clicks.
     * @see setOnlyDoubleClickSelectsFiles
     */
    bool onlyDoubleClickSelectsFiles() const {
	return myOnlyDoubleClickSelectsFiles;
    }

    /**
     * increases the number of dirs and files.
     * @returns true if the item fits the view mode
     */
    bool updateNumbers(const KFileItem *i);

    /**
     * @returns the view-specific action-collection. Every view should
     * add its actions here (if it has any) to make them available to
     * e.g. the KDirOperator's popup-menu.
     */
    virtual TDEActionCollection * actionCollection() const;

    KFileViewSignaler * signaler() const { return sig; }

    virtual void readConfig( TDEConfig *, const TQString& group = TQString::null );
    virtual void writeConfig( TDEConfig *, const TQString& group = TQString::null);

    /**
     * Various options for drag and drop support. 
     * These values can be or'd together.
     * @li @p AutoOpenDirs Automatically open directory after hovering above it 
     * for a short while while dragging.
     * @since 3.2
     */
    enum DropOptions {
	AutoOpenDirs  = 1
    };
    /**
     * Specify DND options. See DropOptions for details.
     * All options are disabled by default.
     * @since 3.2
     */
    // KDE 4: Make virtual
    void setDropOptions(int options);
    
    /**
     * Returns the DND options in effect.
     * See DropOptions for details.
     * @since 3.2
     */
    int dropOptions();
    
    /**
     * This method calculates a TQString from the given parameters, that is
     * suitable for sorting with e.g. TQIconView or TQListView. Their
     * Item-classes usually have a setKey( const TQString& ) method or a virtual
     * method TQString key() that is used for sorting.
     *
     * @param value Any string that should be used as sort criterion
     * @param isDir Tells whether the key is computed for an item representing
     *              a directory (directories are usually sorted before files)
     * @param sortSpec An ORed combination of TQDir::SortSpec flags.
     *                 Currently, the values IgnoreCase, Reversed and
     *                 DirsFirst are taken into account.
     */
    static TQString sortingKey( const TQString& value, bool isDir, int sortSpec);

    /**
     * An overloaded method that takes not a TQString, but a number as sort
     * criterion. You can use this for file-sizes or dates/times for example.
     * If you use a time_t, you need to cast that to TDEIO::filesize_t because
     * of ambiguity problems.
     */
    static TQString sortingKey( TDEIO::filesize_t value, bool isDir,int sortSpec);

    /**
     * @internal
     * delay before auto opening a directory
     */
    static int autoOpenDelay();

protected:
    /**
     * @internal
     * class to distribute the signals
     **/
    KFileViewSignaler *sig;

private:
    static TQDir::SortSpec defaultSortSpec;
    TQDir::SortSpec m_sorting;
    TQString m_viewName;

    /**
     * counters
     **/
    uint filesNumber;
    uint dirsNumber;

    ViewMode view_mode;
    KFile::SelectionMode selection_mode;

    // never use! It's only guaranteed to contain valid items in the items()
    // method!
    mutable KFileItemList m_itemList;

    mutable KFileItemList *m_selectedList;
    bool myOnlyDoubleClickSelectsFiles;

protected:
    virtual void virtual_hook( int id, void* data );
    /* @internal for virtual_hook */
    enum { VIRTUAL_SET_DROP_OPTIONS = 1 };
    void setDropOptions_impl(int options);
private:
    class KFileViewPrivate;
    KFileViewPrivate *d;
};

#endif // TDEFILEINFOLISTWIDGET_H
