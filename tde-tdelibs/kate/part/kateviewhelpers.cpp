/* This file is part of the KDE libraries
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001 Anders Lund <anders@alweb.dk>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>

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

#include "kateviewhelpers.h"
#include "kateviewhelpers.moc"

#include "../interfaces/document.h"
#include "../interfaces/katecmd.h"
#include "kateattribute.h"
#include "katecodefoldinghelpers.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "katefactory.h"
#include "katerenderer.h"
#include "kateview.h"
#include "kateviewinternal.h"

#include <tdeapplication.h>
#include <tdeglobalsettings.h>
#include <tdelocale.h>
#include <knotifyclient.h>
#include <tdeglobal.h>
#include <kcharsets.h>
#include <tdepopupmenu.h>

#include <tqcursor.h>
#include <tqpainter.h>
#include <tqpopupmenu.h>
#include <tqstyle.h>
#include <tqtimer.h>
#include <tqwhatsthis.h>
#include <tqregexp.h>
#include <tqtextcodec.h>

#include <math.h>

#include <kdebug.h>

//BEGIN KateScrollBar
KateScrollBar::KateScrollBar (Orientation orientation, KateViewInternal* parent, const char* name)
  : TQScrollBar (orientation, parent->m_view, name)
  , m_middleMouseDown (false)
  , m_view(parent->m_view)
  , m_doc(parent->m_doc)
  , m_viewInternal(parent)
  , m_topMargin(-1)
  , m_bottomMargin(-1)
  , m_savVisibleLines(0)
  , m_showMarks(false)
{
  connect(this, TQ_SIGNAL(valueChanged(int)), TQ_SLOT(sliderMaybeMoved(int)));
  connect(m_doc, TQ_SIGNAL(marksChanged()), this, TQ_SLOT(marksChanged()));

  m_lines.setAutoDelete(true);
}

void KateScrollBar::mousePressEvent(TQMouseEvent* e)
{
  if (e->button() == TQt::MidButton)
    m_middleMouseDown = true;

  TQScrollBar::mousePressEvent(e);

  redrawMarks();
}

void KateScrollBar::mouseReleaseEvent(TQMouseEvent* e)
{
  TQScrollBar::mouseReleaseEvent(e);

  m_middleMouseDown = false;

  redrawMarks();
}

void KateScrollBar::mouseMoveEvent(TQMouseEvent* e)
{
  TQScrollBar::mouseMoveEvent(e);

  if (e->state() | TQt::LeftButton)
    redrawMarks();
}

void KateScrollBar::paintEvent(TQPaintEvent *e)
{
  TQScrollBar::paintEvent(e);
  redrawMarks();
}

void KateScrollBar::resizeEvent(TQResizeEvent *e)
{
  TQScrollBar::resizeEvent(e);
  recomputeMarksPositions();
}

void KateScrollBar::styleChange(TQStyle &s)
{
  TQScrollBar::styleChange(s);
  m_topMargin = -1;
  recomputeMarksPositions();
}

void KateScrollBar::valueChange()
{
  TQScrollBar::valueChange();
  redrawMarks();
}

void KateScrollBar::rangeChange()
{
  TQScrollBar::rangeChange();
  recomputeMarksPositions();
}

void KateScrollBar::marksChanged()
{
  recomputeMarksPositions(true);
}

void KateScrollBar::redrawMarks()
{
  if (!m_showMarks)
    return;

  TQPainter painter(this);
  TQRect rect = sliderRect();
  for (TQIntDictIterator<TQColor> it(m_lines); it.current(); ++it)
  {
    if (it.currentKey() < rect.top() || it.currentKey() > rect.bottom())
    {
      painter.setPen(*it.current());
      painter.drawLine(0, it.currentKey(), width(), it.currentKey());
    }
  }
}

void KateScrollBar::recomputeMarksPositions(bool forceFullUpdate)
{
  if (m_topMargin == -1)
    watchScrollBarSize();

  m_lines.clear();
  m_savVisibleLines = m_doc->visibleLines();

  int realHeight = frameGeometry().height() - m_topMargin - m_bottomMargin;

  TQPtrList<KTextEditor::Mark> marks = m_doc->marks();
  KateCodeFoldingTree *tree = m_doc->foldingTree();

  for (KTextEditor::Mark *mark = marks.first(); mark; mark = marks.next())
  {
    uint line = mark->line;

    if (tree)
    {
      KateCodeFoldingNode *node = tree->findNodeForLine(line);

      while (node)
      {
        if (!node->isVisible())
          line = tree->getStartLine(node);
        node = node->getParentNode();
      }
    }

    line = m_doc->getVirtualLine(line);

    double d = (double)line / (m_savVisibleLines - 1);
    m_lines.insert(m_topMargin + (int)(d * realHeight),
                   new TQColor(KateRendererConfig::global()->lineMarkerColor((KTextEditor::MarkInterface::MarkTypes)mark->type)));
  }

  if (forceFullUpdate)
    update();
  else
    redrawMarks();
}

void KateScrollBar::watchScrollBarSize()
{
  int savMax = maxValue();
  setMaxValue(0);
  TQRect rect = sliderRect();
  setMaxValue(savMax);

  m_topMargin = rect.top();
  m_bottomMargin = frameGeometry().height() - rect.bottom();
}

void KateScrollBar::sliderMaybeMoved(int value)
{
  if (m_middleMouseDown)
    emit sliderMMBMoved(value);
}
//END

//BEGIN KateCmdLnWhatsThis
class KateCmdLnWhatsThis : public TQWhatsThis
{
  public:
    KateCmdLnWhatsThis( KateCmdLine *parent )
  : TQWhatsThis( parent )
  , m_parent( parent ) {;}

    TQString text( const TQPoint & )
    {
      TQString beg = "<qt background=\"white\"><div><table width=\"100%\"><tr><td bgcolor=\"brown\"><font color=\"white\"><b>Help: <big>";
      TQString mid = "</big></b></font></td></tr><tr><td>";
      TQString end = "</td></tr></table></div><qt>";

      TQString t = m_parent->text();
      TQRegExp re( "\\s*help\\s+(.*)" );
      if ( re.search( t ) > -1 )
      {
        TQString s;
        // get help for command
        TQString name = re.cap( 1 );
        if ( name == "list" )
        {
          return beg + i18n("Available Commands") + mid
              + KateCmd::self()->cmds().join(" ")
              + i18n("<p>For help on individual commands, do <code>'help &lt;command&gt;'</code></p>")
              + end;
        }
        else if ( ! name.isEmpty() )
        {
          Kate::Command *cmd = KateCmd::self()->queryCommand( name );
          if ( cmd )
          {
            if ( cmd->help( (Kate::View*)m_parent->parentWidget(), name, s ) )
              return beg + name + mid + s + end;
            else
              return beg + name + mid + i18n("No help for '%1'").arg( name ) + end;
          }
          else
            return beg + mid + i18n("No such command <b>%1</b>").arg(name) + end;
        }
      }

      return beg + mid + i18n(
          "<p>This is the Katepart <b>command line</b>.<br>"
          "Syntax: <code><b>command [ arguments ]</b></code><br>"
          "For a list of available commands, enter <code><b>help list</b></code><br>"
          "For help for individual commands, enter <code><b>help &lt;command&gt;</b></code></p>")
          + end;
    }

  private:
    KateCmdLine *m_parent;
};
//END KateCmdLnWhatsThis

//BEGIN KateCmdLineFlagCompletion
/**
 * This class provide completion of flags. It shows a short description of
 * each flag, and flags are appended.
 */
