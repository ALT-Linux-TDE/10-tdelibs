/* This file is part of the KDE libraries
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2002,2003 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002,2003 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2003 Anakim Border <aborder@sources.sourceforge.net>

   Based on:
     KWriteView : Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

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

#include "kateviewinternal.h"
#include "kateviewinternal.moc"

#include "kateview.h"
#include "katecodefoldinghelpers.h"
#include "kateviewhelpers.h"
#include "katehighlight.h"
#include "katesupercursor.h"
#include "katerenderer.h"
#include "katecodecompletion.h"
#include "kateconfig.h"

#include <kcursor.h>
#include <kdebug.h>
#include <tdeapplication.h>
#include <tdeglobalsettings.h>
#include <kurldrag.h>

#include <tqstyle.h>
#include <tqdragobject.h>
#include <tqpopupmenu.h>
#include <tqdropsite.h>
#include <tqpainter.h>
#include <tqlayout.h>
#include <tqclipboard.h>
#include <tqpixmap.h>
#include <tqvbox.h>

KateViewInternal::KateViewInternal(KateView *view, KateDocument *doc)
  : TQWidget (view, "", (WFlags)(WStaticContents | WRepaintNoErase | WResizeNoErase) )
  , editSessionNumber (0)
  , editIsRunning (false)
  , m_view (view)
  , m_doc (doc)
  , cursor (doc, true, 0, 0, this)
  , possibleTripleClick (false)
  , m_dummy (0)
  , m_startPos(doc, true, 0,0)
  , m_madeVisible(false)
  , m_shiftKeyPressed (false)
  , m_autoCenterLines (false)
  , m_selChangedByUser (false)
  , selectAnchor (-1, -1)
  , m_selectionMode( Default )
  , m_preserveMaxX(false)
  , m_currentMaxX(0)
  , m_usePlainLines(false)
  , m_updatingView(true)
  , m_cachedMaxStartPos(-1, -1)
  , m_dragScrollTimer(this)
  , m_scrollTimer (this)
  , m_cursorTimer (this)
  , m_textHintTimer (this)
  , m_textHintEnabled(false)
  , m_textHintMouseX(-1)
  , m_textHintMouseY(-1)
  , m_imPreeditStartLine(0)
  , m_imPreeditStart(0)
  , m_imPreeditLength(0)
  , m_imPreeditSelStart(0)
{
  setMinimumSize (0,0);

  // cursor
  cursor.setMoveOnInsert (true);

  // invalidate selStartCached, or keyb selection is screwed initially
  selStartCached.setLine( -1 );
  //
  // scrollbar for lines
  //
  m_lineScroll = new KateScrollBar(TQt::Vertical, this);
  m_lineScroll->show();
  m_lineScroll->setTracking (true);

  m_lineLayout = new TQVBoxLayout();
  m_colLayout = new TQHBoxLayout();

  m_colLayout->addWidget(m_lineScroll);
  m_lineLayout->addLayout(m_colLayout);

  // bottom corner box
  m_dummy = new TQWidget(m_view);
  m_dummy->setFixedHeight(style().scrollBarExtent().width());

  if (m_view->dynWordWrap())
    m_dummy->hide();
  else
    m_dummy->show();

  m_lineLayout->addWidget(m_dummy);

  // Hijack the line scroller's controls, so we can scroll nicely for word-wrap
  connect(m_lineScroll, TQ_SIGNAL(prevPage()), TQ_SLOT(scrollPrevPage()));
  connect(m_lineScroll, TQ_SIGNAL(nextPage()), TQ_SLOT(scrollNextPage()));

  connect(m_lineScroll, TQ_SIGNAL(prevLine()), TQ_SLOT(scrollPrevLine()));
  connect(m_lineScroll, TQ_SIGNAL(nextLine()), TQ_SLOT(scrollNextLine()));

  connect(m_lineScroll, TQ_SIGNAL(sliderMoved(int)), TQ_SLOT(scrollLines(int)));
  connect(m_lineScroll, TQ_SIGNAL(sliderMMBMoved(int)), TQ_SLOT(scrollLines(int)));

  // catch wheel events, completing the hijack
  m_lineScroll->installEventFilter(this);

  //
  // scrollbar for columns
  //
  m_columnScroll = new TQScrollBar(TQt::Horizontal,m_view);

  // hide the column scrollbar in the dynamic word wrap mode
  if (m_view->dynWordWrap())
    m_columnScroll->hide();
  else
    m_columnScroll->show();

  m_columnScroll->setTracking(true);
  m_startX = 0;

  connect( m_columnScroll, TQ_SIGNAL( valueChanged (int) ),
           this, TQ_SLOT( scrollColumns (int) ) );

  //
  // iconborder ;)
  //
  leftBorder = new KateIconBorder( this, m_view );
  leftBorder->show ();

  connect( leftBorder, TQ_SIGNAL(toggleRegionVisibility(unsigned int)),
           m_doc->foldingTree(), TQ_SLOT(toggleRegionVisibility(unsigned int)));

  connect( doc->foldingTree(), TQ_SIGNAL(regionVisibilityChangedAt(unsigned int)),
           this, TQ_SLOT(slotRegionVisibilityChangedAt(unsigned int)));
  connect( doc, TQ_SIGNAL(codeFoldingUpdated()),
           this, TQ_SLOT(slotCodeFoldingChanged()) );

  displayCursor.setPos(0, 0);
  cursor.setPos(0, 0);
  cXPos = 0;

  setAcceptDrops( true );
  setBackgroundMode( NoBackground );

  // event filter
  installEventFilter(this);

  // im
  setInputMethodEnabled(true);

  // set initial cursor
  setCursor( KCursor::ibeamCursor() );
  m_mouseCursor = TQt::IbeamCursor;

  // call mouseMoveEvent also if no mouse button is pressed
  setMouseTracking(true);

  dragInfo.state = diNone;

  // timers
  connect( &m_dragScrollTimer, TQ_SIGNAL( timeout() ),
             this, TQ_SLOT( doDragScroll() ) );

  connect( &m_scrollTimer, TQ_SIGNAL( timeout() ),
             this, TQ_SLOT( scrollTimeout() ) );

  connect( &m_cursorTimer, TQ_SIGNAL( timeout() ),
             this, TQ_SLOT( cursorTimeout() ) );

  connect( &m_textHintTimer, TQ_SIGNAL( timeout() ),
             this, TQ_SLOT( textHintTimeout() ) );

  // selection changed to set anchor
  connect( m_view, TQ_SIGNAL( selectionChanged() ),
             this, TQ_SLOT( viewSelectionChanged() ) );


// this is a work arround for RTL desktops
// should be changed in kde 3.3
// BTW: this comment has been "ported" from 3.1.X tree
//      any hacker with BIDI knowlege is welcomed to fix kate problems :)
  if (TQApplication::reverseLayout()){
      m_view->m_grid->addMultiCellWidget(leftBorder,     0, 1, 2, 2);
      m_view->m_grid->addMultiCellWidget(m_columnScroll, 1, 1, 0, 1);
      m_view->m_grid->addMultiCellLayout(m_lineLayout, 0, 0, 0, 0);
  }
  else{
      m_view->m_grid->addMultiCellLayout(m_lineLayout, 0, 1, 2, 2);
      m_view->m_grid->addMultiCellWidget(m_columnScroll, 1, 1, 0, 1);
      m_view->m_grid->addWidget(leftBorder, 0, 0);
  }

  updateView ();
}

KateViewInternal::~KateViewInternal ()
{
}

void KateViewInternal::prepareForDynWrapChange()
{
  // Which is the current view line?
  m_wrapChangeViewLine = displayViewLine(displayCursor, true);
}

void KateViewInternal::dynWrapChanged()
{
  if (m_view->dynWordWrap())
  {
    m_columnScroll->hide();
    m_dummy->hide ();
  }
  else
  {
    m_columnScroll->show();
    m_dummy->show ();
  }

  tagAll();
  updateView();

  if (m_view->dynWordWrap())
    scrollColumns(0);

  // Determine where the cursor should be to get the cursor on the same view line
  if (m_wrapChangeViewLine != -1) {
    KateTextCursor newStart = viewLineOffset(displayCursor, -m_wrapChangeViewLine);
    makeVisible(newStart, newStart.col(), true);
  } else {
    update();
  }
}

KateTextCursor KateViewInternal::endPos() const
{
  int viewLines = linesDisplayed() - 1;

  if (viewLines < 0) {
    kdDebug(13030) << "WARNING: viewLines wrong!" << endl;
    viewLines = 0;
  }

  // Check to make sure that lineRanges isn't invalid
  if (!lineRanges.count() || lineRanges[0].line == -1 || viewLines >= (int)lineRanges.count()) {
    // Switch off use of the cache
    return KateTextCursor(m_doc->numVisLines() - 1, m_doc->lineLength(m_doc->getRealLine(m_doc->numVisLines() - 1)));
  }

  for (int i = viewLines; i >= 0; i--) {
    KateLineRange& thisRange = lineRanges[i];

    if (thisRange.line == -1) continue;

    if (thisRange.virtualLine >= (int)m_doc->numVisLines()) {
      // Cache is too out of date
      return KateTextCursor(m_doc->numVisLines() - 1, m_doc->lineLength(m_doc->getRealLine(m_doc->numVisLines() - 1)));
    }

    return KateTextCursor(thisRange.virtualLine, thisRange.wrap ? thisRange.endCol - 1 : thisRange.endCol);
  }

  Q_ASSERT(false);
  kdDebug(13030) << "WARNING: could not find a lineRange at all" << endl;
  return KateTextCursor(-1, -1);
}

uint KateViewInternal::endLine() const
{
  return endPos().line();
}

KateLineRange KateViewInternal::yToKateLineRange(uint y) const
{
  uint range = y / m_view->renderer()->fontHeight();

  // lineRanges is always bigger than 0, after the initial updateView call
  if (range >= lineRanges.size())
    return lineRanges[lineRanges.size()-1];

  return lineRanges[range];
}

int KateViewInternal::lineToY(uint viewLine) const
{
  return (viewLine-startLine()) * m_view->renderer()->fontHeight();
}

void KateViewInternal::slotIncFontSizes()
{
  m_view->renderer()->increaseFontSizes();
}

void KateViewInternal::slotDecFontSizes()
{
  m_view->renderer()->decreaseFontSizes();
}

/**
 * Line is the real line number to scroll to.
 */
void KateViewInternal::scrollLines ( int line )
{
  KateTextCursor newPos(line, 0);
  scrollPos(newPos);
}

// This can scroll less than one true line
void KateViewInternal::scrollViewLines(int offset)
{
  KateTextCursor c = viewLineOffset(startPos(), offset);
  scrollPos(c);

  m_lineScroll->blockSignals(true);
  m_lineScroll->setValue(startLine());
  m_lineScroll->blockSignals(false);
}

void KateViewInternal::scrollNextPage()
{
  scrollViewLines(kMax( (int)linesDisplayed() - 1, 0 ));
}

void KateViewInternal::scrollPrevPage()
{
  scrollViewLines(-kMax( (int)linesDisplayed() - 1, 0 ));
}

void KateViewInternal::scrollPrevLine()
{
  scrollViewLines(-1);
}

void KateViewInternal::scrollNextLine()
{
  scrollViewLines(1);
}

KateTextCursor KateViewInternal::maxStartPos(bool changed)
{
  m_usePlainLines = true;

  if (m_cachedMaxStartPos.line() == -1 || changed)
  {
    KateTextCursor end(m_doc->numVisLines() - 1, m_doc->lineLength(m_doc->getRealLine(m_doc->numVisLines() - 1)));

    m_cachedMaxStartPos = viewLineOffset(end, -((int)linesDisplayed() - 1));
  }

  m_usePlainLines = false;

  return m_cachedMaxStartPos;
}

// c is a virtual cursor
void KateViewInternal::scrollPos(KateTextCursor& c, bool force, bool calledExternally)
{
  if (!force && ((!m_view->dynWordWrap() && c.line() == (int)startLine()) || c == startPos()))
    return;

  if (c.line() < 0)
    c.setLine(0);

  KateTextCursor limit = maxStartPos();
  if (c > limit) {
    c = limit;

    // Re-check we're not just scrolling to the same place
    if (!force && ((!m_view->dynWordWrap() && c.line() == (int)startLine()) || c == startPos()))
      return;
  }

  int viewLinesScrolled = 0;

  // only calculate if this is really used and usefull, could be wrong here, please recheck
  // for larger scrolls this makes 2-4 seconds difference on my xeon with dyn. word wrap on
  // try to get it really working ;)
  bool viewLinesScrolledUsable = !force
                                 && (c.line() >= (int)startLine()-(int)linesDisplayed()-1)
                                 && (c.line() <= (int)endLine()+(int)linesDisplayed()+1);

  if (viewLinesScrolledUsable)
    viewLinesScrolled = displayViewLine(c);

  m_startPos.setPos(c);

  // set false here but reversed if we return to makeVisible
  m_madeVisible = false;

  if (viewLinesScrolledUsable)
  {
    int lines = linesDisplayed();
    if ((int)m_doc->numVisLines() < lines) {
      KateTextCursor end(m_doc->numVisLines() - 1, m_doc->lineLength(m_doc->getRealLine(m_doc->numVisLines() - 1)));
      lines = kMin((int)linesDisplayed(), displayViewLine(end) + 1);
    }

    Q_ASSERT(lines >= 0);

    if (!calledExternally && TQABS(viewLinesScrolled) < lines)
    {
      updateView(false, viewLinesScrolled);

      int scrollHeight = -(viewLinesScrolled * (int)m_view->renderer()->fontHeight());
      int scrollbarWidth = style().scrollBarExtent().width();

      //
      // updates are for working around the scrollbar leaving blocks in the view
      //
      scroll(0, scrollHeight);
      update(0, height()+scrollHeight-scrollbarWidth, width(), 2*scrollbarWidth);

      leftBorder->scroll(0, scrollHeight);
      leftBorder->update(0, leftBorder->height()+scrollHeight-scrollbarWidth, leftBorder->width(), 2*scrollbarWidth);

      return;
    }
  }

  updateView();
  update();
  leftBorder->update();
}

