/* This file is part of the KDE libraries
   Copyright (C) 1999,2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KCOMPLETION_H
#define KCOMPLETION_H

#include <tqmap.h>
#include <tqptrlist.h>
#include <tqobject.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqguardedptr.h>

#include "tdelibs_export.h"
#include <tdeglobalsettings.h>
#include <ksortablevaluelist.h>
#include <tdeshortcut.h>

class TDECompTreeNode;
class TDECompletionPrivate;
class TDECompletionBasePrivate;
class TDECompletionMatchesWrapper;
class TDECompletionMatches;
class TQPopupMenu;

/**
 * @short A generic class for completing QStrings
 *
 * This class offers easy use of "auto-completion", "manual-completion" or
 * "shell completion" on TQString objects. A common use is completing filenames
 * or URLs (see KURLCompletion()).
 * But it is not limited to URL-completion -- everything should be completable!
 * The user should be able to complete email-addresses, telephone-numbers,
 * commands, SQL queries, ...
 * Every time your program knows what the user can type into an edit-field, you
 * should offer completion. With TDECompletion, this is very easy, and if you are
 * using a line edit widget ( KLineEdit), it is even more easy.
 * Basically, you tell a TDECompletion object what strings should be completable
 * and whenever completion should be invoked, you call makeCompletion().
 * KLineEdit and (an editable) KComboBox even do this automatically for you.
 *
 * TDECompletion offers the completed string via the signal match() and
 * all matching strings (when the result is ambiguous) via the method
 * allMatches().
 *
 * Notice: auto-completion, shell completion and manual completion work
 *         slightly differently:
 *
 * @li auto-completion always returns a complete item as match.
 *     When more than one matching items are available, it will deliver just
 *     the first (depending on sorting order) item. Iterating over all matches
 *     is possible via nextMatch() and previousMatch().
 *
 * @li popup-completion works in the same way, the only difference being that
 *     the completed items are not put into the edit-widget, but into a
 *     separate popup-box.
 *
 * @li manual completion works the same way as auto-completion, the
 *     subtle difference is, that it isn't invoked automatically while the user
 *     is typing, but only when the user presses a special key. The difference
 *     of manual and auto-completion is therefore only visible in UI classes,
 *     TDECompletion needs to know whether to deliver partial matches
 *     (shell completion) or whole matches (auto/manual completion), therefore
 * TDEGlobalSettings::CompletionMan and
 * TDEGlobalSettings::CompletionAuto have the exact same effect in
 *     TDECompletion.
 *
 * @li shell completion works like how shells complete filenames:
 *     when multiple matches are available, the longest possible string of all
 *     matches is returned (i.e. only a partial item).
 *     Iterating over all matching items (complete, not partial) is possible
 *     via nextMatch() and previousMatch().
 *
 * You don't have to worry much about that though, TDECompletion handles
 * that for you, according to the setting setCompletionMode().
 * The default setting is globally configured by the user and read
 * from TDEGlobalSettings::completionMode().
 *
 * A short example:
 * \code
 * TDECompletion completion;
 * completion.setOrder( TDECompletion::Sorted );
 * completion.addItem( "pfeiffer@kde.org" );
 * completion.addItem( "coolo@kde.org" );
 * completion.addItem( "carpdjih@sp.zrz.tu-berlin.de" );
 * completion.addItem( "carp@cs.tu-berlin.de" );
 *
 * cout << completion.makeCompletion( "ca" ).latin1() << endl;
 * \endcode
 *
 * In shell-completion-mode, this will be "carp"; in auto-completion-
 * mode it will be "carp\@cs.tu-berlin.de", as that is alphabetically
 * smaller.
 * If setOrder was set to Insertion, "carpdjih\@sp.zrz.tu-berlin.de"
 * would be completed in auto-completion-mode, as that was inserted before
 * "carp\@cs.tu-berlin.de".
 *
 * You can dynamically update the completable items by removing and adding them
 * whenever you want.
 * For advanced usage, you could even use multiple TDECompletion objects. E.g.
 * imagine an editor like kwrite with multiple open files. You could store
 * items of each file in a different TDECompletion object, so that you know (and
 * tell the user) where a completion comes from.
 *
 * Note: TDECompletion does not work with strings that contain 0x0 characters
 *       (unicode nul), as this is used internally as a delimiter.
 *
 * You may inherit from TDECompletion and override makeCompletion() in
 * special cases (like reading directories/urls and then supplying the
 * contents to TDECompletion, as KURLCompletion does), but generally, this is
 * not necessary.
 *
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class TDECORE_EXPORT TDECompletion : public TQObject
{
    TQ_ENUMS( CompOrder )
    TQ_PROPERTY( CompOrder order READ order WRITE setOrder )
    TQ_PROPERTY( bool ignoreCase READ ignoreCase WRITE setIgnoreCase )
    TQ_PROPERTY( TQStringList items READ items WRITE setItems )
    TQ_OBJECT

public:
    /**
     * Constants that represent the order in which TDECompletion performs
     * completion-lookups.
     */
    enum CompOrder { Sorted,    ///< Use alphabetically sorted order
                     Insertion, ///< Use order of insertion
                     Weighted   ///< Use weighted order
    };

    /**
     * Constructor, nothing special here :)
     */
    TDECompletion();

    // FIXME: copy constructor, assignment operator...

    /**
     * Destructor, nothing special here, either.
     */
    virtual ~TDECompletion();

    /**
     * Attempts to find an item in the list of available completions,
     * that begins with @p string. Will either return the first matching item
     * (if there is more than one match) or TQString::null, if no match was
     * found.
     *
     * In the latter case, a sound will be issued, depending on
     * isSoundsEnabled().
     * If a match was found, it will also be emitted via the signal
     * match().
     *
     * If this is called twice or more often with the same string while no
     * items were added or removed in the meantime, all available completions
     * will be emitted via the signal #matches().
     * This happens only in shell-completion-mode.
     *
     * @param string the string to complete
     * @return the matching item, or TQString::null if there is no matching
     * item.
     * @see slotMakeCompletion
     * @see substringCompletion
     */
    virtual TQString makeCompletion( const TQString& string );

    /**
     * Returns a list of all completion items that contain the given @p string.
     * @param string the string to complete
     * @return a list of items which all contain @p text as a substring,
     * i.e. not necessarily at the beginning.
     *
     * @see makeCompletion
     */
    TQStringList substringCompletion( const TQString& string ) const;

    /**
     * Returns the next item from the matching-items-list.
     * When reaching the beginning, the list is rotated so it will return the
     * last match and a sound is issued (depending on isSoundsEnabled()).
     * @return the next item from the matching-items-list.
     * When there is no match, TQString::null is returned and
     * a sound is be issued.
     * @see slotPreviousMatch
     */
    TQString previousMatch();

    /**
     * Returns the next item from the matching-items-list.
     * When reaching the last item, the list is rotated, so it will return
     * the first match and a sound is issued (depending on
     * isSoundsEnabled()).
     * @return the next item from the matching-items-list.  When there is no
     * match, TQString::null is returned and a sound is issued
     * @see slotNextMatch
     */
    TQString nextMatch();

    /**
     * Returns the last match. Might be useful if you need to check whether
     * a completion is different from the last one.
     * @return the last match. TQString::null is returned when there is no
     *         last match.
     */
    virtual const TQString& lastMatch() const { return myLastMatch; }

    /**
     * Returns a list of all items inserted into TDECompletion. This is useful
     * if you need to save the state of a TDECompletion object and restore it
     * later.
     *
     * Important note: when order() == Weighted, then every item in the
     * stringlist has its weight appended, delimited by a colon. E.g. an item
     * "www.kde.org" might look like "www.kde.org:4", where 4 is the weight.
     *
     * This is necessary so that you can save the items along with its
     * weighting on disk and load them back with setItems(), restoring its
     * weight as well. If you really don't want the appended weightings, call
     * setOrder( TDECompletion::Insertion )
     * before calling items().
     *
     * @return a list of all items
     * @see setItems
     */
    TQStringList items() const;
    
    /**
     * Returns true when the completion object contains no entries.
     */
    bool isEmpty() const;

    /**
     * Sets the completion mode to Auto/Manual, Shell or None.
     * If you don't set the mode explicitly, the global default value
     * TDEGlobalSettings::completionMode() is used.
     * TDEGlobalSettings::CompletionNone disables completion.
     * @param mode the completion mode
     * @see completionMode
     * @see TDEGlobalSettings::completionMode
     */
    virtual void setCompletionMode( TDEGlobalSettings::Completion mode );

    /**
     * Return the current completion mode.
     * May be different from TDEGlobalSettings::completionMode(), if you
     * explicitly called setCompletionMode().
     * @return the current completion mode
     * @see setCompletionMode
     */
    TDEGlobalSettings::Completion completionMode() const {
        return myCompletionMode;
    }

    /**
     * TDECompletion offers three different ways in which it offers its items:
     * @li in the order of insertion
     * @li sorted alphabetically
     * @li weighted
     *
     * Choosing weighted makes TDECompletion perform an implicit weighting based
     * on how often an item is inserted. Imagine a web browser with a location
     * bar, where the user enters URLs. The more often a URL is entered, the
     * higher priority it gets.
     *
     * Note: Setting the order to sorted only affects new inserted items,
     * already existing items will stay in the current order. So you probably
     * want to call setOrder( Sorted ) before inserting items, when you want
     * everything sorted.
     *
     * Default is insertion order.
     * @param order the new order
     * @see order
     */
    virtual void setOrder( CompOrder order );

    /**
     * Returns the completion order.
     * @return the current completion order.
     * @see setOrder
     */
    CompOrder order() const { return myOrder; }

    /**
     * Setting this to true makes TDECompletion behave case insensitively.
     * E.g. makeCompletion( "CA" ); might return "carp\@cs.tu-berlin.de".
     * Default is false (case sensitive).
     * @param ignoreCase true to ignore the case
     * @see ignoreCase
     */
    virtual void setIgnoreCase( bool ignoreCase );

    /**
     * Return whether TDECompletion acts case insensitively or not.
     * Default is false (case sensitive).
     * @return true if the case will be ignored
     * @see setIgnoreCase
     */
    bool ignoreCase() const { return myIgnoreCase; }

    /**
     * Returns a list of all items matching the last completed string.
     * Might take some time, when you have LOTS of items.
     * @return a list of all matches for the last completed string.
     * @see substringCompletion
     */
    TQStringList allMatches();

    /**
     * Returns a list of all items matching @p string.
     * @param string the string to match
     * @return the list of all matches
     */
    TQStringList allMatches( const TQString& string );

    /**
     * Returns a list of all items matching the last completed string.
     * Might take some time, when you have LOTS of items.
     * The matches are returned as TDECompletionMatches, which also
     * keeps the weight of the matches, allowing
     * you to modify some matches or merge them with matches
     * from another call to allWeightedMatches(), and sort the matches
     * after that in order to have the matches ordered correctly.
     *
     * @return a list of all completion matches
     * @see substringCompletion
     */
    TDECompletionMatches allWeightedMatches();

    /**
     * Returns a list of all items matching @p string.
     * @param string the string to match
     * @return a list of all matches
     */
    TDECompletionMatches allWeightedMatches( const TQString& string );

    /**
     * Enables/disables playing a sound when
     * @li makeCompletion() can't find a match
     * @li there is a partial completion (= multiple matches in
     *     Shell-completion mode)
     * @li nextMatch() or previousMatch() hit the last possible
     *     match -> rotation
     *
     * For playing the sounds, KNotifyClient() is used.
     *
     * @param enable true to enable sounds
     * @see isSoundsEnabled
     */
    virtual void setEnableSounds( bool enable ) { myBeep = enable; }

    /**
     * Tells you whether TDECompletion will play sounds on certain occasions.
     * Default is enabled.
     * @return true if sounds are enabled
     * @see enableSounds
     * @see disableSounds
     */
    bool isSoundsEnabled() const { return myBeep; }

    /**
     * Returns true when more than one match is found.
     * @return true if there are more than one match
     * @see multipleMatches
     */
    bool hasMultipleMatches() const { return myHasMultipleMatches; }

