/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>
                  2001 Frerich Raabe <raabe@kde.org>

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

#ifndef __TDEFILEDIALOG_H__
#define __TDEFILEDIALOG_H__

#include <tqstring.h>

#include <kdialogbase.h>
#include <tdefile.h>
#include <kurl.h>
#include <kmimetype.h>
#include <tdeio/jobclasses.h>

class TQCheckBox;
class TQHBoxLayout;
class TQGridLayout;
class TQLabel;
class TQPopupMenu;
class TQVBoxLayout;

class TDEActionCollection;
class KDirOperator;
class KURLBar;
class KURLComboBox;
class KFileFilterCombo;
class KFileView;
class KFileItem;
class KPushButton;
class TDEToolBar;
class KPreviewWidgetBase;

struct KFileDialogPrivate;

/**
 * Provides a user (and developer) friendly way to
 * select files and directories.
 *
 * The widget can be used as a drop in replacement for the
 * TQFileDialog widget, but has greater functionality and a nicer GUI.
 *
 * You will usually want to use one of the static methods
 * getOpenFileName(), getSaveFileName(), getOpenURL()
 * or for multiple files getOpenFileNames() or getOpenURLs().
 *
 * The dialog has been designed to allow applications to customise it
 * by subclassing. It uses geometry management to ensure that subclasses
 * can easily add children that will be incorporated into the layout.
 *
 * \image html tdefiledialog.png "KDE File Dialog"
 *
 * @short A file selection dialog.
 *
 * @author Richard J. Moore <rich@kde.org>, Carsten Pfeiffer <pfeiffer@kde.org>
 */
class TDEIO_EXPORT KFileDialog : public KDialogBase
{
    TQ_OBJECT

public:

    /**
     * Defines some default behavior of the filedialog.
     * E.g. in mode @p Opening and @p Saving, the selected files/urls will
     * be added to the "recent documents" list. The Saving mode also implies
     * setKeepLocation() being set.
     *
     * @p Other means that no default actions are performed.
     *
     * @see setOperationMode
     * @see operationMode
     */
    enum OperationMode { Other = 0, Opening, Saving };

    /**
      * Constructs a file dialog.
      *
      * @param startDir This can either be
      *         @li The URL of the directory to start in.
      *         @li TQString::null to start in the current working
      *		    directory, or the last directory where a file has been
      *		    selected.
      *         @li ':&lt;keyword&gt;' to start in the directory last used
      *             by a filedialog in the same application that specified
      *             the same keyword.
      *         @li '::&lt;keyword&gt;' to start in the directory last used
      *             by a filedialog in any application that specified the
      *             same keyword.
      *
      * @param filter A shell glob or a mime-type-filter that specifies
      *               which files to display.
      * @param parent The parent widget of this dialog
      * @param name The name of this object
      * @param modal Whether to create a modal dialog or not
      * See setFilter() for details on how to use this argument.
      *
      */
    KFileDialog(const TQString& startDir, const TQString& filter,
		TQWidget *parent, const char *name,
		bool modal);

    /**
      * Constructs a file dialog.
      *
      * The parameters here are identical to the first constructor except
      * for the addition of a TQWidget parameter.
      *
      * Historical note: The original version of KFileDialog did not have this extra
      * parameter. It was added later, and, in order to maintain binary compatibility,
      * it was placed in a new constructor instead of added to the original one.
      *
      * @param startDir This can either be
      *         @li The URL of the directory to start in.
      *         @li TQString::null to start in the current working
      *             directory, or the last directory where a file has been
      *             selected.
      *         @li ':&lt;keyword&gt;' to start in the directory last used
      *             by a filedialog in the same application that specified
      *             the same keyword.
      *         @li '::&lt;keyword&gt;' to start in the directory last used
      *             by a filedialog in any application that specified the
      *             same keyword.
      *
      * @param filter A shell glob or a mime-type-filter that specifies
      *               which files to display.
      * See setFilter() for details on how to use this argument.
      *
      * @param widget A widget, or a widget of widgets, for displaying custom
      *               data in the dialog. This can be used, for example, to
      *               display a check box with the caption "Open as read-only".
      *               When creating this widget, you don't need to specify a parent,
      *               since the widget's parent will be set automatically by KFileDialog.
      * @param parent The parent widget of this dialog
      * @param name The name of this object
      * @param modal Whether to create a modal dialog or not
      * @since 3.1
      */
    KFileDialog(const TQString& startDir, const TQString& filter,
		TQWidget *parent, const char *name,
		bool modal, TQWidget* widget);


    /**
     * Destructs the file dialog.
     */
    ~KFileDialog();

