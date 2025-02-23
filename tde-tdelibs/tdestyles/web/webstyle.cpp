/*
 *  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDE_MENUITEM_DEF
#define INCLUDE_MENUITEM_DEF
#endif

#include <tqmenudata.h>
#include <tqpalette.h>
#include <tqbitmap.h>
#include <tqtabbar.h>
#include <tqpointarray.h>
#include <tqscrollbar.h>
#include <tqframe.h>
#include <tqpushbutton.h>
#include <tqdrawutil.h>
#include <tqpainter.h>

#include <tdeapplication.h>
#include <kdrawutil.h>
#include <tdelocale.h>
#include <kiconloader.h>
#include <kdebug.h>

#include "webstyle.h"

static const int  _indicatorSize = 13;
static TQButton *  _highlightedButton = 0;
static const int  _scrollBarExtent = 14;

static TQFrame *   _currentFrame = 0;
static int        _savedFrameLineWidth;
static int        _savedFrameMidLineWidth;
static ulong      _savedFrameStyle;

static TQColor contrastingForeground(const TQColor & fg, const TQColor & bg)
{
  int h, s, vbg, vfg;

  bg.hsv(&h, &s, &vbg);
  fg.hsv(&h, &s, &vfg);

  int diff(vbg - vfg);

  if ((diff > -72) && (diff < 72))
  {
    return (vbg < 128) ? TQt::white : TQt::black;
  }
  else
  {
    return fg;
  }
}

// Gotta keep it separated.

  static void
scrollBarControlsMetrics
(
 const TQScrollBar * sb,
 int sliderStart,
 int /* sliderMin */,
 int sliderMax,
 int sliderLength,
 int buttonDim,
 TQRect & rSub,
 TQRect & rAdd,
 TQRect & rSubPage,
 TQRect & rAddPage,
 TQRect & rSlider
 )
{
  bool horizontal = sb->orientation() == TQScrollBar::Horizontal;

  int len     = horizontal ? sb->width()  : sb->height();

  int extent  = horizontal ? sb->height() : sb->width();

  TQColorGroup g = sb->colorGroup();

  if (sliderStart > sliderMax)
    sliderStart = sliderMax;

  int sliderEnd = sliderStart + sliderLength;

  int addX, addY;
  int subX, subY;
  int subPageX, subPageY, subPageW, subPageH;
  int addPageX, addPageY, addPageW, addPageH;
  int sliderX, sliderY, sliderW, sliderH;

  if (horizontal)
  {
    subY      = 0;
    addY      = 0;
    subX      = 0;
    addX      = buttonDim;

    subPageX  = buttonDim * 2;
    subPageY  = 0;
    subPageW  = sliderStart - 1;
    subPageH  = extent;

    addPageX  = sliderEnd;
    addPageY  = 0;
    addPageW  = len - sliderEnd;
    addPageH  = extent;

    sliderX   = sliderStart;
    sliderY   = 0;
    sliderW   = sliderLength;
    sliderH   = extent;
  }
  else
  {
    subX    = 0;
    addX    = 0;
    subY    = len - buttonDim * 2;
    addY    = len - buttonDim;

    subPageX = 0;
    subPageY = 0;
    subPageW = extent;
    subPageH = sliderStart;

    addPageX  = 0;
    addPageY  = sliderEnd;
    addPageW  = extent;
    addPageH  = subY - sliderEnd;

    sliderX   = 0;
    sliderY   = sliderStart;
    sliderW   = extent;
    sliderH   = sliderLength;
  }

  rSub      .setRect(    subX,      subY, buttonDim, buttonDim);
  rAdd      .setRect(    addX,      addY, buttonDim, buttonDim);
  rSubPage  .setRect(subPageX,  subPageY,  subPageW,  subPageH);
  rAddPage  .setRect(addPageX,  addPageY,  addPageW,  addPageH);
  rSlider   .setRect( sliderX,   sliderY,   sliderW,   sliderH);
}

// Rounded rects my way.

  static void