void KateViewInternal::scrollColumns ( int x )
{
  if (x == m_startX)
    return;

  if (x < 0)
    x = 0;

  int dx = m_startX - x;
  m_startX = x;

  if (TQABS(dx) < width())
    scroll(dx, 0);
  else
    update();

  m_columnScroll->blockSignals(true);
  m_columnScroll->setValue(m_startX);
  m_columnScroll->blockSignals(false);
}

// If changed is true, the lines that have been set dirty have been updated.
void KateViewInternal::updateView(bool changed, int viewLinesScrolled)
{
  m_updatingView = true;

  uint contentLines = m_doc->visibleLines();

  m_lineScroll->blockSignals(true);

  KateTextCursor maxStart = maxStartPos(changed);
  int maxLineScrollRange = maxStart.line();
  if (m_view->dynWordWrap() && maxStart.col() != 0)
    maxLineScrollRange++;
  m_lineScroll->setRange(0, maxLineScrollRange);

  m_lineScroll->setValue(startPos().line());
  m_lineScroll->setSteps(1, height() / m_view->renderer()->fontHeight());
  m_lineScroll->blockSignals(false);

  uint oldSize = lineRanges.size ();
  uint newSize = (height() / m_view->renderer()->fontHeight()) + 1;
  if (oldSize != newSize) {
    lineRanges.resize((height() / m_view->renderer()->fontHeight()) + 1);
    if (newSize > oldSize) {
      static KateLineRange blank;
      for (uint i = oldSize; i < newSize; i++) {
        lineRanges[i] = blank;
      }
    }
  }

  if (oldSize < lineRanges.size ())
  {
    for (uint i=oldSize; i < lineRanges.size(); i++)
      lineRanges[i].dirty = true;
  }

  // Move the lineRanges data if we've just scrolled...
  if (viewLinesScrolled != 0) {
    // loop backwards if we've just scrolled up...
    bool forwards = viewLinesScrolled >= 0 ? true : false;
    for (uint z = forwards ? 0 : lineRanges.count() - 1; z < lineRanges.count(); forwards ? z++ : z--) {
      uint oldZ = z + viewLinesScrolled;
      if (oldZ < lineRanges.count()) {
        lineRanges[z] = lineRanges[oldZ];
      } else {
        lineRanges[z].dirty = true;
      }
    }
  }

  if (m_view->dynWordWrap())
  {
    KateTextCursor realStart = startPos();
    realStart.setLine(m_doc->getRealLine(realStart.line()));

    KateLineRange startRange = range(realStart);
    uint line = startRange.virtualLine;
    int realLine = startRange.line;
    uint oldLine = line;
    int startCol = startRange.startCol;
    int startX = startRange.startX;
    int endX = startRange.startX;
    int shiftX = startRange.startCol ? startRange.shiftX : 0;
    bool wrap = false;
    int newViewLine = startRange.viewLine;
    // z is the current display view line
    KateTextLine::Ptr text = textLine(realLine);

    bool alreadyDirty = false;

    for (uint z = 0; z < lineRanges.size(); z++)
    {
      if (oldLine != line) {
        realLine = (int)m_doc->getRealLine(line);

        if (z)
          lineRanges[z-1].startsInvisibleBlock = (realLine != lineRanges[z-1].line + 1);

        text = textLine(realLine);
        startCol = 0;
        startX = 0;
        endX = 0;
        shiftX = 0;
        newViewLine = 0;
        oldLine = line;
      }

      if (line >= contentLines || !text)
      {
        if (lineRanges[z].line != -1)
          lineRanges[z].dirty = true;

        lineRanges[z].clear();

        line++;
      }
      else
      {
        if (lineRanges[z].line != realLine || lineRanges[z].startCol != startCol)
          alreadyDirty = lineRanges[z].dirty = true;

        if (lineRanges[z].dirty || changed || alreadyDirty) {
          alreadyDirty = true;

          lineRanges[z].virtualLine = line;
          lineRanges[z].line = realLine;
          lineRanges[z].startsInvisibleBlock = false;

          int tempEndX = 0;

          int endCol = m_view->renderer()->textWidth(text, startCol, width() - shiftX, &wrap, &tempEndX);

          endX += tempEndX;

          if (wrap)
          {
            if (m_view->config()->dynWordWrapAlignIndent() > 0)
            {
              if (startX == 0)
              {
                int pos = text->nextNonSpaceChar(0);

                if (pos > 0)
                  shiftX = m_view->renderer()->textWidth(text, pos);

                if (shiftX > ((double)width() / 100 * m_view->config()->dynWordWrapAlignIndent()))
                  shiftX = 0;
              }
            }

            if ((lineRanges[z].startX != startX) || (lineRanges[z].endX != endX) ||
                (lineRanges[z].startCol != startCol) || (lineRanges[z].endCol != endCol) ||
                (lineRanges[z].shiftX != shiftX))
              lineRanges[z].dirty = true;

            lineRanges[z].startCol = startCol;
            lineRanges[z].endCol = endCol;
            lineRanges[z].startX = startX;
            lineRanges[z].endX = endX;
            lineRanges[z].viewLine = newViewLine;
            lineRanges[z].wrap = true;

            startCol = endCol;
            startX = endX;
          }
          else
          {
            if ((lineRanges[z].startX != startX) || (lineRanges[z].endX != endX) ||
                (lineRanges[z].startCol != startCol) || (lineRanges[z].endCol != endCol))
              lineRanges[z].dirty = true;

            lineRanges[z].startCol = startCol;
            lineRanges[z].endCol = endCol;
            lineRanges[z].startX = startX;
            lineRanges[z].endX = endX;
            lineRanges[z].viewLine = newViewLine;
            lineRanges[z].wrap = false;

            line++;
          }

          lineRanges[z].shiftX = shiftX;

        } else {
          // The cached data is still intact
          if (lineRanges[z].wrap) {
            startCol = lineRanges[z].endCol;
            startX = lineRanges[z].endX;
            endX = lineRanges[z].endX;
          } else {
            line++;
          }
          shiftX = lineRanges[z].shiftX;
        }
      }
      newViewLine++;
    }
  }
  else
  {
    uint z = 0;

    for(; (z + startLine() < contentLines) && (z < lineRanges.size()); z++)
    {
      if (lineRanges[z].dirty || lineRanges[z].line != (int)m_doc->getRealLine(z + startLine())) {
        lineRanges[z].dirty = true;

        lineRanges[z].line = m_doc->getRealLine( z + startLine() );
        if (z)
          lineRanges[z-1].startsInvisibleBlock = (lineRanges[z].line != lineRanges[z-1].line + 1);

        lineRanges[z].virtualLine = z + startLine();
        lineRanges[z].startCol = 0;
        lineRanges[z].endCol = m_doc->lineLength(lineRanges[z].line);
        lineRanges[z].startX = 0;
        lineRanges[z].endX = m_view->renderer()->textWidth( textLine( lineRanges[z].line ), -1 );
        lineRanges[z].shiftX = 0;
        lineRanges[z].viewLine = 0;
        lineRanges[z].wrap = false;
      }
      else if (z && lineRanges[z-1].dirty)
      {
        lineRanges[z-1].startsInvisibleBlock = (lineRanges[z].line != lineRanges[z-1].line + 1);
      }
    }

    for (; z < lineRanges.size(); z++)
    {
      if (lineRanges[z].line != -1)
        lineRanges[z].dirty = true;

      lineRanges[z].clear();
    }

    int max = maxLen(startLine()) - width();
    if (max < 0)
      max = 0;

    // if we lose the ability to scroll horizontally, move view to the far-left
    if (max == 0)
    {
      scrollColumns(0);
    }

    m_columnScroll->blockSignals(true);

    // disable scrollbar
    m_columnScroll->setDisabled (max == 0);

    m_columnScroll->setRange(0, max);

    m_columnScroll->setValue(m_startX);

    // Approximate linescroll
    m_columnScroll->setSteps(m_view->renderer()->config()->fontMetrics()->width('a'), width());

    m_columnScroll->blockSignals(false);
  }

  m_updatingView = false;

  if (changed)
    paintText(0, 0, width(), height(), true);
}

void KateViewInternal::paintText (int x, int y, int width, int height, bool paintOnlyDirty)
{
  //kdDebug() << k_funcinfo << x << " " << y << " " << width << " " << height << " " << paintOnlyDirty << endl;
  int xStart = startX() + x;
  int xEnd = xStart + width;
  uint h = m_view->renderer()->fontHeight();
  uint startz = (y / h);
  uint endz = startz + 1 + (height / h);
  uint lineRangesSize = lineRanges.size();

  static TQPixmap drawBuffer;

  if (drawBuffer.width() < KateViewInternal::width() || drawBuffer.height() < (int)h)
    drawBuffer.resize(KateViewInternal::width(), (int)h);

  if (drawBuffer.isNull())
    return;

  TQPainter paint(this);
  TQPainter paintDrawBuffer(&drawBuffer);

  // TODO put in the proper places
  m_view->renderer()->setCaretStyle(m_view->isOverwriteMode() ? KateRenderer::Replace : KateRenderer::Insert);
  m_view->renderer()->setShowTabs(m_doc->configFlags() & KateDocument::cfShowTabs);

  for (uint z=startz; z <= endz; z++)
  {
    if ( (z >= lineRangesSize) || ((lineRanges[z].line == -1) && (!paintOnlyDirty || lineRanges[z].dirty)) )
    {
      if (!(z >= lineRangesSize))
        lineRanges[z].dirty = false;

      paint.fillRect( x, z * h, width, h, m_view->renderer()->config()->backgroundColor() );
    }
    else if (!paintOnlyDirty || lineRanges[z].dirty)
    {
      lineRanges[z].dirty = false;

      m_view->renderer()->paintTextLine(paintDrawBuffer, &lineRanges[z], xStart, xEnd, &cursor, &bm);

      paint.drawPixmap (x, z * h, drawBuffer, 0, 0, width, h);
    }
  }
}

/**
 * this function ensures a certain location is visible on the screen.
 * if endCol is -1, ignore making the columns visible.
 */
void KateViewInternal::makeVisible (const KateTextCursor& c, uint endCol, bool force, bool center, bool calledExternally)
{
  //kdDebug() << "MakeVisible start [" << startPos().line << "," << startPos().col << "] end [" << endPos().line << "," << endPos().col << "] -> request: [" << c.line << "," << c.col << "]" <<endl;// , new start [" << scroll.line << "," << scroll.col << "] lines " << (linesDisplayed() - 1) << " height " << height() << endl;
    // if the line is in a folded region, unfold all the way up
    //if ( m_doc->foldingTree()->findNodeForLine( c.line )->visible )
    //  kdDebug()<<"line ("<<c.line<<") should be visible"<<endl;

  if ( force )
  {
    KateTextCursor scroll = c;
    scrollPos(scroll, force, calledExternally);
  }
  else if (center && (c < startPos() || c > endPos()))
  {
    KateTextCursor scroll = viewLineOffset(c, -int(linesDisplayed()) / 2);
    scrollPos(scroll, false, calledExternally);
  }
  else if ( c > viewLineOffset(endPos(), -m_minLinesVisible) )
  {
    KateTextCursor scroll = viewLineOffset(c, -((int)linesDisplayed() - m_minLinesVisible - 1));
    scrollPos(scroll, false, calledExternally);
  }
  else if ( c < viewLineOffset(startPos(), m_minLinesVisible) )
  {
    KateTextCursor scroll = viewLineOffset(c, -m_minLinesVisible);
    scrollPos(scroll, false, calledExternally);
  }
  else
  {
    // Check to see that we're not showing blank lines
    KateTextCursor max = maxStartPos();
    if (startPos() > max) {
      scrollPos(max, max.col(), calledExternally);
    }
  }

  if (!m_view->dynWordWrap() && endCol != (uint)-1)
  {
    int sX = (int)m_view->renderer()->textWidth (textLine( m_doc->getRealLine( c.line() ) ), c.col() );

    int sXborder = sX-8;
    if (sXborder < 0)
      sXborder = 0;

    if (sX < m_startX)
      scrollColumns (sXborder);
    else if  (sX > m_startX + width())
      scrollColumns (sX - width() + 8);
  }

  m_madeVisible = !force;
}

void KateViewInternal::slotRegionVisibilityChangedAt(unsigned int)
{
  kdDebug(13030) << "slotRegionVisibilityChangedAt()" << endl;
  m_cachedMaxStartPos.setLine(-1);
  KateTextCursor max = maxStartPos();
  if (startPos() > max)
    scrollPos(max);

  updateView();
  update();
  leftBorder->update();
}

void KateViewInternal::slotCodeFoldingChanged()
{
  leftBorder->update();
}

