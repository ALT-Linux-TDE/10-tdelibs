/*
 * $Id$
 *
 * KDE3 HighColor Style (version 1.0)
 * Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
 *           (C) 2001-2002 Fredrik H�glund  <fredrik@kde.org> 
 *
 * Drawing routines adapted from the KDE2 HCStyle,
 * Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
 *           (C) 2000 Dirk Mueller          <mueller@kde.org>
 *           (C) 2001 Martijn Klingens      <klingens@kde.org>
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

#include <tqdrawutil.h>
#include <tqpainter.h>
#include <tqpointarray.h>
#include <tqstyleplugin.h>

#include <tqcombobox.h>
#include <tqheader.h>
#include <tqmenubar.h>
#include <tqpushbutton.h>
#include <tqscrollbar.h>
#include <tqslider.h>
#include <tqtabbar.h>
#include <tqtoolbutton.h>
#include <tqtoolbar.h>
#include <tqpopupmenu.h>

#include <tqimage.h>
#include <tqapplication.h>

#include <kdrawutil.h>
#include <kpixmapeffect.h>

#include "highcolor.h"
#include "highcolor.moc"
#include "bitmaps.h"


// -- Style Plugin Interface -------------------------
class HighColorStylePlugin : public TQStylePlugin
{
	public:
		HighColorStylePlugin() {}
		~HighColorStylePlugin() {}

		TQStringList keys() const
		{
			return TQStringList() << "HighColor" << "Default" << "B3";
		}

		TQStyle* create( const TQString& key )
		{
			if ( key == "highcolor" )
				return new HighColorStyle( HighColorStyle::HighColor );
			
			else if ( key == "default" )
				return new HighColorStyle( HighColorStyle::Default );

			else if ( key == "b3" )
				return new HighColorStyle( HighColorStyle::B3 );
			
			return 0;
		}
};

TDE_EXPORT_PLUGIN( HighColorStylePlugin )
// ---------------------------------------------------


static TQBitmap lightBmp;
static TQBitmap grayBmp;
static TQBitmap dgrayBmp;
static TQBitmap centerBmp;
static TQBitmap maskBmp;
static TQBitmap xBmp;
static TQIntDict<GradientSet> gDict;

static const int itemFrame       = 1;
static const int itemHMargin     = 3;
static const int itemVMargin     = 0;
static const int arrowHMargin    = 6;
static const int rightBorder     = 12;
static const char* kdeToolbarWidget = "tde toolbar widget";

// ---------------------------------------------------------------------------

GradientSet::GradientSet(const TQColor &baseColor)
{
	c = baseColor;
	for(int i=0; i < GradientCount; i++)
		gradients[i] = NULL;
}


GradientSet::~GradientSet()
{
	for(int i=0; i < GradientCount; i++)
		if(gradients[i])
			delete gradients[i];
}


KPixmap* GradientSet::gradient(GradientType type)
{
	if (gradients[type])
		return gradients[type];

	switch(type)
	{
		case VSmall: {
			gradients[VSmall] = new KPixmap;
			gradients[VSmall]->resize(18, 24);
			KPixmapEffect::gradient(*gradients[VSmall], c.light(110), c.dark(110),
											KPixmapEffect::VerticalGradient);
			break;
		}

		case VMed: {
			gradients[VMed] = new KPixmap;
			gradients[VMed]->resize(18, 34);
			KPixmapEffect::gradient(*gradients[VMed], c.light(110), c.dark(110),
											KPixmapEffect::VerticalGradient);
			break;
		}

		case VLarge: {
			gradients[VLarge] = new KPixmap;
			gradients[VLarge]->resize(18, 64);
			KPixmapEffect::gradient(*gradients[VLarge], c.light(110), c.dark(110),
											KPixmapEffect::VerticalGradient);
			break;
		}

		case HMed: {
			gradients[HMed] = new KPixmap;
			gradients[HMed]->resize(34, 18);
			KPixmapEffect::gradient(*gradients[HMed], c.light(110), c.dark(110),
											KPixmapEffect::HorizontalGradient);
			break;
		}

		case HLarge: {
			gradients[HLarge] = new KPixmap;
			gradients[HLarge]->resize(52, 18);
			KPixmapEffect::gradient(*gradients[HLarge], c.light(110), c.dark(110),
											KPixmapEffect::HorizontalGradient);
			break;
		}

		default:
			break;
	}
	return(gradients[type]);
}



// ---------------------------------------------------------------------------

HighColorStyle::HighColorStyle( StyleType styleType ) 
	: TDEStyle( AllowMenuTransparency | FilledFrameWorkaround, ThreeButtonScrollBar )
{
	type = styleType;
	highcolor = (type == HighColor && TQPixmap::defaultDepth() > 8);
	gDict.setAutoDelete(true);
	selectionBackground = false;
}


HighColorStyle::~HighColorStyle()
{
}


void HighColorStyle::polish(const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr)
{
	if (ceData.widgetObjectTypes.contains("TQWidget")) {
		TQWidget *widget = reinterpret_cast<TQWidget*>(ptr);
		// Put in order of highest occurrence to maximise hit rate
		if (widget->inherits("TQPushButton")) {
			installObjectEventHandler(ceData, elementFlags, ptr, this);
		} else if (widget->inherits("TQMenuBar") || widget->inherits("TQPopupMenu")) {
			widget->setBackgroundMode(TQWidget::NoBackground);
		} else if (type == HighColor && widget->inherits("TQToolBarExtensionWidget")) {
			installObjectEventHandler(ceData, elementFlags, ptr, this);
		} else if ( !qstrcmp( widget->name(), kdeToolbarWidget) ) {
			widget->setBackgroundMode( NoBackground );	// We paint the whole background.
			installObjectEventHandler(ceData, elementFlags, ptr, this);
		} else if (widget->inherits("TQToolBoxButton")) {
			TQFont font = widget->font();
			font.setBold(true);
			widget->setFont(font);
		}
	}

	TDEStyle::polish( ceData, elementFlags, ptr );
}


void HighColorStyle::unPolish(const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr)
{
	if (ceData.widgetObjectTypes.contains("TQWidget")) {
		TQWidget *widget = reinterpret_cast<TQWidget*>(ptr);
		if (widget->inherits("TQPushButton")) {
			removeObjectEventHandler(ceData, elementFlags, ptr, this);
		}
		else if (widget->inherits("TQMenuBar") || widget->inherits("TQPopupMenu")) {
			widget->setBackgroundMode(TQWidget::PaletteBackground);
		} else if (type == HighColor && widget->inherits("TQToolBarExtensionWidget")) {
			removeObjectEventHandler(ceData, elementFlags, ptr, this);
		} else if ( !qstrcmp( widget->name(), kdeToolbarWidget) ) {
			removeObjectEventHandler(ceData, elementFlags, ptr, this);
			widget->setBackgroundMode( PaletteBackground );
		}
	}

	TDEStyle::unPolish( ceData, elementFlags, ptr );
}


/* reimp. */
void HighColorStyle::renderMenuBlendPixmap( KPixmap& pix, const TQColorGroup &cg,
		const TQPopupMenu* /* popup */ ) const
{
	TQColor col = cg.button();

#ifdef TQ_WS_X11 // Only draw menu gradients on TrueColor, X11 visuals
	if ( TQPaintDevice::x11AppDepth() >= 24 )
		KPixmapEffect::gradient( pix, col.light(120), col.dark(115),
				KPixmapEffect::HorizontalGradient );
	else
#endif
	pix.fill( col );
}