class KateCmdLineFlagCompletion : public TDECompletion
{
  public:
    KateCmdLineFlagCompletion() {;}

    TQString makeCompletion( const TQString & string )
    {
      return TQString::null;
    }

};
//END KateCmdLineFlagCompletion

//BEGIN KateCmdLine
KateCmdLine::KateCmdLine (KateView *view)
  : KLineEdit (view)
  , m_view (view)
  , m_msgMode (false)
  , m_histpos( 0 )
  , m_cmdend( 0 )
  , m_command( 0L )
  , m_oldCompletionObject( 0L )
{
  connect (this, TQ_SIGNAL(returnPressed(const TQString &)),
           this, TQ_SLOT(slotReturnPressed(const TQString &)));

  completionObject()->insertItems (KateCmd::self()->cmds());
  setAutoDeleteCompletionObject( false );
  m_help = new KateCmdLnWhatsThis( this );
}

void KateCmdLine::slotReturnPressed ( const TQString& text )
{

  // silently ignore leading space
  uint n = 0;
  while( text[n].isSpace() )
    n++;

  TQString cmd = text.mid( n );

  // Built in help: if the command starts with "help", [try to] show some help
  if ( cmd.startsWith( "help" ) )
  {
    m_help->display( m_help->text( TQPoint() ), mapToGlobal(TQPoint(0,0)) );
    clear();
    KateCmd::self()->appendHistory( cmd );
    m_histpos = KateCmd::self()->historyLength();
    m_oldText = TQString ();
    return;
  }

  if (cmd.length () > 0)
  {
    Kate::Command *p = KateCmd::self()->queryCommand (cmd);

    m_oldText = cmd;
    m_msgMode = true;

    if (p)
    {
      TQString msg;

      if (p->exec (m_view, cmd, msg))
      {
        KateCmd::self()->appendHistory( cmd );
        m_histpos = KateCmd::self()->historyLength();
        m_oldText = TQString ();

        if (msg.length() > 0)
          setText (i18n ("Success: ") + msg);
        else
          setText (i18n ("Success"));
      }
      else
      {
        if (msg.length() > 0)
          setText (i18n ("Error: ") + msg);
        else
          setText (i18n ("Command \"%1\" failed.").arg (cmd));
        KNotifyClient::beep();
      }
    }
    else
    {
      setText (i18n ("No such command: \"%1\"").arg (cmd));
      KNotifyClient::beep();
    }
  }

  // clean up
  if ( m_oldCompletionObject )
  {
    TDECompletion *c = completionObject();
    setCompletionObject( m_oldCompletionObject );
    m_oldCompletionObject = 0;
    delete c;
    c = 0;
  }
  m_command = 0;
  m_cmdend = 0;

  m_view->setFocus ();
  TQTimer::singleShot( 4000, this, TQ_SLOT(hideMe()) );
}