    /**
     * @returns The selected fully qualified filename.
     */
    KURL selectedURL() const;

    /**
     * @returns The list of selected URLs.
     */
    KURL::List selectedURLs() const;

    /**
     * @returns the currently shown directory.
     */
    KURL baseURL() const;

    /**
     * Returns the full path of the selected file in the local filesystem.
     * (Local files only)
     */
    TQString selectedFile() const;

    /**
     * Returns a list of all selected local files.
     */
    TQStringList selectedFiles() const;

    /**
     * Sets the directory to view.
     *
     * @param url URL to show.
     * @param clearforward Indicates whether the forward queue
     * should be cleared.
     */
    void setURL(const KURL &url, bool clearforward = true);

    /**
     * Sets the file name to preselect to @p name
     *
     * This takes absolute URLs and relative file names.
     */
    void setSelection(const TQString& name);

    /**
     * Sets the operational mode of the filedialog to @p Saving, @p Opening
     * or @p Other. This will set some flags that are specific to loading
     * or saving files. E.g. setKeepLocation() makes mostly sense for
     * a save-as dialog. So setOperationMode( KFileDialog::Saving ); sets
     * setKeepLocation for example.
     *
     * The mode @p Saving, together with a default filter set via
     * setMimeFilter() will make the filter combobox read-only.
     *
     * The default mode is @p Opening.
     *
     * Call this method right after instantiating KFileDialog.
     *
     * @see operationMode
     * @see KFileDialog::OperationMode
     */
    void setOperationMode( KFileDialog::OperationMode );

    /**
     * @returns the current operation mode, Opening, Saving or Other. Default
     * is Other.
     *
     * @see operationMode
     * @see KFileDialog::OperationMode
     */
    OperationMode operationMode() const;

    /**
     * Sets whether the filename/url should be kept when changing directories.
     * This is for example useful when having a predefined filename where
     * the full path for that file is searched.
     *
     * This is implicitly set when operationMode() is KFileDialog::Saving
     *
     * getSaveFileName() and getSaveURL() set this to true by default, so that
     * you can type in the filename and change the directory without having
     * to type the name again.
     */
    void setKeepLocation( bool keep );

    /**
     * @returns whether the contents of the location edit are kept when
     * changing directories.
     */
    bool keepsLocation() const;

    /**
     * Sets the filter to be used to @p filter.
     *
     * You can set more
     * filters for the user to select separated by '\n'. Every
     * filter entry is defined through namefilter|text to diplay.
     * If no | is found in the expression, just the namefilter is
     * shown. Examples:
     *
     * \code
     * tdefile->setFilter("*.cpp|C++ Source Files\n*.h|Header files");
     * tdefile->setFilter("*.cpp");
     * tdefile->setFilter("*.cpp|Sources (*.cpp)");
     * tdefile->setFilter("*.cpp|" + i18n("Sources (*.cpp)"));
     * tdefile->setFilter("*.cpp|C++ Source Files\n*.h|Header files");
     * \endcode
     *
     * Note: The text to display is not parsed in any way. So, if you
     * want to show the suffix to select by a specific filter, you must
     * repeat it.
     *
     * If the filter contains an unescaped '/', a mimetype-filter is assumed.
     * If you would like a '/' visible in your filter it can be escaped with
     * a '\'. You can specify multiple mimetypes like this (separated with
     * space):
     *
     * \code
     * tdefile->setFilter( "image/png text/html text/plain" );
     * tdefile->setFilter( "*.cue|CUE\\/BIN Files (*.cue)" );
     * \endcode
     *
     * @see filterChanged
     * @see setMimeFilter
     */
    void setFilter(const TQString& filter);

    /**
     * Returns the current filter as entered by the user or one of the
     * predefined set via setFilter().
     *
     * @see setFilter()
     * @see filterChanged()
     */
    TQString currentFilter() const;

    /**
     * Sets the filter up to specify the output type.
     *
     * @param label the label to use instead of "Filter:"
     * @param types a list of mimetypes that can be used as output format
     * @param defaultType the default mimetype to use as output format.
     *
     * Do not use in conjunction with setFilter()
     * @deprecated
     */
    void setFilterMimeType(const TQString &label, const KMimeType::List &types, const KMimeType::Ptr &defaultType) TDE_DEPRECATED;

    /**
     * Returns the mimetype for the desired output format.
     *
     * This is only valid if setFilterMimeType() has been called
     * previously.
     *
     * @see setFilterMimeType()
     */
    KMimeType::Ptr currentFilterMimeType();

