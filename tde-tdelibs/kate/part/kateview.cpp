/* This file is part of the KDE libraries
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001-2004 Christoph Cullmann <cullmann@kde.org>
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

#define DEBUGACCELS

//BEGIN includes
#include "kateview.h"
#include "kateview.moc"

#include "kateviewinternal.h"
#include "kateviewhelpers.h"
#include "katerenderer.h"
#include "katedocument.h"
#include "katedocumenthelpers.h"
#include "katefactory.h"
#include "katehighlight.h"
#include "katedialogs.h"
#include "katetextline.h"
#include "katecodefoldinghelpers.h"
#include "katecodecompletion.h"
#include "katesearch.h"
#include "kateschema.h"
#include "katebookmarks.h"
#include "katesearch.h"
#include "kateconfig.h"
#include "katefiletype.h"
#include "kateautoindent.h"
#include "katespell.h"

#include <tdetexteditor/plugin.h>

#include <tdeparts/event.h>

#include <tdeio/netaccess.h>

#include <tdeconfig.h>
#include <kurldrag.h>
#include <kdebug.h>
#include <tdeapplication.h>
#include <kcursor.h>
#include <tdelocale.h>
#include <tdeglobal.h>
#include <kcharsets.h>
#include <tdemessagebox.h>
#include <tdeaction.h>
#include <kstdaction.h>
#include <kxmlguifactory.h>
#include <tdeaccel.h>
#include <klibloader.h>
#include <kencodingfiledialog.h>
#include <tdemultipledrag.h>
#include <tdetempfile.h>
#include <ksavefile.h>

#include <tqfont.h>
#include <tqfileinfo.h>
#include <tqstyle.h>
#include <tqevent.h>
#include <tqpopupmenu.h>
#include <tqlayout.h>
#include <tqclipboard.h>
#include <tqstylesheet.h>
//END includes

KateView::KateView( KateDocument *doc, TQWidget *parent, const char * name )
    : Kate::View( doc, parent, name )
    , m_doc( doc )
    , m_search( new KateSearch( this ) )
    , m_spell( new KateSpell( this ) )
    , m_bookmarks( new KateBookmarks( this ) )
    , m_cmdLine (0)
    , m_cmdLineOn (false)
    , m_active( false )
    , m_hasWrap( false )
    , m_startingUp (true)
    , m_updatingDocumentConfig (false)
    , selectStart (m_doc, true)
    , selectEnd (m_doc, true)
    , blockSelect (false)
    , m_imStartLine( 0 )
    , m_imStart( 0 )
    , m_imEnd( 0 )
    , m_imSelStart( 0 )
    , m_imSelEnd( 0 )
    , m_imComposeEvent( false )
{
  KateFactory::self()->registerView( this );
  m_config = new KateViewConfig (this);

  m_renderer = new KateRenderer(doc, this);

  m_grid = new TQGridLayout (this, 3, 3);

  m_grid->setRowStretch ( 0, 10 );
  m_grid->setRowStretch ( 1, 0 );
  m_grid->setColStretch ( 0, 0 );
  m_grid->setColStretch ( 1, 10 );
  m_grid->setColStretch ( 2, 0 );

  m_viewInternal = new KateViewInternal( this, doc );
  m_grid->addWidget (m_viewInternal, 0, 1);

  setClipboardInterfaceDCOPSuffix (viewDCOPSuffix());
  setCodeCompletionInterfaceDCOPSuffix (viewDCOPSuffix());
  setDynWordWrapInterfaceDCOPSuffix (viewDCOPSuffix());
  setPopupMenuInterfaceDCOPSuffix (viewDCOPSuffix());
  setSessionConfigInterfaceDCOPSuffix (viewDCOPSuffix());
  setViewCursorInterfaceDCOPSuffix (viewDCOPSuffix());
  setViewStatusMsgInterfaceDCOPSuffix (viewDCOPSuffix());

  setInstance( KateFactory::self()->instance() );
  doc->addView( this );

  setFocusProxy( m_viewInternal );
  setFocusPolicy( TQWidget::StrongFocus );

  if (!doc->singleViewMode()) {
    setXMLFile( "katepartui.rc" );
  } else {
    if( doc->readOnly() )
      setXMLFile( "katepartreadonlyui.rc" );
    else
      setXMLFile( "katepartui.rc" );
  }

  setupConnections();
  setupActions();
  setupEditActions();
  setupCodeFolding();
  setupCodeCompletion();

  // enable the plugins of this view
  m_doc->enableAllPluginsGUI (this);

  // update the enabled state of the undo/redo actions...
  slotNewUndo();

  m_startingUp = false;
  updateConfig ();

  slotHlChanged();
  /*test texthint
  connect(this,TQ_SIGNAL(needTextHint(int, int, TQString &)),
  this,TQ_SLOT(slotNeedTextHint(int, int, TQString &)));
  enableTextHints(1000);
  test texthint*/
}

KateView::~KateView()
{
  if (!m_doc->singleViewMode())
    m_doc->disableAllPluginsGUI (this);

  m_doc->removeView( this );

  // its a TQObject. don't double-delete
  //delete m_viewInternal;
  //delete m_codeCompletion;

  delete m_renderer;
  m_renderer = 0;

  delete m_config;
  m_config = 0;
  KateFactory::self()->deregisterView (this);
}

void KateView::setupConnections()
{
  connect( m_doc, TQ_SIGNAL(undoChanged()),
           this, TQ_SLOT(slotNewUndo()) );
  connect( m_doc, TQ_SIGNAL(hlChanged()),
           this, TQ_SLOT(slotHlChanged()) );
  connect( m_doc, TQ_SIGNAL(canceled(const TQString&)),
           this, TQ_SLOT(slotSaveCanceled(const TQString&)) );
  connect( m_viewInternal, TQ_SIGNAL(dropEventPass(TQDropEvent*)),
           this,           TQ_SIGNAL(dropEventPass(TQDropEvent*)) );
  connect(this,TQ_SIGNAL(cursorPositionChanged()),this,TQ_SLOT(slotStatusMsg()));
  connect(this,TQ_SIGNAL(newStatus()),this,TQ_SLOT(slotStatusMsg()));
  connect(m_doc, TQ_SIGNAL(undoChanged()), this, TQ_SLOT(slotStatusMsg()));

  if ( m_doc->browserView() )
  {
    connect( this, TQ_SIGNAL(dropEventPass(TQDropEvent*)),
             this, TQ_SLOT(slotDropEventPass(TQDropEvent*)) );
  }
}