#ifndef KDE_NO_COMPAT
    /**
     * @deprecated
     * @see setEnableSounds
     */
    void enableSounds() { myBeep = true; }

    /**
     * @deprecated
     * @see setEnableSounds
     */
    void disableSounds() { myBeep = false; }
#endif
    
public slots:
    /**
     * Attempts to complete "string" and emits the completion via match().
     * Same as makeCompletion() (just as a slot).
     * @param string the string to complete
     * @see makeCompletion
     */
    void slotMakeCompletion( const TQString& string ) {
        (void) makeCompletion( string );
    }

    /**
     * Searches the previous matching item and emits it via match().
     * Same as previousMatch() (just as a slot).
     * @see previousMatch
     */
    void slotPreviousMatch() {
        (void) previousMatch();
    }

    /**
     * Searches the next matching item and emits it via match().
     * Same as nextMatch() (just as a slot).
     * @see nextMatch
     */
    void slotNextMatch() {
        (void) nextMatch();
    }

    // FIXME ###: KDE4: unify the nomenclature.  We have insertItems, addItem,
    //            setItems...
    /**
     * Inserts @p items into the list of possible completions.
     * Does the same as setItems(), but does not call clear() before.
     * @param items the items to insert
     */
    void insertItems( const TQStringList& items );

    /**
     * Sets the list of items available for completion. Removes all previous
     * items.
     *
     * Notice: when order() == Weighted, then the weighting is looked up for
     * every item in the stringlist. Every item should have ":number" appended,
     * where number is an unsigned integer, specifying the weighting.
     *
     * If you don't like this, call
     * setOrder( TDECompletion::Insertion )
     * before calling setItems().
     *
     * @param list the list of items that are available for completion
     * @see items
     */
    virtual void setItems( const TQStringList& list);

    /**
     * Adds an item to the list of available completions.
     * Resets the current item-state ( previousMatch() and nextMatch()
     * won't work anymore).
     * @param item the item to add
     */
    void addItem( const TQString& item);

    /**
     * Adds an item to the list of available completions.
     * Resets the current item-state ( previousMatch() and nextMatch()
     * won't work anymore).
     *
     * Sets the weighting of the item to @p weight or adds it to the current
     * weighting if the item is already available. The weight has to be greater
     * than 1 to take effect (default weight is 1).
     * @param item the item to add
     * @param weight the weight of the item, default is 1
     */
    void addItem( const TQString& item, uint weight );

    /**
     * Removes an item from the list of available completions.
     * Resets the current item-state ( previousMatch() and nextMatch()
     * won't work anymore).
     * @param item the item to remove
     */
    void removeItem( const TQString& item);

    /**
     * Removes all inserted items.
     */
    virtual void clear();