void KateCmdLine::hideMe () // unless i have focus ;)
{
  if ( isVisibleTo(parentWidget()) && ! hasFocus() ) {
     m_view->toggleCmdLine ();
  }
}

void KateCmdLine::focusInEvent ( TQFocusEvent *ev )
{
  if (m_msgMode)
  {
    m_msgMode = false;
    setText (m_oldText);
    selectAll();
  }

  KLineEdit::focusInEvent (ev);
}

void KateCmdLine::keyPressEvent( TQKeyEvent *ev )
{
  if (ev->key() == Key_Escape)
  {
    m_view->setFocus ();
    hideMe();
  }
  else if ( ev->key() == Key_Up )
    fromHistory( true );
  else if ( ev->key() == Key_Down )
    fromHistory( false );

  uint cursorpos = cursorPosition();
  KLineEdit::keyPressEvent (ev);

  // during typing, let us see if we have a valid command
  if ( ! m_cmdend || cursorpos <= m_cmdend  )
  {
    TQChar c;
    if ( ! ev->text().isEmpty() )
      c = ev->text()[0];

    if ( ! m_cmdend && ! c.isNull() ) // we have no command, so lets see if we got one
    {
      if ( ! c.isLetterOrNumber() && c != '-' && c != '_' )
      {
        m_command = KateCmd::self()->queryCommand( text().stripWhiteSpace() );
        if ( m_command )
        {
          //kdDebug(13025)<<"keypress in commandline: We have a command! "<<m_command<<". text is '"<<text()<<"'"<<endl;
          // if the typed character is ":",
          // we try if the command has flag completions
          m_cmdend = cursorpos;
          //kdDebug(13025)<<"keypress in commandline: Set m_cmdend to "<<m_cmdend<<endl;
        }
        else
          m_cmdend = 0;
      }
    }
    else // since cursor is inside the command name, we reconsider it
    {
      kdDebug(13025)<<"keypress in commandline: \\W -- text is "<<text()<<endl;
      m_command = KateCmd::self()->queryCommand( text().stripWhiteSpace() );
      if ( m_command )
      {
        //kdDebug(13025)<<"keypress in commandline: We have a command! "<<m_command<<endl;
        TQString t = text();
        m_cmdend = 0;
        bool b = false;
        for ( ; m_cmdend < t.length(); m_cmdend++ )
        {
          if ( t[m_cmdend].isLetter() )
            b = true;
          if ( b && ( ! t[m_cmdend].isLetterOrNumber() && t[m_cmdend] != '-' && t[m_cmdend] != '_' ) )
            break;
        }

        if ( c == ':' && cursorpos == m_cmdend )
        {
          // check if this command wants to complete flags
          //kdDebug(13025)<<"keypress in commandline: Checking if flag completion is desired!"<<endl;
        }
      }
      else
      {
        // clean up if needed
        if ( m_oldCompletionObject )
        {
          TDECompletion *c = completionObject();
          setCompletionObject( m_oldCompletionObject );
          m_oldCompletionObject = 0;
          delete c;
          c = 0;
        }

        m_cmdend = 0;
      }
    }

    // if we got a command, check if it wants to do semething.
    if ( m_command )
    {
      //kdDebug(13025)<<"Checking for CommandExtension.."<<endl;
      Kate::CommandExtension *ce = dynamic_cast<Kate::CommandExtension*>(m_command);
      if ( ce )
      {
        TDECompletion *cmpl = ce->completionObject( text().left( m_cmdend ).stripWhiteSpace(), m_view );
        if ( cmpl )
        {
        // save the old completion object and use what the command provides
        // instead. We also need to prepend the current command name + flag string
        // when completion is done
          //kdDebug(13025)<<"keypress in commandline: Setting completion object!"<<endl;
          if ( ! m_oldCompletionObject )
            m_oldCompletionObject = completionObject();

          setCompletionObject( cmpl );
        }
      }
    }
  }
  else if ( m_command )// check if we should call the commands processText()
  {
    Kate::CommandExtension *ce = dynamic_cast<Kate::CommandExtension*>( m_command );
    if ( ce && ce->wantsToProcessText( text().left( m_cmdend ).stripWhiteSpace() )
         && ! ( ev->text().isNull() || ev->text().isEmpty() ) )
      ce->processText( m_view, text() );
  }
}