// This function draws primitive elements as well as their masks.
void HighColorStyle::drawPrimitive( PrimitiveElement pe,
									TQPainter *p,
									const TQStyleControlElementData &ceData,
									ControlElementFlags elementFlags,
									const TQRect &r,
									const TQColorGroup &cg,
									SFlags flags,
									const TQStyleOption& opt ) const
{
	bool down    = flags & Style_Down;
	bool on      = flags & Style_On;
	bool active  = flags & Style_Active;
	bool reverse = TQApplication::reverseLayout();

	switch(pe)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PE_ButtonDefault: {
			if ( type != HighColor ) {
				int x1, y1, x2, y2;
				r.coords( &x1, &y1, &x2, &y2 );
				
				// Button default indicator
				p->setPen( cg.shadow() );
				p->drawLine( x1+1, y1, x2-1, y1 );
				p->drawLine( x1, y1+1, x1, y2-1 );
				p->drawLine( x1+1, y2, x2-1, y2 );
				p->drawLine( x2, y1+1, x2, y2-1 );
			}
			break;
		}
			
		case PE_ButtonDropDown:
		case PE_ButtonTool: {
			bool sunken = on || down;
			int  x,y,w,h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;
			TQPen oldPen = p->pen();

			// Outer frame (round style)
			p->setPen(cg.shadow());
			p->drawLine(x+1,y,x2-1,y);
			p->drawLine(x,y+1,x,y2-1);
			p->drawLine(x+1,y2,x2-1,y2);
			p->drawLine(x2,y+1,x2,y2-1);

			// Bevel
			p->setPen(sunken ? cg.mid() : cg.light());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1, y2-1);
			p->setPen(sunken ? cg.light() : cg.mid());
			p->drawLine(x+2, y2-1, x2-1, y2-1);
			p->drawLine(x2-1, y+2, x2-1, y2-1);

			p->fillRect(x+2, y+2, w-4, h-4, cg.button());

			p->setPen( oldPen );
			break;
		}

		// PUSH BUTTON
		// -------------------------------------------------------------------
		case PE_ButtonCommand: {
			bool sunken = on || down;
			bool flat = !(flags & (Style_Raised | Style_Sunken));
			int  x, y, w, h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;

			if ( sunken )
				kDrawBeButton( p, x, y, w, h, cg, true,
						&cg.brush(TQColorGroup::Mid) );

			else if ( flags & Style_MouseOver && !flat ) {
				TQBrush brush(cg.button().light(110));
				kDrawBeButton( p, x, y, w, h, cg, false, &brush );
			}

			// "Flat" button
			else if ( flat ) {
				if ( flags & Style_MouseOver )
					p->fillRect(r, cg.button().light(110));
				else
					renderGradient(p, TQRect(x, y, w-1, h-1),
									cg.button(), false);

				p->setPen(cg.button().light(75));
				p->drawLine(x, y, x2, y);
				p->drawLine(x, y, x, y2);
				p->drawLine(x, y2, x2, y2);
				p->drawLine(x2, y, x2, y2);
			}	   

			else if( highcolor )
			{
				int x2 = x+w-1;
				int y2 = y+h-1;
				p->setPen(cg.shadow());
				p->drawLine(x+1, y, x2-1, y);
				p->drawLine(x+1, y2, x2-1, y2);
				p->drawLine(x, y+1, x, y2-1);
				p->drawLine(x2, y+1, x2, y2-1);

				p->setPen(cg.light());
				p->drawLine(x+2, y+2, x2-1, y+2);
				p->drawLine(x+2, y+3, x2-2, y+3);
				p->drawLine(x+2, y+4, x+2, y2-1);
				p->drawLine(x+3, y+4, x+3, y2-2);

				p->setPen(cg.mid());
				p->drawLine(x2-1, y+2, x2-1, y2-1);
				p->drawLine(x+2, y2-1, x2-1, y2-1);

				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+2, x+1, y2-1);
				p->drawLine(x2-2, y+3, x2-2, y2-2);

				renderGradient(p, TQRect(x+4, y+4, w-6, h-6),
								cg.button(), false);
			} else
				kDrawBeButton(p, x, y, w, h, cg, false,
							  &cg.brush(TQColorGroup::Button));
			break;
		}


		// BEVELS
		// -------------------------------------------------------------------
		case PE_ButtonBevel: {
			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			bool sunken = on || down;
			int x2 = x+w-1;
			int y2 = y+h-1;

			// Outer frame
			p->setPen(cg.shadow());
			p->drawRect(r);

			// Bevel
			p->setPen(sunken ? cg.mid() : cg.light());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1, y2-1);
			p->setPen(sunken ? cg.light() : cg.mid());
			p->drawLine(x+2, y2-1, x2-1, y2-1);
			p->drawLine(x2-1, y+2, x2-1, y2-1);

			if (w > 4 && h > 4) {
				if (sunken)
					p->fillRect(x+2, y+2, w-4, h-4, cg.button());
				else
					renderGradient( p, TQRect(x+2, y+2, w-4, h-4),
								    cg.button(), flags & Style_Horizontal );
			}
			break;
		}


		// FOCUS RECT
		// -------------------------------------------------------------------
		case PE_FocusRect: {
			p->drawWinFocusRect( r );
			break;
		}


		// HEADER SECTION
		// -------------------------------------------------------------------
		case PE_HeaderSectionMenu:
		case PE_HeaderSection: {
			// Temporary solution for the proper orientation of gradients.
			bool horizontal = true;
			if (p && p->device()->devType() == TQInternal::Widget) {
				TQHeader* hdr = dynamic_cast<TQHeader*>(p->device());
				if (hdr)
					horizontal = hdr->orientation() == TQt::Horizontal;
			}

			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			bool sunken = on || down;
			int x2 = x+w-1;
			int y2 = y+h-1;
			TQPen oldPen = p->pen();

			// Bevel
			p->setPen(sunken ? cg.mid() : cg.light());
			p->drawLine(x, y, x2-1, y);
			p->drawLine(x, y, x, y2-1);
			p->setPen(sunken ? cg.light() : cg.mid());
			p->drawLine(x+1, y2-1, x2-1, y2-1);
			p->drawLine(x2-1, y+1, x2-1, y2-1);
			p->setPen(cg.shadow());
			p->drawLine(x, y2, x2, y2);
			p->drawLine(x2, y, x2, y2);

			if (sunken)
				p->fillRect(x+1, y+1, w-3, h-3, cg.button());
			else
				renderGradient( p, TQRect(x+1, y+1, w-3, h-3),
							    cg.button(), !horizontal );
			p->setPen( oldPen );
			break;
		}


		// SCROLLBAR
		// -------------------------------------------------------------------
		case PE_ScrollBarSlider: {
			// Small hack to ensure scrollbar gradients are drawn the right way.
			flags ^= Style_Horizontal;

			drawPrimitive(PE_ButtonBevel, p, ceData, elementFlags, r, cg, flags | Style_Enabled | Style_Raised);

			// Draw a scrollbar riffle (note direction after above changes)
			if ( type != B3 ) {
				
				// HighColor & Default scrollbar
				if (flags & Style_Horizontal) {
					if (r.height() >= 15) {
						int x = r.x()+3;
						int y = r.y() + (r.height()-7)/2;
						int x2 = r.right()-3;
						p->setPen(cg.light());
						p->drawLine(x, y, x2, y);
						p->drawLine(x, y+3, x2, y+3);
						p->drawLine(x, y+6, x2, y+6);

						p->setPen(cg.mid());
						p->drawLine(x, y+1, x2, y+1);
						p->drawLine(x, y+4, x2, y+4);
						p->drawLine(x, y+7, x2, y+7);
					}
				} else {
					if (r.width() >= 15) {
						int y = r.y()+3;
						int x = r.x() + (r.width()-7)/2;
						int y2 = r.bottom()-3;
						p->setPen(cg.light());
						p->drawLine(x, y, x, y2);
						p->drawLine(x+3, y, x+3, y2);
						p->drawLine(x+6, y, x+6, y2);

						p->setPen(cg.mid());
						p->drawLine(x+1, y, x+1, y2);
						p->drawLine(x+4, y, x+4, y2);
						p->drawLine(x+7, y, x+7, y2);
					}
				}
			} else {
				
				// B3 scrollbar
				if (flags & Style_Horizontal) {
					int buttons = 0;
					
					if (r.height() >= 36) buttons = 3;
					else if (r.height() >=24) buttons = 2;
					else if (r.height() >=16) buttons = 1;
					
					int x = r.x() + (r.width()-7) / 2;
					int y = r.y() + (r.height() - (buttons * 5) -
							(buttons-1)) / 2;
					int x2 = x + 7;
					
					for ( int i=0; i<buttons; i++, y+=6 )
					{
						p->setPen( cg.mid() );
						p->drawLine( x+1, y, x2-1, y );
						p->drawLine( x, y+1, x, y+3 );
						p->setPen( cg.light() );
						p->drawLine( x+1, y+1, x2-1, y+1 );
						p->drawLine( x+1, y+1, x+1, y+3 );
						p->setPen( cg.dark() );
						p->drawLine( x+1, y+4, x2-1, y+4 );
						p->drawLine( x2, y+1, x2, y+3 );
					}
				} else {
					int buttons = 0;
					
					if (r.width() >= 36) buttons = 3;
					else if (r.width() >=24) buttons = 2;
					else if (r.width() >=16) buttons = 1;
					
					int x = r.x() + (r.width() - (buttons * 5) -
							(buttons-1)) / 2;
					int y = r.y() + (r.height()-7) / 2;
					int y2 = y + 7;
					
					for ( int i=0; i<buttons; i++, x+=6 )
					{
						p->setPen( cg.mid() );
						p->drawLine( x+1, y, x+3, y );
						p->drawLine( x, y+1, x, y2-1 );
						p->setPen( cg.light() );
						p->drawLine( x+1, y+1, x+3, y+1 );
						p->drawLine( x+1, y+1, x+1, y2-1 );
						p->setPen( cg.dark() );
						p->drawLine( x+1, y2, x+3, y2 );
						p->drawLine( x+4, y+1, x+4, y2-1 );
					}
				}
			}
			break;
		}


		case PE_ScrollBarAddPage:
		case PE_ScrollBarSubPage: {
			int x, y, w, h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;

			if ( type != B3 ) {
				// HighColor & Default scrollbar
				
				p->setPen(cg.shadow());
				
				if (flags & Style_Horizontal) {
					p->drawLine(x, y, x2, y);
					p->drawLine(x, y2, x2, y2);
					renderGradient(p, TQRect(x, y+1, w, h-2),
								cg.mid(), false);
				} else {
					p->drawLine(x, y, x, y2);
					p->drawLine(x2, y, x2, y2);
					renderGradient(p, TQRect(x+1, y, w-2, h),
								cg.mid(), true);
				}	
			} else {
				// B3 scrollbar
				
				p->setPen( cg.mid() );
				
				if (flags & Style_Horizontal) {
					p->drawLine(x, y, x2, y);
					p->drawLine(x, y2, x2, y2);
					p->fillRect( TQRect(x, y+1, w, h-2), 
							flags & Style_Down ? cg.button() : cg.midlight() );
				} else {
					p->drawLine(x, y, x, y2);
					p->drawLine(x2, y, x2, y2);
					p->fillRect( TQRect(x+1, y, w-2, h), 
							flags & Style_Down ? cg.button() : cg.midlight() );
				}
			}
			break;
		}


		case PE_ScrollBarAddLine: {
			drawPrimitive( PE_ButtonBevel, p, ceData, elementFlags, r, cg, (flags & Style_Enabled) |
					((flags & Style_Down) ? Style_Down : Style_Raised) );

			drawPrimitive( ((flags & Style_Horizontal) ? PE_ArrowRight : PE_ArrowDown),
					p, ceData, elementFlags, r, cg, flags );
			break;
		}


		case PE_ScrollBarSubLine: {
			drawPrimitive( PE_ButtonBevel, p, ceData, elementFlags, r, cg, (flags & Style_Enabled) |
					((flags & Style_Down) ? Style_Down : Style_Raised) );

			drawPrimitive( ((flags & Style_Horizontal) ? PE_ArrowLeft : PE_ArrowUp),
					p, ceData, elementFlags, r, cg, flags );
			break;
		}


		// CHECKBOX (indicator)
		// -------------------------------------------------------------------
		case PE_Indicator: {
			
			bool enabled  = flags & Style_Enabled;
			bool nochange = flags & Style_NoChange;
			
			if (xBmp.isNull()) {
				xBmp = TQBitmap(7, 7, x_bits, true);
				xBmp.setMask(xBmp);
			}

			int x,y,w,h;
			x=r.x(); y=r.y(); w=r.width(); h=r.height();
			int x2 = x+w-1;
			int y2 = y+h-1;

			p->setPen(cg.mid());
			p->drawLine(x, y, x2, y);
			p->drawLine(x, y, x, y2);

			p->setPen(cg.light());
			p->drawLine(x2, y+1, x2, y2);
			p->drawLine(x+1, y2, x2, y2);

			p->setPen(cg.shadow());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1, y2-1);

			p->setPen(cg.midlight());
			p->drawLine(x2-1, y+2, x2-1, y2-1);
			p->drawLine(x+2, y2-1, x2-1, y2-1);

			if ( enabled )
				p->fillRect(x+2, y+2, w-4, h-4, 
						down ? cg.button(): cg.base());
			else
				p->fillRect(x+2, y+2, w-4, h-4, cg.background());
			
			if (!(flags & Style_Off)) {
				if (on) {
					p->setPen(nochange ? cg.dark() : cg.text());
					p->drawPixmap(x+3, y+3, xBmp);
				}
				else {
					p->setPen(cg.shadow());
					p->drawRect(x+2, y+2, w-4, h-4);
					p->setPen(nochange ? cg.text() : cg.dark());
					p->drawLine(x+3, (y+h)/2-2, x+w-4, (y+h)/2-2);
					p->drawLine(x+3, (y+h)/2, x+w-4, (y+h)/2);
					p->drawLine(x+3, (y+h)/2+2, x+w-4, (y+h)/2+2);
				}
			}
			break;
		}


		// RADIOBUTTON (exclusive indicator)
		// -------------------------------------------------------------------
		case PE_ExclusiveIndicator: {
			
			if (lightBmp.isNull()) {
				lightBmp  = TQBitmap(13, 13, radiooff_light_bits,  true);
				grayBmp   = TQBitmap(13, 13, radiooff_gray_bits,   true);
				dgrayBmp  = TQBitmap(13, 13, radiooff_dgray_bits,  true);
				centerBmp = TQBitmap(13, 13, radiooff_center_bits, true);
				centerBmp.setMask( centerBmp );
			}

			// Bevel
			kColorBitmaps(p, cg, r.x(), r.y(), &lightBmp , &grayBmp,
						  NULL, &dgrayBmp);
			
			// The center fill of the indicator (grayed out when disabled)
			if ( flags & Style_Enabled )
				p->setPen( down ? cg.button() : cg.base() );
			else
				p->setPen( cg.background() );
			p->drawPixmap( r.x(), r.y(), centerBmp );

			// Indicator "dot"
			if ( on ) {
				TQColor color = flags & Style_NoChange ?
					cg.dark() : cg.text();
				
				p->setPen(color);
				p->drawLine(5, 4, 7, 4);
				p->drawLine(4, 5, 4, 7);
				p->drawLine(5, 8, 7, 8);
				p->drawLine(8, 5, 8, 7);
				p->fillRect(5, 5, 3, 3, color);
			}

			break;
		}


		// RADIOBUTTON (exclusive indicator) mask
		// -------------------------------------------------------------------
		case PE_ExclusiveIndicatorMask: {
			if (maskBmp.isNull()) {
				maskBmp = TQBitmap(13, 13, radiomask_bits, true);
				maskBmp.setMask(maskBmp);
			}
				
			p->setPen(TQt::color1);
			p->drawPixmap(r.x(), r.y(), maskBmp);
			break;
		}


		// SPLITTER/DOCKWINDOW HANDLES
		// -------------------------------------------------------------------
		case PE_DockWindowResizeHandle:
		case PE_Splitter: {
			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;

			p->setPen(cg.dark());
			p->drawRect(x, y, w, h);
			p->setPen(cg.background());
			p->drawPoint(x, y);
			p->drawPoint(x2, y);
			p->drawPoint(x, y2);
			p->drawPoint(x2, y2);
			p->setPen(cg.light());
			p->drawLine(x+1, y+1, x+1, y2-1);
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->setPen(cg.midlight());
			p->drawLine(x+2, y+2, x+2, y2-2);
			p->drawLine(x+2, y+2, x2-2, y+2);
			p->setPen(cg.mid());
			p->drawLine(x2-1, y+1, x2-1, y2-1);
			p->drawLine(x+1, y2-1, x2-1, y2-1);
			p->fillRect(x+3, y+3, w-5, h-5, cg.brush(TQColorGroup::Background));
			break;
		}


		// GENERAL PANELS
		// -------------------------------------------------------------------
		case PE_Panel:
		case PE_PanelPopup:
		case PE_WindowFrame:
		case PE_PanelLineEdit: {
			bool sunken  = flags & Style_Sunken;
			int lw = opt.isDefault() ? pixelMetric(PM_DefaultFrameWidth, ceData, elementFlags)
										: opt.lineWidth();
			if (lw == 2)
			{
				TQPen oldPen = p->pen();
				int x,y,w,h;
				r.rect(&x, &y, &w, &h);
				int x2 = x+w-1;
				int y2 = y+h-1;
				p->setPen(sunken ? cg.light() : cg.dark());
				p->drawLine(x, y2, x2, y2);
				p->drawLine(x2, y, x2, y2);
				p->setPen(sunken ? cg.mid() : cg.light());
				p->drawLine(x, y, x2, y);
				p->drawLine(x, y, x, y2);
				p->setPen(sunken ? cg.midlight() : cg.mid());
				p->drawLine(x+1, y2-1, x2-1, y2-1);
				p->drawLine(x2-1, y+1, x2-1, y2-1);
				p->setPen(sunken ? cg.dark() : cg.midlight());
				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+1, x+1, y2-1);
				p->setPen(oldPen);
			} else
				TDEStyle::drawPrimitive(pe, p, ceData, elementFlags, r, cg, flags, opt);

			break;
		}


		// MENU / TOOLBAR PANEL
		// -------------------------------------------------------------------
		case PE_PanelMenuBar: 			// Menu
		case PE_PanelDockWindow: {		// Toolbar
			int x2 = r.x()+r.width()-1;
			int y2 = r.y()+r.height()-1;
			int lw = opt.isDefault() ? pixelMetric(PM_DefaultFrameWidth, ceData, elementFlags)
				: opt.lineWidth();
			
			if (lw)
			{
				p->setPen(cg.light());
				p->drawLine(r.x(), r.y(), x2-1,  r.y());
				p->drawLine(r.x(), r.y(), r.x(), y2-1);
				p->setPen(cg.dark());
				p->drawLine(r.x(), y2, x2, y2);
				p->drawLine(x2, r.y(), x2, y2);

				// ### Qt should specify Style_Horizontal where appropriate
				renderGradient( p, TQRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2),
								cg.button(), 
								(r.width() < r.height()) && (pe != PE_PanelMenuBar) );
			}
			else
				renderGradient( p, r, cg.button(), 
								(r.width() < r.height()) && (pe != PE_PanelMenuBar) );

			break;
		}



		// TOOLBAR SEPARATOR
		// -------------------------------------------------------------------
		case PE_DockWindowSeparator: {
			renderGradient( p, r, cg.button(),
							!(flags & Style_Horizontal));
			if ( !(flags & Style_Horizontal) ) {
				p->setPen(cg.mid());
				p->drawLine(4, r.height()/2, r.width()-5, r.height()/2);
				p->setPen(cg.light());
				p->drawLine(4, r.height()/2+1, r.width()-5, r.height()/2+1);
			} else {
				p->setPen(cg.mid());
				p->drawLine(r.width()/2, 4, r.width()/2, r.height()-5);
				p->setPen(cg.light());
				p->drawLine(r.width()/2+1, 4, r.width()/2+1, r.height()-5);
			}
			break;
		}

		case PE_MenuItemIndicatorFrame: {
				int x, y, w, h;
				r.rect( &x, &y, &w, &h );
				int checkcol = styleHint(SH_MenuIndicatorColumnWidth, ceData, elementFlags, opt, NULL, NULL);
				TQRect cr = visualRect( TQRect(x, y, checkcol, h), r );

				int cx = reverse ? x+w - checkcol : x;

				// We only have to draw the background if the menu item is inactive -
				// if it's active the "pressed" background is already drawn
				if ( ! active )
					qDrawShadePanel( p, cx, y, checkcol, h, cg, true, 1, &cg.brush(TQColorGroup::Midlight) );
			}
			break;

		case PE_MenuItemIndicatorIconFrame: {
				int x, y, w, h;
				r.rect( &x, &y, &w, &h );
				int checkcol = styleHint(SH_MenuIndicatorColumnWidth, ceData, elementFlags, opt, NULL, NULL);
				TQRect cr = visualRect( TQRect(x, y, checkcol, h), r );
				qDrawShadePanel( p, cr.x(), cr.y(), cr.width(), cr.height(), cg, true, 1, &cg.brush(TQColorGroup::Midlight) );
			}
			break;

		case PE_MenuItemIndicatorCheck: {
				int x, y, w, h;
				r.rect( &x, &y, &w, &h );
				int checkcol = styleHint(SH_MenuIndicatorColumnWidth, ceData, elementFlags, opt, NULL, NULL);
				TQRect cr = visualRect( TQRect(x, y, checkcol, h), r );

				int cx = reverse ? x+w - checkcol : x;

				// Draw the checkmark
				SFlags cflags = Style_Default;
				cflags |= active ? Style_Enabled : Style_On;

				drawPrimitive( PE_CheckMark, p, ceData, elementFlags, TQRect( cx + itemFrame, y + itemFrame, checkcol - itemFrame*2, h - itemFrame*2), cg, cflags );
			}
			break;

		default:
		{
			// ARROWS
			// -------------------------------------------------------------------
			if (pe >= PE_ArrowUp && pe <= PE_ArrowLeft)
			{
				TQPointArray a;
				
				if ( type != B3 ) {
					// HighColor & Default arrows
					switch(pe) {
						case PE_ArrowUp:
							a.setPoints(TQCOORDARRLEN(u_arrow), u_arrow);
							break;

						case PE_ArrowDown:
							a.setPoints(TQCOORDARRLEN(d_arrow), d_arrow);
							break;

						case PE_ArrowLeft:
							a.setPoints(TQCOORDARRLEN(l_arrow), l_arrow);
							break;

						default:
							a.setPoints(TQCOORDARRLEN(r_arrow), r_arrow);
					}
				} else {
					// B3 arrows
					switch(pe) {
						case PE_ArrowUp:
							a.setPoints(TQCOORDARRLEN(B3::u_arrow), B3::u_arrow);
							break;

						case PE_ArrowDown:
							a.setPoints(TQCOORDARRLEN(B3::d_arrow), B3::d_arrow);
							break;

						case PE_ArrowLeft:
							a.setPoints(TQCOORDARRLEN(B3::l_arrow), B3::l_arrow);
							break;

						default:
							a.setPoints(TQCOORDARRLEN(B3::r_arrow), B3::r_arrow);
					}
				}
					
				p->save();
				if ( flags & Style_Down )
					p->translate( pixelMetric( PM_ButtonShiftHorizontal, ceData, elementFlags ),
								  pixelMetric( PM_ButtonShiftVertical, ceData, elementFlags ) );

				if ( flags & Style_Enabled ) {
					a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
					p->setPen( cg.buttonText() );
					p->drawLineSegments( a );
				} else {
					a.translate( r.x() + r.width() / 2 + 1, r.y() + r.height() / 2 + 1 );
					p->setPen( cg.light() );
					p->drawLineSegments( a );
					a.translate( -1, -1 );
					p->setPen( cg.mid() );
					p->drawLineSegments( a );
				}
				p->restore();
			
			} else
				TDEStyle::drawPrimitive( pe, p, ceData, elementFlags, r, cg, flags, opt );
		}
	}
}