signals:
    /**
     * The matching item. Will be emitted by makeCompletion(),
     * previousMatch() or nextMatch(). May be TQString::null if there
     * is no matching item.
     * @param item the match, or TQString::null if there is none
     */
    void match( const TQString& item);

    /**
     * All matching items. Will be emitted by makeCompletion() in shell-
     * completion-mode, when the same string is passed to makeCompletion twice
     * or more often.
     * @param matchlist the list of matches
     */
    void matches( const TQStringList& matchlist);

    /**
     * This signal is emitted, when calling makeCompletion() and more than
     * one matching item is found.
     * @see hasMultipleMatches
     */
    void multipleMatches();

protected:
    /**
     * This method is called after a completion is found and before the
     * matching string is emitted. You can override this method to modify the
     * string that will be emitted.
     * This is necessary e.g. in KURLCompletion(), where files with spaces
     * in their names are shown escaped ("filename\ with\ spaces"), but stored
     * unescaped inside TDECompletion.
     * Never delete that pointer!
     *
     * Default implementation does nothing.
     * @param match the match to process
     * @see postProcessMatches
     */
    virtual void postProcessMatch( TQString *match ) const { Q_UNUSED(match) }

    /**
     * This method is called before a list of all available completions is
     * emitted via #matches. You can override this method to modify the
     * found items before match() or #matches are emitted.
     * Never delete that pointer!
     *
     * Default implementation does nothing.
     * @param matches the matches to process
     * @see postProcessMatch
     */
    virtual void postProcessMatches( TQStringList * matches ) const { Q_UNUSED(matches)}

    /**
     * This method is called before a list of all available completions is
     * emitted via #matches. You can override this method to modify the
     * found items before #match() or #matches() are emitted.
     * Never delete that pointer!
     *
     * Default implementation does nothing.
     * @param matches the matches to process
     * @see postProcessMatch
     */
    virtual void postProcessMatches( TDECompletionMatches * matches ) const {Q_UNUSED(matches)}

