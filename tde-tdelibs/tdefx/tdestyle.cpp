/*
 *
 * TDEStyle
 * Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>
 *
 * TQWindowsStyle CC_ListView and style images were kindly donated by TrollTech,
 * Copyright (C) 1998-2000 TrollTech AS.
 *
 * Many thanks to Bradley T. Hughes for the 3 button scrollbar code.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tdestyle.h"

#include <tqapplication.h>
#include <tqbitmap.h>
#include <tqmetaobject.h>
#include <tqcleanuphandler.h>
#include <tqmap.h>
#include <tqimage.h>
#include <tqlistview.h>
#include <tqmenubar.h>
#include <tqpainter.h>
#include <tqpixmap.h>
#include <tqpopupmenu.h>
#include <tqprogressbar.h>
#include <tqscrollbar.h>
#include <tqsettings.h>
#include <tqslider.h>
#include <tqstylefactory.h>
#include <tqtabbar.h>
#include <tqtoolbar.h>
#include <tqframe.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <kimageeffect.h>

#ifdef TQ_WS_X11
# include <X11/Xlib.h>
# ifdef HAVE_XRENDER
#  include <X11/extensions/Xrender.h> // schroder
   extern bool tqt_use_xrender;
# endif
#else
#undef HAVE_XRENDER
#endif

#ifdef HAVE_XCOMPOSITE
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xcomposite.h>
#include <dlfcn.h>
#endif

#include <limits.h>

namespace
{
	// INTERNAL
	enum TransparencyEngine {
		Disabled = 0,
		SoftwareTint,
		SoftwareBlend,
		XRender
	};

	// Drop Shadow
	struct ShadowElements {
		TQWidget* w1;
		TQWidget* w2;
	};
	typedef TQMap<const TQWidget*,ShadowElements> ShadowMap;
        static ShadowMap *_shadowMap = 0;
        TQSingleCleanupHandler<ShadowMap> cleanupShadowMap;
        ShadowMap &shadowMap() {
	    if ( !_shadowMap ) {
		_shadowMap = new ShadowMap;
		cleanupShadowMap.set( &_shadowMap );
	    }
	    return *_shadowMap;
	}


	// DO NOT ASK ME HOW I MADE THESE TABLES!
	// (I probably won't remember anyway ;)
	const double top_right_corner[16] =
		{ 0.949, 0.965, 0.980, 0.992,
		  0.851, 0.890, 0.945, 0.980,
		  0.706, 0.780, 0.890, 0.960,
		  0.608, 0.706, 0.851, 0.949 };

	const double bottom_right_corner[16] =
		{ 0.608, 0.706, 0.851, 0.949,
		  0.706, 0.780, 0.890, 0.960,
		  0.851, 0.890, 0.945, 0.980,
		  0.949, 0.965, 0.980, 0.992 };

	const double bottom_left_corner[16] =
		{ 0.949, 0.851, 0.706, 0.608,
		  0.965, 0.890, 0.780, 0.706,
		  0.980, 0.945, 0.890, 0.851,
		  0.992, 0.980, 0.960, 0.949 };

	const double shadow_strip[4] =
		{ 0.565, 0.675, 0.835, 0.945 };

	static bool useDropShadow(TQWidget* w)
	{
		return w && w->metaObject() && 
			w->metaObject()->findProperty("TDEStyleMenuDropShadow") != -1;
	}
}

namespace
{
class TransparencyHandler : public TQObject
{
	public:
		TransparencyHandler(TDEStyle* style, TransparencyEngine tEngine,
							float menuOpacity, bool useDropShadow);
		~TransparencyHandler();
		bool eventFilter(TQObject* object, TQEvent* event);

	protected:
		void blendToColor(const TQColor &col);
		void blendToPixmap(const TQColorGroup &cg, const TQWidget* p);
#ifdef HAVE_XRENDER
		void XRenderBlendToPixmap(const TQWidget* p);
#endif
		bool haveX11RGBASupport();
		TQImage handleRealAlpha(TQImage);
		void createShadowWindows(const TQWidget* p);
		void removeShadowWindows(const TQWidget* p);
		void rightShadow(TQImage& dst);
		void bottomShadow(TQImage& dst);
	private:
		bool    dropShadow;
		float   opacity;
		TQPixmap pix;
		TDEStyle* tdestyle;
		TransparencyEngine te;
};
} // namespace

struct TDEStylePrivate
{
	bool  highcolor                : 1;
	bool  useFilledFrameWorkaround : 1;
	bool  etchDisabledText         : 1;
	bool  scrollablePopupmenus     : 1;
	bool  autoHideAccelerators     : 1;
	bool  menuAltKeyNavigation     : 1;
	bool  menuDropShadow           : 1;
	bool  sloppySubMenus           : 1;
	bool  semiTransparentRubberband : 1;
	int   popupMenuDelay;
	float menuOpacity;

	TransparencyEngine   transparencyEngine;
	TDEStyle::TDEStyleScrollBarType  scrollbarType;
	TransparencyHandler* menuHandler;
	TDEStyle::TDEStyleFlags flags;
	
	//For KPE_ListViewBranch
	TQBitmap *verticalLine;
	TQBitmap *horizontalLine;
};

// -----------------------------------------------------------------------------


TDEStyle::TDEStyle( TDEStyleFlags flags, TDEStyleScrollBarType sbtype )
	: TQCommonStyle(), d(new TDEStylePrivate)
{
	d->flags = flags;
	bool useMenuTransparency    = (flags & AllowMenuTransparency);
	d->useFilledFrameWorkaround = (flags & FilledFrameWorkaround);
	d->scrollbarType = sbtype;
	d->highcolor = TQPixmap::defaultDepth() > 8;

	// Read style settings
	TQSettings settings;
	d->popupMenuDelay       = settings.readNumEntry ("/TDEStyle/Settings/PopupMenuDelay", 250);
	d->sloppySubMenus       = settings.readBoolEntry("/TDEStyle/Settings/SloppySubMenus", false);
	d->etchDisabledText     = settings.readBoolEntry("/TDEStyle/Settings/EtchDisabledText", true);
	d->menuAltKeyNavigation = settings.readBoolEntry("/TDEStyle/Settings/MenuAltKeyNavigation", true);
	d->scrollablePopupmenus = settings.readBoolEntry("/TDEStyle/Settings/ScrollablePopupMenus", false);
	d->autoHideAccelerators = settings.readBoolEntry("/TDEStyle/Settings/AutoHideAccelerators", false);
	d->menuDropShadow       = settings.readBoolEntry("/TDEStyle/Settings/MenuDropShadow", false);
	d->semiTransparentRubberband = settings.readBoolEntry("/TDEStyle/Settings/SemiTransparentRubberband", false);
	d->menuHandler = NULL;

	if (useMenuTransparency) {
		TQString effectEngine = settings.readEntry("/TDEStyle/Settings/MenuTransparencyEngine", "Disabled");

#ifdef HAVE_XRENDER
		if (effectEngine == "XRender")
			d->transparencyEngine = XRender;
#else
		if (effectEngine == "XRender")
			d->transparencyEngine = SoftwareBlend;
#endif
		else if (effectEngine == "SoftwareBlend")
			d->transparencyEngine = SoftwareBlend;
		else if (effectEngine == "SoftwareTint")
			d->transparencyEngine = SoftwareTint;
		else
			d->transparencyEngine = Disabled;

		if (d->transparencyEngine != Disabled) {
			// Create an instance of the menu transparency handler
			d->menuOpacity = settings.readDoubleEntry("/TDEStyle/Settings/MenuOpacity", 0.90);
			d->menuHandler = new TransparencyHandler(this, d->transparencyEngine,
													 d->menuOpacity, d->menuDropShadow);
		}
	}
	
	d->verticalLine   = 0;
	d->horizontalLine = 0;

	// Create a transparency handler if only drop shadows are enabled.
	if (!d->menuHandler && d->menuDropShadow)
		d->menuHandler = new TransparencyHandler(this, Disabled, 1.0, d->menuDropShadow);
}


TDEStyle::~TDEStyle()
{
	delete d->verticalLine;
	delete d->horizontalLine;

	delete d->menuHandler;

	d->menuHandler = NULL;
	delete d;
}


TQString TDEStyle::defaultStyle()
{
	if (TQPixmap::defaultDepth() > 8)
	   return TQString("plastik");
	else
	   return TQString("light, 3rd revision");
}

void TDEStyle::polish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr )
{
	if (ceData.widgetObjectTypes.contains("TQWidget")) {
		TQWidget* widget = reinterpret_cast<TQWidget*>(ptr);
		if ( d->useFilledFrameWorkaround )
		{
			if ( TQFrame *frame = ::tqt_cast< TQFrame* >( widget ) ) {
				TQFrame::Shape shape = frame->frameShape();
				if (shape == TQFrame::ToolBarPanel || shape == TQFrame::MenuBarPanel)
					widget->installEventFilter(this);
			}
		}
		if (widget->isTopLevel())
		{
			if (!d->menuHandler && useDropShadow(widget))
				d->menuHandler = new TransparencyHandler(this, Disabled, 1.0, false);

			if (d->menuHandler && useDropShadow(widget))
				widget->installEventFilter(d->menuHandler);
		}
	}
}


void TDEStyle::unPolish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr )
{
	if (ceData.widgetObjectTypes.contains("TQWidget")) {
		TQWidget* widget = reinterpret_cast<TQWidget*>(ptr);
		if ( d->useFilledFrameWorkaround )
		{
			if ( TQFrame *frame = ::tqt_cast< TQFrame* >( widget ) ) {
				TQFrame::Shape shape = frame->frameShape();
				if (shape == TQFrame::ToolBarPanel || shape == TQFrame::MenuBarPanel)
					widget->removeEventFilter(this);
			}
		}
		if (widget->isTopLevel() && d->menuHandler && useDropShadow(widget))
			widget->removeEventFilter(d->menuHandler);
	}
}


// Style changes (should) always re-polish popups.
void TDEStyle::polishPopupMenu( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr )
{
    if ( !(ceData.windowState & WState_Polished ) ) {
        widgetActionRequest(ceData, elementFlags, ptr, WAR_SetCheckable);
    }

	if (ceData.widgetObjectTypes.contains("TQWidget")) {
		TQWidget* widget = reinterpret_cast<TQWidget*>(ptr);
		TQPopupMenu *p = dynamic_cast<TQPopupMenu*>(widget);
		if (p) {
			// Install transparency handler if the effect is enabled.
			if ( d->menuHandler && (strcmp(p->name(), "tear off menu") != 0)) {
				p->installEventFilter(d->menuHandler);
			}
		}
	}
}


// -----------------------------------------------------------------------------
// TDEStyle extensions
// -----------------------------------------------------------------------------

void TDEStyle::setScrollBarType(TDEStyleScrollBarType sbtype)
{
	d->scrollbarType = sbtype;
}

TDEStyle::TDEStyleFlags TDEStyle::styleFlags() const
{
	return d->flags;
}

void TDEStyle::renderMenuBlendPixmap( KPixmap &pix, const TQColorGroup &cg,
	const TQPopupMenu* /* popup */ ) const
{
	pix.fill(cg.button());	// Just tint as the default behavior
}