void KateCmdLine::fromHistory( bool up )
{
  if ( ! KateCmd::self()->historyLength() )
    return;

  TQString s;

  if ( up )
  {
    if ( m_histpos > 0 )
    {
      m_histpos--;
      s = KateCmd::self()->fromHistory( m_histpos );
    }
  }
  else
  {
    if ( m_histpos < ( KateCmd::self()->historyLength() - 1 ) )
    {
      m_histpos++;
      s = KateCmd::self()->fromHistory( m_histpos );
    }
    else
    {
      m_histpos = KateCmd::self()->historyLength();
      setText( m_oldText );
    }
  }
  if ( ! s.isEmpty() )
  {
    // Select the argument part of the command, so that it is easy to overwrite
    setText( s );
    static TQRegExp reCmd = TQRegExp(".*[\\w\\-]+(?:[^a-zA-Z0-9_-]|:\\w+)(.*)");
    if ( reCmd.search( text() ) == 0 )
      setSelection( text().length() - reCmd.cap(1).length(), reCmd.cap(1).length() );
  }
}
//END KateCmdLine

//BEGIN KateIconBorder
using namespace KTextEditor;

static const char* const plus_xpm[] = {
"11 11 3 1",
"       c None",
".      c #000000",
"+      c #FFFFFF",
"...........",
".+++++++++.",
".+++++++++.",
".++++.++++.",
".++++.++++.",
".++.....++.",
".++++.++++.",
".++++.++++.",
".+++++++++.",
".+++++++++.",
"..........."};

static const char* const minus_xpm[] = {
"11 11 3 1",
"       c None",
".      c #000000",
"+      c #FFFFFF",
"...........",
".+++++++++.",
".+++++++++.",
".+++++++++.",
".+++++++++.",
".++.....++.",
".+++++++++.",
".+++++++++.",
".+++++++++.",
".+++++++++.",
"..........."};

static const char * const bookmark_xpm[] = {
"14 13 82 1",
"   c None",
".  c #F27D01",
"+  c #EF7901",
"@  c #F3940F",
"#  c #EE8F12",
"$  c #F9C834",
"%  c #F5C33A",
"&  c #F09110",
"*  c #FCEE3E",
"=  c #FBEB3F",
"-  c #E68614",
";  c #FA8700",
">  c #F78703",
",  c #F4920E",
"'  c #F19113",
")  c #F6C434",
"!  c #FDF938",
"~  c #FDF839",
"{  c #F1BC3A",
"]  c #E18017",
"^  c #DA7210",
"/  c #D5680B",
"(  c #CA5404",
"_  c #FD8F06",
":  c #FCB62D",
"<  c #FDE049",
"[  c #FCE340",
"}  c #FBE334",
"|  c #FDF035",
"1  c #FEF834",
"2  c #FCEF36",
"3  c #F8DF32",
"4  c #F7DC3D",
"5  c #F5CE3E",
"6  c #DE861B",
"7  c #C64C03",
"8  c #F78C07",
"9  c #F8B019",
"0  c #FDE12D",
"a  c #FEE528",
"b  c #FEE229",
"c  c #FBD029",
"d  c #E18814",
"e  c #CB5605",
"f  c #EF8306",
"g  c #F3A00E",
"h  c #FBC718",
"i  c #FED31C",
"j  c #FED11D",
"k  c #F8B91C",
"l  c #E07D0D",
"m  c #CB5301",
"n  c #ED8A0E",
"o  c #F7A90D",
"p  c #FEC113",
"q  c #FEC013",
"r  c #F09B0E",
"s  c #D35E03",
"t  c #EF9213",
"u  c #F9A208",
"v  c #FEAA0C",
"w  c #FCA10B",
"x  c #FCA70B",
"y  c #FEAF0B",
"z  c #F39609",
"A  c #D86203",
"B  c #F08C0D",
"C  c #FA9004",
"D  c #F17F04",
"E  c #E36D04",
"F  c #E16F03",
"G  c #EE8304",
"H  c #F88C04",
"I  c #DC6202",
"J  c #E87204",
"K  c #E66A01",
"L  c #DC6001",
"M  c #D15601",
"N  c #DA5D01",
"O  c #D25200",
"P  c #DA5F00",
"Q  c #BC3C00",
"      .+      ",
"      @#      ",
"      $%      ",
"     &*=-     ",
" ;>,')!~{]^/( ",
"_:<[}|11234567",
" 890aaaaabcde ",
"  fghiiijklm  ",
"   nopqpqrs   ",
"   tuvwxyzA   ",
"   BCDEFGHI   ",
"   JKL  MNO   ",
"   P      Q   "};

