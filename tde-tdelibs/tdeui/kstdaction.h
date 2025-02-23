/* This file is part of the KDE libraries
   Copyright (C) 1999,2000 Kurt Granroth <granroth@kde.org>
   Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

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
#ifndef KSTDACTION_H
#define KSTDACTION_H

class TQObject;
class TQWidget;
class TDEAction;
class TDEActionCollection;
class TDERecentFilesAction;
class TDEToggleAction;
class TDEToggleToolBarAction;
class TDEToggleFullScreenAction;

#include <tqstringlist.h>

#include <tdelibs_export.h>

/**
 * Convenience methods to access all standard KDE actions.
 *
 * These actions should be used instead of hardcoding menubar and
 * toolbar items.  Using these actions helps your application easily
 * conform to the KDE UI Style Guide
 * @see http://developer.kde.org/documentation/standards/kde/style/basics/index.html .
 *
 * All of the documentation for TDEAction holds for KStdAction
 * also.  When in doubt on how things work, check the TDEAction
 * documention first.
 *
 * <b>Simple Example:</b>\n
 *
 * In general, using standard actions should be a drop in replacement
 * for regular actions.  For example, if you previously had:
 *
 * \code
 * TDEAction *newAct = new TDEAction(i18n("&New"), TQIconSet(BarIcon("document-new")),
 *                               TDEStdAccel::shortcut(TDEStdAccel::New), this,
 *                               TQ_SLOT(fileNew()), actionCollection());
 * \endcode
 *
 * You could drop that and replace it with:
 *
 * \code
 * TDEAction *newAct = KStdAction::openNew(this, TQ_SLOT(fileNew()),
 *                                       actionCollection());
 * \endcode
 *
 * <b>Non-standard Usages</b>\n
 *
 * It is possible to use the standard actions in various
 * non-recommended ways.  Say, for instance, you wanted to have a
 * standard action (with the associated correct text and icon and
 * accelerator, etc) but you didn't want it to go in the standard
 * place (this is not recommended, by the way).  One way to do this is
 * to simply not use the XML UI framework and plug it into wherever
 * you want.  If you do want to use the XML UI framework (good!), then
 * it is still possible.
 *
 * Basically, the XML building code matches names in the XML code with
 * the internal names of the actions.  You can find out the internal
 * names of each of the standard actions by using the stdName
 * action like so: KStdAction::stdName(KStdAction::Cut) would return
 * 'edit_cut'.  The XML building code will match 'edit_cut' to the
 * attribute in the global XML file and place your action there.
 *
 * However, you can change the internal name.  In this example, just
 * do something like:
 *
 * \code
 * (void)KStdAction::cut(this, TQ_SLOT(editCut()), actionCollection(), "my_cut");
 * \endcode
 *
 * Now, in your local XML resource file (e.g., yourappui.rc), simply
 * put 'my_cut' where you want it to go.
 *
 * Another non-standard usage concerns getting a pointer to an
 * existing action if, say, you want to enable or disable the action.
 * You could do it the recommended way and just grab a pointer when
 * you instantiate it as in the the 'openNew' example above... or you
 * could do it the hard way:
 *
 * \code
 * TDEAction *cut = actionCollection()->action(KStdAction::stdName(KStdAction::Cut));
 * \endcode
 *
 * Another non-standard usage concerns instantiating the action in the
 * first place.  Usually, you would use the member functions as
 * shown above (e.g., KStdAction::cut(this, TQ_SLOT, parent)).  You
 * may, however, do this using the enums provided.  This author can't
 * think of a reason why you would want to, but, hey, if you do,
 * here's how:
 *
 * \code
 * (void)KStdAction::action(KStdAction::New, this, TQ_SLOT(fileNew()), actionCollection());
 * (void)KStdAction::action(KStdAction::Cut, this, TQ_SLOT(editCut()), actionCollection());
 * \endcode
 *
 * @author Kurt Granroth <granroth@kde.org>
 */
namespace KStdAction
{
	/**
	 * The standard menubar and toolbar actions.
	 */
	enum StdAction {
		ActionNone,