void HighColorStyle::drawTDEStylePrimitive( TDEStylePrimitive kpe,
										  TQPainter* p,
										  const TQStyleControlElementData &ceData,
										  ControlElementFlags elementFlags,
										  const TQRect &r,
										  const TQColorGroup &cg,
										  SFlags flags,
										  const TQStyleOption &opt,
										  const TQWidget* widget ) const
{
	switch ( kpe )
	{
		// TOOLBAR HANDLE
		// -------------------------------------------------------------------
		case KPE_ToolBarHandle: {
			int x = r.x(); int y = r.y();
			int x2 = r.x() + r.width()-1;
			int y2 = r.y() + r.height()-1;

			if (flags & Style_Horizontal) {

				renderGradient( p, r, cg.button(), false);
				p->setPen(cg.light());
				p->drawLine(x+1, y+4, x+1, y2-4);
				p->drawLine(x+3, y+4, x+3, y2-4);
				p->drawLine(x+5, y+4, x+5, y2-4);

				p->setPen(cg.mid());
				p->drawLine(x+2, y+4, x+2, y2-4);
				p->drawLine(x+4, y+4, x+4, y2-4);
				p->drawLine(x+6, y+4, x+6, y2-4);

			} else {
				
				renderGradient( p, r, cg.button(), true);
				p->setPen(cg.light());
				p->drawLine(x+4, y+1, x2-4, y+1);
				p->drawLine(x+4, y+3, x2-4, y+3);
				p->drawLine(x+4, y+5, x2-4, y+5);

				p->setPen(cg.mid());
				p->drawLine(x+4, y+2, x2-4, y+2);
				p->drawLine(x+4, y+4, x2-4, y+4);
				p->drawLine(x+4, y+6, x2-4, y+6);

			}
			break;
		}

							   
		// GENERAL/KICKER HANDLE
		// -------------------------------------------------------------------
		case KPE_GeneralHandle: {
			int x = r.x(); int y = r.y();
			int x2 = r.x() + r.width()-1;
			int y2 = r.y() + r.height()-1;

			if (flags & Style_Horizontal) {

				p->setPen(cg.light());
				p->drawLine(x+1, y, x+1, y2);
				p->drawLine(x+3, y, x+3, y2);
				p->drawLine(x+5, y, x+5, y2);

				p->setPen(cg.mid());
				p->drawLine(x+2, y, x+2, y2);
				p->drawLine(x+4, y, x+4, y2);
				p->drawLine(x+6, y, x+6, y2);
				
			} else {

				p->setPen(cg.light());
				p->drawLine(x, y+1, x2, y+1);
				p->drawLine(x, y+3, x2, y+3);
				p->drawLine(x, y+5, x2, y+5);

				p->setPen(cg.mid());
				p->drawLine(x, y+2, x2, y+2);
				p->drawLine(x, y+4, x2, y+4);
				p->drawLine(x, y+6, x2, y+6);

			}
			break;
		}


		// SLIDER GROOVE
		// -------------------------------------------------------------------
		case KPE_SliderGroove: {
			bool horizontal = ceData.orientation == TQt::Horizontal;
			int gcenter = (horizontal ? r.height() : r.width()) / 2;

			TQRect gr;
			if (horizontal)
				gr = TQRect(r.x(), r.y()+gcenter-3, r.width(), 7);
			else
				gr = TQRect(r.x()+gcenter-3, r.y(), 7, r.height());

			int x,y,w,h;
			gr.rect(&x, &y, &w, &h);
			int x2=x+w-1;
			int y2=y+h-1;

			// Draw the slider groove.
			p->setPen(cg.dark());
			p->drawLine(x+2, y, x2-2, y);
			p->drawLine(x, y+2, x, y2-2);
			p->fillRect(x+2,y+2,w-4, h-4, 
				(elementFlags & CEF_IsEnabled) ? cg.dark() : cg.mid());
			p->setPen(cg.shadow());
			p->drawRect(x+1, y+1, w-2, h-2);
			p->setPen(cg.light());
			p->drawPoint(x+1,y2-1);
			p->drawPoint(x2-1,y2-1);
			p->drawLine(x2, y+2, x2, y2-2);
			p->drawLine(x+2, y2, x2-2, y2);
			break;
		}

		// SLIDER HANDLE
		// -------------------------------------------------------------------
		case KPE_SliderHandle: {
			bool horizontal = ceData.orientation == TQt::Horizontal;
			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;
				
			p->setPen(cg.mid());
			p->drawLine(x+1, y, x2-1, y);
			p->drawLine(x, y+1, x, y2-1);
			p->setPen(cg.shadow());
			p->drawLine(x+1, y2, x2-1, y2);
			p->drawLine(x2, y+1, x2, y2-1);

			p->setPen(cg.light());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1,  y2-1);
			p->setPen(cg.dark());
			p->drawLine(x+2, y2-1, x2-1, y2-1);
			p->drawLine(x2-1, y+2, x2-1, y2-1);
			p->setPen(cg.midlight());
			p->drawLine(x+2, y+2, x2-2, y+2);
			p->drawLine(x+2, y+2, x+2, y2-2);
			p->setPen(cg.mid());
			p->drawLine(x+3, y2-2, x2-2, y2-2);
			p->drawLine(x2-2, y+3, x2-2, y2-2);
			renderGradient(p, TQRect(x+3, y+3, w-6, h-6), 
						   cg.button(), !horizontal);

			// Paint riffles
			if (horizontal) {
				p->setPen(cg.light());
				p->drawLine(x+5, y+4, x+5, y2-4);
				p->drawLine(x+8, y+4, x+8, y2-4);
				p->drawLine(x+11,y+4, x+11, y2-4);
				p->setPen((elementFlags & CEF_IsEnabled) ? cg.shadow(): cg.mid());
				p->drawLine(x+6, y+4, x+6, y2-4);
				p->drawLine(x+9, y+4, x+9, y2-4);
				p->drawLine(x+12,y+4, x+12, y2-4);
			} else {
				p->setPen(cg.light());
				p->drawLine(x+4, y+5, x2-4, y+5);
				p->drawLine(x+4, y+8, x2-4, y+8);
				p->drawLine(x+4, y+11, x2-4, y+11);
				p->setPen((elementFlags & CEF_IsEnabled) ? cg.shadow() : cg.mid());
				p->drawLine(x+4, y+6, x2-4, y+6);
				p->drawLine(x+4, y+9, x2-4, y+9);
				p->drawLine(x+4, y+12, x2-4, y+12);
			}
			break;
		}

		default:
			TDEStyle::drawTDEStylePrimitive( kpe, p, ceData, elementFlags, r, cg, flags, opt, widget);
	}
}


