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

#include "lightstyle-v3.h"

#include "tqmenubar.h"
#include "tqapplication.h"
#include "tqcheckbox.h"
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


// The Light Style, 3rd revision

LightStyleV3::LightStyleV3()
    : TDEStyle(AllowMenuTransparency)
{
    basestyle = TQStyleFactory::create( "Windows" );
    if ( ! basestyle )
	basestyle = TQStyleFactory::create( TQStyleFactory::keys().first() );
    if ( ! basestyle )
	tqFatal( "LightStyle: couldn't find a basestyle!" );
}

LightStyleV3::~LightStyleV3()
{
    delete basestyle;
}

void LightStyleV3::polishPopupMenu( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr )
{
    TDEStyle::polishPopupMenu(ceData, elementFlags, ptr);
    // empty to satisy pure virtual requirements
}

/*
  A LightBevel looks like this:

  EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
  ESSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSE
  ESBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBSE
  ESSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSE
  EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE

  where:
      E is the sunken etching ( optional, not drawn by default )
      S is the border (optional, drawn by default )
      B is the bevel (draw with the line width, minus the width of
                      the etching and border )
      F is the fill ( optional, not drawn by default )
*/
static void drawLightEtch( TQPainter *p,
			   const TQRect &rect,
			   const TQColor &color,
			   bool sunken )
{
    TQPointArray pts( 4 );

    pts.setPoint( 0, rect.left(),     rect.bottom() - 1);
    pts.setPoint( 1, rect.left(),     rect.top() );
    pts.setPoint( 2, rect.left() + 1, rect.top() );
    pts.setPoint( 3, rect.right(),    rect.top() );
    p->setPen( sunken ? color.dark( 115 ) : color.light( 115 ) );
    p->drawLineSegments( pts );

    pts.setPoint( 0, rect.left(),     rect.bottom() );
    pts.setPoint( 1, rect.right(),    rect.bottom() );
    pts.setPoint( 2, rect.right(),    rect.bottom() - 1 );
    pts.setPoint( 3, rect.right(),    rect.top() + 1 );
    p->setPen( sunken ? color.light( 115 ) : color.dark( 115 ) );
    p->drawLineSegments( pts );
}

static void drawLightBevel( TQPainter *p,
			    const TQRect &rect,
			    const TQColorGroup &cg,
			    TQStyle::SFlags flags,
 			    int linewidth,
			    bool etch = false,       // light sunken bevel around border
			    bool border = true,      // rectangle around bevel
			    const TQBrush *fill = 0 ) // contents fill
{
    TQRect br = rect;
    bool bevel = ( flags & ( TQStyle::Style_Down | TQStyle::Style_On |
			     TQStyle::Style_Sunken | TQStyle::Style_Raised ) );
    bool sunken = (flags & (TQStyle::Style_Down | TQStyle::Style_On |
			    TQStyle::Style_Sunken));

    if ( etch && linewidth > 0 ) {
	drawLightEtch( p, br, cg.background(), true );
	linewidth--;
	br.addCoords( 1, 1, -1, -1 );
    }

    if ( ! br.isValid() )
	return;
    if ( border && linewidth > 0 ) {
	p->setPen( cg.dark() );
	p->drawRect( br );
	linewidth--;
	br.addCoords( 1, 1, -1, -1 );
    }

    if ( ! br.isValid() )
	return;
    if ( bevel && linewidth > 0 ) {
	// draw a bevel
	int x, y, w, h;
	br.rect( &x, &y, &w, &h );

	// copied form qDrawShadePanel - just changed the highlight colors...
	TQPointArray a( 4*linewidth );
	if ( sunken )
	    p->setPen( border ? cg.mid() : cg.dark() );
	else
	    p->setPen( cg.light() );
	int x1, y1, x2, y2;
	int i;
	int n = 0;
	x1 = x;
	y1 = y2 = y;
	x2 = x+w-2;
	for ( i=0; i<linewidth; i++ ) {		// top shadow
	    a.setPoint( n++, x1, y1++ );
	    a.setPoint( n++, x2--, y2++ );
	}
	x2 = x1;
	y1 = y+h-2;
	for ( i=0; i<linewidth; i++ ) {		// left shadow
	    a.setPoint( n++, x1++, y1 );
	    a.setPoint( n++, x2++, y2-- );
	}
	p->drawLineSegments( a );
	n = 0;
	if ( sunken )
	    p->setPen( cg.light() );
	else
	    p->setPen( border ? cg.mid() : cg.dark() );
	x1 = x;
	y1 = y2 = y+h-1;
	x2 = x+w-1;
	for ( i=0; i<linewidth; i++ ) {		// bottom shadow
	    a.setPoint( n++, x1++, y1-- );
	    a.setPoint( n++, x2, y2-- );
	}
	x1 = x2;
	y1 = y;
	y2 = y+h-linewidth-1;
	for ( i=0; i<linewidth; i++ ) {		// right shadow
	    a.setPoint( n++, x1--, y1++ );
	    a.setPoint( n++, x2--, y2 );
	}
	p->drawLineSegments( a );

	br.addCoords( linewidth, linewidth, -linewidth, -linewidth );
    }

    // fill
    if ( fill )
	p->fillRect( br, *fill );
}