void KateViewInternal::slotRegionBeginEndAddedRemoved(unsigned int)
{
  kdDebug(13030) << "slotRegionBeginEndAddedRemoved()" << endl;
  // FIXME: performance problem
  leftBorder->update();
}

void KateViewInternal::showEvent ( TQShowEvent *e )
{
  updateView ();

  TQWidget::showEvent (e);
}

uint KateViewInternal::linesDisplayed() const
{
  int h = height();
  int fh = m_view->renderer()->fontHeight();

  return (h - (h % fh)) / fh;
}

TQPoint KateViewInternal::cursorCoordinates()
{
  int viewLine = displayViewLine(displayCursor, true);

  if (viewLine == -1)
    return TQPoint(-1, -1);

  uint y = viewLine * m_view->renderer()->fontHeight();
  uint x = cXPos - m_startX - lineRanges[viewLine].startX + leftBorder->width() + lineRanges[viewLine].xOffset();

  return TQPoint(x, y);
}

void KateViewInternal::updateMicroFocusHint()
{
    int line = displayViewLine(displayCursor, true);
    /* Check for hasFocus() to avoid crashes in QXIMInputContext as in bug #131266.
    This is only a workaround until somebody can find the real reason of the crash
    (probably it's in Qt). */
    if (line == -1 || !hasFocus())
        return;

    KateRenderer *renderer = m_view->renderer();

    // Cursor placement code is changed for Asian input method that
    // shows candidate window. This behavior is same as Qt/E 2.3.7
    // which supports Asian input methods. Asian input methods need
    // start point of IM selection text to place candidate window as
    // adjacent to the selection text.
    uint preeditStrLen = renderer->textWidth(textLine(m_imPreeditStartLine), cursor.col()) - renderer->textWidth(textLine(m_imPreeditStartLine), m_imPreeditSelStart);
    uint x = cXPos - m_startX - lineRanges[line].startX + lineRanges[line].xOffset() - preeditStrLen;
    uint y = line * renderer->fontHeight();

    setMicroFocusHint(x, y, 0, renderer->fontHeight());
}

void KateViewInternal::doReturn()
{
  KateTextCursor c = cursor;
  m_doc->newLine( c, this );
  updateCursor( c );
  updateView();
}

void KateViewInternal::doDelete()
{
  m_doc->del( m_view, cursor );
  if (m_view->m_codeCompletion->codeCompletionVisible()) {
    m_view->m_codeCompletion->updateBox();
  }
}

void KateViewInternal::doBackspace()
{
  m_doc->backspace( m_view, cursor );
  if (m_view->m_codeCompletion->codeCompletionVisible()) {
    m_view->m_codeCompletion->updateBox();
  }
}

void KateViewInternal::doTranspose()
{
  m_doc->transpose( cursor );
}

void KateViewInternal::doDeleteWordLeft()
{
  wordLeft( true );
  m_view->removeSelectedText();
  update();
}

void KateViewInternal::doDeleteWordRight()
{
  wordRight( true );
  m_view->removeSelectedText();
  update();
}

class CalculatingCursor : public KateTextCursor {
public:
  CalculatingCursor(KateViewInternal* vi)
    : KateTextCursor()
    , m_vi(vi)
  {
    Q_ASSERT(valid());
  }

  CalculatingCursor(KateViewInternal* vi, const KateTextCursor& c)
    : KateTextCursor(c)
    , m_vi(vi)
  {
    Q_ASSERT(valid());
  }

  // This one constrains its arguments to valid positions
  CalculatingCursor(KateViewInternal* vi, uint line, uint col)
    : KateTextCursor(line, col)
    , m_vi(vi)
  {
    makeValid();
  }


  virtual CalculatingCursor& operator+=( int n ) = 0;

  virtual CalculatingCursor& operator-=( int n ) = 0;

  CalculatingCursor& operator++() { return operator+=( 1 ); }

  CalculatingCursor& operator--() { return operator-=( 1 ); }

  void makeValid() {
    m_line = kMax( 0, kMin( int( m_vi->m_doc->numLines() - 1 ), line() ) );
    if (m_vi->m_view->wrapCursor())
      m_col = kMax( 0, kMin( m_vi->m_doc->lineLength( line() ), col() ) );
    else
      m_col = kMax( 0, col() );
    Q_ASSERT( valid() );
  }

  void toEdge( Bias bias ) {
    if( bias == left_b ) m_col = 0;
    else if( bias == right_b ) m_col = m_vi->m_doc->lineLength( line() );
  }

  bool atEdge() const { return atEdge( left_b ) || atEdge( right_b ); }

  bool atEdge( Bias bias ) const {
    switch( bias ) {
    case left_b:  return col() == 0;
    case none:    return atEdge();
    case right_b: return col() == m_vi->m_doc->lineLength( line() );
    default: Q_ASSERT(false); return false;
    }
  }

protected:
  bool valid() const {
    return line() >= 0 &&
            uint( line() ) < m_vi->m_doc->numLines() &&
            col() >= 0 &&
            (!m_vi->m_view->wrapCursor() || col() <= m_vi->m_doc->lineLength( line() ));
  }
  KateViewInternal* m_vi;
};

class BoundedCursor : public CalculatingCursor {
public:
  BoundedCursor(KateViewInternal* vi)
    : CalculatingCursor( vi ) {};
  BoundedCursor(KateViewInternal* vi, const KateTextCursor& c )
    : CalculatingCursor( vi, c ) {};
  BoundedCursor(KateViewInternal* vi, uint line, uint col )
    : CalculatingCursor( vi, line, col ) {};
  virtual CalculatingCursor& operator+=( int n ) {
    m_col += n;

    if (n > 0 && m_vi->m_view->dynWordWrap()) {
      // Need to constrain to current visible text line for dynamic wrapping mode
      if (m_col > m_vi->m_doc->lineLength(m_line)) {
        KateLineRange currentRange = m_vi->range(*this);

        int endX;
        bool crap;
        m_vi->m_view->renderer()->textWidth(m_vi->textLine(m_line), currentRange.startCol, m_vi->width() - currentRange.xOffset(), &crap, &endX);
        endX += (m_col - currentRange.endCol + 1) * m_vi->m_view->renderer()->spaceWidth();

        // Constraining if applicable NOTE: some code duplication in KateViewInternal::resize()
        if (endX >= m_vi->width() - currentRange.xOffset()) {
          m_col -= n;
          if ( uint( line() ) < m_vi->m_doc->numLines() - 1 ) {
            m_line++;
            m_col = 0;
          }
        }
      }

    } else if (n < 0 && col() < 0 && line() > 0 ) {
      m_line--;
      m_col = m_vi->m_doc->lineLength( line() );
    }

    m_col = kMax( 0, col() );

    Q_ASSERT( valid() );
    return *this;
  }
  virtual CalculatingCursor& operator-=( int n ) {
    return operator+=( -n );
  }
};

class WrappingCursor : public CalculatingCursor {
public:
  WrappingCursor(KateViewInternal* vi)
    : CalculatingCursor( vi) {};
  WrappingCursor(KateViewInternal* vi, const KateTextCursor& c )
    : CalculatingCursor( vi, c ) {};
  WrappingCursor(KateViewInternal* vi, uint line, uint col )
    : CalculatingCursor( vi, line, col ) {};

  virtual CalculatingCursor& operator+=( int n ) {
    if( n < 0 ) return operator-=( -n );
    int len = m_vi->m_doc->lineLength( line() );
    if( col() + n <= len ) {
      m_col += n;
    } else if( uint( line() ) < m_vi->m_doc->numLines() - 1 ) {
      n -= len - col() + 1;
      m_col = 0;
      m_line++;
      operator+=( n );
    } else {
      m_col = len;
    }
    Q_ASSERT( valid() );
    return *this;
  }
  virtual CalculatingCursor& operator-=( int n ) {
    if( n < 0 ) return operator+=( -n );
    if( col() - n >= 0 ) {
      m_col -= n;
    } else if( line() > 0 ) {
      n -= col() + 1;
      m_line--;
      m_col = m_vi->m_doc->lineLength( line() );
      operator-=( n );
    } else {
      m_col = 0;
    }
    Q_ASSERT( valid() );
    return *this;
  }
};

void KateViewInternal::moveChar( Bias bias, bool sel )
{
  KateTextCursor c;
  if ( m_view->wrapCursor() ) {
    c = WrappingCursor( this, cursor ) += bias;
  } else {
    c = BoundedCursor( this, cursor ) += bias;
  }

  updateSelection( c, sel );
  updateCursor( c );
}

void KateViewInternal::cursorLeft(  bool sel )
{
  if ( ! m_view->wrapCursor() && cursor.col() == 0 )
    return;

  moveChar( left_b, sel );
  if (m_view->m_codeCompletion->codeCompletionVisible()) {
    m_view->m_codeCompletion->updateBox();
  }
}

void KateViewInternal::cursorRight( bool sel )
{
  moveChar( right_b, sel );
  if (m_view->m_codeCompletion->codeCompletionVisible()) {
    m_view->m_codeCompletion->updateBox();
  }
}

void KateViewInternal::wordLeft ( bool sel )
{
  WrappingCursor c( this, cursor );

  // First we skip backwards all space.
  // Then we look up into which category the current position falls:
  // 1. a "word" character
  // 2. a "non-word" character (except space)
  // 3. the beginning of the line
  // and skip all preceding characters that fall into this class.
  // The code assumes that space is never part of the word character class.

  KateHighlighting* h = m_doc->highlight();
  if( !c.atEdge( left_b ) ) {

    while( !c.atEdge( left_b ) && m_doc->textLine( c.line() )[ c.col() - 1 ].isSpace() )
      --c;
  }
  if( c.atEdge( left_b ) )
  {
    --c;
  }
  else if( h->isInWord( m_doc->textLine( c.line() )[ c.col() - 1 ] ) )
  {
    while( !c.atEdge( left_b ) && h->isInWord( m_doc->textLine( c.line() )[ c.col() - 1 ] ) )
      --c;
  }
  else
  {
    while( !c.atEdge( left_b )
           && !h->isInWord( m_doc->textLine( c.line() )[ c.col() - 1 ] )
           // in order to stay symmetric to wordLeft()
           // we must not skip space preceding a non-word sequence
           && !m_doc->textLine( c.line() )[ c.col() - 1 ].isSpace() )
    {
      --c;
    }
  }

  updateSelection( c, sel );
  updateCursor( c );
}

void KateViewInternal::wordRight( bool sel )
{
  WrappingCursor c( this, cursor );

  // We look up into which category the current position falls:
  // 1. a "word" character
  // 2. a "non-word" character (except space)
  // 3. the end of the line
  // and skip all following characters that fall into this class.
  // If the skipped characters are followed by space, we skip that too.
  // The code assumes that space is never part of the word character class.

  KateHighlighting* h = m_doc->highlight();
  if( c.atEdge( right_b ) )
  {
    ++c;
  }
  else if( h->isInWord( m_doc->textLine( c.line() )[ c.col() ] ) )
  {
    while( !c.atEdge( right_b ) && h->isInWord( m_doc->textLine( c.line() )[ c.col() ] ) )
      ++c;
  }
  else
  {
    while( !c.atEdge( right_b )
           && !h->isInWord( m_doc->textLine( c.line() )[ c.col() ] )
           // we must not skip space, because if that space is followed
           // by more non-word characters, we would skip them, too
           && !m_doc->textLine( c.line() )[ c.col() ].isSpace() )
    {
      ++c;
    }
  }

  while( !c.atEdge( right_b ) && m_doc->textLine( c.line() )[ c.col() ].isSpace() )
    ++c;

  updateSelection( c, sel );
  updateCursor( c );
}

void KateViewInternal::moveEdge( Bias bias, bool sel )
{
  BoundedCursor c( this, cursor );
  c.toEdge( bias );
  updateSelection( c, sel );
  updateCursor( c );
}

void KateViewInternal::home( bool sel )
{
  if (m_view->m_codeCompletion->codeCompletionVisible()) {
    TQKeyEvent e(TQEvent::KeyPress, TQt::Key_Home, 0, 0);
    m_view->m_codeCompletion->handleKey(&e);
    return;
  }

  if (m_view->dynWordWrap() && currentRange().startCol) {
    // Allow us to go to the real start if we're already at the start of the view line
    if (cursor.col() != currentRange().startCol) {
      KateTextCursor c(cursor.line(), currentRange().startCol);
      updateSelection( c, sel );
      updateCursor( c );
      return;
    }
  }

  if( !(m_doc->configFlags() & KateDocument::cfSmartHome) ) {
    moveEdge( left_b, sel );
    return;
  }

  KateTextLine::Ptr l = textLine( cursor.line() );

  if (!l)
    return;

  KateTextCursor c = cursor;
  int lc = l->firstChar();

  if( lc < 0 || c.col() == lc ) {
    c.setCol(0);
  } else {
    c.setCol(lc);
  }

  updateSelection( c, sel );
  updateCursor( c, true );
}