void KateView::setupActions()
{
  TDEActionCollection *ac = this->actionCollection ();
  TDEAction *a;

  m_toggleWriteLock = 0;

  m_cut = a=KStdAction::cut(this, TQ_SLOT(cut()), ac);
  a->setWhatsThis(i18n("Cut the selected text and move it to the clipboard"));

  m_paste = a=KStdAction::pasteText(this, TQ_SLOT(paste()), ac);
  a->setWhatsThis(i18n("Paste previously copied or cut clipboard contents"));

  m_copy = a=KStdAction::copy(this, TQ_SLOT(copy()), ac);
  a->setWhatsThis(i18n( "Use this command to copy the currently selected text to the system clipboard."));

  m_copyHTML = a = new TDEAction(i18n("Copy as &HTML"), "edit-copy", 0, this, TQ_SLOT(copyHTML()), ac, "edit_copy_html");
  a->setWhatsThis(i18n( "Use this command to copy the currently selected text as HTML to the system clipboard."));

  if (!m_doc->readOnly())
  {
    a=KStdAction::save(this, TQ_SLOT(save()), ac);
    a->setWhatsThis(i18n("Save the current document"));

    a=m_editUndo = KStdAction::undo(m_doc, TQ_SLOT(undo()), ac);
    a->setWhatsThis(i18n("Revert the most recent editing actions"));

    a=m_editRedo = KStdAction::redo(m_doc, TQ_SLOT(redo()), ac);
    a->setWhatsThis(i18n("Revert the most recent undo operation"));

    (new TDEAction(i18n("&Word Wrap Document"), "", 0, this, TQ_SLOT(applyWordWrap()), ac, "tools_apply_wordwrap"))->setWhatsThis(
  i18n("Use this command to wrap all lines of the current document which are longer than the width of the"
    " current view, to fit into this view.<br><br> This is a static word wrap, meaning it is not updated"
    " when the view is resized."));

    // setup Tools menu
    a=new TDEAction(i18n("&Indent"), "format-indent-more", TQt::CTRL+TQt::Key_I, this, TQ_SLOT(indent()), ac, "tools_indent");
    a->setWhatsThis(i18n("Use this to indent a selected block of text.<br><br>"
        "You can configure whether tabs should be honored and used or replaced with spaces, in the configuration dialog."));
    a=new TDEAction(i18n("&Unindent"), "format-indent-less", TQt::CTRL+TQt::SHIFT+TQt::Key_I, this, TQ_SLOT(unIndent()), ac, "tools_unindent");
    a->setWhatsThis(i18n("Use this to unindent a selected block of text."));

    a=new TDEAction(i18n("&Clean Indentation"), 0, this, TQ_SLOT(cleanIndent()), ac, "tools_cleanIndent");
    a->setWhatsThis(i18n("Use this to clean the indentation of a selected block of text (only tabs/only spaces)<br><br>"
        "You can configure whether tabs should be honored and used or replaced with spaces, in the configuration dialog."));

    a=new TDEAction(i18n("&Align"), 0, this, TQ_SLOT(align()), ac, "tools_align");
    a->setWhatsThis(i18n("Use this to align the current line or block of text to its proper indent level."));

    a=new TDEAction(i18n("C&omment"), CTRL+TQt::Key_D, this, TQ_SLOT(comment()),
        ac, "tools_comment");
    a->setWhatsThis(i18n("This command comments out the current line or a selected block of text.<BR><BR>"
        "The characters for single/multiple line comments are defined within the language's highlighting."));

    a=new TDEAction(i18n("Unco&mment"), CTRL+SHIFT+TQt::Key_D, this, TQ_SLOT(uncomment()),
                                 ac, "tools_uncomment");
    a->setWhatsThis(i18n("This command removes comments from the current line or a selected block of text.<BR><BR>"
    "The characters for single/multiple line comments are defined within the language's highlighting."));
    a = m_toggleWriteLock = new TDEToggleAction(
                i18n("&Read Only Mode"), 0, 0,
                this, TQ_SLOT( toggleWriteLock() ),
                ac, "tools_toggle_write_lock" );
    a->setWhatsThis( i18n("Lock/unlock the document for writing") );

    a = new TDEAction( i18n("Uppercase"), CTRL + TQt::Key_U, this,
      TQ_SLOT(uppercase()), ac, "tools_uppercase" );
    a->setWhatsThis( i18n("Convert the selection to uppercase, or the character to the "
      "right of the cursor if no text is selected.") );

    a = new TDEAction( i18n("Lowercase"), CTRL + SHIFT + TQt::Key_U, this,
      TQ_SLOT(lowercase()), ac, "tools_lowercase" );
    a->setWhatsThis( i18n("Convert the selection to lowercase, or the character to the "
      "right of the cursor if no text is selected.") );

    a = new TDEAction( i18n("Capitalize"), CTRL + ALT + TQt::Key_U, this,
      TQ_SLOT(capitalize()), ac, "tools_capitalize" );
    a->setWhatsThis( i18n("Capitalize the selection, or the word under the "
      "cursor if no text is selected.") );

    a = new TDEAction( i18n("Delete Line"), 0, this, 
      TQ_SLOT( killLine() ), ac, "tools_delete_line");
    a->setWhatsThis(i18n("Use this to delete the current line."));
    
    a = new TDEAction( i18n("Join Lines"), CTRL + TQt::Key_J, this,
      TQ_SLOT( joinLines() ), ac, "tools_join_lines" );
    a->setWhatsThis(i18n("Use this to join lines together."));
  }
  else
  {
    m_cut->setEnabled (false);
    m_paste->setEnabled (false);
    m_editUndo = 0;
    m_editRedo = 0;
  }

  a=KStdAction::print( m_doc, TQ_SLOT(print()), ac );
  a->setWhatsThis(i18n("Print the current document."));

  a=new TDEAction(i18n("Reloa&d"), "reload", TDEStdAccel::reload(), this, TQ_SLOT(reloadFile()), ac, "file_reload");
  a->setWhatsThis(i18n("Reload the current document from disk."));

  a=KStdAction::saveAs(this, TQ_SLOT(saveAs()), ac);
  a->setWhatsThis(i18n("Save the current document to disk, with a name of your choice."));

  a=KStdAction::gotoLine(this, TQ_SLOT(gotoLine()), ac);
  a->setWhatsThis(i18n("This command opens a dialog and lets you choose a line that you want the cursor to move to."));

  a=new TDEAction(i18n("&Configure Editor..."), 0, m_doc, TQ_SLOT(configDialog()),ac, "set_confdlg");
  a->setWhatsThis(i18n("Configure various aspects of this editor."));

  KateViewHighlightAction *menu = new KateViewHighlightAction (i18n("&Highlighting"), ac, "set_highlight");
  menu->setWhatsThis(i18n("Here you can choose how the current document should be highlighted."));
  menu->updateMenu (m_doc);

  KateViewFileTypeAction *ftm = new KateViewFileTypeAction (i18n("&Filetype"),ac,"set_filetype");
  ftm->updateMenu (m_doc);

  KateViewSchemaAction *schemaMenu = new KateViewSchemaAction (i18n("&Schema"),ac,"view_schemas");
  schemaMenu->updateMenu (this);

  // indentation menu
  new KateViewIndentationAction (m_doc, i18n("&Indentation"),ac,"tools_indentation");

  // html export
  a = new TDEAction(i18n("E&xport as HTML..."), 0, 0, this, TQ_SLOT(exportAsHTML()), ac, "file_export_html");
  a->setWhatsThis(i18n("This command allows you to export the current document"
                      " with all highlighting information into a HTML document."));

  m_selectAll = a=KStdAction::selectAll(this, TQ_SLOT(selectAll()), ac);
  a->setWhatsThis(i18n("Select the entire text of the current document."));

  m_deSelect = a=KStdAction::deselect(this, TQ_SLOT(clearSelection()), ac);
  a->setWhatsThis(i18n("If you have selected something within the current document, this will no longer be selected."));

  a=new TDEAction(i18n("Enlarge Font"), "zoom-in", 0, m_viewInternal, TQ_SLOT(slotIncFontSizes()), ac, "incFontSizes");
  a->setWhatsThis(i18n("This increases the display font size."));

  a=new TDEAction(i18n("Shrink Font"), "zoom-out", 0, m_viewInternal, TQ_SLOT(slotDecFontSizes()), ac, "decFontSizes");
  a->setWhatsThis(i18n("This decreases the display font size."));

  a= m_toggleBlockSelection = new TDEToggleAction(
    i18n("Bl&ock Selection Mode"), CTRL + SHIFT + Key_B,
    this, TQ_SLOT(toggleBlockSelectionMode()),
    ac, "set_verticalSelect");
  a->setWhatsThis(i18n("This command allows switching between the normal (line based) selection mode and the block selection mode."));

  a= m_toggleInsert = new TDEToggleAction(
    i18n("Overwr&ite Mode"), Key_Insert,
    this, TQ_SLOT(toggleInsert()),
    ac, "set_insert" );
  a->setWhatsThis(i18n("Choose whether you want the text you type to be inserted or to overwrite existing text."));

  TDEToggleAction *toggleAction;
   a= m_toggleDynWrap = toggleAction = new TDEToggleAction(
    i18n("&Dynamic Word Wrap"), Key_F10,
    this, TQ_SLOT(toggleDynWordWrap()),
    ac, "view_dynamic_word_wrap" );
  a->setWhatsThis(i18n("If this option is checked, the text lines will be wrapped at the view border on the screen."));

  a= m_setDynWrapIndicators = new TDESelectAction(i18n("Dynamic Word Wrap Indicators"), 0, ac, "dynamic_word_wrap_indicators");
  a->setWhatsThis(i18n("Choose when the Dynamic Word Wrap Indicators should be displayed"));

  connect(m_setDynWrapIndicators, TQ_SIGNAL(activated(int)), this, TQ_SLOT(setDynWrapIndicators(int)));
  TQStringList list2;
  list2.append(i18n("&Off"));
  list2.append(i18n("Follow &Line Numbers"));
  list2.append(i18n("&Always On"));
  m_setDynWrapIndicators->setItems(list2);

  a= toggleAction=m_toggleFoldingMarkers = new TDEToggleAction(
    i18n("Show Folding &Markers"), Key_F9,
    this, TQ_SLOT(toggleFoldingMarkers()),
    ac, "view_folding_markers" );
  a->setWhatsThis(i18n("You can choose if the codefolding marks should be shown, if codefolding is possible."));
  toggleAction->setCheckedState(i18n("Hide Folding &Markers"));

   a= m_toggleIconBar = toggleAction = new TDEToggleAction(
    i18n("Show &Icon Border"), Key_F6,
    this, TQ_SLOT(toggleIconBorder()),
    ac, "view_border");
  a=toggleAction;
  a->setWhatsThis(i18n("Show/hide the icon border.<BR><BR> The icon border shows bookmark symbols, for instance."));
  toggleAction->setCheckedState(i18n("Hide &Icon Border"));

  a= toggleAction=m_toggleLineNumbers = new TDEToggleAction(
     i18n("Show &Line Numbers"), Key_F11,
     this, TQ_SLOT(toggleLineNumbersOn()),
     ac, "view_line_numbers" );
  a->setWhatsThis(i18n("Show/hide the line numbers on the left hand side of the view."));
  toggleAction->setCheckedState(i18n("Hide &Line Numbers"));

  a= m_toggleScrollBarMarks = toggleAction = new TDEToggleAction(
     i18n("Show Scroll&bar Marks"), 0,
     this, TQ_SLOT(toggleScrollBarMarks()),
     ac, "view_scrollbar_marks");
  a->setWhatsThis(i18n("Show/hide the marks on the vertical scrollbar.<BR><BR>The marks, for instance, show bookmarks."));
  toggleAction->setCheckedState(i18n("Hide Scroll&bar Marks"));

  a = toggleAction = m_toggleWWMarker = new TDEToggleAction(
        i18n("Show Static &Word Wrap Marker"), 0,
        this, TQ_SLOT( toggleWWMarker() ),
        ac, "view_word_wrap_marker" );
  a->setWhatsThis( i18n(
        "Show/hide the Word Wrap Marker, a vertical line drawn at the word "
        "wrap column as defined in the editing properties" ));
  toggleAction->setCheckedState(i18n("Hide Static &Word Wrap Marker"));

  a= m_switchCmdLine = new TDEAction(
     i18n("Switch to Command Line"), Key_F7,
     this, TQ_SLOT(switchToCmdLine()),
     ac, "switch_to_cmd_line" );
  a->setWhatsThis(i18n("Show/hide the command line on the bottom of the view."));

  a=m_setEndOfLine = new TDESelectAction(i18n("&End of Line"), 0, ac, "set_eol");
  a->setWhatsThis(i18n("Choose which line endings should be used, when you save the document"));
  TQStringList list;
  list.append("&UNIX");
  list.append("&Windows/DOS");
  list.append("&Macintosh");
  m_setEndOfLine->setItems(list);
  m_setEndOfLine->setCurrentItem (m_doc->config()->eol());
  connect(m_setEndOfLine, TQ_SIGNAL(activated(int)), this, TQ_SLOT(setEol(int)));

  // encoding menu
  new KateViewEncodingAction (m_doc, this, i18n("E&ncoding"), ac, "set_encoding");

  m_search->createActions( ac );
  m_spell->createActions( ac );
  m_bookmarks->createActions( ac );

  slotSelectionChanged ();

  connect (this, TQ_SIGNAL(selectionChanged()), this, TQ_SLOT(slotSelectionChanged()));
}