drawFunkyRect
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 bool small
)
{
  p->translate(x, y);

  if (small)
  {
    p->drawLine(      2,      0,  w - 3,      0  );
    p->drawLine(  w - 1,      2,  w - 1,  h - 3  );
    p->drawLine(  w - 3,  h - 1,      2,  h - 1  );
    p->drawLine(      0,  h - 3,      0,      2  );

    // Use an array of points so that there's only one round-trip with the
    // X server.

    QCOORD pointList[] =
    {
          1,      1,
      w - 2,      1,
      w - 2,  h - 2,
          1,  h - 2
    };

    p->drawPoints(TQPointArray(4, pointList));
  }
  else
  {
    p->drawLine(      3,      0,  w - 4,      0  );
    p->drawLine(  w - 1,      3,  w - 1,  h - 4  );
    p->drawLine(  w - 4,  h - 1,      3,  h - 1  );
    p->drawLine(      0,  h - 4,      0,      3  );

    QCOORD pointList[] =
    {
          1,      2,
          2,      1,
      w - 3,      1,
      w - 2,      2,
      w - 2,  h - 3,
      w - 3,  h - 2,
          2,  h - 2,
          1,  h - 3
    };

    p->drawPoints(TQPointArray(8, pointList));
  }

  p->translate(-x, -y);
}

WebStyle::WebStyle()
  : TDEStyle()
{
  setButtonDefaultIndicatorWidth(6);
  setScrollBarExtent(_scrollBarExtent, _scrollBarExtent);
}

WebStyle::~WebStyle()
{
  // Empty.
}

  void
WebStyle::polish(TQApplication *)
{
  // Empty.
}

  void
WebStyle::polish(TQPalette &)
{
  // Empty.
}

  void
WebStyle::unPolish(TQApplication *)
{
  // Empty.
}

  void
WebStyle::polish(TQWidget * w)
{
  if (w->inherits("TQPushButton"))
    w->installEventFilter(this);

  else if (w->inherits("TQGroupBox") || w->inherits("TQFrame"))
  {
    TQFrame * f(static_cast<TQFrame *>(w));

    if (f->frameStyle() != TQFrame::NoFrame)
    {
      _currentFrame = f;

      _savedFrameLineWidth = f->lineWidth();
      _savedFrameMidLineWidth = f->midLineWidth();
      _savedFrameStyle = f->frameStyle();

      if (f->frameShape() == TQFrame::HLine || f->frameShape() == TQFrame::VLine)
      {
        f->setMidLineWidth(1);
        f->setFrameStyle(f->frameShape() | TQFrame::Plain);
      }
      else
      {
        f->setLineWidth(1);
        f->setFrameStyle(TQFrame::Box | TQFrame::Plain);
      }
    }
  }
}

  void
WebStyle::unPolish(TQWidget * w)
{
  if (w->inherits("TQPushButton"))
    w->removeEventFilter(this);

  else if (w == _currentFrame)
  {
    TQFrame * f(static_cast<TQFrame *>(w));

    f->setLineWidth(_savedFrameLineWidth);
    f->setMidLineWidth(_savedFrameMidLineWidth);
    f->setFrameStyle(_savedFrameStyle);
  }
}

  bool
WebStyle::eventFilter(TQObject * o, TQEvent * e)
{
  TQPushButton * pb(static_cast<TQPushButton *>(o));

  if (e->type() == TQEvent::Enter)
  {
    _highlightedButton = pb;
    pb->repaint(false);
  }
  else if (e->type() == TQEvent::Leave)
  {
    _highlightedButton = 0;
    pb->repaint(false);
  }

  return false;
}

  void
WebStyle::drawButton
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 bool sunken,
 const TQBrush * fill
)
{
  p->save();

  if (sunken)
    p->setPen(contrastingForeground(g.light(), g.button()));
  else
    p->setPen(contrastingForeground(g.mid(), g.button()));

  p->setBrush(0 == fill ? NoBrush : *fill);

  drawFunkyRect(p, x, y, w, h, true);

  p->restore();
}

  QRect
WebStyle::buttonRect(int x, int y, int w, int h)
{
  return TQRect(x + 2, y + 2, w - 4, h - 4);
}

  void
WebStyle::drawBevelButton
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 bool sunken,
 const TQBrush * fill
)
{
  drawButton(p, x, y, w, h, g, sunken, fill);
}

  void