private:
    void            addWeightedItem( const TQString& );
    TQString         findCompletion( const TQString& string );
    void            findAllCompletions( const TQString&,
                                        TDECompletionMatchesWrapper *matches,
                                        bool& hasMultipleMatches ) const;

    void extractStringsFromNode( const TDECompTreeNode *,
                                 const TQString& beginning,
                                 TDECompletionMatchesWrapper *matches,
                                 bool addWeight = false ) const;
    void extractStringsFromNodeCI( const TDECompTreeNode *,
                                   const TQString& beginning,
                                   const TQString& restString,
                                   TDECompletionMatchesWrapper *matches) const;

    enum        BeepMode { NoMatch, PartialMatch, Rotation };
    void        doBeep( BeepMode ) const;

    TDEGlobalSettings::Completion myCompletionMode;

    CompOrder       myOrder;
    TQString         myLastString;
    TQString         myLastMatch;
    TQString         myCurrentMatch;
    TDECompTreeNode * myTreeRoot;
    TQStringList     myRotations;
    bool            myBeep;
    bool            myIgnoreCase;
    bool            myHasMultipleMatches;
    uint            myRotationIndex;


protected:
    virtual void virtual_hook( int id, void* data );
private:
    TDECompletionPrivate *d;
};

// some more helper stuff
typedef KSortableValueList<TQString> TDECompletionMatchesList;
class TDECompletionMatchesPrivate;