void KateView::setupEditActions()
{
  m_editActions = new TDEActionCollection( m_viewInternal, this, "edit_actions" );
  TDEActionCollection* ac = m_editActions;

  new TDEAction(
    i18n("Move Word Left"),                         CTRL + Key_Left,
    this,TQ_SLOT(wordLeft()),
    ac, "word_left" );
  new TDEAction(
    i18n("Select Character Left"),          SHIFT +        Key_Left,
    this,TQ_SLOT(shiftCursorLeft()),
    ac, "select_char_left" );
  new TDEAction(
    i18n("Select Word Left"),               SHIFT + CTRL + Key_Left,
    this, TQ_SLOT(shiftWordLeft()),
    ac, "select_word_left" );

  new TDEAction(
    i18n("Move Word Right"),                        CTRL + Key_Right,
    this, TQ_SLOT(wordRight()),
    ac, "word_right" );
  new TDEAction(
    i18n("Select Character Right"),         SHIFT        + Key_Right,
    this, TQ_SLOT(shiftCursorRight()),
    ac, "select_char_right" );
  new TDEAction(
    i18n("Select Word Right"),              SHIFT + CTRL + Key_Right,
    this,TQ_SLOT(shiftWordRight()),
    ac, "select_word_right" );

  new TDEAction(
    i18n("Move to Beginning of Line"),                      Key_Home,
    this, TQ_SLOT(home()),
    ac, "beginning_of_line" );
  new TDEAction(
    i18n("Move to Beginning of Document"),           TDEStdAccel::home(),
    this, TQ_SLOT(top()),
    ac, "beginning_of_document" );
  new TDEAction(
    i18n("Select to Beginning of Line"),     SHIFT +        Key_Home,
    this, TQ_SLOT(shiftHome()),
    ac, "select_beginning_of_line" );
  new TDEAction(
    i18n("Select to Beginning of Document"), SHIFT + CTRL + Key_Home,
    this, TQ_SLOT(shiftTop()),
    ac, "select_beginning_of_document" );

  new TDEAction(
    i18n("Move to End of Line"),                            Key_End,
    this, TQ_SLOT(end()),
    ac, "end_of_line" );
  new TDEAction(
    i18n("Move to End of Document"),                 TDEStdAccel::end(),
    this, TQ_SLOT(bottom()),
    ac, "end_of_document" );
  new TDEAction(
    i18n("Select to End of Line"),           SHIFT +        Key_End,
    this, TQ_SLOT(shiftEnd()),
    ac, "select_end_of_line" );
  new TDEAction(
    i18n("Select to End of Document"),       SHIFT + CTRL + Key_End,
    this, TQ_SLOT(shiftBottom()),
    ac, "select_end_of_document" );

  new TDEAction(
    i18n("Select to Previous Line"),                SHIFT + Key_Up,
    this, TQ_SLOT(shiftUp()),
    ac, "select_line_up" );
  new TDEAction(
    i18n("Scroll Line Up"),"",              CTRL +          Key_Up,
    this, TQ_SLOT(scrollUp()),
    ac, "scroll_line_up" );

  new TDEAction(i18n("Move to Next Line"), Key_Down, this, TQ_SLOT(down()),
	      ac, "move_line_down");

  new TDEAction(i18n("Move to Previous Line"), Key_Up, this, TQ_SLOT(up()),
	      ac, "move_line_up");

  new TDEAction(i18n("Move Character Right"), Key_Right, this,
	      TQ_SLOT(cursorRight()), ac, "move_cursor_right");

  new TDEAction(i18n("Move Character Left"), Key_Left, this, TQ_SLOT(cursorLeft()),
	      ac, "move_cusor_left");

  new TDEAction(
    i18n("Select to Next Line"),                    SHIFT + Key_Down,
    this, TQ_SLOT(shiftDown()),
    ac, "select_line_down" );
  new TDEAction(
    i18n("Scroll Line Down"),               CTRL +          Key_Down,
    this, TQ_SLOT(scrollDown()),
    ac, "scroll_line_down" );

  new TDEAction(
    i18n("Scroll Page Up"),                         TDEStdAccel::prior(),
    this, TQ_SLOT(pageUp()),
    ac, "scroll_page_up" );
  new TDEAction(
    i18n("Select Page Up"),                         SHIFT + Key_PageUp,
    this, TQ_SLOT(shiftPageUp()),
    ac, "select_page_up" );
  new TDEAction(
    i18n("Move to Top of View"),             CTRL +         Key_PageUp,
    this, TQ_SLOT(topOfView()),
    ac, "move_top_of_view" );
  new TDEAction(
    i18n("Select to Top of View"),             CTRL + SHIFT +  Key_PageUp,
    this, TQ_SLOT(shiftTopOfView()),
    ac, "select_top_of_view" );

  new TDEAction(
    i18n("Scroll Page Down"),                          TDEStdAccel::next(),
    this, TQ_SLOT(pageDown()),
    ac, "scroll_page_down" );
  new TDEAction(
    i18n("Select Page Down"),                       SHIFT + Key_PageDown,
    this, TQ_SLOT(shiftPageDown()),
    ac, "select_page_down" );
  new TDEAction(
    i18n("Move to Bottom of View"),          CTRL +         Key_PageDown,
    this, TQ_SLOT(bottomOfView()),
    ac, "move_bottom_of_view" );
  new TDEAction(
    i18n("Select to Bottom of View"),         CTRL + SHIFT + Key_PageDown,
    this, TQ_SLOT(shiftBottomOfView()),
    ac, "select_bottom_of_view" );
  new TDEAction(
    i18n("Move to Matching Bracket"),               CTRL + Key_6,
    this, TQ_SLOT(toMatchingBracket()),
    ac, "to_matching_bracket" );
  new TDEAction(
    i18n("Select to Matching Bracket"),      SHIFT + CTRL + Key_6,
    this, TQ_SLOT(shiftToMatchingBracket()),
    ac, "select_matching_bracket" );

  // anders: shortcuts doing any changes should not be created in browserextension
  if ( !m_doc->readOnly() )
  {
    new TDEAction(
      i18n("Transpose Characters"),           CTRL + Key_T,
      this, TQ_SLOT(transpose()),
      ac, "transpose_char" );

    new TDEAction(
      i18n("Delete Line"),                    CTRL + Key_K,
      this, TQ_SLOT(killLine()),
      ac, "delete_line" );

    new TDEAction(
      i18n("Delete Word Left"),               TDEStdAccel::deleteWordBack(),
      this, TQ_SLOT(deleteWordLeft()),
      ac, "delete_word_left" );

    new TDEAction(
      i18n("Delete Word Right"),              TDEStdAccel::deleteWordForward(),
      this, TQ_SLOT(deleteWordRight()),
      ac, "delete_word_right" );

    new TDEAction(i18n("Delete Next Character"), Key_Delete,
                this, TQ_SLOT(keyDelete()),
                ac, "delete_next_character");

    TDEAction *a = new TDEAction(i18n("Backspace"), Key_Backspace,
                this, TQ_SLOT(backspace()),
                ac, "backspace");
    TDEShortcut cut = a->shortcut();
    cut.append( KKey( SHIFT + Key_Backspace ) );
    a->setShortcut( cut );
  }

  connect( this, TQ_SIGNAL(gotFocus(Kate::View*)),
           this, TQ_SLOT(slotGotFocus()) );
  connect( this, TQ_SIGNAL(lostFocus(Kate::View*)),
           this, TQ_SLOT(slotLostFocus()) );

  m_editActions->readShortcutSettings( "Katepart Shortcuts" );

  if( hasFocus() )
    slotGotFocus();
  else
    slotLostFocus();


}