WebStyle::drawPushButton(TQPushButton * b, TQPainter * p)
{
  // Note: painter is already translated for us.

  bool sunken(b->isDown() || b->isOn());
  bool hl(_highlightedButton == b);

  TQColor bg(b->colorGroup().button());

  p->save();
  p->fillRect(b->rect(), b->colorGroup().brush(TQColorGroup::Background));

  if (b->isDefault())
  {
    TQColor c(hl ? b->colorGroup().highlight() : b->colorGroup().mid());

    p->setPen(contrastingForeground(c, bg));

    drawFunkyRect(p, 0, 0, b->width(), b->height(), false);
  }

  p->fillRect
    (
     4,
     4,
     b->width() - 8,
     b->height() - 8,
     b->colorGroup().brush(TQColorGroup::Button)
    );

  if (b->isEnabled())
  {
    if (sunken)
    {
      p->setPen(contrastingForeground(b->colorGroup().light(), bg));
    }
    else
    {
      if (hl)
        p->setPen(contrastingForeground(b->colorGroup().highlight(), bg));
      else
        p->setPen(contrastingForeground(b->colorGroup().mid(), bg));
    }
  }
  else
  {
    p->setPen(b->colorGroup().button());
  }

  drawFunkyRect(p, 3, 3, b->width() - 6, b->height() - 6, true);

  p->restore();
}

  void
WebStyle::drawPushButtonLabel(TQPushButton * b, TQPainter * p)
{
  // This is complicated stuff and we don't really want to mess with it.

  TDEStyle::drawPushButtonLabel(b, p);
}

  void
WebStyle::drawScrollBarControls
(
 TQPainter * p,
 const TQScrollBar * sb,
 int sliderStart,
 uint controls,
 uint activeControl
)
{
  p->save();

  int sliderMin, sliderMax, sliderLength, buttonDim;

  scrollBarMetrics(sb, sliderMin, sliderMax, sliderLength, buttonDim);

  TQRect rSub, rAdd, rSubPage, rAddPage, rSlider;

  scrollBarControlsMetrics
    (
     sb,
     sliderStart,
     sliderMin,
     sliderMax,
     sliderLength,
     buttonDim,
     rSub,
     rAdd,
     rSubPage,
     rAddPage,
     rSlider
    );

  TQColorGroup g(sb->colorGroup());

  if (controls & AddLine && rAdd.isValid())
  {
    bool active(activeControl & AddLine);

    TQColor c(active ? g.highlight() : g.dark());

    p->setPen(c);
    p->setBrush(g.button());
    p->drawRect(rAdd);

    TQt::ArrowType t =
      sb->orientation() == Horizontal ? TQt::RightArrow : TQt::DownArrow;

    // Is it me or is TDEStyle::drawArrow broken ?

    drawArrow
      (
       p,
       t,
       true, // FIXME - down ?
       rAdd.x(),
       rAdd.y(),
       rAdd.width(),
       rAdd.height(),
       g,
       true // FIXME - enabled ?
      );
  }

  if (controls & SubLine && rSub.isValid())
  {
    bool active(activeControl & SubLine);

    TQColor c(active ? g.highlight() : g.dark());

    p->setPen(c);
    p->setBrush(g.button());
    p->drawRect(rSub);

    TQt::ArrowType t =
      sb->orientation() == Horizontal ? TQt::LeftArrow : TQt::UpArrow;

    drawArrow
      (
       p,
       t,
       true, // FIXME - down ?
       rSub.x(),
       rSub.y(),
       rSub.width(),
       rSub.height(),
       g,
       true // FIXME - enabled ?
      );
  }

  if (controls & SubPage && rSubPage.isValid())
  {
    p->setPen(g.mid());
    p->setBrush(g.base());
    p->drawRect(rSubPage);
  }

  if (controls & AddPage && rAddPage.isValid())
  {
    p->setPen(g.mid());
    p->setBrush(g.base());
    p->drawRect(rAddPage);
  }

  if (controls & Slider && rSlider.isValid())
  {
    p->setPen(activeControl & Slider ? g.highlight() : g.dark());

    p->setBrush(g.button());
    p->drawRect(rSlider);

    p->setBrush(g.light());
    p->setPen(g.dark());

    if (sliderLength > _scrollBarExtent * 2)
    {
      int ellipseSize = 
        Horizontal == sb->orientation()
        ?
        rSlider.height() - 4
        :
        rSlider.width()  - 4
        ;

      TQPoint center(rSlider.center());

      if (Horizontal == sb->orientation())
      {
        p->drawEllipse
          (
           center.x() - ellipseSize / 2, rSlider.y() + 2,
           ellipseSize, ellipseSize
          );
      }
      else
      { 
        p->drawEllipse
          (
           rSlider.x() + 2, center.y() - ellipseSize / 2,
           ellipseSize, ellipseSize
          );
      }
    }
  }

  p->restore();
}

  TQStyle::ScrollControl