void HighColorStyle::drawControl( ControlElement element,
								  TQPainter *p,
								  const TQStyleControlElementData &ceData,
								  ControlElementFlags elementFlags,
								  const TQRect &r,
								  const TQColorGroup &cg,
								  SFlags flags,
								  const TQStyleOption& opt,
								  const TQWidget *widget ) const
{
	switch (element)
	{
		// PUSHBUTTON
		// -------------------------------------------------------------------
		case CE_PushButton: {
			if ( type != HighColor ) {
				TQRect br = r;
				bool btnDefault = (elementFlags & CEF_IsDefault);
				
				if ( btnDefault || (elementFlags & CEF_AutoDefault) ) {
					// Compensate for default indicator
					static int di = pixelMetric( PM_ButtonDefaultIndicator, ceData, elementFlags );
					br.addCoords( di, di, -di, -di );
				}

				if ( btnDefault )
					drawPrimitive( PE_ButtonDefault, p, ceData, elementFlags, r, cg, flags );
				
				drawPrimitive( PE_ButtonCommand, p, ceData, elementFlags, br, cg, flags );
				
			} else
				drawPrimitive( PE_ButtonCommand, p, ceData, elementFlags, r, cg, flags );
			
			break;
		}


		// PUSHBUTTON LABEL
		// -------------------------------------------------------------------
		case CE_PushButtonLabel: {
			bool active = ((elementFlags & CEF_IsOn) || (elementFlags & CEF_IsDown));
			int x, y, w, h;
			r.rect( &x, &y, &w, &h );

			// Shift button contents if pushed.
			if ( active ) {
				x += pixelMetric(PM_ButtonShiftHorizontal, ceData, elementFlags, widget); 
				y += pixelMetric(PM_ButtonShiftVertical, ceData, elementFlags, widget);
				flags |= Style_Sunken;
			}

			// Does the button have a popup menu?
			if ( elementFlags & CEF_IsMenuWidget ) {
				int dx = pixelMetric( PM_MenuButtonIndicator, ceData, elementFlags, widget );
				drawPrimitive( PE_ArrowDown, p, ceData, elementFlags, TQRect(x + w - dx - 2, y + 2, dx, h - 4),
							   cg, flags, opt );
				w -= dx;
			}

			// Draw the icon if there is one
			if ( !ceData.iconSet.isNull() ) {
				TQIconSet::Mode  mode  = TQIconSet::Disabled;
				TQIconSet::State state = TQIconSet::Off;

				if (elementFlags & CEF_IsEnabled)
					mode = (elementFlags & CEF_HasFocus) ? TQIconSet::Active : TQIconSet::Normal;
				if ((elementFlags & CEF_BiState) && (elementFlags & CEF_IsOn))
					state = TQIconSet::On;

				TQPixmap pixmap = ceData.iconSet.pixmap( TQIconSet::Small, mode, state );

				// Center the iconset if there's no text or pixmap
				if (ceData.text.isEmpty() && ceData.fgPixmap.isNull())
					p->drawPixmap( x + (w - pixmap.width())  / 2, 
								   y + (h - pixmap.height()) / 2, pixmap );
				else
					p->drawPixmap( x + 4, y + (h - pixmap.height()) / 2, pixmap );

				int  pw = pixmap.width();
				x += pw + 4;
				w -= pw + 4;
			}

			// Make the label indicate if the button is a default button or not
			if ( active || (elementFlags & CEF_IsDefault) ) {
				// Draw "fake" bold text  - this enables the font metrics to remain
				// the same as computed in TQPushButton::sizeHint(), but gives
				// a reasonable bold effect.
				int i;

				// Text shadow
				if (elementFlags & CEF_IsEnabled) // Don't draw double-shadow when disabled
					for(i=0; i<2; i++)
						drawItem( p, TQRect(x+i+1, y+1, w, h), AlignCenter | ShowPrefix, 
								ceData.colorGroup, (elementFlags & CEF_IsEnabled), NULL,
								ceData.text, -1,	
								(active ? &ceData.colorGroup.dark() : &ceData.colorGroup.mid()) );

				// Normal Text
				for(i=0; i<2; i++)
					drawItem( p, TQRect(x+i, y, w, h), AlignCenter | ShowPrefix, 
							ceData.colorGroup, (elementFlags & CEF_IsEnabled), ((i == 0) ? (ceData.fgPixmap.isNull())?NULL:&ceData.fgPixmap : NULL),
							ceData.text, -1,
							(active ? &ceData.colorGroup.light() : &ceData.colorGroup.buttonText()) );
			} else {
				drawItem( p, TQRect(x, y, w, h), AlignCenter | ShowPrefix, ceData.colorGroup,
						(elementFlags & CEF_IsEnabled), (ceData.fgPixmap.isNull())?NULL:&ceData.fgPixmap, ceData.text, -1,
						(active ? &ceData.colorGroup.light() : &ceData.colorGroup.buttonText()) );
			}

			// Draw a focus rect if the button has focus
			if ( flags & Style_HasFocus )
				drawPrimitive( PE_FocusRect, p, ceData, elementFlags,
						TQStyle::visualRect(subRect(SR_PushButtonFocusRect, ceData, elementFlags, widget), ceData, elementFlags),
						cg, flags );
			break;
		}

		// TOOLBOX TAB
		// -------------------------------------------------------------------
		case CE_ToolBoxTab:
		{
			bool pressed = flags & Style_Down;
			bool selected = flags & Style_Selected;
			int x, y, x2, y2;
			r.coords( &x, &y, &x2, &y2 );

			p->setPen( pressed ? cg.shadow() : cg.light() );
			p->drawLine( x, y, x2-1, y );
			p->drawLine( x, y, x, y2-1 );

			p->setPen( pressed ? cg.light() : cg.shadow() );
			p->drawLine( x, y2, x2, y2 );
			p->drawLine( x2, y, x2, y2 );

			TQColor fill = selected ? cg.highlight() : cg.button();
			selectionBackground = selected;

			if ( pressed )
				p->fillRect( TQRect(x+1, y+1, r.width()-2, r.height()-2), fill );
			else
				renderGradient(p, TQRect(x+1, y+1, r.width()-2, r.height()-2),
						fill, false);
			break;
		}

		// MENUBAR BACKGROUND
		// -------------------------------------------------------------------
		case CE_MenuBarEmptyArea:
		{
			renderGradient(p, r, cg.button(), false);
			break;
		}

		// MENUBAR ITEM (sunken panel on mouse over)
		// -------------------------------------------------------------------
		case CE_MenuBarItem:
		{
			TQMenuBar  *mb = (TQMenuBar*)widget;
			TQMenuItem *mi = opt.menuItem();
			TQRect      pr = mb->rect();

			bool active  = flags & Style_Active;
			bool focused = flags & Style_HasFocus;

			if ( active && focused )
				qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(),
								cg, true, 1, &cg.brush(TQColorGroup::Midlight));
			else
				renderGradient( p, r, cg.button(), false,
								r.x(), r.y()-1, pr.width()-2, pr.height()-2);

			drawItem( p, r, AlignCenter | AlignVCenter | ShowPrefix
					| DontClip | SingleLine, cg, flags & Style_Enabled,
					mi->pixmap(), mi->text() );

			break;
		}


		// POPUPMENU ITEM
		// -------------------------------------------------------------------
		case CE_PopupMenuItem: {
			TQMenuItem *mi = opt.menuItem();
			if ( !mi ) {
				// Don't leave blank holes if we set NoBackground for the TQPopupMenu.
				// This only happens when the popupMenu spans more than one column.
				if (! (!ceData.bgPixmap.isNull()) )
					p->fillRect(r, cg.brush(TQColorGroup::Button) );
				break;
			}

			int  tab        = opt.tabWidth();
			int  checkcol   = opt.maxIconWidth();
			bool enabled    = mi->isEnabled();
			bool checkable  = (elementFlags & CEF_IsCheckable);
			bool active     = flags & Style_Active;
			bool etchtext   = styleHint( SH_EtchDisabledText, ceData, elementFlags );
			bool reverse    = TQApplication::reverseLayout();
			int x, y, w, h;
			r.rect( &x, &y, &w, &h );

			if ( checkable )
				checkcol = TQMAX( checkcol, 20 );

			// Are we a menu item separator?
			if ( mi->isSeparator() ) {
				p->setPen( cg.dark() );
				p->drawLine( x, y, x+w, y );
				p->setPen( cg.light() );
				p->drawLine( x, y+1, x+w, y+1 );
				break;
			}

			// Draw the menu item background
			if ( active )
				qDrawShadePanel( p, x, y, w, h, cg, true, 1,
				                 &cg.brush(TQColorGroup::Midlight) );
			// Draw the transparency pixmap
			else if ( !ceData.bgPixmap.isNull() )
				p->drawPixmap( x, y, ceData.bgPixmap, x, y, w, h );
			// Draw a solid background
			else
				p->fillRect( r, cg.button() );

			// Do we have an icon?
			if ( mi->iconSet() ) {
				TQIconSet::Mode mode;
				TQRect cr = visualRect( TQRect(x, y, checkcol, h), r );
				
				// Select the correct icon from the iconset
				if ( active )
					mode = enabled ? TQIconSet::Active : TQIconSet::Disabled;
				else
					mode = enabled ? TQIconSet::Normal : TQIconSet::Disabled;

				// Do we have an icon and are checked at the same time?
				// Then draw a "pressed" background behind the icon
				if ( checkable && !active && mi->isChecked() )
					drawPrimitive(PE_MenuItemIndicatorIconFrame, p, ceData, elementFlags, TQRect(x, y, checkcol, h), cg, flags, opt);
				// Draw the icon
				TQPixmap pixmap = mi->iconSet()->pixmap( TQIconSet::Small, mode );
				TQRect pmr( 0, 0, pixmap.width(), pixmap.height() );
				pmr.moveCenter( cr.center() );
				p->drawPixmap( pmr.topLeft(), pixmap );
			}

			// Are we checked? (This time without an icon)
			else if ( checkable && mi->isChecked() ) {
				drawPrimitive(PE_MenuItemIndicatorFrame, p, ceData, elementFlags, TQRect(x, y, checkcol, h), cg, flags, opt);
				drawPrimitive(PE_MenuItemIndicatorCheck, p, ceData, elementFlags, TQRect(x, y, checkcol, h), cg, flags, opt);
			}

			// Time to draw the menu item label...
			int xm = itemFrame + checkcol + itemHMargin; // X position margin
			
			int xp = reverse ? // X position
					x + tab + rightBorder + itemHMargin + itemFrame - 1 :
					x + xm;
			
			int offset = reverse ? -1 : 1;	// Shadow offset for etched text
			
			// Label width (minus the width of the accelerator portion)
			int tw = w - xm - tab - arrowHMargin - itemHMargin * 3 - itemFrame + 1; 

			// Set the color for enabled and disabled text 
			// (used for both active and inactive menu items)
			p->setPen( enabled ? cg.buttonText() : cg.mid() );

			// This color will be used instead of the above if the menu item
			// is active and disabled at the same time. (etched text)
			TQColor discol = cg.mid();

			// Does the menu item draw it's own label?
			if ( mi->custom() ) {
				int m = itemVMargin;
				// Save the painter state in case the custom
				// paint method changes it in some way
				p->save();

				// Draw etched text if we're inactive and the menu item is disabled
				if ( etchtext && !enabled && !active ) {
					p->setPen( cg.light() );
					mi->custom()->paint( p, cg, active, enabled, xp+offset, y+m+1, tw, h-2*m );
					p->setPen( discol );
				}
				mi->custom()->paint( p, cg, active, enabled, xp, y+m, tw, h-2*m );
				p->restore();
			}
			else {
				// The menu item doesn't draw it's own label
				TQString s = mi->text();

				// Does the menu item have a text label?
				if ( !s.isNull() ) {
					int t = s.find( '\t' );
					int m = itemVMargin;
					int text_flags = AlignVCenter | ShowPrefix | DontClip | SingleLine;
					text_flags |= reverse ? AlignRight : AlignLeft;
					
					// Does the menu item have a tabstop? (for the accelerator text)
					if ( t >= 0 ) {
						int tabx = reverse ? x + rightBorder + itemHMargin + itemFrame :
							x + w - tab - rightBorder - itemHMargin - itemFrame;

						// Draw the right part of the label (accelerator text)
						if ( etchtext && !enabled && !active ) {
							// Draw etched text if we're inactive and the menu item is disabled
							p->setPen( cg.light() );
							p->drawText( tabx+offset, y+m+1, tab, h-2*m, text_flags, s.mid( t+1 ) );
							p->setPen( discol );
						}
						p->drawText( tabx, y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
						s = s.left( t );
					}

					// Draw the left part of the label (or the whole label 
					// if there's no accelerator)
					if ( etchtext && !enabled && !active ) {
						// Etched text again for inactive disabled menu items...
						p->setPen( cg.light() );
						p->drawText( xp+offset, y+m+1, tw, h-2*m, text_flags, s, t );
						p->setPen( discol );
					}

					p->drawText( xp, y+m, tw, h-2*m, text_flags, s, t );

				}

				// The menu item doesn't have a text label
				// Check if it has a pixmap instead
				else if ( mi->pixmap() ) {
					TQPixmap *pixmap = mi->pixmap();

					// Draw the pixmap
					if ( pixmap->depth() == 1 )
						p->setBackgroundMode( TQt::OpaqueMode );

					int diffw = ( ( w - pixmap->width() ) / 2 )
									+ ( ( w - pixmap->width() ) % 2 );
					p->drawPixmap( x+diffw, y+itemFrame, *pixmap );
					
					if ( pixmap->depth() == 1 )
						p->setBackgroundMode( TQt::TransparentMode );
				}
			}

			// Does the menu item have a submenu?
			if ( mi->popup() ) {
				PrimitiveElement arrow = reverse ? PE_ArrowLeft : PE_ArrowRight;
				int dim = pixelMetric(PM_MenuButtonIndicator, ceData, elementFlags);
				TQRect vr = visualRect( TQRect( x + w - arrowHMargin - 2*itemFrame - dim,
							y + h / 2 - dim / 2, dim, dim), r );

				// Draw an arrow at the far end of the menu item
				if ( active ) {
					if ( enabled )
						discol = cg.buttonText();

					TQColorGroup g2( discol, cg.highlight(), white, white,
									enabled ? white : discol, discol, white );

					drawPrimitive( arrow, p, ceData, elementFlags, vr, g2, Style_Enabled );
				} else
					drawPrimitive( arrow, p, ceData, elementFlags, vr, cg,
							enabled ? Style_Enabled : Style_Default );
			}
			break;
		}

		default:
			TDEStyle::drawControl(element, p, ceData, elementFlags, r, cg, flags, opt, widget);
	}
}


