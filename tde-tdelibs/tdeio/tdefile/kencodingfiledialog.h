/* This file is part of the KDE libraries
    Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
		  2003 Andras Mantia <amantia@freemail.hu>

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

#ifndef __KENCODINGFILEDIALOG_H__
#define __KENCODINGFILEDIALOG_H__

#include <tdefiledialog.h>

struct KEncodingFileDialogPrivate;

/**
 * Provides a user (and developer) friendly way to
 * select files with support for choosing encoding
 *
 *
 * The dialog has been designed to allow applications to customise it
 * by subclassing. It uses geometry management to ensure that subclasses
 * can easily add children that will be incorporated into the layout.
 */

class TDEIO_EXPORT KEncodingFileDialog : public KFileDialog
{
    TQ_OBJECT

public:
    class Result {
	public:
		TQStringList fileNames;
		KURL::List  URLs;
		TQString encoding;
    };

    /**
      * Constructs a file dialog for text files with encoding selection possibility.
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
      * @param encoding The encoding shown in the encoding combo. If it's
      *		    TQString::null, the global default encoding will be shown.
      *
      * @param filter This is a space separated list of shell globs.
      *             You can set the text to be displayed for the glob, and
      *             provide multiple globs.  See setFilter() for details on
      *             how to do this...
      *
      * @param caption The caption of the dialog
      *
      * @param type This can either be
      *		@li Opening (open dialog, the default setting)
      *		@li Saving
      * @param parent The parent widget of this dialog
      * @param name The name of this object
      * @param modal Whether to create a modal dialog or not
      *
      * @since 3.2
      */
    KEncodingFileDialog (const TQString& startDir = TQString::null,
                    const TQString& encoding = TQString::null,
		    const TQString& filter = TQString::null,
		    const TQString& caption = TQString::null, KFileDialog::OperationMode type = KFileDialog::Opening,
                    TQWidget *parent= 0, const char *name="", bool modal = true);
    /**
     * Destructs the file dialog.
     */
    ~KEncodingFileDialog();


    /**
    * @returns The selected encoding if the constructor with the encoding parameter was used, otherwise TQString::null.
    */
    TQString selectedEncoding() const;


    /**
     * Creates a modal file dialog and return the selected
     * filename or an empty string if none was chosen additionally a chosen
     * encoding value is returned.
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
     * @param encoding The encoding shown in the encoding combo.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static Result getOpenFileNameAndEncoding(const TQString& encoding=TQString::null,
   				   const TQString& startDir= TQString::null,
				   const TQString& filter= TQString::null,
				   TQWidget *parent= 0,
				   const TQString& caption = TQString::null);

    /**
     * Creates a modal file dialog and returns the selected encoding and the selected
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
     * @param encoding The encoding shown in the encoding combo.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static Result getOpenFileNamesAndEncoding(const TQString& encoding=TQString::null,
					const TQString& startDir= TQString::null,
					const TQString& filter= TQString::null,
					TQWidget *parent = 0,
					const TQString& caption= TQString::null);

    /**
     * Creates a modal file dialog and returns the selected encoding and
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
     * @param encoding The encoding shown in the encoding combo.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static Result getOpenURLAndEncoding(const TQString& encoding=TQString::null,
			   const TQString& startDir = TQString::null,
			   const TQString& filter= TQString::null,
			   TQWidget *parent= 0,
			   const TQString& caption = TQString::null);




    /**
     * Creates a modal file dialog and returns the selected encoding
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
     * @param encoding The encoding shown in the encoding combo.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static Result getOpenURLsAndEncoding(const TQString& encoding=TQString::null,
				  const TQString& startDir= TQString::null,
				  const TQString& filter= TQString::null,
				  TQWidget *parent = 0,
				  const TQString& caption= TQString::null);



    /**
     * Creates a modal file dialog and returns the selected encoding and
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
     * @param encoding The encoding shown in the encoding combo.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static Result getSaveFileNameAndEncoding(const TQString& encoding=TQString::null,
				   const TQString& startDir=TQString::null,
				   const TQString& filter= TQString::null,
				   TQWidget *parent= 0,
				   const TQString& caption = TQString::null);


    /**
     * Creates a modal file dialog and returns the selected encoding and
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
     * @param encoding The encoding shown in the encoding combo.
     * @param filter This is a space separated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static Result getSaveURLAndEncoding(const TQString& encoding=TQString::null,
			   const TQString& startDir= TQString::null,
			   const TQString& filter= TQString::null,
			   TQWidget *parent= 0,
			   const TQString& caption = TQString::null);



protected:
    virtual void virtual_hook( int id, void* data );
private:
    KEncodingFileDialogPrivate *d;
};

#endif
