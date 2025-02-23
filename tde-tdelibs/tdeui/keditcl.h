/* This file is part of the KDE libraries

   Copyright (C) 1996 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
   Copyright (C) 2000 Waldo Bastian <bastian@kde.org>

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
#ifndef __KEDITCL_H__
#define __KEDITCL_H__

#include <tqmultilineedit.h>
#include <tqstring.h>
#include <kdialogbase.h>

class TQDropEvent;
class TQPushButton;
class TQCheckBox;
class TQRadioButton;
class TQTextStream;
class KHistoryCombo;
class KIntNumInput;
class TQVButtonGroup;

class TDEUI_EXPORT KEdGotoLine : public KDialogBase
{
    TQ_OBJECT

public:
    KEdGotoLine( TQWidget *parent=0, const char *name=0, bool modal=true );
    int getLineNumber();

public slots:
    void selected( int );

private:
    KIntNumInput *lineNum;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KEdGotoLinePrivate;
    KEdGotoLinePrivate *d;
};

///
class TDEUI_EXPORT KEdFind : public KDialogBase
{
    TQ_OBJECT
    TQ_PROPERTY( TQString text READ getText WRITE setText )
    TQ_PROPERTY( bool caseSensitivity READ case_sensitive WRITE setCaseSensitive )
    TQ_PROPERTY( bool direction READ get_direction WRITE setDirection )
public:

    KEdFind( TQWidget *parent = 0, const char *name=0, bool modal=true);
    ~KEdFind();

    TQString getText() const;
    void setText(TQString string);
    void setCaseSensitive( bool b );
    bool case_sensitive() const;
    void setDirection( bool b );
    bool get_direction() const;

    /**
     * @returns the combobox containing the history of searches. Can be used
     * to save and restore the history.
     */
    KHistoryCombo *searchCombo() const;

protected slots:
    void slotCancel( void );
    void slotUser1( void );
    void textSearchChanged ( const TQString & );

protected:
  TQVButtonGroup* group;

private:
    TQCheckBox *sensitive;
    TQCheckBox *direction;

    virtual void done(int i ) { KDialogBase::done(i); }

signals:

    void search();
    void done();
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KEdFindPrivate;
    KEdFindPrivate *d;
};

///
class TDEUI_EXPORT KEdReplace : public KDialogBase
{
    TQ_OBJECT

public:

    KEdReplace ( TQWidget *parent = 0, const char *name=0, bool modal=true );
    ~KEdReplace();

    TQString 	getText();
    TQString 	getReplaceText();
    void 	setText(TQString);

    /**
     * @returns the combobox containing the history of searches. Can be used
     * to save and restore the history.
     */
    KHistoryCombo *searchCombo() const;

    /**
     * @returns the combobox containing the history of replaces. Can be used
     * to save and restore the history.
     */
    KHistoryCombo *replaceCombo() const;

    bool 	case_sensitive();
    bool 	get_direction();

protected slots:
    void slotCancel( void );
    void slotClose( void );
    void slotUser1( void );
    void slotUser2( void );
    void slotUser3( void );
    void textSearchChanged ( const TQString & );

private:
    TQCheckBox 	*sensitive;
    TQCheckBox 	*direction;

	virtual void done(int i ) { KDialogBase::done(i); }

signals:
    void replace();
    void find();
    void replaceAll();
    void done();
protected:
  virtual void virtual_hook( int id, void* data );
private:
    class KEdReplacePrivate;
    KEdReplacePrivate *d;
};


/**
 * A simple text editor for the %KDE project.
 * @deprecated Use KTextEditor::Editor or KTextEdit instead.
 *
 * @author Bernd Johannes Wuebben <wuebben@math.cornell.edu>, Waldo Bastian <bastian@kde.org>
 **/

class TDEUI_EXPORT_DEPRECATED KEdit : public TQMultiLineEdit
{
    TQ_OBJECT

public:
    /**
     * The usual constructor.
     **/
    KEdit (TQWidget *_parent=NULL, const char *name=NULL);

    ~KEdit();

    /**
     * Search directions.
     * @internal
     **/
    enum { NONE,
	   FORWARD,
	   BACKWARD };
    /**
     * Insert text from the text stream into the edit widget.
     **/
    void insertText(TQTextStream *);

    /**
     * Save text from the edit widget to a text stream.
     * If @p softWrap is false soft line wrappings are replaced with line-feeds
     * If @p softWrap is true soft line wrappings are ignored.
     * @since 3.1
     **/
    void saveText(TQTextStream *, bool softWrap);
    void saveText(TQTextStream *); // KDE 4.0: remove

    /**
     *  Let the user select a font and set the font of the textwidget to that
     * selected font.
     **/
    void 	selectFont();

    /**
     * Present a search dialog to the user
     **/
    void 	search();

    /**
     * Repeat the last search specified on the search dialog.
     *
     *  If the user hasn't searched for anything until now, this method
     *   will simply return without doing anything.
     *
     * @return @p true if a search was done. @p false if no search was done.
     **/
    bool 	repeatSearch();