    /**
     * Sets the filter up to specify the output type.
     *
     * @param types a list of mimetypes that can be used as output format
     * @param defaultType the default mimetype to use as output format, if any.
     * If @p defaultType is set, it will be set as the current item.
     * Otherwise, a first item showing all the mimetypes will be created.
     * Typically, @p defaultType should be empty for loading and set for saving.
     *
     * Do not use in conjunction with setFilter()
     */
    void setMimeFilter( const TQStringList& types,
                        const TQString& defaultType = TQString::null );

    /**
     * The mimetype for the desired output format.
     *
     * This is only valid if setMimeFilter() has been called
     * previously.
     *
     * @see setMimeFilter()
     */
    TQString currentMimeFilter() const;

    /**
     *  Clears any mime- or namefilter. Does not reload the directory.
     */
    void clearFilter();

    /**
     * @deprecated
     * Add a preview widget and enter the preview mode.
     *
     * In this mode
     * the dialog is split and the right part contains your widget.
     * This widget has to inherit TQWidget and it has to implement
     * a slot showPreview(const KURL &); which is called
     * every time the file changes. You may want to look at
     * koffice/lib/kofficecore/koFilterManager.cpp for some hints :)
     *
     * Ownership is transferred to KFileDialog. You need to create the
     * preview-widget with "new", i.e. on the heap.
     */
    void setPreviewWidget(const TQWidget *w) TDE_DEPRECATED;

    /**
     * Adds a preview widget and enters the preview mode.
     *
     * In this mode the dialog is split and the right part contains your
     * preview widget.
     *
     * Ownership is transferred to KFileDialog. You need to create the
     * preview-widget with "new", i.e. on the heap.
     *
     * @param w The widget to be used for the preview.
     */
   void setPreviewWidget(const KPreviewWidgetBase *w);

    /**
     * Creates a modal file dialog and return the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li TQString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static TQString getOpenFileName(const TQString& startDir= TQString::null,
				   const TQString& filter= TQString::null,
				   TQWidget *parent= 0,
				   const TQString& caption = TQString::null);


   /**
     * Use this version only if you have no TQWidget available as
     * parent widget. This can be the case if the parent widget is
     * a widget in another process or if the parent widget is a
     * non-Qt widget. For example, in a GTK program.
     *
     * @since 3.4
    */
   static TQString getOpenFileNameWId(const TQString& startDir,
                                     const TQString& filter,
                                     WId parent_id, const TQString& caption);

    /**
     * Creates a modal file dialog and returns the selected
     * filenames or an empty list if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li TQString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static TQStringList getOpenFileNames(const TQString& startDir= TQString::null,
					const TQString& filter= TQString::null,
					TQWidget *parent = 0,
					const TQString& caption= TQString::null);



    /**
     * Creates a modal file dialog and returns the selected
     * URL or an empty string if none was chosen.
     *
     * Note that with
     * this method the user must select an existing URL.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li TQString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KURL getOpenURL(const TQString& startDir = TQString::null,
			   const TQString& filter= TQString::null,
			   TQWidget *parent= 0,
			   const TQString& caption = TQString::null);



    /**
     * Creates a modal file dialog and returns the selected
     * URLs or an empty list if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li TQString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KURL::List getOpenURLs(const TQString& startDir= TQString::null,
				  const TQString& filter= TQString::null,
				  TQWidget *parent = 0,
				  const TQString& caption= TQString::null);



    /**
     * Creates a modal file dialog and returns the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li a relative path or a filename determining the
     *             directory to start in and the file to be selected.
     *         @li TQString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static TQString getSaveFileName(const TQString& startDir= TQString::null,
				   const TQString& filter= TQString::null,
				   TQWidget *parent= 0,
				   const TQString& caption = TQString::null);
           
                   
    /**
     * This function accepts the window id of the parent window, instead
     * of TQWidget*. It should be used only when necessary.
     * @since 3.4
     */         
    static TQString getSaveFileNameWId(const TQString& dir, const TQString& filter,
                                     WId parent_id,
                                     const TQString& caption);