const int iconPaneWidth = 16;
const int halfIPW = 8;

KateIconBorder::KateIconBorder ( KateViewInternal* internalView, TQWidget *parent )
  : TQWidget(parent, "", (WFlags)(WStaticContents | WRepaintNoErase | WResizeNoErase) )
  , m_view( internalView->m_view )
  , m_doc( internalView->m_doc )
  , m_viewInternal( internalView )
  , m_iconBorderOn( false )
  , m_lineNumbersOn( false )
  , m_foldingMarkersOn( false )
  , m_dynWrapIndicatorsOn( false )
  , m_dynWrapIndicators( 0 )
  , m_cachedLNWidth( 0 )
  , m_maxCharWidth( 0 )
{
  setSizePolicy( TQSizePolicy(  TQSizePolicy::Fixed, TQSizePolicy::Minimum ) );

  setBackgroundMode( NoBackground );

  m_doc->setDescription( MarkInterface::markType01, i18n("Bookmark") );
  m_doc->setPixmap( MarkInterface::markType01, TQPixmap((const char**)bookmark_xpm) );

  updateFont();
}

void KateIconBorder::setIconBorderOn( bool enable )
{
  if( enable == m_iconBorderOn )
    return;

  m_iconBorderOn = enable;

  updateGeometry();

  TQTimer::singleShot( 0, this, TQ_SLOT(update()) );
}

void KateIconBorder::setLineNumbersOn( bool enable )
{
  if( enable == m_lineNumbersOn )
    return;

  m_lineNumbersOn = enable;
  m_dynWrapIndicatorsOn = (m_dynWrapIndicators == 1) ? enable : m_dynWrapIndicators;

  updateGeometry();

  TQTimer::singleShot( 0, this, TQ_SLOT(update()) );
}

void KateIconBorder::setDynWrapIndicators( int state )
{
  if (state == m_dynWrapIndicators )
    return;

  m_dynWrapIndicators = state;
  m_dynWrapIndicatorsOn = (state == 1) ? m_lineNumbersOn : state;

  updateGeometry ();

  TQTimer::singleShot( 0, this, TQ_SLOT(update()) );
}

void KateIconBorder::setFoldingMarkersOn( bool enable )
{
  if( enable == m_foldingMarkersOn )
    return;

  m_foldingMarkersOn = enable;

  updateGeometry();

  TQTimer::singleShot( 0, this, TQ_SLOT(update()) );
}

TQSize KateIconBorder::sizeHint() const
{
  int w = 0;

  if (m_iconBorderOn)
    w += iconPaneWidth + 1;

  if (m_lineNumbersOn || (m_view->dynWordWrap() && m_dynWrapIndicatorsOn)) {
    w += lineNumberWidth();
  }

  if (m_foldingMarkersOn)
    w += iconPaneWidth;

  w += 4;

  return TQSize( w, 0 );
}

// This function (re)calculates the maximum width of any of the digit characters (0 -> 9)
// for graceful handling of variable-width fonts as the linenumber font.
void KateIconBorder::updateFont()
{
  const TQFontMetrics *fm = m_view->renderer()->config()->fontMetrics();
  m_maxCharWidth = 0;
  // Loop to determine the widest numeric character in the current font.
  // 48 is ascii '0'
  for (int i = 48; i < 58; i++) {
    int charWidth = fm->width( TQChar(i) );
    m_maxCharWidth = kMax(m_maxCharWidth, charWidth);
  }
}