    /**
     * Present a Search and Replace Dialog to the user.
     **/
    void 	replace();

    /**
     * Present a "Goto Line" dialog to the user.
     */
    void 	doGotoLine();

    /**
     * Clean up redundant whitespace from selected text.
     */
    void        cleanWhiteSpace();

    /**
     * Install a context menu for KEdit.
     *
     *  The Popup Menu will be activated on a right mouse button press event.
     */
    void 	installRBPopup( TQPopupMenu* );

    /**
     * Retrieve the current line number.
     *
     * The current line is the line the cursor is on.
     **/
    int 	currentLine();

    /**
     * Retrieve the actual column number the cursor is on.
     *
     *  This call differs
     *    from TQMultiLineEdit::getCursorPosition() in that it returns the actual cursor
     *    position and not the character position. Use currentLine() and currentColumn()
     *    if you want to display the current line or column in the status bar for
     *    example.
     */
    int 	currentColumn();


    /**
     * Start spellchecking mode.
     */
    void spellcheck_start();

    /**
     * Exit spellchecking mode.
     */
    void spellcheck_stop();

    /**
     * Allow the user to toggle between insert mode and overwrite mode with
     * the "Insert" key. See also toggle_overwrite_signal();
     *
     * The default is false: the user can not toggle.
     */
    void setOverwriteEnabled(bool b);

    TQString selectWordUnderCursor();

    /// @since 3.3
    TQPopupMenu *createPopupMenu( const TQPoint& pos );

    void setAutoUpdate(bool b);

signals:
    /** This signal is emitted if the user dropped a URL over the text editor
      * TQMultiLineEdit widget.
      *
      *  Note that the user can drop also Text on it, but
      * this is already handled internally by TQMultiLineEdit.
      */
    void        gotUrlDrop(TQDropEvent* e);

    /** This signal is emitted whenever the cursor position changes.
     *
     * Use this in conjunction with currentLine(), currentColumn()
     * if you need to know the cursor position.
     */
    void 	CursorPositionChanged();

    /**
     * This signal is emitted if the user toggles from insert to overwrite mode
     * or vice versa.
     *
     * The user can do so by pressing the "Insert" button on a PC keyboard.
     *
     * This feature must be activated by calling setOverwriteEnabled(true)
     * first.
     */
    void 	toggle_overwrite_signal();

public slots:
      /**
       * @internal
       **/
    void corrected (const TQString &originalword, const TQString &newword, unsigned int pos);
      /**
       * @internal
       **/
    void misspelling (const TQString &word, const TQStringList &, unsigned int pos);
private slots:

      /**
       * @internal
       * Called from search dialog.
       **/
    void search_slot();

      /**
       * @internal
       **/
    void searchdone_slot();

      /**
       * @internal
       **/
    void replace_slot();

      /**
       * @internal
       **/
    void replace_all_slot();

      /**
       * @internal
       **/
    void replace_search_slot();

      /**
       * @internal
       **/
    void replacedone_slot();

      /**
       * Cursor moved...
       */
    void slotCursorPositionChanged();

protected:
    void computePosition();
    int 	doSearch(TQString s_pattern, bool case_sensitive,
			 bool regex, bool forward,int line, int col);

    int 	doReplace(TQString s_pattern, bool case_sensitive,
			  bool regex, bool forward,int line, int col,bool replace);

      /**
       * Sets line and col to the position pos, considering word wrap.
       **/
    void	posToRowCol(unsigned int pos, unsigned int &line, unsigned int &col);

    /**
     * Reimplemented for internal reasons, the API is not affected.
     */
    virtual void create( WId = 0, bool initializeWindow = true,
                         bool destroyOldWindow = true );

    /**
     * Reimplemented for internal reasons, the API is not affected.
     */
    virtual void ensureCursorVisible();
    virtual void setCursor( const TQCursor & );
    virtual void viewportPaintEvent( TQPaintEvent* );

protected:

    void 	keyPressEvent 	 ( TQKeyEvent *  );

    // DnD interface
    void        dragMoveEvent(TQDragMoveEvent* e);
    void        dragEnterEvent(TQDragEnterEvent* e);
    void        dropEvent(TQDropEvent* e);
    void        contentsDragMoveEvent(TQDragMoveEvent* e);
    void        contentsDragEnterEvent(TQDragEnterEvent* e);
    void        contentsDropEvent(TQDropEvent* e);

private:
    TQTimer* repaintTimer;

    TQString	killbufferstring;
    TQWidget     *parent;
    KEdFind 	*srchdialog;
    KEdReplace 	*replace_dialog;
    KEdGotoLine *gotodialog;

    TQString     pattern;

    bool 	can_replace;
    bool	killing;
    bool 	killtrue;
    bool 	lastwasanewline;
    bool        saved_readonlystate;
    int 	last_search;
    int 	last_replace;
    int 	replace_all_line;
    int 	replace_all_col;

    int 	line_pos, col_pos;
    bool        fill_column_is_set;
    bool        word_wrap_is_set;
    int         fill_column_value;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KEditPrivate;
    KEditPrivate *d;
};

#endif