    /**
     * Creates a modal file dialog and returns the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li a relative path or a filename determining the
     *             directory to start in and the file to be selected.
     *         @li TQString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KURL getSaveURL(const TQString& startDir= TQString::null,
			   const TQString& filter= TQString::null,
			   TQWidget *parent= 0,
			   const TQString& caption = TQString::null);


    /**
     * Creates a modal file dialog and returns the selected
     * directory or an empty string if none was chosen.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li TQString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static TQString getExistingDirectory(const TQString & startDir = TQString::null,
					TQWidget * parent = 0,
					const TQString& caption= TQString::null);

    /**
     * Creates a modal file dialog and returns the selected
     * directory or an empty string if none was chosen.
     *
     * Contrary to getExistingDirectory(), this method allows the
     * selection of a remote directory.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li TQString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     * @since 3.1
     */
    static KURL getExistingURL(const TQString & startDir = TQString::null,
                                  TQWidget * parent = 0,
                                  const TQString& caption= TQString::null);
    /**
     * Creates a modal file dialog with an image previewer and returns the
     * selected url or an empty string if none was chosen.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li TQString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KURL getImageOpenURL( const TQString& startDir = TQString::null,
				 TQWidget *parent = 0,
				 const TQString& caption = TQString::null );
    virtual void show();

    /**
     * Convenient overload of the other setMode(unsigned int) method.
     */
    void setMode( KFile::Mode m );

    /**
     * Sets the mode of the dialog.
     *
     * The mode is defined as (in tdefile.h):
     * \code
     *    enum Mode {
     *         File         = 1,
     *         Directory    = 2,
     *         Files        = 4,
     *         ExistingOnly = 8,
     *         LocalOnly    = 16
     *    };
     * \endcode
     * You can OR the values, e.g.
     * \code
     * KFile::Mode mode = static_cast<KFile::Mode>( KFile::Files |
     *                                              KFile::ExistingOnly |
     *                                              KFile::LocalOnly );
     * setMode( mode );
     * \endcode
     */
    void setMode( unsigned int m );

    /**
     * Returns the mode of the filedialog.
     * @see setMode()
     */
    KFile::Mode mode() const;

    /**
     * Sets the text to be displayed in front of the selection.
     *
     * The default is "Location".
     * Most useful if you want to make clear what
     * the location is used for.
     */
    void setLocationLabel(const TQString& text);

    /**
     * Returns a pointer to the toolbar.
     *
     * You can use this to insert custom
     * items into it, e.g.:
     * \code
     *      yourAction = new TDEAction( i18n("Your Action"), 0,
     *                                this, TQ_SLOT( yourSlot() ),
     *                                this, "action name" );
     *      yourAction->plug( tdefileDialog->toolBar() );
     * \endcode
     */
    TDEToolBar *toolBar() const { return toolbar; }

    /**
     * @returns a pointer to the OK-Button in the filedialog. You may use it
     * e.g. to set a custom text to it.
     */
    KPushButton *okButton() const;

    /**
     * @returns a pointer to the Cancel-Button in the filedialog. You may use
     * it e.g. to set a custom text to it.
     */
    KPushButton *cancelButton() const;

    /**
     * @returns the KURLBar object used as the "speed bar". You can add custom
     * entries to it like that:
     * \code
     * KURLBar *urlBar = fileDialog->speedBar();
     * if ( urlBar )
     *     urlBar->insertDynamicItem( someURL, i18n("The URL's description") );
     * \endcode
     *
     * Note that this method may return a null-pointer if the user configured
     * to not use the speed-bar.
     * @see KURLBar
     * @see KURLBar::insertDynamicItem
     * @since 3.2
     */
    KURLBar *speedBar();

    /**
     * @returns a pointer to the action collection, holding all the used
     * TDEActions.
     */
    TDEActionCollection *actionCollection() const;

    /**
     * @returns the index of the path combobox so when inserting widgets into
     * the dialog (e.g. subclasses) they can do so without hardcoding in an index
     */
    int pathComboIndex();

    /**
     * This method implements the logic to determine the user's default directory
     * to be listed. E.g. the documents direcory, home directory or a recently
     * used directory.
     * @param startDir A url/directory, to be used. May use the ':' and '::' syntax
     *        as documented in the KFileDialog() constructor.
     * @param recentDirClass If the ':' or '::' syntax is used, recentDirClass
     *        will contain the string to be used later for TDERecentDir::dir()
     * @return The URL that should be listed by default (e.g. by KFileDialog or
     *         KDirSelectDialog).
     * @since 3.1
     */
    static KURL getStartURL( const TQString& startDir, TQString& recentDirClass );

    /**
     * @internal
     * Used by KDirSelectDialog to share the dialog's start directory.
     */
    static void setStartDir( const KURL& directory );

signals:
    /**
      * Emitted when the user selects a file. It is only emitted in single-
      * selection mode. The best way to get notified about selected file(s)
      * is to connect to the okClicked() signal inherited from KDialogBase
      * and call selectedFile(), selectedFiles(),
      * selectedURL() or selectedURLs().
      */
    void fileSelected(const TQString&);

    /**
      * Emitted when the user highlights a file.
      */
    void fileHighlighted(const TQString&);