void KateViewInternal::end( bool sel )
{
  if (m_view->m_codeCompletion->codeCompletionVisible()) {
    TQKeyEvent e(TQEvent::KeyPress, TQt::Key_End, 0, 0);
    m_view->m_codeCompletion->handleKey(&e);
    return;
  }

  KateLineRange range = currentRange();

  if (m_view->dynWordWrap() && range.wrap) {
    // Allow us to go to the real end if we're already at the end of the view line
    if (cursor.col() < range.endCol - 1) {
      KateTextCursor c(cursor.line(), range.endCol - 1);
      updateSelection( c, sel );
      updateCursor( c );
      return;
    }
  }

  if( !(m_doc->configFlags() & KateDocument::cfSmartHome) ) {
    moveEdge( right_b, sel );
    return;
  }

  KateTextLine::Ptr l = textLine( cursor.line() );

  if (!l)
    return;

  // "Smart End", as requested in bugs #78258 and #106970
  KateTextCursor c = cursor;

  // If the cursor is already the real end, jump to last non-space character.
  // Otherwise, go to the real end ... obviously.
  if (c.col() == m_doc->lineLength(c.line())) {
    c.setCol(l->lastChar() + 1);
    updateSelection(c, sel);
    updateCursor(c, true);
  } else {
    moveEdge(right_b, sel);
  }
}

KateLineRange KateViewInternal::range(int realLine, const KateLineRange* previous)
{
  // look at the cache first
  if (!m_updatingView && realLine >= lineRanges[0].line && realLine <= lineRanges[lineRanges.count() - 1].line)
    for (uint i = 0; i < lineRanges.count(); i++)
      if (realLine == lineRanges[i].line)
        if (!m_view->dynWordWrap() || (!previous && lineRanges[i].startCol == 0) || (previous && lineRanges[i].startCol == previous->endCol))
          return lineRanges[i];

  // Not in the cache, we have to create it
  KateLineRange ret;

  KateTextLine::Ptr text = textLine(realLine);
  if (!text) {
    return KateLineRange();
  }

  if (!m_view->dynWordWrap()) {
    Q_ASSERT(!previous);
    ret.line = realLine;
    ret.virtualLine = m_doc->getVirtualLine(realLine);
    ret.startCol = 0;
    ret.endCol = m_doc->lineLength(realLine);
    ret.startX = 0;
    ret.endX = m_view->renderer()->textWidth(text, -1);
    ret.viewLine = 0;
    ret.wrap = false;
    return ret;
  }

  ret.endCol = (int)m_view->renderer()->textWidth(text, previous ? previous->endCol : 0, width() - (previous ? previous->shiftX : 0), &ret.wrap, &ret.endX);

  Q_ASSERT(ret.endCol > ret.startCol);

  ret.line = realLine;

  if (previous) {
    ret.virtualLine = previous->virtualLine;
    ret.startCol = previous->endCol;
    ret.startX = previous->endX;
    ret.endX += previous->endX;
    ret.shiftX = previous->shiftX;
    ret.viewLine = previous->viewLine + 1;

  } else {
    // TODO worthwhile optimising this to get the data out of the initial textWidth call?
    if (m_view->config()->dynWordWrapAlignIndent() > 0) {
      int pos = text->nextNonSpaceChar(0);

      if (pos > 0)
        ret.shiftX = m_view->renderer()->textWidth(text, pos);

      if (ret.shiftX > ((double)width() / 100 * m_view->config()->dynWordWrapAlignIndent()))
        ret.shiftX = 0;
    }

    ret.virtualLine = m_doc->getVirtualLine(realLine);
    ret.startCol = 0;
    ret.startX = 0;
    ret.viewLine = 0;
  }

  return ret;
}

KateLineRange KateViewInternal::currentRange()
{
//  Q_ASSERT(m_view->dynWordWrap());

  return range(cursor);
}

KateLineRange KateViewInternal::previousRange()
{
  uint currentViewLine = viewLine(cursor);

  if (currentViewLine)
    return range(cursor.line(), currentViewLine - 1);
  else
    return range(m_doc->getRealLine(displayCursor.line() - 1), -1);
}

KateLineRange KateViewInternal::nextRange()
{
  uint currentViewLine = viewLine(cursor) + 1;

  if (currentViewLine >= viewLineCount(cursor.line())) {
    currentViewLine = 0;
    return range(cursor.line() + 1, currentViewLine);
  } else {
    return range(cursor.line(), currentViewLine);
  }
}

KateLineRange KateViewInternal::range(const KateTextCursor& realCursor)
{
//  Q_ASSERT(m_view->dynWordWrap());

  KateLineRange thisRange;
  bool first = true;

  do {
    thisRange = range(realCursor.line(), first ? 0L : &thisRange);
    first = false;
  } while (thisRange.wrap && !(realCursor.col() >= thisRange.startCol && realCursor.col() < thisRange.endCol) && thisRange.startCol != thisRange.endCol);

  return thisRange;
}

KateLineRange KateViewInternal::range(uint realLine, int viewLine)
{
//  Q_ASSERT(m_view->dynWordWrap());

  KateLineRange thisRange;
  bool first = true;

  do {
    thisRange = range(realLine, first ? 0L : &thisRange);
    first = false;
  } while (thisRange.wrap && viewLine != thisRange.viewLine && thisRange.startCol != thisRange.endCol);

  if (viewLine != -1 && viewLine != thisRange.viewLine)
    kdDebug(13030) << "WARNING: viewLine " << viewLine << " of line " << realLine << " does not exist." << endl;

  return thisRange;
}

/**
 * This returns the view line upon which realCursor is situated.
 * The view line is the number of lines in the view from the first line
 * The supplied cursor should be in real lines.
 */
uint KateViewInternal::viewLine(const KateTextCursor& realCursor)
{
  if (!m_view->dynWordWrap()) return 0;

  if (realCursor.col() == 0) return 0;

  KateLineRange thisRange;
  bool first = true;

  do {
    thisRange = range(realCursor.line(), first ? 0L : &thisRange);
    first = false;
  } while (thisRange.wrap && !(realCursor.col() >= thisRange.startCol && realCursor.col() < thisRange.endCol) && thisRange.startCol != thisRange.endCol);

  return thisRange.viewLine;
}

int KateViewInternal::displayViewLine(const KateTextCursor& virtualCursor, bool limitToVisible)
{
  KateTextCursor work = startPos();

  int limit = linesDisplayed();

  // Efficient non-word-wrapped path
  if (!m_view->dynWordWrap()) {
    int ret = virtualCursor.line() - startLine();
    if (limitToVisible && (ret < 0 || ret > limit))
      return -1;
    else
      return ret;
  }

  if (work == virtualCursor) {
    return 0;
  }

  int ret = -(int)viewLine(work);
  bool forwards = (work < virtualCursor) ? true : false;

  // FIXME switch to using ranges? faster?
  if (forwards) {
    while (work.line() != virtualCursor.line()) {
      ret += viewLineCount(m_doc->getRealLine(work.line()));
      work.setLine(work.line() + 1);
      if (limitToVisible && ret > limit)
        return -1;
    }
  } else {
    while (work.line() != virtualCursor.line()) {
      work.setLine(work.line() - 1);
      ret -= viewLineCount(m_doc->getRealLine(work.line()));
      if (limitToVisible && ret < 0)
        return -1;
    }
  }

  // final difference
  KateTextCursor realCursor = virtualCursor;
  realCursor.setLine(m_doc->getRealLine(realCursor.line()));
  if (realCursor.col() == -1) realCursor.setCol(m_doc->lineLength(realCursor.line()));
  ret += viewLine(realCursor);

  if (limitToVisible && (ret < 0 || ret > limit))
    return -1;

  return ret;
}

uint KateViewInternal::lastViewLine(uint realLine)
{
  if (!m_view->dynWordWrap()) return 0;

  KateLineRange thisRange;
  bool first = true;

  do {
    thisRange = range(realLine, first ? 0L : &thisRange);
    first = false;
  } while (thisRange.wrap && thisRange.startCol != thisRange.endCol);

  return thisRange.viewLine;
}

uint KateViewInternal::viewLineCount(uint realLine)
{
  return lastViewLine(realLine) + 1;
}

/*
 * This returns the cursor which is offset by (offset) view lines.
 * This is the main function which is called by code not specifically dealing with word-wrap.
 * The opposite conversion (cursor to offset) can be done with displayViewLine.
 *
 * The cursors involved are virtual cursors (ie. equivalent to displayCursor)
 */
KateTextCursor KateViewInternal::viewLineOffset(const KateTextCursor& virtualCursor, int offset, bool keepX)
{
  if (!m_view->dynWordWrap()) {
    KateTextCursor ret(kMin((int)m_doc->visibleLines() - 1, virtualCursor.line() + offset), 0);

    if (ret.line() < 0)
      ret.setLine(0);

    if (keepX) {
      int realLine = m_doc->getRealLine(ret.line());
      ret.setCol(m_doc->lineLength(realLine) - 1);

      if (m_currentMaxX > cXPos)
        cXPos = m_currentMaxX;

      if (m_view->wrapCursor())
        cXPos = kMin(cXPos, (int)m_view->renderer()->textWidth(textLine(realLine), m_doc->lineLength(realLine)));

      m_view->renderer()->textWidth(ret, cXPos);
    }

    return ret;
  }

  KateTextCursor realCursor = virtualCursor;
  realCursor.setLine(m_doc->getRealLine(virtualCursor.line()));

  uint cursorViewLine = viewLine(realCursor);

  int currentOffset = 0;
  int virtualLine = 0;

  bool forwards = (offset > 0) ? true : false;

  if (forwards) {
    currentOffset = lastViewLine(realCursor.line()) - cursorViewLine;
    if (offset <= currentOffset) {
      // the answer is on the same line
      KateLineRange thisRange = range(realCursor.line(), cursorViewLine + offset);
      Q_ASSERT(thisRange.virtualLine == virtualCursor.line());
      return KateTextCursor(virtualCursor.line(), thisRange.startCol);
    }

    virtualLine = virtualCursor.line() + 1;

  } else {
    offset = -offset;
    currentOffset = cursorViewLine;
    if (offset <= currentOffset) {
      // the answer is on the same line
      KateLineRange thisRange = range(realCursor.line(), cursorViewLine - offset);
      Q_ASSERT(thisRange.virtualLine == virtualCursor.line());
      return KateTextCursor(virtualCursor.line(), thisRange.startCol);
    }

    virtualLine = virtualCursor.line() - 1;
  }

  currentOffset++;

  while (virtualLine >= 0 && virtualLine < (int)m_doc->visibleLines())
  {
    KateLineRange thisRange;
    bool first = true;
    int realLine = m_doc->getRealLine(virtualLine);

    do {
      thisRange = range(realLine, first ? 0L : &thisRange);
      first = false;

      if (offset == currentOffset) {
        if (!forwards) {
          // We actually want it the other way around
          int requiredViewLine = lastViewLine(realLine) - thisRange.viewLine;
          if (requiredViewLine != thisRange.viewLine) {
            thisRange = range(realLine, requiredViewLine);
          }
        }

        KateTextCursor ret(virtualLine, thisRange.startCol);

        // keep column position
        if (keepX) {
          ret.setCol(thisRange.endCol - 1);
          KateTextCursor realCursorTemp(m_doc->getRealLine(virtualCursor.line()), virtualCursor.col());
          int visibleX = m_view->renderer()->textWidth(realCursorTemp) - range(realCursorTemp).startX;
          int xOffset = thisRange.startX;

          if (m_currentMaxX > visibleX)
            visibleX = m_currentMaxX;

          cXPos = xOffset + visibleX;

          cXPos = kMin(cXPos, lineMaxCursorX(thisRange));

          m_view->renderer()->textWidth(ret, cXPos);
        }

        return ret;
      }

      currentOffset++;

    } while (thisRange.wrap);

    if (forwards)
      virtualLine++;
    else
      virtualLine--;
  }

  // Looks like we were asked for something a bit exotic.
  // Return the max/min valid position.
  if (forwards)
    return KateTextCursor(m_doc->visibleLines() - 1, m_doc->lineLength(m_doc->visibleLines() - 1));
  else
    return KateTextCursor(0, 0);
}

int KateViewInternal::lineMaxCursorX(const KateLineRange& range)
{
  if (!m_view->wrapCursor() && !range.wrap)
    return INT_MAX;

  int maxX = range.endX;

  if (maxX && range.wrap) {
    TQChar lastCharInLine = textLine(range.line)->getChar(range.endCol - 1);

    if (lastCharInLine == TQChar('\t')) {
      int lineSize = 0;
      int lastTabSize = 0;
      for(int i = range.startCol; i < range.endCol; i++) {
        if (textLine(range.line)->getChar(i) == TQChar('\t')) {
          lastTabSize = m_view->tabWidth() - (lineSize % m_view->tabWidth());
          lineSize += lastTabSize;
        } else {
          lineSize++;
        }
      }
      maxX -= lastTabSize * m_view->renderer()->spaceWidth();
    } else {
      maxX -= m_view->renderer()->config()->fontMetrics()->width(lastCharInLine);
    }
  }

  return maxX;
}

int KateViewInternal::lineMaxCol(const KateLineRange& range)
{
  int maxCol = range.endCol;

  if (maxCol && range.wrap)
    maxCol--;

  return maxCol;
}