void KateView::setupCodeFolding()
{
  TDEActionCollection *ac=this->actionCollection();
  new TDEAction( i18n("Collapse Toplevel"), CTRL+SHIFT+Key_Minus,
       m_doc->foldingTree(),TQ_SLOT(collapseToplevelNodes()),ac,"folding_toplevel");
  new TDEAction( i18n("Expand Toplevel"), CTRL+SHIFT+Key_Plus,
       this,TQ_SLOT(slotExpandToplevel()),ac,"folding_expandtoplevel");
  new TDEAction( i18n("Collapse One Local Level"), CTRL+Key_Minus,
       this,TQ_SLOT(slotCollapseLocal()),ac,"folding_collapselocal");
  new TDEAction( i18n("Expand One Local Level"), CTRL+Key_Plus,
       this,TQ_SLOT(slotExpandLocal()),ac,"folding_expandlocal");

#ifdef DEBUGACCELS
  TDEAccel* debugAccels = new TDEAccel(this,this);
  debugAccels->insert("KATE_DUMP_REGION_TREE",i18n("Show the code folding region tree"),"","Ctrl+Shift+Alt+D",m_doc,TQ_SLOT(dumpRegionTree()));
  debugAccels->insert("KATE_TEMPLATE_TEST",i18n("Basic template code test"),"","Ctrl+Shift+Alt+T",m_doc,TQ_SLOT(testTemplateCode()));
  debugAccels->setEnabled(true);
#endif
}

void KateView::slotExpandToplevel()
{
  m_doc->foldingTree()->expandToplevelNodes(m_doc->numLines());
}

void KateView::slotCollapseLocal()
{
  int realLine = m_doc->foldingTree()->collapseOne(cursorLine());
  if (realLine != -1)
    // TODO rodda: fix this to only set line and allow internal view to chose column
    // Explicitly call internal because we want this to be registered as an internal call
    setCursorPositionInternal(realLine, cursorColumn(), tabWidth(), false);
}

void KateView::slotExpandLocal()
{
  m_doc->foldingTree()->expandOne(cursorLine(), m_doc->numLines());
}

void KateView::setupCodeCompletion()
{
  m_codeCompletion = new KateCodeCompletion(this);
  connect( m_codeCompletion, TQ_SIGNAL(completionAborted()),
           this,             TQ_SIGNAL(completionAborted()));
  connect( m_codeCompletion, TQ_SIGNAL(completionDone()),
           this,             TQ_SIGNAL(completionDone()));
  connect( m_codeCompletion, TQ_SIGNAL(argHintHidden()),
           this,             TQ_SIGNAL(argHintHidden()));
  connect( m_codeCompletion, TQ_SIGNAL(completionDone(KTextEditor::CompletionEntry)),
           this,             TQ_SIGNAL(completionDone(KTextEditor::CompletionEntry)));
  connect( m_codeCompletion, TQ_SIGNAL(filterInsertString(KTextEditor::CompletionEntry*,TQString*)),
           this,             TQ_SIGNAL(filterInsertString(KTextEditor::CompletionEntry*,TQString*)));
}

void KateView::slotGotFocus()
{
  m_editActions->accel()->setEnabled( true );

  slotStatusMsg ();
}

void KateView::slotLostFocus()
{
  m_editActions->accel()->setEnabled( false );
}

void KateView::setDynWrapIndicators(int mode)
{
  config()->setDynWordWrapIndicators (mode);
}

void KateView::slotStatusMsg ()
{
  TQString ovrstr;
  if (m_doc->isReadWrite())
  {
    if (m_doc->config()->configFlags() & KateDocument::cfOvr)
      ovrstr = i18n(" OVR ");
    else
      ovrstr = i18n(" INS ");
  }
  else
    ovrstr = i18n(" R/O ");

  uint r = cursorLine() + 1;
  uint c = cursorColumn() + 1;

  TQString s1 = i18n(" Line: %1").arg(TDEGlobal::locale()->formatNumber(r, 0));
  TQString s2 = i18n(" Col: %1").arg(TDEGlobal::locale()->formatNumber(c, 0));

  TQString modstr = m_doc->isModified() ? TQString (" * ") : TQString ("   ");
  TQString blockstr = blockSelectionMode() ? i18n(" BLK ") : i18n(" NORM ");

  emit viewStatusMsg (s1 + s2 + " " + ovrstr + blockstr + modstr);
}

void KateView::slotSelectionTypeChanged()
{
  m_toggleBlockSelection->setChecked( blockSelectionMode() );

  emit newStatus();
}

bool KateView::isOverwriteMode() const
{
  return m_doc->config()->configFlags() & KateDocument::cfOvr;
}

void KateView::reloadFile()
{
  m_doc->reloadFile();
  emit newStatus();
}

void KateView::slotUpdate()
{
  emit newStatus();

  slotNewUndo();
}

void KateView::slotReadWriteChanged ()
{
  if ( m_toggleWriteLock )
    m_toggleWriteLock->setChecked( ! m_doc->isReadWrite() );

  m_cut->setEnabled (m_doc->isReadWrite());
  m_paste->setEnabled (m_doc->isReadWrite());

  TQStringList l;

  l << "edit_replace" << "set_insert" << "tools_spelling" << "tools_indent"
      << "tools_unindent" << "tools_cleanIndent" << "tools_align"  << "tools_comment"
      << "tools_uncomment" << "tools_uppercase" << "tools_lowercase"
      << "tools_capitalize" << "tools_delete_line" << "tools_join_lines"
      << "tools_apply_wordwrap" << "edit_undo" << "edit_redo" << "tools_spelling_from_cursor"
      << "tools_spelling_selection";

  TDEAction *a = 0;
  for (uint z = 0; z < l.size(); z++)
    if ((a = actionCollection()->action( l[z].ascii() )))
      a->setEnabled (m_doc->isReadWrite());
}

