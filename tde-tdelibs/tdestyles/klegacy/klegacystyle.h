/*

  Copyright (c) 2000 KDE Project

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

 */

#ifndef   __KLegacyStyle_hh
#define   __KLegacyStyle_hh

#include <tdestyle.h>

// forward declaration
class KLegacyStylePrivate;


class TQ_EXPORT KLegacyStyle : public TDEStyle {
    TQ_OBJECT
public:
    KLegacyStyle(void);
    virtual ~KLegacyStyle(void);

    virtual int defaultFrameWidth() const;

    virtual void polish(TQApplication *);
    virtual void polish(TQWidget *);
    virtual void polishPopupMenu(TQPopupMenu *);
    virtual void unPolish(TQWidget *);
    virtual void unPolish(TQApplication *);

    // combo box
    virtual void drawComboButton(TQPainter *, int, int, int, int, const TQColorGroup &,
				 bool = false, bool = false, bool = true,
				 const TQBrush * = 0);
    virtual TQRect comboButtonRect(int, int, int, int);
    virtual TQRect comboButtonFocusRect(int, int, int, int);

    // menubar items
    virtual void drawMenuBarItem(TQPainter *, int, int, int, int, TQMenuItem *,
				 TQColorGroup &, bool, bool);
    virtual void drawKMenuItem(TQPainter *, int, int, int, int, const TQColorGroup &, bool,
                               TQMenuItem *, TQBrush * = 0);
    
    // toolbar stuffs
    virtual void drawKBarHandle(TQPainter *p, int x, int y, int w, int h,
                                const TQColorGroup &g, TDEToolBarPos type, TQBrush *fill = 0);
    virtual void drawKickerHandle(TQPainter *p, int x, int y, int w, int h,
                                  const TQColorGroup &g, TQBrush *fill = 0);
    virtual void drawKickerAppletHandle(TQPainter *p, int x, int y, int w, int h,
                                        const TQColorGroup &g, TQBrush *fill = 0);
    virtual void drawKickerTaskButton(TQPainter *p, int x, int y, int w, int h,
                                      const TQColorGroup &g, const TQString &title, bool active,
                                      TQPixmap *icon = 0, TQBrush *fill = 0);

    // arrows
    virtual void drawArrow(TQPainter *, ArrowType, bool, int, int, int, int,
			   const TQColorGroup &, bool, const TQBrush * = 0);

    // button stuffs
    virtual void drawButton(TQPainter *, int, int, int, int, const TQColorGroup &g,
    			    bool = false, const TQBrush * = 0);
    virtual void drawPushButton(TQPushButton *, TQPainter *);
    virtual void drawBevelButton(TQPainter *, int, int, int, int,
				 const TQColorGroup &, bool = false,
				 const TQBrush * = 0);

    // indicators (TQCheckBox)
    virtual void drawCheckMark(TQPainter *, int, int, int, int, const TQColorGroup &,
			       bool = false, bool = true);
    virtual void drawIndicator(TQPainter *, int, int, int, int, const TQColorGroup &,
			       int, bool = false, bool = true);
    virtual void drawIndicatorMask(TQPainter *, int, int, int, int, int);
    virtual TQSize indicatorSize(void) const;

    // exclusive indicators (TQRadioButton)
    virtual void drawExclusiveIndicator(TQPainter *, int, int, int, int,
					const TQColorGroup &, bool, bool = false,
					bool = true);
    virtual void drawExclusiveIndicatorMask(TQPainter *, int, int, int, int, bool);
    virtual TQSize exclusiveIndicatorSize(void) const;

    // popup menus
    virtual void drawPopupPanel(TQPainter *, int, int, int, int, const TQColorGroup &,
				int = 2, const TQBrush * = 0);
    virtual void drawPopupMenuItem(TQPainter *, bool, int, int, TQMenuItem *,
				   const TQPalette &, bool, bool, int, int, int, int);


    // scrollbars
    virtual ScrollControl scrollBarPointOver(const TQScrollBar *, int, const TQPoint &);
    virtual void scrollBarMetrics(const TQScrollBar *, int &, int &, int &, int &);
    virtual void drawScrollBarControls(TQPainter *, const TQScrollBar *,
				       int, uint, uint);

    // sliders
    virtual void drawSlider(TQPainter *, int , int , int , int ,
			    const TQColorGroup &, Orientation, bool, bool);
    virtual void drawSliderGroove(TQPainter *, int, int, int, int, const TQColorGroup &,
				  QCOORD, Orientation);

    // panel
    virtual void drawPanel(TQPainter *, int, int, int, int, const TQColorGroup &,
			   bool = false, int = 1, const TQBrush * = 0);

    // splitters
    virtual void drawSplitter(TQPainter *, int, int, int, int,
			      const TQColorGroup &, Orientation);

    // tabs
    virtual void drawTab(TQPainter *, const TQTabBar *, TQTab *, bool);


protected:
    bool eventFilter(TQObject *, TQEvent *);

    void drawMenuArrow(TQPainter *, ArrowType, bool, int, int, int, int,
		       const TQColorGroup &, bool, const TQBrush * = 0);


private:
    KLegacyStylePrivate *priv;

#if defined(TQ_DISABLE_COPY)
    KLegacyStyle( const KLegacyStyle & );
    KLegacyStyle& operator=( const KLegacyStyle & );
#endif

};


#endif // __KLegacyStyle_hh
