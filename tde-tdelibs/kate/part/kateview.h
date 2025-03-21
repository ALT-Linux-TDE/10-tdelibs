/* This file is part of the KDE libraries
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

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

#ifndef kate_view_h
#define kate_view_h

#include "katedocument.h"
#include "kateviewinternal.h"
#include "kateconfig.h"

#include "../interfaces/view.h"

#include <tdetexteditor/sessionconfiginterface.h>
#include <tdetexteditor/viewstatusmsginterface.h>
#include <tdetexteditor/texthintinterface.h>

#include <tqguardedptr.h>

class KateDocument;
class KateBookmarks;
class KateSearch;
class KateCmdLine;
class KateCodeCompletion;
class KateViewConfig;
class KateViewSchemaAction;
class KateRenderer;
class KateSpell;

class TDEToggleAction;
class TDEAction;
class TDERecentFilesAction;
class TDESelectAction;

class TQGridLayout;

//
// Kate KTextEditor::View class ;)
//
class KateView : public Kate::View,
                 public KTextEditor::SessionConfigInterface,
                 public KTextEditor::ViewStatusMsgInterface,
                 public KTextEditor::TextHintInterface,
                 public KTextEditor::SelectionInterface,
                 public KTextEditor::SelectionInterfaceExt,
                 public KTextEditor::BlockSelectionInterface
{
    TQ_OBJECT

    friend class KateViewInternal;
    friend class KateIconBorder;
    friend class KateCodeCompletion;

  public:
    KateView( KateDocument* doc, TQWidget* parent = 0L, const char* name = 0 );
    ~KateView ();

  //
  // KTextEditor::View
  //
  public:
    KTextEditor::Document* document() const       { return m_doc; }

  //
  // KTextEditor::ClipboardInterface
  //
  public slots:
    // TODO: Factor out of m_viewInternal
    void paste();
    void cut();
    void copy() const;

    /**
     * internal use, copy text as HTML to clipboard
     */
    void copyHTML();

  // helper to export text as html stuff
  private:
    TQString selectionAsHtml ();
    TQString textAsHtml ( uint startLine, uint startCol, uint endLine, uint endCol, bool blockwise);
    void textAsHtmlStream ( uint startLine, uint startCol, uint endLine, uint endCol, bool blockwise, TQTextStream *ts);

    /**
     * Gets a substring in valid-xml html.
     * Example:  "<b>const</b> b = <i>34</i>"
     * It won't contain <p> or <body> or <html> or anything like that.
     *
     * @param startCol start column of substring
     * @param length length of substring
     * @param renderer The katerenderer.  This will have the schema
     *                 information that describes how to render the
     *                 attributes.
     * @param outputStream A stream to write the html to
     */
    void lineAsHTML (KateTextLine::Ptr line, uint startCol, uint length, TQTextStream *outputStream);

  public slots:
    void exportAsHTML ();

  //
  // KTextEditor::PopupMenuInterface
  //
  public:
    void installPopup( TQPopupMenu* menu ) { m_rmbMenu = menu; }
    TQPopupMenu* popup() const             { return m_rmbMenu; }

  //
  // KTextEditor::ViewCursorInterface
  //
  public slots:
    TQPoint cursorCoordinates()
        { return m_viewInternal->cursorCoordinates();                 }
    void cursorPosition( uint* l, uint* c )
        { if( l ) *l = cursorLine(); if( c ) *c = cursorColumn();     }
    void cursorPositionReal( uint* l, uint* c )
        { if( l ) *l = cursorLine(); if( c ) *c = cursorColumnReal(); }
    bool setCursorPosition( uint line, uint col )
        { return setCursorPositionInternal( line, col, tabWidth(), true );  }
    bool setCursorPositionReal( uint line, uint col)
        { return setCursorPositionInternal( line, col, 1, true );           }
    uint cursorLine()
        { return m_viewInternal->getCursor().line();                    }
    uint cursorColumn();
    uint cursorColumnReal()
        { return m_viewInternal->getCursor().col();                     }

  signals:
    void cursorPositionChanged();

  //
  // KTextEditor::CodeCompletionInterface
  //
  public slots:
    void showArgHint( TQStringList arg1, const TQString& arg2, const TQString& arg3 );
    void showCompletionBox( TQValueList<KTextEditor::CompletionEntry> arg1, int offset = 0, bool cs = true );

  signals:
    void completionAborted();
    void completionDone();
    void argHintHidden();
    void completionDone(KTextEditor::CompletionEntry);
    void filterInsertString(KTextEditor::CompletionEntry*,TQString *);
    void aboutToShowCompletionBox();

  //
  // KTextEditor::TextHintInterface
  //
  public:
    void enableTextHints(int timeout);
    void disableTextHints();

  signals:
    void needTextHint(int line, int col, TQString &text);

  //
  // KTextEditor::DynWordWrapInterface
  //
  public:
    void setDynWordWrap( bool b );
    bool dynWordWrap() const      { return m_hasWrap; }

  //
  // KTextEditor::SelectionInterface stuff
  //
  public slots:
    bool setSelection ( const KateTextCursor & start,
      const KateTextCursor & end );
    bool setSelection ( uint startLine, uint startCol,
      uint endLine, uint endCol );
    bool clearSelection ();
    bool clearSelection (bool redraw, bool finishedChangingSelection = true);

    bool hasSelection () const;
    TQString selection () const ;

    bool removeSelectedText ();

    bool selectAll();

    //
    // KTextEditor::SelectionInterfaceExt
    //
    int selStartLine() { return selectStart.line(); };
    int selStartCol()  { return selectStart.col(); };
    int selEndLine()   { return selectEnd.line(); };
    int selEndCol()    { return selectEnd.col(); };

  signals:
    void selectionChanged ();

  //
  // internal helper stuff, for katerenderer and so on
  //
  public:
    /**
     * accessors to the selection start
     * @return selection start cursor (read-only)
     */
    inline const KateSuperCursor &selStart () const { return selectStart; }

    /**
     * accessors to the selection end
     * @return selection end cursor (read-only)
     */
    inline const KateSuperCursor &selEnd () const { return selectEnd; }

    // should cursor be wrapped ? take config + blockselection state in account
    bool wrapCursor ();

    // some internal functions to get selection state of a line/col
    bool lineColSelected (int line, int col);
    bool lineSelected (int line);
    bool lineEndSelected (int line, int endCol);
    bool lineHasSelected (int line);
    bool lineIsSelection (int line);

    void tagSelection (const KateTextCursor &oldSelectStart, const KateTextCursor &oldSelectEnd);

    void selectWord(   const KateTextCursor& cursor );
    void selectLine(   const KateTextCursor& cursor );
    void selectLength( const KateTextCursor& cursor, int length );

    // this method will sync the KateViewInternal's sel{Start,End}Cached and selectAnchor
    // with the current selection, to make it "stick" instead of reverting back to sel*Cached
    void syncSelectionCache();

  //
  // KTextEditor::BlockSelectionInterface stuff
  //
  public slots:
    bool blockSelectionMode ();
    bool setBlockSelectionMode (bool on);
    bool toggleBlockSelectionMode ();


  //BEGIN EDIT STUFF
  public:
    void editStart ();
    void editEnd (int editTagLineStart, int editTagLineEnd, bool tagFrom);

    void editSetCursor (const KateTextCursor &cursor);
  //END

  //BEGIN TAG & CLEAR
  public:
    bool tagLine (const KateTextCursor& virtualCursor);

    bool tagLines (int start, int end, bool realLines = false );
    bool tagLines (KateTextCursor start, KateTextCursor end, bool realCursors = false);

    void tagAll ();

    void clear ();

    void repaintText (bool paintOnlyDirty = false);

    void updateView (bool changed = false);
  //END

  //
  // Kate::View
  //
  public:
    bool isOverwriteMode() const;
    void setOverwriteMode( bool b );

    TQString currentTextLine()
        { return getDoc()->textLine( cursorLine() ); }
    TQString currentWord()
        { return m_doc->getWord( m_viewInternal->getCursor() ); }
    void insertText( const TQString& mark )
        { getDoc()->insertText( cursorLine(), cursorColumnReal(), mark ); }
    bool canDiscard();
    int tabWidth()                { return m_doc->config()->tabWidth(); }
    void setTabWidth( int w )     { m_doc->config()->setTabWidth(w);  }
    void setEncoding( TQString e ) { m_doc->setEncoding(e);       }
    bool isLastView()             { return m_doc->isLastView(1); }

  public slots:
    void flush();
    saveResult save();
    saveResult saveAs();

    void indent()             { m_doc->indent( this, cursorLine(), 1 );  }
    void unIndent()           { m_doc->indent( this, cursorLine(), -1 ); }
    void cleanIndent()        { m_doc->indent( this, cursorLine(), 0 );  }
    void align()              { m_doc->align( this, cursorLine() ); }
    void comment()            { m_doc->comment( this, cursorLine(), cursorColumnReal(), 1 );  }
    void uncomment()          { m_doc->comment( this, cursorLine(), cursorColumnReal(),-1 ); }
    void killLine()           { m_doc->removeLine( cursorLine() ); }

    /**
      Uppercases selected text, or an alphabetic character next to the cursor.
    */
    void uppercase() { m_doc->transform( this, m_viewInternal->cursor, KateDocument::Uppercase ); }
    /**
      Lowercases selected text, or an alphabetic character next to the cursor.
    */
    void lowercase() { m_doc->transform( this, m_viewInternal->cursor, KateDocument::Lowercase ); }
    /**
      Capitalizes the selection (makes each word start with an uppercase) or
      the word under the cursor.
    */
    void capitalize() { m_doc->transform( this, m_viewInternal->cursor, KateDocument::Capitalize ); }
    /**
      Joins lines touched by the selection
    */
    void joinLines();


    void keyReturn()          { m_viewInternal->doReturn();          }
    void backspace()          { m_viewInternal->doBackspace();       }
    void deleteWordLeft()     { m_viewInternal->doDeleteWordLeft();  }
    void keyDelete()          { m_viewInternal->doDelete();          }
    void deleteWordRight()    { m_viewInternal->doDeleteWordRight(); }
    void transpose()          { m_viewInternal->doTranspose();       }
    void cursorLeft()         { m_viewInternal->cursorLeft();        }
    void shiftCursorLeft()    { m_viewInternal->cursorLeft(true);    }
    void cursorRight()        { m_viewInternal->cursorRight();       }
    void shiftCursorRight()   { m_viewInternal->cursorRight(true);   }
    void wordLeft()           { m_viewInternal->wordLeft();          }
    void shiftWordLeft()      { m_viewInternal->wordLeft(true);      }
    void wordRight()          { m_viewInternal->wordRight();         }
    void shiftWordRight()     { m_viewInternal->wordRight(true);     }
    void home()               { m_viewInternal->home();              }
    void shiftHome()          { m_viewInternal->home(true);          }
    void end()                { m_viewInternal->end();               }
    void shiftEnd()           { m_viewInternal->end(true);           }
    void up()                 { m_viewInternal->cursorUp();          }
    void shiftUp()            { m_viewInternal->cursorUp(true);      }
    void down()               { m_viewInternal->cursorDown();        }
    void shiftDown()          { m_viewInternal->cursorDown(true);    }
    void scrollUp()           { m_viewInternal->scrollUp();          }
    void scrollDown()         { m_viewInternal->scrollDown();        }
    void topOfView()          { m_viewInternal->topOfView();         }
    void shiftTopOfView()     { m_viewInternal->topOfView(true);     }
    void bottomOfView()       { m_viewInternal->bottomOfView();      }
    void shiftBottomOfView()  { m_viewInternal->bottomOfView(true);  }
    void pageUp()             { m_viewInternal->pageUp();            }
    void shiftPageUp()        { m_viewInternal->pageUp(true);        }
    void pageDown()           { m_viewInternal->pageDown();          }
    void shiftPageDown()      { m_viewInternal->pageDown(true);      }
    void top()                { m_viewInternal->top_home();          }
    void shiftTop()           { m_viewInternal->top_home(true);      }
    void bottom()             { m_viewInternal->bottom_end();        }
    void shiftBottom()        { m_viewInternal->bottom_end(true);    }
    void toMatchingBracket()  { m_viewInternal->cursorToMatchingBracket();}
    void shiftToMatchingBracket()  { m_viewInternal->cursorToMatchingBracket(true);}

    void gotoLine();
    void gotoLineNumber( int linenumber );

  // config file / session management functions
  public:
    void readSessionConfig(TDEConfig *);
    void writeSessionConfig(TDEConfig *);

  public slots:
    int getEol();
    void setEol( int eol );
    void find();
    void find( const TQString&, long, bool add=true ); ///< proxy for KateSearch
    void replace();
    void replace( const TQString&, const TQString &, long ); ///< proxy for KateSearch
    /** Highly confusing but KateSearch::findAgain() is backwards too. */
    void findAgain( bool back );
    void findAgain()              { findAgain( false );          }
    void findPrev()               { findAgain( true );           }

    void setFoldingMarkersOn( bool enable ); // Not in Kate::View, but should be
    void setIconBorder( bool enable );
    void setLineNumbersOn( bool enable );
    void setScrollBarMarks( bool enable );
    void showCmdLine ( bool enable );
    void toggleFoldingMarkers();
    void toggleIconBorder();
    void toggleLineNumbersOn();
    void toggleScrollBarMarks();
    void toggleDynWordWrap ();
    void toggleCmdLine ();
    void setDynWrapIndicators(int mode);
    
    void applyWordWrap ();

  public:
    KateRenderer *renderer ();

    bool iconBorder();
    bool lineNumbersOn();
    bool scrollBarMarks();
    int dynWrapIndicators();
    bool foldingMarkersOn();
    Kate::Document* getDoc()    { return m_doc; }

    void setActive( bool b )    { m_active = b; }
    bool isActive()             { return m_active; }

  public slots:
    void gotoMark( KTextEditor::Mark* mark ) { setCursorPositionInternal ( mark->line, 0, 1 ); }
    void slotSelectionChanged ();

  signals:
    void gotFocus( Kate::View* );
    void lostFocus( Kate::View* );
    void newStatus(); // Not in Kate::View, but should be (Kate app connects to it)

  //
  // Extras
  //
  public:
    // Is it really necessary to have 3 methods for this?! :)
    KateDocument*  doc() const       { return m_doc; }

    TDEActionCollection* editActionCollection() const { return m_editActions; }

  public slots:
    void slotNewUndo();
    void slotUpdate();
    void toggleInsert();
    void reloadFile();
    void toggleWWMarker();
    void toggleWriteLock();
    void switchToCmdLine ();
    void slotReadWriteChanged ();

  signals:
    void dropEventPass(TQDropEvent*);
    void viewStatusMsg (const TQString &msg);

  public:
    bool setCursorPositionInternal( uint line, uint col, uint tabwidth = 1, bool calledExternally = false );

  protected:
    void contextMenuEvent( TQContextMenuEvent* );
    bool checkOverwrite( KURL );

  public slots:
    void slotSelectionTypeChanged();

  private slots:
    void slotGotFocus();
    void slotLostFocus();
    void slotDropEventPass( TQDropEvent* ev );
    void slotStatusMsg();
    void slotSaveCanceled( const TQString& error );
    void slotExpandToplevel();
    void slotCollapseLocal();
    void slotExpandLocal();

  private:
    void setupConnections();
    void setupActions();
    void setupEditActions();
    void setupCodeFolding();
    void setupCodeCompletion();

    TDEActionCollection*     m_editActions;
    TDEAction*               m_editUndo;
    TDEAction*               m_editRedo;
    TDERecentFilesAction*    m_fileRecent;
    TDEToggleAction*         m_toggleFoldingMarkers;
    TDEToggleAction*         m_toggleIconBar;
    TDEToggleAction*         m_toggleLineNumbers;
    TDEToggleAction*         m_toggleScrollBarMarks;
    TDEToggleAction*         m_toggleDynWrap;
    TDESelectAction*         m_setDynWrapIndicators;
    TDEToggleAction*         m_toggleWWMarker;
    TDEAction*               m_switchCmdLine;

    TDESelectAction*         m_setEndOfLine;

    TDEAction *m_cut;
    TDEAction *m_copy;
    TDEAction *m_copyHTML;
    TDEAction *m_paste;
    TDEAction *m_selectAll;
    TDEAction *m_deSelect;

    TDEToggleAction *m_toggleBlockSelection;
    TDEToggleAction *m_toggleInsert;
    TDEToggleAction *m_toggleWriteLock;

    KateDocument*          m_doc;
    KateViewInternal*      m_viewInternal;
    KateRenderer*          m_renderer;
    KateSearch*            m_search;
    KateSpell             *m_spell;
    KateBookmarks*         m_bookmarks;
    TQGuardedPtr<TQPopupMenu>  m_rmbMenu;
    KateCodeCompletion*    m_codeCompletion;

    KateCmdLine *m_cmdLine;
    bool m_cmdLineOn;

    TQGridLayout *m_grid;

    bool       m_active;
    bool       m_hasWrap;

  private slots:
    void slotNeedTextHint(int line, int col, TQString &text);
    void slotHlChanged();

  /**
   * Configuration
   */
  public:
    inline KateViewConfig *config () { return m_config; };

    void updateConfig ();

    void updateDocumentConfig();

    void updateRendererConfig();

  private slots:
    void updateFoldingConfig ();

  private:
    KateViewConfig *m_config;
    bool m_startingUp;
    bool m_updatingDocumentConfig;

  private:
    // stores the current selection
    KateSuperCursor selectStart;
    KateSuperCursor selectEnd;

    // do we select normal or blockwise ?
    bool blockSelect;

  /**
   * IM input stuff
   */
  public:
    void setIMSelectionValue( uint imStartLine, uint imStart, uint imEnd,
                              uint imSelStart, uint imSelEnd, bool m_imComposeEvent );
    void getIMSelectionValue( uint *imStartLine, uint *imStart, uint *imEnd,
                              uint *imSelStart, uint *imSelEnd );
    bool isIMSelection( int _line, int _column );
    bool isIMEdit( int _line, int _column );
    bool imComposeEvent () const { return m_imComposeEvent; }

  private:
    uint m_imStartLine;
    uint m_imStart;
    uint m_imEnd;
    uint m_imSelStart;
    uint m_imSelEnd;
    bool m_imComposeEvent;
};

#endif
