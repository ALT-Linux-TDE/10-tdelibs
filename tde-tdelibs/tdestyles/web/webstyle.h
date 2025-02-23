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

#ifndef WEB_STYLE_H
#define WEB_STYLE_H

#include <tdestyle.h>
#include <tqpalette.h>

class TQPainter;
class TQScrollBar;
class TQPushButton;
class TQWidget;

class WebStyle : public TDEStyle
{
  public:

    WebStyle();

    ~WebStyle();

    void polish(TQApplication *);

    void unPolish(TQWidget *);

    void polish(TQWidget *);

    void polish(TQPalette &);

    void unPolish(TQApplication *);

    void drawButton
      (
       TQPainter * p,
       int x,
       int y,
       int w,
       int h,
       const TQColorGroup & g,
       bool sunken = false,
       const TQBrush * fill = 0
      );

    TQRect buttonRect(int x, int y, int w, int h);

    void drawBevelButton
      (
       TQPainter *,
       int x,
       int y,
       int w,
       int h,
       const TQColorGroup &,
       bool sunken = false,
       const TQBrush * fill = 0
      );

    void drawPushButton(TQPushButton *, TQPainter *);

    virtual void drawPushButtonLabel(TQPushButton *, TQPainter *);

    void drawScrollBarControls
      (
       TQPainter *,
       const TQScrollBar *,
       int sliderStart,
       uint controls,
       uint activeControl
      );

    TQStyle::ScrollControl scrollBarPointOver
      (
       const TQScrollBar *,
       int sliderStart,
       const TQPoint &
      );

    void scrollBarMetrics
      (
       const TQScrollBar *,
       int & sliderMin,
       int & sliderMax,
       int & sliderLength,
       int & buttonDim
      );

    TQSize indicatorSize() const;

    void drawIndicator
      (
       TQPainter *,
       int x,
       int y,
       int w,
       int h,
       const TQColorGroup &,
       int state,
       bool down = false,
       bool enabled = true
      );

    TQSize exclusiveIndicatorSize() const;

    void drawExclusiveIndicator
      (
       TQPainter *,
       int x,
       int y,
       int w,
       int h,
       const TQColorGroup &,
       bool on,
       bool down = false,
       bool enabled = true
      );

    void drawIndicatorMask
      (
       TQPainter *,
       int x,
       int y,
       int w,
       int h,
       int state
      );

    void drawExclusiveIndicatorMask
      (
       TQPainter *,
       int x, 
       int y, 
       int w,
       int h, 
       bool on
      );

    void drawComboButton
      (
       TQPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const TQColorGroup &, 
       bool sunken = false,
       bool editable = false, 
       bool enabled = true,
       const TQBrush * fill = 0
      );

    TQRect comboButtonRect(int x, int y, int w, int h);

    TQRect comboButtonFocusRect(int x, int y, int w, int h);

    int sliderLength() const;

    void drawSliderGroove
      (
       TQPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const TQColorGroup &,
       QCOORD, 
       Orientation
      );

    void drawArrow
      (
       TQPainter *,
       TQt::ArrowType, 
       bool down,
       int x, 
       int y, 
       int w, 
       int h, 
       const TQColorGroup &,
       bool enabled = true, 
       const TQBrush * fill = 0
      );

    void drawSlider
      (
       TQPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const TQColorGroup &, 
       Orientation,
       bool tickAbove, 
       bool tickBelow
      );

    void drawTDEToolBar
      (
       TQPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const TQColorGroup &, 
       TDEToolBarPos,
       TQBrush * fill = 0
      );

    void drawKBarHandle
      (
       TQPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const TQColorGroup &,
       TDEToolBarPos, 
       TQBrush * fill = 0
      );

    void drawKMenuBar
      (
       TQPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const TQColorGroup &, 
       bool macMode,
       TQBrush * fill = 0
      );

    void drawTDEToolBarButton
      (
       TQPainter * p, 
       int x, 
       int y, 
       int w, 
       int h,
       const TQColorGroup & g, 
       bool sunken = false,
       bool raised = true, 
       bool enabled = true,
       bool popup = false,
       TDEToolButtonType = Icon,
       const TQString & btext = TQString::null,
       const TQPixmap * = 0,
       TQFont * = 0,
       TQWidget * button = 0
      );

    void drawKMenuItem
      (
       TQPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const TQColorGroup &, 
       bool active,
       TQMenuItem *, 
       TQBrush * fill = 0
      );

    void drawPopupMenuItem
      (
       TQPainter *, 
       bool checkable, 
       int maxpmw,
       int tab,
       TQMenuItem *, 
       const TQPalette &,
       bool act, 
       bool enabled, 
       int x, 
       int y, 
       int w,
       int h
      );

    void drawKProgressBlock
      (
       TQPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const TQColorGroup &, 
       TQBrush * fill
      );

    void drawFocusRect
      (
       TQPainter *, 
       const TQRect &, 
       const TQColorGroup &,
       const TQColor * pen, 
       bool atBorder
      );

    void drawPanel
      (
       TQPainter *,
       int x,
       int y,
       int w,
       int h,
       const TQColorGroup &,
       bool sunken,
       int lineWidth = 1,
       const TQBrush * = 0
      );

    void drawPopupPanel
      (
       TQPainter *,
       int x,
       int y,
       int w,
       int h,
       const TQColorGroup &,
       int lineWidth = 2,
       const TQBrush * = 0
      );

    void drawSeparator
      (
       TQPainter *,
       int x,
       int y,
       int w,
       int h,
       const TQColorGroup &,
       bool sunken = true,
       int lineWidth = 1,
       int midLineWidth = 0
      );

    void drawTab
      (
       TQPainter * p,
       const TQTabBar * tabBar,
       TQTab * tab,
       bool selected
      );

    void drawTabMask
      (
       TQPainter * p,
       const TQTabBar *,
       TQTab * tab,
       bool
      );

    void drawKickerHandle
      (
       TQPainter * p,
       int x,
       int y,
       int w,
       int h,
       const TQColorGroup & g,
       TQBrush *
      );

    void drawKickerAppletHandle
      (
       TQPainter * p,
       int x,
       int y,
       int w,
       int h,
       const TQColorGroup & g,
       TQBrush *
      );

    void drawKickerTaskButton
      (
       TQPainter * p,
       int x,
       int y,
       int w,
       int h,
       const TQColorGroup & g,
       const TQString & title,
       bool active,
       TQPixmap * icon,
       TQBrush *
      );

    int popupMenuItemHeight(bool, TQMenuItem *, const TQFontMetrics &);

    GUIStyle guiStyle() const { return TQt::MotifStyle; }

    bool eventFilter(TQObject *, TQEvent *);
};

#endif