WebStyle::scrollBarPointOver
(
 const TQScrollBar * sb,
 int sliderStart,
 const TQPoint & point
)
{
  if (!sb->rect().contains(point))
    return NoScroll;

  int sliderMin, sliderMax, sliderLength, buttonDim;

  scrollBarMetrics(sb, sliderMin, sliderMax, sliderLength, buttonDim);

  if (sb->orientation() == TQScrollBar::Horizontal)
  {
    int x = point.x();

    if (x <= buttonDim)
      return SubLine;

    else if (x <= buttonDim * 2)
      return AddLine;

    else if (x < sliderStart)
      return SubPage;

    else if (x < sliderStart+sliderLength)
      return Slider;

    return AddPage;
  }
  else
  {
    int y = point.y();

    if (y < sliderStart)
      return SubPage;

    else if (y < sliderStart + sliderLength)
      return Slider;

    else if (y < sliderMax + sliderLength)
      return AddPage;

    else if (y < sliderMax + sliderLength + buttonDim)
      return SubLine;

    return AddLine;
  }
}

  void
WebStyle::scrollBarMetrics
(
 const TQScrollBar * sb,
 int & sliderMin,
 int & sliderMax,
 int & sliderLength,
 int & buttonDim
)
{
  int maxlen;

  bool horizontal = sb->orientation() == TQScrollBar::Horizontal;

  int len = (horizontal) ? sb->width() : sb->height();

  int extent = (horizontal) ? sb->height() : sb->width();

  if (len > (extent - 1) * 2)
    buttonDim = extent;
  else
    buttonDim = len / 2 - 1;

  if (horizontal)
    sliderMin = buttonDim * 2;
  else
    sliderMin = 1;

  maxlen = len - buttonDim * 2 - 1;

  sliderLength =
    (sb->pageStep() * maxlen) /
    (sb->maxValue() - sb->minValue() + sb->pageStep());

  if (sliderLength < _scrollBarExtent)
    sliderLength = _scrollBarExtent;

  if (sliderLength > maxlen)
    sliderLength = maxlen;

  sliderMax = sliderMin + maxlen - sliderLength;
}

  QSize
WebStyle::indicatorSize() const
{
  return TQSize(_indicatorSize, _indicatorSize);
}

  void
WebStyle::drawIndicator
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 int state,
 bool down,
 bool enabled
)
{
  p->save();

  p->fillRect(x, y, w, h, g.background());

  if (enabled)
  {
    p->setPen(down ? g.highlight() : contrastingForeground(g.dark(), g.background()));
  }
  else
  {
    g.mid();
  }

  p->drawRect(x, y, w, h);

  if (state != TQButton::Off)
  {
    p->fillRect(x + 2, y + 2, w - 4, h - 4, enabled ? g.highlight() : g.mid());

    if (state == TQButton::NoChange)
    {
      p->fillRect(x + 4, y + 4, w - 8, h - 8, g.background());
    }
  }

  p->restore();
}

  QSize
WebStyle::exclusiveIndicatorSize() const
{
  return TQSize(_indicatorSize, _indicatorSize);
}

  void