void TDEStyle::drawTDEStylePrimitive( TDEStylePrimitive kpe,
								  TQPainter* p,
								  const TQWidget* widget,
								  const TQRect &r,
								  const TQColorGroup &cg,
								  SFlags flags,
								  const TQStyleOption &opt ) const
{
	const TQStyleControlElementData &ceData = populateControlElementDataFromWidget(widget, TQStyleOption());
	drawTDEStylePrimitive(kpe, p, ceData, getControlElementFlagsForObject(widget, TQStyleOption()), r, cg, flags, opt);
}

void TDEStyle::drawTDEStylePrimitive( TDEStylePrimitive kpe,
								  TQPainter* p,
								  const TQStyleControlElementData &ceData,
								  ControlElementFlags elementFlags,
								  const TQRect &r,
								  const TQColorGroup &cg,
								  SFlags flags,
								  const TQStyleOption&, /* opt */
								  const TQWidget* widget ) const
{
	switch( kpe )
	{
		// Dock / Toolbar / General handles.
		// ---------------------------------

		case KPE_DockWindowHandle: {

			// Draws a nice DockWindow handle including the dock title.
			TQWidget* wid = const_cast<TQWidget*>(widget);
			bool horizontal = flags & Style_Horizontal;
			int x,y,w,h,x2,y2;

			r.rect( &x, &y, &w, &h );
			if ((w <= 2) || (h <= 2)) {
				p->fillRect(r, cg.highlight());
				return;
			}

			
			x2 = x + w - 1;
			y2 = y + h - 1;

			TQFont fnt;
			fnt = TQApplication::font(wid);
			fnt.setPointSize( fnt.pointSize()-2 );

			// Draw the item on an off-screen pixmap
			// to preserve Xft antialiasing for
			// vertically oriented handles.
			TQPixmap pix;
			if (horizontal)
				pix.resize( h-2, w-2 );
			else
				pix.resize( w-2, h-2 );

			TQString title = wid->parentWidget()->caption();
			TQPainter p2;
			p2.begin(&pix);
			p2.fillRect(pix.rect(), cg.brush(TQColorGroup::Highlight));
			p2.setPen(cg.highlightedText());
			p2.setFont(fnt);
			p2.drawText(pix.rect(), AlignCenter, title);
			p2.end();

			// Draw a sunken bevel
			p->setPen(cg.dark());
			p->drawLine(x, y, x2, y);
			p->drawLine(x, y, x, y2);
			p->setPen(cg.light());
			p->drawLine(x+1, y2, x2, y2);
			p->drawLine(x2, y+1, x2, y2);

			if (horizontal) {
				TQWMatrix m;
				m.rotate(-90.0);
				TQPixmap vpix = pix.xForm(m);
				bitBlt(wid, r.x()+1, r.y()+1, &vpix);
			} else
				bitBlt(wid, r.x()+1, r.y()+1, &pix);

			break;
		}


		/*
		 * KPE_ListViewExpander and KPE_ListViewBranch are based on code from
		 * QWindowStyle's CC_ListView, kindly donated by TrollTech.
		 * CC_ListView code is Copyright (C) 1998-2000 TrollTech AS.
		 */

		case KPE_ListViewExpander: {
			// Typical Windows style expand/collapse element.
			int radius = (r.width() - 4) / 2;
			int centerx = r.x() + r.width()/2;
			int centery = r.y() + r.height()/2;

			// Outer box
			p->setPen( cg.mid() );
			p->drawRect( r );

			// plus or minus
			p->setPen( cg.text() );
			p->drawLine( centerx - radius, centery, centerx + radius, centery );
			if ( flags & Style_On )	// Collapsed = On
				p->drawLine( centerx, centery - radius, centerx, centery + radius );
			break;
		}

		case KPE_ListViewBranch: {
			// Typical Windows style listview branch element (dotted line).

			// Create the dotline pixmaps if not already created
			if ( !d->verticalLine )
			{
				// make 128*1 and 1*128 bitmaps that can be used for
				// drawing the right sort of lines.
				d->verticalLine   = new TQBitmap( 1, 129, true );
				d->horizontalLine = new TQBitmap( 128, 1, true );
				TQPointArray a( 64 );
				TQPainter p2;
				p2.begin( d->verticalLine );

				int i;
				for( i=0; i < 64; i++ )
					a.setPoint( i, 0, i*2+1 );
				p2.setPen( color1 );
				p2.drawPoints( a );
				p2.end();
				TQApplication::flushX();
				d->verticalLine->setMask( *d->verticalLine );

				p2.begin( d->horizontalLine );
				for( i=0; i < 64; i++ )
					a.setPoint( i, i*2+1, 0 );
				p2.setPen( color1 );
				p2.drawPoints( a );
				p2.end();
				TQApplication::flushX();
				d->horizontalLine->setMask( *d->horizontalLine );
			}

			p->setPen( cg.text() );		// cg.dark() is bad for dark color schemes.

			if (flags & Style_Horizontal)
			{
				int point = r.x();
				int other = r.y();
				int end = r.x()+r.width();
				int thickness = r.height();

				while( point < end )
				{
					int i = 128;
					if ( i+point > end )
						i = end-point;
					p->drawPixmap( point, other, *d->horizontalLine, 0, 0, i, thickness );
					point += i;
				}

			} else {
				int point = r.y();
				int other = r.x();
				int end = r.y()+r.height();
				int thickness = r.width();
				int pixmapoffset = (flags & Style_NoChange) ? 0 : 1;	// ### Hackish

				while( point < end )
				{
					int i = 128;
					if ( i+point > end )
						i = end-point;
					p->drawPixmap( other, point, *d->verticalLine, 0, pixmapoffset, thickness, i );
					point += i;
				}
			}

			break;
		}

		// Reimplement the other primitives in your styles.
		// The current implementation just paints something visibly different.
		case KPE_ToolBarHandle:
		case KPE_GeneralHandle:
		case KPE_SliderHandle:
			p->fillRect(r, cg.light());
			break;

		case KPE_SliderGroove:
			p->fillRect(r, cg.dark());
			break;

		default:
			p->fillRect(r, TQt::yellow);	// Something really bad happened - highlight.
			break;
	}
}


int TDEStyle::kPixelMetric( TDEStylePixelMetric kpm, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQWidget* /* widget */) const
{
	int value;
	switch(kpm)
	{
		case KPM_ListViewBranchThickness:
			value = 1;
			break;

		case KPM_MenuItemSeparatorHeight:
		case KPM_MenuItemHMargin:
		case KPM_MenuItemVMargin:
		case KPM_MenuItemHFrame:
		case KPM_MenuItemVFrame:
		case KPM_MenuItemCheckMarkHMargin:
		case KPM_MenuItemArrowHMargin:
		case KPM_MenuItemTabSpacing:
		default:
			value = 0;
	}

	return value;
}

// -----------------------------------------------------------------------------

void TDEStyle::drawPrimitive( PrimitiveElement pe,
							TQPainter* p,
							const TQStyleControlElementData &ceData,
							ControlElementFlags elementFlags,
							const TQRect &r,
							const TQColorGroup &cg,
							SFlags flags,
							const TQStyleOption& opt ) const
{
	// TOOLBAR/DOCK WINDOW HANDLE
	// ------------------------------------------------------------------------
	if (pe == PE_DockWindowHandle)
	{
		// Wild workarounds are here. Beware.
		TQWidget *widget, *parent;

		if (p && p->device()->devType() == TQInternal::Widget) {
			widget = static_cast<TQWidget*>(p->device());
			parent = widget->parentWidget();
		} else
			return;		// Don't paint on non-widgets

		// Check if we are a normal toolbar or a hidden dockwidget.
		if ( parent &&
			(parent->inherits("TQToolBar") ||		// Normal toolbar
			(parent->inherits("TQMainWindow")) ))	// Collapsed dock

			// Draw a toolbar handle
			drawTDEStylePrimitive( KPE_ToolBarHandle, p, ceData, elementFlags, r, cg, flags, opt, widget );

		else if (ceData.widgetObjectTypes.contains("TQDockWindowHandle"))

			// Draw a dock window handle
			drawTDEStylePrimitive( KPE_DockWindowHandle, p, ceData, elementFlags, r, cg, flags, opt, widget );

		else
			// General handle, probably a kicker applet handle.
			drawTDEStylePrimitive( KPE_GeneralHandle, p, ceData, elementFlags, r, cg, flags, opt, widget );
#if TQT_VERSION >= 0x030300
#ifdef HAVE_XRENDER
	} else if ( d->semiTransparentRubberband && pe == TQStyle::PE_RubberBand ) {
			TQRect rect = r.normalize();
			TQPoint point;
			point = p->xForm( point );
	
			static XRenderColor clr = { 0, 0, 0, 0 };
			static unsigned long fillColor = 0;
			if ( fillColor != cg.highlight().rgb() ) {
				fillColor = cg.highlight().rgb();
				
				unsigned long color = fillColor << 8 | 0x40;

				int red = (color >> 24) & 0xff;
				int green = (color >> 16) & 0xff;
				int blue = (color >> 8) & 0xff;
				int alpha = (color >> 0) & 0xff;

				red = red * alpha / 255;
				green = green * alpha / 255;
				blue = blue * alpha / 255;

				clr.red = (red << 8) + red;
				clr.green = (green << 8) + green;
				clr.blue = (blue << 8) + blue;
				clr.alpha = (alpha << 8) + alpha;
			}
		
			XRenderFillRectangle(
					p->device()->x11Display(),
					PictOpOver,
					p->device()->x11RenderHandle(),
					&clr,
					rect.x() + point.x(),
					rect.y() + point.y(),
					rect.width(),
					rect.height() );

			p->save();
			p->setRasterOp( TQt::CopyROP );
			p->setPen( TQPen( cg.highlight().dark( 160 ), 1 ) );
			p->setBrush( NoBrush );
			p->drawRect(
					rect.x() + point.x(),
					rect.y() + point.y(),
					rect.width(),
					rect.height() );
			p->restore();
#endif
#endif
	} else
		TQCommonStyle::drawPrimitive( pe, p, ceData, elementFlags, r, cg, flags, opt );
}