void KateView::slotNewUndo()
{
  if (m_doc->readOnly())
    return;

  if ((m_doc->undoCount() > 0) != m_editUndo->isEnabled())
    m_editUndo->setEnabled(m_doc->undoCount() > 0);

  if ((m_doc->redoCount() > 0) != m_editRedo->isEnabled())
    m_editRedo->setEnabled(m_doc->redoCount() > 0);
}

void KateView::slotDropEventPass( TQDropEvent * ev )
{
  KURL::List lstDragURLs;
  bool ok = KURLDrag::decode( ev, lstDragURLs );

  KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject( doc() );
  if ( ok && ext )
    emit ext->openURLRequest( lstDragURLs.first() );
}

void KateView::contextMenuEvent( TQContextMenuEvent *ev )
{
  if ( !m_doc || !m_doc->browserExtension()  )
    return;
  emit m_doc->browserExtension()->popupMenu( /*this, */ev->globalPos(), m_doc->url(),
                                        TQString::fromLatin1( "text/plain" ) );
  ev->accept();
}

bool KateView::setCursorPositionInternal( uint line, uint col, uint tabwidth, bool calledExternally )
{
  KateTextLine::Ptr l = m_doc->kateTextLine( line );

  if (!l)
    return false;

  TQString line_str = m_doc->textLine( line );

  uint z;
  uint x = 0;
  for (z = 0; z < line_str.length() && z < col; z++) {
    if (line_str[z] == TQChar('\t')) x += tabwidth - (x % tabwidth); else x++;
  }

  m_viewInternal->updateCursor( KateTextCursor( line, x ), false, true, calledExternally );

  return true;
}

void KateView::setOverwriteMode( bool b )
{
  if ( isOverwriteMode() && !b )
    m_doc->setConfigFlags( m_doc->config()->configFlags() ^ KateDocument::cfOvr );
  else
    m_doc->setConfigFlags( m_doc->config()->configFlags() | KateDocument::cfOvr );

  m_toggleInsert->setChecked (isOverwriteMode ());
}

void KateView::toggleInsert()
{
  m_doc->setConfigFlags(m_doc->config()->configFlags() ^ KateDocument::cfOvr);
  m_toggleInsert->setChecked (isOverwriteMode ());

  emit newStatus();
}

bool KateView::canDiscard()
{
  return m_doc->closeURL();
}

void KateView::flush()
{
  m_doc->closeURL();
}

KateView::saveResult KateView::save()
{
  if( !m_doc->url().isValid() || !doc()->isReadWrite() )
    return saveAs();

  if( m_doc->save() )
    return SAVE_OK;

  return SAVE_ERROR;
}

KateView::saveResult KateView::saveAs()
{

  KEncodingFileDialog::Result res=KEncodingFileDialog::getSaveURLAndEncoding(doc()->config()->encoding(),
                m_doc->url().url(),TQString::null,this,i18n("Save File"));

//   kdDebug()<<"urllist is emtpy?"<<res.URLs.isEmpty()<<endl;
//   kdDebug()<<"url is:"<<res.URLs.first()<<endl;
  if( res.URLs.isEmpty() || !checkOverwrite( res.URLs.first() ) )
    return SAVE_CANCEL;

  m_doc->config()->setEncoding( res.encoding );

  if( m_doc->saveAs( res.URLs.first() ) )
    return SAVE_OK;

  return SAVE_ERROR;
}

bool KateView::checkOverwrite( KURL u )
{
  if( !u.isLocalFile() )
    return true;

  TQFileInfo info( u.path() );
  if( !info.exists() )
    return true;

  return KMessageBox::Continue
         == KMessageBox::warningContinueCancel
              ( this,
                i18n( "A file named \"%1\" already exists. Are you sure you want to overwrite it?" ).arg( info.fileName() ),
                i18n( "Overwrite File?" ),
                KGuiItem( i18n( "&Overwrite" ), "document-save", i18n( "Overwrite the file" ) )
              );
}

void KateView::slotSaveCanceled( const TQString& error )
{
  if ( !error.isEmpty() ) // happens when cancelling a job
    KMessageBox::error( this, error );
}

void KateView::gotoLine()
{
  KateGotoLineDialog *dlg = new KateGotoLineDialog (this, m_viewInternal->getCursor().line() + 1, m_doc->numLines());

  if (dlg->exec() == TQDialog::Accepted)
    gotoLineNumber( dlg->getLine() - 1 );

  delete dlg;
}

void KateView::gotoLineNumber( int line )
{
  // clear selection, unless we are in persistent selection mode
  if ( !config()->persistentSelection() )
    clearSelection();
  setCursorPositionInternal ( line, 0, 1 );
}

void KateView::joinLines()
{
  int first = selStartLine();
  int last = selEndLine();
  //int left = m_doc->textLine( last ).length() - m_doc->selEndCol();
  if ( first == last )
  {
    first = cursorLine();
    last = first + 1;
  }
  m_doc->joinLines( first, last );
}

void KateView::readSessionConfig(TDEConfig *config)
{
  setCursorPositionInternal (config->readNumEntry("CursorLine"), config->readNumEntry("CursorColumn"), 1);
}

void KateView::writeSessionConfig(TDEConfig *config)
{
  config->writeEntry("CursorLine",m_viewInternal->cursor.line());
  config->writeEntry("CursorColumn",m_viewInternal->cursor.col());
}

int KateView::getEol()
{
  return m_doc->config()->eol();
}

void KateView::setEol(int eol)
{
  if (!doc()->isReadWrite())
    return;

  if (m_updatingDocumentConfig)
    return;

  m_doc->config()->setEol (eol);
}

void KateView::setIconBorder( bool enable )
{
  config()->setIconBar (enable);
}

void KateView::toggleIconBorder()
{
  config()->setIconBar (!config()->iconBar());
}

void KateView::setLineNumbersOn( bool enable )
{
  config()->setLineNumbers (enable);
}

void KateView::toggleLineNumbersOn()
{
  config()->setLineNumbers (!config()->lineNumbers());
}

void KateView::setScrollBarMarks( bool enable )
{
  config()->setScrollBarMarks (enable);
}

void KateView::toggleScrollBarMarks()
{
  config()->setScrollBarMarks (!config()->scrollBarMarks());
}

void KateView::toggleDynWordWrap()
{
  config()->setDynWordWrap( !config()->dynWordWrap() );
}

void KateView::setDynWordWrap( bool b )
{
  config()->setDynWordWrap( b );
}

void KateView::toggleWWMarker()
{
  m_renderer->config()->setWordWrapMarker (!m_renderer->config()->wordWrapMarker());
}

void KateView::setFoldingMarkersOn( bool enable )
{
  config()->setFoldingBar ( enable );
}

void KateView::toggleFoldingMarkers()
{
  config()->setFoldingBar ( !config()->foldingBar() );
}

bool KateView::iconBorder() {
  return m_viewInternal->leftBorder->iconBorderOn();
}

bool KateView::lineNumbersOn() {
  return m_viewInternal->leftBorder->lineNumbersOn();
}

bool KateView::scrollBarMarks() {
  return m_viewInternal->m_lineScroll->showMarks();
}

int KateView::dynWrapIndicators() {
  return m_viewInternal->leftBorder->dynWrapIndicators();
}

bool KateView::foldingMarkersOn() {
  return m_viewInternal->leftBorder->foldingMarkersOn();
}

void KateView::showCmdLine ( bool enabled )
{
  if (enabled == m_cmdLineOn)
    return;

  if (enabled)
  {
    if (!m_cmdLine)
    {
      m_cmdLine = new KateCmdLine (this);
      m_grid->addMultiCellWidget (m_cmdLine, 2, 2, 0, 2);
    }

    m_cmdLine->show ();
    m_cmdLine->setFocus();
  }
  else {
    m_cmdLine->hide ();
    //m_toggleCmdLine->setChecked(false);
  }

  m_cmdLineOn = enabled;
}

void KateView::toggleCmdLine ()
{
  m_config->setCmdLine (!m_config->cmdLine ());
}

void KateView::toggleWriteLock()
{
  m_doc->setReadWrite( ! m_doc->isReadWrite() );
}

void KateView::enableTextHints(int timeout)
{
  m_viewInternal->enableTextHints(timeout);
}