    /**
     * Emitted when the user hilights one or more files in multiselection mode.
     *
     * Note: fileHighlighted() or fileSelected() are @em not
     * emitted in multiselection mode. You may use selectedItems() to
     * ask for the current highlighted items.
     * @see fileSelected
     */
    void selectionChanged();

    /**
     * Emitted when the filter changed, i.e. the user entered an own filter
     * or chose one of the predefined set via setFilter().
     *
     * @param filter contains the new filter (only the extension part,
     * not the explanation), i.e. "*.cpp".
     *
     * @see setFilter()
     * @see currentFilter()
     */
    void filterChanged( const TQString& filter );

protected:
    TDEToolBar *toolbar;

    static KURL *lastDirectory;

    KURLComboBox *locationEdit;

    KFileFilterCombo *filterWidget;

    /**
     * Reimplemented to animate the cancel button.
     */
    virtual void keyPressEvent( TQKeyEvent *e );

    /**
      * Perform basic initialization tasks. Called by constructors.
      * @since 3.1
      */
    void init(const TQString& startDir, const TQString& filter, TQWidget* widget);

    /**
      * rebuild geometry management.
      *
      */
    virtual void initGUI();

    /**
     * called when an item is highlighted/selected in multiselection mode.
     * handles setting the locationEdit.
     */
    void multiSelectionChanged();

    /**
     * Reads configuration and applies it (size, recent directories, ...)
     */
    virtual void readConfig( TDEConfig *, const TQString& group = TQString::null );

    /**
     * Saves the current configuration
     */
    virtual void writeConfig( TDEConfig *, const TQString& group = TQString::null );

    /**
     * Reads the recent used files and inserts them into the location combobox
     */
    virtual void readRecentFiles( TDEConfig * );

    /**
     * Saves the entries from the location combobox.
     */
    virtual void saveRecentFiles( TDEConfig * );

    /**
     * Parses the string "line" for files. If line doesn't contain any ", the
     * whole line will be interpreted as one file. If the number of " is odd,
     * an empty list will be returned. Otherwise, all items enclosed in " "
     * will be returned as correct urls.
     */
    KURL::List tokenize(const TQString& line) const;

    /**
     * Returns the absolute version of the URL specified in locationEdit.
     * @since 3.2
     */
    KURL getCompleteURL(const TQString&);

    /**
     * Returns the filename extension associated with the currentFilter().
     * TQString::null is returned if an extension is not available or if
     * operationMode() != Saving.
     * @since 3.2
     */
    TQString currentFilterExtension();

    /**
     * Updates the currentFilterExtension and the availability of the
     * Automatically Select Extension Checkbox (visible if operationMode()
     * == Saving and enabled if an extension _will_ be associated with the
     * currentFilter(), _after_ this call).  You should call this after
     * filterWidget->setCurrentItem().
     * @since 3.2
     */
    void updateAutoSelectExtension();


protected slots:
    void urlEntered( const KURL& );
    void enterURL( const KURL& url );
    void enterURL( const TQString& url );
    void locationActivated( const TQString& url );

    /**
     * @deprecated,
     */
    // ### remove in KDE4
    void toolbarCallback(int);
    /**
     * @deprecated
     */
    // ### remove in KDE4
    void pathComboChanged( const TQString& );
    /**
     * @deprecated
     */
    // ### remove in KDE4
    void dirCompletion( const TQString& );

    void slotFilterChanged();
    void fileHighlighted(const KFileItem *i);
    void fileSelected(const KFileItem *i);
    void slotStatResult(TDEIO::Job* job);
    void slotLoadingFinished();

    void fileCompletion( const TQString& );
    /**
     * @since 3.1
     */
    void toggleSpeedbar( bool );

    /**
     * @since 3.4
     */
    void toggleBookmarks(bool show);

    /**
     * @deprecated
     */
    virtual void updateStatusLine(int dirs, int files);

    virtual void slotOk();
    virtual void accept();
    virtual void slotCancel();

    void slotAutoSelectExtClicked();
    void addToRecentDocuments();
    void initSpeedbar();

private slots:
    void slotLocationChanged( const TQString& text );

private:
    KFileDialog(const KFileDialog&);
    KFileDialog operator=(const KFileDialog&);

    void setLocationText( const TQString& text );
    void updateLocationWhatsThis();

    void appendExtension(KURL &url);
    void updateLocationEditExtension(const TQString &);
    void updateFilter();

    static void initStatic();

    void setNonExtSelection();

protected:
    KDirOperator *ops;
    bool autoDirectoryFollowing;

    KURL::List& parseSelectedURLs() const;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    KFileDialogPrivate *d;
};

#endif