void TDEStyle::drawControl( ControlElement element,
						  TQPainter* p,
						  const TQStyleControlElementData &ceData,
						  ControlElementFlags elementFlags,
						  const TQRect &r,
						  const TQColorGroup &cg,
						  SFlags flags,
						  const TQStyleOption &opt,
						  const TQWidget* widget ) const
{
	switch (element)
	{
		// TABS
		// ------------------------------------------------------------------------
		case CE_TabBarTab: {
			TQTabBar::Shape tbs = ceData.tabBarData.shape;
			bool selected      = flags & Style_Selected;
			int x = r.x(), y=r.y(), bottom=r.bottom(), right=r.right();

			switch (tbs) {

				case TQTabBar::RoundedAbove: {
					if (!selected)
						p->translate(0,1);
					p->setPen(selected ? cg.light() : cg.shadow());
					p->drawLine(x, y+4, x, bottom);
					p->drawLine(x, y+4, x+4, y);
					p->drawLine(x+4, y, right-1, y);
					if (selected)
						p->setPen(cg.shadow());
					p->drawLine(right, y+1, right, bottom);

					p->setPen(cg.midlight());
					p->drawLine(x+1, y+4, x+1, bottom);
					p->drawLine(x+1, y+4, x+4, y+1);
					p->drawLine(x+5, y+1, right-2, y+1);

					if (selected) {
						p->setPen(cg.mid());
						p->drawLine(right-1, y+1, right-1, bottom);
					} else {
						p->setPen(cg.mid());
						p->drawPoint(right-1, y+1);
						p->drawLine(x+4, y+2, right-1, y+2);
						p->drawLine(x+3, y+3, right-1, y+3);
						p->fillRect(x+2, y+4, r.width()-3, r.height()-6, cg.mid());

						p->setPen(cg.light());
						p->drawLine(x, bottom-1, right, bottom-1);
						p->translate(0,-1);
					}
					break;
				}

				case TQTabBar::RoundedBelow: {
					if (!selected)
						p->translate(0,-1);
					p->setPen(selected ? cg.light() : cg.shadow());
					p->drawLine(x, bottom-4, x, y);
					if (selected)
						p->setPen(cg.mid());
					p->drawLine(x, bottom-4, x+4, bottom);
					if (selected)
						p->setPen(cg.shadow());
					p->drawLine(x+4, bottom, right-1, bottom);
					p->drawLine(right, bottom-1, right, y);

					p->setPen(cg.midlight());
					p->drawLine(x+1, bottom-4, x+1, y);
					p->drawLine(x+1, bottom-4, x+4, bottom-1);
					p->drawLine(x+5, bottom-1, right-2, bottom-1);

					if (selected) {
						p->setPen(cg.mid());
						p->drawLine(right-1, y, right-1, bottom-1);
					} else {
						p->setPen(cg.mid());
						p->drawPoint(right-1, bottom-1);
						p->drawLine(x+4, bottom-2, right-1, bottom-2);
						p->drawLine(x+3, bottom-3, right-1, bottom-3);
						p->fillRect(x+2, y+2, r.width()-3, r.height()-6, cg.mid());
						p->translate(0,1);
						p->setPen(cg.dark());
						p->drawLine(x, y, right, y);
					}
					break;
				}

				case TQTabBar::TriangularAbove: {
					if (!selected)
						p->translate(0,1);
					p->setPen(selected ? cg.light() : cg.shadow());
					p->drawLine(x, bottom, x, y+6);
					p->drawLine(x, y+6, x+6, y);
					p->drawLine(x+6, y, right-6, y);
					if (selected)
						p->setPen(cg.mid());
					p->drawLine(right-5, y+1, right-1, y+5);
					p->setPen(cg.shadow());
					p->drawLine(right, y+6, right, bottom);

					p->setPen(cg.midlight());
					p->drawLine(x+1, bottom, x+1, y+6);
					p->drawLine(x+1, y+6, x+6, y+1);
					p->drawLine(x+6, y+1, right-6, y+1);
					p->drawLine(right-5, y+2, right-2, y+5);
					p->setPen(cg.mid());
					p->drawLine(right-1, y+6, right-1, bottom);

					TQPointArray a(6);
					a.setPoint(0, x+2, bottom);
					a.setPoint(1, x+2, y+7);
					a.setPoint(2, x+7, y+2);
					a.setPoint(3, right-7, y+2);
					a.setPoint(4, right-2, y+7);
					a.setPoint(5, right-2, bottom);
					p->setPen  (selected ? cg.background() : cg.mid());
					p->setBrush(selected ? cg.background() : cg.mid());
					p->drawPolygon(a);
					p->setBrush(NoBrush);
					if (!selected) {
						p->translate(0,-1);
						p->setPen(cg.light());
						p->drawLine(x, bottom, right, bottom);
					}
					break;
				}

				default: { // TQTabBar::TriangularBelow
					if (!selected)
						p->translate(0,-1);
					p->setPen(selected ? cg.light() : cg.shadow());
					p->drawLine(x, y, x, bottom-6);
					if (selected)
						p->setPen(cg.mid());
					p->drawLine(x, bottom-6, x+6, bottom);
					if (selected)
						p->setPen(cg.shadow());
					p->drawLine(x+6, bottom, right-6, bottom);
					p->drawLine(right-5, bottom-1, right-1, bottom-5);
					if (!selected)
						p->setPen(cg.shadow());
					p->drawLine(right, bottom-6, right, y);

					p->setPen(cg.midlight());
					p->drawLine(x+1, y, x+1, bottom-6);
					p->drawLine(x+1, bottom-6, x+6, bottom-1);
					p->drawLine(x+6, bottom-1, right-6, bottom-1);
					p->drawLine(right-5, bottom-2, right-2, bottom-5);
					p->setPen(cg.mid());
					p->drawLine(right-1, bottom-6, right-1, y);

					TQPointArray a(6);
					a.setPoint(0, x+2, y);
					a.setPoint(1, x+2, bottom-7);
					a.setPoint(2, x+7, bottom-2);
					a.setPoint(3, right-7, bottom-2);
					a.setPoint(4, right-2, bottom-7);
					a.setPoint(5, right-2, y);
					p->setPen  (selected ? cg.background() : cg.mid());
					p->setBrush(selected ? cg.background() : cg.mid());
					p->drawPolygon(a);
					p->setBrush(NoBrush);
					if (!selected) {
						p->translate(0,1);
						p->setPen(cg.dark());
						p->drawLine(x, y, right, y);
					}
					break;
				}
			};

			break;
		}
		
		// Popup menu scroller
		// ------------------------------------------------------------------------
		case CE_PopupMenuScroller: {
			p->fillRect(r, cg.background());
			drawPrimitive(PE_ButtonTool, p, ceData, elementFlags, r, cg, Style_Enabled);
			drawPrimitive((flags & Style_Up) ? PE_ArrowUp : PE_ArrowDown, p, ceData, elementFlags, r, cg, Style_Enabled);
			break;
		}


		// PROGRESSBAR
		// ------------------------------------------------------------------------
		case CE_ProgressBarGroove: {
			TQRect fr = subRect(SR_ProgressBarGroove, ceData, elementFlags, widget);
			drawPrimitive(PE_Panel, p, ceData, elementFlags, fr, cg, Style_Sunken, TQStyleOption::Default);
			break;
		}

		case CE_ProgressBarContents: {
			// ### Take into account totalSteps() for busy indicator
			TQRect cr = subRect(SR_ProgressBarContents, ceData, elementFlags, widget);
			double progress = ceData.currentStep;
			bool reverse = TQApplication::reverseLayout();
			int steps = ceData.totalSteps;

			if (!cr.isValid())
				return;

			// Draw progress bar
			if (progress > 0 || steps == 0) {
				double pg = (steps == 0) ? 0.1 : progress / steps;
				int width = TQMIN(cr.width(), (int)(pg * cr.width()));
				if (steps == 0) { //Busy indicator

					if (width < 1) width = 1; //A busy indicator with width 0 is kind of useless

					int remWidth = cr.width() - width; //Never disappear completely
					if (remWidth <= 0) remWidth = 1; //Do something non-crashy when too small...

					int pstep =  int(progress) % ( 2 *  remWidth );

					if ( pstep > remWidth ) {
						//Bounce about.. We're remWidth + some delta, we want to be remWidth - delta...
						// - ( (remWidth + some delta) - 2* remWidth )  = - (some deleta - remWidth) = remWidth - some delta..
						pstep = - (pstep - 2 * remWidth );
					}

					if (reverse)
						p->fillRect(cr.x() + cr.width() - width - pstep, cr.y(), width, cr.height(),
									cg.brush(TQColorGroup::Highlight));
					else
						p->fillRect(cr.x() + pstep, cr.y(), width, cr.height(),
									cg.brush(TQColorGroup::Highlight));

					return;
				}


				// Do fancy gradient for highcolor displays
				if (d->highcolor) {
					TQColor c(cg.highlight());
					KPixmap pix;
					pix.resize(cr.width(), cr.height());
					KPixmapEffect::gradient(pix, reverse ? c.light(150) : c.dark(150),
											reverse ? c.dark(150) : c.light(150),
											KPixmapEffect::HorizontalGradient);
					if (reverse)
						p->drawPixmap(cr.x()+(cr.width()-width), cr.y(), pix,
									  cr.width()-width, 0, width, cr.height());
					else
						p->drawPixmap(cr.x(), cr.y(), pix, 0, 0, width, cr.height());
				} else
					if (reverse)
						p->fillRect(cr.x()+(cr.width()-width), cr.y(), width, cr.height(),
									cg.brush(TQColorGroup::Highlight));
					else
						p->fillRect(cr.x(), cr.y(), width, cr.height(),
									cg.brush(TQColorGroup::Highlight));
			}
			break;
		}

		case CE_ProgressBarLabel: {
			TQRect cr = subRect(SR_ProgressBarContents, ceData, elementFlags, widget);
			double progress = ceData.currentStep;
			bool reverse = TQApplication::reverseLayout();
			int steps = ceData.totalSteps;

			if (!cr.isValid())
				return;

			TQFont font = p->font();
			font.setBold(true);
			p->setFont(font);

			// Draw label
			if (progress > 0 || steps == 0) {
				double pg = (steps == 0) ? 1.0 : progress / steps;
				int width = TQMIN(cr.width(), (int)(pg * cr.width()));
				TQRect crect;
				if (reverse)
					crect.setRect(cr.x()+(cr.width()-width), cr.y(), cr.width(), cr.height());
				else
					crect.setRect(cr.x()+width, cr.y(), cr.width(), cr.height());
					
				p->save();
				p->setPen((elementFlags & CEF_IsEnabled) ? (reverse ? cg.text() : cg.highlightedText()) : cg.text());
				p->drawText(r, AlignCenter, ceData.progressText);
				p->setClipRect(crect);
				p->setPen(reverse ? cg.highlightedText() : cg.text());
				p->drawText(r, AlignCenter, ceData.progressText);
				p->restore();

			} else {
				p->setPen(cg.text());
				p->drawText(r, AlignCenter, ceData.progressText);
			}

			break;
		}

		default:
			TQCommonStyle::drawControl(element, p, ceData, elementFlags, r, cg, flags, opt, widget);
	}
}