void LightStyleV3::drawPrimitive( PrimitiveElement pe,
				TQPainter *p,
				const TQStyleControlElementData &ceData,
				ControlElementFlags elementFlags,
				const TQRect &r,
				const TQColorGroup &cg,
				SFlags flags,
				const TQStyleOption &data ) const
{
    TQRect br = r;
    const TQBrush *fill = 0;

    switch (pe) {
    case PE_HeaderSectionMenu:
    case PE_HeaderSection:
	// don't draw any headers sunken
	flags = ((flags | Style_Sunken) ^ Style_Sunken) | Style_Raised;

	p->setPen( cg.background() );
	// hard border at the bottom/right of the header
	if ( flags & Style_Horizontal ) {
	    p->drawLine( br.bottomLeft(), br.bottomRight() );
	    br.addCoords( 0, 0, 0, -1 );
	} else {
	    p->drawLine( br.topRight(), br.bottomRight() );
	    br.addCoords( 0, 0, -1, 0 );
	}

	// draw the header ( just an etching )
	if ( ! br.isValid() )
	    break;
	drawLightEtch( p, br, ( ( flags & Style_Down ) ?
                                cg.midlight() : cg.button() ),
                       ( flags & Style_Down ) );
	br.addCoords( 1, 1, -1, -1 );

	// fill the header
	if ( ! br.isValid() )
	    break;
	p->fillRect( br, cg.brush( ( flags & Style_Down ) ?
				   TQColorGroup::Midlight : TQColorGroup::Button ) );

	// the taskbuttons in kicker seem to allow the style to set the pencolor
	// here, which will be used to draw the text for focused window buttons...
	// how utterly silly
	p->setPen( cg.buttonText() );
	break;

    case PE_ButtonCommand:
	{
	    TQRect br = r;

	    if (flags & TQStyle::Style_Enabled) {
		if (flags & (TQStyle::Style_Down |
			     TQStyle::Style_On |
			     TQStyle::Style_Sunken))
		    fill = &cg.brush(TQColorGroup::Midlight);
		else
		    fill = &cg.brush(TQColorGroup::Button);
	    } else
		fill = &cg.brush(TQColorGroup::Background);

	    bool etch = true;
	    if ( flags & Style_ButtonDefault ) {
		etch = false;
		br.addCoords( 1, 1, -1, -1 );
	    }
	    drawLightBevel( p, br, cg, flags,
			    pixelMetric( PM_DefaultFrameWidth, ceData, elementFlags ) + ( etch ? 1 : 0 ),
			    etch, true, fill );
	    break;
	}

    case PE_ButtonBevel:
    case PE_ButtonTool:
	if (flags & TQStyle::Style_Enabled) {
	    if (flags & (TQStyle::Style_Down |
			 TQStyle::Style_On |
			 TQStyle::Style_Sunken))
		fill = &cg.brush(TQColorGroup::Midlight);
	    else
		fill = &cg.brush(TQColorGroup::Button);
	} else
	    fill = &cg.brush(TQColorGroup::Background);
	drawLightBevel( p, r, cg, flags, pixelMetric( PM_DefaultFrameWidth, ceData, elementFlags ),
			false, true, fill );
	break;

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

	    p->setPen( cg.dark() );
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
	p->setPen( cg.shadow() );
	p->drawRect( r );
	break;

    case PE_Indicator:
	const TQBrush *fill;
	if (! (flags & Style_Enabled))
	    fill = &cg.brush(TQColorGroup::Background);
	else if (flags & Style_Down)
	    fill = &cg.brush(TQColorGroup::Mid);
	else
	    fill = &cg.brush(TQColorGroup::Base);
	drawLightBevel( p, r, cg, flags | Style_Sunken, 2, true, true, fill );

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
		  lr = r, // outline rect
		  cr = r, // contents rect
		  ir = r; // indicator rect
	    lr.addCoords( 1, 1, -1, -1 );
	    cr.addCoords( 2, 2, -2, -2 );
	    ir.addCoords( 3, 3, -3, -3 );

	    p->fillRect( r, cg.brush( TQColorGroup::Background ) );

	    p->setPen( flags & Style_Down ? cg.mid() :
		       ( flags & Style_Enabled ? cg.base() : cg.background() ) );
	    p->setBrush( flags & Style_Down ? cg.mid() :
			 ( flags & Style_Enabled ? cg.base() : cg.background() ) );
	    p->drawEllipse( lr );

	    p->setPen( cg.background().dark( 115 ) );
	    p->drawArc( br, 45*16, 180*16 );
	    p->setPen( cg.background().light( 115 ) );
	    p->drawArc( br, 235*16, 180*16 );

	    p->setPen( cg.dark() );
	    p->drawArc( lr, 0, 16*360 );

	    if ( flags & Style_On ) {
		p->setPen( flags & Style_Down ? cg.mid() :
			   ( flags & Style_Enabled ? cg.base() : cg.background() ) );
		p->setBrush( cg.text() );
		p->drawEllipse( ir );
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
		    for ( int i = r.left() - 1; i < r.right(); i += 3 ) {
			p->setPen( cg.midlight() );
			p->drawLine( i, r.top(), i, r.bottom() );
			p->setPen( cg.background() );
			p->drawLine( i + 1, r.top(), i + 1, r.bottom() );
			p->setPen( cg.mid() );
			p->drawLine( i + 2, r.top(), i + 2, r.bottom() );
		    }
		}
	    } else {
		if (drawTitle) {
		    p->fillRect(r, cg.brush(TQColorGroup::Highlight));
		    p->setPen(cg.highlightedText());
		    p->drawText(r, AlignCenter, title);
		} else {
		    for ( int i = r.top() - 1; i < r.bottom(); i += 3 ) {
			p->setPen( cg.midlight() );
			p->drawLine( r.left(), i, r.right(), i );
			p->setPen( cg.background() );
			p->drawLine( r.left(), i + 1, r.right(), i + 1);
			p->setPen( cg.mid() );
			p->drawLine( r.left(), i + 2, r.right(), i + 2 );
		    }

		}
	    }
	    break;
	}

    case PE_DockWindowSeparator:
	{
	    if (flags & Style_Horizontal) {
		int hw = r.width() / 2;
		p->setPen( cg.mid() );
		p->drawLine( hw,     r.top() + 6, hw,     r.bottom() - 6 );
		p->setPen( cg.light() );
		p->drawLine( hw + 1, r.top() + 6, hw + 1, r.bottom() - 6 );
	    } else {
		int hh = r.height() / 2;
		p->setPen( cg.mid() );
		p->drawLine( r.left() + 6, hh,     r.right() - 6, hh     );
		p->setPen( cg.light() );
		p->drawLine( r.left() + 6, hh + 1, r.right() - 6, hh + 1 );
	    }
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
	    TQRect br = r;

	    p->setPen( cg.shadow() );
	    p->drawRect( br );

	    br.addCoords( 1, 1, -1, -1 );

	    if ( ! br.isValid() )
		break;
	    p->setPen( cg.light() );
	    p->drawLine( br.left(), br.top(), br.right() - 1, br.top() );
	    p->drawLine( br.left(), br.top() + 1, br.left(), br.bottom() );
	    p->setPen( cg.mid() );
	    p->drawLine( br.bottomLeft(), br.bottomRight() );
	    p->drawLine( br.right(), br.top(), br.right(), br.bottom() - 1 );

	    br.addCoords( 1, 1, -1, -1 );

	    if ( ! br.isValid() )
		break;
	    p->fillRect( br, cg.brush( TQColorGroup::Button ) );
	    break;
	}

    case PE_PanelPopup:
	drawLightBevel( p, r, cg, flags,
			( data.isDefault() ? pixelMetric(PM_DefaultFrameWidth, ceData, elementFlags) :
			  data.lineWidth() ), false, true );
	break;

    case PE_Panel:
    case PE_PanelLineEdit:
    case PE_PanelTabWidget:
    case PE_WindowFrame:
	{
	    TQRect br = r;

	    int cover = 0;
	    int reallw = ( data.isDefault() ?
			   pixelMetric( PM_DefaultFrameWidth, ceData, elementFlags ) : data.lineWidth() );
	    cover = reallw - 1;

	    if ( ! ( flags & Style_Sunken ) )
		flags |= Style_Raised;
	    drawLightBevel( p, br, cg, flags, 1, false, false );
	    br.addCoords( 1, 1, -1, -1 );

	    while ( cover-- > 0 ) {
		TQPointArray pts( 8 );
		pts.setPoint( 0, br.left(),     br.bottom() - 1);
		pts.setPoint( 1, br.left(),     br.top() );
		pts.setPoint( 2, br.left() + 1, br.top() );
		pts.setPoint( 3, br.right(),    br.top() );
		pts.setPoint( 4, br.left(),     br.bottom() );
		pts.setPoint( 5, br.right(),    br.bottom() );
		pts.setPoint( 6, br.right(),    br.bottom() - 1 );
		pts.setPoint( 7, br.right(),    br.top() + 1 );
		p->setPen( cg.background() );
		p->drawLineSegments( pts );

		br.addCoords( 1, 1, -1, -1 );
	    }
	    break;
	}

    case PE_PanelDockWindow:
	drawLightBevel( p, r, cg, flags, ( data.isDefault() ?
					   pixelMetric(PM_DefaultFrameWidth, ceData, elementFlags) :
					   data.lineWidth() ), false, false,
			&cg.brush( TQColorGroup::Button ) );
	break;

    case PE_PanelMenuBar:
	drawLightBevel( p, r, cg, flags, ( data.isDefault() ?
					   pixelMetric(PM_MenuBarFrameWidth, ceData, elementFlags) :
					   data.lineWidth() ), false, false,
			&cg.brush( TQColorGroup::Button ) );
	break;

    case PE_ScrollBarSubLine:
	{
	    TQRect br = r;
	    PrimitiveElement pe;

	    p->setPen( cg.background() );
	    if (flags & Style_Horizontal) {
		pe = PE_ArrowLeft;
		p->drawLine( br.topLeft(), br.topRight() );
		br.addCoords( 0, 1, 0, 0 );
	    } else {
		pe = PE_ArrowUp;
		p->drawLine( br.topLeft(), br.bottomLeft() );
		br.addCoords( 1, 0, 0, 0 );
	    }

	    if ( ! br.isValid() )
		break;
	    drawLightEtch( p, br, cg.button(), false );
	    br.addCoords( 1, 1, -1, -1 );

	    if ( ! br.isValid() )
		break;
	    p->fillRect( br, cg.brush( ( flags & Style_Down ) ?
				       TQColorGroup::Midlight :
				       TQColorGroup::Button ) );
	    br.addCoords( 2, 2, -2, -2 );

	    if ( ! br.isValid() )
		break;
	    drawPrimitive( pe, p, ceData, elementFlags, br, cg, flags );
	    break;
	}

    case PE_ScrollBarAddLine:
	{
	    TQRect br = r;
	    PrimitiveElement pe;

	    p->setPen( cg.background() );
	    if (flags & Style_Horizontal) {
		pe = PE_ArrowRight;
		p->drawLine( br.topLeft(), br.topRight() );
		br.addCoords( 0, 1, 0, 0 );
	    } else {
		pe = PE_ArrowDown;
		p->drawLine( br.topLeft(), br.bottomLeft() );
		br.addCoords( 1, 0, 0, 0 );
	    }

	    if ( ! br.isValid() )
		break;
	    drawLightEtch( p, br, cg.button(), false );
	    br.addCoords( 1, 1, -1, -1 );

	    if ( ! br.isValid() )
		break;
	    p->fillRect( br, cg.brush( ( flags & Style_Down ) ?
				       TQColorGroup::Midlight :
				       TQColorGroup::Button ) );
	    br.addCoords( 2, 2, -2, -2 );

	    if ( ! br.isValid() )
		break;
	    drawPrimitive( pe, p, ceData, elementFlags, br, cg, flags );
	    break;
	}

    case PE_ScrollBarSubPage:
	{
	    TQRect br = r;

	    p->setPen( cg.background() );
	    if (flags & Style_Horizontal) {
		p->drawLine( br.topLeft(), br.topRight() );
		br.addCoords( 0, 1, 0, 0 );
	    } else {
		p->drawLine( br.topLeft(), br.bottomLeft() );
		br.addCoords( 1, 0, 0, 0 );
	    }

	    if ( ! br.isValid() )
		break;
	    drawLightEtch( p, br, cg.button(), false );
	    br.addCoords( 1, 1, -1, -1 );

	    if ( ! br.isValid() )
		break;
	    p->fillRect( br, cg.brush( ( flags & Style_Down ) ?
				       TQColorGroup::Midlight :
				       TQColorGroup::Button ) );
	    break;
	}

    case PE_ScrollBarAddPage:
	{
	    TQRect br = r;

	    p->setPen( cg.background() );
	    if (flags & Style_Horizontal) {
		p->drawLine( br.topLeft(), br.topRight() );
		br.addCoords( 0, 1, 0, 0 );
	    } else {
		p->drawLine( br.topLeft(), br.bottomLeft() );
		br.addCoords( 1, 0, 0, 0 );
	    }

	    if ( ! br.isValid() )
		break;
	    drawLightEtch( p, br, cg.button(), false );
	    br.addCoords( 1, 1, -1, -1 );

	    if ( ! br.isValid() )
		break;
	    p->fillRect( br, cg.brush( ( flags & Style_Down ) ?
				       TQColorGroup::Midlight :
				       TQColorGroup::Button ) );
	    break;
	}

    case PE_ScrollBarSlider:
	{
	    TQRect br = r;

	    p->setPen( cg.background() );
	    if (flags & Style_Horizontal) {
		p->drawLine( br.topLeft(), br.topRight() );
		br.addCoords( 0, 1, 0, 0 );
	    } else {
		p->drawLine( br.topLeft(), br.bottomLeft() );
		br.addCoords( 1, 0, 0, 0 );
	    }

	    if ( ! br.isValid() )
		break;
	    p->setPen( cg.highlight().light() );
	    p->drawLine( br.topLeft(), br.topRight() );
	    p->drawLine( br.left(), br.top() + 1, br.left(), br.bottom() - 1 );

	    p->setPen( cg.highlight().dark() );
	    p->drawLine( br.left(), br.bottom(), br.right() - 1, br.bottom() );
	    p->drawLine( br.topRight(), br.bottomRight() );
	    br.addCoords( 1, 1, -1, -1 );

	    p->fillRect( br, cg.brush( TQColorGroup::Highlight ) );
	    break;
	}

    case PE_FocusRect:
	p->setBrush( NoBrush );
	if ( flags & Style_FocusAtBorder )
	    p->setPen( cg.shadow() );
	else
	    p->setPen( cg.dark() );
	p->drawRect( r );
	break;

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