void KateView::disableTextHints()
{
  m_viewInternal->disableTextHints();
}

void KateView::applyWordWrap ()
{
  if (hasSelection())
    m_doc->wrapText (selectStart.line(), selectEnd.line());
  else
    m_doc->wrapText (0, m_doc->lastLine());
}

void KateView::slotNeedTextHint(int line, int col, TQString &text)
{
  text=TQString("test %1 %2").arg(line).arg(col);
}

void KateView::find()
{
  m_search->find();
}

void KateView::find( const TQString& pattern, long flags, bool add )
{
  m_search->find( pattern, flags, add );
}

void KateView::replace()
{
  m_search->replace();
}

void KateView::replace( const TQString &pattern, const TQString &replacement, long flags )
{
  m_search->replace( pattern, replacement, flags );
}

void KateView::findAgain( bool back )
{
  m_search->findAgain( back );
}

void KateView::slotSelectionChanged ()
{
  m_copy->setEnabled (hasSelection());
  m_copyHTML->setEnabled (hasSelection());
  m_deSelect->setEnabled (hasSelection());

  if (m_doc->readOnly())
    return;

  m_cut->setEnabled (hasSelection());

  m_spell->updateActions ();
}

void KateView::switchToCmdLine ()
{
  if (!m_cmdLineOn)
    m_config->setCmdLine (true);
  else {
	if (m_cmdLine->hasFocus()) {
		this->setFocus();
		return;
	}
  }
  m_cmdLine->setFocus ();
}

void KateView::showArgHint( TQStringList arg1, const TQString& arg2, const TQString& arg3 )
{
  m_codeCompletion->showArgHint( arg1, arg2, arg3 );
}

void KateView::showCompletionBox( TQValueList<KTextEditor::CompletionEntry> arg1, int offset, bool cs )
{
  emit aboutToShowCompletionBox();
  m_codeCompletion->showCompletionBox( arg1, offset, cs );
}

KateRenderer *KateView::renderer ()
{
  return m_renderer;
}

void KateView::updateConfig ()
{
  if (m_startingUp)
    return;

  m_editActions->readShortcutSettings( "Katepart Shortcuts" );

  // dyn. word wrap & markers
  if (m_hasWrap != config()->dynWordWrap()) {
    m_viewInternal->prepareForDynWrapChange();

    m_hasWrap = config()->dynWordWrap();

    m_viewInternal->dynWrapChanged();

    m_setDynWrapIndicators->setEnabled(config()->dynWordWrap());
    m_toggleDynWrap->setChecked( config()->dynWordWrap() );
  }

  m_viewInternal->leftBorder->setDynWrapIndicators( config()->dynWordWrapIndicators() );
  m_setDynWrapIndicators->setCurrentItem( config()->dynWordWrapIndicators() );

  // line numbers
  m_viewInternal->leftBorder->setLineNumbersOn( config()->lineNumbers() );
  m_toggleLineNumbers->setChecked( config()->lineNumbers() );

  // icon bar
  m_viewInternal->leftBorder->setIconBorderOn( config()->iconBar() );
  m_toggleIconBar->setChecked( config()->iconBar() );

  // scrollbar marks
  m_viewInternal->m_lineScroll->setShowMarks( config()->scrollBarMarks() );
  m_toggleScrollBarMarks->setChecked( config()->scrollBarMarks() );

  // cmd line
  showCmdLine (config()->cmdLine());
  //m_toggleCmdLine->setChecked( config()->cmdLine() );

  // misc edit
  m_toggleBlockSelection->setChecked( blockSelectionMode() );
  m_toggleInsert->setChecked( isOverwriteMode() );

  updateFoldingConfig ();

  // bookmark
  m_bookmarks->setSorting( (KateBookmarks::Sorting) config()->bookmarkSort() );

  m_viewInternal->setAutoCenterLines(config()->autoCenterLines ());
}

void KateView::updateDocumentConfig()
{
  if (m_startingUp)
    return;

  m_updatingDocumentConfig = true;

  m_setEndOfLine->setCurrentItem (m_doc->config()->eol());

  m_updatingDocumentConfig = false;

  m_viewInternal->updateView (true);

  m_renderer->setTabWidth (m_doc->config()->tabWidth());
  m_renderer->setIndentWidth (m_doc->config()->indentationWidth());
}

void KateView::updateRendererConfig()
{
  if (m_startingUp)
    return;

  m_toggleWWMarker->setChecked( m_renderer->config()->wordWrapMarker()  );

  // update the text area
  m_viewInternal->updateView (true);
  m_viewInternal->repaint ();

  // update the left border right, for example linenumbers
  m_viewInternal->leftBorder->updateFont();
  m_viewInternal->leftBorder->repaint ();

// @@ showIndentLines is not cached anymore.
//  m_renderer->setShowIndentLines (m_renderer->config()->showIndentationLines());
}

void KateView::updateFoldingConfig ()
{
  // folding bar
  bool doit = config()->foldingBar() && m_doc->highlight() && m_doc->highlight()->allowsFolding();
  m_viewInternal->leftBorder->setFoldingMarkersOn(doit);
  m_toggleFoldingMarkers->setChecked( doit );
  m_toggleFoldingMarkers->setEnabled( m_doc->highlight() && m_doc->highlight()->allowsFolding() );

  TQStringList l;

  l << "folding_toplevel" << "folding_expandtoplevel"
    << "folding_collapselocal" << "folding_expandlocal";

  TDEAction *a = 0;
  for (uint z = 0; z < l.size(); z++)
    if ((a = actionCollection()->action( l[z].ascii() )))
      a->setEnabled (m_doc->highlight() && m_doc->highlight()->allowsFolding());
}

//BEGIN EDIT STUFF
void KateView::editStart ()
{
  m_viewInternal->editStart ();
}

void KateView::editEnd (int editTagLineStart, int editTagLineEnd, bool tagFrom)
{
  m_viewInternal->editEnd (editTagLineStart, editTagLineEnd, tagFrom);
}

void KateView::editSetCursor (const KateTextCursor &cursor)
{
  m_viewInternal->editSetCursor (cursor);
}
//END

//BEGIN TAG & CLEAR
bool KateView::tagLine (const KateTextCursor& virtualCursor)
{
  return m_viewInternal->tagLine (virtualCursor);
}

bool KateView::tagLines (int start, int end, bool realLines)
{
  return m_viewInternal->tagLines (start, end, realLines);
}

bool KateView::tagLines (KateTextCursor start, KateTextCursor end, bool realCursors)
{
  return m_viewInternal->tagLines (start, end, realCursors);
}

void KateView::tagAll ()
{
  m_viewInternal->tagAll ();
}

void KateView::clear ()
{
  m_viewInternal->clear ();
}

void KateView::repaintText (bool paintOnlyDirty)
{
  m_viewInternal->paintText(0,0,m_viewInternal->width(),m_viewInternal->height(), paintOnlyDirty);
}

void KateView::updateView (bool changed)
{
  m_viewInternal->updateView (changed);
  m_viewInternal->leftBorder->update();
}

//END

void KateView::slotHlChanged()
{
  KateHighlighting *hl = m_doc->highlight();
  bool ok ( !hl->getCommentStart(0).isEmpty() || !hl->getCommentSingleLineStart(0).isEmpty() );

  if (actionCollection()->action("tools_comment"))
    actionCollection()->action("tools_comment")->setEnabled( ok );

  if (actionCollection()->action("tools_uncomment"))
    actionCollection()->action("tools_uncomment")->setEnabled( ok );

  // show folding bar if "view defaults" says so, otherwise enable/disable only the menu entry
  updateFoldingConfig ();
}

uint KateView::cursorColumn()
{
  uint r = m_doc->currentColumn(m_viewInternal->getCursor());
  if ( !( m_doc->config()->configFlags() & KateDocumentConfig::cfWrapCursor ) &&
       (uint)m_viewInternal->getCursor().col() > m_doc->textLine( m_viewInternal->getCursor().line() ).length()  )
    r += m_viewInternal->getCursor().col() - m_doc->textLine( m_viewInternal->getCursor().line() ).length();

  return r;
}

//BEGIN KTextEditor::SelectionInterface stuff