TQRect TDEStyle::subRect(SubRect r, const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, const TQWidget* widget) const
{
	switch(r)
	{
		// KDE2 look smooth progress bar
		// ------------------------------------------------------------------------
		case SR_ProgressBarGroove:
			return ceData.rect;

		case SR_ProgressBarContents:
		case SR_ProgressBarLabel: {
			// ### take into account indicatorFollowsStyle()
			TQRect rt = ceData.rect;
			return TQRect(rt.x()+2, rt.y()+2, rt.width()-4, rt.height()-4);
		}

		default:
			return TQCommonStyle::subRect(r, ceData, elementFlags, widget);
	}
}


int TDEStyle::pixelMetric(PixelMetric m, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQWidget* widget) const
{
	switch(m)
	{
		// BUTTONS
		// ------------------------------------------------------------------------
		case PM_ButtonShiftHorizontal:		// Offset by 1
		case PM_ButtonShiftVertical:		// ### Make configurable
			return 1;

		case PM_DockWindowHandleExtent:
		{
			// Check that we are not a normal toolbar or a hidden dockwidget,
			// in which case we need to adjust the height for font size
			if (widget
				&& !(ceData.parentWidgetData.widgetObjectTypes.contains("TQToolBar"))
				&& !(ceData.parentWidgetData.widgetObjectTypes.contains("TQMainWindow"))
				&& (ceData.widgetObjectTypes.contains("TQDockWindowHandle")) )
					return widget->fontMetrics().lineSpacing();
			else
				return TQCommonStyle::pixelMetric(m, ceData, elementFlags, widget);
		}

		// TABS
		// ------------------------------------------------------------------------
		case PM_TabBarTabHSpace:
			return 24;

		case PM_TabBarTabVSpace: {
			if ( ceData.tabBarData.shape == TQTabBar::RoundedAbove ||
				 ceData.tabBarData.shape == TQTabBar::RoundedBelow )
				return 10;
			else
				return 4;
		}

		case PM_TabBarTabOverlap: {
			TQTabBar::Shape tbs = ceData.tabBarData.shape;

			if ( (tbs == TQTabBar::RoundedAbove) ||
				 (tbs == TQTabBar::RoundedBelow) )
				return 0;
			else
				return 2;
		}

		// SLIDER
		// ------------------------------------------------------------------------
		case PM_SliderLength:
			return 18;

		case PM_SliderThickness:
			return 24;

		// Determines how much space to leave for the actual non-tickmark
		// portion of the slider.
		case PM_SliderControlThickness: {
			TQSlider::TickSetting ts = (TQSlider::TickSetting)ceData.tickMarkSetting;
			int thickness = (ceData.orientation == TQt::Horizontal) ?
							 ceData.rect.height() : ceData.rect.width();
			switch (ts) {
				case TQSlider::NoMarks:				// Use total area.
					break;
				case TQSlider::Both:
					thickness = (thickness/2) + 3;	// Use approx. 1/2 of area.
					break;
				default:							// Use approx. 2/3 of area
					thickness = ((thickness*2)/3) + 3;
					break;
			};
			return thickness;
		}

		// SPLITTER
		// ------------------------------------------------------------------------
		case PM_SplitterWidth:
			if (ceData.widgetObjectTypes.contains("TQDockWindowResizeHandle"))
				return 8;	// ### why do we need 2pix extra?
			else
				return 6;

		// FRAMES
		// ------------------------------------------------------------------------
		case PM_MenuBarFrameWidth:
			return 1;

		case PM_DockWindowFrameWidth:
			return 1;

		// GENERAL
		// ------------------------------------------------------------------------
		case PM_MaximumDragDistance:
			return -1;

		case PM_MenuBarItemSpacing:
			return 5;

		case PM_ToolBarItemSpacing:
			return 0;

		case PM_PopupMenuScrollerHeight:
			return pixelMetric( PM_ScrollBarExtent, ceData, elementFlags, 0);

		default:
			return TQCommonStyle::pixelMetric( m, ceData, elementFlags, widget );
	}
}

//Helper to find the next sibling that's not hidden
static TQListViewItem* nextVisibleSibling(TQListViewItem* item)
{
    TQListViewItem* sibling = item;
    do
    {
        sibling = sibling->nextSibling();
    }
    while (sibling && !sibling->isVisible());
    
    return sibling;
}