		// File Menu
		New, Open, OpenRecent, Save, SaveAs, Revert, Close,
		Print, PrintPreview, Mail, Quit,

		// Edit Menu
		Undo, Redo, Cut, Copy, Paste, SelectAll, Deselect, Find, FindNext, FindPrev,
		Replace,

		// View Menu
		ActualSize, FitToPage, FitToWidth, FitToHeight, ZoomIn, ZoomOut,
		Zoom, Redisplay,

		// Go Menu
		Up, Back, Forward, Home, Prior, Next, Goto, GotoPage, GotoLine,
		FirstPage, LastPage,

		// Bookmarks Menu
		AddBookmark, EditBookmarks,

		// Tools Menu
		Spelling,

		// Settings Menu
		ShowMenubar, ShowToolbar, ShowStatusbar,
		SaveOptions, KeyBindings,
		Preferences, ConfigureToolbars,

		// Help Menu
		Help, HelpContents, WhatsThis, ReportBug, AboutApp, AboutKDE,
		TipofDay, ///< @since 3.1

		// Another settings menu item
		ConfigureNotifications,
		FullScreen, ///< @since 3.2
		Clear, ///< @since 3.2
		PasteText, ///< @since 3.2
		SwitchApplicationLanguage ///< @since 3.5.8
	};

	/**
	 * Creates an action corresponding to the
	 * KStdAction::StdAction enum.
	 */
	TDEUI_EXPORT TDEAction* create( StdAction id, const char *name,
		const TQObject *recvr, const char *slot,
		TDEActionCollection* parent );

	inline TDEAction* create( StdAction id,
		const TQObject *recvr, const char *slot,
		TDEActionCollection* parent )
		{ return KStdAction::create( id, 0, recvr, slot, parent ); }

	/**
	* @obsolete. Creates an action corresponding to the
	* KStdAction::StdAction enum.
	*/
	inline TDEAction *action(StdAction act_enum,
		const TQObject *recvr, const char *slot,
		TDEActionCollection *parent, const char *name = 0L )
		{ return KStdAction::create( act_enum, name, recvr, slot, parent ); }

	/**
	 * This will return the internal name of a given standard action.
	 */
	TDEUI_EXPORT const char* name( StdAction id );

        /// @obsolete. Use name()
	inline const char* stdName(StdAction act_enum) { return name( act_enum ); }

       /**
         * Returns a list of all standard names. Used by TDEAccelManager
         * to give those heigher weight.
	 * @since 3.1
        */
        TDEUI_EXPORT TQStringList stdNames();

	/**
	 * Create a new document or window.
	 */
	TDEUI_EXPORT TDEAction *openNew(const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name = 0 );

	/**
	 * Open an existing file.
	 */
	TDEUI_EXPORT TDEAction *open(const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name = 0 );

	/**
	 * Open a recently used document. The signature of the slot being called
	 * is of the form slotURLSelected( const KURL & ).
	 * @param recvr object to receive slot
	 * @param slot The slot to invoke when a URL is selected. The slot's
	 * signature is slotURLSelected( const KURL & ).
	 * @param parent parent widget
	 * @param name name of widget
	 */
	TDEUI_EXPORT TDERecentFilesAction *openRecent(const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name = 0 );