/**
 * This structure is returned by TDECompletion::allWeightedMatches .
 * It also keeps the weight of the matches, allowing
 * you to modify some matches or merge them with matches
 * from another call to allWeightedMatches(), and sort the matches
 * after that in order to have the matches ordered correctly
 *
 * Example (a simplified example of what Konqueror's completion does):
 * \code
 * TDECompletionMatches matches = completion->allWeightedMatches( location );
 * if( !location.startsWith( "www." ))
 matches += completion->allWeightedmatches( "www." + location" );
 * matches.removeDuplicates();
 * TQStringList list = matches.list();
 * \endcode
 *
 * @short List for keeping matches returned from TDECompletion
 */
class TDECORE_EXPORT TDECompletionMatches : public TDECompletionMatchesList
{
public:
    TDECompletionMatches( bool sort );
    /**
     * @internal
     */
    TDECompletionMatches( const TDECompletionMatchesWrapper& matches );
    ~TDECompletionMatches();
    /**
     * Removes duplicate matches. Needed only when you merged several matches
     * results and there's a possibility of duplicates.
     */
    void removeDuplicates();
    /**
     * Returns the matches as a TQStringList.
     * @param sort if false, the matches won't be sorted before the conversion,
     *             use only if you're sure the sorting is not needed
     * @return the list of matches
     */
    TQStringList list( bool sort = true ) const;
    /**
     * If sorting() returns false, the matches aren't sorted by their weight,
     * even if true is passed to list().
     * @return true if the matches won't be sorted
     */
    bool sorting() const {
        return _sorting;
    }
private:
    bool _sorting;
    TDECompletionMatchesPrivate* d;
};