void TDEStyle::drawComplexControl( ComplexControl control,
								 TQPainter* p,
								 const TQStyleControlElementData &ceData,
								 ControlElementFlags elementFlags,
								 const TQRect &r,
								 const TQColorGroup &cg,
								 SFlags flags,
								 SCFlags controls,
								 SCFlags active,
								 const TQStyleOption &opt,
								 const TQWidget* widget ) const
{
	switch(control)
	{
		// 3 BUTTON SCROLLBAR
		// ------------------------------------------------------------------------
		case CC_ScrollBar: {
			// Many thanks to Brad Hughes for contributing this code.
			bool useThreeButtonScrollBar = (d->scrollbarType & ThreeButtonScrollBar);

			bool   maxedOut   = (ceData.minSteps    == ceData.maxSteps);
			bool   horizontal = (ceData.orientation == TQt::Horizontal);
			SFlags sflags     = ((horizontal ? Style_Horizontal : Style_Default) |
								 (maxedOut   ? Style_Default : Style_Enabled));

			TQRect  addline, subline, subline2, addpage, subpage, slider, first, last;
			subline = querySubControlMetrics(control, ceData, elementFlags, SC_ScrollBarSubLine, opt, widget);
			addline = querySubControlMetrics(control, ceData, elementFlags, SC_ScrollBarAddLine, opt, widget);
			subpage = querySubControlMetrics(control, ceData, elementFlags, SC_ScrollBarSubPage, opt, widget);
			addpage = querySubControlMetrics(control, ceData, elementFlags, SC_ScrollBarAddPage, opt, widget);
			slider  = querySubControlMetrics(control, ceData, elementFlags, SC_ScrollBarSlider,  opt, widget);
			first   = querySubControlMetrics(control, ceData, elementFlags, SC_ScrollBarFirst,   opt, widget);
			last    = querySubControlMetrics(control, ceData, elementFlags, SC_ScrollBarLast,    opt, widget);
			subline2 = addline;

			if ( useThreeButtonScrollBar ) {
				if (horizontal) {
					subline2.moveBy(-addline.width(), 0);
				}
				else {
					subline2.moveBy(0, -addline.height());
				}
			}

			// Draw the up/left button set
			if ((controls & SC_ScrollBarSubLine) && subline.isValid()) {
				drawPrimitive(PE_ScrollBarSubLine, p, ceData, elementFlags, subline, cg,
							sflags | (active == SC_ScrollBarSubLine ?
								Style_Down : Style_Default));

				if (useThreeButtonScrollBar && subline2.isValid())
					drawPrimitive(PE_ScrollBarSubLine, p, ceData, elementFlags, subline2, cg,
							sflags | (active == SC_ScrollBarSubLine ?
								Style_Down : Style_Default));
			}

			if ((controls & SC_ScrollBarAddLine) && addline.isValid())
				drawPrimitive(PE_ScrollBarAddLine, p, ceData, elementFlags, addline, cg,
							sflags | ((active == SC_ScrollBarAddLine) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarSubPage) && subpage.isValid())
				drawPrimitive(PE_ScrollBarSubPage, p, ceData, elementFlags, subpage, cg,
							sflags | ((active == SC_ScrollBarSubPage) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarAddPage) && addpage.isValid())
				drawPrimitive(PE_ScrollBarAddPage, p, ceData, elementFlags, addpage, cg,
							sflags | ((active == SC_ScrollBarAddPage) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarFirst) && first.isValid())
				drawPrimitive(PE_ScrollBarFirst, p, ceData, elementFlags, first, cg,
							sflags | ((active == SC_ScrollBarFirst) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarLast) && last.isValid())
				drawPrimitive(PE_ScrollBarLast, p, ceData, elementFlags, last, cg,
							sflags | ((active == SC_ScrollBarLast) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarSlider) && slider.isValid()) {
				drawPrimitive(PE_ScrollBarSlider, p, ceData, elementFlags, slider, cg,
							sflags | ((active == SC_ScrollBarSlider) ?
										Style_Down : Style_Default));
				// Draw focus rect
				if (elementFlags & CEF_HasFocus) {
					TQRect fr(slider.x() + 2, slider.y() + 2,
							 slider.width() - 5, slider.height() - 5);
					drawPrimitive(PE_FocusRect, p, ceData, elementFlags, fr, cg, Style_Default);
				}
			}
			break;
		}


		// SLIDER
		// -------------------------------------------------------------------
		case CC_Slider: {
			TQRect groove = querySubControlMetrics(CC_Slider, ceData, elementFlags, SC_SliderGroove, opt, widget);
			TQRect handle = querySubControlMetrics(CC_Slider, ceData, elementFlags, SC_SliderHandle, opt, widget);

			// Double-buffer slider for no flicker
			TQPixmap pix(ceData.rect.size());
			TQPainter p2;
			p2.begin(&pix);

			if ( (elementFlags & CEF_HasParentWidget) &&
				 !ceData.parentWidgetData.bgPixmap.isNull() ) {
				TQPixmap pixmap = ceData.parentWidgetData.bgPixmap;
				p2.drawTiledPixmap(r, pixmap, ceData.pos);
			} else
				pix.fill(cg.background());

			// Draw slider groove
			if ((controls & SC_SliderGroove) && groove.isValid()) {
				drawTDEStylePrimitive( KPE_SliderGroove, &p2, ceData, elementFlags, groove, cg, flags, opt, widget );

				// Draw the focus rect around the groove
				if (elementFlags & CEF_HasFocus) {
					drawPrimitive(PE_FocusRect, &p2, ceData, elementFlags, groove, cg);
				}
			}

			// Draw the tickmarks
			if (controls & SC_SliderTickmarks)
				TQCommonStyle::drawComplexControl(control, &p2, ceData, elementFlags,
						r, cg, flags, SC_SliderTickmarks, active, opt, widget);

			// Draw the slider handle
			if ((controls & SC_SliderHandle) && handle.isValid()) {
				if (active == SC_SliderHandle)
					flags |= Style_Active;
				drawTDEStylePrimitive( KPE_SliderHandle, &p2, ceData, elementFlags, handle, cg, flags, opt, widget );
			}

			p2.end();

			TQPaintDevice* ppd = p->device();
			if (ppd->isExtDev()) {
				p->drawPixmap(0, 0, pix);
			}
			else {
				bitBlt((TQWidget*)widget, r.x(), r.y(), &pix);
			}
			break;
		}

		// LISTVIEW
		// -------------------------------------------------------------------
		case CC_ListView: {

			/*
			 * Many thanks to TrollTech AS for donating CC_ListView from TQWindowsStyle.
			 * CC_ListView code is Copyright (C) 1998-2000 TrollTech AS.
			 */

			// Paint the icon and text.
			if ( controls & SC_ListView )
				TQCommonStyle::drawComplexControl( control, p, ceData, elementFlags, r, cg, flags, controls, active, opt, widget );

			// If we're have a branch or are expanded...
			if ( controls & (SC_ListViewBranch | SC_ListViewExpand) )
			{
				// If no list view item was supplied, break
				if (opt.isDefault())
					break;

				TQListViewItem *item  = opt.listViewItem();
				TQListViewItem *child = item->firstChild();

				int y = r.y();
				int c;	// dotline vertice count
				int dotoffset = 0;
				TQPointArray dotlines;

				if ( active == SC_All && controls == SC_ListViewExpand ) {
					// We only need to draw a vertical line
					c = 2;
					dotlines.resize(2);
					dotlines[0] = TQPoint( r.right(), r.top() );
					dotlines[1] = TQPoint( r.right(), r.bottom() );

				} else {

					int linetop = 0, linebot = 0;
					// each branch needs at most two lines, ie. four end points
					dotoffset = (item->itemPos() + item->height() - y) % 2;
					dotlines.resize( item->childCount() * 4 );
					c = 0;

					// skip the stuff above the exposed rectangle
					while ( child && y + child->height() <= 0 )
					{
						y += child->totalHeight();
						child = nextVisibleSibling(child);
					}

					int bx = r.width() / 2;

					// paint stuff in the magical area
					TQListView* v = item->listView();
					int lh = TQMAX( p->fontMetrics().height() + 2 * v->itemMargin(),
								   TQApplication::globalStrut().height() );
					if ( lh % 2 > 0 )
						lh++;

					// Draw all the expand/close boxes...
					TQRect boxrect;
					TQStyle::StyleFlags boxflags;
					while ( child && y < r.height() )
					{
						linebot = y + lh/2;
						if ( (child->isExpandable() || child->childCount()) &&
							 (child->height() > 0) )
						{
							// The primitive requires a rect.
							boxrect = TQRect( bx-4, linebot-4, 9, 9 );
							boxflags = child->isOpen() ? TQStyle::Style_Off : TQStyle::Style_On;

							// TDEStyle extension: Draw the box and expand/collapse indicator
							drawTDEStylePrimitive( KPE_ListViewExpander, p, ceData, elementFlags, boxrect, cg, boxflags, opt, NULL );

							// dotlinery
							p->setPen( cg.mid() );
							dotlines[c++] = TQPoint( bx, linetop );
							dotlines[c++] = TQPoint( bx, linebot - 5 );
							dotlines[c++] = TQPoint( bx + 5, linebot );
							dotlines[c++] = TQPoint( r.width(), linebot );
							linetop = linebot + 5;
						} else {
							// just dotlinery
							dotlines[c++] = TQPoint( bx+1, linebot );
							dotlines[c++] = TQPoint( r.width(), linebot );
						}

						y += child->totalHeight();
						child = nextVisibleSibling(child);
					}

					if ( child ) // there's a child to draw, so move linebot to edge of rectangle
						linebot = r.height();

					if ( linetop < linebot )
					{
						dotlines[c++] = TQPoint( bx, linetop );
						dotlines[c++] = TQPoint( bx, linebot );
					}
				}

				// Draw all the branches...
				static int thickness = kPixelMetric( KPM_ListViewBranchThickness, ceData, elementFlags );
				int line; // index into dotlines
				TQRect branchrect;
				TQStyle::StyleFlags branchflags;
				for( line = 0; line < c; line += 2 )
				{
					// assumptions here: lines are horizontal or vertical.
					// lines always start with the numerically lowest
					// coordinate.

					// point ... relevant coordinate of current point
					// end ..... same coordinate of the end of the current line
					// other ... the other coordinate of the current point/line
					if ( dotlines[line].y() == dotlines[line+1].y() )
					{
						// Horizontal branch
						int end = dotlines[line+1].x();
						int point = dotlines[line].x();
						int other = dotlines[line].y();

						branchrect  = TQRect( point, other-(thickness/2), end-point, thickness );
						branchflags = TQStyle::Style_Horizontal;

						// TDEStyle extension: Draw the horizontal branch
						drawTDEStylePrimitive( KPE_ListViewBranch, p, ceData, elementFlags, branchrect, cg, branchflags, opt, NULL );

					} else {
						// Vertical branch
						int end = dotlines[line+1].y();
						int point = dotlines[line].y();
						int other = dotlines[line].x();
						int pixmapoffset = ((point & 1) != dotoffset ) ? 1 : 0;

						branchrect  = TQRect( other-(thickness/2), point, thickness, end-point );
						if (!pixmapoffset)	// ### Hackish - used to hint the offset
							branchflags = TQStyle::Style_NoChange;
						else
							branchflags = TQStyle::Style_Default;

						// TDEStyle extension: Draw the vertical branch
						drawTDEStylePrimitive( KPE_ListViewBranch, p, ceData, elementFlags, branchrect, cg, branchflags, opt, NULL );
					}
				}
			}
			break;
		}

		default:
			TQCommonStyle::drawComplexControl( control, p, ceData, elementFlags, r, cg,
											  flags, controls, active, opt, widget );
			break;
	}
}


TQStyle::SubControl TDEStyle::querySubControl( ComplexControl control,
											const TQStyleControlElementData &ceData,
											ControlElementFlags elementFlags,
											const TQPoint &pos,
											const TQStyleOption &opt,
											const TQWidget* widget ) const
{
	TQStyle::SubControl ret = TQCommonStyle::querySubControl(control, ceData, elementFlags, pos, opt, widget);

	if (d->scrollbarType == ThreeButtonScrollBar) {
		// Enable third button
		if (control == CC_ScrollBar && ret == SC_None)
			ret = SC_ScrollBarSubLine;
	}
	return ret;
}