int KateIconBorder::lineNumberWidth() const
{
  int width = m_lineNumbersOn ? ((int)log10((double)(m_view->doc()->numLines())) + 1) * m_maxCharWidth + 4 : 0;

  if (m_view->dynWordWrap() && m_dynWrapIndicatorsOn) {
    width = kMax(style().scrollBarExtent().width() + 4, width);

    if (m_cachedLNWidth != width || m_oldBackgroundColor != m_view->renderer()->config()->iconBarColor()) {
      int w = style().scrollBarExtent().width();
      int h = m_view->renderer()->config()->fontMetrics()->height();

      TQSize newSize(w, h);
      if ((m_arrow.size() != newSize || m_oldBackgroundColor != m_view->renderer()->config()->iconBarColor()) && !newSize.isEmpty()) {
        m_arrow.resize(newSize);

        TQPainter p(&m_arrow);
        p.fillRect( 0, 0, w, h, m_view->renderer()->config()->iconBarColor() );

        h = m_view->renderer()->config()->fontMetrics()->ascent();

        p.setPen(m_view->renderer()->attribute(0)->textColor());
        p.drawLine(w/2, h/2, w/2, 0);
#if 1
        p.lineTo(w/4, h/4);
        p.lineTo(0, 0);
        p.lineTo(0, h/2);
        p.lineTo(w/2, h-1);
        p.lineTo(w*3/4, h-1);
        p.lineTo(w-1, h*3/4);
        p.lineTo(w*3/4, h/2);
        p.lineTo(0, h/2);
#else
        p.lineTo(w*3/4, h/4);
        p.lineTo(w-1,0);
        p.lineTo(w-1, h/2);
        p.lineTo(w/2, h-1);
        p.lineTo(w/4,h-1);
        p.lineTo(0, h*3/4);
        p.lineTo(w/4, h/2);
        p.lineTo(w-1, h/2);
#endif
      }
    }
  }

  return width;
}

void KateIconBorder::paintEvent(TQPaintEvent* e)
{
  paintBorder(e->rect().x(), e->rect().y(), e->rect().width(), e->rect().height());
}

void KateIconBorder::paintBorder (int /*x*/, int y, int /*width*/, int height)
{
  static TQPixmap minus_px ((const char**)minus_xpm);
  static TQPixmap plus_px ((const char**)plus_xpm);

  uint h = m_view->renderer()->config()->fontStruct()->fontHeight;
  uint startz = (y / h);
  uint endz = startz + 1 + (height / h);
  uint lineRangesSize = m_viewInternal->lineRanges.size();

  // center the folding boxes
  int m_px = (h - 11) / 2;
  if (m_px < 0)
    m_px = 0;

  int lnWidth( 0 );
  if ( m_lineNumbersOn || (m_view->dynWordWrap() && m_dynWrapIndicatorsOn) ) // avoid calculating unless needed ;-)
  {
    lnWidth = lineNumberWidth();
    if ( lnWidth != m_cachedLNWidth || m_oldBackgroundColor != m_view->renderer()->config()->iconBarColor() )
    {
      // we went from n0 ->n9 lines or vice verca
      // this causes an extra updateGeometry() first time the line numbers
      // are displayed, but sizeHint() is supposed to be const so we can't set
      // the cached value there.
      m_cachedLNWidth = lnWidth;
      m_oldBackgroundColor = m_view->renderer()->config()->iconBarColor();
      updateGeometry();
      update ();
      return;
    }
  }

  int w( this->width() );                     // sane value/calc only once

  TQPainter p ( this );
  p.setFont ( *m_view->renderer()->config()->font() ); // for line numbers
  // the line number color is for the line numbers, vertical separator lines
  // and for for the code folding lines.
  p.setPen ( m_view->renderer()->config()->lineNumberColor() );

  KateLineInfo oldInfo;
  if (startz < lineRangesSize)
  {
    if ((m_viewInternal->lineRanges[startz].line-1) < 0)
      oldInfo.topLevel = true;
    else
       m_doc->lineInfo(&oldInfo,m_viewInternal->lineRanges[startz].line-1);
  }

  for (uint z=startz; z <= endz; z++)
  {
    int y = h * z;
    int realLine = -1;

    if (z < lineRangesSize)
     realLine = m_viewInternal->lineRanges[z].line;

    int lnX ( 0 );

    p.fillRect( 0, y, w-4, h, m_view->renderer()->config()->iconBarColor() );
    p.fillRect( w-4, y, 4, h, m_view->renderer()->config()->backgroundColor() );

    // icon pane
    if( m_iconBorderOn )
    {
      p.drawLine(lnX+iconPaneWidth, y, lnX+iconPaneWidth, y+h);

      if( (realLine > -1) && (m_viewInternal->lineRanges[z].startCol == 0) )
      {
        uint mrk ( m_doc->mark( realLine ) ); // call only once

        if ( mrk )
        {
          for( uint bit = 0; bit < 32; bit++ )
          {
            MarkInterface::MarkTypes markType = (MarkInterface::MarkTypes)(1<<bit);
            if( mrk & markType )
            {
              TQPixmap *px_mark (m_doc->markPixmap( markType ));

              if (px_mark)
              {
                // center the mark pixmap
                int x_px = (iconPaneWidth - px_mark->width()) / 2;
                if (x_px < 0)
                  x_px = 0;

                int y_px = (h - px_mark->height()) / 2;
                if (y_px < 0)
                  y_px = 0;

                p.drawPixmap( lnX+x_px, y+y_px, *px_mark);
              }
            }
          }
        }
      }

      lnX += iconPaneWidth + 1;
    }

    // line number
    if( m_lineNumbersOn || (m_view->dynWordWrap() && m_dynWrapIndicatorsOn) )
    {
      lnX +=2;

      if (realLine > -1)
        if (m_viewInternal->lineRanges[z].startCol == 0) {
          if (m_lineNumbersOn)
            p.drawText( lnX + 1, y, lnWidth-4, h, TQt::AlignRight|TQt::AlignVCenter, TQString("%1").arg( realLine + 1 ) );
        } else if (m_view->dynWordWrap() && m_dynWrapIndicatorsOn) {
          p.drawPixmap(lnX + lnWidth - m_arrow.width() - 4, y, m_arrow);
        }

      lnX += lnWidth;
    }

    // folding markers
    if( m_foldingMarkersOn )
    {
      if( realLine > -1 )
      {
        KateLineInfo info;
        m_doc->lineInfo(&info,realLine);

        if (!info.topLevel)
        {
          if (info.startsVisibleBlock && (m_viewInternal->lineRanges[z].startCol == 0))
          {
            if (oldInfo.topLevel)
              p.drawLine(lnX+halfIPW,y+m_px,lnX+halfIPW,y+h-1);
            else
              p.drawLine(lnX+halfIPW,y,lnX+halfIPW,y+h-1);

            p.drawPixmap(lnX+3,y+m_px,minus_px);
          }
          else if (info.startsInVisibleBlock)
          {
            if (m_viewInternal->lineRanges[z].startCol == 0)
            {
              if (oldInfo.topLevel)
                p.drawLine(lnX+halfIPW,y+m_px,lnX+halfIPW,y+h-1);
              else
                p.drawLine(lnX+halfIPW,y,lnX+halfIPW,y+h-1);

              p.drawPixmap(lnX+3,y+m_px,plus_px);
            }
            else
            {
              p.drawLine(lnX+halfIPW,y,lnX+halfIPW,y+h-1);
            }

            if (!m_viewInternal->lineRanges[z].wrap)
              p.drawLine(lnX+halfIPW,y+h-1,lnX+iconPaneWidth-2,y+h-1);
          }
          else
          {
            p.drawLine(lnX+halfIPW,y,lnX+halfIPW,y+h-1);

            if (info.endsBlock && !m_viewInternal->lineRanges[z].wrap)
              p.drawLine(lnX+halfIPW,y+h-1,lnX+iconPaneWidth-2,y+h-1);
          }
        }

        oldInfo = info;
      }

      lnX += iconPaneWidth;
    }
  }
}