void LightStyleV3::drawControl( ControlElement control,
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
	    TQRect br = r;

	    if ( ceData.tabBarData.shape == TQTabBar::RoundedAbove ) {
		if ( ! ( flags & Style_Selected ) ) {
		    p->setPen( cg.background() );
		    p->drawLine( br.left(),  br.bottom(),
				 br.right(), br.bottom() );
		    p->setPen( cg.light() );
		    p->drawLine( br.left(),  br.bottom() - 1,
				 br.right(), br.bottom() - 1);
		    br.addCoords( 0, 2, -1, -2 );
		    if ( br.left() == 0 )
			p->drawPoint( br.left(), br.bottom() + 2 );
		} else {
		    p->setPen( cg.background() );
		    p->drawLine( br.bottomLeft(), br.bottomRight() );
		    if ( br.left() == 0 ) {
			p->setPen( cg.light() );
			p->drawPoint( br.bottomLeft() );
		    }
		    br.addCoords( 0, 0, 0, -1 );
		}

		p->setPen( cg.light() );
		p->drawLine( br.bottomLeft(), br.topLeft() );
		p->drawLine( br.topLeft(), br.topRight() );
		p->setPen( cg.dark() );
		p->drawLine( br.right(), br.top() + 1, br.right(), br.bottom() );

		if ( flags & Style_Selected )
		{
                    p->fillRect( br.right() - 3, br.top() + 1, 3, br.height() - 1, cg.brush(TQColorGroup::Highlight));
		    br.addCoords( 1, 1, -4, 0 );
		}
		else
		    br.addCoords( 1, 1, -1, 0 );
		p->fillRect( br, cg.background() );
	    } else if ( ceData.tabBarData.shape == TQTabBar::RoundedBelow ) {
		if ( ! ( flags & Style_Selected ) ) {
		    p->setPen( cg.background() );
		    p->drawLine( br.left(),  br.top(),
				 br.right(), br.top() );
		    p->setPen( cg.dark() );
		    p->drawLine( br.left(),  br.top() + 1,
				 br.right(), br.top() + 1);
		    br.addCoords( 0, 2, -1, -2 );
		    if ( br.left() == 0 ) {
			p->setPen( cg.light() );
			p->drawPoint( br.left(), br.top() - 2 );
		    }
		} else {
		    p->setPen( cg.background() );
		    p->drawLine( br.topLeft(), br.topRight() );
		    if ( br.left() == 0 ) {
			p->setPen( cg.light() );
			p->drawPoint( br.topLeft() );
		    }
		    br.addCoords( 0, 1, 0, 0 );
		}

		p->setPen( cg.light() );
		p->drawLine( br.topLeft(), br.bottomLeft() );
		p->setPen( cg.dark() );
		p->drawLine( br.bottomLeft(), br.bottomRight() );
		p->drawLine( br.right(), br.top(), br.right(), br.bottom() - 1 );
		br.addCoords( 1, 0, -1, -1 );

		if ( flags & Style_Selected )
		{
		    p->fillRect( br.right() - 2, br.top(), 3, br.height(), cg.brush(TQColorGroup::Highlight));
		    br.addCoords( 1, 0, -3, -1 );
		}
		else
		    br.addCoords( 1, 0, -1, -1 );

		p->fillRect( br, cg.background() );
	    } else
		TQCommonStyle::drawControl( control, p, ceData, elementFlags, r, cg, flags, data, widget );
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
            	if ( !ceData.bgPixmap.isNull() )
                	p->drawPixmap( r.topLeft(), ceData.bgPixmap, r );
		else
			p->fillRect(r, cg.brush(TQColorGroup::Button));
		p->setPen( cg.mid() );
		p->drawLine(r.left() + 12,  r.top() + 1,
			    r.right() - 12, r.top() + 1);
		p->setPen( cg.light() );
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
		drawPrimitive( (TQApplication::reverseLayout() ? PE_ArrowLeft : PE_ArrowRight),
			       p, ceData, elementFlags, sr, cg, flags);
	    break;
	}

    case CE_MenuBarEmptyArea:
	{
	    p->fillRect(r, cg.brush(TQColorGroup::Button));
	    break;
	}

    case CE_MenuBarItem:
	{
	    if ( flags & Style_Active )
		qDrawShadePanel(p, r, cg, true, 1, &cg.brush(TQColorGroup::Midlight));
	    else
		p->fillRect( r, cg.brush( TQColorGroup::Button ) );

	    if (data.isDefault())
		break;

	    TQMenuItem *mi = data.menuItem();
	    drawItem(p, r, AlignCenter | ShowPrefix | DontClip | SingleLine, cg,
		     flags & Style_Enabled, mi->pixmap(), mi->text(), -1,
		     &cg.buttonText());
	    break;
	}

    case CE_ProgressBarGroove:
	drawLightBevel( p, r, cg, Style_Sunken, pixelMetric( PM_DefaultFrameWidth, ceData, elementFlags ),
			true, true, &cg.brush( TQColorGroup::Background ) );
	break;

    default:
	TQCommonStyle::drawControl(control, p, ceData, elementFlags, r, cg, flags, data, widget);
	break;
    }
}

