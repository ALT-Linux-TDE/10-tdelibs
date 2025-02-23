/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>.
    Copyright (C) 2002, David Faure <david@mandrakesoft.com>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KFINDDIALOG_H
#define KFINDDIALOG_H

#include <kdialogbase.h>
class KHistoryCombo;
class TQPushButton;
class TQPopupMenu;
class TQGridLayout;
class TQLabel;
class TQGroupBox;
class TQCheckBox;

/**
 * @ingroup main
 * @ingroup findreplace
 * @brief A generic "find" dialog.
 *
 * @author S.R.Haque <srhaque@iee.org>
 *
 * \b Detail:
 *
 * This widget inherits from KDialogBase and implements
 * the following additional functionalities:  a find string
 * object and an area for a user-defined widget to extend the dialog.
 *
 * \b Example:
 *
 * To use the basic modal find dialog, and then run the search:
 *
 * \code
 *  KFindDialog dlg(....)
 *  if ( dlg.exec() != TQDialog::Accepted )
 *      return;
 *
 *  // proceed with KFind from here
 * \endcode
 *
 * To create a non-modal find dialog:
 * \code
 *   if ( m_findDia )
 *     KWin::setActiveWindow( m_findDia->winId() );
 *   else
 *   {
 *     m_findDia = new KFindDialog(false,...);
 *     connect( m_findDia, TQ_SIGNAL(okClicked()), this, TQ_SLOT(findTextNext()) );
 *   }
 * \endcode
 * Don't forget to delete and reset m_findDia when closed.
 * (But do NOT delete your KFind object at that point, it's needed for "Find Next")
 *
 * To use your own extensions: see findExtension().
 *
 */