void KateViewInternal::cursorUp(bool sel)
{
  if (m_view->m_codeCompletion->codeCompletionVisible()) {
    TQKeyEvent e(TQEvent::KeyPress, TQt::Key_Up, 0, 0);
    m_view->m_codeCompletion->handleKey(&e);
    return;
  }

  if (displayCursor.line() == 0 && (!m_view->dynWordWrap() || viewLine(cursor) == 0))
    return;

  int newLine = cursor.line(), newCol = 0, xOffset = 0, startCol = 0;
  m_preserveMaxX = true;

  if (m_view->dynWordWrap()) {
    // Dynamic word wrapping - navigate on visual lines rather than real lines
    KateLineRange thisRange = currentRange();
    // This is not the first line because that is already simplified out above
    KateLineRange pRange = previousRange();

    // Ensure we're in the right spot
    Q_ASSERT((cursor.line() == thisRange.line) &&
             (cursor.col() >= thisRange.startCol) &&
             (!thisRange.wrap || cursor.col() < thisRange.endCol));

    // VisibleX is the distance from the start of the text to the cursor on the current line.
    int visibleX = m_view->renderer()->textWidth(cursor) - thisRange.startX;
    int currentLineVisibleX = visibleX;

    // Translate to new line
    visibleX += thisRange.xOffset();
    visibleX -= pRange.xOffset();

    // Limit to >= 0
    visibleX = kMax(0, visibleX);

    startCol = pRange.startCol;
    xOffset = pRange.startX;
    newLine = pRange.line;

    // Take into account current max X (ie. if the current line was smaller
    // than the last definitely specified width)
    if (thisRange.xOffset() && !pRange.xOffset() && currentLineVisibleX == 0) // Special case for where xOffset may be > m_currentMaxX
      visibleX = m_currentMaxX;
    else if (visibleX < m_currentMaxX - pRange.xOffset())
      visibleX = m_currentMaxX - pRange.xOffset();

    cXPos = xOffset + visibleX;

    cXPos = kMin(cXPos, lineMaxCursorX(pRange));

    newCol = kMin((int)m_view->renderer()->textPos(newLine, visibleX, startCol), lineMaxCol(pRange));

  } else {
    newLine = m_doc->getRealLine(displayCursor.line() - 1);

    if ((m_view->wrapCursor()) && m_currentMaxX > cXPos)
      cXPos = m_currentMaxX;
  }

  KateTextCursor c(newLine, newCol);
  m_view->renderer()->textWidth(c, cXPos);

  updateSelection( c, sel );
  updateCursor( c );
}

void KateViewInternal::cursorDown(bool sel)
{
  if (m_view->m_codeCompletion->codeCompletionVisible()) {
    TQKeyEvent e(TQEvent::KeyPress, TQt::Key_Down, 0, 0);
    m_view->m_codeCompletion->handleKey(&e);
    return;
  }

  if ((displayCursor.line() >= (int)m_doc->numVisLines() - 1) && (!m_view->dynWordWrap() || viewLine(cursor) == lastViewLine(cursor.line())))
    return;

  int newLine = cursor.line(), newCol = 0, xOffset = 0, startCol = 0;
  m_preserveMaxX = true;

  if (m_view->dynWordWrap()) {
    // Dynamic word wrapping - navigate on visual lines rather than real lines
    KateLineRange thisRange = currentRange();
    // This is not the last line because that is already simplified out above
    KateLineRange nRange = nextRange();

    // Ensure we're in the right spot
    Q_ASSERT((cursor.line() == thisRange.line) &&
             (cursor.col() >= thisRange.startCol) &&
             (!thisRange.wrap || cursor.col() < thisRange.endCol));

    // VisibleX is the distance from the start of the text to the cursor on the current line.
    int visibleX = m_view->renderer()->textWidth(cursor) - thisRange.startX;
    int currentLineVisibleX = visibleX;

    // Translate to new line
    visibleX += thisRange.xOffset();
    visibleX -= nRange.xOffset();

    // Limit to >= 0
    visibleX = kMax(0, visibleX);

    if (!thisRange.wrap) {
      newLine = m_doc->getRealLine(displayCursor.line() + 1);
    } else {
      startCol = thisRange.endCol;
      xOffset = thisRange.endX;
    }

    // Take into account current max X (ie. if the current line was smaller
    // than the last definitely specified width)
    if (thisRange.xOffset() && !nRange.xOffset() && currentLineVisibleX == 0) // Special case for where xOffset may be > m_currentMaxX
      visibleX = m_currentMaxX;
    else if (visibleX < m_currentMaxX - nRange.xOffset())
      visibleX = m_currentMaxX - nRange.xOffset();

    cXPos = xOffset + visibleX;

    cXPos = kMin(cXPos, lineMaxCursorX(nRange));

    newCol = kMin((int)m_view->renderer()->textPos(newLine, visibleX, startCol), lineMaxCol(nRange));

  } else {
    newLine = m_doc->getRealLine(displayCursor.line() + 1);

    if ((m_view->wrapCursor()) && m_currentMaxX > cXPos)
      cXPos = m_currentMaxX;
  }

  KateTextCursor c(newLine, newCol);
  m_view->renderer()->textWidth(c, cXPos);

  updateSelection(c, sel);
  updateCursor(c);
}

void KateViewInternal::cursorToMatchingBracket( bool sel )
{
  KateTextCursor start( cursor ), end;

  if( !m_doc->findMatchingBracket( start, end ) )
    return;

  // The cursor is now placed just to the left of the matching bracket.
  // If it's an ending bracket, put it to the right (so we can easily
  // get back to the original bracket).
  if( end > start )
    end.setCol(end.col() + 1);

  updateSelection( end, sel );
  updateCursor( end );
}

void KateViewInternal::topOfView( bool sel )
{
  KateTextCursor c = viewLineOffset(startPos(), m_minLinesVisible);
  updateSelection( c, sel );
  updateCursor( c );
}

void KateViewInternal::bottomOfView( bool sel )
{
  // FIXME account for wordwrap
  KateTextCursor c = viewLineOffset(endPos(), -m_minLinesVisible);
  updateSelection( c, sel );
  updateCursor( c );
}

// lines is the offset to scroll by
void KateViewInternal::scrollLines( int lines, bool sel )
{
  KateTextCursor c = viewLineOffset(displayCursor, lines, true);

  // Fix the virtual cursor -> real cursor
  c.setLine(m_doc->getRealLine(c.line()));

  updateSelection( c, sel );
  updateCursor( c );
}

// This is a bit misleading... it's asking for the view to be scrolled, not the cursor
void KateViewInternal::scrollUp()
{
  KateTextCursor newPos = viewLineOffset(m_startPos, -1);
  scrollPos(newPos);
}

void KateViewInternal::scrollDown()
{
  KateTextCursor newPos = viewLineOffset(m_startPos, 1);
  scrollPos(newPos);
}

void KateViewInternal::setAutoCenterLines(int viewLines, bool updateView)
{
  m_autoCenterLines = viewLines;
  m_minLinesVisible = kMin(int((linesDisplayed() - 1)/2), m_autoCenterLines);
  if (updateView)
    KateViewInternal::updateView();
}

void KateViewInternal::pageUp( bool sel )
{
  if (m_view->m_codeCompletion->codeCompletionVisible()) {
    TQKeyEvent e(TQEvent::KeyPress, TQt::Key_PageUp, 0, 0);
    m_view->m_codeCompletion->handleKey(&e);
    return;
  }

  // remember the view line and x pos
  int viewLine = displayViewLine(displayCursor);
  bool atTop = (startPos().line() == 0 && startPos().col() == 0);

  // Adjust for an auto-centering cursor
  int lineadj = 2 * m_minLinesVisible;
  int cursorStart = (linesDisplayed() - 1) - viewLine;
  if (cursorStart < m_minLinesVisible)
    lineadj -= m_minLinesVisible - cursorStart;

  int linesToScroll = -kMax( ((int)linesDisplayed() - 1) - lineadj, 0 );
  m_preserveMaxX = true;

  if (!m_doc->pageUpDownMovesCursor () && !atTop) {
    int xPos = m_view->renderer()->textWidth(cursor) - currentRange().startX;

    KateTextCursor newStartPos = viewLineOffset(startPos(), linesToScroll - 1);
    scrollPos(newStartPos);

    // put the cursor back approximately where it was
    KateTextCursor newPos = viewLineOffset(newStartPos, viewLine, true);
    newPos.setLine(m_doc->getRealLine(newPos.line()));

    KateLineRange newLine = range(newPos);

    if (m_currentMaxX - newLine.xOffset() > xPos)
      xPos = m_currentMaxX - newLine.xOffset();

    cXPos = kMin(newLine.startX + xPos, lineMaxCursorX(newLine));

    m_view->renderer()->textWidth( newPos, cXPos );

    m_preserveMaxX = true;
    updateSelection( newPos, sel );
    updateCursor(newPos);

  } else {
    scrollLines( linesToScroll, sel );
  }
}

void KateViewInternal::pageDown( bool sel )
{
  if (m_view->m_codeCompletion->codeCompletionVisible()) {
    TQKeyEvent e(TQEvent::KeyPress, TQt::Key_PageDown, 0, 0);
    m_view->m_codeCompletion->handleKey(&e);
    return;
  }

  // remember the view line
  int viewLine = displayViewLine(displayCursor);
  bool atEnd = startPos() >= m_cachedMaxStartPos;

  // Adjust for an auto-centering cursor
  int lineadj = 2 * m_minLinesVisible;
  int cursorStart = m_minLinesVisible - viewLine;
  if (cursorStart > 0)
    lineadj -= cursorStart;

  int linesToScroll = kMax( ((int)linesDisplayed() - 1) - lineadj, 0 );
  m_preserveMaxX = true;

  if (!m_doc->pageUpDownMovesCursor () && !atEnd) {
    int xPos = m_view->renderer()->textWidth(cursor) - currentRange().startX;

    KateTextCursor newStartPos = viewLineOffset(startPos(), linesToScroll + 1);
    scrollPos(newStartPos);

    // put the cursor back approximately where it was
    KateTextCursor newPos = viewLineOffset(newStartPos, viewLine, true);
    newPos.setLine(m_doc->getRealLine(newPos.line()));

    KateLineRange newLine = range(newPos);

    if (m_currentMaxX - newLine.xOffset() > xPos)
      xPos = m_currentMaxX - newLine.xOffset();

    cXPos = kMin(newLine.startX + xPos, lineMaxCursorX(newLine));

    m_view->renderer()->textWidth( newPos, cXPos );

    m_preserveMaxX = true;
    updateSelection( newPos, sel );
    updateCursor(newPos);

  } else {
    scrollLines( linesToScroll, sel );
  }
}

int KateViewInternal::maxLen(uint startLine)
{
//  Q_ASSERT(!m_view->dynWordWrap());

  int displayLines = (m_view->height() / m_view->renderer()->fontHeight()) + 1;

  int maxLen = 0;

  for (int z = 0; z < displayLines; z++) {
    int virtualLine = startLine + z;

    if (virtualLine < 0 || virtualLine >= (int)m_doc->visibleLines())
      break;

    KateLineRange thisRange = range((int)m_doc->getRealLine(virtualLine));

    maxLen = kMax(maxLen, thisRange.endX);
  }

  return maxLen;
}

void KateViewInternal::top( bool sel )
{
  KateTextCursor c( 0, cursor.col() );
  m_view->renderer()->textWidth( c, cXPos );
  updateSelection( c, sel );
  updateCursor( c );
}

void KateViewInternal::bottom( bool sel )
{
  KateTextCursor c( m_doc->lastLine(), cursor.col() );
  m_view->renderer()->textWidth( c, cXPos );
  updateSelection( c, sel );
  updateCursor( c );
}

void KateViewInternal::top_home( bool sel )
{
  if (m_view->m_codeCompletion->codeCompletionVisible()) {
    TQKeyEvent e(TQEvent::KeyPress, TQt::Key_Home, 0, 0);
    m_view->m_codeCompletion->handleKey(&e);
    return;
  }
  KateTextCursor c( 0, 0 );
  updateSelection( c, sel );
  updateCursor( c );
}

void KateViewInternal::bottom_end( bool sel )
{
  if (m_view->m_codeCompletion->codeCompletionVisible()) {
    TQKeyEvent e(TQEvent::KeyPress, TQt::Key_End, 0, 0);
    m_view->m_codeCompletion->handleKey(&e);
    return;
  }
  KateTextCursor c( m_doc->lastLine(), m_doc->lineLength( m_doc->lastLine() ) );
  updateSelection( c, sel );
  updateCursor( c );
}