TQRect TDEStyle::querySubControlMetrics( ComplexControl control,
									  const TQStyleControlElementData &ceData,
									  ControlElementFlags elementFlags,
									  SubControl sc,
									  const TQStyleOption &opt,
									  const TQWidget* widget ) const
{
    TQRect ret;

	if (control == CC_ScrollBar)
	{
		bool threeButtonScrollBar = d->scrollbarType & ThreeButtonScrollBar;
		bool platinumScrollBar    = d->scrollbarType & PlatinumStyleScrollBar;
		bool nextScrollBar        = d->scrollbarType & NextStyleScrollBar;

		bool horizontal = ceData.orientation == TQt::Horizontal;
		int sliderstart = ceData.startStep;
		int sbextent    = pixelMetric(PM_ScrollBarExtent, ceData, elementFlags, widget);
		int maxlen      = (horizontal ? ceData.rect.width() : ceData.rect.height())
						  - (sbextent * (threeButtonScrollBar ? 3 : 2));
		int sliderlen;

		// calculate slider length
		if (ceData.maxSteps != ceData.minSteps)
		{
			uint range = ceData.maxSteps - ceData.minSteps;
			sliderlen = (ceData.pageStep * maxlen) /	(range + ceData.pageStep);

			int slidermin = pixelMetric( PM_ScrollBarSliderMin, ceData, elementFlags, widget );
			if ( sliderlen < slidermin || range > INT_MAX / 2 )
				sliderlen = slidermin;
			if ( sliderlen > maxlen )
				sliderlen = maxlen;
		} else
			sliderlen = maxlen;

		// Subcontrols
		switch (sc)
		{
			case SC_ScrollBarSubLine: {
				// top/left button
				if (platinumScrollBar) {
					if (horizontal)
						ret.setRect(ceData.rect.width() - 2 * sbextent, 0, sbextent, sbextent);
					else
						ret.setRect(0, ceData.rect.height() - 2 * sbextent, sbextent, sbextent);
				} else
					ret.setRect(0, 0, sbextent, sbextent);
				break;
			}

			case SC_ScrollBarAddLine: {
				// bottom/right button
				if (nextScrollBar) {
					if (horizontal)
						ret.setRect(sbextent, 0, sbextent, sbextent);
					else
						ret.setRect(0, sbextent, sbextent, sbextent);
				} else {
					if (horizontal)
						ret.setRect(ceData.rect.width() - sbextent, 0, sbextent, sbextent);
					else
						ret.setRect(0, ceData.rect.height() - sbextent, sbextent, sbextent);
				}
				break;
			}

			case SC_ScrollBarSubPage: {
				// between top/left button and slider
				if (platinumScrollBar) {
					if (horizontal)
						ret.setRect(0, 0, sliderstart, sbextent);
					else
						ret.setRect(0, 0, sbextent, sliderstart);
				} else if (nextScrollBar) {
					if (horizontal)
						ret.setRect(sbextent*2, 0, sliderstart-2*sbextent, sbextent);
					else
						ret.setRect(0, sbextent*2, sbextent, sliderstart-2*sbextent);
				} else {
					if (horizontal)
						ret.setRect(sbextent, 0, sliderstart - sbextent, sbextent);
					else
						ret.setRect(0, sbextent, sbextent, sliderstart - sbextent);
				}
				break;
			}

			case SC_ScrollBarAddPage: {
				// between bottom/right button and slider
				int fudge;

				if (platinumScrollBar)
					fudge = 0;
				else if (nextScrollBar)
					fudge = 2*sbextent;
				else
					fudge = sbextent;

				if (horizontal)
					ret.setRect(sliderstart + sliderlen, 0,
							maxlen - sliderstart - sliderlen + fudge, sbextent);
				else
					ret.setRect(0, sliderstart + sliderlen, sbextent,
							maxlen - sliderstart - sliderlen + fudge);
				break;
			}

			case SC_ScrollBarGroove: {
				int multi = threeButtonScrollBar ? 3 : 2;
				int fudge;

				if (platinumScrollBar)
					fudge = 0;
				else if (nextScrollBar)
					fudge = 2*sbextent;
				else
					fudge = sbextent;

				if (horizontal)
					ret.setRect(fudge, 0, ceData.rect.width() - sbextent * multi, ceData.rect.height());
				else
					ret.setRect(0, fudge, ceData.rect.width(), ceData.rect.height() - sbextent * multi);
				break;
			}

			case SC_ScrollBarSlider: {
				if (horizontal)
					ret.setRect(sliderstart, 0, sliderlen, sbextent);
				else
					ret.setRect(0, sliderstart, sbextent, sliderlen);
				break;
			}

			default:
				ret = TQCommonStyle::querySubControlMetrics(control, ceData, elementFlags, sc, opt, widget);
				break;
		}
	} else
		ret = TQCommonStyle::querySubControlMetrics(control, ceData, elementFlags, sc, opt, widget);

	return ret;
}

static const char * const tdestyle_close_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"..##....##..",
"...##..##...",
"....####....",
".....##.....",
"....####....",
"...##..##...",
"..##....##..",
"............",
"............",
"............"};

static const char * const tdestyle_maximize_xpm[]={
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
".##########.",
".##########.",
".#........#.",
".#........#.",
".#........#.",
".#........#.",
".#........#.",
".#........#.",
".##########.",
"............"};


static const char * const tdestyle_minimize_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"............",
"............",
"............",
"............",
"............",
"...######...",
"...######...",
"............",
"............",
"............"};

static const char * const tdestyle_normalizeup_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"...#######..",
"...#######..",
"...#.....#..",
".#######.#..",
".#######.#..",
".#.....#.#..",
".#.....###..",
".#.....#....",
".#.....#....",
".#######....",
"............"};


static const char * const tdestyle_shade_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"............",
"............",
"............",
".....#......",
"....###.....",
"...#####....",
"..#######...",
"............",
"............",
"............"};

static const char * const tdestyle_unshade_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"............",
"............",
"..#######...",
"...#####....",
"....###.....",
".....#......",
"............",
"............",
"............",
"............"};

static const char * const dock_window_close_xpm[] = {
"8 8 2 1",
"# c #000000",
". c None",
"##....##",
".##..##.",
"..####..",
"...##...",
"..####..",
".##..##.",
"##....##",
"........"};

// Message box icons, from page 210 of the Windows style guide.

// Hand-drawn to resemble Microsoft's icons, but in the Mac/Netscape
// palette.  The "question mark" icon, which Microsoft recommends not
// using but a lot of people still use, is left out.

/* XPM */
static const char * const information_xpm[]={
"32 32 5 1",
". c None",
"c c #000000",
"* c #999999",
"a c #ffffff",
"b c #0000ff",
"...........********.............",
"........***aaaaaaaa***..........",
"......**aaaaaaaaaaaaaa**........",
".....*aaaaaaaaaaaaaaaaaa*.......",
"....*aaaaaaaabbbbaaaaaaaac......",
"...*aaaaaaaabbbbbbaaaaaaaac.....",
"..*aaaaaaaaabbbbbbaaaaaaaaac....",
".*aaaaaaaaaaabbbbaaaaaaaaaaac...",
".*aaaaaaaaaaaaaaaaaaaaaaaaaac*..",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaaac*.",
"*aaaaaaaaaabbbbbbbaaaaaaaaaaac*.",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
".*aaaaaaaaaaabbbbbaaaaaaaaaac***",
".*aaaaaaaaaaabbbbbaaaaaaaaaac***",
"..*aaaaaaaaaabbbbbaaaaaaaaac***.",
"...caaaaaaabbbbbbbbbaaaaaac****.",
"....caaaaaaaaaaaaaaaaaaaac****..",
".....caaaaaaaaaaaaaaaaaac****...",
"......ccaaaaaaaaaaaaaacc****....",
".......*cccaaaaaaaaccc*****.....",
"........***cccaaaac*******......",
"..........****caaac*****........",
".............*caaac**...........",
"...............caac**...........",
"................cac**...........",
".................cc**...........",
"..................***...........",
"...................**..........."};
/* XPM */
static const char* const warning_xpm[]={
"32 32 4 1",
". c None",
"a c #ffff00",
"* c #000000",
"b c #999999",
".............***................",
"............*aaa*...............",
"...........*aaaaa*b.............",
"...........*aaaaa*bb............",
"..........*aaaaaaa*bb...........",
"..........*aaaaaaa*bb...........",
".........*aaaaaaaaa*bb..........",
".........*aaaaaaaaa*bb..........",
"........*aaaaaaaaaaa*bb.........",
"........*aaaa***aaaa*bb.........",
".......*aaaa*****aaaa*bb........",
".......*aaaa*****aaaa*bb........",
"......*aaaaa*****aaaaa*bb.......",
"......*aaaaa*****aaaaa*bb.......",
".....*aaaaaa*****aaaaaa*bb......",
".....*aaaaaa*****aaaaaa*bb......",
"....*aaaaaaaa***aaaaaaaa*bb.....",
"....*aaaaaaaa***aaaaaaaa*bb.....",
"...*aaaaaaaaa***aaaaaaaaa*bb....",
"...*aaaaaaaaaa*aaaaaaaaaa*bb....",
"..*aaaaaaaaaaa*aaaaaaaaaaa*bb...",
"..*aaaaaaaaaaaaaaaaaaaaaaa*bb...",
".*aaaaaaaaaaaa**aaaaaaaaaaa*bb..",
".*aaaaaaaaaaa****aaaaaaaaaa*bb..",
"*aaaaaaaaaaaa****aaaaaaaaaaa*bb.",
"*aaaaaaaaaaaaa**aaaaaaaaaaaa*bb.",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaa*bbb",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaa*bbb",
".*aaaaaaaaaaaaaaaaaaaaaaaaa*bbbb",
"..*************************bbbbb",
"....bbbbbbbbbbbbbbbbbbbbbbbbbbb.",
".....bbbbbbbbbbbbbbbbbbbbbbbbb.."};
/* XPM */
static const char* const critical_xpm[]={
"32 32 4 1",
". c None",
"a c #999999",
"* c #ff0000",
"b c #ffffff",
"...........********.............",
".........************...........",
".......****************.........",
"......******************........",
".....********************a......",
"....**********************a.....",
"...************************a....",
"..*******b**********b*******a...",
"..******bbb********bbb******a...",
".******bbbbb******bbbbb******a..",
".*******bbbbb****bbbbb*******a..",
"*********bbbbb**bbbbb*********a.",
"**********bbbbbbbbbb**********a.",
"***********bbbbbbbb***********aa",
"************bbbbbb************aa",
"************bbbbbb************aa",
"***********bbbbbbbb***********aa",
"**********bbbbbbbbbb**********aa",
"*********bbbbb**bbbbb*********aa",
".*******bbbbb****bbbbb*******aa.",
".******bbbbb******bbbbb******aa.",
"..******bbb********bbb******aaa.",
"..*******b**********b*******aa..",
"...************************aaa..",
"....**********************aaa...",
"....a********************aaa....",
".....a******************aaa.....",
"......a****************aaa......",
".......aa************aaaa.......",
".........aa********aaaaa........",
"...........aaaaaaaaaaa..........",
".............aaaaaaa............"};