class TDEUTILS_EXPORT KFindDialog:
    public KDialogBase
{
    TQ_OBJECT

public:

    // Options.

    // KDE4: move to KFind
    /**
     * Options for the search
     */
    enum Options
    {
        WholeWordsOnly = 1,     ///< Match whole words only.
        FromCursor = 2,         ///< Start from current cursor position.
        SelectedText = 4,       ///< Only search selected area.
        CaseSensitive = 8,      ///< Consider case when matching.
        FindBackwards = 16,     ///< Go backwards.
        RegularExpression = 32, ///< Interpret the pattern as a regular expression.
        FindIncremental = 64,   ///< Find incremental.
        // Note that KReplaceDialog uses 256 and 512
        // User extensions can use boolean options above this value.
        MinimumUserOption = 65536 ///< The first flag which can be used by extensions.
    };

    /**
     * Construct a modal find dialog
     *
     * @param parent The parent object of this widget.
     * @param name The name of this widget.
     * @param options A bitfield of the Options to be checked.
     * @param findStrings The find history, see findHistory()
     * @param hasSelection Whether a selection exists
     */
    KFindDialog( TQWidget *parent = 0, const char *name = 0, long options = 0,
                 const TQStringList &findStrings = TQStringList(), bool hasSelection = false );
    // KDE4: fix ambiguity with private constructor
    // Maybe remove options (there's setOptions) and findStrings (setFindHistory) and hasSelection (setHasSelection)

    /**
     * Construct a non-modal find dialog
     *
     * @param modal set to @c false to get a non-modal dialog
     * @param parent The parent object of this widget.
     * @param name The name of this widget.
     * @param options A bitfield of the Options to be checked.
     * @param findStrings The find history, see findHistory()
     * @param hasSelection Whether a selection exists
     */
    KFindDialog( bool modal, TQWidget *parent = 0, const char *name = 0, long options = 0,
                 const TQStringList &findStrings = TQStringList(), bool hasSelection = false );
    // KDE4: consider simplifying

    /**
     * Destructor.
     */
    virtual ~KFindDialog();

    /**
     * Provide the list of @p strings to be displayed as the history
     * of find strings. @p strings might get truncated if it is
     * too long.
     *
     * @param history The find history.
     * @see findHistory
     */
    void setFindHistory( const TQStringList &history );

    /**
     * Returns the list of history items.
     *
     * @return The find history.
     * @see setFindHistory
     */
    TQStringList findHistory() const;

    /**
     * Enable/disable the 'search in selection' option, depending
     * on whether there actually is a selection.
     *
     * @param hasSelection @c true if a selection exists
     */
    void setHasSelection( bool hasSelection );

    /**
     * Hide/show the 'from cursor' option, depending
     * on whether the application implements a cursor.
     *
     * @param hasCursor @c true if the application features a cursor
     * This is assumed to be the case by default.
     */
    void setHasCursor( bool hasCursor );

     /**
     * Enable/disable the 'Find backwards' option, depending
     * on whether the application supports it.
     *
     * @param supports @c true if the application supports backwards find
     * This is assumed to be the case by default.
     * @since 3.4
     */
    void setSupportsBackwardsFind( bool supports );

     /**
     * Enable/disable the 'Case sensitive' option, depending
     * on whether the application supports it.
     *
     * @param supports @c true if the application supports case sensitive find
     * This is assumed to be the case by default.
     * @since 3.4
     */
    void setSupportsCaseSensitiveFind( bool supports );

     /**
     * Enable/disable the 'Whole words only' option, depending
     * on whether the application supports it.
     *
     * @param supports @c true if the application supports whole words only find
     * This is assumed to be the case by default.
     * @since 3.4
     */
    void setSupportsWholeWordsFind( bool supports );

     /**
     * Enable/disable the 'Regular expression' option, depending
     * on whether the application supports it.
     *
     * @param supports @c true if the application supports regular expression find
     * This is assumed to be the case by default.
     * @since 3.4
     */
    void setSupportsRegularExpressionFind( bool supports );

    /**
     * Set the options which are checked.
     *
     * @param options The setting of the Options.
     * @see Options
     */
    void setOptions( long options );

    /**
     * Returns the state of the options. Disabled options may be returned in
     * an indeterminate state.
     *
     * @return The options.
     * @see Options, setOptions
     */
    long options() const;

    /**
     * Returns the pattern to find.
     * @return The search text.
     */
    TQString pattern() const;

    /**
     * Sets the pattern to find.
     * @param pattern The new search pattern.
     */
    void setPattern ( const TQString &pattern );

    /**
     * Returns an empty widget which the user may fill with additional UI
     * elements as required. The widget occupies the width of the dialog,
     * and is positioned immediately below the regular expression support
     * widgets for the pattern string.
     * @return An extensible TQWidget.
     */
    TQWidget *findExtension();

protected slots:

    void slotOk();
    void slotSelectedTextToggled(bool);
    void showPatterns();
    void showPlaceholders();
    void textSearchChanged(const TQString &);
    void slotRegexCheckBoxToggled(bool checked);
    void slotPlaceholdersCheckBoxToggled(bool checked);

protected:
    virtual void showEvent ( TQShowEvent * );

private slots:
    /**
     * connected to the aboutToShow of the placeholders menu,
     * updates it according to the text in the pattern.
     */
    void slotPlaceholdersAboutToShow();

private:

    TQGroupBox *m_findGrp;
    TQLabel *m_findLabel;
    KHistoryCombo *m_find;
    TQCheckBox *m_regExp;
    TQPushButton *m_regExpItem;
    TQGridLayout *m_findLayout;
    TQWidget *m_findExtension;

    TQGroupBox *m_optionGrp;
    TQCheckBox *m_wholeWordsOnly;
    TQCheckBox *m_fromCursor;
    TQCheckBox *m_selectedText;
    TQCheckBox *m_caseSensitive;
    TQCheckBox *m_findBackwards;

    TQPopupMenu *m_patterns;

    // Our dirty little secret is that we also implement the "replace" dialog. But we
    // keep that fact hidden from all but our friends.

    friend class KReplaceDialog;

    /**
     * Construct a find dialog with a parent object and a name. This version of the
     * constructor is for use by friends only!
     *
     * @param forReplace Is this a replace dialog?
     */
    KFindDialog( TQWidget *parent, const char *name, bool forReplace );
    void init( bool forReplace, const TQStringList &findStrings, bool hasSelection );

    TQGroupBox *m_replaceGrp;
    TQLabel *m_replaceLabel;
    KHistoryCombo *m_replace;
    TQCheckBox* m_backRef;
    TQPushButton* m_backRefItem;
    TQGridLayout *m_replaceLayout;
    TQWidget *m_replaceExtension;

    TQCheckBox* m_promptOnReplace;

    TQPopupMenu *m_placeholders;

    // Binary compatible extensibility.
    class KFindDialogPrivate;
    KFindDialogPrivate *d;
};

#endif // KFINDDIALOG_H