void HighColorStyle::drawControlMask( ControlElement element,
								  	  TQPainter *p,
								  	  const TQStyleControlElementData &ceData,
								  	  ControlElementFlags elementFlags,
								  	  const TQRect &r,
								  	  const TQStyleOption& opt,
								  	  const TQWidget *widget ) const
{
	switch (element)
	{
		// PUSHBUTTON MASK
		// ----------------------------------------------------------------------
		case CE_PushButton: {
			int x1, y1, x2, y2;
			r.coords( &x1, &y1, &x2, &y2 );
			TQCOORD corners[] = { x1,y1, x2,y1, x1,y2, x2,y2 };
			p->fillRect( r, color1 );
			p->setPen( color0 );
			p->drawPoints( TQPointArray(4, corners) );
			break;
		}

		default:
			TDEStyle::drawControlMask(element, p, ceData, elementFlags, r, opt, widget);
	}
}


void HighColorStyle::drawComplexControl( ComplexControl control,
                                         TQPainter *p,
                                         const TQStyleControlElementData &ceData,
                                         ControlElementFlags elementFlags,
                                         const TQRect &r,
                                         const TQColorGroup &cg,
                                         SFlags flags,
									     SCFlags controls,
									     SCFlags active,
                                         const TQStyleOption& opt,
                                         const TQWidget *widget ) const
{
	switch(control)
	{
		// COMBOBOX
		// -------------------------------------------------------------------
		case CC_ComboBox: {

			// Draw box and arrow
			if ( controls & SC_ComboBoxArrow ) {
				bool sunken = (active == SC_ComboBoxArrow);

				// Draw the combo
				int x,y,w,h;
				r.rect(&x, &y, &w, &h);
				int x2 = x+w-1;
				int y2 = y+h-1;

				p->setPen(cg.shadow());
				p->drawLine(x+1, y, x2-1, y);
				p->drawLine(x+1, y2, x2-1, y2);
				p->drawLine(x, y+1, x, y2-1);
				p->drawLine(x2, y+1, x2, y2-1);

				// Ensure the edge notches are properly colored
				p->setPen(cg.button());
				p->drawPoint(x,y);
				p->drawPoint(x,y2);
				p->drawPoint(x2,y);
				p->drawPoint(x2,y2);

				renderGradient( p, TQRect(x+2, y+2, w-4, h-4),
								cg.button(), false);

				p->setPen(sunken ? cg.light() : cg.mid());
				p->drawLine(x2-1, y+2, x2-1, y2-1);
				p->drawLine(x+1, y2-1, x2-1, y2-1);

				p->setPen(sunken ? cg.mid() : cg.light());
				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+2, x+1, y2-2);

				// Get the button bounding box
				TQRect ar = TQStyle::visualRect(
					querySubControlMetrics(CC_ComboBox, ceData, elementFlags, SC_ComboBoxArrow, TQStyleOption::Default, widget),
					ceData, elementFlags );

				// Are we enabled?
				if ( elementFlags & CEF_IsEnabled )
					flags |= Style_Enabled;

				// Are we "pushed" ?
				if ( active & Style_Sunken )
					flags |= Style_Sunken;

				drawPrimitive(PE_ArrowDown, p, ceData, elementFlags, ar, cg, flags);
			}

			// Draw an edit field if required
			if ( controls & SC_ComboBoxEditField )
			{
				const TQComboBox * cb = (const TQComboBox *) widget;
				TQRect re = TQStyle::visualRect(
					querySubControlMetrics( CC_ComboBox, ceData, elementFlags,
						                    SC_ComboBoxEditField, TQStyleOption::Default, widget), ceData, elementFlags );

				// Draw the indent
				if ( elementFlags & CEF_IsEditable ) {
					p->setPen( cg.dark() );
					p->drawLine( re.x(), re.y()-1, re.x()+re.width(), re.y()-1 );
					p->drawLine( re.x()-1, re.y(), re.x()-1, re.y()+re.height() );
				}

				if ( elementFlags & CEF_HasFocus ) {
					p->setPen( cg.highlightedText() );
					p->setBackgroundColor( cg.highlight() );
				} else {
					p->setPen( cg.text() );
					p->setBackgroundColor( cg.button() );
				}

				if (  (elementFlags & CEF_HasFocus) && !(elementFlags & CEF_IsEditable) ) {
					// Draw the contents
					p->fillRect( re.x(), re.y(), re.width(), re.height(),
								 cg.brush( TQColorGroup::Highlight ) );

					TQRect re = TQStyle::visualRect( 
								subRect(SR_ComboBoxFocusRect, ceData, elementFlags, cb), ceData, elementFlags);

					drawPrimitive( PE_FocusRect, p, ceData, elementFlags, re, cg,
								   Style_FocusAtBorder, TQStyleOption(cg.highlight()));
				}
			}
			break;
		}

		// TOOLBUTTON
		// -------------------------------------------------------------------
		case CC_ToolButton: {
			TQRect button, menuarea;
			button   = querySubControlMetrics(control, ceData, elementFlags, SC_ToolButton, opt, widget);
			menuarea = querySubControlMetrics(control, ceData, elementFlags, SC_ToolButtonMenu, opt, widget);

			SFlags bflags = flags,
				   mflags = flags;

			if (active & SC_ToolButton)
				bflags |= Style_Down;
			if (active & SC_ToolButtonMenu)
				mflags |= Style_Down;

			if (controls & SC_ToolButton)
			{
				// If we're pressed, on, or raised...
				if (bflags & (Style_Down | Style_On | Style_Raised))
					drawPrimitive(PE_ButtonTool, p, ceData, elementFlags, button, cg, bflags, opt);

				// Check whether to draw a background pixmap
				else if ( !ceData.parentWidgetData.bgPixmap.isNull() )
				{
					TQPixmap pixmap = ceData.parentWidgetData.bgPixmap;
					p->drawTiledPixmap( r, pixmap, ceData.pos );
				}
				else if (!ceData.parentWidgetData.widgetObjectTypes.isEmpty())
				{
					if (ceData.parentWidgetData.widgetObjectTypes.contains("TQToolBar"))
					{
						TQToolBar* parent = (TQToolBar*)widget->parent();
						TQRect pr = parent->rect();

						renderGradient( p, r, cg.button(),
									parent->orientation() == TQt::Vertical,
									r.x(), r.y(), pr.width()-2, pr.height()-2);
					}
					else if (ceData.parentWidgetData.widgetObjectTypes.contains("TQToolBarExtensionWidget"))
					{
						TQWidget* parent = (TQWidget*)widget->parent();
						TQToolBar* toolbar = (TQToolBar*)parent->parent();
						TQRect tr = toolbar->rect();

						if ( toolbar->orientation() == TQt::Horizontal ) {
							renderGradient( p, r, cg.button(), false, r.x(), r.y(),
									r.width(), tr.height() );
						} else {
							renderGradient( p, r, cg.button(), true, r.x(), r.y(),
									tr.width(), r.height() );
						}
					}
				}
			}

			// Draw a toolbutton menu indicator if required
			if (controls & SC_ToolButtonMenu)
			{
				if (mflags & (Style_Down | Style_On | Style_Raised))
					drawPrimitive(PE_ButtonDropDown, p, ceData, elementFlags, menuarea, cg, mflags, opt);
				drawPrimitive(PE_ArrowDown, p, ceData, elementFlags, menuarea, cg, mflags, opt);
			}

			if ((elementFlags & CEF_HasFocus) && !(elementFlags & CEF_HasFocusProxy)) {
				TQRect fr = ceData.rect;
				fr.addCoords(3, 3, -3, -3);
				drawPrimitive(PE_FocusRect, p, ceData, elementFlags, fr, cg);
			}

			break;
		}


		default:
			TDEStyle::drawComplexControl(control, p, ceData, elementFlags,
						r, cg, flags, controls, active, opt, widget);
			break;
	}
}