KateIconBorder::BorderArea KateIconBorder::positionToArea( const TQPoint& p ) const
{
  int x = 0;
  if( m_iconBorderOn ) {
    x += iconPaneWidth;
    if( p.x() <= x )
      return IconBorder;
  }
  if( m_lineNumbersOn || m_dynWrapIndicators ) {
    x += lineNumberWidth();
    if( p.x() <= x )
      return LineNumbers;
  }
  if( m_foldingMarkersOn ) {
    x += iconPaneWidth;
    if( p.x() <= x )
      return FoldingMarkers;
  }
  return None;
}

void KateIconBorder::mousePressEvent( TQMouseEvent* e )
{
  m_lastClickedLine = m_viewInternal->yToKateLineRange(e->y()).line;

  if ( positionToArea( e->pos() ) != IconBorder )
  {
    TQMouseEvent forward( TQEvent::MouseButtonPress,
      TQPoint( 0, e->y() ), e->button(), e->state() );
    m_viewInternal->mousePressEvent( &forward );
  }
  e->accept();
}

void KateIconBorder::mouseMoveEvent( TQMouseEvent* e )
{
  if ( positionToArea( e->pos() ) != IconBorder )
  {
    TQMouseEvent forward( TQEvent::MouseMove,
      TQPoint( 0, e->y() ), e->button(), e->state() );
    m_viewInternal->mouseMoveEvent( &forward );
  }
}

