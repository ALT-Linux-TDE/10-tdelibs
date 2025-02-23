/* Keramik Style for KDE3
   Copyright (c) 2002 Malte Starostik <malte@kde.org>
             (c) 2002,2003 Maksim Orlovich <mo002j@mail.rochester.edu>

   based on the KDE3 HighColor Style

   Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
             (C) 2001-2002 Fredrik H�glund  <fredrik@kde.org>

   Drawing routines adapted from the KDE2 HCStyle,
   Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
             (C) 2000 Dirk Mueller          <mueller@kde.org>
             (C) 2001 Martijn Klingens      <klingens@kde.org>

   Progressbar code based on TDEStyle, Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>,
   Improvements to progressbar animation from Plastik, Copyright (C) 2003 Sandro Giessl <sandro@giessl.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// $Id$

#include <tqapplication.h>
#include <tqbitmap.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqdrawutil.h>
#include <tqframe.h>
#include <tqheader.h>
#include <tqintdict.h>
#include <tqlineedit.h>
#include <tqlistbox.h>
#include <tqmenubar.h>
#include <tqpainter.h>
#include <tqpointarray.h>
#include <tqprogressbar.h>
#include <tqpushbutton.h>
#include <tqsettings.h>
#include <tqslider.h>
#include <tqstyleplugin.h>
#include <tqtabbar.h>
#include <tqtimer.h>
#include <tqtoolbar.h>
#include <tqtoolbutton.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>

#include "keramik.moc"

#include "gradients.h"
#include "colorutil.h"
#include "keramikrc.h"
#include "keramikimage.h"

#include "bitmaps.h"
#include "pixmaploader.h"

#define loader Keramik::PixmapLoader::the()

// -- Style Plugin Interface -------------------------
class KeramikStylePlugin : public TQStylePlugin
{
public:
	KeramikStylePlugin() {}
	~KeramikStylePlugin() {}

	TQStringList keys() const
	{
		if (TQPixmap::defaultDepth() > 8)
			return TQStringList() << "Keramik";
		else
			return TQStringList();
	}

	TQStyle* create( const TQString& key )
	{
		if ( key == "keramik" ) return new KeramikStyle();
		return 0;
	}
};

TDE_EXPORT_PLUGIN( KeramikStylePlugin )
// ---------------------------------------------------


// ### Remove globals
/*
TQBitmap lightBmp;
TQBitmap grayBmp;
TQBitmap dgrayBmp;
TQBitmap centerBmp;
TQBitmap maskBmp;
TQBitmap xBmp;
*/
namespace
{
	const int itemFrame       = 2;
	const int itemHMargin     = 6;
	const int itemVMargin     = 0;
	const int arrowHMargin    = 6;
	const int rightBorder     = 12;
	const char* kdeToolbarWidget = "tde toolbar widget";

	const int smallButMaxW    = 27;
	const int smallButMaxH    = 20;
	const int titleBarH       = 22;
}
// ---------------------------------------------------------------------------

namespace
{
	void drawKeramikArrow(TQPainter* p, TQColorGroup cg, TQRect r, TQStyle::PrimitiveElement pe, bool down, bool enabled)
	{
		TQPointArray a;

		switch(pe)
		 {
			case TQStyle::PE_ArrowUp:
				a.setPoints(TQCOORDARRLEN(keramik_up_arrow), keramik_up_arrow);
				break;

			case TQStyle::PE_ArrowDown:
				a.setPoints(TQCOORDARRLEN(keramik_down_arrow), keramik_down_arrow);
				break;

			case TQStyle::PE_ArrowLeft:
				a.setPoints(TQCOORDARRLEN(keramik_left_arrow), keramik_left_arrow);
				break;

			default:
				a.setPoints(TQCOORDARRLEN(keramik_right_arrow), keramik_right_arrow);
		}

		p->save();
		/*if ( down )
			p->translate( pixelMetric( PM_ButtonShiftHorizontal, ceData, elementFlags ),
						pixelMetric( PM_ButtonShiftVertical, ceData, elementFlags ) );
		*/

		if ( enabled ) {
			//CHECKME: Why is the -1 needed?
			a.translate( r.x() + r.width() / 2 - 1, r.y() + r.height() / 2 );

			if (!down)
				p->setPen( cg.buttonText() );
			else
				p->setPen ( cg.button() );
			p->drawLineSegments( a );
		} else {
			a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 + 1 );
			p->setPen( cg.light() );
			p->drawLineSegments( a );
			a.translate( -1, -1 );
			p->setPen( cg.mid() );
			p->drawLineSegments( a );
		}
		p->restore();
	}
}