	/**
	 * Save the current document.
	 */
	TDEUI_EXPORT TDEAction *save(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Save the current document under a different name.
	*/
	TDEUI_EXPORT TDEAction *saveAs(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Revert the current document to the last saved version
	* (essentially will undo all changes).
	*/
	TDEUI_EXPORT TDEAction *revert(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Close the current document.
	*/
	TDEUI_EXPORT TDEAction *close(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Print the current document.
	*/
	TDEUI_EXPORT TDEAction *print(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Show a print preview of the current document.
	*/
	TDEUI_EXPORT TDEAction *printPreview(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Mail this document.
	*/
	TDEUI_EXPORT TDEAction *mail(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Quit the program.
	*/
	TDEUI_EXPORT TDEAction *quit(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Undo the last operation.
	*/
	TDEUI_EXPORT TDEAction *undo(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Redo the last operation.
	*/
	TDEUI_EXPORT TDEAction *redo(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Cut selected area and store it in the clipboard.
	*/
	TDEUI_EXPORT TDEAction *cut(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Copy the selected area into the clipboard.
	*/
	TDEUI_EXPORT TDEAction *copy(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Paste the contents of clipboard at the current mouse or cursor
	* position.
	*/
	TDEUI_EXPORT TDEAction *paste(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Paste the contents of clipboard at the current mouse or cursor
	* position. Provide a button on the toolbar with the clipboard history
	* menu if Klipper is running.
	* @since 3.2
	*/
	TDEUI_EXPORT TDEAction *pasteText(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Clear the content of the focus widget
	* @since 3.2
	*/
	TDEUI_EXPORT TDEAction *clear(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Select all elements in the current document.
	*/
	TDEUI_EXPORT TDEAction *selectAll(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Deselect any selected elements in the current document.
	*/
	TDEUI_EXPORT TDEAction *deselect(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Initiate a 'find' request in the current document.
	*/
	TDEUI_EXPORT TDEAction *find(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Find the next instance of a stored 'find'.
	*/
	TDEUI_EXPORT TDEAction *findNext(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Find a previous instance of a stored 'find'.
	*/
	TDEUI_EXPORT TDEAction *findPrev(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Find and replace matches.
	*/
	TDEUI_EXPORT TDEAction *replace(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* View the document at its actual size.
	*/
	TDEUI_EXPORT TDEAction *actualSize(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Fit the document view to the size of the current window.
	*/
	TDEUI_EXPORT TDEAction *fitToPage(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Fit the document view to the width of the current window.
	*/
	TDEUI_EXPORT TDEAction *fitToWidth(const TQObject *recvr, const char *slot,
		TDEActionCollection* parent, const char *name = 0 );

	/**
	* Fit the document view to the height of the current window.
	*/
	TDEUI_EXPORT TDEAction *fitToHeight(const TQObject *recvr, const char *slot,
					TDEActionCollection* parent, const char *name = 0 );

	/**
	* Zoom in.
	*/
	TDEUI_EXPORT TDEAction *zoomIn(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Zoom out.
	*/
	TDEUI_EXPORT TDEAction *zoomOut(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Popup a zoom dialog.
	*/
	TDEUI_EXPORT TDEAction *zoom(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Redisplay or redraw the document.
	*/
	TDEUI_EXPORT TDEAction *redisplay(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Move up (web style menu).
	*/
	TDEUI_EXPORT TDEAction *up(const TQObject *recvr, const char *slot,
			TDEActionCollection* parent, const char *name = 0 );

	/**
	* Move back (web style menu).
	*/
	TDEUI_EXPORT TDEAction *back(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Move forward (web style menu).
	*/
	TDEUI_EXPORT TDEAction *forward(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Go to the "Home" position or document.
	*/
	TDEUI_EXPORT TDEAction *home(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Scroll up one page.
	*/
	TDEUI_EXPORT TDEAction *prior(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Scroll down one page.
	*/
	TDEUI_EXPORT TDEAction *next(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Go to somewhere in general.
	*/
	TDEUI_EXPORT TDEAction *goTo(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );


	/**
	* Go to a specific page (dialog).
	*/
	TDEUI_EXPORT TDEAction *gotoPage(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Go to a specific line (dialog).
	*/
	TDEUI_EXPORT TDEAction *gotoLine(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Jump to the first page.
	*/
	TDEUI_EXPORT TDEAction *firstPage(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Jump to the last page.
	*/
	TDEUI_EXPORT TDEAction *lastPage(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Add the current page to the bookmarks tree.
	*/
	TDEUI_EXPORT TDEAction *addBookmark(const TQObject *recvr, const char *slot,
					TDEActionCollection* parent, const char *name = 0 );

	/**
	* Edit the application bookmarks.
	*/
	TDEUI_EXPORT TDEAction *editBookmarks(const TQObject *recvr, const char *slot,
					TDEActionCollection* parent, const char *name = 0 );

	/**
	* Pop up the spell checker.
	*/
	TDEUI_EXPORT TDEAction *spelling(const TQObject *recvr, const char *slot,
					TDEActionCollection* parent, const char *name = 0 );


	/**
	* Show/Hide the menubar.
	*/
	TDEUI_EXPORT TDEToggleAction *showMenubar(const TQObject *recvr, const char *slot,
					TDEActionCollection* parent, const char *name = 0 );

	/**
	* @obsolete. toolbar actions are created automatically now in the
	* Settings menu. Don't use this anymore.
	* See: TDEMainWindow::createStandardStatusBarAction()
	* Show/Hide the primary toolbar.
	* @since 3.1
	*/
	TDEUI_EXPORT TDEToggleAction *showToolbar(const TQObject *recvr, const char *slot,
					TDEActionCollection* parent, const char *name = 0 ) TDE_DEPRECATED;
	/**
	* @obsolete. toolbar actions are created automatically now in the
	* Settings menu. Don't use this anymore.
	* See: TDEMainWindow::setStandardToolBarMenuEnabled(bool);
	* Show/Hide the primary toolbar.
	*/
	TDEUI_EXPORT TDEToggleToolBarAction *showToolbar(const char* toolBarName,
					TDEActionCollection* parent, const char *name = 0 ) TDE_DEPRECATED;

	/**
	* Show/Hide the statusbar.
	*/
	TDEUI_EXPORT TDEToggleAction *showStatusbar(const TQObject *recvr, const char *slot,
					TDEActionCollection* parent, const char *name = 0 );

	/**
	* Switch to/from full screen mode
	* @since 3.2
	*/
	TDEUI_EXPORT TDEToggleFullScreenAction *fullScreen(const TQObject *recvr, const char *slot,
					TDEActionCollection* parent, TQWidget* window, const char *name = 0 );

	/**
	* Display the save options dialog.
	*/
	TDEUI_EXPORT TDEAction *saveOptions(const TQObject *recvr, const char *slot,
					TDEActionCollection* parent, const char *name = 0 );

	/**
	* Display the configure key bindings dialog.
	*
	*  Note that you might be able to use the pre-built KXMLGUIFactory's fuction:
	*  KStdAction::keyBindings(guiFactory(), TQ_SLOT(configureShortcuts()), actionCollection());
        */
	TDEUI_EXPORT TDEAction *keyBindings(const TQObject *recvr, const char *slot,
					TDEActionCollection* parent, const char *name = 0 );

	/**
	* Display the preferences/options dialog.
	*/
	TDEUI_EXPORT TDEAction *preferences(const TQObject *recvr, const char *slot,
					TDEActionCollection* parent, const char *name = 0 );

	/**
	* The Customize Toolbar dialog.
	*/
	TDEUI_EXPORT TDEAction *configureToolbars(const TQObject *recvr,
					const char *slot,
					TDEActionCollection* parent,
					const char *name = 0 );

	/**
	* The Configure Notifications dialog.
	* @since 3.1
	*/
	TDEUI_EXPORT TDEAction *configureNotifications(const TQObject *recvr,
					const char *slot,
					TDEActionCollection *parent,
					const char *name = 0);

	/**
	* Display the help.
	*/
	TDEUI_EXPORT TDEAction *help(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Display the help contents.
	*/
	TDEUI_EXPORT TDEAction *helpContents(const TQObject *recvr, const char *slot,
					TDEActionCollection* parent, const char *name = 0 );

	/**
	* Trigger the What's This cursor.
	*/
	TDEUI_EXPORT TDEAction *whatsThis(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Display "Tip of the Day"
	* @since 3.1
	*/
	TDEUI_EXPORT TDEAction *tipOfDay(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Open up the Report Bug dialog.
	*/
	TDEUI_EXPORT TDEAction *reportBug(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Display the application's About box.
	*/
	TDEUI_EXPORT TDEAction *aboutApp(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Display the About KDE dialog.
	*/
	TDEUI_EXPORT TDEAction *aboutKDE(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );

	/**
	* Display "Switch application language" dialog.
	* @since 3.5.8
	*/
	TDEUI_EXPORT TDEAction *switchApplicationLanguage(const TQObject *recvr, const char *slot,
				TDEActionCollection* parent, const char *name = 0 );
}

#endif // KSTDACTION_H