WebStyle::drawExclusiveIndicator
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 bool on,
 bool down,
 bool enabled
)
{
  p->save();

  p->fillRect(x, y, w, h, g.background());

  if (enabled)
  {
    p->setPen(down ? g.highlight() : contrastingForeground(g.dark(), g.background()));
  }
  else
  {
    p->setPen(g.mid());
  }

  p->setBrush(g.brush(TQColorGroup::Background));

  // Avoid misshapen ellipses. Qt or X bug ? Who knows...

  if (0 == w % 2)
    --w;

  if (0 == h % 2)
    --h;

  p->drawEllipse(x, y, w, h);

  if (on)
  {
    p->setPen(enabled ? g.highlight() : g.mid());
    p->setBrush(enabled ? g.highlight() : g.mid());
    p->drawEllipse(x + 3, y + 3, w - 6, h - 6);
  }

  p->restore();
}

  void
WebStyle::drawIndicatorMask
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 int /* state */
)
{
  p->fillRect(x, y, w, h, TQt::color1);
}

  void
WebStyle::drawExclusiveIndicatorMask
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 bool /* on */
)
{
  if (0 == w % 2)
    --w;

  if (0 == h % 2)
    --h;

  p->setPen(TQt::color1);
  p->setBrush(TQt::color1);
  p->drawEllipse(x, y, w, h);
}

  void
WebStyle::drawComboButton
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 bool sunken,
 bool editable,
 bool enabled,
 const TQBrush * fill
)
{
  p->save();

  p->setPen(NoPen);
  p->setBrush(0 == fill ? g.brush(TQColorGroup::Background) : *fill);
  p->drawRect(x, y, w, h);

  if (enabled)
  {
    if (sunken)
      p->setPen(contrastingForeground(g.highlight(), g.background()));
    else
      p->setPen(contrastingForeground(g.mid(), g.background()));
  }
  else
  {
    p->setPen(contrastingForeground(g.mid(), g.background()));
  }

  drawFunkyRect(p, x, y, w, h, true);

  p->drawPoint(w - 10, h - 6);
  p->drawPoint(w - 9, h - 6);
  p->drawPoint(w - 8, h - 6);
  p->drawPoint(w - 7, h - 6);
  p->drawPoint(w - 6, h - 6);

  p->drawPoint(w - 9, h - 7);
  p->drawPoint(w - 8, h - 7);
  p->drawPoint(w - 7, h - 7);
  p->drawPoint(w - 6, h - 7);

  p->drawPoint(w - 8, h - 8);
  p->drawPoint(w - 7, h - 8);
  p->drawPoint(w - 6, h - 8);

  p->drawPoint(w - 7, h - 9);
  p->drawPoint(w - 6, h - 9);

  p->drawPoint(w - 6, h - 10);

  if (editable)
    p->fillRect(comboButtonFocusRect(x, y, w, h), TQt::red);

  p->restore();
}

  QRect
WebStyle::comboButtonRect(int x, int y, int w, int h)
{
  return TQRect(x + 2, y + 2, w - 20, h - 4);
}

  QRect
WebStyle::comboButtonFocusRect(int x, int y, int w, int h)
{
  return TQRect(x + 2, y + 2, w - 20, h - 4);
}

  int
WebStyle::sliderLength() const
{
  return 13;
}

  void
WebStyle::drawSliderGroove
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 QCOORD /* c */,
 Orientation o
)
{
  p->save();

  p->setPen(TQPen(g.dark(), 0, TQt::DotLine));

  if( o == TQt::Horizontal )
    p->drawLine(x, y + h / 2, w, y + h / 2);
  else
  if( o == TQt::Vertical )
    p->drawLine(x + w / 2, y, x + w / 2, h);

  p->restore();
}

  void
WebStyle::drawArrow
(
 TQPainter * p,
 TQt::ArrowType type,
 bool down,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 bool enabled,
 const TQBrush * fill
)
{
  TDEStyle::drawArrow(p, type, down, x, y, w, h, g, enabled, fill);
}

  void
WebStyle::drawSlider
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 Orientation o,
 bool /* tickAbove */,
 bool /* tickBelow */
)
{
  p->save();

  p->fillRect(x + 1, y + 1, w - 2, h - 2, g.background());
  p->setPen(g.dark());
  p->setBrush(g.light());

  int sl = sliderLength();

  if( o == TQt::Horizontal )
    p->drawEllipse(x, y + h / 2 - sl / 2, sl, sl);
  else
  if( o == TQt::Vertical )
    p->drawEllipse(x + w / 2 - sl / 2, y, sl, sl);

  p->restore();
}

  void