void KateIconBorder::mouseReleaseEvent( TQMouseEvent* e )
{
  uint cursorOnLine = m_viewInternal->yToKateLineRange(e->y()).line;

  if (cursorOnLine == m_lastClickedLine &&
      cursorOnLine <= m_doc->lastLine() )
  {
    BorderArea area = positionToArea( e->pos() );
    if( area == IconBorder) {
      if (e->button() == TQt::LeftButton) {
        if( m_doc->editableMarks() & KateViewConfig::global()->defaultMarkType() ) {
          if( m_doc->mark( cursorOnLine ) & KateViewConfig::global()->defaultMarkType() )
            m_doc->removeMark( cursorOnLine, KateViewConfig::global()->defaultMarkType() );
          else
            m_doc->addMark( cursorOnLine, KateViewConfig::global()->defaultMarkType() );
          } else {
            showMarkMenu( cursorOnLine, TQCursor::pos() );
          }
        }
        else
        if (e->button() == TQt::RightButton) {
          showMarkMenu( cursorOnLine, TQCursor::pos() );
        }
    }

    if ( area == FoldingMarkers) {
      KateLineInfo info;
      m_doc->lineInfo(&info,cursorOnLine);
      if ((info.startsVisibleBlock) || (info.startsInVisibleBlock)) {
        emit toggleRegionVisibility(cursorOnLine);
      }
    }
  }

  TQMouseEvent forward( TQEvent::MouseButtonRelease,
    TQPoint( 0, e->y() ), e->button(), e->state() );
  m_viewInternal->mouseReleaseEvent( &forward );
}

void KateIconBorder::mouseDoubleClickEvent( TQMouseEvent* e )
{
  TQMouseEvent forward( TQEvent::MouseButtonDblClick,
    TQPoint( 0, e->y() ), e->button(), e->state() );
  m_viewInternal->mouseDoubleClickEvent( &forward );
}

void KateIconBorder::showMarkMenu( uint line, const TQPoint& pos )
{
  TQPopupMenu markMenu;
  TQPopupMenu selectDefaultMark;

  typedef TQValueVector<int> MarkTypeVector;
  MarkTypeVector vec( 33 );
  int i=1;

  for( uint bit = 0; bit < 32; bit++ ) {
    MarkInterface::MarkTypes markType = (MarkInterface::MarkTypes)(1<<bit);
    if( !(m_doc->editableMarks() & markType) )
      continue;

    if( !m_doc->markDescription( markType ).isEmpty() ) {
      markMenu.insertItem( m_doc->markDescription( markType ), i );
      selectDefaultMark.insertItem( m_doc->markDescription( markType ), i+100);
    } else {
      markMenu.insertItem( i18n("Mark Type %1").arg( bit + 1 ), i );
      selectDefaultMark.insertItem( i18n("Mark Type %1").arg( bit + 1 ), i+100);
    }

    if( m_doc->mark( line ) & markType )
      markMenu.setItemChecked( i, true );

    if( markType & KateViewConfig::global()->defaultMarkType() )
      selectDefaultMark.setItemChecked( i+100, true );

    vec[i++] = markType;
  }

  if( markMenu.count() == 0 )
    return;

  if( markMenu.count() > 1 )
    markMenu.insertItem( i18n("Set Default Mark Type" ), &selectDefaultMark);

  int result = markMenu.exec( pos );
  if( result <= 0 )
    return;

  if ( result > 100)
  {
     KateViewConfig::global()->setDefaultMarkType (vec[result-100]);
     // flush config, otherwise it isn't nessecarily done
     TDEConfig *config = kapp->config();
     config->setGroup("Kate View Defaults");
     KateViewConfig::global()->writeConfig( config );
  }
  else
  {
    MarkInterface::MarkTypes markType = (MarkInterface::MarkTypes) vec[result];
    if( m_doc->mark( line ) & markType ) {
      m_doc->removeMark( line, markType );
    } else {
        m_doc->addMark( line, markType );
    }
  }
}
//END KateIconBorder

KateViewEncodingAction::KateViewEncodingAction(KateDocument *_doc, KateView *_view, const TQString& text, TQObject* parent, const char* name)
       : TDEActionMenu (text, parent, name), doc(_doc), view (_view)
{
  connect(popupMenu(),TQ_SIGNAL(aboutToShow()),this,TQ_SLOT(slotAboutToShow()));
}

void KateViewEncodingAction::slotAboutToShow()
{
  TQStringList modes (TDEGlobal::charsets()->descriptiveEncodingNames());

  popupMenu()->clear ();
  for (uint z=0; z<modes.size(); ++z)
  {
    popupMenu()->insertItem ( modes[z], this, TQ_SLOT(setMode(int)), 0,  z);

    bool found = false;
    TQTextCodec *codecForEnc = TDEGlobal::charsets()->codecForName(TDEGlobal::charsets()->encodingForName(modes[z]), found);

    if (found && codecForEnc)
    {
      if (codecForEnc->name() == doc->config()->codec()->name())
        popupMenu()->setItemChecked (z, true);
    }
  }
}

void KateViewEncodingAction::setMode (int mode)
{
  TQStringList modes (TDEGlobal::charsets()->descriptiveEncodingNames());
  doc->config()->setEncoding( TDEGlobal::charsets()->encodingForName( modes[mode] ) );
  // now we don't want the encoding changed again unless the user does so using the menu.
  doc->setEncodingSticky( true );
  doc->reloadFile();
}