void HighColorStyle::drawComplexControlMask( ComplexControl control,
											 TQPainter *p,
											 const TQStyleControlElementData &ceData,
											 const ControlElementFlags elementFlags,
											 const TQRect &r,
											 const TQStyleOption& opt,
											 const TQWidget *widget ) const
{
	switch (control)
	{
		// COMBOBOX & TOOLBUTTON MASKS
		// -------------------------------------------------------------------
		case CC_ComboBox:
		case CC_ToolButton: {
			int x1, y1, x2, y2;
			r.coords( &x1, &y1, &x2, &y2 );
			TQCOORD corners[] = { x1,y1, x2,y1, x1,y2, x2,y2 };
			p->fillRect( r, color1 );
			p->setPen( color0 );
			p->drawPoints( TQPointArray(4, corners) );
			break;
		}

		default:
			TDEStyle::drawComplexControlMask(control, p, ceData, elementFlags, r, opt, widget);
	}
}


void HighColorStyle::drawItem( TQPainter *p,
                               const TQRect &r,
                               int flags,
                               const TQColorGroup &cg,
                               bool enabled,
                               const TQPixmap *pixmap,
                               const TQString &text,
                               int len,
                               const TQColor *penColor ) const
{
	// We only reimplement this method this so we can override the
	// text color used for widgets when we draw them with the selection
	// color and Qt expects them to be drawn them with the button color.
	// -------------------------------------------------------------------
	const TQColor *col;

	if ( selectionBackground ) {
		col = &cg.highlightedText();
		selectionBackground = false;
	} else
		col = penColor;

	TDEStyle::drawItem( p, r, flags, cg, enabled, pixmap,
			text, len, col );
}