// XXX
/* reimp. */
void KeramikStyle::renderMenuBlendPixmap( KPixmap& pix, const TQColorGroup &cg,
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

// XXX
TQRect KeramikStyle::subRect(SubRect r, const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, const TQWidget *widget) const
{
	// We want the focus rect for buttons to be adjusted from
	// the Qt3 defaults to be similar to Qt 2's defaults.
	// -------------------------------------------------------------------
	switch ( r )
	{
		case SR_PushButtonFocusRect:
		{
			TQRect wrect(ceData.rect);

			if ((elementFlags & CEF_IsDefault) || (elementFlags & CEF_AutoDefault))
			{
				return TQRect(wrect.x() + 6, wrect.y() + 5, wrect.width() - 12,  wrect.height() - 10);
			}
			else
			{
				return TQRect(wrect.x() + 3, wrect.y() + 5, wrect.width() - 8,  wrect.height() - 10);
			}

			break;
		}

		case SR_ComboBoxFocusRect:
		{
			return querySubControlMetrics( CC_ComboBox, ceData, elementFlags, SC_ComboBoxEditField, TQStyleOption::Default, widget );
		}

		case SR_CheckBoxFocusRect:
		{
			//Only checkbox, no label
			if (ceData.text.isEmpty() && (ceData.fgPixmap.isNull()) )
			{
				TQRect bounding = ceData.rect;
				TQSize checkDim = loader.size( keramik_checkbox_on);
				int   cw = checkDim.width();
				int   ch = checkDim.height();

				TQRect checkbox(bounding.x() + 1, bounding.y() + 1 + (bounding.height() - ch)/2,
								cw - 3, ch - 4);

				return checkbox;
			}

			//Fallthrough intentional
		}

		default:
			return TDEStyle::subRect( r, ceData, elementFlags, widget );
	}
}


TQPixmap KeramikStyle::stylePixmap(StylePixmap stylepixmap,
									const TQStyleControlElementData &ceData,
									ControlElementFlags elementFlags,
									const TQStyleOption& opt,
									const TQWidget* widget) const
{
    switch (stylepixmap) {
		case SP_TitleBarMinButton:
			return Keramik::PixmapLoader::the().pixmap(keramik_title_iconify,
				TQt::black, TQt::black, false, false);
			//return qpixmap_from_bits( iconify_bits, "title-iconify.png" );
		case SP_TitleBarMaxButton:
			return Keramik::PixmapLoader::the().pixmap(keramik_title_maximize,
				TQt::black, TQt::black, false, false);
		case SP_TitleBarCloseButton:
			if (widget && widget->inherits("KDockWidgetHeader"))
				return Keramik::PixmapLoader::the().pixmap(keramik_title_close_tiny,
				TQt::black, TQt::black, false, false);
			else	return Keramik::PixmapLoader::the().pixmap(keramik_title_close,
				TQt::black, TQt::black, false, false);
		case SP_TitleBarNormalButton:
			return Keramik::PixmapLoader::the().pixmap(keramik_title_restore,
				TQt::black, TQt::black, false, false);
		default:
			break;
	}

	return TDEStyle::stylePixmap(stylepixmap, ceData, elementFlags, opt, widget);
}




KeramikStyle::KeramikStyle()
	:TDEStyle( AllowMenuTransparency | FilledFrameWorkaround, ThreeButtonScrollBar ),
		maskMode(false), formMode(false),
		toolbarBlendWidget(0), titleBarMode(None), flatMode(false), customScrollMode(false), kickerMode(false)
{
	forceSmallMode = false;

	TQSettings settings;

	highlightScrollBar = settings.readBoolEntry("/keramik/Settings/highlightScrollBar", true);
	animateProgressBar = settings.readBoolEntry("/keramik/Settings/animateProgressBar", false);

	if (animateProgressBar)
	{
		animationTimer = new TQTimer( this );
		connect( animationTimer, TQ_SIGNAL(timeout()), this, TQ_SLOT(updateProgressPos()) );
	}
	
	firstComboPopupRelease = false;
}

void KeramikStyle::updateProgressPos()
{
	//Update the registered progressbars.
	TQMap<TQProgressBar*, int>::iterator iter;
	bool visible = false;
	for (iter = progAnimWidgets.begin(); iter != progAnimWidgets.end(); ++iter)
	{
		TQProgressBar* pbar = iter.key(); 
		if (pbar->isVisible() && pbar->isEnabled() &&
			pbar->progress() != pbar->totalSteps())
		{
			++iter.data();
			if (iter.data() == 28)
				iter.data() = 0;
			iter.key()->update();
		}
		if (iter.key()->isVisible())
			visible = true;
		
	}
	if (!visible)
		animationTimer->stop();
}

KeramikStyle::~KeramikStyle()
{
	Keramik::PixmapLoader::release();
	Keramik::GradientPainter::releaseCache();
	KeramikDbCleanup();
}

void KeramikStyle::applicationPolish(const TQStyleControlElementData &ceData, ControlElementFlags, void *ptr)
{
    if (ceData.widgetObjectTypes.contains("TQApplication")) {
		TQApplication *app = reinterpret_cast<TQApplication*>(ptr);
		if (!qstrcmp(app->argv()[0], "kicker")) {
			kickerMode = true;
		}
	}
}

void KeramikStyle::polish(const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr)
{
	if (ceData.widgetObjectTypes.contains("TQWidget")) {
		TQWidget *widget = reinterpret_cast<TQWidget*>(ptr);

		// Put in order of highest occurrence to maximise hit rate
		if ( widget->inherits( "TQPushButton" )  || widget->inherits( "TQComboBox" ) || widget->inherits("TQToolButton") )
		{
			installObjectEventHandler(ceData, elementFlags, ptr, this);
			if ( widget->inherits( "TQComboBox" ) )
				widget->setBackgroundMode( NoBackground );
		}
		else if ( widget->inherits( "TQMenuBar" ) || widget->inherits( "TQPopupMenu" ) )
			widget->setBackgroundMode( NoBackground );
	
		else if ( widget->parentWidget() &&
				( ( widget->inherits( "TQListBox" ) && widget->parentWidget()->inherits( "TQComboBox" ) ) ||
					widget->inherits( "TDECompletionBox" ) ) ) {
			TQListBox* listbox = (TQListBox*) widget;
			listbox->setLineWidth( 4 );
			listbox->setBackgroundMode( NoBackground );
			installObjectEventHandler(ceData, elementFlags, ptr, this);
	
		} else if (widget->inherits("TQToolBarExtensionWidget")) {
			installObjectEventHandler(ceData, elementFlags, ptr, this);
			//widget->setBackgroundMode( NoBackground );
		}
		else if ( !qstrcmp( widget->name(), kdeToolbarWidget ) ) {
			widget->setBackgroundMode( NoBackground );
			installObjectEventHandler(ceData, elementFlags, ptr, this);
		}
	
		if (animateProgressBar && ::tqt_cast<TQProgressBar*>(widget))
		{
			installObjectEventHandler(ceData, elementFlags, ptr, this);
			progAnimWidgets[static_cast<TQProgressBar*>(widget)] = 0;
			connect(widget, TQ_SIGNAL(destroyed(TQObject*)), this, TQ_SLOT(progressBarDestroyed(TQObject*)));
			if (!animationTimer->isActive())
				animationTimer->start( 50, false );
		}
	}

	TDEStyle::polish(ceData, elementFlags, ptr);
}

void KeramikStyle::unPolish(const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr)
{
	if (ceData.widgetObjectTypes.contains("TQWidget")) {
		TQWidget *widget = reinterpret_cast<TQWidget*>(ptr);

		//### TODO: This needs major cleanup (and so does polish() )
		if ( widget->inherits( "TQPushButton" ) || widget->inherits( "TQComboBox"  ) )
		{
			if ( widget->inherits( "TQComboBox" ) )
				widget->setBackgroundMode( PaletteButton );
			removeObjectEventHandler(ceData, elementFlags, ptr, this);
		}
		else if ( widget->inherits( "TQMenuBar" ) || widget->inherits( "TQPopupMenu" ) )
			widget->setBackgroundMode( PaletteBackground );
	
		else if ( widget->parentWidget() &&
				( ( widget->inherits( "TQListBox" ) && widget->parentWidget()->inherits( "TQComboBox" ) ) ||
					widget->inherits( "TDECompletionBox" ) ) ) {
			TQListBox* listbox = (TQListBox*) widget;
			listbox->setLineWidth( 1 );
			listbox->setBackgroundMode( PaletteBackground );
			removeObjectEventHandler(ceData, elementFlags, ptr, this);
			widget->clearMask();
		} else if (widget->inherits("TQToolBarExtensionWidget")) {
			removeObjectEventHandler(ceData, elementFlags, ptr, this);
		}
		else if ( !qstrcmp( widget->name(), kdeToolbarWidget ) ) {
			widget->setBackgroundMode( PaletteBackground );
			removeObjectEventHandler(ceData, elementFlags, ptr, this);
		}
		else if ( ::tqt_cast<TQProgressBar*>(widget) )
		{
			progAnimWidgets.remove(static_cast<TQProgressBar*>(widget));
		}
	}

	TDEStyle::unPolish(ceData, elementFlags, ptr);
}

void KeramikStyle::progressBarDestroyed(TQObject* obj)
{
	progAnimWidgets.remove(static_cast<TQProgressBar*>(obj));
}


void KeramikStyle::polish( TQPalette& )
{
	loader.clear();
}

/** 
 Draws gradient background for toolbar buttons, handles and spacers
*/
static void renderToolbarEntryBackground(TQPainter* paint,
		 const TQToolBar* parent, TQRect r, const TQColorGroup& cg, bool horiz)
{
	int  toolWidth, toolHeight;
	
	//Do we have a parent toolbar to use?
	if (parent)
	{
		//Calculate the toolbar geometry.
		//The initial guess is the size of the parent widget
		toolWidth  = parent->width();
		toolHeight = parent->height();
					
		//If we're floating, however, wee need to fiddle
		//with height to skip the titlebar
		if (parent->place() == TQDockWindow::OutsideDock)
		{
			toolHeight = toolHeight - titleBarH - 2*parent->frameWidth() + 2;
			//2 at the end = the 2 pixels of border of a "regular"
			//toolbar we normally paint over.
		}
	}
	else
	{
		//No info, make a guess.
		//We take the advantage of the fact that the non-major
		//sizing direction parameter is ignored
		toolWidth  = r.width () + 2;
		toolHeight = r.height() + 2;
	}

	//Calculate where inside the toolbar we're
	int xoff = 0, yoff = 0;
	if (horiz)
		yoff = (toolHeight - r.height())/2;
	else
		xoff = (toolWidth - r.width())/2;
						
	Keramik::GradientPainter::renderGradient( paint, r, cg.button(),
		horiz, false /*Not a menubar*/,
		xoff, yoff,
		toolWidth, toolHeight);
}

static void renderToolbarWidgetBackground(TQPainter* painter, const TQStyleControlElementData &ceData, const TQStyle::ControlElementFlags elementFlags, const TQWidget* widget)
{
	// Draw a gradient background for custom widgets in the toolbar
	// that have specified a "tde toolbar widget" name, or
	// are caught as toolbar widgets otherwise

	// Find the top-level toolbar of this widget, since it may be nested in other
	// widgets that are on the toolbar.
	TQWidget *parent = (widget)?widget->parentWidget():NULL;
	int x_offset = ceData.rect.x(), y_offset = ceData.rect.y();
	while (parent && parent->parent() && !qstrcmp( parent->name(), kdeToolbarWidget ) )
	{
		x_offset += parent->x();
		y_offset += parent->y();
		parent = static_cast<TQWidget*>(parent->parent());
	}

	TQRect pr = ceData.parentWidgetData.rect;
	bool horiz_grad = pr.width() > pr.height();

	int  toolHeight = ceData.parentWidgetData.rect.height();
	int  toolWidth  = ceData.parentWidgetData.rect.width ();

	// Check if the parent is a QToolbar, and use its orientation, else guess.
	//Also, get the height to use in the gradient from it.
	TQToolBar* tb = dynamic_cast<TQToolBar*>(parent);
	if (tb)
	{
		horiz_grad = ceData.orientation == TQt::Horizontal;

		//If floating, we need to skip the titlebar.
		if (tb->place() == TQDockWindow::OutsideDock)
		{
			toolHeight = tb->height() - titleBarH - 2*tb->frameWidth() + 2;
			//Calculate offset here by looking at the bottom edge difference, and height.
			//First, calculate by how much the widget needs to be extended to touch
			//the bottom edge, minus the frame (except we use the current y_offset
			// to map to parent coordinates)
			int needToTouchBottom = tb->height() - tb->frameWidth() -
									(ceData.rect.bottom() + y_offset);

			//Now, with that, we can see which portion to skip in the full-height
			//gradient -- which is the stuff other than the extended
			//widget
			y_offset = toolHeight - (ceData.rect.height() + needToTouchBottom) - 1;
		}
	}

	if (painter)
	{
		Keramik::GradientPainter::renderGradient( painter, ceData.rect,
			 ceData.colorGroup.button(), horiz_grad, false,
			 x_offset, y_offset, toolWidth, toolHeight);
	}
	else
	{
		TQPainter p( widget );
		Keramik::GradientPainter::renderGradient( &p, ceData.rect,
			 ceData.colorGroup.button(), horiz_grad, false,
			 x_offset, y_offset, toolWidth, toolHeight);
	}
}

// This function draws primitive elements as well as their masks.
void KeramikStyle::drawPrimitive( PrimitiveElement pe,
									TQPainter *p,
									const TQStyleControlElementData &ceData,
									ControlElementFlags elementFlags,
									const TQRect &r,
									const TQColorGroup &cg,
									SFlags flags,
									const TQStyleOption& opt ) const
{
	bool down     = flags & Style_Down;
	bool on       = flags & Style_On;
	bool active   = flags & Style_Active;
	bool disabled = ( flags & Style_Enabled ) == 0;
	int  x, y, w, h;
	r.rect(&x, &y, &w, &h);

	int x2 = x+w-1;
	int y2 = y+h-1;

	switch(pe)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PE_ButtonDefault:
		{
			bool sunken = on || down;

			int id;
			if ( sunken ) id  = keramik_pushbutton_default_pressed;
				else id = keramik_pushbutton_default;

			if (flags & Style_MouseOver && id == keramik_pushbutton_default )
				id = keramik_pushbutton_default_hov;


			//p->fillRect( r, cg.background() );
			Keramik::RectTilePainter( id, false ).draw(p, r, cg.button(), cg.background(), disabled,  pmode() );
			break;
		}

		case PE_ButtonDropDown:
		case PE_ButtonTool:
		{
			if (titleBarMode)
			{
				TQRect nr;
				if (titleBarMode == Maximized)
				{
					//### What should we draw at sides?
					nr = TQRect(r.x(), r.y(), r.width()-1, r.height() );
				}
				else
				{
					int offX = (r.width() - 15)/2;
					int offY = (r.height() - 15)/2;

					if (flags & Style_Down)
						offY += 1;

					nr = TQRect(r.x()+offX, r.y()+offY, 15, 15);
				}

				Keramik::ScaledPainter(flags & Style_Down ? keramik_titlebutton_pressed : keramik_titlebutton,
										Keramik::ScaledPainter::Both).draw( p, nr, cg.button(), cg.background());
				return;
			}

			if (on)
			{
				Keramik::RectTilePainter(keramik_toolbar_clk).draw(p, r, cg.button(), cg.background());
				p->setPen(cg.dark());
				p->drawLine(x, y, x2, y);
				p->drawLine(x, y, x, y2);
			}
			else if (down)
			{
				Keramik::RectTilePainter(keramik_toolbar_clk).draw(p, r, cg.button(), cg.background());
			}
			else {
				if (flags & Style_MouseOver)
				{
				Keramik::GradientPainter::renderGradient( p,
					TQRect(r.x(), 0, r.width(), r.height()),
					Keramik::ColorUtil::lighten(cg.button(), 115), flags & Style_Horizontal, false );
				}
				else
					Keramik::GradientPainter::renderGradient( p,
						TQRect(r.x(), 0, r.width(), r.height()),
						cg.button(), flags & Style_Horizontal, false );

				p->setPen(cg.button().light(70));
				p->drawLine(x, y, x2-1, y);
				p->drawLine(x, y, x, y2-1);
				p->drawLine(x+2, y2-1, x2-1, y2-1);
				p->drawLine(x2-1, y+2, x2-1, y2-2);

				p->setPen(Keramik::ColorUtil::lighten(cg.button(), 115) );
				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+1, x+1, y2);
				p->drawLine(x, y2, x2, y2);
				p->drawLine(x2, y, x2, y2);
			}

			break;
		}

		// PUSH BUTTON
		// -------------------------------------------------------------------
		case PE_ButtonCommand:
		{
			bool sunken = on || down;

			int  name;

			if ( w <= smallButMaxW || h <= smallButMaxH || forceSmallMode)
			{
				if (sunken)
					name = keramik_pushbutton_small_pressed;
				else
					name =  keramik_pushbutton_small;
				forceSmallMode = false;
			}
			else
			{
				if (sunken)
					name = keramik_pushbutton_pressed;
				else
					name =  keramik_pushbutton;
			}

			if (flags & Style_MouseOver && name == keramik_pushbutton )
				name = keramik_pushbutton_hov;

			if (toolbarBlendWidget && !flatMode )
			{
				//Render the toolbar gradient.
				renderToolbarWidgetBackground(p, ceData, elementFlags, toolbarBlendWidget);

				//Draw and blend the actual bevel..
				Keramik::RectTilePainter( name, false ).draw(p, r, cg.button(), cg.background(),
					disabled, Keramik::TilePainter::PaintFullBlend  );
			}
			else if (!flatMode)
				Keramik::RectTilePainter( name, false ).draw(p, r, cg.button(), cg.background(), disabled, pmode() );
			else {
				Keramik::ScaledPainter( name + KeramikTileCC, Keramik::ScaledPainter::Vertical).draw(
					p, r, cg.button(), cg.background(), disabled, pmode() );

				p->setPen(cg.button().light(75));

				p->drawLine(x, y, x2, y);
				p->drawLine(x, y, x, y2);
				p->drawLine(x, y2, x2, y2);
				p->drawLine(x2, y, x2, y2);
				flatMode = false;
			}

			break;

		}

		// BEVELS
		// -------------------------------------------------------------------
		case PE_ButtonBevel:
		{
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

			if (w > 4 && h > 4)
			{
				if (sunken)
					p->fillRect(x+2, y+2, w-4, h-4, cg.button());
				else
					Keramik::GradientPainter::renderGradient( p, TQRect(x+2, y+2, w-4, h-4),
							cg.button(), flags & Style_Horizontal );
			}
			break;
		}


			// FOCUS RECT
			// -------------------------------------------------------------------
		case PE_FocusRect:
			//Qt may pass the background color to use for the focus indicator
			//here. This particularly happens within the iconview.
			//If that happens, pass it on to drawWinFocusRect() so it can
			//honor it
			if ( opt.isDefault() )
				p->drawWinFocusRect( r );
			else
				p->drawWinFocusRect( r, opt.color() );
			break;

			// HEADER SECTION
			// -------------------------------------------------------------------
		case PE_HeaderSectionMenu:
		case PE_HeaderSection:
			if ( flags & Style_Down )
				Keramik::RectTilePainter( keramik_listview_pressed, false ).draw( p, r, cg.button(), cg.background() );
			else
				Keramik::RectTilePainter( keramik_listview, false ).draw( p, r, cg.button(), cg.background() );
			break;

		case PE_HeaderArrow:
			if ( flags & Style_Up )
				drawPrimitive( PE_ArrowUp, p, ceData, elementFlags, r, cg, Style_Enabled );
			else drawPrimitive( PE_ArrowDown, p, ceData, elementFlags, r, cg, Style_Enabled );
			break;


		// 	// SCROLLBAR
		// -------------------------------------------------------------------

		case PE_ScrollBarSlider:
		{
			bool horizontal = flags & Style_Horizontal;
			bool active = ( flags & Style_Active ) || ( flags & Style_Down );
			int name = KeramikSlider1;
			unsigned int count = 3;



			if ( horizontal )
			{
				if ( w > ( loader.size( keramik_scrollbar_hbar+KeramikSlider1 ).width() +
				           loader.size( keramik_scrollbar_hbar+KeramikSlider4 ).width() +
				           loader.size( keramik_scrollbar_hbar+KeramikSlider3 ).width() + 2 ) )
					count = 5;
			}
			else if ( h > ( loader.size( keramik_scrollbar_vbar+KeramikSlider1 ).height() +
			                loader.size( keramik_scrollbar_vbar+KeramikSlider4 ).height() +
			                loader.size( keramik_scrollbar_vbar+KeramikSlider3 ).height() + 2 ) )
					count = 5;

			TQColor col = cg.highlight();

			if (customScrollMode || !highlightScrollBar)
				col = cg.button();

			if (!active)
				Keramik::ScrollBarPainter( name, count, horizontal ).draw( p, r, col, cg.background(), false, pmode() );
			else
				Keramik::ScrollBarPainter( name, count, horizontal ).draw( p, r, Keramik::ColorUtil::lighten(col ,110),
																					cg.background(), false, pmode() );
			break;
		}

		case PE_ScrollBarAddLine:
		{
			bool down = flags & Style_Down;

			if ( flags & Style_Horizontal )
			{
				Keramik::CenteredPainter painter(  keramik_scrollbar_hbar_arrow2 );
				painter.draw( p, r, down? cg.buttonText() : cg.button(), cg.background(), disabled, pmode() );

				p->setPen( cg.buttonText() );
				p->drawLine(r.x()+r.width()/2 - 1, r.y() + r.height()/2 - 3,
				            r.x()+r.width()/2 - 1, r.y() + r.height()/2 + 3);


				drawKeramikArrow(p, cg, TQRect(r.x(), r.y(), r.width()/2, r.height()), PE_ArrowLeft, down, !disabled);

				drawKeramikArrow(p, cg, TQRect(r.x()+r.width()/2, r.y(), r.width() - r.width()/2, r.height()),
									PE_ArrowRight, down, !disabled);
			}
			else
			{
				Keramik::CenteredPainter painter(  keramik_scrollbar_vbar_arrow2 );
				painter.draw( p, r, down? cg.buttonText() : cg.button(), cg.background(), disabled, pmode() );

				p->setPen( cg.buttonText() );
				p->drawLine(r.x()+r.width()/2 - 4, r.y()+r.height()/2,
				            r.x()+r.width()/2 + 2, r.y()+r.height()/2);


				drawKeramikArrow(p, cg, TQRect(r.x(), r.y(), r.width(), r.height()/2), PE_ArrowUp, down, !disabled);

				drawKeramikArrow(p, cg, TQRect(r.x(), r.y()+r.height()/2, r.width(), r.height() - r.height()/2),
									PE_ArrowDown, down, !disabled);
				//drawKeramikArrow(p, cg, r, PE_ArrowUp, down, !disabled);
			}


			break;
		}

		case PE_ScrollBarSubLine:
		{
			bool down = flags & Style_Down;

			if ( flags & Style_Horizontal )
			{
				Keramik::CenteredPainter painter(keramik_scrollbar_hbar_arrow1 );
				painter.draw( p, r, down? cg.buttonText() : cg.button(), cg.background(), disabled, pmode() );
				drawKeramikArrow(p, cg, r, PE_ArrowLeft, down, !disabled);

			}
			else
			{
				Keramik::CenteredPainter painter( keramik_scrollbar_vbar_arrow1 );
				painter.draw( p, r, down? cg.buttonText() : cg.button(), cg.background(), disabled, pmode() );
				drawKeramikArrow(p, cg, r, PE_ArrowUp, down, !disabled);
			}
			break;
		}
		
		// CHECKBOX (indicator)
		// -------------------------------------------------------------------
		case PE_Indicator:
		case PE_IndicatorMask:

			if (flags & Style_On)
				Keramik::ScaledPainter( keramik_checkbox_on ).draw( p, r, cg.button(), cg.background(), disabled, pmode() );
			else if (flags & Style_Off)
				Keramik::ScaledPainter( keramik_checkbox_off ).draw( p, r, cg.button(), cg.background(), disabled, pmode() );
			else
				Keramik::ScaledPainter( keramik_checkbox_tri ).draw( p, r, cg.button(), cg.background(), disabled, pmode() );

			break;

			// RADIOBUTTON (exclusive indicator)
			// -------------------------------------------------------------------
		case PE_ExclusiveIndicator:
		case PE_ExclusiveIndicatorMask:
		{

			Keramik::ScaledPainter( on ? keramik_radiobutton_on : keramik_radiobutton_off ).draw( p, r, cg.button(), cg.background(), disabled, pmode() );
			break;
		}

			// line edit frame
		case PE_PanelLineEdit:
		{
			if ( opt.isDefault() || opt.lineWidth() == 1 )
			{
				//1-pixel frames can not be simply clipped wider frames, as that would produce too little contrast on the lower border
				p->setPen( cg.dark() );
				p->drawLine( x, y, x + w - 1, y );
				p->drawLine( x, y, x, y + h - 1 );
				
				p->setPen( cg.light().dark( 110 ) );
				p->drawLine( x + w - 1, y, x + w - 1, y + h - 1 );
				p->drawLine( x, y + h - 1, x + w - 1, y + h - 1);
			}
			else
			{
				p->setPen( cg.dark() );
				p->drawLine( x, y, x + w - 1, y );
				p->drawLine( x, y, x, y + h - 1 );
				p->setPen( cg.mid() );
				p->drawLine( x + 1, y + 1, x + w - 1, y + 1 );
				p->drawLine( x + 1, y + 1, x + 1, y + h - 1 );
				p->setPen( cg.light() );
				p->drawLine( x + w - 1, y, x + w - 1, y + h - 1 );
				p->drawLine( x, y + h - 1, x + w - 1, y + h - 1);
				p->setPen( cg.light().dark( 110 ) );
				p->drawLine( x + w - 2, y + 1, x + w - 2, y + h - 2 );
				p->drawLine( x + 1, y + h - 2, x + w - 2, y + h - 2);
			}
			break;
		}

			// SPLITTER/DOCKWINDOW HANDLES
			// -------------------------------------------------------------------
		case PE_DockWindowResizeHandle:
		case PE_Splitter:
		{
			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;

			p->setPen(cg.dark());
			p->drawRect( r );
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


		//case PE_PanelPopup:
			//p->setPen (cg.light() );
			//p->setBrush( cg.background().light( 110 ) );
			//p->drawRect( r );

			//p->setPen( cg.shadow() );
			//p->drawRect( r.x()+1, r.y()+1, r.width()-2, r.height()-2);
			//p->fillRect( visualRect( TQRect( x + 1, y + 1, 23, h - 2 ), r ), cg.background().dark( 105 ) );
			//break;

			// GENERAL PANELS
			// -------------------------------------------------------------------
		case PE_Panel:
		{
			if (kickerMode)
			{
				if (p->device() && p->device()->devType() == TQInternal::Widget &&
											 TQCString(static_cast<TQWidget*>(p->device())->className()) == "FittsLawFrame" )
				{
					int x2 = x + r.width() - 1;
					int y2 = y + r.height() - 1;
					p->setPen(cg.dark());
					p->drawLine(x+1,y2,x2-1,y2);
					p->drawLine(x2,y+1,x2,y2);

					p->setPen( cg.light() );
					p->drawLine(x, y, x2, y);
					p->drawLine(x, y, x, y2);


					return;
				}
			}
			TDEStyle::drawPrimitive(pe, p, ceData, elementFlags, r, cg, flags, opt);
			break;
		}
		case PE_WindowFrame:
		{
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
		{
			Keramik::GradientPainter::renderGradient( p, r, cg.button(), true, true);
			//Keramik::ScaledPainter( keramik_menubar , Keramik::ScaledPainter::Vertical).draw( p, r, cg.button(), cg.background() );

			int x2 = r.x()+r.width()-1;
			int y2 = r.y()+r.height()-1;
			int lw = opt.isDefault() ? pixelMetric(PM_DefaultFrameWidth, ceData, elementFlags)
				: opt.lineWidth();
			if (lw)
			{
				p->setPen(cg.mid());
				p->drawLine(x2, y, x2, y2);
			}

			break;
		}

		case PE_PanelDockWindow:		// Toolbar
		{
			bool horiz = r.width() > r.height();

			//Here, we just draw the border.
			int x  = r.x();
			int y  = r.y();
			int x2 = r.x() + r.width()  - 1;
			int y2 = r.y() + r.height() - 1;
			int lw = opt.isDefault() ? pixelMetric(PM_DefaultFrameWidth, ceData, elementFlags)
				: opt.lineWidth();

			if (lw)
			{
				//Gradient border colors.
				//(Same as in gradients.cpp)
				TQColor gradTop = Keramik::ColorUtil::lighten(cg.button(),110);
				TQColor gradBot = Keramik::ColorUtil::lighten(cg.button(),109);
				if (horiz)
				{
					//Top line
					p->setPen(gradTop);
					p->drawLine(x, y, x2, y);

					//Bottom line
					p->setPen(gradBot);
					p->drawLine(x, y2, x2, y2);

					//Left line
					Keramik::GradientPainter::renderGradient(
						p, TQRect(r.x(), r.y(), 1, r.height()),
						cg.button(), true);

					//Right end-line
					p->setPen(cg.mid());
					p->drawLine(x2, y, x2, y2);
				}
				else
				{
					//Left line
					p->setPen(gradTop);
					p->drawLine(x, y, x, y2);

					//Right line
					p->setPen(gradBot);
					p->drawLine(x2, y, x2, y2);

					//Top line
					Keramik::GradientPainter::renderGradient(
						p, TQRect(r.x(), r.y(), r.width(), 1),
						cg.button(), false);

					//Bottom end-line
					p->setPen(cg.mid());
					p->drawLine(x, y2, x2, y2);
				}
			}
			break;
		}

		// TOOLBAR SEPARATOR
		// -------------------------------------------------------------------
		case PE_DockWindowSeparator:
		{
			TQWidget*  paintWidget = dynamic_cast<TQWidget*>(p->device());
			TQToolBar* parent      = 0;
			if (paintWidget)
				parent = ::tqt_cast<TQToolBar*>(paintWidget->parentWidget());

			renderToolbarEntryBackground(p, parent, r, cg, (flags & Style_Horizontal) );
			if ( !(flags & Style_Horizontal) )
			{
				p->setPen(cg.mid());
				p->drawLine(4, r.height()/2-1, r.width()-5, r.height()/2-1);
				p->setPen(cg.light());
				p->drawLine(4, r.height()/2, r.width()-5, r.height()/2);
			}
			else
			{
				p->setPen(cg.mid());
				p->drawLine(r.width()/2-1, 4, r.width()/2-1, r.height()-5);
				p->setPen(cg.light());
				p->drawLine(r.width()/2, 4, r.width()/2, r.height()-5);
			}
			break;
		}

		case PE_PanelScrollBar:
		{
			Keramik::ScrollBarPainter( KeramikGroove1, 2, (ceData.orientation == TQt::Horizontal)?true:false ).draw( p, r, cg.button(), cg.background(), (( flags & Style_Enabled ) == 0)?true:false );
			break;
		}

		case PE_MenuItemIndicatorFrame:
			{
				int x, y, w, h;
				r.rect( &x, &y, &w, &h );
				int checkcol = styleHint(SH_MenuIndicatorColumnWidth, ceData, elementFlags, opt, NULL, NULL);
				TQRect cr = visualRect( TQRect( x + 2, y + 2, checkcol - 1, h - 4 ), r );
				qDrawShadePanel( p, cr.x(), cr.y(), cr.width(), cr.height(), cg, true, 1, &cg.brush(TQColorGroup::Midlight) );
			}
			break;
		case PE_MenuItemIndicatorIconFrame:
			{
				int x, y, w, h;
				r.rect( &x, &y, &w, &h );
				int checkcol = styleHint(SH_MenuIndicatorColumnWidth, ceData, elementFlags, opt, NULL, NULL);
				TQRect cr = visualRect( TQRect( x + 2, y + 2, checkcol - 1, h - 4 ), r );
				qDrawShadePanel( p, cr.x(), cr.y(), cr.width(), cr.height(), cg, true, 1, &cg.brush(TQColorGroup::Midlight) );
			}
			break;
		case PE_MenuItemIndicatorCheck:
			{
				int x, y, w, h;
				r.rect( &x, &y, &w, &h );
				int checkcol = styleHint(SH_MenuIndicatorColumnWidth, ceData, elementFlags, opt, NULL, NULL);
				TQRect cr = visualRect( TQRect( x + 2, y + 2, checkcol - 1, h - 4 ), r );

				SFlags cflags = Style_Default;
				cflags |= active ? Style_Enabled : Style_On;

				drawPrimitive( PE_CheckMark, p, ceData, elementFlags, cr, cg, cflags );
			}
			break;

		default:
		{
			// ARROWS
			// -------------------------------------------------------------------
			if (pe >= PE_ArrowUp && pe <= PE_ArrowLeft)
			{
				TQPointArray a;

				switch(pe)
				{
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

				p->save();
				if ( flags & Style_Down )
					p->translate( pixelMetric( PM_ButtonShiftHorizontal, ceData, elementFlags ),
							pixelMetric( PM_ButtonShiftVertical, ceData, elementFlags ) );

				if ( flags & Style_Enabled )
				{
					a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
					p->setPen( cg.buttonText() );
					p->drawLineSegments( a );
				}
				else
				{
					a.translate( r.x() + r.width() / 2 + 1, r.y() + r.height() / 2 + 1 );
					p->setPen( cg.light() );
					p->drawLineSegments( a );
					a.translate( -1, -1 );
					p->setPen( cg.mid() );
					p->drawLineSegments( a );
				}
				p->restore();

			}
			else
				TDEStyle::drawPrimitive( pe, p, ceData, elementFlags, r, cg, flags, opt );
		}
	}
}

void KeramikStyle::drawTDEStylePrimitive( TDEStylePrimitive kpe,
										  TQPainter* p,
										  const TQStyleControlElementData &ceData,
										  ControlElementFlags elementFlags,
										  const TQRect &r,
										  const TQColorGroup &cg,
										  SFlags flags,
										  const TQStyleOption &opt,
										  const TQWidget* widget ) const
{
	bool disabled = ( flags & Style_Enabled ) == 0;
	int x, y, w, h;
	r.rect( &x, &y, &w, &h );

	switch ( kpe )
	{
		// SLIDER GROOVE
		// -------------------------------------------------------------------
		case KPE_SliderGroove:
		{
			bool horizontal = ceData.orientation == TQt::Horizontal;
			
			Keramik::TilePainter::PaintMode pmod = Keramik::TilePainter::PaintNormal;
			
			if (!ceData.bgPixmap.isNull())
				pmod = Keramik::TilePainter::PaintFullBlend;

			if ( horizontal )
				Keramik::RectTilePainter( keramik_slider_hgroove, false ).draw(p, r, cg.button(), cg.background(), disabled, pmod);
			else
				Keramik::RectTilePainter( keramik_slider_vgroove, true, false ).draw( p, r, cg.button(), cg.background(), disabled, pmod);

			break;
		}

		// SLIDER HANDLE
		// -------------------------------------------------------------------
		case KPE_SliderHandle:
			{
				bool horizontal = ceData.orientation == TQt::Horizontal;

				TQColor hl = cg.highlight();
				if (!disabled && flags & Style_Active)
					hl = Keramik::ColorUtil::lighten(cg.highlight(),110);

				if (horizontal)
					Keramik::ScaledPainter( keramik_slider ).draw( p, r, disabled ? cg.button() : hl,
						TQt::black,  disabled, Keramik::TilePainter::PaintFullBlend );
				else
					Keramik::ScaledPainter( keramik_vslider ).draw( p, r, disabled ? cg.button() : hl,
						TQt::black,  disabled, Keramik::TilePainter::PaintFullBlend );
				break;
			}

		// TOOLBAR HANDLE
		// -------------------------------------------------------------------
		case KPE_ToolBarHandle: {
			int x = r.x(); int y = r.y();
			int x2 = r.x() + r.width()-1;
			int y2 = r.y() + r.height()-1;
			
			TQToolBar* parent = 0;
			
			if (widget && widget->parent() && widget->parent()->inherits("TQToolBar"))
				parent = static_cast<TQToolBar*>(widget->parent());
				
			renderToolbarEntryBackground(p, parent, r, cg, (flags & Style_Horizontal));
			if (flags & Style_Horizontal) {
				p->setPen(cg.light());
				p->drawLine(x+1, y+4, x+1, y2-4);
				p->drawLine(x+3, y+4, x+3, y2-4);
				p->drawLine(x+5, y+4, x+5, y2-4);

				p->setPen(cg.mid());
				p->drawLine(x+2, y+4, x+2, y2-4);
				p->drawLine(x+4, y+4, x+4, y2-4);
				p->drawLine(x+6, y+4, x+6, y2-4);
			} else {
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


		default:
			TDEStyle::drawTDEStylePrimitive( kpe, p, ceData, elementFlags, r, cg, flags, opt, widget);
	}
}

bool KeramikStyle::isFormWidget(const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, const TQWidget* widget) const
{
	if (widget) {
		//Form widgets are in the TDEHTMLView, but that has 2 further inner levels
		//of widgets - QClipperWidget, and outside of that, QViewportWidget
		TQWidget* potentialClipPort = widget->parentWidget();
		if ((ceData.parentWidgetData.widgetObjectTypes.isEmpty()) && (ceData.parentWidgetFlags & CEF_IsTopLevel)) {
			return false;
		}
	
		TQWidget* potentialViewPort = potentialClipPort->parentWidget();
		if (!potentialViewPort || potentialViewPort->isTopLevel() ||
				qstrcmp(potentialViewPort->name(), "qt_viewport") )
			return false;
	
		TQWidget* potentialTDEHTML  = potentialViewPort->parentWidget();
		if (!potentialTDEHTML || potentialTDEHTML->isTopLevel() ||
				qstrcmp(potentialTDEHTML->className(), "TDEHTMLView") )
			return false;
	
	
		return true;
	}
	return false;
}

void KeramikStyle::drawControl( ControlElement element,
								  TQPainter *p,
								  const TQStyleControlElementData &ceData,
								  ControlElementFlags elementFlags,
								  const TQRect &r,
								  const TQColorGroup &cg,
								  SFlags flags,
								  const TQStyleOption& opt,
								  const TQWidget *widget ) const
{
	bool disabled = ( flags & Style_Enabled ) == 0;
	int x, y, w, h;
	r.rect( &x, &y, &w, &h );

	switch (element)
	{
		// PUSHBUTTON
		// -------------------------------------------------------------------
		case CE_PushButton:
		{
			const TQPushButton* btn = dynamic_cast< const TQPushButton* >( widget );

			if (isFormWidget(ceData, elementFlags, btn))
				formMode = true;

			if ( elementFlags & CEF_IsFlat )
				flatMode = true;

			if ( (elementFlags & CEF_IsDefault) && !flatMode )
			{
				drawPrimitive( PE_ButtonDefault, p, ceData, elementFlags, r, cg, flags );
			}
			else
			{
				if (ceData.parentWidgetData.widgetObjectTypes.contains("TQToolBar"))
					toolbarBlendWidget = widget;

				drawPrimitive( PE_ButtonCommand, p, ceData, elementFlags, r, cg, flags );
				toolbarBlendWidget = 0;
			}

			formMode = false;
			break;
		}


		// PUSHBUTTON LABEL
		// -------------------------------------------------------------------
		case CE_PushButtonLabel:
		{
			const TQPushButton* button = dynamic_cast<const TQPushButton *>( widget );
			bool active = ((elementFlags & CEF_IsOn) || (elementFlags & CEF_IsDown));
			bool cornArrow = false;

			// Shift button contents if pushed.
			if ( active )
			{
				x += pixelMetric(PM_ButtonShiftHorizontal, ceData, elementFlags, widget);
				y += pixelMetric(PM_ButtonShiftVertical, ceData, elementFlags, widget);
				flags |= Style_Sunken;
			}

			// Does the button have a popup menu?
			if (elementFlags & CEF_IsMenuWidget)
			{
				int dx = pixelMetric( PM_MenuButtonIndicator, ceData, elementFlags, widget );
				if ( !ceData.iconSet.isNull()  &&
					(dx + ceData.iconSet.pixmap (TQIconSet::Small, TQIconSet::Normal, TQIconSet::Off ).width()) >= w )
				{
					cornArrow = true; //To little room. Draw the arrow in the corner, don't adjust the widget
				}
				else
				{
					drawPrimitive( PE_ArrowDown, p, ceData, elementFlags, visualRect( TQRect(x + w - dx - 8, y + 2, dx, h - 4), r ),
							   cg, flags, opt );
					w -= dx;
				}
			}

			// Draw the icon if there is one
			if ( !ceData.iconSet.isNull() )
			{
				TQIconSet::Mode  mode  = TQIconSet::Disabled;
				TQIconSet::State state = TQIconSet::Off;

				if (elementFlags & CEF_IsEnabled)
					mode = (elementFlags & CEF_HasFocus) ? TQIconSet::Active : TQIconSet::Normal;
				if ((elementFlags & CEF_BiState) && (elementFlags & CEF_IsOn))
					state = TQIconSet::On;

				TQPixmap icon = ceData.iconSet.pixmap( TQIconSet::Small, mode, state );

				if (!ceData.text.isEmpty())
				{
					const int TextToIconMargin = 6;
					//Center text + icon w/margin in between..
					
					//Calculate length of both.
					int length = icon.width() + TextToIconMargin
					              + p->fontMetrics().size(ShowPrefix, ceData.text).width();
					
					//Calculate offset.
					int offset = (w - length)/2;
					
					//draw icon
					p->drawPixmap( x + offset, y + h / 2 - icon.height() / 2, icon );
					
					//new bounding rect for the text
					x += offset + icon.width() + TextToIconMargin;
					w =  length - icon.width() - TextToIconMargin;
				}
				else
				{
					//Icon only. Center it. 
					if (ceData.fgPixmap.isNull())
						p->drawPixmap( x + w/2 - icon.width()/2, y + h / 2 - icon.height() / 2,
										icon );
					else  //icon + pixmap. Ugh. 
						p->drawPixmap( x + (elementFlags & CEF_IsDefault) ? 8 : 4 , y + h / 2 - icon.height() / 2, icon );
				}

				if (cornArrow) //Draw over the icon
					drawPrimitive( PE_ArrowDown, p, ceData, elementFlags, visualRect( TQRect(x + w - 6, x + h - 6, 7, 7), r ),
							   cg, flags, opt );
			}

			// Make the label indicate if the button is a default button or not
			drawItem( p, TQRect(x, y, w, h), AlignCenter | ShowPrefix, ceData.colorGroup,
						(elementFlags & CEF_IsEnabled),  (ceData.fgPixmap.isNull())?NULL:&ceData.fgPixmap, ceData.text, -1,
						&ceData.colorGroup.buttonText() );

			if ( flags & Style_HasFocus )
				drawPrimitive( PE_FocusRect, p, ceData, elementFlags,
				               visualRect( subRect( SR_PushButtonFocusRect, ceData, elementFlags, widget ), ceData, elementFlags ),
				               cg, flags );
			break;
		}

		case CE_ToolButtonLabel:
		{
			bool onToolbar = ceData.parentWidgetData.widgetObjectTypes.contains( "TQToolBar" );
			TQRect nr = r;

			if (!onToolbar)
			{
				if (!qstrcmp(ceData.parentWidgetData.name.ascii(),"qt_maxcontrols" ) )
				{
					//Make sure we don't cut into the endline
					if (!qstrcmp(ceData.name.ascii(), "close"))
					{
						nr.addCoords(0,0,-1,0);
						p->setPen(cg.dark());
						p->drawLine(r.x() + r.width() - 1, r.y(),
								r.x() + r.width() - 1, r.y() + r.height() - 1 );
					}
				}
				//else if ( w > smallButMaxW && h > smallButMaxH )
				//	nr.setWidth(r.width()-2); //Account for shadow
			}

			TDEStyle::drawControl(element, p, ceData, elementFlags, nr, cg, flags, opt, widget);
			break;
		}

		case CE_TabBarTab:
		{
			bool bottom = ceData.tabBarData.shape == TQTabBar::RoundedBelow ||
			              ceData.tabBarData.shape == TQTabBar::TriangularBelow;

			if ( flags & Style_Selected )
			{
				TQRect tabRect = r;
				//If not the right-most tab, readjust the painting to be one pixel wider
				//to avoid a doubled line
				int rightMost = TQApplication::reverseLayout() ? 0 : ceData.tabBarData.tabCount - 1;

				if (ceData.tabBarData.identIndexMap[opt.tab()->identifier()] != rightMost)
					tabRect.setWidth( tabRect.width() + 1);
				Keramik::ActiveTabPainter( bottom ).draw( p, tabRect, cg.button().light(110), cg.background(), !(elementFlags & CEF_IsEnabled), pmode());
			}
			else
			{
				Keramik::InactiveTabPainter::Mode mode;
				int index = ceData.tabBarData.identIndexMap[opt.tab()->identifier()];
				if ( index == 0 ) mode = Keramik::InactiveTabPainter::First;
				else if ( index == ceData.tabBarData.tabCount - 1 ) mode = Keramik::InactiveTabPainter::Last;
				else mode = Keramik::InactiveTabPainter::Middle;

				if ( bottom )
				{
					Keramik::InactiveTabPainter( mode, bottom ).draw( p, x, y, w, h - 3, cg.button(), cg.background(), disabled, pmode() );
					p->setPen( cg.dark() );
					p->drawLine( x, y, x + w, y );
				}
				else
				{
					Keramik::InactiveTabPainter( mode, bottom ).draw( p, x, y + 3, w, h - 3, cg.button(), cg.background(), disabled, pmode() );
					p->setPen( cg.light() );
					p->drawLine( x, y + h - 1, x + w, y + h - 1 );
				}
			}

			break;
		}

		case CE_DockWindowEmptyArea:
		{
			TQRect pr = r;
			if (ceData.widgetObjectTypes.contains("TQToolBar"))
			{
				const TQToolBar* tb = static_cast<const TQToolBar*>(widget);
				if (tb->place() == TQDockWindow::OutsideDock)
				{
					//Readjust the paint rectangle to skip the titlebar
					pr = TQRect(r.x(), titleBarH + tb->frameWidth(),
						r.width(), tb->height() - titleBarH - 2 * tb->frameWidth() + 2);
					//2 at the end = the 2 pixels of border of a "regular"
					//toolbar we normally paint over.
				}
				Keramik::GradientPainter::renderGradient( p, pr, cg.button(),
										 tb->orientation() == TQt::Horizontal);
			}
			else
				TDEStyle::drawControl( (ControlElement)CE_DockWindowEmptyArea, p, ceData, elementFlags,
					r, cg, flags, opt, widget );
			break;
		}
		case CE_MenuBarEmptyArea:
		{
			Keramik::GradientPainter::renderGradient( p, r, cg.button(), true, true);
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
				Keramik::GradientPainter::renderGradient( p, pr, cg.button(), true, true);

			drawItem( p, r, AlignCenter | AlignVCenter | ShowPrefix
					| DontClip | SingleLine, cg, flags & Style_Enabled,
					mi->pixmap(), mi->text() );

			break;
		}


		// POPUPMENU ITEM
		// -------------------------------------------------------------------
		case CE_PopupMenuItem: {
			TQRect main = r;

			TQMenuItem *mi = opt.menuItem();

			if ( !mi )
			{
				// Don't leave blank holes if we set NoBackground for the TQPopupMenu.
				// This only happens when the popupMenu spans more than one column.
				if (! ( !ceData.bgPixmap.isNull() ) )
					p->fillRect( r, cg.background().light( 105 ) );

				break;
			}
			int  tab        = opt.tabWidth();
			int  checkcol   = opt.maxIconWidth();
			bool enabled    = mi->isEnabled();
			bool checkable  = (elementFlags & CEF_IsCheckable);
			bool active     = flags & Style_Active;
			bool etchtext   = styleHint( SH_EtchDisabledText, ceData, elementFlags );
			bool reverse    = TQApplication::reverseLayout();
			if ( checkable )
				checkcol = TQMAX( checkcol, 20 );

			// Draw the menu item background
			if ( active )
			{
				if ( enabled )
					Keramik::RowPainter( keramik_menuitem ).draw( p, main, cg.highlight(), cg.background() );
				else {
					if ( !ceData.bgPixmap.isNull() )
						p->drawPixmap( main.topLeft(), ceData.bgPixmap, main );
					else p->fillRect( main, cg.background().light( 105 ) );
					p->drawWinFocusRect( r );
				}
			}
			// Draw the transparency pixmap
			else if ( !ceData.bgPixmap.isNull() )
				p->drawPixmap( main.topLeft(), ceData.bgPixmap, main );
			// Draw a solid background
			else
				p->fillRect( main, cg.background().light( 105 ) );
			// Are we a menu item separator?

			if ( mi->isSeparator() )
			{
				p->setPen( cg.mid() );
				p->drawLine( main.x()+5, main.y() + 1, main.right()-5, main.y() + 1 );
				p->setPen( cg.light() );
				p->drawLine( main.x()+5, main.y() + 2, main.right()-5, main.y() + 2 );
				break;
			}

			TQRect cr = visualRect( TQRect( x + 2, y + 2, checkcol - 1, h - 4 ), r );
			// Do we have an icon?
			if ( mi->iconSet() )
			{
				TQIconSet::Mode mode;



				// Select the correct icon from the iconset
				if ( active )
					mode = enabled ? TQIconSet::Active : TQIconSet::Disabled;
				else
					mode = enabled ? TQIconSet::Normal : TQIconSet::Disabled;

				// Do we have an icon and are checked at the same time?
				// Then draw a "pressed" background behind the icon
				if ( checkable && /*!active &&*/ mi->isChecked() )
					drawPrimitive(PE_MenuItemIndicatorIconFrame, p, ceData, elementFlags, r, cg, flags, opt);
				// Draw the icon
				TQPixmap pixmap = mi->iconSet()->pixmap( TQIconSet::Small, mode );
				TQRect pmr( 0, 0, pixmap.width(), pixmap.height() );
				pmr.moveCenter( cr.center() );
				p->drawPixmap( pmr.topLeft(), pixmap );
			}

			// Are we checked? (This time without an icon)
			else if ( checkable && mi->isChecked() )
			{
				// We only have to draw the background if the menu item is inactive -
				// if it's active the "pressed" background is already drawn
			//	if ( ! active )
					drawPrimitive(PE_MenuItemIndicatorFrame, p, ceData, elementFlags, r, cg, flags, opt);
				// Draw the checkmark
				drawPrimitive(PE_MenuItemIndicatorCheck, p, ceData, elementFlags, r, cg, flags, opt);
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

					//TQColor draw = cg.text();
					TQColor draw = (active && enabled) ? cg.highlightedText () : cg.text();
					p->setPen(draw);


					// Does the menu item have a tabstop? (for the accelerator text)
					if ( t >= 0 ) {
						int tabx = reverse ? x + rightBorder + itemHMargin + itemFrame :
							x + w - tab - rightBorder - itemHMargin - itemFrame;

						// Draw the right part of the label (accelerator text)
						if ( etchtext && !enabled ) {
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
					if ( etchtext && !enabled ) {
						// Etched text again for inactive disabled menu items...
						p->setPen( cg.light() );
						p->drawText( xp+offset, y+m+1, tw, h-2*m, text_flags, s, t );
						p->setPen( discol );
					}


					p->drawText( xp, y+m, tw, h-2*m, text_flags, s, t );

					p->setPen(cg.text());

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
				int dim = pixelMetric(PM_MenuButtonIndicator, ceData, elementFlags) - itemFrame;
				TQRect vr = visualRect( TQRect( x + w - arrowHMargin - itemFrame - dim,
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
		case CE_ProgressBarContents: {
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
				if (steps == 0)
					width = TQMIN(width,20); //Don't cross squares

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

					//Store the progress rect.
					TQRect progressRect;
					if (reverse)
						progressRect = TQRect(cr.x() + cr.width() - width - pstep, cr.y(),
										width, cr.height());
					else
						progressRect = TQRect(cr.x() + pstep, cr.y(), width, cr.height());

					Keramik::RowPainter(keramik_progressbar).draw(p, progressRect,
									 cg.highlight(), cg.background() );
					return;
				}

				TQRect progressRect;

				if (reverse)
					progressRect = TQRect(cr.x()+(cr.width()-width), cr.y(), width, cr.height());
				else
					progressRect = TQRect(cr.x(), cr.y(), width, cr.height());

				//Apply the animation rectangle.
				//////////////////////////////////////
				if (animateProgressBar)
				{
					const TQProgressBar* pb = (const TQProgressBar*)widget;
					int progAnimShift = progAnimWidgets[const_cast<TQProgressBar*>(pb)];
					if (reverse)
					{
						//Here, we can't simply shift, as the painter code calculates everything based
						//on the left corner, so we need to draw the 2 portions ourselves.

						//Start off by checking the geometry of the end pixmap - it introduces a bit of an offset
						TQSize   endDim = loader.size(keramik_progressbar + 3); //3 = 3*1 + 0 = (1,0) = cl

						//We might not have anything to animate at all, though, if the ender is the only thing to paint
						if (endDim.width() < progressRect.width())
						{
							//OK, so we do have some stripes.
							// Render the endline now - the clip region below will protect it from getting overwriten
							TQPixmap endline = loader.scale(keramik_progressbar + 3, endDim.width(), progressRect.height(),
															cg.highlight(), cg.background());
							p->drawPixmap(progressRect.x(), progressRect.y(), endline);

							//Now, calculate where the stripes should be, and set a clip region to that
							progressRect.setLeft(progressRect.x() + endline.width());
							p->setClipRect(progressRect, TQPainter::CoordPainter);

							//Expand the paint region slightly to get the animation offset.
							progressRect.setLeft(progressRect.x() - progAnimShift);

							//Paint the stripes.
							TQPixmap stripe = loader.scale(keramik_progressbar + 4, 28, progressRect.height(),
														  cg.highlight(), cg.background());
														  //4 = 3*1 + 1 = (1,1) = cc

							p->drawTiledPixmap(progressRect,  stripe);
							//Exit out here to skip the regular paint path
							return;
						}
					}
					else
					{
						//Clip to the old rectangle
						p->setClipRect(progressRect, TQPainter::CoordPainter);
						//Expand the paint region
						progressRect.setLeft(progressRect.x() - 28 + progAnimShift);
					}
				}

				Keramik::ProgressBarPainter(keramik_progressbar, reverse).draw( p,
							progressRect , cg.highlight(), cg.background());
			}
			break;
		}


		default:
			TDEStyle::drawControl(element, p, ceData, elementFlags, r, cg, flags, opt, widget);
	}
}

void KeramikStyle::drawControlMask( ControlElement element,
								    TQPainter *p,
								    const TQStyleControlElementData &ceData,
								    ControlElementFlags elementFlags,
								    const TQRect &r,
								    const TQStyleOption& opt,
								    const TQWidget *widget ) const
{
	p->fillRect(r, color1);
	maskMode = true;
	drawControl( element, p, ceData, elementFlags, r, TQApplication::palette().active(), TQStyle::Style_Default, opt, widget);
	maskMode = false;
}

bool KeramikStyle::isSizeConstrainedCombo(const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, const TQComboBox* combo) const
{
	if (ceData.rect.width() >= 80)
		return false;

	if (combo) {
		int suggestedWidth = combo->sizeHint().width();

		if (ceData.rect.width() - suggestedWidth < -5)
			return true;

		return false;
	}
	else {
		return true;
	}
}

void KeramikStyle::drawComplexControl( ComplexControl control,
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
	bool disabled = ( flags & Style_Enabled ) == 0;
	switch(control)
	{
		// COMBOBOX
		// -------------------------------------------------------------------
		case CC_ComboBox:
		{
			bool             toolbarMode = false;
			const TQComboBox* cb         = dynamic_cast< const TQComboBox* >( widget );
			bool             compact     = isSizeConstrainedCombo(ceData, elementFlags, cb);

			if (isFormWidget(ceData, elementFlags, cb))
				formMode = true;

			TQPixmap * buf = 0;
			TQPainter* p2 = p;

			TQRect br = r;

			if (controls == SC_All)
			{
				//Double-buffer only when we are in the slower full-blend mode
				if ( ceData.parentWidgetData.widgetObjectTypes.contains("TQToolBar") || !qstrcmp(ceData.parentWidgetData.name.ascii(), kdeToolbarWidget) )
				{
					buf = new TQPixmap( r.width(), r.height() );
					br.setX(0);
					br.setY(0);
					p2 = new TQPainter(buf);
					
					//Ensure that we have clipping on, and have a sane base.
					//If need be, Qt will shrink us to the paint region.
					p->setClipRect(r);
					toolbarMode = true;
				}
			}


			if ( br.width() >= 28 && br.height() > 20 && !compact )
				br.addCoords( 0, -2, 0, 0 );
				
			//When in compact mode, we force the shadow-less bevel mode,
			//but that also alters height and not just width.
			//readjust height to fake the other metrics (plus clear 
			//the other areas, as appropriate). The automasker
			//will take care of the overall shape.
			if ( compact )
			{
				forceSmallMode = true;
				br.setHeight( br.height() - 2);
				p->fillRect ( r.x(), r.y() + br.height(), r.width(), 2, cg.background());
			}
				
				
			if ( controls & SC_ComboBoxFrame )
			{
				if (toolbarMode)
					toolbarBlendWidget = widget;

				drawPrimitive( PE_ButtonCommand, p2, ceData, elementFlags, br, cg, flags );

				toolbarBlendWidget = 0;
			}

			// don't draw the focus rect etc. on the mask
			if ( cg.button() == color1 && cg.background() == color0 )
				break;

			if ( controls & SC_ComboBoxArrow )
			{
				if ( active )
					flags |= Style_On;
					
				TQRect ar = querySubControlMetrics( CC_ComboBox, ceData, elementFlags,
													 SC_ComboBoxArrow, TQStyleOption::Default, widget );
				if (!compact)
				{
					ar.setWidth(ar.width()-13);
					TQRect rr = visualRect( TQRect( ar.x(), ar.y() + 4,
							loader.size(keramik_ripple ).width(), ar.height() - 8 ),
							ceData, elementFlags );

					ar = visualRect( TQRect( ar.x() + loader.size( keramik_ripple ).width() + 4, ar.y(), 
											11, ar.height() ), 
									ceData, elementFlags );

					TQPointArray a;
	
					a.setPoints(TQCOORDARRLEN(keramik_combo_arrow), keramik_combo_arrow);
	
					a.translate( ar.x() + ar.width() / 2, ar.y() + ar.height() / 2 );
					p2->setPen( cg.buttonText() );
					p2->drawLineSegments( a );
	
					Keramik::ScaledPainter( keramik_ripple ).draw( p2, rr, cg.button(), TQt::black, disabled, Keramik::TilePainter::PaintFullBlend );
				}
				else //Size-constrained combo -- loose the ripple.
				{
					ar.setWidth(ar.width() - 7);
					ar = visualRect( TQRect( ar.x(), ar.y(), 11, ar.height() ), ceData, elementFlags);
					TQPointArray a;
	
					a.setPoints(TQCOORDARRLEN(keramik_combo_arrow), keramik_combo_arrow);
	
					a.translate( ar.x() + ar.width() / 2, ar.y() + ar.height() / 2 );
					p2->setPen( cg.buttonText() );
					p2->drawLineSegments( a );
				}
			}

			if ( controls & SC_ComboBoxEditField )
			{
				if ( elementFlags & CEF_IsEditable )
				{
					TQRect er = visualRect( querySubControlMetrics( CC_ComboBox, ceData, elementFlags, SC_ComboBoxEditField, TQStyleOption::Default, widget ), ceData, elementFlags );
					er.addCoords( -2, -2, 2, 2 );
					p2->fillRect( er, cg.base() );
					drawPrimitive( PE_PanelLineEdit, p2, ceData, elementFlags, er, cg );
					Keramik::RectTilePainter( keramik_frame_shadow, false, false, 2, 2 ).draw( p2, er, cg.button(),
						TQt::black, false, pmodeFullBlend() );
				}
				else if ( elementFlags & CEF_HasFocus )
				{
					TQRect re = TQStyle::visualRect(subRect(SR_ComboBoxFocusRect, ceData, elementFlags, cb), ceData, elementFlags);
					if ( compact )
						re.addCoords( 3, 3, 0, -3 );
					p2->fillRect( re, cg.brush( TQColorGroup::Highlight ) );
					drawPrimitive( PE_FocusRect, p2, ceData, elementFlags, re, cg,
					Style_FocusAtBorder, TQStyleOption( cg.highlight() ) );
				}
				// TQComboBox draws the text on its own and uses the painter's current colors
				if ( elementFlags & CEF_HasFocus )
				{
					p->setPen( cg.highlightedText() );
					p->setBackgroundColor( cg.highlight() );
				}
				else
				{
					p->setPen( cg.text() );
					p->setBackgroundColor( cg.button() );
				}
			}

			if (p2 != p)
			{
				p2->end();
				delete p2;
				p->drawPixmap(r.x(), r.y(), *buf);
				delete buf;
			}

			formMode = false;
			break;
		}

		case CC_SpinWidget:
		{
			const TQSpinWidget* sw = static_cast< const TQSpinWidget* >( widget );
			TQRect br = visualRect( querySubControlMetrics( (ComplexControl)CC_SpinWidget, ceData, elementFlags, SC_SpinWidgetButtonField, TQStyleOption::Default, widget ), ceData, elementFlags );
			if ( controls & SC_SpinWidgetButtonField )
			{
				Keramik::SpinBoxPainter().draw( p, br, cg.button(), cg.background(), !sw->isEnabled() );
				if ( active & SC_SpinWidgetUp )
					Keramik::CenteredPainter( keramik_spinbox_pressed_arrow_up ).draw( p, br.x(), br.y() + 3, br.width(), br.height() / 2, cg.button(), cg.background() );
				else
					Keramik::CenteredPainter( keramik_spinbox_arrow_up ).draw( p, br.x(), br.y() + 3, br.width(), br.height() / 2, cg.button(), cg.background(), !sw->isUpEnabled() );
				if ( active & SC_SpinWidgetDown )
					Keramik::CenteredPainter( keramik_spinbox_pressed_arrow_down ).draw( p, br.x(), br.y() + br.height() / 2 , br.width(), br.height() / 2 - 8, cg.button(), cg.background()  );
				else
					Keramik::CenteredPainter( keramik_spinbox_arrow_down ).draw( p, br.x(), br.y() + br.height() / 2, br.width(), br.height() / 2 - 8, cg.background(), cg.button(), !sw->isDownEnabled() );
			}

			if ( controls & SC_SpinWidgetFrame )
				drawPrimitive( PE_PanelLineEdit, p, ceData, elementFlags, r, cg );

			break;
		}
		case CC_ScrollBar:
		{
			if (highlightScrollBar && (elementFlags & CEF_HasParentWidget)) //Don't do the check if not highlighting anyway
			{
				if (ceData.parentWidgetData.colorGroup.button() != ceData.colorGroup.button())
					customScrollMode = true;
			}
			bool horizontal = ceData.orientation == TQt::Horizontal;
			TQRect slider, subpage, addpage, subline, addline;
			if ( ceData.minSteps == ceData.maxSteps ) flags &= ~Style_Enabled;

			slider = querySubControlMetrics( control, ceData, elementFlags, SC_ScrollBarSlider, opt, widget );
			subpage = querySubControlMetrics( control, ceData, elementFlags, SC_ScrollBarSubPage, opt, widget );
			addpage = querySubControlMetrics( control, ceData, elementFlags, SC_ScrollBarAddPage, opt, widget );
			subline = querySubControlMetrics( control, ceData, elementFlags, SC_ScrollBarSubLine, opt, widget );
			addline = querySubControlMetrics( control, ceData, elementFlags, SC_ScrollBarAddLine, opt, widget );

			if ( controls & SC_ScrollBarSubLine )
				drawPrimitive( PE_ScrollBarSubLine, p, ceData, elementFlags, subline, cg,
				               flags | ( ( active & SC_ScrollBarSubLine ) ? Style_Down : 0 ) );

			TQRegion clip;
			if ( controls & SC_ScrollBarSubPage ) clip |= subpage;
			if ( controls & SC_ScrollBarAddPage ) clip |= addpage;
			if ( horizontal )
				clip |= TQRect( slider.x(), 0, slider.width(), ceData.rect.height() );
			else
				clip |= TQRect( 0, slider.y(), ceData.rect.width(), slider.height() );
			clip ^= slider;

			p->setClipRegion( clip );
			Keramik::ScrollBarPainter( KeramikGroove1, 2, horizontal ).draw( p, slider | subpage | addpage, cg.button(), cg.background(), disabled );

			if ( controls & SC_ScrollBarSlider )
			{
				if ( horizontal )
					p->setClipRect( slider.x(), slider.y(), addpage.right() - slider.x() + 1, slider.height() );
				else
					p->setClipRect( slider.x(), slider.y(), slider.width(), addpage.bottom() - slider.y() + 1 );
				drawPrimitive( PE_ScrollBarSlider, p, ceData, elementFlags, slider, cg,
					flags | ( ( active == SC_ScrollBarSlider ) ? Style_Down : 0 )  );
			}
			p->setClipping( false );

			if ( controls & ( SC_ScrollBarSubLine | SC_ScrollBarAddLine ) )
			{
				drawPrimitive( PE_ScrollBarAddLine, p, ceData, elementFlags, addline, cg, flags );
				if ( active & SC_ScrollBarSubLine )
				{
					if ( horizontal )
						p->setClipRect( TQRect( addline.x(), addline.y(), addline.width() / 2, addline.height() ) );
					else
						p->setClipRect( TQRect( addline.x(), addline.y(), addline.width(), addline.height() / 2 ) );
					drawPrimitive( PE_ScrollBarAddLine, p, ceData, elementFlags, addline, cg, flags | Style_Down );
				}
				else if ( active & SC_ScrollBarAddLine )
				{
					if ( horizontal )
						p->setClipRect( TQRect( addline.x() + addline.width() / 2, addline.y(), addline.width() / 2, addline.height() ) );
					else
						p->setClipRect( TQRect( addline.x(), addline.y() + addline.height() / 2, addline.width(), addline.height() / 2 ) );
					drawPrimitive( PE_ScrollBarAddLine, p, ceData, elementFlags, addline, cg, flags | Style_Down );
				}
			}

			customScrollMode = false;


			break;
		}

		// TOOLBUTTON
		// -------------------------------------------------------------------
		case CC_ToolButton: {
			bool onToolbar = ceData.parentWidgetData.widgetObjectTypes.contains("TQToolBar");
			bool onExtender = !onToolbar &&
				ceData.parentWidgetData.widgetObjectTypes.contains( "TQToolBarExtensionWidget") &&
				widget && widget->parentWidget()->parentWidget()->inherits( "TQToolBar" );

			bool onControlButtons = false;
			if (!onToolbar && !onExtender && !ceData.parentWidgetData.widgetObjectTypes.isEmpty() &&
				 !qstrcmp(ceData.parentWidgetData.name.ascii(),"qt_maxcontrols" ) )
			{
				onControlButtons = true;
				titleBarMode = Maximized;
			}

			TQRect button, menuarea;
			button   = querySubControlMetrics(control, ceData, elementFlags, SC_ToolButton, opt, widget);
			menuarea = querySubControlMetrics(control, ceData, elementFlags, SC_ToolButtonMenu, opt, widget);

			SFlags bflags = flags,
				   mflags = flags;

			if (active & SC_ToolButton)
				bflags |= Style_Down;
			if (active & SC_ToolButtonMenu)
				mflags |= Style_Down;

			if (onToolbar &&  ceData.toolBarData.orientation == TQt::Horizontal)
				bflags |= Style_Horizontal;

			if (controls & SC_ToolButton)
			{
				// If we're pressed, on, or raised...
				if (bflags & (Style_Down | Style_On | Style_Raised) || onControlButtons)
				{
					//Make sure the standalone toolbuttons have a gradient in the right direction
					if (!onToolbar && !onControlButtons)
						bflags |= Style_Horizontal;

					drawPrimitive( PE_ButtonTool, p, ceData, elementFlags, button, cg,
					 bflags, opt);
				}

				// Check whether to draw a background pixmap
				else if ( !ceData.parentWidgetData.bgPixmap.isNull() )
				{
					TQPixmap pixmap = ceData.parentWidgetData.bgPixmap;
					p->drawTiledPixmap( r, pixmap, ceData.pos );
				}
				else if (onToolbar)
				{
					renderToolbarWidgetBackground(p, ceData, elementFlags, widget);
				}
				else if (onExtender)
				{
					// This assumes floating toolbars can't have extenders,
					//(so if we're on an extender, we're not floating)
					TQWidget*  parent  = static_cast<TQWidget*> (static_cast<TQWidget*>(widget->parent()));
					TQToolBar* toolbar = static_cast<TQToolBar*>(parent->parent());
					TQRect tr    = ceData.parentWidgetData.rect;
					bool  horiz = ceData.toolBarData.orientation == TQt::Horizontal;

					//Calculate offset. We do this by translating our coordinates,
					//which are relative to the parent, to be relative to the toolbar.
					int xoff = 0, yoff = 0;
					if (horiz)
						yoff = parent->mapToParent(ceData.pos).y();
					else
						xoff = parent->mapToParent(ceData.pos).x();

					Keramik::GradientPainter::renderGradient( p, r, cg.button(),
							horiz, false, /*Not a menubar*/
							xoff,  yoff,
							tr.width(), tr.height());
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

			titleBarMode = None;

			break;
		}

		case CC_TitleBar:
			titleBarMode = Regular; //Handle buttons on titlebar different from toolbuttons
		default:
			TDEStyle::drawComplexControl( control, p, ceData, elementFlags,
						r, cg, flags, controls, active, opt, widget );

			titleBarMode = None;
	}
}

void KeramikStyle::drawComplexControlMask( ComplexControl control,
                                         TQPainter *p,
                                         const TQStyleControlElementData &ceData,
                                         const ControlElementFlags elementFlags,
                                         const TQRect &r,
                                         const TQStyleOption& opt,
                                         const TQWidget *widget ) const
{
	if (control == CC_ComboBox)
	{
		maskMode = true;
		drawComplexControl(CC_ComboBox, p, ceData, elementFlags, r,
				TQApplication::palette().active(), Style_Default,
				SC_ComboBoxFrame,SC_None, opt, widget);
		maskMode = false;

	}
	else
		p->fillRect(r, color1);

}

int KeramikStyle::pixelMetric(PixelMetric m, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQWidget *widget) const
{
	switch(m)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PM_ButtonMargin:				// Space btw. frame and label
			return 4;

		case PM_SliderLength:
			return 12;
		case PM_SliderControlThickness:
			return loader.size( keramik_slider ).height() - 4;
		case PM_SliderThickness:
			return loader.size( keramik_slider ).height();

		case PM_ButtonShiftHorizontal:
			return 0;
		case PM_ButtonShiftVertical: // Offset by 1
			return 1;


		// CHECKBOXES / RADIO BUTTONS
		// -------------------------------------------------------------------
		case PM_ExclusiveIndicatorWidth:	// Radiobutton size
			return loader.size( keramik_radiobutton_on ).width();
		case PM_ExclusiveIndicatorHeight:
			return loader.size( keramik_radiobutton_on ).height();
		case PM_IndicatorWidth:				// Checkbox size
			return loader.size( keramik_checkbox_on ).width();
		case PM_IndicatorHeight:
			return loader.size( keramik_checkbox_on) .height();

		case PM_ScrollBarExtent:
			return loader.size( keramik_scrollbar_vbar + KeramikGroove1).width();
		case PM_ScrollBarSliderMin:
			return loader.size( keramik_scrollbar_vbar + KeramikSlider1 ).height() +
                        loader.size( keramik_scrollbar_vbar + KeramikSlider3 ).height();

		case PM_SpinBoxFrameWidth:		
		case PM_DefaultFrameWidth:
			return 1;

		case PM_MenuButtonIndicator:
			return 13;

		case PM_TabBarTabVSpace:
			return 12;

		case PM_TabBarTabOverlap:
			return 0;
			
		case PM_TabBarTabShiftVertical:
		{
			if (ceData.widgetObjectTypes.contains("TQTabBar"))
			{
				if (ceData.tabBarData.shape == TQTabBar::RoundedBelow || 
					ceData.tabBarData.shape == TQTabBar::TriangularBelow)
					return 0;
			}
			
			return 2; //For top, or if not sure
		}
			

		case PM_TitleBarHeight:
			return titleBarH;

		case PM_MenuIndicatorFrameHBorder:
		case PM_MenuIndicatorFrameVBorder:
		case PM_MenuIconIndicatorFrameHBorder:
		case PM_MenuIconIndicatorFrameVBorder:
			return 2;

		default:
			return TDEStyle::pixelMetric(m, ceData, elementFlags, widget);
	}
}


TQSize KeramikStyle::sizeFromContents( ContentsType contents,
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
		case CT_PushButton:
		{
			const TQPushButton* btn = dynamic_cast< const TQPushButton* >( widget );

			int w = contentSize.width() + 2 * pixelMetric( PM_ButtonMargin, ceData, elementFlags, widget );
			int h = contentSize.height() + 2 * pixelMetric( PM_ButtonMargin, ceData, elementFlags, widget );
			if ( ceData.text.isEmpty() && contentSize.width() < 32 ) return TQSize( w, h );


			//For some reason kcontrol no longer does this...
			//if ( (elementFlags & CEF_IsDefault) || (elementFlags & CEF_AutoDefault) )
			//            w = TQMAX( w, 40 );

			return TQSize( w + 30, h + 5 ); //MX: No longer blank space -- can make a bit smaller
		}

		case CT_ToolButton:
		{
			bool onToolbar = widget->parentWidget() && widget->parentWidget()->inherits( "TQToolBar" );
			if (!onToolbar) //Behaves like a button, so scale appropriately to the border
			{
				int w = contentSize.width();
				int h = contentSize.height();
				return TQSize( w + 12, h + 10 );
			}
			else
			{
				return TDEStyle::sizeFromContents( contents, ceData, elementFlags, contentSize, opt, widget );
			}
		}

		case CT_ComboBox: {
			int arrow = 11 + loader.size( keramik_ripple ).width();
			return TQSize( contentSize.width() + arrow + ((elementFlags & CEF_IsEditable) ? 26 : 22),
					contentSize.height() + 10 );
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
				w = 30; // Arbitrary
				h = 3;
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
				w += itemHMargin + itemFrame*2 + 7;
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


TQStyle::SubControl KeramikStyle::querySubControl( ComplexControl control,
                                                  const TQStyleControlElementData &ceData,
                                                  ControlElementFlags elementFlags,
                                                  const TQPoint& point,
                                                  const TQStyleOption& opt,
                                                  const TQWidget* widget ) const
{
	SubControl result = TDEStyle::querySubControl( control, ceData, elementFlags, point, opt, widget );
	if ( control == CC_ScrollBar && result == SC_ScrollBarAddLine )
	{
		TQRect addline = querySubControlMetrics( control, ceData, elementFlags, result, opt, widget );
		if ( static_cast< const TQScrollBar* >( widget )->orientation() == TQt::Horizontal )
		{
			if ( point.x() < addline.center().x() ) result = SC_ScrollBarSubLine;
		}
		else if ( point.y() < addline.center().y() ) result = SC_ScrollBarSubLine;
	}
	return result;
}

TQRect KeramikStyle::querySubControlMetrics( ComplexControl control,
	                              const TQStyleControlElementData &ceData,
	                              ControlElementFlags elementFlags,
	                              SubControl subcontrol,
	                              const TQStyleOption& opt,
	                              const TQWidget* widget ) const
{
	switch ( control )
	{
		case CC_ComboBox:
		{
			int arrow;
			bool compact = false;
			if ( isSizeConstrainedCombo(ceData, elementFlags, dynamic_cast<const TQComboBox*>(widget)) ) //### constant
				compact = true;
				
			if ( compact )
				arrow = 11;
			else
				arrow = 11 + loader.size( keramik_ripple ).width();
				
			switch ( subcontrol )
			{
				case SC_ComboBoxArrow:
					if ( compact )
						return TQRect( ceData.rect.width() - arrow - 7, 0, arrow + 6, ceData.rect.height() );
					else
						return TQRect( ceData.rect.width() - arrow - 14, 0, arrow + 13, ceData.rect.height() );

				case SC_ComboBoxEditField:
				{
					if ( compact )
						return TQRect( 2, 4, ceData.rect.width() - arrow - 2 - 7, ceData.rect.height() - 8 );
					else if ( ceData.rect.width() < 36 || ceData.rect.height() < 22 )
						return TQRect( 4, 3, ceData.rect.width() - arrow - 20, ceData.rect.height() - 6 );
					else if ( elementFlags & CEF_IsEditable )
						return TQRect( 8, 4, ceData.rect.width() - arrow - 26, ceData.rect.height() - 11 );
					else
						return TQRect( 6, 4, ceData.rect.width() - arrow - 22, ceData.rect.height() - 9 );
				}

				case SC_ComboBoxListBoxPopup:
				{
					//Note that the widget here == the combo, not the completion
					//box, so we don't get any recursion
					int suggestedWidth = widget->sizeHint().width(); 
					TQRect def = opt.rect();
					def.addCoords( 4, -4, -6, 4 );
					
					if ((def.width() - suggestedWidth < -12) && (def.width() < 80))
						def.setWidth(TQMIN(80, suggestedWidth - 10));

					return def;
				}

				default: break;
			}
			break;
		}

		case CC_ScrollBar:
		{
			bool horizontal = ceData.orientation == TQt::Horizontal;
			int addline, subline, sliderpos, sliderlen, maxlen, slidermin;
			if ( horizontal )
			{
				subline = loader.size( keramik_scrollbar_hbar_arrow1 ).width();
				addline = loader.size( keramik_scrollbar_hbar_arrow2 ).width();
				maxlen = ceData.rect.width() - subline - addline + 2;
			}
			else
			{
				subline = loader.size( keramik_scrollbar_vbar_arrow1 ).height();
				addline = loader.size( keramik_scrollbar_vbar_arrow2 ).height();
				maxlen = ceData.rect.height() - subline - addline + 2;
			}
			sliderpos = ceData.startStep;
			if ( ceData.minSteps != ceData.maxSteps )
			{
				int range = ceData.maxSteps - ceData.minSteps;
				sliderlen = ( ceData.pageStep * maxlen ) / ( range + ceData.pageStep );
				slidermin = pixelMetric( PM_ScrollBarSliderMin, ceData, elementFlags, widget );
				if ( sliderlen < slidermin ) sliderlen = slidermin;
				if ( sliderlen > maxlen ) sliderlen = maxlen;
			}
			else sliderlen = maxlen;

			switch ( subcontrol )
			{
				case SC_ScrollBarGroove:
					if ( horizontal ) return TQRect( subline, 0, maxlen, ceData.rect.height() );
					else return TQRect( 0, subline, ceData.rect.width(), maxlen );

				case SC_ScrollBarSlider:
					if (horizontal) return TQRect( sliderpos, 0, sliderlen, ceData.rect.height() );
					else return TQRect( 0, sliderpos, ceData.rect.width(), sliderlen );

				case SC_ScrollBarSubLine:
					if ( horizontal ) return TQRect( 0, 0, subline, ceData.rect.height() );
					else return TQRect( 0, 0, ceData.rect.width(), subline );

				case SC_ScrollBarAddLine:
					if ( horizontal ) return TQRect( ceData.rect.width() - addline, 0, addline, ceData.rect.height() );
					else return TQRect( 0, ceData.rect.height() - addline, ceData.rect.width(), addline );

				case SC_ScrollBarSubPage:
					if ( horizontal ) return TQRect( subline, 0, sliderpos - subline, ceData.rect.height() );
					else return TQRect( 0, subline, ceData.rect.width(), sliderpos - subline );

				case SC_ScrollBarAddPage:
					if ( horizontal ) return TQRect( sliderpos + sliderlen, 0, ceData.rect.width() - addline  - (sliderpos + sliderlen) , ceData.rect.height() );
					else return TQRect( 0, sliderpos + sliderlen, ceData.rect.width(), ceData.rect.height() - addline - (sliderpos + sliderlen)
                    /*maxlen - sliderpos - sliderlen + subline - 5*/ );

				default: break;
			};
			break;
		}
		case CC_Slider:
		{
			bool horizontal = ceData.orientation == TQt::Horizontal;
			TQSlider::TickSetting ticks = (TQSlider::TickSetting)ceData.tickMarkSetting;
			int pos = ceData.startStep;
			int size = pixelMetric( PM_SliderControlThickness, ceData, elementFlags, widget );
			int handleSize = pixelMetric( PM_SliderThickness, ceData, elementFlags, widget );
			int len = pixelMetric( PM_SliderLength, ceData, elementFlags, widget );

			//Shrink the metrics if the widget is too small
			//to fit our normal values for them.
			if (horizontal)
				handleSize = TQMIN(handleSize, ceData.rect.height());
			else
				handleSize = TQMIN(handleSize, ceData.rect.width());

			size = TQMIN(size, handleSize);

			switch ( subcontrol )
			{
				case SC_SliderGroove:
					if ( horizontal )
					{
						if ( ticks == TQSlider::Both )
							return TQRect( 0, ( ceData.rect.height() - size ) / 2, ceData.rect.width(), size );
						else if ( ticks == TQSlider::Above )
							return TQRect( 0, ceData.rect.height() - size - ( handleSize - size ) / 2, ceData.rect.width(), size );
						return TQRect( 0, ( handleSize - size ) / 2, ceData.rect.width(), size );
					}
					else
					{
						if ( ticks == TQSlider::Both )
							return TQRect( ( ceData.rect.width() - size ) / 2, 0, size, ceData.rect.height() );
						else if ( ticks == TQSlider::Above )
							return TQRect( ceData.rect.width() - size - ( handleSize - size ) / 2, 0, size, ceData.rect.height() );
						return TQRect( ( handleSize - size ) / 2, 0, size, ceData.rect.height() );
					}
				case SC_SliderHandle:
					if ( horizontal )
					{
						if ( ticks == TQSlider::Both )
							return TQRect( pos, ( ceData.rect.height() - handleSize ) / 2, len, handleSize );
						else if ( ticks == TQSlider::Above )
							return TQRect( pos, ceData.rect.height() - handleSize, len, handleSize );
						return TQRect( pos, 0, len, handleSize );
					}
					else
					{
						if ( ticks == TQSlider::Both )
							return TQRect( ( ceData.rect.width() - handleSize ) / 2, pos, handleSize, len );
						else if ( ticks == TQSlider::Above )
							return TQRect( ceData.rect.width() - handleSize, pos, handleSize, len );
						return TQRect( 0, pos, handleSize, len );
					}
				default: break;
			}
			break;
		}
		default: break;
	}
	return TDEStyle::querySubControlMetrics( control, ceData, elementFlags, subcontrol, opt, widget );
}


#include <config.h>

#if !defined TQ_WS_X11 || defined K_WS_QTONLY
#undef HAVE_X11_EXTENSIONS_SHAPE_H
#endif 

#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
//Xlib headers are a mess -> include them down here (any way to ensure that we go second in enable-final order?)
#include <X11/Xlib.h> 
#include <X11/extensions/shape.h> 
#undef   KeyPress
#undef   KeyRelease
#endif

bool KeramikStyle::objectEventHandler( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void* source, TQEvent *event )
{
	if (TDEStyle::objectEventHandler( ceData, elementFlags, source, event ))
		return true;

	if (ceData.widgetObjectTypes.contains("TQObject")) {
		TQObject* object = reinterpret_cast<TQObject*>(source);

		if ( !object->isWidgetType() ) return false;
	
		//Combo line edits get special frames
		if ( event->type() == TQEvent::Paint && ::tqt_cast<TQLineEdit*>(object) )
		{
			static bool recursion = false;
			if (recursion )
				return false;
	
			recursion = true;
			object->event( static_cast<TQPaintEvent*>( event ) );
			TQWidget* widget = static_cast<TQWidget*>( object );
			TQPainter p( widget );
			Keramik::RectTilePainter( keramik_frame_shadow, false, false, 2, 2 ).draw( &p, ceData.rect,
				widget->palette().color( TQPalette::Normal, TQColorGroup::Button ),
				TQt::black, false, Keramik::TilePainter::PaintFullBlend);
			recursion = false;
			return true;
		}
		else if ( ::tqt_cast<TQListBox*>(object) ) 
		{
			//Handle combobox drop downs
			switch (event->type())
			{
#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
				//Combo dropdowns are shaped	
				case TQEvent::Resize:
				{
					TQListBox* listbox = static_cast<TQListBox*>(object);
					TQResizeEvent* resize = static_cast<TQResizeEvent*>(event);
					if (resize->size().height() < 6)
						return false;
			
					//CHECKME: Not sure the rects are perfect..
					XRectangle rects[5] = {
						{0, 0, (unsigned short)(resize->size().width()-2), (unsigned short)(resize->size().height()-6)},
						{0, (short)(resize->size().height()-6), (unsigned short)(resize->size().width()-2), 1},
						{1, (short)(resize->size().height()-5), (unsigned short)(resize->size().width()-3), 1},
						{2, (short)(resize->size().height()-4), (unsigned short)(resize->size().width()-5), 1},
						{3, (short)(resize->size().height()-3), (unsigned short)(resize->size().width()-7), 1}
					};
			
					XShapeCombineRectangles(tqt_xdisplay(), listbox->handle(), ShapeBounding, 0, 0,
						rects, 5, ShapeSet, YXSorted);
				}
				break;
#endif
				//Combo dropdowns get fancy borders
				case TQEvent::Paint:
				{
					static bool recursion = false;
					if (recursion )
						return false;
					TQListBox* listbox = (TQListBox*) object;
					TQPaintEvent* paint = (TQPaintEvent*) event;
			
			
					if ( !listbox->contentsRect().contains( paint->rect() ) )
					{
						TQPainter p( listbox );
						Keramik::RectTilePainter( keramik_combobox_list, false, false ).draw( &p, 0, 0, listbox->width(), listbox->height(),
								listbox->palette().color( TQPalette::Normal, TQColorGroup::Button ),
								listbox->palette().color( TQPalette::Normal, TQColorGroup::Background ) );
			
						TQPaintEvent newpaint( paint->region().intersect( listbox->contentsRect() ), paint->erased() );
						recursion = true;
						object->event( &newpaint );
						recursion = false;
						return true;
					}
				}
				break;
				
				/**
				Since our popup is shown a bit overlapping the combo body, a mouse click at the bottom of the
				widget will result in the release going to the popup, which will cause it to close (#56435). 
				We solve it by filtering out the first release, if it's in the right area. To do this, we notices shows,
				move ourselves to front of event filter list, and then capture the first release event, and if it's
				in the overlap area, filter it out.
				*/
				case TQEvent::Show:
					//Prioritize ourselves to see the mouse events first
					removeObjectEventHandler(ceData, elementFlags, source, this);
					installObjectEventHandler(ceData, elementFlags, source, this);
					firstComboPopupRelease = true;
					break;
				
				//We need to filter some clicks out.
				case TQEvent::MouseButtonRelease:
					if (firstComboPopupRelease)
					{
						firstComboPopupRelease = false;
	
						TQMouseEvent* mev = static_cast<TQMouseEvent*>(event);
						TQListBox*    box = static_cast<TQListBox*>(object);
						
						TQWidget* parent = box->parentWidget();
						if (!parent)
							return false;
						
						TQPoint inParCoords = parent->mapFromGlobal(mev->globalPos());
						if (parent->rect().contains(inParCoords))
							return true;
					}
					break;
				case TQEvent::MouseButtonPress:
				case TQEvent::MouseButtonDblClick:
				case TQEvent::Wheel:
				case TQEvent::KeyPress:
				case TQEvent::KeyRelease:
					firstComboPopupRelease = false;
				default:
					return false;
			}
		}
		//Toolbar background gradient handling
		else if (event->type() == TQEvent::Paint &&
				object->parent() && !qstrcmp(object->name(), kdeToolbarWidget) )
		{
			// Draw a gradient background for custom widgets in the toolbar
			// that have specified a "tde toolbar widget" name.
			renderToolbarWidgetBackground(0, ceData, elementFlags, static_cast<TQWidget*>(object));
	
			return false;	// Now draw the contents
		}
#if 0	// FIXME
		// This does not work on modern systems
		// Rather than resorting to hacks like this, which can stop working at any time, the required functionality should simply be added to TQt3!
		else if (event->type() == TQEvent::Paint  &&  object->parent() && ::tqt_cast<TQToolBar*>(object->parent()) 
				&& !::tqt_cast<TQPopupMenu*>(object) )
		{
			// We need to override the paint event to draw a
			// gradient on a QToolBarExtensionWidget.
			TQToolBar* toolbar = static_cast<TQToolBar*>(object->parent());
			TQWidget* widget = static_cast<TQWidget*>(object);
			TQRect wr = widget->rect (), tr = toolbar->rect();
			TQPainter p( widget );
	
			if ( toolbar->orientation() == TQt::Horizontal )
			{
				Keramik::GradientPainter::renderGradient( &p, wr, widget->colorGroup().button(),
														true /*horizontal*/, false /*not a menu*/,
														0, widget->y(), wr.width(), tr.height());
			}
			else
			{
				Keramik::GradientPainter::renderGradient( &p, wr, widget->colorGroup().button(),
														false /*vertical*/, false /*not a menu*/,
														widget->x(), 0, tr.width(), wr.height());
			}
	
			
			//Draw terminator line, too
			p.setPen( toolbar->colorGroup().mid() );
			if ( toolbar->orientation() == TQt::Horizontal )
				p.drawLine( wr.width()-1, 0, wr.width()-1, wr.height()-1 );
			else
				p.drawLine( 0, wr.height()-1, wr.width()-1, wr.height()-1 );
			return true;
	
		}
#endif
		// Track show events for progress bars
		if ( animateProgressBar && ::tqt_cast<TQProgressBar*>(object) )
		{
			if ((event->type() == TQEvent::Show) && !animationTimer->isActive())
			{
				animationTimer->start( 50, false );
			}
		}
	}

	return false;
}

/*! \reimp */
int KeramikStyle::styleHint(StyleHint sh, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQStyleOption &opt, TQStyleHintReturn *returnData, const TQWidget *w) const
{
	int ret;

	switch (sh) {
		case SH_MenuIndicatorColumnWidth:
			{
				int  checkcol   = opt.maxIconWidth();
				bool checkable = (elementFlags & CEF_IsCheckable);

				if ( checkable )
					checkcol = TQMAX( checkcol, 20 );
			
				ret = checkcol;
			}
			break;
		case SH_ScrollBar_CombineAddLineRegionDrawingAreas:
			ret = 1;
			break;
		default:
			ret = TDEStyle::styleHint(sh, ceData, elementFlags, opt, returnData, w);
			break;
	}

	return ret;
}