WebStyle::drawTDEToolBar
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 TDEToolBarPos /* pos */,
 TQBrush * /* fill */
)
{
  p->save();
  p->setPen(g.background());
  p->setBrush(g.background());
  p->drawRect(x, y, w, h);
  p->restore();
}

  void
WebStyle::drawKBarHandle
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 TDEToolBarPos /* pos */,
 TQBrush * /* fill */
)
{
  p->save();
  p->setPen(g.mid());
  p->setBrush(g.background());
  p->drawRect(x + 1, y + 1, w - 2, h - 2);
  p->restore();
}

  void
WebStyle::drawKMenuBar
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 bool /* macMode */,
 TQBrush * /* fill */
)
{
  p->save();
  p->setPen(g.mid());
  p->setBrush(g.background());
  p->drawRect(x, y, w, h);
  p->restore();
}

  void
WebStyle::drawTDEToolBarButton
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 bool sunken,
 bool raised,
 bool enabled,
 bool popup,
 TDEToolButtonType type,
 const TQString & btext,
 const TQPixmap * pixmap,
 TQFont * font,
 TQWidget * button
)
{
  bool toggleAndOn = false;

  if (button->inherits("TQButton"))
  {
    TQButton * b = static_cast<TQButton *>(button);
    toggleAndOn = b->isToggleButton() && b->isOn();
  }

  p->save();

  TQColor borderColour;
  TQColor textColour;
  TQColor fillColour;

  if (!enabled)
  {
    borderColour  = g.background();
    fillColour    = g.background();
    textColour    = g.text();
  }
  else
  {
    if (toggleAndOn)
    {
      borderColour  = g.dark();
      fillColour    = g.button();
      textColour    = g.buttonText();
    }
    else if (sunken)
    {
      borderColour  = g.light();
      fillColour    = g.button();
      textColour    = g.buttonText();
    }
    else if (raised)
    {
      borderColour  = g.highlight();
      fillColour    = g.background();
      textColour    = g.text();
    }
    else
    {
      borderColour  = g.background();
      fillColour    = g.background();
      textColour    = g.text();
    }
  }

  p->setPen(borderColour);
  p->setBrush(fillColour);

  p->drawRect(x, y, w, h);

  p->setPen(g.background());

  p->drawPoint(x, y);
  p->drawPoint(x + w, y);
  p->drawPoint(x, y + h);
  p->drawPoint(x + w, y + h);

  switch (type)
  {
    case Icon:

      if (0 != pixmap)
        p->drawPixmap
          (
           x + (w - pixmap->width()) / 2,
           y + (h - pixmap->height()) / 2,
           *pixmap
          );
      break;

    case Text:

    if (!btext.isNull())
      {
        if (0 != font)
          p->setFont(*font);

        p->setPen(textColour);

        p->drawText
          (
           x,
           y,
           w,
           h,
           AlignCenter,
           btext
          );
      }

      break;

    case IconTextRight:

      if (0 != pixmap)
        p->drawPixmap
          (
           x + 2,
           y + (h - pixmap->height()) / 2,
           *pixmap
          );

      if (!btext.isNull())
      {
        if (0 != font)
          p->setFont(*font);

        p->setPen(textColour);

        if (0 != pixmap)
        {
          int textLeft = pixmap->width() + 4;

          p->drawText
            (
             x + textLeft,
             y,
             w - textLeft,
             h,
             AlignVCenter | AlignLeft,
             btext
            );
        }
        else
        {
          p->drawText
            (
             x,
             y,
             w,
             h,
             AlignVCenter | AlignLeft,
             btext
            );
        }
      }
      break;

    case IconTextBottom:

      if (0 != pixmap)
        p->drawPixmap
          (
           x + (w - pixmap->width()) / 2,
           y + 2,
           *pixmap
          );

      if (!btext.isNull())
      {
        if (0 != font)
          p->setFont(*font);

        p->setPen(textColour);

        if (0 != pixmap)
        {
          int textTop = y + pixmap->height() + 4;

          p->drawText
            (
             x + 2,
             textTop,
             w - 4,
             h - x - textTop,
             AlignCenter,
             btext
            );
        }
        else
        {
          p->drawText
            (
             x,
             y,
             w,
             h,
             AlignCenter,
             btext
            );
        }
      }
      break;

    default:
      break;
  }

  if (popup)
  {
    p->setPen(g.dark());
    for (int px = 0; px < 5; ++px)
      for (int py = 0; py < 5 - px; ++py)
        p->drawPoint(w - 6 - px, h - 6 - py);
  }
      
  p->restore();
}

  void