TQPixmap TDEStyle::stylePixmap( StylePixmap stylepixmap,
						  const TQStyleControlElementData &ceData,
						  ControlElementFlags elementFlags,
						  const TQStyleOption& opt,
						  const TQWidget* widget) const
{
	switch (stylepixmap) {
		case SP_TitleBarShadeButton:
			return TQPixmap(const_cast<const char**>(tdestyle_shade_xpm));
		case SP_TitleBarUnshadeButton:
			return TQPixmap(const_cast<const char**>(tdestyle_unshade_xpm));
		case SP_TitleBarNormalButton:
			return TQPixmap(const_cast<const char**>(tdestyle_normalizeup_xpm));
		case SP_TitleBarMinButton:
			return TQPixmap(const_cast<const char**>(tdestyle_minimize_xpm));
		case SP_TitleBarMaxButton:
			return TQPixmap(const_cast<const char**>(tdestyle_maximize_xpm));
		case SP_TitleBarCloseButton:
			return TQPixmap(const_cast<const char**>(tdestyle_close_xpm));
		case SP_DockWindowCloseButton:
			return TQPixmap(const_cast<const char**>(dock_window_close_xpm ));
		case SP_MessageBoxInformation:
			return TQPixmap(const_cast<const char**>(information_xpm));
		case SP_MessageBoxWarning:
			return TQPixmap(const_cast<const char**>(warning_xpm));
		case SP_MessageBoxCritical:
			return TQPixmap(const_cast<const char**>(critical_xpm));
		default:
			break;
    }
    return TQCommonStyle::stylePixmap(stylepixmap, ceData, elementFlags, opt, widget);
}


int TDEStyle::styleHint( StyleHint sh, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags,
					   const TQStyleOption &opt, TQStyleHintReturn* shr, const TQWidget* w) const
{
	switch (sh)
	{
		case SH_EtchDisabledText:
			return d->etchDisabledText ? 1 : 0;

		case SH_PopupMenu_Scrollable:
			return d->scrollablePopupmenus ? 1 : 0;

		case SH_HideUnderlineAcceleratorWhenAltUp:
			return d->autoHideAccelerators ? 1 : 0;

		case SH_MenuBar_AltKeyNavigation:
			return d->menuAltKeyNavigation ? 1 : 0;

		case SH_PopupMenu_SubMenuPopupDelay:
			if ( styleHint( SH_PopupMenu_SloppySubMenus, ceData, elementFlags, TQStyleOption::Default, 0, w ) )
				return TQMIN( 100, d->popupMenuDelay );
			else
				return d->popupMenuDelay;

		case SH_PopupMenu_SloppySubMenus:
			return d->sloppySubMenus;

		case SH_ItemView_ChangeHighlightOnFocus:
		case SH_Slider_SloppyKeyEvents:
		case SH_MainWindow_SpaceBelowMenuBar:
		case SH_PopupMenu_AllowActiveAndDisabled:
			return 0;

		case SH_Slider_SnapToValue:
		case SH_PrintDialog_RightAlignButtons:
		case SH_FontDialog_SelectAssociatedText:
		case SH_MenuBar_MouseTracking:
		case SH_PopupMenu_MouseTracking:
		case SH_ComboBox_ListMouseTracking:
		case SH_ScrollBar_MiddleClickAbsolutePosition:
			return 1;
		case SH_LineEdit_PasswordCharacter:
		{
			if (w) {
				const TQFontMetrics &fm = w->fontMetrics();
				if (fm.inFont(TQChar(0x25CF))) {
					return 0x25CF;
				} else if (fm.inFont(TQChar(0x2022))) {
					return 0x2022;
				}
			}
			return '*';
		}

		default:
			return TQCommonStyle::styleHint(sh, ceData, elementFlags, opt, shr, w);
	}
}


bool TDEStyle::objectEventHandler( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void* source, TQEvent *event )
{
	if (ceData.widgetObjectTypes.contains("TQObject")) {
		TQObject* object = reinterpret_cast<TQObject*>(source);
		if ( d->useFilledFrameWorkaround )
		{
			// Make the QMenuBar/TQToolBar paintEvent() cover a larger area to
			// ensure that the filled frame contents are properly painted.
			// We essentially modify the paintEvent's rect to include the
			// panel border, which also paints the widget's interior.
			// This is nasty, but I see no other way to properly repaint
			// filled frames in all QMenuBars and QToolBars.
			// -- Karol.
			TQFrame *frame = 0;
			if ( event->type() == TQEvent::Paint
					&& (frame = ::tqt_cast<TQFrame*>(object)) )
			{
				if (frame->frameShape() != TQFrame::ToolBarPanel && frame->frameShape() != TQFrame::MenuBarPanel)
					return false;

				bool horizontal = true;
				TQPaintEvent* pe = (TQPaintEvent*)event;
				TQToolBar *toolbar = ::tqt_cast< TQToolBar *>( frame );
				TQRect r = pe->rect();

				if (toolbar && toolbar->orientation() == TQt::Vertical)
					horizontal = false;

				if (horizontal) {
					if ( r.height() == frame->height() )
						return false;	// Let TQFrame handle the painting now.

					// Else, send a new paint event with an updated paint rect.
					TQPaintEvent dummyPE( TQRect( r.x(), 0, r.width(), frame->height()) );
					TQApplication::sendEvent( frame, &dummyPE );
				}
				else {	// Vertical
					if ( r.width() == frame->width() )
						return false;
	
					TQPaintEvent dummyPE( TQRect( 0, r.y(), frame->width(), r.height()) );
					TQApplication::sendEvent( frame, &dummyPE );
				}

				// Discard this event as we sent a new paintEvent.
				return true;
			}
		}
	}

	return false;
}


// -----------------------------------------------------------------------------
// I N T E R N A L -  TDEStyle menu transparency handler
// -----------------------------------------------------------------------------

TransparencyHandler::TransparencyHandler( TDEStyle* style,
	TransparencyEngine tEngine, float menuOpacity, bool useDropShadow )
	: TQObject()
{
	te = tEngine;
	tdestyle = style;
	opacity = menuOpacity;
	dropShadow = useDropShadow;
	pix.setOptimization(TQPixmap::BestOptim);
}

TransparencyHandler::~TransparencyHandler()
{
}

bool TransparencyHandler::haveX11RGBASupport()
{
	// Simple way to determine if we have ARGB support
	if (TQPaintDevice::x11AppDepth() == 32) {
		return true;
	}
	else {
		return false;
	}
}

#define REAL_ALPHA_STRENGTH 255.0

// This is meant to be ugly but fast.
void TransparencyHandler::rightShadow(TQImage& dst)
{
	bool have_composite = haveX11RGBASupport();

	if (dst.depth() != 32)
		dst = dst.convertDepth(32);

	// blend top-right corner.
	int pixels = dst.width() * dst.height();
#ifdef WORDS_BIGENDIAN
	unsigned char* data = dst.bits() + 1;	// Skip alpha
#else
	unsigned char* data = dst.bits();		// Skip alpha
#endif
	for(int i = 0; i < 16; i++) {
		if (have_composite) {
			data++;
			data++;
			data++;
			*data = (unsigned char)(REAL_ALPHA_STRENGTH*(1.0-top_right_corner[i])); data++;
		}
		else {
			*data = (unsigned char)((*data)*top_right_corner[i]); data++;
			*data = (unsigned char)((*data)*top_right_corner[i]); data++;
			*data = (unsigned char)((*data)*top_right_corner[i]); data++;
			data++;	// skip alpha
		}
	}

	pixels -= 32;	// tint right strip without rounded edges.
	int c = 0;
	for(int i = 0; i < pixels; i++) {
		if (have_composite) {
			data++;
			data++;
			data++;;
			*data = (unsigned char)(REAL_ALPHA_STRENGTH*(1.0-shadow_strip[c])); data++;
		}
		else {
			*data = (unsigned char)((*data)*shadow_strip[c]); data++;
			*data = (unsigned char)((*data)*shadow_strip[c]); data++;
			*data = (unsigned char)((*data)*shadow_strip[c]); data++;
			data++; // skip alpha
		}
		++c;
		c %= 4;
	}

	// tint bottom edge
	for(int i = 0; i < 16; i++) {
		if (have_composite) {
			data++;
			data++;
			data++;
			*data = (unsigned char)(REAL_ALPHA_STRENGTH*(1.0-bottom_right_corner[i])); data++;
		}
		else {
			*data = (unsigned char)((*data)*bottom_right_corner[i]); data++;
			*data = (unsigned char)((*data)*bottom_right_corner[i]); data++;
			*data = (unsigned char)((*data)*bottom_right_corner[i]); data++;
			data++;	// skip alpha
		}
	}
}

void TransparencyHandler::bottomShadow(TQImage& dst)
{
	bool have_composite = haveX11RGBASupport();

	if (dst.depth() != 32)
		dst = dst.convertDepth(32);

	int line = 0;
	int width = dst.width() - 4;
	double strip_data = shadow_strip[0];
	double* corner = const_cast<double*>(bottom_left_corner);

#ifdef WORDS_BIGENDIAN
	unsigned char* data = dst.bits() + 1;	// Skip alpha
#else
	unsigned char* data = dst.bits();	// Skip alpha
#endif

	for(int y = 0; y < 4; y++)
	{
		// Bottom-left Corner
		for(int x = 0; x < 4; x++) {
			if (have_composite) {
				data++;
				data++;
				data++;
				*data = (unsigned char)(REAL_ALPHA_STRENGTH*(1.0-(*corner))); data++;
			}
			else {
				*data = (unsigned char)((*data)*(*corner)); data++;
				*data = (unsigned char)((*data)*(*corner)); data++;
				*data = (unsigned char)((*data)*(*corner)); data++;
				data++; // skip alpha
			}
			corner++;
		}

		// Scanline
		for(int x = 0; x < width; x++) {
			if (have_composite) {
				data++;
				data++;
				data++;
				*data = (unsigned char)(REAL_ALPHA_STRENGTH*(1.0-strip_data)); data++;
			}
			else {
				*data = (unsigned char)((*data)*strip_data); data++;
				*data = (unsigned char)((*data)*strip_data); data++;
				*data = (unsigned char)((*data)*strip_data); data++;
				data++; // skip alpha
			}
		}

		strip_data = shadow_strip[line++];
	}
}