void KateViewInternal::updateSelection( const KateTextCursor& _newCursor, bool keepSel )
{
  KateTextCursor newCursor = _newCursor;
  if( keepSel )
  {
    if ( !m_view->hasSelection() || (selectAnchor.line() == -1)
         || (m_view->config()->persistentSelection()
             && ((cursor < m_view->selectStart) || (cursor > m_view->selectEnd))) )
    {
      selectAnchor = cursor;
      m_view->setSelection( cursor, newCursor );
    }
    else
    {
      bool doSelect = true;
      switch (m_selectionMode)
      {
        case Word:
        {
          // Restore selStartCached if needed. It gets nuked by
          // viewSelectionChanged if we drag the selection into non-existence,
          // which can legitimately happen if a shift+DC selection is unable to
          // set a "proper" (i.e. non-empty) cached selection, e.g. because the
          // start was on something that isn't a word. Word select mode relies
          // on the cached selection being set properly, even if it is empty
          // (i.e. selStartCached == selEndCached).
          if ( selStartCached.line() == -1 )
            selStartCached = selEndCached;

          int c;
          if ( newCursor > selEndCached )
          {
            selectAnchor = selStartCached;

            KateTextLine::Ptr l = m_doc->kateTextLine( newCursor.line() );

            c = newCursor.col();
            if ( c > 0 && m_doc->highlight()->isInWord( l->getChar( c-1 ) ) ) {
              for (; c < l->length(); c++ )
                if ( !m_doc->highlight()->isInWord( l->getChar( c ) ) )
                  break;
            }

            newCursor.setCol( c );
          }
          else if ( newCursor < selStartCached )
          {
            selectAnchor = selEndCached;

            KateTextLine::Ptr l = m_doc->kateTextLine( newCursor.line() );

            c = newCursor.col();
            if ( c > 0 && c < m_doc->textLine( newCursor.line() ).length()
                 && m_doc->highlight()->isInWord( l->getChar( c ) )
                 && m_doc->highlight()->isInWord( l->getChar( c-1 ) ) ) {
              for ( c -= 2; c >= 0; c-- )
                if ( !m_doc->highlight()->isInWord( l->getChar( c ) ) )
                  break;
              newCursor.setCol( c+1 );
            }

          }
          else
            doSelect = false;

        }
        break;
        case Line:
          if ( newCursor.line() > selStartCached.line() )
          {
            if ( newCursor.line()+1 >= m_doc->numLines() )
              newCursor.setCol( m_doc->textLine( newCursor.line() ).length() );
            else
              newCursor.setPos( newCursor.line() + 1, 0 );
            // Grow to include entire line
            selectAnchor = selStartCached;
            selectAnchor.setCol( 0 );
          }
          else if ( newCursor.line() < selStartCached.line() )
          {
            newCursor.setCol( 0 );
            // Grow to include entire line
            selectAnchor = selEndCached;
            if ( selectAnchor.col() > 0 )
            {
              if ( selectAnchor.line()+1 >= m_doc->numLines() )
                selectAnchor.setCol( m_doc->textLine( selectAnchor.line() ).length() );
              else
                selectAnchor.setPos( selectAnchor.line() + 1, 0 );
            }
          }
          else // same line, ignore
            doSelect = false;
        break;
        case Mouse:
        {
          if ( selStartCached.line() < 0 ) // invalid
            break;

          if ( newCursor > selEndCached )
            selectAnchor = selStartCached;
          else if ( newCursor < selStartCached )
            selectAnchor = selEndCached;
          else
            doSelect = false;
        }
        break;
        default:
        {
          if ( selectAnchor.line() < 0 ) // invalid
            break;
        }
      }

      if ( doSelect )
        m_view->setSelection( selectAnchor, newCursor);
      else if ( selStartCached.line() >= 0 ) // we have a cached selection, so we restore that
        m_view->setSelection( selStartCached, selEndCached );
    }

    m_selChangedByUser = true;
  }
  else if ( !m_view->config()->persistentSelection() )
  {
    m_view->clearSelection();
    selStartCached.setLine( -1 );
    selectAnchor.setLine( -1 );
  }
}

void KateViewInternal::updateCursor( const KateTextCursor& newCursor, bool force, bool center, bool calledExternally )
{
  if ( !force && (cursor == newCursor) )
  {
    if ( !m_madeVisible && m_view == m_doc->activeView() )
    {
      // unfold if required
      m_doc->foldingTree()->ensureVisible( newCursor.line() );

      makeVisible ( displayCursor, displayCursor.col(), false, center, calledExternally );
    }

    return;
  }

  // unfold if required
  m_doc->foldingTree()->ensureVisible( newCursor.line() );

  KateTextCursor oldDisplayCursor = displayCursor;

  cursor.setPos (newCursor);
  displayCursor.setPos (m_doc->getVirtualLine(cursor.line()), cursor.col());

  cXPos = m_view->renderer()->textWidth( cursor );
  if (m_view == m_doc->activeView())
    makeVisible ( displayCursor, displayCursor.col(), false, center, calledExternally );

  updateBracketMarks();

  // It's efficient enough to just tag them both without checking to see if they're on the same view line
  tagLine(oldDisplayCursor);
  tagLine(displayCursor);

  updateMicroFocusHint();

  if (m_cursorTimer.isActive ())
  {
    if ( TDEApplication::cursorFlashTime() > 0 )
      m_cursorTimer.start( TDEApplication::cursorFlashTime() / 2 );
    m_view->renderer()->setDrawCaret(true);
  }

  // Remember the maximum X position if requested
  if (m_preserveMaxX)
    m_preserveMaxX = false;
  else
    if (m_view->dynWordWrap())
      m_currentMaxX = m_view->renderer()->textWidth(displayCursor) - currentRange().startX + currentRange().xOffset();
    else
      m_currentMaxX = cXPos;

  //kdDebug() << "m_currentMaxX: " << m_currentMaxX << " (was "<< oldmaxx << "), cXPos: " << cXPos << endl;
  //kdDebug(13030) << "Cursor now located at real " << cursor.line << "," << cursor.col << ", virtual " << displayCursor.line << ", " << displayCursor.col << "; Top is " << startLine() << ", " << startPos().col <<  endl;

  paintText(0, 0, width(), height(), true);

  emit m_view->cursorPositionChanged();
}

void KateViewInternal::updateBracketMarks()
{
  if ( bm.isValid() ) {
    KateTextCursor bmStart(m_doc->getVirtualLine(bm.start().line()), bm.start().col());
    KateTextCursor bmEnd(m_doc->getVirtualLine(bm.end().line()), bm.end().col());

    if( bm.getMinIndent() != 0 )
    {
      // @@ Do this only when cursor near start/end.
      if( bmStart > bmEnd )
      {
        tagLines(bmEnd, bmStart);
      }
      else
      {
        tagLines(bmStart, bmEnd);
      }
    }
    else
    {
      tagLine(bmStart);
      tagLine(bmEnd);
    }
  }

  // add some limit to this, this is really endless on big files without limit
  int maxLines = linesDisplayed () * 3;
  m_doc->newBracketMark( cursor, bm, maxLines );

  if ( bm.isValid() ) {
    KateTextCursor bmStart(m_doc->getVirtualLine(bm.start().line()), bm.start().col());
    KateTextCursor bmEnd(m_doc->getVirtualLine(bm.end().line()), bm.end().col());

    if( bm.getMinIndent() != 0 )
    {
      // @@ Do this only when cursor near start/end.
      if( bmStart > bmEnd )
      {
        tagLines(bmEnd, bmStart);
      }
      else
      {
        tagLines(bmStart, bmEnd);
      }
    }
    else
    {
      tagLine(bmStart);
      tagLine(bmEnd);
    }
  }
}

bool KateViewInternal::tagLine(const KateTextCursor& virtualCursor)
{
  int viewLine = displayViewLine(virtualCursor, true);
  if (viewLine >= 0 && viewLine < (int)lineRanges.count()) {
    lineRanges[viewLine].dirty = true;
    leftBorder->update (0, lineToY(viewLine), leftBorder->width(), m_view->renderer()->fontHeight());
    return true;
  }
  return false;
}

bool KateViewInternal::tagLines( int start, int end, bool realLines )
{
  return tagLines(KateTextCursor(start, 0), KateTextCursor(end, -1), realLines);
}

bool KateViewInternal::tagLines(KateTextCursor start, KateTextCursor end, bool realCursors)
{
  if (realCursors)
  {
    //kdDebug()<<"realLines is true"<<endl;
    start.setLine(m_doc->getVirtualLine( start.line() ));
    end.setLine(m_doc->getVirtualLine( end.line() ));
  }

  if (end.line() < (int)startLine())
  {
    //kdDebug()<<"end<startLine"<<endl;
    return false;
  }
  if (start.line() > (int)endLine())
  {
    //kdDebug()<<"start> endLine"<<start<<" "<<((int)endLine())<<endl;
    return false;
  }

  //kdDebug(13030) << "tagLines( [" << start.line << "," << start.col << "], [" << end.line << "," << end.col << "] )\n";

  bool ret = false;

  for (uint z = 0; z < lineRanges.size(); z++)
  {
    if ((lineRanges[z].virtualLine > start.line() || (lineRanges[z].virtualLine == start.line() && lineRanges[z].endCol >= start.col() && start.col() != -1)) && (lineRanges[z].virtualLine < end.line() || (lineRanges[z].virtualLine == end.line() && (lineRanges[z].startCol <= end.col() || end.col() == -1)))) {
      ret = lineRanges[z].dirty = true;
      //kdDebug() << "Tagged line " << lineRanges[z].line << endl;
    }
  }

  if (!m_view->dynWordWrap())
  {
    int y = lineToY( start.line() );
    // FIXME is this enough for when multiple lines are deleted
    int h = (end.line() - start.line() + 2) * m_view->renderer()->fontHeight();
    if (end.line() == (int)m_doc->numVisLines() - 1)
      h = height();

    leftBorder->update (0, y, leftBorder->width(), h);
  }
  else
  {
    // FIXME Do we get enough good info in editRemoveText to optimise this more?
    //bool justTagged = false;
    for (uint z = 0; z < lineRanges.size(); z++)
    {
      if ((lineRanges[z].virtualLine > start.line() || (lineRanges[z].virtualLine == start.line() && lineRanges[z].endCol >= start.col() && start.col() != -1)) && (lineRanges[z].virtualLine < end.line() || (lineRanges[z].virtualLine == end.line() && (lineRanges[z].startCol <= end.col() || end.col() == -1))))
      {
        //justTagged = true;
        leftBorder->update (0, z * m_view->renderer()->fontHeight(), leftBorder->width(), leftBorder->height());
        break;
      }
      /*else if (justTagged)
      {
        justTagged = false;
        leftBorder->update (0, z * m_doc->viewFont.fontHeight, leftBorder->width(), m_doc->viewFont.fontHeight);
        break;
      }*/
    }
  }

  return ret;
}

void KateViewInternal::tagAll()
{
  //kdDebug(13030) << "tagAll()" << endl;
  for (uint z = 0; z < lineRanges.size(); z++)
  {
      lineRanges[z].dirty = true;
  }

  leftBorder->updateFont();
  leftBorder->update ();
}

void KateViewInternal::paintCursor()
{
  if (tagLine(displayCursor))
    paintText (0,0,width(), height(), true);
}

// Point in content coordinates
void KateViewInternal::placeCursor( const TQPoint& p, bool keepSelection, bool updateSelection )
{
  KateLineRange thisRange = yToKateLineRange(p.y());

  if (thisRange.line == -1) {
    for (int i = (p.y() / m_view->renderer()->fontHeight()); i >= 0; i--) {
      thisRange = lineRanges[i];
      if (thisRange.line != -1)
        break;
    }
    Q_ASSERT(thisRange.line != -1);
  }

  int realLine = thisRange.line;
  int visibleLine = thisRange.virtualLine;
  uint startCol = thisRange.startCol;

  visibleLine = kMax( 0, kMin( visibleLine, int(m_doc->numVisLines()) - 1 ) );

  KateTextCursor c(realLine, 0);

  int x = kMin(kMax(-m_startX, p.x() - thisRange.xOffset()), lineMaxCursorX(thisRange) - thisRange.startX);

  m_view->renderer()->textWidth( c, startX() + x, startCol);

  if (updateSelection)
    KateViewInternal::updateSelection( c, keepSelection );

  updateCursor( c );
}

// Point in content coordinates
bool KateViewInternal::isTargetSelected( const TQPoint& p )
{
  KateLineRange thisRange = yToKateLineRange(p.y());

  KateTextLine::Ptr l = textLine( thisRange.line );
  if( !l )
    return false;

  int col = m_view->renderer()->textPos( l, startX() + p.x() - thisRange.xOffset(), thisRange.startCol, false );

  return m_view->lineColSelected( thisRange.line, col );
}

//BEGIN EVENT HANDLING STUFF

bool KateViewInternal::eventFilter( TQObject *obj, TQEvent *e )
{
  if (obj == m_lineScroll)
  {
    // the second condition is to make sure a scroll on the vertical bar doesn't cause a horizontal scroll ;)
    if (e->type() == TQEvent::Wheel && m_lineScroll->minValue() != m_lineScroll->maxValue())
    {
      wheelEvent((TQWheelEvent*)e);
      return true;
    }

    // continue processing
    return TQWidget::eventFilter( obj, e );
  }

  switch( e->type() )
  {
    case TQEvent::KeyPress:
    {
      TQKeyEvent *k = (TQKeyEvent *)e;

      if (m_view->m_codeCompletion->codeCompletionVisible ())
      {
        kdDebug (13030) << "hint around" << endl;

        if( k->key() == Key_Escape )
          m_view->m_codeCompletion->abortCompletion();
      }

      if ((k->key() == TQt::Key_Escape) && !m_view->config()->persistentSelection() )
      {
        m_view->clearSelection();
        return true;
      }
      else if ( !((k->state() & ControlButton) || (k->state() & AltButton)) )
      {
        keyPressEvent( k );
        return k->isAccepted();
      }

    } break;

    case TQEvent::DragMove:
    {
      TQPoint currentPoint = ((TQDragMoveEvent*) e)->pos();

      TQRect doNotScrollRegion( scrollMargin, scrollMargin,
                          width() - scrollMargin * 2,
                          height() - scrollMargin * 2 );

      if ( !doNotScrollRegion.contains( currentPoint ) )
      {
          startDragScroll();
          // Keep sending move events
          ( (TQDragMoveEvent*)e )->accept( TQRect(0,0,0,0) );
      }

      dragMoveEvent((TQDragMoveEvent*)e);
    } break;

    case TQEvent::DragLeave:
      // happens only when pressing ESC while dragging
      stopDragScroll();
      break;

    case TQEvent::WindowBlocked:
      // next focus originates from an internal dialog:
      // don't show the modonhd prompt
      m_doc->m_isasking = -1;
      break;

    default:
      break;
  }

  return TQWidget::eventFilter( obj, e );
}