/**
 * An abstract base class for adding a completion feature
 * into widgets.
 *
 * This is a convenience class that provides the basic functions
 * needed to add text completion support into widgets.  All that
 * is required is an implementation for the pure virtual function
 * setCompletedText.  Refer to KLineEdit or KComboBox
 * to see how easily such support can be added using this as a base
 * class.
 *
 * @short An abstract class for adding text completion support to widgets.
 * @author Dawit Alemayehu <adawit@kde.org>
 */
class TDECORE_EXPORT TDECompletionBase
{
public:
    /**
     * Constants that represent the items whose short-cut
     * key-binding is programmable.  The default key-bindings
     * for these items are defined in TDEStdAccel.
     */
    enum KeyBindingType {
        /**
         * Text completion (by default Ctrl-E).
         */
        TextCompletion,
        /**
         * Switch to previous completion (by default Ctrl-Up).
         */
        PrevCompletionMatch,
        /**
         * Switch to next completion (by default Ctrl-Down).
         */
        NextCompletionMatch,
        /**
         * Substring completion (by default Ctrl-T).
         */
        SubstringCompletion
    };


    // Map for the key binding types mentioned above.
    typedef TQMap<KeyBindingType, TDEShortcut> KeyBindingMap;

    /**
     * Default constructor.
     */
    TDECompletionBase();

    /**
     * Destructor.
     */
    virtual ~TDECompletionBase();
    
    /**
     * Returns a pointer to the current completion object.
     *
     * If the completion object does not exist, it is automatically created and
     * by default handles all the completion signals internally unless @p hsig
     * is set to false. It is also automatically destroyed when the destructor
     * is called. You can change this default behavior using the 
     * @ref setAutoDeleteCompletionObject and @ref setHandleSignals member 
     * functions.
     *
     * See also @ref compObj.
     *
     * @param hsig if true, handles completion signals internally.
     * @return a pointer the completion object.
     */
    TDECompletion* completionObject( bool hsig = true );
    
    /**
     * Sets up the completion object to be used.
     *
     * This method assigns the completion object and sets it up to automatically
     * handle the completion and rotation signals internally.  You should use 
     * this function if you want to share one completion object among your
     * widgets or need to use a customized completion object.
     *
     * The object assigned through this method is not deleted when this object's
     * destructor is invoked unless you explicitly call @ref setAutoDeleteCompletionObject
     * after calling this method. Be sure to set the bool argument to false, if
     * you want to handle the completion signals yourself.
     *
     * @param compObj a TDECompletion() or a derived child object.
     * @param hsig if true, handles completion signals internally.
     */
    virtual void setCompletionObject( TDECompletion* compObj, bool hsig = true );

    /**
     * Enables this object to handle completion and rotation
     * events internally.
     *
     * This function simply assigns a boolean value that
     * indicates whether it should handle rotation and
     * completion events or not.  Note that this does not
     * stop the object from emitting signals when these
     * events occur.
     *
     * @param handle if true, handle completion & rotation internally.
     */
    virtual void setHandleSignals( bool handle );

    /**
     * Returns true if the completion object is deleted
     * upon this widget's destruction.
     *
     * See setCompletionObject() and enableCompletion()
     * for details.
     *
     * @return true if the completion object will be deleted
     *              automatically
     */
    bool isCompletionObjectAutoDeleted() const {
        return m_delegate ? m_delegate->isCompletionObjectAutoDeleted() : m_bAutoDelCompObj;
    }