bool KateView::setSelection( const KateTextCursor& start, const KateTextCursor& end )
{
  KateTextCursor oldSelectStart = selectStart;
  KateTextCursor oldSelectEnd = selectEnd;

  if (start <= end) {
    selectStart.setPos(start);
    selectEnd.setPos(end);
  } else {
    selectStart.setPos(end);
    selectEnd.setPos(start);
  }

  tagSelection(oldSelectStart, oldSelectEnd);

  repaintText(true);

  emit selectionChanged ();
  emit m_doc->selectionChanged ();

  return true;
}

bool KateView::setSelection( uint startLine, uint startCol, uint endLine, uint endCol )
{
  if (hasSelection())
    clearSelection(false, false);

  return setSelection( KateTextCursor(startLine, startCol), KateTextCursor(endLine, endCol) );
}

void KateView::syncSelectionCache()
{
  m_viewInternal->selStartCached = selectStart;
  m_viewInternal->selEndCached = selectEnd;
  m_viewInternal->selectAnchor = selectEnd;
}

bool KateView::clearSelection()
{
  return clearSelection(true);
}

bool KateView::clearSelection(bool redraw, bool finishedChangingSelection)
{
  if( !hasSelection() )
    return false;

  KateTextCursor oldSelectStart = selectStart;
  KateTextCursor oldSelectEnd = selectEnd;

  selectStart.setPos(-1, -1);
  selectEnd.setPos(-1, -1);

  tagSelection(oldSelectStart, oldSelectEnd);

  oldSelectStart = selectStart;
  oldSelectEnd = selectEnd;

  if (redraw)
    repaintText(true);

  if (finishedChangingSelection)
  {
    emit selectionChanged();
    emit m_doc->selectionChanged ();
  }

  return true;
}

bool KateView::hasSelection() const
{
  return selectStart != selectEnd;
}

TQString KateView::selection() const
{
  int sc = selectStart.col();
  int ec = selectEnd.col();

  if ( blockSelect )
  {
    if (sc > ec)
    {
      uint tmp = sc;
      sc = ec;
      ec = tmp;
    }
  }
  return m_doc->text (selectStart.line(), sc, selectEnd.line(), ec, blockSelect);
}

bool KateView::removeSelectedText ()
{
  if (!hasSelection())
    return false;

  m_doc->editStart ();

  int sc = selectStart.col();
  int ec = selectEnd.col();

  if ( blockSelect )
  {
    if (sc > ec)
    {
      uint tmp = sc;
      sc = ec;
      ec = tmp;
    }
  }

  m_doc->removeText (selectStart.line(), sc, selectEnd.line(), ec, blockSelect);

  // don't redraw the cleared selection - that's done in editEnd().
  clearSelection(false);

  m_doc->editEnd ();

  return true;
}

bool KateView::selectAll()
{
  setBlockSelectionMode (false);

  return setSelection (0, 0, m_doc->lastLine(), m_doc->lineLength(m_doc->lastLine()));
}

bool KateView::lineColSelected (int line, int col)
{
  if ( (!blockSelect) && (col < 0) )
    col = 0;

  KateTextCursor cursor(line, col);

  if (blockSelect)
    return cursor.line() >= selectStart.line() && cursor.line() <= selectEnd.line() && cursor.col() >= selectStart.col() && cursor.col() < selectEnd.col();
  else
    return (cursor >= selectStart) && (cursor < selectEnd);
}

bool KateView::lineSelected (int line)
{
  return (!blockSelect)
    && (selectStart <= KateTextCursor(line, 0))
    && (line < selectEnd.line());
}

bool KateView::lineEndSelected (int line, int endCol)
{
  return (!blockSelect)
    && (line > selectStart.line() || (line == selectStart.line() && (selectStart.col() < endCol || endCol == -1)))
    && (line < selectEnd.line() || (line == selectEnd.line() && (endCol <= selectEnd.col() && endCol != -1)));
}

bool KateView::lineHasSelected (int line)
{
  return (selectStart < selectEnd)
    && (line >= selectStart.line())
    && (line <= selectEnd.line());
}

bool KateView::lineIsSelection (int line)
{
  return (line == selectStart.line() && line == selectEnd.line());
}

void KateView::tagSelection(const KateTextCursor &oldSelectStart, const KateTextCursor &oldSelectEnd)
{
  if (hasSelection()) {
    if (oldSelectStart.line() == -1) {
      // We have to tag the whole lot if
      // 1) we have a selection, and:
      //  a) it's new; or
      tagLines(selectStart, selectEnd, true);

    } else if (blockSelectionMode() && (oldSelectStart.col() != selectStart.col() || oldSelectEnd.col() != selectEnd.col())) {
      //  b) we're in block selection mode and the columns have changed
      tagLines(selectStart, selectEnd, true);
      tagLines(oldSelectStart, oldSelectEnd, true);

    } else {
      if (oldSelectStart != selectStart) {
        if (oldSelectStart < selectStart)
          tagLines(oldSelectStart, selectStart, true);
        else
          tagLines(selectStart, oldSelectStart, true);
      }

      if (oldSelectEnd != selectEnd) {
        if (oldSelectEnd < selectEnd)
          tagLines(oldSelectEnd, selectEnd, true);
        else
          tagLines(selectEnd, oldSelectEnd, true);
      }
    }

  } else {
    // No more selection, clean up
    tagLines(oldSelectStart, oldSelectEnd, true);
  }
}

void KateView::selectWord( const KateTextCursor& cursor )
{
  int start, end, len;

  KateTextLine::Ptr textLine = m_doc->plainKateTextLine(cursor.line());

  if (!textLine)
    return;

  len = textLine->length();
  start = end = cursor.col();
  while (start > 0 && m_doc->highlight()->isInWord(textLine->getChar(start - 1), textLine->attribute(start - 1))) start--;
  while (end < len && m_doc->highlight()->isInWord(textLine->getChar(end), textLine->attribute(start - 1))) end++;
  if (end <= start) return;

  setSelection (cursor.line(), start, cursor.line(), end);
}

void KateView::selectLine( const KateTextCursor& cursor )
{
  if (cursor.line()+1 >= m_doc->numLines())
    setSelection (cursor.line(), 0, cursor.line(), m_doc->lineLength(cursor.line()));
  else
    setSelection (cursor.line(), 0, cursor.line()+1, 0);
}

void KateView::selectLength( const KateTextCursor& cursor, int length )
{
  int start, end;

  KateTextLine::Ptr textLine = m_doc->plainKateTextLine(cursor.line());

  if (!textLine)
    return;

  start = cursor.col();
  end = start + length;
  if (end <= start) return;

  setSelection (cursor.line(), start, cursor.line(), end);
}

void KateView::paste()
{
  m_doc->paste( this );
  emit selectionChanged();
  m_viewInternal->repaint();
}

void KateView::cut()
{
  if (!hasSelection())
    return;

  copy();
  removeSelectedText();
}

void KateView::copy() const
{
  if (!hasSelection())
    return;

  TQApplication::clipboard()->setText(selection ());
}

void KateView::copyHTML()
{
  if (!hasSelection())
    return;

  KMultipleDrag *drag = new KMultipleDrag();

  TQTextDrag *htmltextdrag = new TQTextDrag(selectionAsHtml()) ;
  htmltextdrag->setSubtype("html");

  drag->addDragObject( htmltextdrag);
  drag->addDragObject( new TQTextDrag( selection()));

  TQApplication::clipboard()->setData(drag);
}

TQString KateView::selectionAsHtml()
{
  int sc = selectStart.col();
  int ec = selectEnd.col();

  if ( blockSelect )
  {
    if (sc > ec)
    {
      uint tmp = sc;
      sc = ec;
      ec = tmp;
    }
  }

  return textAsHtml (selectStart.line(), sc, selectEnd.line(), ec, blockSelect);
}

TQString KateView::textAsHtml ( uint startLine, uint startCol, uint endLine, uint endCol, bool blockwise)
{
  kdDebug(13020) << "textAsHtml" << endl;
  if ( blockwise && (startCol > endCol) )
    return TQString ();

  TQString s;
  TQTextStream ts( &s, IO_WriteOnly );
  ts.setEncoding(TQTextStream::UnicodeUTF8);
  ts << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"DTD/xhtml1-strict.dtd\">" << endl;
  ts << "<html xmlns=\"http://www.w3.org/1999/xhtml\">" << endl;
  ts << "<head>" << endl;
  ts << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />" << endl;
  ts << "<meta name=\"Generator\" content=\"Kate, the KDE Advanced Text Editor\" />" << endl;
  ts << "</head>" << endl;

  ts << "<body>" << endl;
  textAsHtmlStream(startLine, startCol, endLine, endCol, blockwise, &ts);

  ts << "</body>" << endl;
  ts << "</html>" << endl;
  kdDebug(13020) << "html is: " << s << endl;
  return s;
}