WebStyle::drawKMenuItem
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 bool active,
 TQMenuItem * mi,
 TQBrush * /* fill */
)
{
  p->save();

  TQColor bg(active ? g.highlight() : g.background());

  p->fillRect(x, y, w, h, bg);

  TQColor textColour =
    active ?
    contrastingForeground(g.highlightedText(), bg) :
    contrastingForeground(g.text(), bg);

  TQApplication::style().drawItem
    (
     p,
     x,
     y,
     w,
     h,
     AlignCenter | ShowPrefix | DontClip | SingleLine,
     g,
     mi->isEnabled(),
     mi->pixmap(),
     mi->text(),
     -1,
     &textColour
    );

  p->restore();
}

  void
WebStyle::drawPopupMenuItem
(
 TQPainter * p,
 bool checkable,
 int maxpmw,
 int tab,
 TQMenuItem * mi,
 const TQPalette & pal,
 bool act,
 bool enabled,
 int x,
 int y,
 int w,
 int h
)
{
  // TODO
  TDEStyle::drawPopupMenuItem(p, checkable, maxpmw, tab, mi, pal, act, enabled, x, y, w, h);
}

  void
WebStyle::drawKProgressBlock
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 TQBrush * fill
)
{
  p->save();

  p->setBrush(0 == fill ? NoBrush : *fill);

  p->fillRect(x, y, w, h, g.highlight());

  p->restore();
}

  void
WebStyle::drawFocusRect
(
 TQPainter * p,
 const TQRect & r,
 const TQColorGroup & g,
 const TQColor * pen,
 bool atBorder
)
{
  p->save();

  if (0 != pen)
  p->setPen(0 == pen ? g.foreground() : *pen);
  p->setBrush(NoBrush);

  if (atBorder)
  {
    p->drawRect(TQRect(r.x() + 1, r.y() + 1, r.width() - 2, r.height() - 2));
  }
  else
  {
    p->drawRect(r);
  }

  p->restore();
}

  void
WebStyle::drawPanel
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 bool /* sunken */,
 int /* lineWidth */,
 const TQBrush * fill
)
{
  p->save();

  p->setPen(g.dark());

  p->setBrush(0 == fill ? NoBrush : *fill);

  p->drawRect(x, y, w, h);

  p->restore();
}

  void
WebStyle::drawPopupPanel
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 int /* lineWidth */,
 const TQBrush * fill
)
{
  p->save();

  p->setPen(g.dark());

  p->setBrush(0 == fill ? NoBrush : *fill);

  p->drawRect(x, y, w, h);

  p->restore();
}

  void
WebStyle::drawSeparator
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 bool /* sunken */,
 int /* lineWidth */,
 int /* midLineWidth */
)
{
  p->save();

  p->setPen(g.dark());

  if (w > h)
  {
    p->drawLine(x, y + h / 2, x + w, y + h / 2);
  }
  else
  {
    p->drawLine(x + w / 2, y, x + w / 2, y + h);
  }

  p->restore();
}

  void
WebStyle::drawTab
(
 TQPainter * p,
 const TQTabBar * tabBar,
 TQTab * tab,
 bool selected
)
{
  TQRect r(tab->rect());

  TQColorGroup g(tabBar->colorGroup());

  p->save();

  p->setPen(selected ? g.dark() : g.mid());
  p->fillRect(r, g.brush(TQColorGroup::Background));

  switch (tabBar->shape())
  {
    case TQTabBar::RoundedAbove:
    case TQTabBar::TriangularAbove:
      p->drawLine(r.left(), r.top(), r.left(), r.bottom());
      p->drawLine(r.left(), r.top(), r.right(), r.top());
      p->drawLine(r.right(), r.top(), r.right(), r.bottom());
      if (!selected)
      {
        p->setPen(g.dark());
        p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
      }
      break;
    case TQTabBar::RoundedBelow:
    case TQTabBar::TriangularBelow:
      if (!selected)
      {
        p->setPen(g.dark());
        p->drawLine(r.left(), r.top(), r.right(), r.top());
      }
      p->drawLine(r.left(), r.top(), r.left(), r.bottom());
      p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
      p->drawLine(r.right(), r.top(), r.right(), r.bottom());
      break;
  }

  p->restore();
}

  void