    /**
     * Sets the completion object when this widget's destructor
     * is called.
     *
     * If the argument is set to true, the completion object
     * is deleted when this widget's destructor is called.
     *
     * @param autoDelete if true, delete completion object on destruction.
     */
    void setAutoDeleteCompletionObject( bool autoDelete ) {
        if ( m_delegate )
            m_delegate->setAutoDeleteCompletionObject( autoDelete );
        else
            m_bAutoDelCompObj = autoDelete;
    }

    /**
     * Sets the widget's ability to emit text completion and
     * rotation signals.
     *
     * Invoking this function with @p enable set to @p false will
     * cause the completion & rotation signals not to be emitted.
     * However, unlike setting the completion object to @p NULL
     * using setCompletionObject, disabling the emition of
     * the signals through this method does not affect the current
     * completion object.
     *
     * There is no need to invoke this function by default.  When a
     * completion object is created through completionObject or
     * setCompletionObject, these signals are set to emit
     * automatically.  Also note that disabling this signals will not
     * necessarily interfere with the objects ability to handle these
     * events internally.  See setHandleSignals.
     *
     * @param enable if false, disables the emition of completion & rotation signals.
     */
    void setEnableSignals( bool enable ) {
        if ( m_delegate )
            m_delegate->setEnableSignals( enable );
        else
            m_bEmitSignals = enable;
    }

    /**
     * Returns true if the object handles the signals.
     *
     * @return true if this signals are handled internally.
     */
    bool handleSignals() const { return m_delegate ? m_delegate->handleSignals() : m_bHandleSignals; }

    /**
     * Returns true if the object emits the signals.
     *
     * @return true if signals are emitted
     */
    bool emitSignals() const { return m_delegate ? m_delegate->emitSignals() : m_bEmitSignals; }

    /**
     * Sets the type of completion to be used.
     *
     * The completion modes supported are those defined in
     * TDEGlobalSettings().  See below.
     *
     * @param mode Completion type:
     *   @li CompletionNone:  Disables completion feature.
     *   @li CompletionAuto:  Attempts to find a match &
     *                        fills-in the remaining text.
     *   @li CompletionMan:   Acts the same as the above
     *                        except the action has to be
     *                        manually triggered through
     *                        pre-defined completion key.
     *   @li CompletionShell: Mimics the completion feature
     *                        found in typical *nix shell
     *                        environments.
     *   @li CompletionPopup: Shows all available completions at once,
     *                        in a listbox popping up.
     */
    virtual void setCompletionMode( TDEGlobalSettings::Completion mode );

    /**
     * Returns the current completion mode.
     *
     * The return values are of type TDEGlobalSettings::Completion.
     * See setCompletionMode() for details.
     *
     * @return the completion mode.
     */
    TDEGlobalSettings::Completion completionMode() const {
        return m_delegate ? m_delegate->completionMode() : m_iCompletionMode;
    }

    /**
     * Sets the key-binding to be used for manual text
     * completion, text rotation in a history list as
     * well as a completion list.
     *
     *
     * When the keys set by this function are pressed, a
     * signal defined by the inheriting widget will be activated.
     * If the default value or 0 is specified by the second
     * parameter, then the key-binding as defined in the global
     * setting should be used.  This method returns false value
     * for @p key is negative or the supplied key-binding conflicts
     * with the ones set for one of the other features.
     *
     * NOTE: To use a modifier key (Shift, Ctrl, Alt) as part of
     * the key-binding simply simply @p sum up the values of the
     * modifier and the actual key.  For example, to use CTRL+E as
     * a key binding for one of the items, you would simply supply
     * @p "TQt::CtrlButton + TQt::Key_E" as the second argument to this
     * function.
     *
     * @param item the feature whose key-binding needs to be set:
     *   @li TextCompletion the manual completion key-binding.
     *   @li PrevCompletionMatch    the previous match key for multiple completion.
     *   @li NextCompletionMatch    the next match key for for multiple completion.
     *   @li SubstringCompletion  the key for substring completion
     * @param key key-binding used to rotate down in a list.
     * @return true if key-binding can successfully be set.
     * @see getKeyBinding
     */
    bool setKeyBinding( KeyBindingType item , const TDEShortcut& key );