TQImage TransparencyHandler::handleRealAlpha(TQImage img) {
	TQImage clearImage = img.convertDepth(32);
	clearImage.setAlphaBuffer(true);

	int w = clearImage.width();
	int h = clearImage.height();

	for (int y = 0; y < h; ++y) {
		TQRgb *ls = (TQRgb *)clearImage.scanLine( y );
		for (int x = 0; x < w; ++x) {
			ls[x] = tqRgba( 0, 0, 0, 0 );
		}
	}

	return clearImage;
}

// Create a shadow of thickness 4.
void TransparencyHandler::createShadowWindows(const TQWidget* p)
{
#ifdef TQ_WS_X11
	int x2 = p->x()+p->width();
	int y2 = p->y()+p->height();
	TQRect shadow1(x2, p->y() + 4, 4, p->height());
	TQRect shadow2(p->x() + 4, y2, p->width() - 4, 4);

	bool have_composite = haveX11RGBASupport();

	// Create a fake drop-down shadow effect via blended Xwindows
	ShadowElements se;
	se.w1 = new TQWidget(0, 0, (WFlags)(WStyle_Customize | WType_Popup | WX11BypassWM) );
	se.w2 = new TQWidget(0, 0, (WFlags)(WStyle_Customize | WType_Popup | WX11BypassWM) );
	se.w1->setGeometry(shadow1);
	se.w2->setGeometry(shadow2);
	XSelectInput(tqt_xdisplay(), se.w1->winId(), StructureNotifyMask );
	XSelectInput(tqt_xdisplay(), se.w2->winId(), StructureNotifyMask );

	// Insert a new ShadowMap entry
	shadowMap()[p] = se;

	// Some hocus-pocus here to create the drop-shadow.
	TQPixmap pix_shadow1;
	TQPixmap pix_shadow2;
	if (have_composite) {
		pix_shadow1 = TQPixmap(shadow1.width(), shadow1.height());
		pix_shadow2 = TQPixmap(shadow2.width(), shadow2.height());
	}
	else {
		pix_shadow1 = TQPixmap::grabWindow(tqt_xrootwin(),
				shadow1.x(), shadow1.y(), shadow1.width(), shadow1.height());
		pix_shadow2 = TQPixmap::grabWindow(tqt_xrootwin(),
				shadow2.x(), shadow2.y(), shadow2.width(), shadow2.height());
	}

	TQImage img;
	img = pix_shadow1.convertToImage();
	if (have_composite) img = handleRealAlpha(img);
	rightShadow(img);
	pix_shadow1.convertFromImage(img);
	img = pix_shadow2.convertToImage();
	if (have_composite) img = handleRealAlpha(img);
	bottomShadow(img);
	pix_shadow2.convertFromImage(img);

	// Set the background pixmaps
	se.w1->setErasePixmap(pix_shadow1);
	se.w2->setErasePixmap(pix_shadow2);

	// Show the 'shadow' just before showing the popup menu window
	// Don't use TQWidget::show() so we don't confuse QEffects, thus causing broken focus.
	XMapWindow(tqt_xdisplay(), se.w1->winId());
	XMapWindow(tqt_xdisplay(), se.w2->winId());
#else
	Q_UNUSED( p )
#endif
}

void TransparencyHandler::removeShadowWindows(const TQWidget* p)
{
#ifdef TQ_WS_X11
	ShadowMap::iterator it = shadowMap().find(p);
	if (it != shadowMap().end())
	{
		ShadowElements se = it.data();
		XUnmapWindow(tqt_xdisplay(), se.w1->winId());	// hide
		XUnmapWindow(tqt_xdisplay(), se.w2->winId());
		XFlush(tqt_xdisplay());							// try to hide faster
		delete se.w1;
		delete se.w2;
		shadowMap().erase(it);
	}
#else
	Q_UNUSED( p )
#endif
}

bool TransparencyHandler::eventFilter( TQObject* object, TQEvent* event )
{
#if !defined TQ_WS_MAC && !defined TQ_WS_WIN
	// Transparency idea was borrowed from KDE2's "MegaGradient" Style,
	// Copyright (C) 2000 Daniel M. Duley <mosfet@kde.org>

	// Added 'fake' menu shadows <04-Jul-2002> -- Karol
	TQWidget* p = (TQWidget*)object;
	TQEvent::Type et = event->type();

	if (et == TQEvent::Show)
	{
		// Handle translucency
		if (te != Disabled)
		{
			pix = TQPixmap::grabWindow(tqt_xrootwin(),
					p->x(), p->y(), p->width(), p->height());

			switch (te) {
#ifdef HAVE_XRENDER
				case XRender:
					if (tqt_use_xrender) {
						XRenderBlendToPixmap(p);
						break;
					}
					// Fall through intended
#else
				case XRender:
#endif
				case SoftwareBlend:
					blendToPixmap(p->colorGroup(), p);
					break;

				case SoftwareTint:
				default:
					blendToColor(p->colorGroup().button());
			};

			p->setErasePixmap(pix);
		}

		// Handle drop shadow
		// * FIXME : !shadowMap().contains(p) is a workaround for leftover
		// * shadows after duplicate show events.
		// * TODO : determine real cause for duplicate events
		// * till 20021005
		if ((dropShadow  || useDropShadow(p))
		    && p->width() > 16 && p->height() > 16 && !shadowMap().contains( p ))
			createShadowWindows(p);
	}
        else if (et == TQEvent::Resize && p->isShown() && p->isTopLevel())
        {
		// Handle drop shadow
		if (dropShadow || useDropShadow(p))
		{
			removeShadowWindows(p);
			createShadowWindows(p);
		}
        }
	else if (et == TQEvent::Hide)
	{
		// Handle drop shadow
		if (dropShadow || useDropShadow(p))
			removeShadowWindows(p);

		// Handle translucency
		if (te != Disabled)
			p->setErasePixmap(TQPixmap());
	}

#endif
	return false;
}


// Blends a TQImage to a predefined color, with a given opacity.
void TransparencyHandler::blendToColor(const TQColor &col)
{
	if (opacity < 0.0 || opacity > 1.0)
		return;

	TQImage img = pix.convertToImage();
	KImageEffect::blend(col, img, opacity);
	pix.convertFromImage(img);
}


void TransparencyHandler::blendToPixmap(const TQColorGroup &cg, const TQWidget* p)
{
	if (opacity < 0.0 || opacity > 1.0)
		return;

	KPixmap blendPix;
	blendPix.resize( pix.width(), pix.height() );

	if (blendPix.width()  != pix.width() ||
		blendPix.height() != pix.height())
		return;

	// Allow styles to define the blend pixmap - allows for some interesting effects.
	if (::tqt_cast<TQPopupMenu*>(p))
		tdestyle->renderMenuBlendPixmap( blendPix, cg, ::tqt_cast<TQPopupMenu*>(p) );
	else
		blendPix.fill(cg.button());	// Just tint as the default behavior

	TQImage blendImg = blendPix.convertToImage();
	TQImage backImg  = pix.convertToImage();
	KImageEffect::blend(blendImg, backImg, opacity);
	pix.convertFromImage(backImg);
}


#ifdef HAVE_XRENDER
// Here we go, use XRender in all its glory.
// NOTE: This is actually a bit slower than the above routines
// on non-accelerated displays. -- Karol.
void TransparencyHandler::XRenderBlendToPixmap(const TQWidget* p)
{
	KPixmap renderPix;
	renderPix.resize( pix.width(), pix.height() );

	// Allow styles to define the blend pixmap - allows for some interesting effects.
	if (::tqt_cast<TQPopupMenu*>(p))
	   tdestyle->renderMenuBlendPixmap( renderPix, p->colorGroup(),
			   ::tqt_cast<TQPopupMenu*>(p) );
	else
		renderPix.fill(p->colorGroup().button());	// Just tint as the default behavior

	Display* dpy = tqt_xdisplay();
	Pixmap   alphaPixmap;
	Picture  alphaPicture;
	XRenderPictFormat        Rpf;
	XRenderPictureAttributes Rpa;
	XRenderColor clr;
	clr.alpha = ((unsigned short)(255*opacity) << 8);

	Rpf.type  = PictTypeDirect;
	Rpf.depth = 8;
	Rpf.direct.alphaMask = 0xff;
	Rpa.repeat = True;	// Tile

	XRenderPictFormat* xformat = XRenderFindFormat(dpy,
		PictFormatType | PictFormatDepth | PictFormatAlphaMask, &Rpf, 0);

	alphaPixmap = XCreatePixmap(dpy, p->handle(), 1, 1, 8);
	alphaPicture = XRenderCreatePicture(dpy, alphaPixmap, xformat, CPRepeat, &Rpa);

	XRenderFillRectangle(dpy, PictOpSrc, alphaPicture, &clr, 0, 0, 1, 1);

	XRenderComposite(dpy, PictOpOver,
			renderPix.x11RenderHandle(), alphaPicture, pix.x11RenderHandle(), // src, mask, dst
			0, 0, 	// srcx,  srcy
			0, 0,	// maskx, masky
			0, 0,	// dstx,  dsty
			pix.width(), pix.height());

	XRenderFreePicture(dpy, alphaPicture);
	XFreePixmap(dpy, alphaPixmap);
}
#endif

void TDEStyle::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

// HACK for gtk-qt-engine

extern "C" TDE_EXPORT
void kde_tdestyle_set_scrollbar_type_windows( void* style )
{
    ((TDEStyle*)style)->setScrollBarType( TDEStyle::WindowsStyleScrollBar );
}
#include "tdestyle.moc"