WebStyle::drawTabMask
(
 TQPainter * p,
 const TQTabBar *,
 TQTab * tab,
 bool
)
{
  p->fillRect(tab->rect(), TQt::color1);
}

  void
WebStyle::drawKickerHandle
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 TQBrush * fill
)
{
  p->save();

  p->setPen(g.mid());

  p->setBrush(0 == fill ? NoBrush : *fill);

  p->drawRect(x, y, w, h);
  
  p->restore();
}

  void
WebStyle::drawKickerAppletHandle
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 TQBrush * fill
)
{
  p->save();

  p->setPen(g.mid());

  p->setBrush(0 == fill ? NoBrush : *fill);

  p->drawRect(x, y, w, h);
  
  p->restore();
}

  void
WebStyle::drawKickerTaskButton
(
 TQPainter * p,
 int x,
 int y,
 int w,
 int h,
 const TQColorGroup & g,
 const TQString & text,
 bool active,
 TQPixmap * icon,
 TQBrush * /* fill */
)
{
  p->save();

  TQColor bg;

  if (active)
  {
    p->setPen(g.light());
    bg = g.highlight();
  }
  else
  {
    p->setPen(g.mid());
    bg = g.button();
  }

  p->setBrush(bg);

  p->drawRect(x, y, w, h);

  if (text.isEmpty() && 0 == icon)
  {
    p->restore();
    return;
  }

  const int pxWidth = 20;

  int textPos = pxWidth;

  TQRect br(buttonRect(x, y, w, h));

  if ((0 != icon) && !icon->isNull())
  {
    int dx = (pxWidth - icon->width())  / 2;
    int dy = (h - icon->height())       / 2;

    p->drawPixmap(br.x() + dx, dy, *icon);
  }

  TQString s(text);

  static TQString modStr =
    TQString::fromUtf8("[") + i18n("modified") + TQString::fromUtf8("]");

  int modStrPos = s.find(modStr);

  if (-1 != modStrPos)
  {
    // +1 because we include a space after the closing brace.
    s.remove(modStrPos, modStr.length() + 1);

    TQPixmap modPixmap = SmallIcon("modified");

    int dx = (pxWidth - modPixmap.width())  / 2;
    int dy = (h       - modPixmap.height()) / 2;

    p->drawPixmap(br.x() + textPos + dx, dy, modPixmap);

    textPos += pxWidth;
  }

  if (!s.isEmpty())
  {
    if (p->fontMetrics().width(s) > br.width() - textPos)
    {
      int maxLen = br.width() - textPos - p->fontMetrics().width("...");

      while ((!s.isEmpty()) && (p->fontMetrics().width(s) > maxLen))
        s.truncate(s.length() - 1);

      s.append("...");
    }

    if (active)
    {
      p->setPen(contrastingForeground(g.buttonText(), bg));
    }
    else
    {
      p->setPen(contrastingForeground(g.text(), bg));
    }

    p->setPen(TQt::white);

    p->drawText
      (
       br.x() + textPos,
       -1,
       w - textPos,
       h,
       AlignLeft | AlignVCenter,
       s
      );
  }

  p->restore();
  p->setPen(TQt::white);
}

  int
WebStyle::popupMenuItemHeight(bool, TQMenuItem * i, const TQFontMetrics & fm)
{
  if (i->isSeparator())
    return 1;

  int h = 0;

  if (0 != i->pixmap())
  {
    h = i->pixmap()->height();
  }

  if (0 != i->iconSet())
  {
    h = TQMAX
      (
       i->iconSet()->pixmap(TQIconSet::Small, TQIconSet::Normal).height(),
       h
      );
  }

  h = TQMAX(fm.height() + 4, h);

  h = TQMAX(18, h);

  return h;

}

