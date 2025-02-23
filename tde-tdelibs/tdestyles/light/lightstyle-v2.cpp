/*
  Copyright (c) 2000-2001 Trolltech AS (info@trolltech.com)

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

#include "lightstyle-v2.h"

#include "tqmenubar.h"
#include "tqapplication.h"
#include "tqpainter.h"
#include "tqpalette.h"
#include "tqframe.h"
#include "tqpushbutton.h"
#include "tqdrawutil.h"
#include "tqprogressbar.h"
#include "tqscrollbar.h"
#include "tqtabbar.h"
#include "tqguardedptr.h"
#include "tqlayout.h"
#include "tqlineedit.h"
#include "tqimage.h"
#include "tqcombobox.h"
#include "tqslider.h"
#include "tqstylefactory.h"


class LightStyleV2Private
{
public:
    LightStyleV2Private()
	: ref(1)
    {
	basestyle = TQStyleFactory::create( "Windows" );
	if ( ! basestyle )
	    basestyle = TQStyleFactory::create( TQStyleFactory::keys().first() );
	if ( ! basestyle )
	    tqFatal( "LightStyle: couldn't find a basestyle!" );
    }

    ~LightStyleV2Private()
    {
	delete basestyle;
    }

    TQStyle *basestyle;
    int ref;
};

static LightStyleV2Private *singleton = 0;


LightStyleV2::LightStyleV2()
    : TDEStyle(AllowMenuTransparency)
{
    if (! singleton)
	singleton = new LightStyleV2Private;
    else
	singleton->ref++;
}

LightStyleV2::~LightStyleV2()
{
    if (singleton && --singleton->ref <= 0) {
	delete singleton;
	singleton = 0;
    }
}

void LightStyleV2::polishPopupMenu( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr )
{
    TDEStyle::polishPopupMenu(ceData, elementFlags, ptr);
}

static void drawLightBevel(TQPainter *p, const TQRect &r, const TQColorGroup &cg,
			   TQStyle::SFlags flags, const TQBrush *fill = 0)
{
    TQRect br = r;
    bool sunken = (flags & (TQStyle::Style_Down | TQStyle::Style_On |
			    TQStyle::Style_Sunken));

    p->setPen(cg.dark());
    p->drawRect(r);

    if (flags & (TQStyle::Style_Down | TQStyle::Style_On |
		 TQStyle::Style_Sunken | TQStyle::Style_Raised)) {
	// button bevel
	if (sunken)
	    p->setPen(cg.mid());
	else
	    p->setPen(cg.light());

	p->drawLine(r.x() + 1, r.y() + 2,
		    r.x() + 1, r.y() + r.height() - 3); // left
	p->drawLine(r.x() + 1, r.y() + 1,
		    r.x() + r.width() - 2, r.y() + 1); // top

	if (sunken)
	    p->setPen(cg.light());
	else
	    p->setPen(cg.mid());

	p->drawLine(r.x() + r.width() - 2, r.y() + 2,
		    r.x() + r.width() - 2, r.y() + r.height() - 3); // right
	p->drawLine(r.x() + 1, r.y() + r.height() - 2,
		    r.x() + r.width() - 2, r.y() + r.height() - 2); // bottom

	br.addCoords(2, 2, -2, -2);
    } else
	br.addCoords(1, 1, -1, -1);

    // fill
    if (fill) p->fillRect(br, *fill);
}

void LightStyleV2::drawPrimitive( PrimitiveElement pe,
				TQPainter *p,
				const TQStyleControlElementData &ceData,
				ControlElementFlags elementFlags,
				const TQRect &r,
				const TQColorGroup &cg,
				SFlags flags,
				const TQStyleOption &data ) const
{
    switch (pe) {
    case PE_HeaderSectionMenu:
    case PE_HeaderSection:
	{
	    flags = ((flags | Style_Sunken) ^ Style_Sunken) | Style_Raised; 
	    	//Don't show pressed too often (as in light 3)
	    TQBrush fill(cg.background());
	    if (flags & TQStyle::Style_Enabled)
		fill.setColor(cg.button());
	    
	    drawLightBevel(p, r, cg, flags, &fill);
	    p->setPen( cg.buttonText() );
	    break;
	}
	
    case PE_ButtonCommand:
    case PE_ButtonBevel:
    case PE_ButtonTool:
	{
	    const TQBrush *fill;
	    if (flags & TQStyle::Style_Enabled) {
		if (flags & (TQStyle::Style_Down |
			     TQStyle::Style_On |
			     TQStyle::Style_Sunken))
		    fill = &cg.brush(TQColorGroup::Midlight);
		else
		    fill = &cg.brush(TQColorGroup::Button);
	    } else
		fill = &cg.brush(TQColorGroup::Background);
	    drawLightBevel(p, r, cg, flags, fill);
	    break;
	}

    case PE_ButtonDropDown:
	{
	    TQBrush thefill;
	    bool sunken =
		(flags & (TQStyle::Style_Down | TQStyle::Style_On | TQStyle::Style_Sunken));

	    if (flags & TQStyle::Style_Enabled) {
		if (sunken)
		    thefill = cg.brush(TQColorGroup::Midlight);
		else
		    thefill = cg.brush(TQColorGroup::Button);
	    } else
		thefill = cg.brush(TQColorGroup::Background);

	    p->setPen(cg.dark());
	    p->drawLine(r.topLeft(),     r.topRight());
	    p->drawLine(r.topRight(),    r.bottomRight());
	    p->drawLine(r.bottomRight(), r.bottomLeft());

	    if (flags & (TQStyle::Style_Down | TQStyle::Style_On |
			 TQStyle::Style_Sunken | TQStyle::Style_Raised)) {
		// button bevel
		if (sunken)
		    p->setPen(cg.mid());
		else
		    p->setPen(cg.light());

		p->drawLine(r.x(), r.y() + 2,
			    r.x(), r.y() + r.height() - 3); // left
		p->drawLine(r.x(), r.y() + 1,
			    r.x() + r.width() - 2, r.y() + 1); // top

		if (sunken)
		    p->setPen(cg.light());
		else
		    p->setPen(cg.mid());

		p->drawLine(r.x() + r.width() - 2, r.y() + 2,
			    r.x() + r.width() - 2, r.y() + r.height() - 3); // right
		p->drawLine(r.x() + 1, r.y() + r.height() - 2,
			    r.x() + r.width() - 2, r.y() + r.height() - 2); // bottom
	    }

	    p->fillRect(r.x() + 1, r.y() + 2, r.width() - 3, r.height() - 4, thefill);
	    break;
	}

    case PE_ButtonDefault:
	p->setPen(cg.dark());
	p->setBrush(cg.light());
	p->drawRect(r);
	break;

    case PE_Indicator:
	const TQBrush *fill;
	if (! (flags & Style_Enabled))
	    fill = &cg.brush(TQColorGroup::Background);
	else if (flags & Style_Down)
	    fill = &cg.brush(TQColorGroup::Mid);
	else
	    fill = &cg.brush(TQColorGroup::Base);
	drawLightBevel(p, r, cg, flags | Style_Sunken, fill);

	p->setPen(cg.text());
	if (flags & Style_NoChange) {
	    p->drawLine(r.x() + 3, r.y() + r.height() / 2,
			r.x() + r.width() - 4, r.y() + r.height() / 2);
	    p->drawLine(r.x() + 3, r.y() + 1 + r.height() / 2,
			r.x() + r.width() - 4, r.y() + 1 + r.height() / 2);
	    p->drawLine(r.x() + 3, r.y() - 1 + r.height() / 2,
			r.x() + r.width() - 4, r.y() - 1 + r.height() / 2);
	} else if (flags & Style_On) {
	    p->drawLine(r.x() + 4, r.y() + 3,
			r.x() + r.width() - 4, r.y() + r.height() - 5);
	    p->drawLine(r.x() + 3, r.y() + 3,
			r.x() + r.width() - 4, r.y() + r.height() - 4);
	    p->drawLine(r.x() + 3, r.y() + 4,
			r.x() + r.width() - 5, r.y() + r.height() - 4);
	    p->drawLine(r.x() + 3, r.y() + r.height() - 5,
			r.x() + r.width() - 5, r.y() + 3);
	    p->drawLine(r.x() + 3, r.y() + r.height() - 4,
			r.x() + r.width() - 4, r.y() + 3);
	    p->drawLine(r.x() + 4, r.y() + r.height() - 4,
			r.x() + r.width() - 4, r.y() + 4);
	}

	break;

    case PE_ExclusiveIndicator:
	{
	    TQRect br = r, // bevel rect
		  cr = r, // contents rect
		  ir = r; // indicator rect
	    br.addCoords(1, 1, -1, -1);
	    cr.addCoords(2, 2, -2, -2);
	    ir.addCoords(3, 3, -3, -3);

	    p->fillRect(r, cg.brush(TQColorGroup::Background));

	    p->setPen(cg.dark());
	    p->drawArc(r, 0, 16*360);
	    p->setPen(cg.mid());
	    p->drawArc(br, 45*16, 180*16);
	    p->setPen(cg.light());
	    p->drawArc(br, 235*16, 180*16);

	    p->setPen(flags & Style_Down ? cg.mid() :
		      (flags & Style_Enabled ? cg.base() : cg.background()));
	    p->setBrush(flags & Style_Down ? cg.mid() :
			(flags & Style_Enabled ? cg.base() : cg.background()));
	    p->drawEllipse(cr);

	    if (flags & Style_On) {
		p->setBrush(cg.text());
		p->drawEllipse(ir);
	    }

	    break;
	}

    case PE_DockWindowHandle:
	{
	    TQString title;
	    bool drawTitle = false;
	    if ( p && p->device()->devType() == TQInternal::Widget ) {
		TQWidget *w = (TQWidget *) p->device();
		TQWidget *p = w->parentWidget();
		if (p->inherits("TQDockWindow") && ! p->inherits("TQToolBar")) {
		    drawTitle = true;
		    title = p->caption();
		}
	    }

	    flags |= Style_Raised;
	    if (flags & Style_Horizontal) {
		if (drawTitle) {
		    TQPixmap pm(r.height(), r.width());
		    TQPainter p2(&pm);
		    p2.fillRect(0, 0, pm.width(), pm.height(),
				cg.brush(TQColorGroup::Highlight));
		    p2.setPen(cg.highlightedText());
		    p2.drawText(0, 0, pm.width(), pm.height(), AlignCenter, title);
		    p2.end();

		    TQWMatrix m;
		    m.rotate(270.0);
		    pm = pm.xForm(m);
		    p->drawPixmap(r.x(), r.y(), pm);
		} else {
		    p->fillRect(r, cg.background());
		    p->setPen(cg.mid().dark());
		    p->drawLine(r.right() - 6, r.top() + 2,
				r.right() - 6, r.bottom() - 2);
		    p->drawLine(r.right() - 3, r.top() + 2,
				r.right() - 3, r.bottom() - 2);
		    p->setPen(cg.light());
		    p->drawLine(r.right() - 5, r.top() + 2,
				r.right() - 5, r.bottom() - 2);
		    p->drawLine(r.right() - 2, r.top() + 2,
				r.right() - 2, r.bottom() - 2);
		}
	    } else {
		if (drawTitle) {
		    p->fillRect(r, cg.brush(TQColorGroup::Highlight));
		    p->setPen(cg.highlightedText());
		    p->drawText(r, AlignCenter, title);
		} else {
		    p->fillRect(r, cg.background());
		    p->setPen(cg.mid().dark());
		    p->drawLine(r.left() + 2,  r.bottom() - 6,
				r.right() - 2, r.bottom() - 6);
		    p->drawLine(r.left() + 2,  r.bottom() - 3,
				r.right() - 2, r.bottom() - 3);
		    p->setPen(cg.light());
		    p->drawLine(r.left() + 2,  r.bottom() - 5,
				r.right() - 2, r.bottom() - 5);
		    p->drawLine(r.left() + 2,  r.bottom() - 2,
				r.right() - 2, r.bottom() - 2);
		}
	    }
	    break;
	}

    case PE_DockWindowSeparator:
	{
	    if (r.width() > 20 || r.height() > 20) {
		if (flags & Style_Horizontal) {
		    p->setPen(cg.mid().dark(120));
		    p->drawLine(r.left() + 1, r.top() + 6, r.left() + 1, r.bottom() - 6);
		    p->setPen(cg.light());
		    p->drawLine(r.left() + 2, r.top() + 6, r.left() + 2, r.bottom() - 6);
		} else {
		    p->setPen(cg.mid().dark(120));
		    p->drawLine(r.left() + 6, r.top() + 1, r.right() - 6, r.top() + 1);
		    p->setPen(cg.light());
		    p->drawLine(r.left() + 6, r.top() + 2, r.right() - 6, r.top() + 2);
		}
	    } else
		TQCommonStyle::drawPrimitive(pe, p, ceData, elementFlags, r, cg, flags, data);
	    break;
	}

    case PE_Splitter:
	if (flags & Style_Horizontal)
	    flags &= ~Style_Horizontal;
	else
	    flags |= Style_Horizontal;
	// fall through intended

    case PE_DockWindowResizeHandle:
	{
	    p->fillRect(r, cg.background());
	    if (flags & Style_Horizontal) {
		p->setPen(cg.highlight().light());
		p->drawLine(r.left() + 1, r.top() + 1, r.right() - 1, r.top() + 1);
		p->setPen(cg.highlight());
		p->drawLine(r.left() + 1, r.top() + 2, r.right() - 1, r.top() + 2);
		p->setPen(cg.highlight().dark());
		p->drawLine(r.left() + 1, r.top() + 3, r.right() - 1, r.top() + 3);
	    } else {
		p->setPen(cg.highlight().light());
		p->drawLine(r.left() + 1, r.top() + 1, r.left() + 1, r.bottom() - 1);
		p->setPen(cg.highlight());
		p->drawLine(r.left() + 2, r.top() + 1, r.left() + 2, r.bottom() - 1);
		p->setPen(cg.highlight().dark());
		p->drawLine(r.left() + 3, r.top() + 1, r.left() + 3, r.bottom() - 1);
	    }
	    break;
	}

    case PE_Panel:
    case PE_PanelPopup:
    case PE_PanelLineEdit:
    case PE_PanelTabWidget:
    case PE_WindowFrame:
	{
	    int lw = data.isDefault() ?
		     pixelMetric(PM_DefaultFrameWidth, ceData, elementFlags) : data.lineWidth();

	    if ( ! ( flags & Style_Sunken ) )
		flags |= Style_Raised;
	    if (lw == 2)
		drawLightBevel(p, r, cg, flags);
	    else
		TQCommonStyle::drawPrimitive(pe, p, ceData, elementFlags, r, cg, flags, data);
	    break;
	}

    case PE_PanelDockWindow:
	{
	    int lw = data.isDefault() ?
		     pixelMetric(PM_DockWindowFrameWidth, ceData, elementFlags) : data.lineWidth();

	    if (lw == 2)
		drawLightBevel(p, r, cg, flags | Style_Raised,
			       &cg.brush(TQColorGroup::Button));
	    else
		TQCommonStyle::drawPrimitive(pe, p, ceData, elementFlags, r, cg, flags, data);
	    break;
	}

    case PE_PanelMenuBar:
	{
	    int lw = data.isDefault() ?
		     pixelMetric(PM_MenuBarFrameWidth, ceData, elementFlags) : data.lineWidth();

	    if (lw == 2)
		drawLightBevel(p, r, cg, flags, &cg.brush(TQColorGroup::Button));
	    else
		TQCommonStyle::drawPrimitive(pe, p, ceData, elementFlags, r, cg, flags, data);
	    break;
	}

    case PE_ScrollBarSubLine:
	{
	    TQRect fr = r, ar = r;
	    PrimitiveElement pe;

	    p->setPen(cg.dark());
	    if (flags & Style_Horizontal) {
		p->drawLine(r.topLeft(), r.topRight());
		fr.addCoords(0, 1, 0, 0);
		ar.addCoords(0, 1, 0, 0);
		pe = PE_ArrowLeft;
	    } else {
		p->drawLine(r.topLeft(), r.bottomLeft());
		fr.addCoords(1, 0, 0, 0);
		ar.addCoords(2, 0, 0, 0);
		pe = PE_ArrowUp;
	    }

	    p->fillRect(fr, cg.brush((flags & Style_Down) ?
				     TQColorGroup::Midlight :
				     TQColorGroup::Background));
	    drawPrimitive(pe, p, ceData, elementFlags, ar, cg, flags);
	    break;
	}

    case PE_ScrollBarAddLine:
	{
	    TQRect fr = r, ar = r;
	    PrimitiveElement pe;

	    p->setPen(cg.dark());
	    if (flags & Style_Horizontal) {
		p->drawLine(r.topLeft(), r.topRight());
		fr.addCoords(0, 1, 0, 0);
		ar.addCoords(0, 1, 0, 0);
		pe = PE_ArrowRight;
	    } else {
		p->drawLine(r.topLeft(), r.bottomLeft());
		fr.addCoords(1, 0, 0, 0);
		ar.addCoords(2, 0, 0, 0);
		pe = PE_ArrowDown;
	    }

	    p->fillRect(fr, cg.brush((flags & Style_Down) ?
				     TQColorGroup::Midlight :
				     TQColorGroup::Background));
	    drawPrimitive(pe, p, ceData, elementFlags, ar, cg, flags);
	    break;
	}

    case PE_ScrollBarSubPage:
    case PE_ScrollBarAddPage:
	{
	    TQRect fr = r;

	    p->setPen(cg.dark());
	    if (flags & Style_Horizontal) {
		p->drawLine(r.topLeft(), r.topRight());
		p->setPen(cg.background());
		p->drawLine(r.left(), r.top() + 1, r.right(), r.top() + 1);
		p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
		fr.addCoords(0, 2, 0, -1);
	    } else {
		p->drawLine(r.topLeft(), r.bottomLeft());
		p->setPen(cg.background());
		p->drawLine(r.left() + 1, r.top(), r.left() + 1, r.bottom());
		fr.addCoords(2, 0, 0, 0);
	    }

	    p->fillRect(fr, cg.brush((flags & Style_Down) ?
				     TQColorGroup::Midlight :
				     TQColorGroup::Mid));
	    break;
	}

    case PE_ScrollBarSlider:
	{
	    TQRect fr = r;

	    p->setPen(cg.dark());
	    if (flags & Style_Horizontal) {
		p->drawLine(r.topLeft(), r.topRight());
		p->setPen(cg.background());
		p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
		p->drawLine(r.left(), r.top() + 1, r.right(), r.top() + 1);
		fr.addCoords(0, 2, 0, -1);
	    } else {
		p->drawLine(r.topLeft(), r.bottomLeft());
		p->setPen(cg.background());
		p->drawLine(r.right(), r.top(), r.right(), r.bottom());
		p->drawLine(r.left() + 1, r.top(), r.left() + 1, r.bottom());
		fr.addCoords(2, 0, -1, 0);
	    }

	    drawLightBevel(p, fr, cg, ((flags | Style_Down) ^ Style_Down) |
			   ((flags & Style_Enabled) ? Style_Raised : Style_Default),
			   &cg.brush(TQColorGroup::Button));
	    break;
	}

    case PE_FocusRect:
	{
	    p->setBrush(NoBrush);
	    if (flags & Style_FocusAtBorder)
		p->setPen(cg.shadow());
	    else
		p->setPen(cg.dark());
	    p->drawRect(r);
	    break;
	}

    case PE_ProgressBarChunk:
	p->fillRect(r.x(), r.y() + 2, r.width(), r.height() - 4, cg.highlight());
	break;

    case PE_MenuItemIndicatorFrame:
    case PE_MenuItemIndicatorIconFrame:
	{
	    int checkcol = styleHint(SH_MenuIndicatorColumnWidth, ceData, elementFlags, data, NULL, NULL);
	    TQRect cr(r.left(), r.top(), checkcol, r.height());
	    bool reverse = TQApplication::reverseLayout();
	    if ( reverse ) {
		cr = visualRect( cr, r );
	    }
	    qDrawShadePanel(p, cr, cg, true, 1, &cg.brush(TQColorGroup::Midlight));
	}
	break;
    case PE_MenuItemIndicatorCheck:
	{
	    int checkcol = styleHint(SH_MenuIndicatorColumnWidth, ceData, elementFlags, data, NULL, NULL);
	    TQRect cr(r.left(), r.top(), checkcol, r.height());
	    bool reverse = TQApplication::reverseLayout();
	    if ( reverse ) {
		cr = visualRect( cr, r );
	    }
	    drawPrimitive(PE_CheckMark, p, ceData, elementFlags, cr, cg, (flags & Style_Enabled) | Style_On);
	}
	break;

    default:
	if (pe == PE_HeaderArrow) {
	    if (flags & Style_Down)
		pe = PE_ArrowDown;
	    else
		pe = PE_ArrowUp;
	}
	

	if (pe >= PE_ArrowUp && pe <= PE_ArrowLeft) {
	    TQPointArray a;

	    switch ( pe ) {
	    case PE_ArrowUp:
		a.setPoints( 7, -4,1, 2,1, -3,0, 1,0, -2,-1, 0,-1, -1,-2 );
		break;

	    case PE_ArrowDown:
		a.setPoints( 7, -4,-2, 2,-2, -3,-1, 1,-1, -2,0, 0,0, -1,1 );
		break;

	    case PE_ArrowRight:
		a.setPoints( 7, -2,-3, -2,3, -1,-2, -1,2, 0,-1, 0,1, 1,0 );
		break;

	    case PE_ArrowLeft:
		a.setPoints( 7, 0,-3, 0,3, -1,-2, -1,2, -2,-1, -2,1, -3,0 );
		break;

	    default:
		break;
	    }

	    if (a.isNull())
		return;

	    p->save();
	    if ( flags & Style_Enabled ) {
		a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
		p->setPen( cg.buttonText() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
	    } else {
		a.translate( r.x() + r.width() / 2 + 1, r.y() + r.height() / 2 + 1 );
		p->setPen( cg.light() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
		a.translate( -1, -1 );
		p->setPen( cg.mid() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
	    }
	    p->restore();
	} else
	    TQCommonStyle::drawPrimitive(pe, p, ceData, elementFlags, r, cg, flags, data);
	break;
    }
}

void LightStyleV2::drawControl( ControlElement control,
			      TQPainter *p,
			      const TQStyleControlElementData &ceData,
			      ControlElementFlags elementFlags,
			      const TQRect &r,
			      const TQColorGroup &cg,
			      SFlags flags,
			      const TQStyleOption &data,
			      const TQWidget *widget ) const
{
    switch (control) {
    case CE_TabBarTab:
	{
	    bool below = false;
	    TQRect tr(r);
	    TQRect fr(r);

	    tr.addCoords(0, 0,  0, -1);
	    fr.addCoords(2, 2, -2, -2);
	    
	    if ( ceData.tabBarData.shape == TQTabBar::RoundedBelow || ceData.tabBarData.shape == TQTabBar::TriangularBelow) {
		tr = r; tr.addCoords(0, 1, 0, 0);
		fr = r; fr.addCoords(2, 2,-2, -4);
		below = true;
	    }
		
	    if (! (flags & Style_Selected)) {
		if (below) {
		    tr.addCoords(0, 0, 0, -1);
		    tr.addCoords(0, 0, 0, -1);
		} else {
		    tr.addCoords(0, 1, 0, 0);
		    fr.addCoords(0, 1, 0, 0);
		}

		p->setPen(cg.dark());
		p->drawRect(tr);

		if (tr.left() == 0)
		    if (below) 
			p->drawPoint(tr.left(), tr.top() - 1);
		    else
			p->drawPoint(tr.left(), tr.bottom() + 1);

		p->setPen(cg.light());
		if (below) {
		    p->drawLine(tr.left() + 1, tr.top() + 1,
			    tr.left() + 1, tr.bottom() - 2);
		    p->drawLine(tr.left() + 1, tr.bottom() - 1,
			    tr.right() - 1, tr.bottom() - 1);
		} else {
		    p->drawLine(tr.left() + 1, tr.bottom() - 1,
			    tr.left() + 1, tr.top() + 2);
		    p->drawLine(tr.left() + 1, tr.top() + 1,
			    tr.right() - 1, tr.top() + 1);
		}
		
		if (below) {
		    if (tr.left() == 0)
			p->drawLine(tr.left() + 1, tr.top() - 1,
				    tr.right(), tr.top() - 1);
		    else
		    {
			p->setPen(cg.mid()); //To match lower border of the frame
			p->drawLine(tr.left(), tr.top() - 1,
				    tr.right(), tr.top() - 1);
		    }
		} else {
		    if (tr.left() == 0)
			p->drawLine(tr.left() + 1, tr.bottom() + 1,
				    tr.right(), tr.bottom() + 1);
		    else
			p->drawLine(tr.left(), tr.bottom() + 1,
				    tr.right(), tr.bottom() + 1);
		}
		
		p->setPen(cg.mid());
		
		if (below) {
		    p->drawLine(tr.right() - 1, tr.bottom() - 2,
				tr.right() - 1, tr.top() + 1);
		} else {
		    p->drawLine(tr.right() - 1, tr.top() + 2,
				tr.right() - 1, tr.bottom() - 1);
		}
	    } else {
		p->setPen(cg.dark());
		if (tr.left() == 0)
		    if (below)
			p->drawLine(tr.left(), tr.top() - 1,
				    tr.left(), tr.bottom() - 1);
		    else
			p->drawLine(tr.left(), tr.bottom() + 1,
				    tr.left(), tr.top() + 1);
		else
		    if (below)
			p->drawLine(tr.left(), tr.bottom(),
				    tr.left(), tr.top() + 1);
		    else
			p->drawLine(tr.left(), tr.bottom(),
				    tr.left(), tr.top() + 1);
				    
		if (below) {
		    p->drawLine(tr.left(), tr.bottom(),
				tr.right(), tr.bottom());
		    p->drawLine(tr.right(), tr.bottom() - 1,
				tr.right(), tr.top());

		} else {
		    p->drawLine(tr.left(), tr.top(),
				tr.right(), tr.top());
		    p->drawLine(tr.right(), tr.top() + 1,
				tr.right(), tr.bottom());
		}

		p->setPen(cg.light());
		if (tr.left() == 0)
		    if (below)
			p->drawLine(tr.left() + 1, tr.top() - 2,
				    tr.left() + 1, tr.bottom() - 2);
		    else
			p->drawLine(tr.left() + 1, tr.bottom() + 2,
				    tr.left() + 1, tr.top() + 2);
		else {
		    if (below) {
			p->drawLine(tr.left() + 1, tr.top(),
				    tr.left() + 1, tr.bottom() - 2);
			p->drawPoint(tr.left(), tr.top() - 1);

		    } else {
			p->drawLine(tr.left() + 1, tr.bottom(),
				    tr.left() + 1, tr.top() + 2);
			p->drawPoint(tr.left(), tr.bottom() + 1);
		    }
		}
		
		if (below) {
		    p->drawLine(tr.left() + 1, tr.bottom() - 1,
				tr.right() - 1, tr.bottom() - 1);
		    p->drawPoint(tr.right(), tr.top() - 1);

		    p->setPen(cg.mid());
		    p->drawLine(tr.right() - 1, tr.bottom() - 2,
				tr.right() - 1, tr.top());
		} else {
		    p->drawLine(tr.left() + 1, tr.top() + 1,
				tr.right() - 1, tr.top() + 1);
		    p->drawPoint(tr.right(), tr.bottom() + 1);

		    p->setPen(cg.mid());
		    p->drawLine(tr.right() - 1, tr.top() + 2,
				tr.right() - 1, tr.bottom());
		}
	    }

	    p->fillRect(fr, ((flags & Style_Selected) ?
			     cg.background() : cg.mid()));
	    break;
	}

    case CE_PopupMenuItem:
	{
	    if (data.isDefault())
		break;

	    TQMenuItem *mi = data.menuItem();
	    int tab = data.tabWidth();
	    int maxpmw = data.maxIconWidth();

	    if ( mi && mi->isSeparator() ) {
		// draw separator (bg first, though)      
		if ( !ceData.bgPixmap.isNull() )
		    p->drawPixmap( r.topLeft(), ceData.bgPixmap, r );
		else
		    p->fillRect(r, cg.brush(TQColorGroup::Button));
	    
		p->setPen(cg.mid().dark(120));
		p->drawLine(r.left() + 12,  r.top() + 1,
			    r.right() - 12, r.top() + 1);
		p->setPen(cg.light());
		p->drawLine(r.left() + 12,  r.top() + 2,
			    r.right() - 12, r.top() + 2);
		break;
	    }

	    if (flags & Style_Active)
		qDrawShadePanel(p, r, cg, true, 1,
				&cg.brush(TQColorGroup::Midlight));
	    else if ( !ceData.bgPixmap.isNull() )
		p->drawPixmap( r.topLeft(), ceData.bgPixmap, r );
	    else 
		p->fillRect(r, cg.brush(TQColorGroup::Button));

	    if ( !mi )
		break;

	    maxpmw = TQMAX(maxpmw, 16);

	    TQRect cr, ir, tr, sr;
	    // check column
	    cr.setRect(r.left(), r.top(), maxpmw, r.height());
	    // submenu indicator column
	    sr.setCoords(r.right() - maxpmw, r.top(), r.right(), r.bottom());
	    // tab/accelerator column
	    tr.setCoords(sr.left() - tab - 4, r.top(), sr.left(), r.bottom());
	    // item column
	    ir.setCoords(cr.right() + 4, r.top(), tr.right() - 4, r.bottom());

	    bool reverse = TQApplication::reverseLayout();
	    if ( reverse ) {
		cr = visualRect( cr, r );
		sr = visualRect( sr, r );
		tr = visualRect( tr, r );
		ir = visualRect( ir, r );
	    }

	    if (mi->isChecked() &&
		! (flags & Style_Active) &
		(flags & Style_Enabled))
		drawPrimitive(PE_MenuItemIndicatorFrame, p, ceData, elementFlags, r, cg, flags, data);

	    if (mi->iconSet()) {
		TQIconSet::Mode mode =
		    (flags & Style_Enabled) ? TQIconSet::Normal : TQIconSet::Disabled;
		if ((flags & Style_Active) && (flags & Style_Enabled))
		    mode = TQIconSet::Active;
		TQPixmap pixmap;
		if ((elementFlags & CEF_IsCheckable) && mi->isChecked())
		    pixmap =
			mi->iconSet()->pixmap( TQIconSet::Small, mode, TQIconSet::On );
		else
		    pixmap =
			mi->iconSet()->pixmap( TQIconSet::Small, mode );
		TQRect pmr(TQPoint(0, 0), pixmap.size());
		pmr.moveCenter(cr.center());
		p->setPen(cg.text());
		p->drawPixmap(pmr.topLeft(), pixmap);
	    } else if ((elementFlags & CEF_IsCheckable) && mi->isChecked())
		drawPrimitive(PE_MenuItemIndicatorCheck, p, ceData, elementFlags, r, cg, flags, data);

	    TQColor textcolor;
	    TQColor embosscolor;
	    if (flags & Style_Active) {
		if (! (flags & Style_Enabled))
		    textcolor = cg.midlight().dark();
		else
		    textcolor = cg.buttonText();
		embosscolor = cg.midlight().light();
	    } else if (! (flags & Style_Enabled)) {
		textcolor = cg.text();
		embosscolor = cg.light();
	    } else
		textcolor = embosscolor = cg.buttonText();
	    p->setPen(textcolor);

	    if (mi->custom()) {
		p->save();
		if (! (flags & Style_Enabled)) {
		    p->setPen(cg.light());
		    mi->custom()->paint(p, cg, flags & Style_Active,
					flags & Style_Enabled,
					ir.x() + 1, ir.y() + 1,
					ir.width() - 1, ir.height() - 1);
		    p->setPen(textcolor);
		}
		mi->custom()->paint(p, cg, flags & Style_Active,
				    flags & Style_Enabled,
				    ir.x(), ir.y(),
				    ir.width(), ir.height());
		p->restore();
	    }

	    TQString text = mi->text();
	    if (! text.isNull()) {
		int t = text.find('\t');

		// draw accelerator/tab-text
		if (t >= 0) {
		    int alignFlag = AlignVCenter | ShowPrefix | DontClip | SingleLine;
		    alignFlag |= ( reverse ? AlignLeft : AlignRight );
		    if (! (flags & Style_Enabled)) {
			p->setPen(embosscolor);
			tr.moveBy(1, 1);
			p->drawText(tr, alignFlag, text.mid(t + 1));
			tr.moveBy(-1, -1);
			p->setPen(textcolor);
		    }

		    p->drawText(tr, alignFlag, text.mid(t + 1));
		}

		int alignFlag = AlignVCenter | ShowPrefix | DontClip | SingleLine;
		alignFlag |= ( reverse ? AlignRight : AlignLeft );

		if (! (flags & Style_Enabled)) {
		    p->setPen(embosscolor);
		    ir.moveBy(1, 1);
		    p->drawText(ir, alignFlag, text, t);
		    ir.moveBy(-1, -1);
		    p->setPen(textcolor);
		}

		p->drawText(ir, alignFlag, text, t);
	    } else if (mi->pixmap()) {
		TQPixmap pixmap = *mi->pixmap();
		if (pixmap.depth() == 1)
		    p->setBackgroundMode(TQt::OpaqueMode);
		p->drawPixmap(ir.x(), ir.y() + (ir.height() - pixmap.height()) / 2, pixmap);
		if (pixmap.depth() == 1)
		    p->setBackgroundMode(TQt::TransparentMode);
	    }

	    if (mi->popup())
		drawPrimitive( (reverse ? PE_ArrowLeft : PE_ArrowRight), p, ceData, elementFlags, sr, cg, flags);
	    break;
	}
	
    case CE_MenuBarEmptyArea:
	{
	    p->fillRect(r, cg.brush(TQColorGroup::Button));
	    break;
	}
	
    case CE_DockWindowEmptyArea:
	{
	    p->fillRect(r, cg.brush(TQColorGroup::Button));
	    break;
	}


    case CE_MenuBarItem:
	{
	    if (flags & Style_Active)
		qDrawShadePanel(p, r, cg, true, 1, &cg.brush(TQColorGroup::Midlight));
	    else
		p->fillRect(r, cg.brush(TQColorGroup::Button));

	    if (data.isDefault())
		break;

	    TQMenuItem *mi = data.menuItem();
	    drawItem(p, r, AlignCenter | ShowPrefix | DontClip | SingleLine, cg,
		     flags & Style_Enabled, mi->pixmap(), mi->text(), -1,
		     &cg.buttonText());
	    break;
	}

    case CE_ProgressBarGroove:
	drawLightBevel(p, r, cg, Style_Sunken, &cg.brush(TQColorGroup::Background));
	break;

    default:
	TQCommonStyle::drawControl(control, p, ceData, elementFlags, r, cg, flags, data, widget);
	break;
    }
}

void LightStyleV2::drawControlMask( ControlElement control,
				  TQPainter *p,
				  const TQStyleControlElementData &ceData,
				  ControlElementFlags elementFlags,
				  const TQRect &r,
				  const TQStyleOption &data,
				  const TQWidget *widget ) const
{
    switch (control) {
    case CE_PushButton:
	p->fillRect(r, color1);
	break;

    default:
	TQCommonStyle::drawControlMask(control, p, ceData, elementFlags, r, data, widget);
	break;
    }
}

TQRect LightStyleV2::subRect(SubRect subrect, const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, const TQWidget *widget) const
{
    TQRect rect, wrect(ceData.rect);

    switch (subrect) {
    case SR_PushButtonFocusRect:
 	{
 	    int dbw1 = 0, dbw2 = 0;
 	    if ((elementFlags & CEF_IsDefault) || (elementFlags & CEF_AutoDefault)) {
 		dbw1 = pixelMetric(PM_ButtonDefaultIndicator, ceData, elementFlags, widget);
 		dbw2 = dbw1 * 2;
 	    }

 	    rect.setRect(wrect.x()      + 3 + dbw1,
 			 wrect.y()      + 3 + dbw1,
 			 wrect.width()  - 6 - dbw2,
 			 wrect.height() - 6 - dbw2);
 	    break;
 	}

    default:
	rect = TQCommonStyle::subRect(subrect, ceData, elementFlags, widget);
    }

    return rect;
}

void LightStyleV2::drawComplexControl( ComplexControl control,
				     TQPainter* p,
				     const TQStyleControlElementData &ceData,
				     ControlElementFlags elementFlags,
				     const TQRect& r,
				     const TQColorGroup& cg,
				     SFlags flags,
				     SCFlags controls,
				     SCFlags active,
				     const TQStyleOption &data,
				     const TQWidget* widget ) const
{
    switch (control) {
    case CC_ComboBox:
	{
	    const TQComboBox *combobox = (const TQComboBox *) widget;
	    TQRect frame, arrow, field;
	    frame =
		TQStyle::visualRect(querySubControlMetrics(CC_ComboBox, ceData, elementFlags,
							  SC_ComboBoxFrame, data, widget),
				   ceData, elementFlags);
	    arrow =
		TQStyle::visualRect(querySubControlMetrics(CC_ComboBox, ceData, elementFlags,
							  SC_ComboBoxArrow, data, widget),
				   ceData, elementFlags);
	    field =
		TQStyle::visualRect(querySubControlMetrics(CC_ComboBox, ceData, elementFlags,
							  SC_ComboBoxEditField, data, widget),
				   ceData, elementFlags);

	    if ((controls & SC_ComboBoxFrame) && frame.isValid())
		drawLightBevel(p, frame, cg, flags | Style_Raised,
			       &cg.brush(TQColorGroup::Button));

	    if ((controls & SC_ComboBoxArrow) && arrow.isValid()) {
		if (active == SC_ComboBoxArrow)
		    p->fillRect(arrow, cg.brush(TQColorGroup::Mid));
		arrow.addCoords(4, 2, -2, -2);
		drawPrimitive(PE_ArrowDown, p, ceData, elementFlags, arrow, cg, flags);
	    }

	    if ((controls & SC_ComboBoxEditField) && field.isValid()) {
		p->setPen(cg.dark());
		if (elementFlags & CEF_IsEditable) {
		    field.addCoords(-1, -1, 1, 1);
		    p->drawRect(field);
		} else
		    p->drawLine(field.right() + 1, field.top(),
				field.right() + 1, field.bottom());

		if (flags & Style_HasFocus) {
		    if (! (elementFlags & CEF_IsEditable)) {
			p->fillRect( field, cg.brush( TQColorGroup::Highlight ) );
			TQRect fr =
			    TQStyle::visualRect( subRect( SR_ComboBoxFocusRect, ceData, elementFlags, widget ),
						ceData, elementFlags );
			drawPrimitive( PE_FocusRect, p, ceData, elementFlags, fr, cg,
				       flags | Style_FocusAtBorder,
				       TQStyleOption(cg.highlight()));
		    }

		    p->setPen(cg.highlightedText());
		} else
		    p->setPen(cg.buttonText());
	    }

	    break;
	}

    case CC_SpinWidget:
	{
	    const TQSpinWidget *spinwidget = (const TQSpinWidget *) widget;
	    TQRect frame, up, down;

	    frame = querySubControlMetrics((ComplexControl)CC_SpinWidget, ceData, elementFlags,
					   SC_SpinWidgetFrame, data, widget);
	    up = ceData.spinWidgetData.upRect;
	    down = ceData.spinWidgetData.downRect;

	    if ((controls & SC_SpinWidgetFrame) && frame.isValid())
		drawLightBevel(p, frame, cg, flags | Style_Sunken,
			       &cg.brush(TQColorGroup::Base));

	    if ((controls & SC_SpinWidgetUp) && up.isValid()) {
		PrimitiveElement pe = PE_SpinWidgetUp;
		if ( ceData.spinWidgetData.buttonSymbols == TQSpinWidget::PlusMinus )
		    pe = PE_SpinWidgetPlus;

		p->setPen(cg.dark());
		p->drawLine(up.topLeft(), up.bottomLeft());

		up.addCoords(1, 0, 0, 0);
		p->fillRect(up, cg.brush(TQColorGroup::Button));
		if (active == SC_SpinWidgetUp)
		    p->setPen(cg.mid());
		else
		    p->setPen(cg.light());
		p->drawLine(up.left(), up.top(),
			    up.right() - 1, up.top());
		p->drawLine(up.left(), up.top() + 1,
			    up.left(), up.bottom() - 1);
		if (active == SC_SpinWidgetUp)
		    p->setPen(cg.light());
		else
		    p->setPen(cg.mid());
		p->drawLine(up.right(), up.top(),
			    up.right(), up.bottom());
		p->drawLine(up.left(), up.bottom(),
			    up.right() - 1, up.bottom());

		up.addCoords(1, 0, 0, 0);
		drawPrimitive(pe, p, ceData, elementFlags, up, cg, flags |
			      ((active == SC_SpinWidgetUp) ?
			       Style_On | Style_Sunken : Style_Raised));
	    }

	    if ((controls & SC_SpinWidgetDown) && down.isValid()) {
		PrimitiveElement pe = PE_SpinWidgetDown;
		if ( ceData.spinWidgetData.buttonSymbols == TQSpinWidget::PlusMinus )
		    pe = PE_SpinWidgetMinus;

		p->setPen(cg.dark());
		p->drawLine(down.topLeft(), down.bottomLeft());

		down.addCoords(1, 0, 0, 0);
		p->fillRect(down, cg.brush(TQColorGroup::Button));
		if (active == SC_SpinWidgetDown)
		    p->setPen(cg.mid());
		else
		    p->setPen(cg.light());
		p->drawLine(down.left(), down.top(),
			    down.right() - 1, down.top());
		p->drawLine(down.left(), down.top() + 1,
			    down.left(), down.bottom() - 1);
		if (active == SC_SpinWidgetDown)
		    p->setPen(cg.light());
		else
		    p->setPen(cg.mid());
		p->drawLine(down.right(), down.top(),
			    down.right(), down.bottom());
		p->drawLine(down.left(), down.bottom(),
			    down.right() - 1, down.bottom());

		down.addCoords(1, 0, 0, 0);
		drawPrimitive(pe, p, ceData, elementFlags, down, cg, flags |
			      ((active == SC_SpinWidgetDown) ?
			       Style_On | Style_Sunken : Style_Raised));
	    }

	    break;
	}

    case CC_ScrollBar:
	{
	    TQRect addline, subline, subline2, addpage, subpage, slider, first, last;
	    bool maxedOut = (ceData.minSteps == ceData.maxSteps);

	    subline = querySubControlMetrics(control, ceData, elementFlags, SC_ScrollBarSubLine, data, widget);
	    addline = querySubControlMetrics(control, ceData, elementFlags, SC_ScrollBarAddLine, data, widget);
	    subpage = querySubControlMetrics(control, ceData, elementFlags, SC_ScrollBarSubPage, data, widget);
	    addpage = querySubControlMetrics(control, ceData, elementFlags, SC_ScrollBarAddPage, data, widget);
	    slider  = querySubControlMetrics(control, ceData, elementFlags, SC_ScrollBarSlider,  data, widget);
	    first   = querySubControlMetrics(control, ceData, elementFlags, SC_ScrollBarFirst,   data, widget);
	    last    = querySubControlMetrics(control, ceData, elementFlags, SC_ScrollBarLast,    data, widget);

	    subline2 = addline;
	    if (ceData.orientation == TQt::Horizontal)
		subline2.moveBy(-addline.width(), 0);
	    else
		subline2.moveBy(0, -addline.height());

       	    if ((controls & SC_ScrollBarSubLine) && subline.isValid()) {
		drawPrimitive(PE_ScrollBarSubLine, p, ceData, elementFlags, subline, cg,
			      Style_Enabled | ((active == SC_ScrollBarSubLine) ?
					       Style_Down : Style_Default) |
			      ((ceData.orientation == TQt::Horizontal) ?
			       Style_Horizontal : 0));

		if (subline2.isValid())
		    drawPrimitive(PE_ScrollBarSubLine, p, ceData, elementFlags, subline2, cg,
				  Style_Enabled | ((active == SC_ScrollBarSubLine) ?
						   Style_Down : Style_Default) |
				  ((ceData.orientation == TQt::Horizontal) ?
				   Style_Horizontal : 0));
	    }
	    if ((controls & SC_ScrollBarAddLine) && addline.isValid())
		drawPrimitive(PE_ScrollBarAddLine, p, ceData, elementFlags, addline, cg,
			      Style_Enabled | ((active == SC_ScrollBarAddLine) ?
					       Style_Down : Style_Default) |
			      ((ceData.orientation == TQt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarSubPage) && subpage.isValid())
		drawPrimitive(PE_ScrollBarSubPage, p, ceData, elementFlags, subpage, cg,
			      Style_Enabled | ((active == SC_ScrollBarSubPage) ?
					       Style_Down : Style_Default) |
			      ((ceData.orientation == TQt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarAddPage) && addpage.isValid())
		drawPrimitive(PE_ScrollBarAddPage, p, ceData, elementFlags, addpage, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarAddPage) ?
			       Style_Down : Style_Default) |
			      ((ceData.orientation == TQt::Horizontal) ?
			       Style_Horizontal : 0));
       	    if ((controls & SC_ScrollBarFirst) && first.isValid())
		drawPrimitive(PE_ScrollBarFirst, p, ceData, elementFlags, first, cg,
			      Style_Enabled | ((active == SC_ScrollBarFirst) ?
					       Style_Down : Style_Default) |
			      ((ceData.orientation == TQt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarLast) && last.isValid())
		drawPrimitive(PE_ScrollBarLast, p, ceData, elementFlags, last, cg,
			      Style_Enabled | ((active == SC_ScrollBarLast) ?
					       Style_Down : Style_Default) |
			      ((ceData.orientation == TQt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarSlider) && slider.isValid()) {
		drawPrimitive(PE_ScrollBarSlider, p, ceData, elementFlags, slider, cg,
			      Style_Enabled | ((active == SC_ScrollBarSlider) ?
					       Style_Down : Style_Default) |
			      ((ceData.orientation == TQt::Horizontal) ?
			       Style_Horizontal : 0));

		// ### perhaps this should not be able to accept focus if maxedOut?
		if (elementFlags & CEF_HasFocus) {
		    TQRect fr(slider.x() + 2, slider.y() + 2,
			     slider.width() - 5, slider.height() - 5);
		    drawPrimitive(PE_FocusRect, p, ceData, elementFlags, fr, cg, Style_Default);
		}
	    }

	    break;
	}

    case CC_Slider:
	{
	    TQRect groove = querySubControlMetrics(CC_Slider, ceData, elementFlags, SC_SliderGroove,
						  data, widget),
		  handle = querySubControlMetrics(CC_Slider, ceData, elementFlags, SC_SliderHandle,
						  data, widget);

	    if ((controls & SC_SliderGroove) && groove.isValid()) {
		if (flags & Style_HasFocus)
		    drawPrimitive( PE_FocusRect, p, ceData, elementFlags, groove, cg );

		if (ceData.orientation == TQt::Horizontal) {
		    int dh = (groove.height() - 5) / 2;
		    groove.addCoords(0, dh, 0, -dh);
		} else {
		    int dw = (groove.width() - 5) / 2;
		    groove.addCoords(dw, 0, -dw, 0);
		}

		drawLightBevel(p, groove, cg, ((flags | Style_Raised) ^ Style_Raised) |
			       ((flags & Style_Enabled) ? Style_Sunken : Style_Default),
			       &cg.brush(TQColorGroup::Midlight));
	    }

	    if ((controls & SC_SliderHandle) && handle.isValid()) {
		drawLightBevel(p, handle, cg, ((flags | Style_Down) ^ Style_Down) |
			       ((flags & Style_Enabled) ? Style_Raised : Style_Default),
			       &cg.brush(TQColorGroup::Button));

	    }

	    if (controls & SC_SliderTickmarks)
		TQCommonStyle::drawComplexControl(control, p, ceData, elementFlags, r, cg, flags,
						 SC_SliderTickmarks, active, data, widget );
	    break;
	}

    case CC_ListView:
	// use the base style for CC_ListView
	singleton->basestyle->drawComplexControl(control, p, ceData, elementFlags, r, cg, flags,
						 controls, active, data, widget);
	break;

    default:
	TQCommonStyle::drawComplexControl(control, p, ceData, elementFlags, r, cg, flags,
					 controls, active, data, widget);
	break;
    }
}

TQRect LightStyleV2::querySubControlMetrics( ComplexControl control,
					  const TQStyleControlElementData &ceData,
					  ControlElementFlags elementFlags,
					  SubControl sc,
					  const TQStyleOption &data,
					  const TQWidget *widget ) const
{
    TQRect ret;

    switch (control) {
    case CC_ScrollBar:
	{
	    int sliderstart = ceData.startStep;
	    int sbextent = pixelMetric(PM_ScrollBarExtent, ceData, elementFlags, widget);
	    int maxlen = ((ceData.orientation == TQt::Horizontal) ?
			  ceData.rect.width() : ceData.rect.height()) - (sbextent * 3);
	    int sliderlen;

	    // calculate slider length
	    if (ceData.maxSteps != ceData.minSteps) {
		uint range = ceData.maxSteps - ceData.minSteps;
		sliderlen = (ceData.pageStep * maxlen) /
			    (range + ceData.pageStep);

		int slidermin = pixelMetric( PM_ScrollBarSliderMin, ceData, elementFlags, widget );
		if ( sliderlen < slidermin || range > INT_MAX / 2 )
		    sliderlen = slidermin;
		if ( sliderlen > maxlen )
		    sliderlen = maxlen;
	    } else
		sliderlen = maxlen;

	    switch (sc) {
	    case SC_ScrollBarSubLine:
		// top/left button
		ret.setRect(0, 0, sbextent, sbextent);
		break;

	    case SC_ScrollBarAddLine:
		// bottom/right button
		if (ceData.orientation == TQt::Horizontal)
		    ret.setRect(ceData.rect.width() - sbextent, 0, sbextent, sbextent);
		else
		    ret.setRect(0, ceData.rect.height() - sbextent, sbextent, sbextent);
		break;

	    case SC_ScrollBarSubPage:
		// between top/left button and slider
		if (ceData.orientation == TQt::Horizontal)
		    ret.setRect(sbextent, 0, sliderstart - sbextent, sbextent);
		else
		    ret.setRect(0, sbextent, sbextent, sliderstart - sbextent);
		break;

	    case SC_ScrollBarAddPage:
		// between bottom/right button and slider
		if (ceData.orientation == TQt::Horizontal)
		    ret.setRect(sliderstart + sliderlen, 0,
				maxlen - sliderstart - sliderlen + sbextent, sbextent);
		else
		    ret.setRect(0, sliderstart + sliderlen,
				sbextent, maxlen - sliderstart - sliderlen + sbextent);
		break;

	    case SC_ScrollBarGroove:
		if (ceData.orientation == TQt::Horizontal)
		    ret.setRect(sbextent, 0, ceData.rect.width() - sbextent * 3,
				ceData.rect.height());
		else
		    ret.setRect(0, sbextent, ceData.rect.width(),
				ceData.rect.height() - sbextent * 3);
		break;

	    case SC_ScrollBarSlider:
		if (ceData.orientation == TQt::Horizontal)
		    ret.setRect(sliderstart, 0, sliderlen, sbextent);
		else
		    ret.setRect(0, sliderstart, sbextent, sliderlen);
		break;

	    default:
		break;
	    }

	    break;
	}

    default:
	ret = TQCommonStyle::querySubControlMetrics(control, ceData, elementFlags, sc, data, widget);
	break;
    }

    return ret;
}

TQStyle::SubControl LightStyleV2::querySubControl( ComplexControl control,
						const TQStyleControlElementData &ceData,
						ControlElementFlags elementFlags,
						const TQPoint &pos,
						const TQStyleOption &data,
						const TQWidget *widget ) const
{
    TQStyle::SubControl ret = TQCommonStyle::querySubControl(control, ceData, elementFlags, pos, data, widget);

    // this is an ugly hack, but i really don't care, it's the quickest way to
    // enabled the third button
    if (control == CC_ScrollBar &&
	ret == SC_None)
	ret = SC_ScrollBarSubLine;

    return ret;
}

int LightStyleV2::pixelMetric( PixelMetric metric, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags,
			     const TQWidget *widget ) const
{
    int ret;

    switch (metric) {
    case PM_ButtonMargin:
	ret = 4;
	break;

    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 0;
	break;

    case PM_ButtonDefaultIndicator:
    case PM_DefaultFrameWidth:
	ret = 2;
	break;

    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
	ret = 13;
	break;

    case PM_TabBarTabOverlap:
	ret = 0;
	break;

    case PM_ScrollBarExtent:
    case PM_ScrollBarSliderMin:
	ret = 14;
	break;

    case PM_MenuBarFrameWidth:
	ret = 1;
	break;

    case PM_ProgressBarChunkWidth:
	ret = 1;
	break;

    case PM_DockWindowSeparatorExtent:
	ret = 4;
	break;
	
    case PM_SplitterWidth:
	ret = 6;
	break;


    case PM_SliderLength:
    case PM_SliderControlThickness:
	ret = singleton->basestyle->pixelMetric( metric, ceData, elementFlags, widget );
	break;

    case PM_MaximumDragDistance:
	ret = -1;
	break;

    case PM_MenuIndicatorFrameHBorder:
    case PM_MenuIndicatorFrameVBorder:
    case PM_MenuIconIndicatorFrameHBorder:
    case PM_MenuIconIndicatorFrameVBorder:
	ret = 0;
	break;

    default:
	ret = TQCommonStyle::pixelMetric(metric, ceData, elementFlags, widget);
	break;
    }

    return ret;
}

TQSize LightStyleV2::sizeFromContents( ContentsType contents,
				    const TQStyleControlElementData &ceData,
				    ControlElementFlags elementFlags,
				    const TQSize &contentsSize,
				    const TQStyleOption &data,
				    const TQWidget *widget ) const
{
    TQSize ret;

    switch (contents) {
    case CT_PushButton:
	{
	    const TQPushButton *button = (const TQPushButton *) widget;
	    ret = TQCommonStyle::sizeFromContents( contents, ceData, elementFlags, contentsSize, data, widget );
	    int w = ret.width(), h = ret.height();

	    // only expand the button if we are displaying text...
	    if ( ! button->text().isEmpty() ) {
		if ( (elementFlags & CEF_IsDefault) || button->autoDefault() ) {
		    // default button minimum size
		    if ( w < 80 )
			w = 80;
		    if ( h < 25 )
			h = 25;
		} else {
		    // regular button minimum size
		    if ( w < 76 )
			w = 76;
		    if ( h < 21 )
			h = 21;
		}
	    }

	    ret = TQSize( w, h );
	    break;
	}

    case CT_PopupMenuItem:
	{
	    if (data.isDefault())
		break;

	    TQMenuItem *mi = data.menuItem();
	    int maxpmw = data.maxIconWidth();
	    int w = contentsSize.width(), h = contentsSize.height();

	    if (mi->custom()) {
		w = mi->custom()->sizeHint().width();
		h = mi->custom()->sizeHint().height();
		if (! mi->custom()->fullSpan() && h < 22)
		    h = 22;
	    } else if(mi->widget()) {
	    } else if (mi->isSeparator()) {
		w = 10;
		h = 4;
	    } else {
		// check is at least 16x16
		if (h < 16)
		    h = 16;
		if (mi->pixmap())
		    h = TQMAX(h, mi->pixmap()->height());
		else if (! mi->text().isNull())
		    h = TQMAX(h, TQFontMetrics(ceData.font).height() + 2);
		if (mi->iconSet() != 0)
		    h = TQMAX(h, mi->iconSet()->pixmap(TQIconSet::Small,
						      TQIconSet::Normal).height());
		h += 2;
	    }

	    // check | 4 pixels | item | 8 pixels | accel | 4 pixels | check

	    // check is at least 16x16
	    maxpmw = TQMAX(maxpmw, 16);
	    w += (maxpmw * 2) + 8;

	    if (! mi->text().isNull() && mi->text().find('\t') >= 0)
		w += 8;

	    ret = TQSize(w, h);
	    break;
	}
    case CT_ProgressBar:
	{
	    //If we have to display the indicator, and we do it on RHS, give some more room
	    //for it. This tries to match the logic and the spacing in SR_ProgressBarGroove/Contents
	    //sizing in TQCommonStyle.
	    if (ceData.percentageVisible && 
	        ((elementFlags & CEF_IndicatorFollowsStyle) || ! (elementFlags & CEF_CenterIndicator)))
	    {
		int addw = TQFontMetrics(ceData.font).width("100%") + 6;
		return TQSize(contentsSize.width() + addw, contentsSize.height());
	    }
	    else
	    	return contentsSize; //Otherwise leave unchanged

	    break;
	}

    default:
	ret = TQCommonStyle::sizeFromContents(contents, ceData, elementFlags, contentsSize, data, widget);
	break;
    }

    return ret;
}

int LightStyleV2::styleHint( StyleHint stylehint,
			   const TQStyleControlElementData &ceData,
			   ControlElementFlags elementFlags,
			   const TQStyleOption &option,
			   TQStyleHintReturn* returnData,
			   const TQWidget *widget ) const
{
    int ret;

    switch (stylehint) {
    case SH_EtchDisabledText:
    case SH_Slider_SnapToValue:
    case SH_PrintDialog_RightAlignButtons:
    case SH_FontDialog_SelectAssociatedText:
    case SH_MenuBar_AltKeyNavigation:
    case SH_MenuBar_MouseTracking:
    case SH_PopupMenu_MouseTracking:
    case SH_ComboBox_ListMouseTracking:
    case SH_ScrollBar_MiddleClickAbsolutePosition:
	ret = 1;
	break;

    case SH_MainWindow_SpaceBelowMenuBar:
	ret = 0;
	break;

    case SH_MenuIndicatorColumnWidth:
	{
		int maxpmw = option.maxIconWidth();
		maxpmw = TQMAX(maxpmw, 16);
	
		ret = maxpmw;
	}
	break;

    default:
	ret = TQCommonStyle::styleHint(stylehint, ceData, elementFlags, option, returnData, widget);
	break;
    }

    return ret;
}

TQPixmap LightStyleV2::stylePixmap( StylePixmap stylepixmap,
				   const TQStyleControlElementData &ceData,
				   ControlElementFlags elementFlags,
				   const TQStyleOption &data,
				   const TQWidget *widget ) const
{
    return singleton->basestyle->stylePixmap( stylepixmap, ceData, elementFlags, data, widget );
}
#include "lightstyle-v2.moc"