    /**
     * Returns the key-binding used for the specified item.
     *
     * This methods returns the key-binding used to activate
     * the feature feature given by @p item.  If the binding
     * contains modifier key(s), the SUM of the modifier key
     * and the actual key code are returned.
     *
     * @param item the item to check
     * @return the key-binding used for the feature given by @p item.
     * @see setKeyBinding
     */
    const TDEShortcut& getKeyBinding( KeyBindingType item ) const {
        return m_delegate ? m_delegate->getKeyBinding( item ) : m_keyMap[ item ];
    }

    /**
     * Sets this object to use global values for key-bindings.
     *
     * This method changes the values of the key bindings for
     * rotation and completion features to the default values
     * provided in TDEGlobalSettings.
     *
     * NOTE: By default inheriting widgets should uses the
     * global key-bindings so that there will be no need to
     * call this method.
     */
    void useGlobalKeyBindings();

    /**
     * A pure virtual function that must be implemented by
     * all inheriting classes.
     *
     * This function is intended to allow external completion
     * implementations to set completed text appropriately.  It
     * is mostly relevant when the completion mode is set to
     * CompletionAuto and CompletionManual modes. See
     * TDECompletionBase::setCompletedText.
     * Does nothing in CompletionPopup mode, as all available
     * matches will be shown in the popup.
     *
     * @param text the completed text to be set in the widget.
     */
    virtual void setCompletedText( const TQString& text ) = 0;

    /**
     * A pure virtual function that must be implemented by
     * all inheriting classes.
     * @param items the list of completed items
     */
    virtual void setCompletedItems( const TQStringList& items ) = 0;

    /**
     * Returns a pointer to the completion object.
     *
     * This method is only different from completionObject()
     * in that it does not create a new TDECompletion object even if
     * the internal pointer is @p NULL. Use this method to get the
     * pointer to a completion object when inheriting so that you
     * won't inadvertently create it!!
     *
     * @return the completion object or NULL if one does not exist.
     */
    TDECompletion* compObj() const { return m_delegate ? m_delegate->compObj() : (TDECompletion*) m_pCompObj; }

protected:
    /**
     * Returns a key-binding map.
     *
     * This method is the same as getKeyBinding() except it
     * returns the whole keymap containing the key-bindings.
     *
     * @return the key-binding used for the feature given by @p item.
     */
    KeyBindingMap getKeyBindings() const { return m_delegate ? m_delegate->getKeyBindings() : m_keyMap; }

    /**
     * Sets or removes the delegation object. If a delegation object is
     * set, all function calls will be forwarded to the delegation object.
     * @param delegate the delegation object, or 0 to remove it
     */
    void setDelegate( TDECompletionBase *delegate );

    /**
     * Returns the delegation object.
     * @return the delegation object, or 0 if there is none
     * @see setDelegate()
     */
    TDECompletionBase *delegate() const { return m_delegate; }

private:
    // This method simply sets the autodelete boolean for
    // the completion object, the emit signals and handle
    // signals internally flags to the provided values.
    void setup( bool, bool, bool );

    // Flag that determined whether the completion object
    // should be deleted when this object is destroyed.
    bool m_bAutoDelCompObj;
    // Determines whether this widget handles completion signals
    // internally or not
    bool m_bHandleSignals;
    // Determines whether this widget fires rotation signals
    bool m_bEmitSignals;
    // Stores the completion mode locally.
    TDEGlobalSettings::Completion m_iCompletionMode;
    // Pointer to Completion object.
    TQGuardedPtr<TDECompletion> m_pCompObj;
    // Keybindings
    KeyBindingMap m_keyMap;
    // we may act as a proxy to another TDECompletionBase object
    TDECompletionBase *m_delegate;

    // BCI
protected:
    virtual void virtual_hook( int id, void* data );
private:
    TDECompletionBasePrivate *d;
};

#endif // KCOMPLETION_H