void KateViewInternal::keyPressEvent( TQKeyEvent* e )
{
  KKey key(e);

  bool codeComp = m_view->m_codeCompletion->codeCompletionVisible ();

  if (codeComp)
  {
    kdDebug (13030) << "hint around" << endl;

    if( e->key() == Key_Enter || e->key() == Key_Return  ||
    (key == SHIFT + TQt::Key_Return) || (key == SHIFT + TQt::Key_Enter)) {
      m_view->m_codeCompletion->doComplete();
      e->accept();
      return;
    }
  }

  if( !m_doc->isReadWrite() )
  {
    e->ignore();
    return;
  }

  if ((key == TQt::Key_Return) || (key == TQt::Key_Enter))
  {
    m_view->keyReturn();
    e->accept();
    return;
  }

  if ((key == SHIFT + TQt::Key_Return) || (key == SHIFT + TQt::Key_Enter))
  {
    uint ln = cursor.line();
    int col = cursor.col();
    KateTextLine::Ptr line = m_doc->kateTextLine( ln );
    int pos = line->firstChar();
    if (pos > cursor.col()) pos = cursor.col();
    if (pos != -1) {
      while ((int)line->length() > pos &&
             !line->getChar(pos).isLetterOrNumber() &&
             pos < cursor.col()) ++pos;
    } else {
      pos = line->length(); // stay indented
    }
    m_doc->editStart();
    m_doc->insertText( cursor.line(), line->length(), "\n" +  line->string(0, pos)
      + line->string().right( line->length() - cursor.col() ) );
    cursor.setPos(ln + 1, pos);
    if (col < int(line->length()))
      m_doc->editRemoveText(ln, col, line->length() - col);
    m_doc->editEnd();
    updateCursor(cursor, true);
    updateView();
    e->accept();

    return;
  }

  if (key == TQt::Key_Backspace || key == SHIFT + TQt::Key_Backspace)
  {
    m_view->backspace();
    e->accept();

    if (codeComp)
      m_view->m_codeCompletion->updateBox ();

    return;
  }

  if  (key == TQt::Key_Tab || key == SHIFT+TQt::Key_Backtab || key == TQt::Key_Backtab)
  {
    if (m_doc->invokeTabInterceptor(key)) {
      e->accept();
      return;
    } else
    if (m_doc->configFlags() & KateDocumentConfig::cfTabIndents)
    {
      if( key == TQt::Key_Tab )
      {
        if (m_view->hasSelection() || (m_doc->configFlags() & KateDocumentConfig::cfTabIndentsMode))
          m_doc->indent( m_view, cursor.line(), 1 );
        else if (m_doc->configFlags() & KateDocumentConfig::cfTabInsertsTab)
          m_doc->typeChars ( m_view, TQString ("\t") );
        else
          m_doc->insertIndentChars ( m_view );

        e->accept();

        if (codeComp)
          m_view->m_codeCompletion->updateBox ();

        return;
      }

      if (key == SHIFT+TQt::Key_Backtab || key == TQt::Key_Backtab)
      {
        m_doc->indent( m_view, cursor.line(), -1 );
        e->accept();

        if (codeComp)
          m_view->m_codeCompletion->updateBox ();

        return;
      }
    }
}
  if ( !(e->state() & ControlButton) && !(e->state() & AltButton)
       && m_doc->typeChars ( m_view, e->text() ) )
  {
    e->accept();

    if (codeComp)
      m_view->m_codeCompletion->updateBox ();

    return;
  }

  e->ignore();
}

void KateViewInternal::keyReleaseEvent( TQKeyEvent* e )
{
  KKey key(e);

  if (key == SHIFT)
    m_shiftKeyPressed = true;
  else
  {
    if (m_shiftKeyPressed)
    {
      m_shiftKeyPressed = false;

      if (m_selChangedByUser)
      {
        TQApplication::clipboard()->setSelectionMode( true );
        m_view->copy();
        TQApplication::clipboard()->setSelectionMode( false );

        m_selChangedByUser = false;
      }
    }
  }

  e->ignore();
  return;
}

void KateViewInternal::contextMenuEvent ( TQContextMenuEvent * e )
{
  // try to show popup menu

  TQPoint p = e->pos();

  if ( m_view->m_doc->browserView() )
  {
    m_view->contextMenuEvent( e );
    return;
  }

  if ( e->reason() == TQContextMenuEvent::Keyboard )
  {
    makeVisible( cursor, 0 );
    p = cursorCoordinates();
  }
  else if ( ! m_view->hasSelection() || m_view->config()->persistentSelection() )
    placeCursor( e->pos() );

  // popup is a qguardedptr now
  if (m_view->popup()) {
    m_view->popup()->popup( mapToGlobal( p ) );
    e->accept ();
  }
}

void KateViewInternal::mousePressEvent( TQMouseEvent* e )
{
  switch (e->button())
  {
    case TQt::LeftButton:
        m_selChangedByUser = false;

        if (possibleTripleClick)
        {
          possibleTripleClick = false;

          m_selectionMode = Line;

          if ( e->state() & TQt::ShiftButton )
          {
            updateSelection( cursor, true );
          }
          else
          {
            m_view->selectLine( cursor );
          }

          TQApplication::clipboard()->setSelectionMode( true );
          m_view->copy();
          TQApplication::clipboard()->setSelectionMode( false );

          // Keep the line at the select anchor selected during further
          // mouse selection
          if ( selectAnchor.line() > m_view->selectStart.line() )
          {
            // Preserve the last selected line
            if ( selectAnchor == m_view->selectEnd && selectAnchor.col() == 0 )
              selStartCached = KateTextCursor( selectAnchor.line()-1, 0 );
            else
              selStartCached = KateTextCursor( selectAnchor.line(), 0 );
            selEndCached = m_view->selectEnd;
          }
          else
          {
            // Preserve the first selected line
            selStartCached = m_view->selectStart;
            if ( m_view->selectEnd.line() > m_view->selectStart.line() )
              selEndCached = KateTextCursor( m_view->selectStart.line()+1, 0 );
            else
              selEndCached = m_view->selectEnd;
          }

          // Set cursor to edge of selection... which edge depends on what
          // "direction" the selection was made in
          if ( m_view->selectStart < selectAnchor
               && selectAnchor.line() != m_view->selectStart.line() )
            updateCursor( m_view->selectStart );
          else
            updateCursor( m_view->selectEnd );

          e->accept ();
          return;
        }
        else if (m_selectionMode == Default)
        {
          m_selectionMode = Mouse;
        }

        if ( e->state() & TQt::ShiftButton )
        {
          if (selectAnchor.line() < 0)
            selectAnchor = cursor;
        }
        else
        {
          selStartCached.setLine( -1 ); // invalidate
        }

        if( !( e->state() & TQt::ShiftButton ) && isTargetSelected( e->pos() ) )
        {
          dragInfo.state = diPending;
          dragInfo.start = e->pos();
        }
        else
        {
          dragInfo.state = diNone;

          if ( e->state() & TQt::ShiftButton )
          {
            placeCursor( e->pos(), true, false );
            if ( selStartCached.line() >= 0 )
            {
              if ( cursor > selEndCached )
              {
                m_view->setSelection( selStartCached, cursor );
                selectAnchor = selStartCached;
              }
              else if ( cursor < selStartCached )
              {
                m_view->setSelection( cursor, selEndCached );
                selectAnchor = selEndCached;
              }
              else
              {
                m_view->setSelection( selStartCached, cursor );
              }
            }
            else
            {
              m_view->setSelection( selectAnchor, cursor );
            }
          }
          else
          {
            placeCursor( e->pos() );
          }

          scrollX = 0;
          scrollY = 0;

          m_scrollTimer.start (50);
        }

        e->accept ();
        break;

    default:
      e->ignore ();
      break;
  }
}

void KateViewInternal::mouseDoubleClickEvent(TQMouseEvent *e)
{
  switch (e->button())
  {
    case TQt::LeftButton:
      m_selectionMode = Word;

      if ( e->state() & TQt::ShiftButton )
      {
        KateTextCursor oldSelectStart = m_view->selectStart;
        KateTextCursor oldSelectEnd = m_view->selectEnd;

        // Now select the word under the select anchor
        int cs, ce;
        KateTextLine::Ptr l = m_doc->kateTextLine( selectAnchor.line() );

        ce = selectAnchor.col();
        if ( ce > 0 && m_doc->highlight()->isInWord( l->getChar( ce ) ) ) {
          for (; ce < l->length(); ce++ )
            if ( !m_doc->highlight()->isInWord( l->getChar( ce ) ) )
              break;
        }

        cs = selectAnchor.col() - 1;
        if ( cs < m_doc->textLine( selectAnchor.line() ).length()
              && m_doc->highlight()->isInWord( l->getChar( cs ) ) ) {
          for ( cs--; cs >= 0; cs-- )
            if ( !m_doc->highlight()->isInWord( l->getChar( cs ) ) )
              break;
        }

        // ...and keep it selected
        if (cs+1 < ce)
        {
          selStartCached = KateTextCursor( selectAnchor.line(), cs+1 );
          selEndCached = KateTextCursor( selectAnchor.line(), ce );
        }
        else
        {
          selStartCached = selectAnchor;
          selEndCached = selectAnchor;
        }
        // Now word select to the mouse cursor
        placeCursor( e->pos(), true );
      }
      else
      {
        // first clear the selection, otherwise we run into bug #106402
        // ...and set the cursor position, for the same reason (otherwise there
        // are *other* idiosyncrasies we can't fix without reintroducing said
        // bug)
        // Parameters: 1st false: don't redraw
        //             2nd false: don't emit selectionChanged signals, as
        //             selectWord() emits this already
        m_view->clearSelection( false, false );
        placeCursor( e->pos() );
        m_view->selectWord( cursor );
        if (m_view->hasSelection())
        {
          selectAnchor = selStartCached = m_view->selectStart;
          selEndCached = m_view->selectEnd;
        }
        else
        {
          // if we didn't actually select anything, restore the selection mode
          // -- see bug #131369 (kling)
          m_selectionMode = Default;
        }
      }

      // Move cursor to end (or beginning) of selected word
      if (m_view->hasSelection())
      {
        TQApplication::clipboard()->setSelectionMode( true );
        m_view->copy();
        TQApplication::clipboard()->setSelectionMode( false );

        // Shift+DC before the "cached" word should move the cursor to the
        // beginning of the selection, not the end
        if (m_view->selectStart < selStartCached)
          updateCursor( m_view->selectStart );
        else
          updateCursor( m_view->selectEnd );
      }

      possibleTripleClick = true;
      TQTimer::singleShot ( TQApplication::doubleClickInterval(), this, TQ_SLOT(tripleClickTimeout()) );

      scrollX = 0;
      scrollY = 0;

      m_scrollTimer.start (50);

      e->accept ();
      break;

    default:
      e->ignore ();
      break;
  }
}

void KateViewInternal::tripleClickTimeout()
{
  possibleTripleClick = false;
}

void KateViewInternal::mouseReleaseEvent( TQMouseEvent* e )
{
  switch (e->button())
  {
    case TQt::LeftButton:
      m_selectionMode = Default;
//       selStartCached.setLine( -1 );

      if (m_selChangedByUser)
      {
        TQApplication::clipboard()->setSelectionMode( true );
        m_view->copy();
        TQApplication::clipboard()->setSelectionMode( false );
        // Set cursor to edge of selection... which edge depends on what
        // "direction" the selection was made in
        if ( m_view->selectStart < selectAnchor )
          updateCursor( m_view->selectStart );
        else
          updateCursor( m_view->selectEnd );

        m_selChangedByUser = false;
      }

      if (dragInfo.state == diPending)
        placeCursor( e->pos(), e->state() & ShiftButton );
      else if (dragInfo.state == diNone)
        m_scrollTimer.stop ();

      dragInfo.state = diNone;

      e->accept ();
      break;

    case TQt::MidButton:
      placeCursor( e->pos() );

      if( m_doc->isReadWrite() )
      {
        TQApplication::clipboard()->setSelectionMode( true );
        m_view->paste ();
        TQApplication::clipboard()->setSelectionMode( false );
      }

      e->accept ();
      break;

    default:
      e->ignore ();
      break;
  }
}

void KateViewInternal::mouseMoveEvent( TQMouseEvent* e )
{
  if( e->state() & TQt::LeftButton )
  {
    if (dragInfo.state == diPending)
    {
      // we had a mouse down, but haven't confirmed a drag yet
      // if the mouse has moved sufficiently, we will confirm
      TQPoint p( e->pos() - dragInfo.start );

      // we've left the drag square, we can start a real drag operation now
      if( p.manhattanLength() > TDEGlobalSettings::dndEventDelay() )
        doDrag();

      return;
    }
    else if (dragInfo.state == diDragging)
    {
      // Don't do anything after a canceled drag until the user lets go of
      // the mouse button!
      return;
    }

    mouseX = e->x();
    mouseY = e->y();

    scrollX = 0;
    scrollY = 0;
    int d = m_view->renderer()->fontHeight();

    if (mouseX < 0)
      scrollX = -d;

    if (mouseX > width())
      scrollX = d;

    if (mouseY < 0)
    {
      mouseY = 0;
      scrollY = -d;
    }

    if (mouseY > height())
    {
      mouseY = height();
      scrollY = d;
    }

    placeCursor( TQPoint( mouseX, mouseY ), true );

  }
  else
  {
    if (isTargetSelected( e->pos() ) ) {
      // mouse is over selected text. indicate that the text is draggable by setting
      // the arrow cursor as other Qt text editing widgets do
      if (m_mouseCursor != ArrowCursor) {
        setCursor( KCursor::arrowCursor() );
        m_mouseCursor = TQt::ArrowCursor;
      }
    } else {
      // normal text cursor
      if (m_mouseCursor != IbeamCursor) {
        setCursor( KCursor::ibeamCursor() );
        m_mouseCursor = TQt::IbeamCursor;
      }
    }

    if (m_textHintEnabled)
    {
       m_textHintTimer.start(m_textHintTimeout);
       m_textHintMouseX=e->x();
       m_textHintMouseY=e->y();
    }
  }
}