void KateView::textAsHtmlStream ( uint startLine, uint startCol, uint endLine, uint endCol, bool blockwise, TQTextStream *ts)
{
  if ( (blockwise || startLine == endLine) && (startCol > endCol) )
    return;

  if (startLine == endLine)
  {
    KateTextLine::Ptr textLine = m_doc->kateTextLine(startLine);
    if ( !textLine )
      return;

    (*ts) << "<pre>" << endl;

    lineAsHTML(textLine, startCol, endCol-startCol, ts);
  }
  else
  {
    (*ts) << "<pre>" << endl;

    for (uint i = startLine; (i <= endLine) && (i < m_doc->numLines()); i++)
    {
      KateTextLine::Ptr textLine = m_doc->kateTextLine(i);

      if ( !blockwise )
      {
        if (i == startLine)
          lineAsHTML(textLine, startCol, textLine->length()-startCol, ts);
        else if (i == endLine)
          lineAsHTML(textLine, 0, endCol, ts);
        else
          lineAsHTML(textLine, 0, textLine->length(), ts);
      }
      else
      {
        lineAsHTML( textLine, startCol, endCol-startCol, ts);
      }

      if ( i < endLine )
        (*ts) << "\n";    //we are inside a <pre>, so a \n is a new line
    }
  }
  (*ts) << "</pre>";
}

// fully rewritten to use only inline CSS and support all used attribs.
// anders, 2005-11-01 23:39:43
void KateView::lineAsHTML (KateTextLine::Ptr line, uint startCol, uint length, TQTextStream *outputStream)
{
  if(length == 0)
    return;

  // do not recalculate the style strings again and again
  TQMap<uchar,TQString> stylecache;
  // do not insert equally styled characters one by one
  TQString textcache;

  KateAttribute *charAttributes = 0;

  for (uint curPos=startCol;curPos<(length+startCol);curPos++)
  {
    if ( curPos == 0 || line->attribute( curPos ) != line->attribute( curPos - 1 ) &&
         // Since many highlight files contains itemdatas that have the exact
         // same styles, join those to keep the HTML text size down
         KateAttribute(*charAttributes) != KateAttribute(*m_renderer->attribute(line->attribute(curPos))) )
    {
      (*outputStream) << textcache;
      textcache.truncate(0);

      if ( curPos > startCol )
        (*outputStream) << "</span>";

      charAttributes = m_renderer->attribute(line->attribute(curPos));

      if ( ! stylecache.contains( line->attribute(curPos) ) )
      {
        TQString textdecoration;
        TQString style;

        if ( charAttributes->bold() )
          style.append("font-weight: bold;");
        if ( charAttributes->italic() )
          style.append("font-style: italic;");
        if ( charAttributes->underline() )
          textdecoration = "underline";
        if ( charAttributes->overline() )
          textdecoration.append(" overline" );
        if ( charAttributes->strikeOut() )
          textdecoration.append(" line-trough" );
        if ( !textdecoration.isEmpty() )
          style.append("text-decoration: %1;").arg(textdecoration);
        // TQColor::name() returns a string in the form "#RRGGBB" in Qt 3.
        // NOTE Qt 4 returns "#AARRGGBB"
        if ( charAttributes->itemSet(KateAttribute::BGColor) )
          style.append(TQString("background-color: %1;").arg(charAttributes->bgColor().name()));
        if ( charAttributes->itemSet(KateAttribute::TextColor) )
          style.append(TQString("color: %1;").arg(charAttributes->textColor().name()));

        stylecache[line->attribute(curPos)] = style;
      }
      (*outputStream)<<"<span style=\""
          << stylecache[line->attribute(curPos)]
          << "\">";
    }

    TQString s( line->getChar(curPos) );
    if ( s == "&" ) s = "&amp;";
    else if ( s == "<" ) s = "&lt;";
    else if ( s == ">" ) s = "&gt;";
    textcache.append( s );
  }

  (*outputStream) << textcache << "</span>";
}

void KateView::exportAsHTML ()
{
  KURL url = KFileDialog::getSaveURL(m_doc->docName(),"text/html",0,i18n("Export File as HTML"));

  if ( url.isEmpty() )
    return;

  TQString filename;
  KTempFile tmp; // ### only used for network export

  if ( url.isLocalFile() )
    filename = url.path();
  else
    filename = tmp.name();

  KSaveFile *savefile=new KSaveFile(filename);
  if (!savefile->status())
  {
    TQTextStream *outputStream = savefile->textStream();

    outputStream->setEncoding(TQTextStream::UnicodeUTF8);

    // let's write the HTML header :
    (*outputStream) << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    (*outputStream) << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"DTD/xhtml1-strict.dtd\">" << endl;
    (*outputStream) << "<html xmlns=\"http://www.w3.org/1999/xhtml\">" << endl;
    (*outputStream) << "<head>" << endl;
    (*outputStream) << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />" << endl;
    (*outputStream) << "<meta name=\"Generator\" content=\"Kate, the KDE Advanced Text Editor\" />" << endl;
    // for the title, we write the name of the file (/usr/local/emmanuel/myfile.cpp -> myfile.cpp)
    (*outputStream) << "<title>" << m_doc->docName () << "</title>" << endl;
    (*outputStream) << "</head>" << endl;
    (*outputStream) << "<body>" << endl;

    textAsHtmlStream(0,0, m_doc->lastLine(), m_doc->lineLength(m_doc->lastLine()), false, outputStream);

    (*outputStream) << "</body>" << endl;
    (*outputStream) << "</html>" << endl;


    savefile->close();
    //if (!savefile->status()) --> Error
  }
//     else
//       {/*ERROR*/}
  delete savefile;

  if ( url.isLocalFile() )
      return;

  TDEIO::NetAccess::upload( filename, url, 0 );
}
//END

//BEGIN KTextEditor::BlockSelectionInterface stuff

bool KateView::blockSelectionMode ()
{
  return blockSelect;
}

bool KateView::setBlockSelectionMode (bool on)
{
  if (on != blockSelect)
  {
    blockSelect = on;

    KateTextCursor oldSelectStart = selectStart;
    KateTextCursor oldSelectEnd = selectEnd;

    clearSelection(false, false);

    setSelection(oldSelectStart, oldSelectEnd);

    slotSelectionTypeChanged();
  }

  return true;
}

bool KateView::toggleBlockSelectionMode ()
{
  m_toggleBlockSelection->setChecked (!blockSelect);
  return setBlockSelectionMode (!blockSelect);
}

bool KateView::wrapCursor ()
{
  return !blockSelectionMode() && (m_doc->configFlags() & KateDocument::cfWrapCursor);
}

//END

//BEGIN IM INPUT STUFF
void KateView::setIMSelectionValue( uint imStartLine, uint imStart, uint imEnd,
                                        uint imSelStart, uint imSelEnd, bool imComposeEvent )
{
  m_imStartLine = imStartLine;
  m_imStart = imStart;
  m_imEnd = imEnd;
  m_imSelStart = imSelStart;
  m_imSelEnd = imSelEnd;
  m_imComposeEvent = imComposeEvent;
}

bool KateView::isIMSelection( int _line, int _column )
{
  return ( ( int( m_imStartLine ) == _line ) && ( m_imSelStart < m_imSelEnd ) && ( _column >= int( m_imSelStart ) ) &&
    ( _column < int( m_imSelEnd ) ) );
}

bool KateView::isIMEdit( int _line, int _column )
{
  return ( ( int( m_imStartLine ) == _line ) && ( m_imStart < m_imEnd ) && ( _column >= int( m_imStart ) ) &&
    ( _column < int( m_imEnd ) ) );
}

void KateView::getIMSelectionValue( uint *imStartLine, uint *imStart, uint *imEnd,
                                        uint *imSelStart, uint *imSelEnd )
{
  *imStartLine = m_imStartLine;
  *imStart = m_imStart;
  *imEnd = m_imEnd;
  *imSelStart = m_imSelStart;
  *imSelEnd = m_imSelEnd;
}
//END IM INPUT STUFF