TQRect HighColorStyle::subRect(SubRect r, const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, const TQWidget *widget) const
{
	// We want the focus rect for buttons to be adjusted from
	// the Qt3 defaults to be similar to Qt 2's defaults.
	// -------------------------------------------------------------------
	if (r == SR_PushButtonFocusRect ) {
		TQRect wrect(ceData.rect);
		int dbw1 = 0, dbw2 = 0;

		if ((elementFlags & CEF_IsDefault) || (elementFlags & CEF_AutoDefault)) {
			dbw1 = pixelMetric(PM_ButtonDefaultIndicator, ceData, elementFlags, widget);
			dbw2 = dbw1 * 2;
		}

		int dfw1 = pixelMetric(PM_DefaultFrameWidth, ceData, elementFlags, widget) * 2,
			dfw2 = dfw1 * 2;

		return TQRect(wrect.x()      + dfw1 + dbw1 + 1,
					 wrect.y()      + dfw1 + dbw1 + 1,
					 wrect.width()  - dfw2 - dbw2 - 1,
					 wrect.height() - dfw2 - dbw2 - 1);
	} else
		return TDEStyle::subRect(r, ceData, elementFlags, widget);
}


int HighColorStyle::pixelMetric(PixelMetric m, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQWidget *widget) const
{
	switch(m)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PM_ButtonMargin:				// Space btw. frame and label
			return 4;

		case PM_ButtonDefaultIndicator: {
			if ( type == HighColor )
				return 0;					// No indicator when highcolor
			else
				return 3;
		}

		case PM_MenuButtonIndicator: {		// Arrow width
			if ( type != B3 )
				return 8;
			else
				return 7;
		}
										
		// CHECKBOXES / RADIO BUTTONS
		// -------------------------------------------------------------------
		case PM_ExclusiveIndicatorWidth:	// Radiobutton size
		case PM_ExclusiveIndicatorHeight:
		case PM_IndicatorWidth:				// Checkbox size
		case PM_IndicatorHeight: {
			return 13;						// 13x13
		}

		case PM_MenuIndicatorFrameHBorder:
		case PM_MenuIndicatorFrameVBorder:
		case PM_MenuIconIndicatorFrameHBorder:
		case PM_MenuIconIndicatorFrameVBorder:
			return 0;

		default:
			return TDEStyle::pixelMetric(m, ceData, elementFlags, widget);
	}
}

/*! \reimp */
int HighColorStyle::styleHint(StyleHint sh, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQStyleOption &opt, TQStyleHintReturn *returnData, const TQWidget *w) const
{
	int ret;

	switch (sh) {
		case SH_MenuIndicatorColumnWidth:
			{
				int  checkcol = opt.maxIconWidth();
				bool checkable = (elementFlags & CEF_IsCheckable);

				if ( checkable )
					checkcol = TQMAX( checkcol, 20 );
			
				ret = checkcol;
			}
			break;
		default:
			ret = TDEStyle::styleHint(sh, ceData, elementFlags, opt, returnData, w);
			break;
	}

	return ret;
}