void KateViewInternal::paintEvent(TQPaintEvent *e)
{
  paintText(e->rect().x(), e->rect().y(), e->rect().width(), e->rect().height());
}

void KateViewInternal::resizeEvent(TQResizeEvent* e)
{
  bool expandedHorizontally = width() > e->oldSize().width();
  bool expandedVertically = height() > e->oldSize().height();
  bool heightChanged = height() != e->oldSize().height();

  m_madeVisible = false;

  if (heightChanged) {
    setAutoCenterLines(m_autoCenterLines, false);
    m_cachedMaxStartPos.setPos(-1, -1);
  }

  if (m_view->dynWordWrap()) {
    bool dirtied = false;

    for (uint i = 0; i < lineRanges.count(); i++) {
      // find the first dirty line
      // the word wrap updateView algorithm is forced to check all lines after a dirty one
      if (lineRanges[i].wrap ||
         (!expandedHorizontally && (lineRanges[i].endX - lineRanges[i].startX) > width())) {
        dirtied = lineRanges[i].dirty = true;
        break;
      }
    }

    if (dirtied || heightChanged) {
      updateView(true);
      leftBorder->update();
    }

    if (width() < e->oldSize().width()) {
      if (!m_view->wrapCursor()) {
        // May have to restrain cursor to new smaller width...
        if (cursor.col() > m_doc->lineLength(cursor.line())) {
          KateLineRange thisRange = currentRange();

          KateTextCursor newCursor(cursor.line(), thisRange.endCol + ((width() - thisRange.xOffset() - (thisRange.endX - thisRange.startX)) / m_view->renderer()->spaceWidth()) - 1);
          updateCursor(newCursor);
        }
      }
    }

  } else {
    updateView();

    if (expandedHorizontally && startX() > 0)
      scrollColumns(startX() - (width() - e->oldSize().width()));
  }

  if (expandedVertically) {
    KateTextCursor max = maxStartPos();
    if (startPos() > max)
      scrollPos(max);
  }
}

void KateViewInternal::scrollTimeout ()
{
  if (scrollX || scrollY)
  {
    scrollLines (startPos().line() + (scrollY / (int)m_view->renderer()->fontHeight()));
    placeCursor( TQPoint( mouseX, mouseY ), true );
  }
}

void KateViewInternal::cursorTimeout ()
{
  m_view->renderer()->setDrawCaret(!m_view->renderer()->drawCaret());
  paintCursor();
}

void KateViewInternal::textHintTimeout ()
{
  m_textHintTimer.stop ();

  KateLineRange thisRange = yToKateLineRange(m_textHintMouseY);

  if (thisRange.line == -1) return;

  if (m_textHintMouseX> (lineMaxCursorX(thisRange) - thisRange.startX)) return;

  int realLine = thisRange.line;
  int startCol = thisRange.startCol;

  KateTextCursor c(realLine, 0);
  m_view->renderer()->textWidth( c, startX() + m_textHintMouseX, startCol);

  TQString tmp;

  emit m_view->needTextHint(c.line(), c.col(), tmp);

  if (!tmp.isEmpty()) kdDebug(13030)<<"Hint text: "<<tmp<<endl;
}

void KateViewInternal::focusInEvent (TQFocusEvent *)
{
  if (TDEApplication::cursorFlashTime() > 0)
    m_cursorTimer.start ( TDEApplication::cursorFlashTime() / 2 );

  if (m_textHintEnabled)
    m_textHintTimer.start( m_textHintTimeout );

  paintCursor();

  m_doc->setActiveView( m_view );

  emit m_view->gotFocus( m_view );
}

void KateViewInternal::focusOutEvent (TQFocusEvent *)
{
  if( m_view->renderer() && ! m_view->m_codeCompletion->codeCompletionVisible() )
  {
    m_cursorTimer.stop();

    m_view->renderer()->setDrawCaret(true);
    paintCursor();
    emit m_view->lostFocus( m_view );
  }

  m_textHintTimer.stop();
}

void KateViewInternal::doDrag()
{
  dragInfo.state = diDragging;
  dragInfo.dragObject = new TQTextDrag(m_view->selection(), this);
  dragInfo.dragObject->drag();
}

void KateViewInternal::dragEnterEvent( TQDragEnterEvent* event )
{
  event->accept( (TQTextDrag::canDecode(event) && m_doc->isReadWrite()) ||
                  KURLDrag::canDecode(event) );
}

void KateViewInternal::dragMoveEvent( TQDragMoveEvent* event )
{
  // track the cursor to the current drop location
  placeCursor( event->pos(), true, false );

  // important: accept action to switch between copy and move mode
  // without this, the text will always be copied.
  event->acceptAction();
}

void KateViewInternal::dropEvent( TQDropEvent* event )
{
  if ( KURLDrag::canDecode(event) ) {

      emit dropEventPass(event);

  } else if ( TQTextDrag::canDecode(event) && m_doc->isReadWrite() ) {

    TQString text;

    if (!TQTextDrag::decode(event, text))
      return;

    // is the source our own document?
    bool priv = false;
    if (event->source() && event->source()->inherits("KateViewInternal"))
      priv = m_doc->ownedView( ((KateViewInternal*)(event->source()))->m_view );

    // dropped on a text selection area?
    bool selected = isTargetSelected( event->pos() );

    if( priv && selected ) {
      // this is a drag that we started and dropped on our selection
      // ignore this case
      return;
    }

    // use one transaction
    m_doc->editStart ();

    // on move: remove selected text; on copy: duplicate text
    if ( event->action() != TQDropEvent::Copy )
      m_view->removeSelectedText();

    m_doc->insertText( cursor.line(), cursor.col(), text );

    m_doc->editEnd ();

    placeCursor( event->pos() );

    event->acceptAction();
    updateView();
  }

  // finally finish drag and drop mode
  dragInfo.state = diNone;
  // important, because the eventFilter`s DragLeave does not occur
  stopDragScroll();
}
//END EVENT HANDLING STUFF

void KateViewInternal::clear()
{
  cursor.setPos(0, 0);
  displayCursor.setPos(0, 0);
}

void KateViewInternal::wheelEvent(TQWheelEvent* e)
{
	if (e->state() & ControlButton)
	{
		if (e->delta() > 0)
		{
			slotIncFontSizes();
		}
		else
		{
			slotDecFontSizes();
		}
	}
	else
	{
		if (m_lineScroll->minValue() != m_lineScroll->maxValue() && e->orientation() != TQt::Horizontal)
		{
			// React to this as a vertical event
			if ( e->state() & ShiftButton )
			{
				if (e->delta() > 0)
					scrollPrevPage();
				else
					scrollNextPage();
			}
			else
			{
				scrollViewLines(-((e->delta() / 120) * TQApplication::wheelScrollLines()));
				// maybe a menu was opened or a bubbled window title is on us -> we shall erase it
				update();
				leftBorder->update();
			}
		} else if (columnScrollingPossible()) {
			TQWheelEvent copy = *e;
			TQApplication::sendEvent(m_columnScroll, &copy);

		} else {
			e->ignore();
		}
	}
}

void KateViewInternal::startDragScroll()
{
  if ( !m_dragScrollTimer.isActive() ) {
    m_dragScrollTimer.start( scrollTime );
  }
}

void KateViewInternal::stopDragScroll()
{
  m_dragScrollTimer.stop();
  updateView();
}

void KateViewInternal::doDragScroll()
{
  TQPoint p = this->mapFromGlobal( TQCursor::pos() );

  int dx = 0, dy = 0;
  if ( p.y() < scrollMargin ) {
    dy = p.y() - scrollMargin;
  } else if ( p.y() > height() - scrollMargin ) {
    dy = scrollMargin - (height() - p.y());
  }

  if ( p.x() < scrollMargin ) {
    dx = p.x() - scrollMargin;
  } else if ( p.x() > width() - scrollMargin ) {
    dx = scrollMargin - (width() - p.x());
  }

  dy /= 4;

  if (dy)
    scrollLines(startPos().line() + dy);

  if (columnScrollingPossible () && dx)
    scrollColumns(kMin (m_startX + dx, m_columnScroll->maxValue()));

  if (!dy && !dx)
    stopDragScroll();
}

void KateViewInternal::enableTextHints(int timeout)
{
  m_textHintTimeout=timeout;
  m_textHintEnabled=true;
  m_textHintTimer.start(timeout);
}

void KateViewInternal::disableTextHints()
{
  m_textHintEnabled=false;
  m_textHintTimer.stop ();
}

bool KateViewInternal::columnScrollingPossible ()
{
  return !m_view->dynWordWrap() && m_columnScroll->isEnabled() && (m_columnScroll->maxValue() > 0);
}

//BEGIN EDIT STUFF
void KateViewInternal::editStart()
{
  editSessionNumber++;

  if (editSessionNumber > 1)
    return;

  editIsRunning = true;
  editOldCursor = cursor;
}

void KateViewInternal::editEnd(int editTagLineStart, int editTagLineEnd, bool tagFrom)
{
   if (editSessionNumber == 0)
    return;

  editSessionNumber--;

  if (editSessionNumber > 0)
    return;

  if (tagFrom && (editTagLineStart <= int(m_doc->getRealLine(startLine()))))
    tagAll();
  else
    tagLines (editTagLineStart, tagFrom ? m_doc->lastLine() : editTagLineEnd, true);

  if (editOldCursor == cursor)
    updateBracketMarks();

  if (m_imPreeditLength <= 0)
    updateView(true);

  if ((editOldCursor != cursor) && (m_imPreeditLength <= 0))
  {
    m_madeVisible = false;
    updateCursor ( cursor, true );
  }
  else if ( m_view == m_doc->activeView() )
  {
    makeVisible(displayCursor, displayCursor.col());
  }

  editIsRunning = false;
}

void KateViewInternal::editSetCursor (const KateTextCursor &cursor)
{
  if (this->cursor != cursor)
  {
    this->cursor.setPos (cursor);
  }
}
//END

void KateViewInternal::viewSelectionChanged ()
{
  if (!m_view->hasSelection())
  {
    selectAnchor.setPos (-1, -1);
    selStartCached.setPos (-1, -1);
  }
}

//BEGIN IM INPUT STUFF
void KateViewInternal::imStartEvent( TQIMEvent *e )
{
  if ( m_doc->m_bReadOnly ) {
    e->ignore();
    return;
  }

  if ( m_view->hasSelection() )
    m_view->removeSelectedText();

  m_imPreeditStartLine = cursor.line();
  m_imPreeditStart = cursor.col();
  m_imPreeditLength = 0;
  m_imPreeditSelStart = m_imPreeditStart;

  m_view->setIMSelectionValue( m_imPreeditStartLine, m_imPreeditStart, 0, 0, 0, true );
}

void KateViewInternal::imComposeEvent( TQIMEvent *e )
{
  if ( m_doc->m_bReadOnly ) {
    e->ignore();
    return;
  }

  // remove old preedit
  if ( m_imPreeditLength > 0 ) {
    cursor.setPos( m_imPreeditStartLine, m_imPreeditStart );
    m_doc->removeText( m_imPreeditStartLine, m_imPreeditStart,
                       m_imPreeditStartLine, m_imPreeditStart + m_imPreeditLength );
  }

  m_imPreeditLength = e->text().length();
  m_imPreeditSelStart = m_imPreeditStart + e->cursorPos();

  // update selection
  m_view->setIMSelectionValue( m_imPreeditStartLine, m_imPreeditStart, m_imPreeditStart + m_imPreeditLength,
                              m_imPreeditSelStart, m_imPreeditSelStart + e->selectionLength(),
                              true );

  // insert new preedit
  m_doc->insertText( m_imPreeditStartLine, m_imPreeditStart, e->text() );


  // update cursor
  cursor.setPos( m_imPreeditStartLine, m_imPreeditSelStart );
  updateCursor( cursor, true );

  updateView( true );
}

void KateViewInternal::imEndEvent( TQIMEvent *e )
{
  if ( m_doc->m_bReadOnly ) {
    e->ignore();
    return;
  }

  if ( m_imPreeditLength > 0 ) {
    cursor.setPos( m_imPreeditStartLine, m_imPreeditStart );
    m_doc->removeText( m_imPreeditStartLine, m_imPreeditStart,
                       m_imPreeditStartLine, m_imPreeditStart + m_imPreeditLength );
  }

  m_view->setIMSelectionValue( m_imPreeditStartLine, m_imPreeditStart, 0, 0, 0, false );

  if ( e->text().length() > 0 ) {
    m_doc->insertText( cursor.line(), cursor.col(), e->text() );

    if ( !m_cursorTimer.isActive() && TDEApplication::cursorFlashTime() > 0 )
      m_cursorTimer.start ( TDEApplication::cursorFlashTime() / 2 );

    updateView( true );
    updateCursor( cursor, true );
  }

  m_imPreeditStart = 0;
  m_imPreeditLength = 0;
  m_imPreeditSelStart = 0;
}
//END IM INPUT STUFF