void LightStyleV3::drawControlMask( ControlElement control,
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

TQRect LightStyleV3::subRect(SubRect subrect, const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, const TQWidget *widget) const
{
    TQRect rect;

    switch (subrect) {
    case SR_PushButtonFocusRect:
	{
	    rect = TQCommonStyle::subRect( SR_PushButtonContents, ceData, elementFlags, widget );
	    int bm = pixelMetric( PM_ButtonMargin, ceData, elementFlags, widget ), hbm = bm / 2;
	    rect.addCoords( hbm, hbm, -hbm, -hbm );
  	    break;
  	}

    case SR_ComboBoxFocusRect:
	{
	    rect = TQCommonStyle::subRect( SR_ComboBoxFocusRect, ceData, elementFlags, widget );
	    rect.addCoords( -1, -1, 1, 1 );
	    break;
	}

	case SR_CheckBoxFocusRect:
	{
		const TQCheckBox* cb = static_cast<const TQCheckBox*>(widget);

		//Only checkbox, no label
		if (cb->text().isEmpty() && (cb->pixmap() == 0) )
		{
			TQRect bounding = cb->rect();
			TQRect checkbox(bounding.x(), bounding.y() + (bounding.height() - 13)/2,
							13, 13);

			return checkbox;
		}
		//Fallthrough intentional
	}


    default:
	rect = TQCommonStyle::subRect(subrect, ceData, elementFlags, widget);
	break;
    }

    return rect;
}

void LightStyleV3::drawComplexControl( ComplexControl control,
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
		drawPrimitive( PE_Panel, p, ceData, elementFlags, frame, cg, flags | Style_Sunken );

	    if ((controls & SC_ComboBoxArrow) && arrow.isValid()) {
		drawLightEtch( p, arrow, cg.button(), ( active == SC_ComboBoxArrow ) );
		arrow.addCoords( 1, 1, -1, -1 );
		p->fillRect( arrow, cg.brush( TQColorGroup::Button ) );
		arrow.addCoords(3, 1, -1, -1);
		drawPrimitive(PE_ArrowDown, p, ceData, elementFlags, arrow, cg, flags);
	    }

	    if ((controls & SC_ComboBoxEditField) && field.isValid()) {
		if (flags & Style_HasFocus) {
		    if (! (elementFlags & CEF_IsEditable)) {
			TQRect fr =
			    TQStyle::visualRect( subRect( SR_ComboBoxFocusRect, ceData, elementFlags, widget ),
						ceData, elementFlags );
			p->fillRect( fr, cg.brush( TQColorGroup::Highlight ) );
			drawPrimitive( PE_FocusRect, p, ceData, elementFlags, fr, cg,
				       flags | Style_FocusAtBorder,
				       TQStyleOption(cg.highlight()));
		    }

		    p->setPen(cg.highlightedText());
		} else {
		    p->fillRect( field, ( ( flags & Style_Enabled ) ?
					  cg.brush( TQColorGroup::Base ) :
					  cg.brush( TQColorGroup::Background ) ) );
		    p->setPen( cg.text() );
		}
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
		drawPrimitive( PE_Panel, p, ceData, elementFlags, frame, cg, flags | Style_Sunken );

	    if ((controls & SC_SpinWidgetUp) && up.isValid()) {
		PrimitiveElement pe = PE_SpinWidgetUp;
		if ( ceData.spinWidgetData.buttonSymbols == TQSpinWidget::PlusMinus )
		    pe = PE_SpinWidgetPlus;

		p->setPen( cg.background() );
		p->drawLine( up.topLeft(), up.bottomLeft() );

		up.addCoords( 1, 0, 0, 0 );
		p->fillRect( up, cg.brush( TQColorGroup::Button ) );
		drawLightEtch( p, up, cg.button(), ( active == SC_SpinWidgetUp ) );

		up.addCoords( 1, 0, 0, 0 );
		drawPrimitive(pe, p, ceData, elementFlags, up, cg, flags |
			      ((active == SC_SpinWidgetUp) ?
			       Style_On | Style_Sunken : Style_Raised));
	    }

	    if ((controls & SC_SpinWidgetDown) && down.isValid()) {
		PrimitiveElement pe = PE_SpinWidgetDown;
		if ( ceData.spinWidgetData.buttonSymbols == TQSpinWidget::PlusMinus )
		    pe = PE_SpinWidgetMinus;

		p->setPen( cg.background() );
		p->drawLine( down.topLeft(), down.bottomLeft() );

		down.addCoords( 1, 0, 0, 0 );
		p->fillRect( down, cg.brush( TQColorGroup::Button ) );
		drawLightEtch( p, down, cg.button(), ( active == SC_SpinWidgetDown ) );

		down.addCoords( 1, 0, 0, 0 );
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
	    const TQSlider *slider = (const TQSlider *) widget;
	    TQRect groove = querySubControlMetrics(CC_Slider, ceData, elementFlags, SC_SliderGroove,
						  data, widget),
		  handle = querySubControlMetrics(CC_Slider, ceData, elementFlags, SC_SliderHandle,
						  data, widget);

	    if ((controls & SC_SliderGroove) && groove.isValid()) {
		TQColor grooveColor = cg.midlight();
		if (!(flags & Style_Enabled))
		    grooveColor = cg.background();


		TQBrush brush(grooveColor);
		drawLightBevel( p, groove, cg,
				( ( flags | Style_Raised ) ^ Style_Raised ) |
				( ( flags & Style_Enabled ) ? Style_Sunken :
				  Style_Default ), 2, true, true,
				&brush );
		groove.addCoords( 2, 2, -2, -2 );
		drawLightEtch( p, groove, grooveColor, false );

		if (flags & Style_HasFocus) {
		    groove.addCoords( -2, -2, 2, 2 );
		    drawPrimitive( PE_FocusRect, p, ceData, elementFlags, groove, cg, flags );
		}
	    }

	    if ((controls & SC_SliderHandle) && handle.isValid()) {
		TQColor sliderColor = cg.highlight();
		if (!(flags & Style_Enabled))
		    sliderColor = cg.button();

		p->setPen( sliderColor.light() );

		p->drawLine( handle.topLeft(), handle.topRight() );
		p->drawLine( handle.left(), handle.top() + 1,
			     handle.left(), handle.bottom() - 1 );
		p->setPen( sliderColor.dark() );
		p->drawLine( handle.left(), handle.bottom(),
			     handle.right() - 1, handle.bottom() );
		p->drawLine( handle.topRight(), handle.bottomRight() );
		handle.addCoords( 1, 1, -1, -1 );
		p->fillRect( handle, sliderColor );
		p->setPen( cg.midlight() );

		if ( ceData.orientation == TQt::Horizontal )
		    p->drawLine( handle.left() + handle.width() / 2,
				 handle.top() + 1,
				 handle.left() + handle.width() / 2,
				 handle.bottom() - 1 );
		else
		    p->drawLine( handle.left() + 1,
				 handle.top() + handle.height() / 2,
				 handle.right() - 1,
				 handle.top() + handle.height() / 2 );
	    }

	    if (controls & SC_SliderTickmarks)
		TQCommonStyle::drawComplexControl(control, p, ceData, elementFlags, r, cg, flags,
						 SC_SliderTickmarks, active, data, widget );
	    break;
	}

    case CC_ListView:
	// use the base style for CC_ListView
	basestyle->drawComplexControl(control, p, ceData, elementFlags, r, cg, flags,
				      controls, active, data, widget);
	break;

    default:
	TQCommonStyle::drawComplexControl(control, p, ceData, elementFlags, r, cg, flags,
				      controls, active, data, widget);
	break;
    }
}

TQRect LightStyleV3::querySubControlMetrics( ComplexControl control,
					  const TQStyleControlElementData &ceData,
					  ControlElementFlags elementFlags,
					  SubControl sc,
					  const TQStyleOption &data,
					  const TQWidget *widget ) const
{
    TQRect ret;

    switch (control) {
    case CC_ComboBox:
	{
	    int fw = pixelMetric( PM_DefaultFrameWidth, ceData, elementFlags, widget );
	    int sb = pixelMetric( PM_ScrollBarExtent, ceData, elementFlags ); // width of the arrow

	    switch ( sc ) {
	    case SC_ComboBoxFrame:
		ret = ceData.rect;
		break;
	    case SC_ComboBoxArrow:
		ret.setRect( ceData.rect.width() - fw - sb, fw,
			     sb, ceData.rect.height() - fw*2 );
		break;
	    case SC_ComboBoxEditField:
		ret.setRect( fw, fw, ceData.rect.width() - fw*2 - sb - 1,
			     ceData.rect.height() - fw*2 );
		break;
	    default:
		break;
	    }

	    break;
	}

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
		    ret.setRect(sliderstart + sliderlen, 0, maxlen - sliderstart -
				sliderlen + sbextent, sbextent);
		else
		    ret.setRect(0, sliderstart + sliderlen, sbextent, maxlen -
				sliderstart - sliderlen + sbextent);
		break;

	    case SC_ScrollBarGroove:
		if (ceData.orientation == TQt::Horizontal)
		    ret.setRect(sbextent, 0, maxlen, sbextent );
		else
		    ret.setRect(0, sbextent, sbextent, maxlen );
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

    case CC_Slider:
	{
	    const TQSlider *slider = (const TQSlider *) widget;
	    int tickOffset = pixelMetric( PM_SliderTickmarkOffset, ceData, elementFlags, widget );
	    int thickness = pixelMetric( PM_SliderControlThickness, ceData, elementFlags, widget );

	    switch ( sc ) {
	    case SC_SliderGroove:
		if ( ceData.orientation == TQt::Horizontal )
		    ret.setRect( 0, tickOffset, ceData.rect.width(), thickness );
		else
		    ret.setRect( tickOffset, 0, thickness, ceData.rect.height() );
		break;

	    case SC_SliderHandle:
		{
		    int pos = ceData.startStep;
		    int len = pixelMetric( PM_SliderLength, ceData, elementFlags, widget );

		    if ( ceData.orientation == TQt::Horizontal )
			ret.setRect( pos + 2, tickOffset + 2, len - 4, thickness - 4 );
		    else
			ret.setRect( tickOffset + 2, pos + 2, thickness - 4, len - 4 );
		    break;
		}

	    default:
		ret = TQCommonStyle::querySubControlMetrics(control, ceData, elementFlags, sc, data, widget);
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

TQStyle::SubControl LightStyleV3::querySubControl( ComplexControl control,
						const TQStyleControlElementData &ceData,
						ControlElementFlags elementFlags,
						const TQPoint &pos,
						const TQStyleOption &data,
						const TQWidget *widget ) const
{
    TQStyle::SubControl ret =
	TQCommonStyle::querySubControl(control, ceData, elementFlags, pos, data, widget);

    // this is an ugly hack, but i really don't care, it's the quickest way to
    // enabled the third button
    if (control == CC_ScrollBar &&
	ret == SC_None)
	ret = SC_ScrollBarSubLine;

    return ret;
}

int LightStyleV3::pixelMetric( PixelMetric metric, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags,
			     const TQWidget *widget ) const
{
    int ret;

    switch (metric) {
    case PM_ButtonMargin:
	ret = 6;
	break;

    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 0;
	break;

    case PM_ButtonDefaultIndicator:
	ret = 0;
	break;

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
    case PM_TabBarBaseOverlap:
	ret = 0;
	break;

    case PM_ScrollBarExtent:
    case PM_ScrollBarSliderMin:
	ret = 15;
	break;

    case PM_MenuBarFrameWidth:
	ret = 1;
	break;

    case PM_ProgressBarChunkWidth:
	ret = 1;
	break;

    case PM_DockWindowHandleExtent:
	ret = 8;
	break;

    case PM_DockWindowSeparatorExtent:
	ret = 8;
	break;

    case PM_SplitterWidth:
	ret = 8;
	break;

    case PM_SliderLength:
	ret = 25;
	break;

    case PM_SliderThickness:
	ret = 11;
	break;

    case PM_SliderControlThickness:
	{
	    int space = (ceData.orientation == TQt::Horizontal) ? ceData.rect.height()
			: ceData.rect.width();
	    int ticks = ceData.tickMarkSetting;
	    int n = 0;
	    if ( ticks & TQSlider::Above ) n++;
	    if ( ticks & TQSlider::Below ) n++;
	    if ( !n ) {
		ret = space;
		break;
	    }

	    int thick = 6;	// Magic constant to get 5 + 16 + 5

	    space -= thick;
	    //### the two sides may be unequal in size
	    if ( space > 0 )
		thick += (space * 2) / (n + 2);
	    ret = thick;
	    break;
	}

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

TQSize LightStyleV3::sizeFromContents( ContentsType contents,
				    const TQStyleControlElementData &ceData,
				    ControlElementFlags elementFlags,
				    const TQSize &contentsSize,
				    const TQStyleOption &data,
				    const TQWidget *widget ) const
{
    TQSize ret;

    switch (contents) {
    case CT_ComboBox:
	{
	    int fw = pixelMetric( PM_DefaultFrameWidth, ceData, elementFlags, widget ) * 2;
	    int sb = pixelMetric( PM_ScrollBarExtent, ceData, elementFlags ); // width of the arrow
	    int w = contentsSize.width();
	    int h = contentsSize.height();

	    w += fw + sb + 1;
	    h += fw;

	    // try to keep a similar height to buttons
	    if ( h < 21 )
		h = 21;

	    ret = TQSize( w, h );
	    break;
	}

    case CT_PushButton:
	{
	    const TQPushButton *button = (const TQPushButton *) widget;
	    ret = TQCommonStyle::sizeFromContents( contents, ceData, elementFlags, contentsSize, data, widget );
	    int w = ret.width(), h = ret.height();
	    int dbi = pixelMetric( PM_ButtonDefaultIndicator, ceData, elementFlags, widget ) * 2;
	    int mw = 80 - dbi, mh = 25 - dbi;

	    // only expand the button if we are displaying text...
	    if ( ! button->text().isEmpty() ) {
		// button minimum size
		if ( w < mw )
		    w = mw;
		if ( h < mh )
		    h = mh;
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

int LightStyleV3::styleHint( StyleHint stylehint,
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

    case SH_ScrollBar_BackgroundMode:
	ret = NoBackground;
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

TQPixmap LightStyleV3::stylePixmap( StylePixmap stylepixmap,
				   const TQStyleControlElementData &ceData,
				   ControlElementFlags elementFlags,
				   const TQStyleOption &data,
				   const TQWidget *widget ) const
{
    return basestyle->stylePixmap( stylepixmap, ceData, elementFlags, data, widget );
}
#include "lightstyle-v3.moc"