TQSize HighColorStyle::sizeFromContents( ContentsType contents,
										const TQStyleControlElementData &ceData,
										ControlElementFlags elementFlags,
										const TQSize &contentSize,
										const TQStyleOption& opt,
										const TQWidget* widget ) const
{
	switch (contents)
	{
		// PUSHBUTTON SIZE
		// ------------------------------------------------------------------
		case CT_PushButton: {
			int w  = contentSize.width();
			int h  = contentSize.height();
			int bm = pixelMetric( PM_ButtonMargin, ceData, elementFlags, widget );
			int fw = pixelMetric( PM_DefaultFrameWidth, ceData, elementFlags, widget ) * 2;

			w += bm + fw + 6;	// ### Add 6 to make way for bold font.
			h += bm + fw;

			// Ensure we stick to standard width and heights.
			if ((elementFlags & CEF_IsDefault) || (elementFlags & CEF_AutoDefault)) {
				if ( w < 80 && !ceData.text.isEmpty() )
					w = 80;

				if ( type != HighColor ) {
					// Compensate for default indicator
					int di = pixelMetric( PM_ButtonDefaultIndicator, ceData, elementFlags );
					w += di * 2;
					h += di * 2;
				}
			}

			if ( h < 22 )
				h = 22;

			return TQSize( w, h );
		}


		// POPUPMENU ITEM SIZE
		// -----------------------------------------------------------------
		case CT_PopupMenuItem: {
			if ( ! widget || opt.isDefault() )
				return contentSize;

			const TQPopupMenu *popup = (const TQPopupMenu *) widget;
			bool checkable = popup->isCheckable();
			TQMenuItem *mi = opt.menuItem();
			int maxpmw = opt.maxIconWidth();
			int w = contentSize.width(), h = contentSize.height();

			if ( mi->custom() ) {
				w = mi->custom()->sizeHint().width();
				h = mi->custom()->sizeHint().height();
				if ( ! mi->custom()->fullSpan() )
					h += 2*itemVMargin + 2*itemFrame;
			}
			else if ( mi->widget() ) {
			} else if ( mi->isSeparator() ) {
				w = 10; // Arbitrary
				h = 2;
			}
			else {
				if ( mi->pixmap() )
					h = TQMAX( h, mi->pixmap()->height() + 2*itemFrame );
				else {
					// Ensure that the minimum height for text-only menu items
					// is the same as the icon size used by KDE.
					h = TQMAX( h, 16 + 2*itemFrame );
					h = TQMAX( h, popup->fontMetrics().height()
							+ 2*itemVMargin + 2*itemFrame );
				}
					
				if ( mi->iconSet() )
					h = TQMAX( h, mi->iconSet()->pixmap(
								TQIconSet::Small, TQIconSet::Normal).height() +
								2 * itemFrame );
			}

			if ( ! mi->text().isNull() && mi->text().find('\t') >= 0 )
				w += 12;
			else if ( mi->popup() )
				w += 2 * arrowHMargin;

			if ( maxpmw )
				w += maxpmw + 6;
			if ( checkable && maxpmw < 20 )
				w += 20 - maxpmw;
			if ( checkable || maxpmw > 0 )
				w += 12;

			w += rightBorder;

			return TQSize( w, h );
		}


		default:
			return TDEStyle::sizeFromContents( contents, ceData, elementFlags, contentSize, opt, widget );
	}
}


// Fix Qt's wacky image alignment
TQPixmap HighColorStyle::stylePixmap(StylePixmap stylepixmap,
									const TQStyleControlElementData &ceData,
									ControlElementFlags elementFlags,
									const TQStyleOption& opt,
									const TQWidget* widget) const
{
    switch (stylepixmap) {
		case SP_TitleBarMinButton:
			return TQPixmap((const char **)hc_minimize_xpm);
		case SP_TitleBarCloseButton:
			return TQPixmap((const char **)hc_close_xpm);
		default:
			break;
	}

	return TDEStyle::stylePixmap(stylepixmap, ceData, elementFlags, opt, widget);
}


bool HighColorStyle::objectEventHandler( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void* source, TQEvent *event )
{
	if (TDEStyle::objectEventHandler( ceData, elementFlags, source, event ))
		return true;

	TQToolBar* toolbar;

	if (ceData.widgetObjectTypes.contains("TQObject"))
	{
		TQObject* object = reinterpret_cast<TQObject*>(source);

		if ( object->parent() && !qstrcmp( object->name(), kdeToolbarWidget ) )
		{
			// Draw a gradient background for custom widgets in the toolbar
			// that have specified a "tde toolbar widget" name.
			// FIXME
			// This currently requires direct widget access
			// Is there any way to do this without it?
	
			if (event->type() == TQEvent::Paint ) {

				// Find the top-level toolbar of this widget, since it may be nested in other
				// widgets that are on the toolbar.
				TQWidget *widget = static_cast<TQWidget*>(object);
				TQWidget *parent = static_cast<TQWidget*>(object->parent());
				int x_offset = ceData.rect.x(), y_offset = ceData.rect.y();
				while (parent && parent->parent() && !qstrcmp( parent->name(), kdeToolbarWidget ) )
				{
					x_offset += parent->x();
					y_offset += parent->y();
					parent = static_cast<TQWidget*>(parent->parent());
				}
	
				TQRect r  = ceData.rect;
				TQRect pr = ceData.parentWidgetData.rect;
				bool horiz_grad = pr.width() < pr.height();
	
				// Check if the parent is a QToolbar, and use its orientation, else guess.
				TQToolBar* tb = dynamic_cast<TQToolBar*>(parent);
				if (tb) horiz_grad = tb->orientation() == TQt::Vertical;
	
				TQPainter p( widget );
				renderGradient(&p, r, parent->colorGroup().button(), horiz_grad,
						x_offset, y_offset, pr.width(), pr.height());
	
				return false;	// Now draw the contents
			}
		} else if ( object->parent() &&
			(toolbar = dynamic_cast<TQToolBar*>(object->parent())) )
		{
			// We need to override the paint event to draw a 
			// gradient on a QToolBarExtensionWidget.
			// FIXME
			// This currently requires direct widget access
			// Is there any way to do this without it?
	
			if ( event->type() == TQEvent::Paint ) {

				TQWidget *widget = static_cast<TQWidget*>(object);
				TQRect wr = ceData.rect, tr = ceData.parentWidgetData.rect;
				TQPainter p( widget );
				renderGradient(&p, wr, toolbar->colorGroup().button(),
						toolbar->orientation() == TQt::Vertical,
						wr.x(), wr.y(), tr.width() - 2, tr.height() - 2);
				
				p.setPen( toolbar->colorGroup().dark() );
				if ( toolbar->orientation() == TQt::Horizontal )
					p.drawLine( wr.width()-1, 0, wr.width()-1, wr.height()-1 );
				else
					p.drawLine( 0, wr.height()-1, wr.width()-1, wr.height()-1 );
				
				return true;
			}
		}
	}
	
	return false;
}


void HighColorStyle::renderGradient( TQPainter* p, const TQRect& r,
	TQColor clr, bool horizontal, int px, int py, int pwidth, int pheight) const
{
	// Make 8 bit displays happy
	if (!highcolor) {
		p->fillRect(r, clr);
		return;
	}

	// px, py specify the gradient pixmap offset relative to the top-left corner.
	// pwidth, pheight specify the width and height of the parent's pixmap.
	// We use these to draw parent-relative pixmaps for toolbar buttons
	// and menubar items.

	GradientSet* grSet = gDict.find( clr.rgb() );

	if (!grSet) {
		grSet = new GradientSet(clr);
		gDict.insert( clr.rgb(), grSet );
	}

	if (horizontal) {
		int width = (pwidth != -1) ? pwidth : r.width();

		if (width <= 34)
			p->drawTiledPixmap(r, *grSet->gradient(HMed), TQPoint(px, 0));
		else if (width <= 52)
			p->drawTiledPixmap(r, *grSet->gradient(HLarge), TQPoint(px, 0));
		else {
			KPixmap *hLarge = grSet->gradient(HLarge);

			// Don't draw a gradient if we don't need to
			if (hLarge->width() > px)
			{
				int pixmapWidth = hLarge->width() - px;

				// Draw the gradient
				p->drawTiledPixmap( r.x(), r.y(), pixmapWidth, r.height(),
									*hLarge, px, 0 );
				// Draw the remaining fill
				p->fillRect(r.x()+pixmapWidth, r.y(), r.width()-pixmapWidth, 
						r.height(),	clr.dark(110));

			} else
				p->fillRect(r, clr.dark(110));
		}

	} else {
		// Vertical gradient
		// -----------------
		int height = (pheight != -1) ? pheight : r.height();

		if (height <= 24)
			p->drawTiledPixmap(r, *grSet->gradient(VSmall), TQPoint(0, py));
		else if (height <= 34)
			p->drawTiledPixmap(r, *grSet->gradient(VMed), TQPoint(0, py));
		else if (height <= 64)
			p->drawTiledPixmap(r, *grSet->gradient(VLarge), TQPoint(0, py));
		else {
			KPixmap *vLarge = grSet->gradient(VLarge);

			// Only draw the upper gradient if we need to.
			if (vLarge->height() > py)
			{
				int pixmapHeight = vLarge->height() - py;

				// Draw the gradient
				p->drawTiledPixmap( r.x(), r.y(), r.width(), pixmapHeight,
									*vLarge, 0, py );
				// Draw the remaining fill
				p->fillRect(r.x(), r.y()+pixmapHeight, r.width(), 
						r.height()-pixmapHeight, clr.dark(110));

			} else
				p->fillRect(r, clr.dark(110));
		}
	}
}
