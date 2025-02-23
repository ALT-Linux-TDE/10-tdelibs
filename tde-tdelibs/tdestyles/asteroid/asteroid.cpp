/*
 *	KDE3 asteroid style (version 1.0)
 *
 *	Copyright (C) 2003, Chris Lee <clee@kde.org>
 *	Modified by David Chester in 2004 for Munjoy Linux 
 *
 *	See LICENSE for details about copying.
 */

/* Required. Period. */
#include <tqstyleplugin.h>
#include <tqstylefactory.h>
#include <tqpainter.h>
#include <tqapplication.h>

#include <tqpopupmenu.h>
#include <tqmenubar.h>
#include <tqheader.h>
#include <tqcombobox.h>
#include <tqlistbox.h>
#include <tqslider.h>
#include <tqpushbutton.h>
#include <tqtextedit.h>
#include <tqlineedit.h>
#include <tqtoolbar.h>
#include <tqcheckbox.h>
#include <tqradiobutton.h>
#include <tqprogressbar.h>
#include <tqtabwidget.h>
#include <tqtabbar.h>
#include <tqgroupbox.h>
#include <tqtoolbutton.h>
#include <tqdockwindow.h>
#include <tqtooltip.h>
#include <tqdrawutil.h>
#include <tqlistview.h>
#include <tqcleanuphandler.h>

#include <kpixmap.h>

#include <tqbitmap.h>
#define u_arrow -4,1, 2,1, -3,0, 1,0, -2,-1, 0,-1, -1,-2
#define d_arrow -4,-2, 2,-2, -3,-1, 1,-1, -2,0, 0,0, -1,1
#define l_arrow 0,-3, 0,3, -1,-2, -1,2, -2,-1, -2,1, -3,0
#define r_arrow -2,-3, -2,3, -1,-2, -1,2, 0,-1, 0,1, 1,0

#include "asteroid.h"

#define MINIMUM_PUSHBUTTON_WIDTH 73;
#define MINIMUM_PUSHBUTTON_HEIGHT 21;

#define ETCH_X_OFFSET 1
#define ETCH_Y_OFFSET 1

#define SPINBOX_BUTTON_WIDTH 12

/* Hackery to make metasources work */
#include "asteroid.moc"

/* TQStyleFactory stuff. Required. */
class AsteroidStylePlugin : public TQStylePlugin
{
public:
	AsteroidStylePlugin() {}
	~AsteroidStylePlugin() {}

	TQStringList keys() const {
		return TQStringList() << "Asteroid";
	}

	TQStyle *create(const TQString &key) {
		if (key == "asteroid") {
			return new AsteroidStyle;
		}
		return 0;
	}
};

TQ_EXPORT_PLUGIN(AsteroidStylePlugin);

/* Ok, now we get to the good stuff. */

AsteroidStyle::AsteroidStyle() : TDEStyle(AllowMenuTransparency)
{
	if (tqApp->inherits("TDEApplication")) {
		connect(tqApp, TQ_SIGNAL(tdedisplayPaletteChanged()), TQ_SLOT(paletteChanged()));
	}

	backwards = TQApplication::reverseLayout();
}

AsteroidStyle::~AsteroidStyle()
{
/*	The destructor is empty for now, but any member pointers should
 *	get deleted here. */
}

void AsteroidStyle::polish(const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr)
{
	if (ceData.widgetObjectTypes.contains("TQWidget")) {
		TQWidget *w = reinterpret_cast<TQWidget*>(ptr);
/*		Screwing with the palette is fun! and required in order to make it feel
		authentic. -clee */
		TQPalette wp = w->palette();
		//wp.setColor(TQColorGroup::Dark, wp.active().color(TQColorGroup::Button).dark(350));
		wp.setColor(TQColorGroup::Dark, TQColor(128, 128, 128));
		wp.setColor(TQColorGroup::Mid, wp.active().color(TQColorGroup::Button).dark(150));	// Which GUI element(s) does this correspond to?
	
		bool isProtectedObject = false;
		TQObject *curparent = w;
		while (curparent) {
			if (curparent->inherits("KonqFileTip") || curparent->inherits("AppletItem")
				|| curparent->inherits("KJanusWidget")
				) {
					isProtectedObject = true;
			}
			curparent = curparent->parent();
		}
		if ((w->parent()) && (!w->ownPalette())) {
			isProtectedObject = true;
		}
		if (!isProtectedObject) {
			w->setPalette(wp);
		}
	}

	if (ceData.widgetObjectTypes.contains("TQPushButton")) {
		installObjectEventHandler(ceData, elementFlags, ptr, this);
	}
	else {
		TDEStyle::polish(ceData, elementFlags, ptr);
	}
}

void AsteroidStyle::unPolish(const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr)
{
	TDEStyle::unPolish(ceData, elementFlags, ptr);
}

/*!
    \reimp

    Changes some application-wide settings
*/
void
AsteroidStyle::applicationPolish(const TQStyleControlElementData&, ControlElementFlags, void *)
{
	TQPalette wp = TQApplication::palette();
	wp.setColor(TQColorGroup::Dark, TQColor(128, 128, 128));
	wp.setColor(TQColorGroup::Mid, wp.active().color(TQColorGroup::Button).dark(150));	// Which GUI element(s) does this correspond to?
	TQApplication::setPalette( wp, TRUE );
}

/*! \reimp
*/
void
AsteroidStyle::applicationUnPolish(const TQStyleControlElementData&, ControlElementFlags, void *)
{

}

void AsteroidStyle::renderMenuBlendPixmap(KPixmap &pix,
                                          const TQColorGroup &cg,
                                          const TQPopupMenu *) const
{
	TQPainter p(&pix);

	p.fillRect(0, 0, pix.width(), pix.height(), cg.background());
}

void AsteroidStyle::drawTDEStylePrimitive(TDEStylePrimitive ksp,
                                        TQPainter *p,
                                        const TQStyleControlElementData &ceData,
                                        ControlElementFlags elementFlags,
                                        const TQRect &r,
                                        const TQColorGroup &cg,
                                        SFlags sf,
                                        const TQStyleOption &o,
                                        const TQWidget *w) const
{
	switch (ksp) {

        	case KPE_SliderGroove: {
			int x, y, v, h;
			r.rect(&x, &y, &v, &h);
			bool horizontal = ceData.orientation == TQt::Horizontal;
			int gcenter = (horizontal ? h : v) / 2;
			int pad = 3;

			/*p->setPen(cg.background());
			p->setBrush(cg.background());
			p->drawRect(r);*/

			if (horizontal) {
				gcenter += y;
				p->setPen (cg.background().dark());
				p->drawLine(x+pad, gcenter-1, x+v-pad, gcenter-1);
				p->drawPoint(x+pad, gcenter);
				p->setPen (cg.background().light());
				p->drawLine(x+pad, gcenter+1, x+v-pad, gcenter+1);
				p->drawLine(x+v-pad, gcenter, x+v-pad, gcenter-1);
			} else {
				gcenter += x;
				p->setPen (cg.background().dark());
				p->drawLine(gcenter-1, y+pad, gcenter-1, y+h-pad);
				p->drawPoint(gcenter, y+pad);
				p->setPen (cg.background().light());
				p->drawLine(gcenter+1, y+pad, gcenter+1, y+h-pad);
				p->drawLine(gcenter, y+h-pad, gcenter-1, y+h-pad);
			}
			break;
		}

		case KPE_SliderHandle: {

			int x, y, x2, y2, xmin, xmax, ymin, ymax, v, h;
			int pcenter;
			r.coords(&xmin, &ymin, &xmax, &ymax);
			r.rect(&x, &y, &v, &h);
			bool horizontal = ceData.orientation == TQt::Horizontal;

			if (horizontal) {
				x  = v / 5 + xmin;
				x2 = v * 4 / 5 + xmin;
				if (((x2 - x) % 2)) x2--;
				y2 = ymax - (x2 - x - 1) / 2;
				y  = ymax - y2 + ymin;
				pcenter = (x2 - x) / 2 + x;

				p->setPen(cg.background());
				p->setBrush(cg.background());
				TQRect hr(x-1, y, x2-x+2, y2-y);
				p->drawRect(hr);

				p->setPen(cg.light());
				p->drawLine(x, y, x2-1, y);
				p->drawLine(x, y, x, y2);
				p->drawLine(x, y2, pcenter-1, ymax);

				p->setPen(cg.mid());
				p->drawLine(x2-1, y+1, x2-1, y2);
				p->drawLine(x2-1, y2, pcenter, ymax);

				p->setPen(cg.dark());
				p->drawLine(x2, y, x2, y2);
				p->drawLine(x2, y2, pcenter+1, ymax);

			}
			else {
				y  = h / 5 + ymin;
				y2 = h * 4 / 5 + ymin;
				if (((y2 - y) % 2)) y2--;
				x2  = xmax - (y2 - y - 1) / 2;
				x  = (xmax-x2) + xmin;
				pcenter = (y2 - y)/2 + y;

				p->setPen(cg.background());
				p->setBrush(cg.background());
				TQRect hr(x, y-1, x2-x, y2-y+2);
				p->drawRect(hr);

				p->setPen(cg.light());
				p->drawLine(x, y, x2-1, y);
				p->drawLine(x, y, x, y2-1);
				p->drawLine(x2, y, xmax, pcenter-1);

				p->setPen(cg.mid());
				p->drawLine(x2-1, y2-1, x+1, y2-1);
				p->drawLine(x2, y2-1, xmax, pcenter);

				p->setPen(cg.dark());
				p->drawLine(x, y2, x2, y2);
				p->drawLine(x2, y2, xmax, pcenter+1);

			}

			break;
        	}

		default: {
			TDEStyle::drawTDEStylePrimitive(ksp, p, ceData, elementFlags, r, cg, sf, o, w);
		}
	}
}

int AsteroidStyle::styleHint( StyleHint stylehint,
                                   const TQStyleControlElementData &ceData,
                                   ControlElementFlags elementFlags,
                                   const TQStyleOption &option,
                                   TQStyleHintReturn* returnData,
                                   const TQWidget *widget ) const
{
	switch (stylehint) {
		case SH_EtchDisabledText:
// 		case SH_Slider_SnapToValue:
// 		case SH_PrintDialog_RightAlignButtons:
// 		case SH_MainWindow_SpaceBelowMenuBar:
// 		case SH_FontDialog_SelectAssociatedText:
// 		case SH_PopupMenu_AllowActiveAndDisabled:
// 		case SH_MenuBar_AltKeyNavigation:
// 		case SH_MenuBar_MouseTracking:
// 		case SH_PopupMenu_MouseTracking:
// 		case SH_ComboBox_ListMouseTracking:
// 		case SH_ScrollBar_StopMouseOverSlider:
			return 1;

		case SH_MenuIndicatorColumnWidth: {
			return TQMAX(option.maxIconWidth(), 12);
		}
		
		default:
			return TDEStyle::styleHint(stylehint, ceData, elementFlags, option, returnData, widget);
	}
}

void AsteroidStyle::drawPrimitive(PrimitiveElement pe,
                                  TQPainter *p,
                                  const TQStyleControlElementData &ceData,
                                  ControlElementFlags elementFlags,
                                  const TQRect &r,
                                  const TQColorGroup &cg,
                                  SFlags sf,
                                  const TQStyleOption &o) const
{
	int x, y, x2, y2, w, h;
	r.coords(&x, &y, &x2, &y2);
	r.rect(&x, &y, &w, &h);

	switch (pe) {
	/*	Primitives to draw are:

		PE_ButtonCommand
		PE_ButtonDropDown

		PE_DockWindowSeparator
		PE_DockWindowResizeHandle

		PE_Splitter

		PE_PanelGroupBox
		PE_PanelTabWidget
		PE_TabBarBase

		PE_ProgressBarChunk
		PE_GroupBoxFrame
		PE_WindowFrame
		PE_SizeGrip

		PE_CheckMark
		PE_CheckListController
		PE_CheckListIndicator
		PE_CheckListExclusiveIndicator

		PE_ScrollBarFirst
		PE_ScrollBarLast
	 */

		case PE_Splitter:
		{
			TQPen oldPen = p->pen();
			p->setPen(cg.background());
			p->drawRect(r);
			p->setPen( cg.light() );
// 			if ( sf & Style_Horizontal ) {
// 				p->drawLine( r.x() + 1, r.y(), r.x() + 1, r.height() );
// 				p->setPen( cg.dark() );
// 				p->drawLine( r.x(), r.y(), r.x(), r.height() );
// 				p->drawLine( r.right()-1, r.y(), r.right()-1, r.height() );
// 				p->setPen( cg.shadow() );
// 				p->drawLine( r.right(), r.y(), r.right(), r.height() );
// 			} else {
// 				p->drawLine( r.x(), r.y() + 1, r.width(), r.y() + 1 );
// 				p->setPen( cg.dark() );
// 				p->drawLine( r.x(), r.bottom() - 1, r.width(), r.bottom() - 1 );
// 				p->setPen( cg.shadow() );
// 				p->drawLine( r.x(), r.bottom(), r.width(), r.bottom() );
// 			}
			p->setPen( oldPen );
			break;
		}

		case PE_FocusRect: {
			p->drawWinFocusRect(r, cg.background());
			break;
		}

		// FIXME
		// This appears to do double duty,
		// specifically it appears both in popup menu headers
		// *and* at the top of tree views!
		// The tree views need the stuff that is commented out
		// to look correct, but when that is done the popup menus
		// look absolutely HORRIBLE.
		// How can we tell the two apart?  Create PE_HeaderSectionMenu perhaps?
		case PE_HeaderSection: {
			p->setPen(cg.shadow());
			p->setBrush(cg.background());
			p->drawRect(r);

			if (sf & Style_On) {
				p->setPen(cg.mid());
				p->setBrush(TQBrush(cg.light(),TQt::Dense4Pattern));
				p->drawRect(r);
				p->setPen(cg.buttonText());
			} else if (sf & Style_Down) {
				p->setPen(cg.mid());
				p->drawRect(r);
				p->setPen(cg.buttonText());
			} else {
				p->setPen(cg.light());
				p->drawLine(x, y, x2-1, y);
				p->drawLine(x, y, x, y2-1);

				p->setPen(cg.mid());
				p->drawLine(x2-1, y2-1, x+1, y2-1);
				p->drawLine(x2-1, y2-1, x2-1, y+1);
			}

			break;
		}

		case PE_HeaderSectionMenu: {
			p->setPen(cg.shadow());
			p->setBrush(cg.background());
			p->drawRect(r);

// 			if (sf & Style_On) {
// 				p->setPen(cg.mid());
// 				p->setBrush(TQBrush(cg.light(),TQt::Dense4Pattern));
// 				p->drawRect(r);
// 				p->setPen(cg.buttonText());
// 			} else if (sf & Style_Down) {
				p->setPen(cg.mid());
				p->drawRect(r);
				p->setPen(cg.buttonText());
// 			} else {
// 				p->setPen(cg.light());
// 				p->drawLine(x, y, x2-1, y);
// 				p->drawLine(x, y, x, y2-1);
// 
// 				p->setPen(cg.mid());
// 				p->drawLine(x2-1, y2-1, x+1, y2-1);
// 				p->drawLine(x2-1, y2-1, x2-1, y+1);
// 			}

			break;
		}

		case PE_ButtonBevel: {
			p->setPen(cg.shadow());
			p->setBrush(cg.background());
			p->drawRect(r);

			if (sf & Style_On) {
				p->setPen(cg.mid());
				p->setBrush(TQBrush(cg.light(),TQt::Dense4Pattern));
				p->drawRect(r);

				if (!(sf & Style_ButtonDefault)) {
					p->setPen(cg.shadow());
					p->drawLine(x, y, x2-1, y);
					p->drawLine(x, y, x, y2-1);
	
					p->setPen(cg.dark());
					p->drawLine(x+1, y+1, x2-2, y+1);
					p->drawLine(x+1, y+1, x+1, y2-2);
	
					p->setPen(cg.light());
					p->drawLine(x, y2, x2, y2);
					p->drawLine(x2, y, x2, y2);
	
					p->setPen(cg.background());
					p->drawLine(x2-1, y2-1, x+1, y2-1);
					p->drawLine(x2-1, y2-1, x2-1, y+1);
				}

				p->setPen(cg.buttonText());
			} else if (sf & Style_Down) {
				p->setPen(cg.mid());
				p->drawRect(r);

				if (!(sf & Style_ButtonDefault)) {
					p->setPen(cg.shadow());
					p->drawLine(x, y, x2-1, y);
					p->drawLine(x, y, x, y2-1);
	
					p->setPen(cg.dark());
					p->drawLine(x+1, y+1, x2-2, y+1);
					p->drawLine(x+1, y+1, x+1, y2-2);
	
					p->setPen(cg.light());
					p->drawLine(x, y2, x2, y2);
					p->drawLine(x2, y, x2, y2);
	
					p->setPen(cg.background());
					p->drawLine(x2-1, y2-1, x+1, y2-1);
					p->drawLine(x2-1, y2-1, x2-1, y+1);
				}

				p->setPen(cg.buttonText());
			} else {
				p->setPen(cg.light());
				p->drawLine(x, y, x2-1, y);
				p->drawLine(x, y, x, y2-1);

				p->setPen(cg.mid());
				p->drawLine(x2-1, y2-1, x+1, y2-1);
				p->drawLine(x2-1, y2-1, x2-1, y+1);
			}

			break;
		}

		case PE_ButtonDefault: {
			p->setPen(cg.shadow());
			p->drawRect(r);
			break;
		}

		case PE_ButtonTool: {
			p->setPen(sf & Style_On || sf & Style_Down ? cg.mid() : cg.light());
			p->drawRect(r);
			p->setPen(sf & Style_On || sf & Style_Down ? cg.light() : cg.mid());
			p->drawLine(r.bottomRight(), r.topRight());
			p->drawLine(r.bottomRight(), r.bottomLeft());
			p->setPen(cg.buttonText());
			break;
		}

		case PE_ScrollBarSlider: {
			if (sf & Style_Enabled) {
				p->fillRect(r, cg.background());

				p->setPen(cg.light());
				p->drawLine(x+1, y+1, x2-2, y+1);
				p->drawLine(x+1, y+1, x+1, y2-2);

				p->setPen(cg.mid());
				p->drawLine(x2-1, y2-1, x+1, y2-1);
				p->drawLine(x2-1, y2-1, x2-1, y+1);

				p->setPen(cg.dark());
				p->drawLine(x2, y2, x, y2);
				p->drawLine(x2, y2, x2, y);
			} else {
				p->fillRect(r, cg.background());
				p->fillRect(r, TQBrush(cg.light(), Dense4Pattern));
			}

			break;
		}

		case PE_StatusBarSection: {
			p->fillRect(r, cg.background());
			p->setPen(cg.mid());
			p->drawLine(x, y, x2-1, y);
			p->drawLine(x, y, x, y2-1);

			p->setPen(cg.light());
			p->drawLine(x2, y2, x, y2);
			p->drawLine(x2, y2, x2, y);

			break;
		}

		case PE_CheckMark: {
			int x = r.center().x() - 3, y = r.center().y() - 3;
			const TQCOORD check[] = { x, y + 2, x, y + 4, x + 2, y + 6, x + 6, y + 2, x + 6, y, x + 2, y + 4 };
			const TQPointArray a(6, check);

			p->setPen(cg.text());
			p->setBrush(cg.text());
			p->drawPolygon(a);

			break;
		}

		case PE_Indicator: { 
			p->setPen(cg.mid());
			p->setBrush(sf & Style_Down ? cg.background() : cg.base());
			p->drawRect(r);

			p->setPen(cg.mid());
			p->drawLine(x, y, x2, y);
			p->drawLine(x, y, x, y2);

			p->setPen(cg.dark());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1, y2-1);

			p->setPen(cg.background());
			p->drawLine(x2-1, y2-1, x2-1, y+1);
			p->drawLine(x2-1, y2-1, x+1, y2-1);

			p->setPen(cg.light());
			p->drawLine(x2, y2, x, y2);
			p->drawLine(x2, y2, x2, y);
			break;
		}

		case PE_IndicatorMask: { 
			p->fillRect (r, color1);

			break;
		}


		case PE_ExclusiveIndicator: {
			const TQCOORD outside[] = { 1, 9, 1, 8, 0, 7, 0, 4, 1, 3, 1, 2, 2, 1, 3, 1, 4, 0, 7, 0, 8, 1, 9, 1, 10, 2, 10, 3, 11, 4, 11, 7, 10, 8, 10, 9, 9, 10, 8, 10, 7, 11, 4, 11, 3, 10, 2, 10 };
			const TQCOORD inside[] = { 2, 8, 1, 7, 1, 4, 2, 3, 2, 2, 3, 2, 4, 1, 7, 1, 8, 2, 9, 2, 9, 3, 10, 4, 10, 7, 9, 8, 9, 9, 8, 9, 7, 10, 4, 10, 3, 9, 2, 9 }; 
			p->fillRect(r, cg.background());
			if (sf & Style_Enabled && !(sf & Style_Down)) {
				p->setPen(TQPen::NoPen);
				p->setBrush(cg.base());
				p->drawPolygon(TQPointArray(24, outside));
			}
			p->setPen(cg.mid());
			p->drawPolyline(TQPointArray(24, outside), 0, 12);
			p->setPen(cg.light());
			p->drawPolyline(TQPointArray(24, outside), 12, 12);
			p->setPen(cg.dark());
			p->drawPolyline(TQPointArray(20, inside), 0, 10);
			p->setPen(cg.background());
			p->drawPolyline(TQPointArray(20, inside), 10, 10);
			break;
		}

		case PE_ExclusiveIndicatorMask: {
			const TQCOORD outside[] = { 1, 9, 1, 8, 0, 7, 0, 4, 1, 3, 1, 2, 2, 1, 3, 1, 4, 0, 7, 0, 8, 1, 9, 1, 10, 2, 10, 3, 11, 4, 11, 7, 10, 8, 10, 9, 9, 10, 8, 10, 7, 11, 4, 11, 3, 10, 2, 10 };
			p->fillRect(r, color0);
			p->setPen(color1);
			p->setBrush(color1);
			p->drawPolygon(TQPointArray(24, outside));
			break;
		}

		case PE_WindowFrame:
		case PE_Panel: {
			bool sunken = sf & Style_Sunken;

			p->setPen(sunken ? cg.mid() : cg.light());
			p->drawLine(x, y, x2-1, y);
			p->drawLine(x, y, x, y2-1);

			p->setPen(sunken ? cg.dark() : cg.background());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1, y2-1);

			p->setPen(sunken ? cg.light() : cg.mid());
			p->drawLine(x2-1, y2-1, x+1, y2-1);
			p->drawLine(x2-1, y2-1, x2-1, y+1);

			p->setPen(sunken ? cg.background() : cg.dark());
			p->drawLine(x2, y2, x, y2);
			p->drawLine(x2, y2, x2, y);

			break;
		}

		case PE_PanelLineEdit: {
			p->fillRect(r, cg.base());

			p->setPen(cg.light());
			p->drawLine(x2, y2, x, y2);
			p->drawLine(x2, y2, x2, y);

			p->setPen(cg.background());
			p->drawLine(x2-1, y2-1, x, y2-1);
			p->drawLine(x2-1, y2-1, x2-1, y);

			p->setPen(cg.mid());
			p->drawLine(x, y, x2, y);
			p->drawLine(x, y, x, y2);

			p->setPen(cg.dark());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1, y2-1);

			break;
		}

		case PE_PanelDockWindow: {
			p->setPen(cg.mid());
			p->drawLine(r.bottomLeft(), r.bottomRight());
			p->setPen(cg.light());
			p->drawLine(r.topLeft(), r.topRight());
			break;
		}

		case PE_PanelMenuBar: {
			p->setPen(cg.mid());
			p->drawLine(r.bottomLeft(), r.bottomRight());
			break;
		}

		case PE_ScrollBarAddPage:
		case PE_ScrollBarSubPage: {
			if (sf & Style_On || sf & Style_Down) {
				p->fillRect(r, cg.mid().dark());
			} else {
				p->fillRect(r, cg.background());
			}
			p->fillRect(r, TQBrush(cg.light(), Dense4Pattern));
			break;
		}

		case PE_ScrollBarAddLine: {
			p->fillRect(r, cg.background());

			if (sf & Style_Down) {
				p->setPen(cg.mid());
				p->drawRect(r);
			} else {
				p->setPen(cg.light());
				p->drawLine(x+1, y+1, x2-2, y+1);
				p->drawLine(x+1, y+1, x+1, y2-2);

				p->setPen(cg.mid());
				p->drawLine(x2-1, y2-1, x+1, y2-1);
				p->drawLine(x2-1, y2-1, x2-1, y+1);

				p->setPen(cg.dark());
				p->drawLine(x2, y2, x, y2);
				p->drawLine(x2, y2, x2, y);
			}

			p->setPen(cg.foreground());
			if (sf & Style_Enabled) {
				drawPrimitive(sf & Style_Horizontal ? PE_ArrowRight : PE_ArrowDown, p, ceData, elementFlags, r, cg, sf);
			} else {
				TQPen oldPen = p->pen();
				p->setPen(cg.light());
				drawPrimitive(sf & Style_Horizontal ? PE_ArrowRight : PE_ArrowDown, p, ceData, elementFlags, TQRect(x+1, y+1, w, h), cg, sf);
				p->setPen(cg.dark());
				drawPrimitive(sf & Style_Horizontal ? PE_ArrowRight : PE_ArrowDown, p, ceData, elementFlags, r, cg, sf);
				p->setPen(oldPen);
			}
			break;
		}

		case PE_ScrollBarSubLine: {
			p->fillRect(r, cg.background());

			if (sf & Style_Down) {
				p->setPen(cg.mid());
				p->drawRect(r);
			} else {
				p->setPen(cg.light());
				p->drawLine(x+1, y+1, x2-2, y+1);
				p->drawLine(x+1, y+1, x+1, y2-2);

				p->setPen(cg.mid());
				p->drawLine(x2-1, y2-1, x+1, y2-1);
				p->drawLine(x2-1, y2-1, x2-1, y+1);

				p->setPen(cg.dark());
				p->drawLine(x2, y2, x, y2);
				p->drawLine(x2, y2, x2, y);
			}

			p->setPen(cg.foreground());
			if (sf & Style_Enabled) {
				drawPrimitive(sf & Style_Horizontal ? PE_ArrowLeft : PE_ArrowUp, p, ceData, elementFlags, r, cg, sf);
			} else {
				TQPen oldPen = p->pen();
				p->setPen(cg.light());
				drawPrimitive(sf & Style_Horizontal ? PE_ArrowLeft : PE_ArrowUp, p, ceData, elementFlags, TQRect(x+1, y+1, w, h), cg, sf);
				p->setPen(cg.dark());
				drawPrimitive(sf & Style_Horizontal ? PE_ArrowLeft : PE_ArrowUp, p, ceData, elementFlags, r, cg, sf);
				p->setPen(oldPen);
			}
			break;
		}

		case PE_DockWindowHandle: {
			p->setPen(cg.light());

			if (sf & Style_Horizontal) {
			TQRect hr(0, 0, 3, r.height()-6);
			hr.moveCenter(r.center());

			p->fillRect(r, cg.background());
			p->drawRect(hr);
			p->setPen(cg.mid());
			p->drawLine(hr.bottomRight(), hr.topRight());
			p->drawLine(hr.bottomRight(), hr.bottomLeft());
			} else {
				TQRect hr(0, 0, r.width()-6, 3);
				hr.moveCenter(r.center());

				p->fillRect(r, cg.background());
				p->drawRect(hr);
				p->setPen(cg.mid());
				p->drawLine(hr.bottomLeft(), hr.bottomRight());
				p->drawLine(hr.topRight(), hr.bottomRight());
			}

			break;
		}

		case PE_Separator:
			p->fillRect(r, cg.background());

			if (!(sf & Style_Horizontal)) {
				p->setPen(cg.background());
				p->drawLine(x + 2, y , x2 - 2, y);
				p->setPen(cg.light());
				p->drawLine(x + 2, y + 1, x2 - 2, y + 1);
			}
			else
			{
				p->setPen(cg.background());
				p->drawLine(x + 2, y + 2, x + 2, y2 - 2);
				p->setPen(cg.light());
				p->drawLine(x + 3, y + 2, x + 3, y2 - 2);
			}
			break;

		case PE_DockWindowSeparator: {
			p->fillRect(r, cg.background());

			if (!(sf & Style_Horizontal)) {
				p->setPen(cg.mid());
				p->drawLine(x + 2, y , x2 - 2, y);
				p->setPen(cg.light());
				p->drawLine(x + 2, y + 1, x2 - 2, y + 1);
			}
			else
			{
				p->setPen(cg.mid());
				p->drawLine(x + 2, y + 2, x + 2, y2 - 2);
				p->setPen(cg.light());
				p->drawLine(x + 3, y + 2, x + 3, y2 - 2);
			}
			break;
		}

		case PE_DockWindowResizeHandle: {
			p->fillRect(r, cg.mid());
			break;
		}

		case PE_PanelPopup: {
			p->setPen(cg.background());
			p->setBrush(cg.background());

			int menuborder = 3;
			p->drawRect(r.x(), r.y(), r.x()+menuborder, r.height());			// Left
			p->drawRect(r.x()+r.width()-menuborder, r.y(), r.width(), r.height());		// Right
			p->drawRect(r.x(), r.y(), r.width(), r.y()+menuborder);				// Top
			p->drawRect(r.x(), r.y()+r.height()-menuborder, r.width(), r.height());		// Bottom

			p->setPen(cg.dark());
			p->drawLine(x2, y2, x, y2);
			p->drawLine(x2, y2, x2, y);

			p->setPen(cg.light());
			p->drawLine(x+1, y+1, x+1, y2-1);
			p->drawLine(x+1, y+1, x2-1, y+1);

			p->setPen(cg.mid());
			p->drawLine(x2-1, y2-1, x+1, y2-1);
			p->drawLine(x2-1, y2-1, x2-1, y+1);

			break;
		}

		case PE_SpinWidgetUp:
		case PE_SpinWidgetDown:
		case PE_HeaderArrow:
		case PE_ArrowUp:
		case PE_ArrowDown:
		case PE_ArrowLeft:
		case PE_ArrowRight: {
			TQPointArray a;

			switch (pe) {

				case PE_SpinWidgetUp:
				case PE_ArrowUp: {
					a.setPoints(7, u_arrow);
					break;
				}

				case PE_SpinWidgetDown:
				case PE_ArrowDown: {
					a.setPoints(7, d_arrow);
					break;
				}

				case PE_ArrowLeft: {
					a.setPoints(7, l_arrow);
					break;
				}

				case PE_ArrowRight: {
					a.setPoints(7, r_arrow);
					break;
				}

				default: {
					if (sf & Style_Up) {
						a.setPoints(7, u_arrow);
					} else {
						a.setPoints(7, d_arrow);
					}
				}
			}

			if (p->pen() == TQt::NoPen) {
				p->setPen(sf & Style_Enabled ? cg.foreground() : cg.light());
			}

			if (sf & Style_Down) {
				p->translate(pixelMetric(PM_ButtonShiftHorizontal, ceData, elementFlags),
				             pixelMetric(PM_ButtonShiftVertical, ceData, elementFlags));
			}

			a.translate((x + w/2), (y + (h-1)/2));
			p->drawLineSegments(a, 0, 3);
			p->drawPoint(a[6]);

			if (sf & Style_Down) {
				p->translate(-pixelMetric(PM_ButtonShiftHorizontal, ceData, elementFlags),
				             -pixelMetric(PM_ButtonShiftVertical, ceData, elementFlags));
			}

			break;
		}

		case PE_MenuItemIndicatorIconFrame:
		case PE_MenuItemIndicatorFrame: {
			// Draw nothing
			break;
		}

		case PE_MenuItemIndicatorCheck: {
			int x, y, w, h;
			r.rect( &x, &y, &w, &h );
			int checkcol = styleHint(SH_MenuIndicatorColumnWidth, ceData, elementFlags, o, NULL, NULL);

			bool active = sf & Style_Active;
			bool disabled = !(sf & Style_Enabled);
		
			int xp = x;

			SFlags cflags = Style_Default;
			if (disabled) {
				cflags |= Style_On;
			} else {
				cflags |= Style_Enabled;
			}

			p->setPen(active ? cg.highlightedText() : cg.buttonText());

			TQRect rr = TQRect(xp, y, checkcol, h);
			if (backwards) {
				rr = visualRect(rr, r);
			}

			drawPrimitive(PE_CheckMark, p, ceData, elementFlags, rr, cg, cflags);

			break;
		}

		default: {
			TDEStyle::drawPrimitive(pe, p, ceData, elementFlags, r, cg, sf, o);
		}
	}
}

void AsteroidStyle::drawControl(ControlElement ce,
                                TQPainter *p,
                                const TQStyleControlElementData &ceData,
                                ControlElementFlags elementFlags,
                                const TQRect &r,
                                const TQColorGroup &cg,
                                SFlags sf,
                                const TQStyleOption &o,
                                const TQWidget *w) const
{
	int x, y, x2, y2, sw, sh;
	r.coords(&x, &y, &x2, &y2);
	r.rect(&x, &y, &sw, &sh);

	switch (ce) {
	/*	TQ_ControlElements to draw are:

		CE_CheckBoxLabel
		CE_RadioButtonLabel

		CE_TabBarTab
		CE_TabBarLabel

		CE_ProgressBarGroove
		CE_ProgressBarContents
		CE_ProgressBarLabel

		CE_PopupMenuScroller
		CE_PopupMenuHorizontalExtra
		CE_PopupMenuVerticalExtra
		CE_MenuBarEmptyArea
		CE_DockWindowEmptyArea

		CE_ToolButtonLabel
		CE_ToolBoxTab

	 */

#ifndef TQT_NO_TABBAR
	case CE_TabBarTab:
		{
		if ( ceData.parentWidgetData.widgetObjectTypes.isEmpty() || !o.tab() )
			break;
	
		const TQTab * t = o.tab();
		bool selected = sf & Style_Selected;
		bool lastTab = (ceData.tabBarData.identIndexMap[t->identifier()] == ceData.tabBarData.tabCount-1) ?
				TRUE : FALSE;
		TQRect r2( r );
		if ( ceData.tabBarData.shape == TQTabBar::RoundedAbove ) {
			p->setPen( cg.light() );
			p->drawLine( r2.left(), r2.bottom()-1, r2.right(), r2.bottom()-1 );
			if ( r2.left() == 0 )
			p->drawPoint( ceData.rect.bottomLeft() );
	
			if ( selected ) {
			p->fillRect( TQRect( r2.left()+1, r2.bottom()-1, r2.width()-3, 2),
					cg.brush( TQColorGroup::Background ));
			p->setPen( cg.background() );
			p->drawLine( r2.left()+1, r2.bottom(), r2.left()+1, r2.top()+2 );
			p->setPen( cg.light() );
			} else {
			p->setPen( cg.light() );
			r2.setRect( r2.left() + 2, r2.top() + 2,
				r2.width() - 4, r2.height() - 2 );
			}
	
			int x1, x2;
			x1 = r2.left();
			x2 = r2.right() - 2;
			p->drawLine( x1, r2.bottom()-1, x1, r2.top() + 2 );
			x1++;
			p->drawPoint( x1, r2.top() + 1 );
			x1++;
			p->drawLine( x1, r2.top(), x2, r2.top() );
			x1 = r2.left();
	
			p->setPen( cg.dark() );
			x2 = r2.right() - 1;
			p->drawLine( x2, r2.top() + 2, x2, r2.bottom() - 1 +
				(selected ? 0:-1) );
			p->setPen( cg.shadow() );
			p->drawPoint( x2, r2.top() + 1 );
			p->drawPoint( x2, r2.top() + 1 );
			x2++;
			p->drawLine( x2, r2.top() + 2, x2, r2.bottom() -
				(selected ? (lastTab ? 0:1) :2));
		} else if ( ceData.tabBarData.shape == TQTabBar::RoundedBelow ) {
			bool rightAligned = styleHint( SH_TabBar_Alignment, ceData, elementFlags, TQStyleOption::Default, 0, w ) == TQt::AlignRight;
			bool firstTab = ceData.tabBarData.identIndexMap[t->identifier()] == 0;
			if ( selected ) {
			p->fillRect( TQRect( r2.left()+1, r2.top(), r2.width()-3, 1),
					cg.brush( TQColorGroup::Background ));
			p->setPen( cg.background() );
			p->drawLine( r2.left()+1, r2.top(), r2.left()+1, r2.bottom()-2 );
			p->setPen( cg.dark() );
			} else {
			p->setPen( cg.shadow() );
			p->drawLine( r2.left() +
					(rightAligned && firstTab ? 0 : 1),
					r2.top() + 1,
					r2.right() - (lastTab ? 0 : 2),
					r2.top() + 1 );
	
			if ( rightAligned && lastTab )
				p->drawPoint( r2.right(), r2.top() );
			p->setPen( cg.dark() );
			p->drawLine( r2.left(), r2.top(), r2.right() - 1,
					r2.top() );
			r2.setRect( r2.left() + 2, r2.top(),
					r2.width() - 4, r2.height() - 2 );
			}
	
			p->drawLine( r2.right() - 1, r2.top() + (selected ? 0: 2),
				r2.right() - 1, r2.bottom() - 2 );
			p->drawPoint( r2.right() - 2, r2.bottom() - 2 );
			p->drawLine( r2.right() - 2, r2.bottom() - 1,
				r2.left() + 1, r2.bottom() - 1 );
	
			p->setPen( cg.shadow() );
			p->drawLine( r2.right(),
				r2.top() + (lastTab && rightAligned &&
						selected) ? 0 : 1,
				r2.right(), r2.bottom() - 1 );
			p->drawPoint( r2.right() - 1, r2.bottom() - 1 );
			p->drawLine( r2.right() - 1, r2.bottom(),
				r2.left() + 2, r2.bottom() );
	
			p->setPen( cg.light() );
			p->drawLine( r2.left(), r2.top() + (selected ? 0 : 2),
				r2.left(), r2.bottom() - 2 );
		} else {
			TQCommonStyle::drawControl(ce, p, ceData, elementFlags, r, cg, sf, o, w);
		}
		break;
		}

	case CE_TabBarLabel:
		{
		if ( o.isDefault() )
			break;
	
		TQTab * t = o.tab();

		const bool enabled = sf & Style_Enabled;
		bool etchtext = styleHint( SH_EtchDisabledText, ceData, elementFlags );
	
		TQRect tr = r;
		if ( t->identifier() == ceData.tabBarData.currentTabIndex )
			tr.setBottom( tr.bottom() -
				pixelMetric( TQStyle::PM_DefaultFrameWidth, ceData, elementFlags, w ) );
	
		int alignment = TQt::AlignCenter | TQt::ShowPrefix;
		if ((!styleHint(SH_UnderlineAccelerator, ceData, elementFlags, TQStyleOption::Default, 0, w)) || ((styleHint(SH_HideUnderlineAcceleratorWhenAltUp, ceData, elementFlags, TQStyleOption::Default, 0, w)) && (!acceleratorsShown())))
			alignment |= TQt::NoAccel;
		tr.setWidth(tr.width()+4);	// Compensate for text appearing too far to the left
//		TQRect tr_offset = TQRect(tr.x()+ETCH_X_OFFSET, tr.y()+ETCH_Y_OFFSET, tr.width(), tr.height());
		TQRect tr_offset = TQRect(tr.x()+0, tr.y()+0, tr.width(), tr.height());
		if ( etchtext && !enabled ) {
			TQPen savePen = p->pen();
			p->setPen( cg.light() );
			TQColorGroup etchedcg = cg;
			etchedcg.setColor( TQColorGroup::Text, cg.light() );
			etchedcg.setColor( TQColorGroup::Mid, cg.light() );
			etchedcg.setColor( TQColorGroup::Midlight, cg.light() );
			etchedcg.setColor( TQColorGroup::Foreground, cg.light() );
			etchedcg.setColor( TQColorGroup::HighlightedText, cg.light() );
			etchedcg.setColor( TQColorGroup::BrightText, cg.light() );
			etchedcg.setColor( TQColorGroup::ButtonText, cg.light() );
			drawItem( p, tr_offset, alignment, etchedcg, enabled, 0, t->text() );
			p->setPen( cg.dark() );
			etchedcg.setColor( TQColorGroup::Text, cg.dark() );
			etchedcg.setColor( TQColorGroup::Mid, cg.dark() );
			etchedcg.setColor( TQColorGroup::Midlight, cg.dark() );
			etchedcg.setColor( TQColorGroup::Foreground, cg.dark() );
			etchedcg.setColor( TQColorGroup::HighlightedText, cg.dark() );
			etchedcg.setColor( TQColorGroup::BrightText, cg.dark() );
			etchedcg.setColor( TQColorGroup::ButtonText, cg.dark() );
			drawItem( p, tr, alignment, etchedcg, enabled, 0, t->text() );
			p->setPen(savePen);
		}
		else {
			drawItem( p, tr, alignment, cg, enabled, 0, t->text() );
		}
	
		if ( (sf & Style_HasFocus) && !t->text().isEmpty() )
			drawPrimitive( TQStyle::PE_FocusRect, p, ceData, elementFlags, r, cg );
		break;
		}
#endif // TQT_NO_TABBAR

	case CE_CheckBoxLabel:
	{
#ifndef TQT_NO_CHECKBOX
		const TQCheckBox *checkbox = (const TQCheckBox *) w;

		const bool enabled = sf & Style_Enabled;
		bool etchtext = styleHint( SH_EtchDisabledText, ceData, elementFlags );
	
		int alignment = TQApplication::reverseLayout() ? TQt::AlignRight : TQt::AlignLeft;
		if ((!styleHint(SH_UnderlineAccelerator, ceData, elementFlags, TQStyleOption::Default, 0, w)) || ((styleHint(SH_HideUnderlineAcceleratorWhenAltUp, ceData, elementFlags, TQStyleOption::Default, 0, w)) && (!acceleratorsShown())))
			alignment |= TQt::NoAccel;

		//TQRect r_offset = TQRect(r.x()+ETCH_X_OFFSET, r.y()+ETCH_Y_OFFSET, r.width(), r.height());
		TQRect r_offset = TQRect(r.x()+0, r.y()+0, r.width(), r.height());
		if ( etchtext && !enabled ) {
			TQPen savePen = p->pen();
			p->setPen( cg.light() );
			TQColorGroup etchedcg = cg;
			etchedcg.setColor( TQColorGroup::Text, cg.light() );
			etchedcg.setColor( TQColorGroup::Mid, cg.light() );
			etchedcg.setColor( TQColorGroup::Midlight, cg.light() );
			etchedcg.setColor( TQColorGroup::Foreground, cg.light() );
			etchedcg.setColor( TQColorGroup::HighlightedText, cg.light() );
			etchedcg.setColor( TQColorGroup::BrightText, cg.light() );
			etchedcg.setColor( TQColorGroup::ButtonText, cg.light() );
			drawItem(p, r_offset, alignment | TQt::AlignVCenter | TQt::ShowPrefix, etchedcg, sf & Style_Enabled, checkbox->pixmap(), checkbox->text());
			p->setPen( cg.dark() );
			etchedcg.setColor( TQColorGroup::Text, cg.dark() );
			etchedcg.setColor( TQColorGroup::Mid, cg.dark() );
			etchedcg.setColor( TQColorGroup::Midlight, cg.dark() );
			etchedcg.setColor( TQColorGroup::Foreground, cg.dark() );
			etchedcg.setColor( TQColorGroup::HighlightedText, cg.dark() );
			etchedcg.setColor( TQColorGroup::BrightText, cg.dark() );
			etchedcg.setColor( TQColorGroup::ButtonText, cg.dark() );
			drawItem(p, r, alignment | TQt::AlignVCenter | TQt::ShowPrefix, etchedcg, sf & Style_Enabled, checkbox->pixmap(), checkbox->text());
			p->setPen(savePen);
		}
		else {
			drawItem(p, r, alignment | TQt::AlignVCenter | TQt::ShowPrefix, cg, sf & Style_Enabled, checkbox->pixmap(), checkbox->text());
		}
	
		if (sf & Style_HasFocus) {
			TQRect fr = visualRect(subRect(SR_CheckBoxFocusRect, ceData, elementFlags, w), ceData, elementFlags);
			drawPrimitive(TQStyle::PE_FocusRect, p, ceData, elementFlags, fr, cg, sf);
		}
#endif
	    break;
	}

	case CE_RadioButtonLabel:
	{
#ifndef TQT_NO_RADIOBUTTON
		const TQRadioButton *radiobutton = (const TQRadioButton *) w;

		const bool enabled = sf & Style_Enabled;
		bool etchtext = styleHint( SH_EtchDisabledText, ceData, elementFlags );

		int alignment = TQApplication::reverseLayout() ? TQt::AlignRight : TQt::AlignLeft;
		if ((!styleHint(SH_UnderlineAccelerator, ceData, elementFlags, TQStyleOption::Default, 0, w)) || ((styleHint(SH_HideUnderlineAcceleratorWhenAltUp, ceData, elementFlags, TQStyleOption::Default, 0, w)) && (!acceleratorsShown())))
			alignment |= TQt::NoAccel;

//		TQRect r_offset = TQRect(r.x()+ETCH_X_OFFSET, r.y()+ETCH_Y_OFFSET, r.width(), r.height());
		TQRect r_offset = TQRect(r.x()+0, r.y()+0, r.width(), r.height());
		if ( etchtext && !enabled ) {
			TQPen savePen = p->pen();
			p->setPen( cg.light() );
			TQColorGroup etchedcg = cg;
			etchedcg.setColor( TQColorGroup::Text, cg.light() );
			etchedcg.setColor( TQColorGroup::Mid, cg.light() );
			etchedcg.setColor( TQColorGroup::Midlight, cg.light() );
			etchedcg.setColor( TQColorGroup::Foreground, cg.light() );
			etchedcg.setColor( TQColorGroup::HighlightedText, cg.light() );
			etchedcg.setColor( TQColorGroup::BrightText, cg.light() );
			etchedcg.setColor( TQColorGroup::ButtonText, cg.light() );
			drawItem(p, r_offset, alignment | TQt::AlignVCenter | TQt::ShowPrefix, etchedcg, enabled, radiobutton->pixmap(), radiobutton->text());
			p->setPen( cg.dark() );
			etchedcg.setColor( TQColorGroup::Text, cg.dark() );
			etchedcg.setColor( TQColorGroup::Mid, cg.dark() );
			etchedcg.setColor( TQColorGroup::Midlight, cg.dark() );
			etchedcg.setColor( TQColorGroup::Foreground, cg.dark() );
			etchedcg.setColor( TQColorGroup::HighlightedText, cg.dark() );
			etchedcg.setColor( TQColorGroup::BrightText, cg.dark() );
			etchedcg.setColor( TQColorGroup::ButtonText, cg.dark() );
			drawItem(p, r, alignment | TQt::AlignVCenter | TQt::ShowPrefix, etchedcg, enabled, radiobutton->pixmap(), radiobutton->text());
			p->setPen(savePen);
		}
		drawItem(p, r, alignment | TQt::AlignVCenter | TQt::ShowPrefix, cg, enabled, radiobutton->pixmap(), radiobutton->text());
	
		if (sf & Style_HasFocus) {
			TQRect fr = visualRect(subRect(SR_RadioButtonFocusRect, ceData, elementFlags, w), ceData, elementFlags);
			drawPrimitive(TQStyle::PE_FocusRect, p, ceData, elementFlags, fr, cg, sf);
		}
#endif
	    break;
	}

	case CE_ToolBoxTab:
		{
		qDrawShadePanel( p, r, cg, sf & (Style_Sunken | Style_Down | Style_On) , 1,
				&cg.brush(TQColorGroup::Button));
		break;
		}

		case CE_ProgressBarContents: {


			// ### Take into account totalSteps() for busy indicator
			TQRect cr = subRect(SR_ProgressBarContents, ceData, elementFlags, w);
			double progress = ceData.currentStep;
			bool reverse = TQApplication::reverseLayout();
			int steps = ceData.totalSteps;

			if (!cr.isValid())
				return;


			p->setPen(cg.light());
			p->setBrush(cg.light());
			p->drawRect(r);

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



				if (reverse)
					p->fillRect(cr.x()+(cr.width()-width), cr.y(), width, cr.height(),
								cg.brush(TQColorGroup::Highlight));
				else
					p->fillRect(cr.x(), cr.y(), width, cr.height(),
								cg.brush(TQColorGroup::Highlight));
			}
			break;
		}

		case CE_CheckBox: {
			drawPrimitive(PE_Indicator, p, ceData, elementFlags, r, cg, sf);
			if (sf & Style_On) {
				drawPrimitive(PE_CheckMark, p, ceData, elementFlags, r, cg, sf);
			}
			break;
		}

		case CE_RadioButton: {
			drawPrimitive(PE_ExclusiveIndicator, p, ceData, elementFlags, r, cg, sf);
			if (sf & Style_On) {
				TQCOORD center[] = { 4, 5, 4, 6, 5, 7, 6, 7, 7, 6, 7, 5, 6, 4, 5, 4 };
				TQPointArray c(8, center);
				p->setPen(cg.text());
				p->setBrush(cg.text());
				p->drawPolygon(c);
			}
			break;
		}

		case CE_PushButton: {
			// Get rid of ugliness in Konqueror and KDevelop tab bar buttons, respectively
			if ( ceData.widgetObjectTypes.contains("KMultiTabBarButton") || ceData.widgetObjectTypes.contains("Ideal::Button")) {
				p->setPen(cg.mid());
				p->setBrush(cg.background());
				p->drawRect(r);
	
				if (sf & Style_On) {
					p->setPen(cg.mid());
					p->setBrush(TQBrush(cg.light(),TQt::Dense4Pattern));
					p->drawRect(r);
					p->setPen(cg.buttonText());
				} else if (sf & Style_Down) {
					p->setPen(cg.mid());
					p->drawRect(r);
					p->setPen(cg.buttonText());
				} else {
					p->setPen(cg.mid());
					p->drawLine(x, y, x2-1, y);
					p->drawLine(x, y, x, y2-1);
				}
			}
			else {
				if (elementFlags & CEF_IsDefault) {
					drawPrimitive(PE_ButtonDefault, p, ceData, elementFlags, r, cg, sf);
					drawPrimitive(PE_ButtonBevel, p, ceData, elementFlags, TQRect(x+1, y+1, sw-2, sh-2), cg, sf);
				} else {
					drawPrimitive(PE_ButtonBevel, p, ceData, elementFlags, r, cg, sf);
				}
			}

			break;
		}

		case CE_MenuBarItem: {
			bool active = sf & Style_Active;
			bool focused = sf & Style_HasFocus;
			bool down = sf & Style_Down;
			bool enabled = sf & Style_Enabled;
			bool etchtext = styleHint( SH_EtchDisabledText, ceData, elementFlags );
			const int text_flags = AlignVCenter | AlignHCenter | ShowPrefix | DontClip | SingleLine;

			if (active && focused) {
				p->setBrush(cg.background());
				p->setPen(cg.background());
				p->drawRect(r);

				if (down) {
					p->setPen(cg.mid());
					p->drawLine(x, y, x2, y);
					p->drawLine(x, y, x, y2);
					p->setPen(cg.light());
					p->drawLine(x2, y2, x, y2);
					p->drawLine(x2, y2, x2, y);
					/*p->translate(1, 1);*/
				} else {
					p->setPen(cg.light());
					p->drawLine(x, y, x2, y);
					p->drawLine(x, y, x, y2);
					p->setPen(cg.mid());
					p->drawLine(x2, y2, x, y2);
					p->drawLine(x2, y2, x2, y);
				}
			}

			p->setPen( ( etchtext && !enabled )?cg.dark():cg.foreground() );
			TQRect r_offset = TQRect(r.x()+ETCH_X_OFFSET, r.y()+ETCH_Y_OFFSET, r.width(), r.height());
			if ( etchtext && !enabled ) {
				TQPen savePen = p->pen();
				p->setPen( cg.light() );
				p->drawText(r_offset, text_flags, o.menuItem()->text());
				p->setPen(savePen);
			}
			p->drawText(r, text_flags, o.menuItem()->text());

			/*if (active && focused && down) {
				p->translate(-1, -1);
			}*/

			break;
		}


		case CE_PushButtonLabel: {
			const TQPushButton *pb = dynamic_cast<const TQPushButton *>(w);
			const bool enabled = sf & Style_Enabled;
			bool etchtext = styleHint( SH_EtchDisabledText, ceData, elementFlags );
			const int text_flags = AlignVCenter | AlignHCenter | ShowPrefix | DontClip | SingleLine;
			int dx = 0;

			if (sf & Style_Down) {
				p->translate(pixelMetric(PM_ButtonShiftHorizontal, ceData, elementFlags),
				             pixelMetric(PM_ButtonShiftVertical, ceData, elementFlags));
			}

			if (pb->iconSet() && !pb->iconSet()->isNull()) {
				TQIconSet::Mode mode = enabled ? TQIconSet::Normal : TQIconSet::Disabled;
				TQPixmap pixmap = pb->iconSet()->pixmap(TQIconSet::Small, mode);
				TQRect pr(0, 0, pixmap.width(), pixmap.height());

				if (pb->text().isNull())
					dx = r.center().x()-(pixmap.width()/2);
				else
					dx = (r.height() - pixmap.height())/2;
				if ( pb->inherits("KMultiTabBarButton") ) {
					pr.moveCenter(TQPoint(((pixmap.width()/2)+dx), r.center().y()));
					dx = (pixmap.width()+dx+(dx*0.5));
				}
				else {
					pr.moveCenter(TQPoint(((pixmap.width()/2)+dx), r.center().y()));
					dx = (pixmap.width()+dx+(dx*0.5));
				}

				p->drawPixmap(pr.topLeft(), pixmap);
			} else if (pb->pixmap() && !pb->text()) {
				TQRect pr(0, 0, pb->pixmap()->width(), pb->pixmap()->height());
				pr.moveCenter(r.center());
				p->drawPixmap(pr.topLeft(), *pb->pixmap());
			}

			if (!pb->text().isNull()) {
				p->setPen(( etchtext && !enabled )?cg.dark():(enabled ? cg.buttonText() : pb->palette().disabled().buttonText()));
				if (pb->iconSet() && !pb->iconSet()->isNull()) {
					TQRect tpr(dx, r.y(), r.width()-dx, r.height());
					TQRect tr(p->boundingRect(tpr, text_flags, pb->text()));

					TQRect tr_offset = TQRect(tr.x()+ETCH_X_OFFSET, tr.y()+ETCH_Y_OFFSET, tr.width(), tr.height());
					if ( etchtext && !enabled ) {
						TQPen savePen = p->pen();
						p->setPen( cg.light() );
						p->drawText(tr_offset, text_flags, pb->text());
						p->setPen(savePen);
					}
					p->drawText(tr, text_flags, pb->text());
				} else {
					TQRect r_offset = TQRect(r.x()+ETCH_X_OFFSET, r.y()+ETCH_Y_OFFSET, r.width(), r.height());
					if ( etchtext && !enabled ) {
						TQPen savePen = p->pen();
						p->setPen( cg.light() );
						p->drawText(r_offset, text_flags, pb->text());
						p->setPen(savePen);
					}
					p->drawText(r, text_flags, pb->text());
				}
			}

			break;
		}

	/*	Note: This is very poorly documented by TT. I'm disappointed. -clee */
		case CE_HeaderLabel: {
			const bool enabled = sf & Style_Enabled;
			bool etchtext = styleHint( SH_EtchDisabledText, ceData, elementFlags );
			const int text_flags = AlignVCenter | ShowPrefix | DontClip | SingleLine;

			if (!ceData.iconSet.isNull()) {
				TQPixmap pm = ceData.iconSet.pixmap(TQIconSet::Small, sf & Style_Enabled ? TQIconSet::Normal : TQIconSet::Disabled);
				TQRect pr(0, 0, pm.width(), pm.height());
				pr.moveCenter(r.center());
				pr.setLeft(r.center().y() - (pm.height() - 1) / 2);
				p->drawPixmap(pr.topLeft(), pm);
				pr = TQRect(pr.width(), r.top(), r.width() - pr.width(), r.height());
				TQRect pr_offset = TQRect(pr.x()+ETCH_X_OFFSET, pr.y()+ETCH_Y_OFFSET, pr.width(), pr.height());
				if ( etchtext && !enabled ) {
					p->setPen( cg.dark()) ;
					TQPen savePen = p->pen();
					p->setPen( cg.light() );
					p->drawText(pr_offset, text_flags, ceData.textLabel);
					p->setPen(savePen);
				}
				p->drawText(pr, text_flags, ceData.textLabel);
			} else {
				p->setPen( ( etchtext && !enabled )?cg.dark():cg.buttonText() );
				TQRect r_offset = TQRect(r.x()+ETCH_X_OFFSET, r.y()+ETCH_Y_OFFSET, r.width(), r.height());
				if ( etchtext && !enabled ) {
					TQPen savePen = p->pen();
					p->setPen( cg.light() );
					p->drawText(r_offset, text_flags, ceData.textLabel);
					p->setPen(savePen);
				}
				p->drawText(r, text_flags, ceData.textLabel);
			}
			break;
		}

		case CE_PopupMenuItem: {
			TQMenuItem *mi = o.menuItem();

			if (!mi) {
				return;
			}

			static const int itemFrame = 2;
			static const int itemHMargin = 3;
			static const int itemVMargin = 3;
			static const int arrowHMargin = 4;
			static const int rightBorder = 16;
			const int tab = o.tabWidth();

			int checkcol = TQMAX(o.maxIconWidth(), 12);

			bool active = sf & Style_Active;
			bool disabled = !mi->isEnabled();
			bool checkable = (elementFlags & CEF_IsCheckable);
			bool enabled = mi->isEnabled();
			bool etchtext = styleHint( SH_EtchDisabledText, ceData, elementFlags );

			int xpos = x;
			int xm = itemFrame + checkcol + itemHMargin;

			if (!ceData.bgPixmap.isNull()) {
				p->drawPixmap(x, y, ceData.bgPixmap, x, y, sw, sh);
			} else {
				p->fillRect(x, y, sw, sh, cg.background());
			}

			if (mi->widget()) {
			/*	nOOOOOOOOOOOP */
			} else if (mi->isSeparator()) {
				p->setPen(cg.mid());
				p->drawLine(x + 1, y + 3, x2 - 1, y + 3);
				p->setPen(cg.light());
				p->drawLine(x + 1, y + 4, x2 - 1, y + 4);
				return;
			}

			if (active && !disabled && !mi->isSeparator()) {
				p->setBrush(cg.highlight());
				p->fillRect(r, cg.highlight());
				p->setPen(cg.highlightedText());
			}

			if (mi->iconSet()) {
				TQIconSet::Mode mode =
					disabled ? TQIconSet::Disabled : TQIconSet::Normal;
				TQPixmap pixmap = mi->iconSet()->pixmap(TQIconSet::Small, mode);

				int pixw = pixmap.width();
				int pixh = pixmap.height();

				TQRect cr(xpos, y, o.maxIconWidth(), sh);
				TQRect pmr(0, 0, pixw, pixh);
				pmr.moveCenter(cr.center());

				if (backwards)
					pmr = visualRect(pmr, r);

				p->setPen(cg.highlightedText());
				p->drawPixmap(pmr.topLeft(), pixmap);
			} else if (checkable) {
				if (mi->isChecked()) {
					drawPrimitive(PE_MenuItemIndicatorCheck, p, ceData, elementFlags, r, cg, sf, o);
				}
			}

			xpos += xm;

			if (mi->custom()) {
				int m = itemVMargin;
				p->setPen( ( etchtext && !enabled )?cg.dark():cg.foreground() );
				if ( etchtext && !enabled ) {
					TQPen savePen = p->pen();
					p->setPen( cg.light() );
					TQColorGroup etchedcg = cg;
					etchedcg.setColor( TQColorGroup::Text, cg.light() );
					etchedcg.setColor( TQColorGroup::Mid, cg.light() );
					etchedcg.setColor( TQColorGroup::Midlight, cg.light() );
					etchedcg.setColor( TQColorGroup::Foreground, cg.light() );
					etchedcg.setColor( TQColorGroup::HighlightedText, cg.light() );
					etchedcg.setColor( TQColorGroup::BrightText, cg.light() );
					etchedcg.setColor( TQColorGroup::ButtonText, cg.light() );
					mi->custom()->paint(p, etchedcg, active, !disabled, x+xm+ETCH_X_OFFSET, y+m+ETCH_Y_OFFSET, sw-xm-tab+1, sh-2*m);
					p->setPen(savePen);
				}
				mi->custom()->paint(p, cg, active, !disabled, x+xm, y+m, sw-xm-tab+1, sh-2*m);
				return;
			} else {
				p->setPen(active ? cg.highlightedText() : cg.buttonText());
				TQString s = mi->text();

				if(!s.isNull()) {
					int t = s.find('\t');
					int m = itemVMargin;

					int text_flags = AlignVCenter | ShowPrefix | DontClip | SingleLine;

					if (active && !disabled) {
						p->setPen(cg.highlightedText());
					} else if (disabled) {
						p->setPen(cg.mid().light(110));
					} else {
						p->setPen(cg.text());
					}

					if (t >= 0) {
						int xp = x + sw - tab - rightBorder - itemHMargin - itemFrame + 1;

						TQRect rr = TQRect(xp, y+m, tab, sh-(2*m));
						TQRect rr_offset = TQRect(xp+ETCH_X_OFFSET, y+m+ETCH_Y_OFFSET, tab, sh-(2*m));
						if (backwards) {
							rr = visualRect(rr, r);
							rr_offset = visualRect(rr_offset, r);
						}

						if ( etchtext && !enabled ) {
							p->setPen(cg.dark());
							TQPen savePen = p->pen();
							p->setPen( cg.light() );
							p->drawText(rr_offset, text_flags, s.mid(t+1));
							p->setPen(savePen);
						}
						p->drawText(rr, text_flags, s.mid(t+1));
						s = s.left(t);
					}

					TQRect rr = TQRect(xpos, y+m, sw-xm-tab+1, sh-(2*m));
					TQRect rr_offset = TQRect(xpos+ETCH_X_OFFSET, y+m+ETCH_Y_OFFSET, sw-xm-tab+1, sh-(2*m));
					if (backwards) {
						rr = visualRect(rr, r);
						rr_offset = visualRect(rr_offset, r);
						text_flags |= AlignRight;
					}

					if ( etchtext && !enabled ) {
						p->setPen(cg.dark());
						TQPen savePen = p->pen();
						p->setPen( cg.light() );
						p->drawText(rr_offset, text_flags, s);
						p->setPen(savePen);
					}
					p->drawText(rr, text_flags, s);
				} else if (mi->pixmap()) {
					TQPixmap *pixmap = mi->pixmap();
					if (pixmap->depth() == 1) {
						p->setBackgroundMode(TQt::OpaqueMode);
					}
					p->drawPixmap(xpos, y, *pixmap);
					if (pixmap->depth() == 1) {
						p->setBackgroundMode(TQt::TransparentMode);
					}
				}
			}

			if (mi->popup()) {
				int dim = pixelMetric(PM_MenuButtonIndicator, ceData, elementFlags);

				xpos = x + sw - arrowHMargin - 2 * itemFrame - dim;

				if (active && !disabled) {
					p->setPen(cg.highlightedText());
				} else {
					p->setPen(cg.text());
				}

				TQRect rr = TQRect(xpos, y + sh/2 - dim/2, dim, dim);
				if (backwards)
					rr = visualRect(rr, r);
				drawPrimitive((backwards ? PE_ArrowLeft : PE_ArrowRight), p, ceData, elementFlags, rr, cg, Style_Enabled);
			}

			break;
		}

		default: {
			TDEStyle::drawControl(ce, p, ceData, elementFlags, r, cg, sf, o, w);
		}
	}
}

void AsteroidStyle::drawControlMask(ControlElement ce,
                                    TQPainter *p,
                                    const TQStyleControlElementData &ceData,
                                    ControlElementFlags elementFlags,
                                    const TQRect &r,
                                    const TQStyleOption &o,
                                    const TQWidget *w) const
{
	switch (ce) {
	/*
		TQ_ControlElements to draw are:

		CE_PushButton
		CE_PushButtonLabel

		CE_CheckBox
		CE_CheckBoxLabel

		CE_RadioButton
		CE_RadioButtonLabel

		CE_TabBarTab
		CE_TabBarLabel

		CE_ProgressBarGroove
		CE_ProgressBarContents
		CE_ProgressBarLabel

		CE_PopupMenuScroller
		CE_PopupMenuHorizontalExtra
		CE_PopupMenuVerticalExtra
		CE_PopupMenuItem
		CE_MenuBarItem
		CE_MenuBarEmptyArea
		CE_DockWindowEmptyArea

		CE_ToolButtonLabel
		CE_ToolBoxTab

		CE_HeaderLabel
	 */
			default: {
			TDEStyle::drawControlMask(ce, p, ceData, elementFlags, r, o, w);
		}
	}
}

void AsteroidStyle::drawComplexControl(ComplexControl cc,
                                       TQPainter *p,
                                       const TQStyleControlElementData &ceData,
                                       ControlElementFlags elementFlags,
                                       const TQRect &r,
                                       const TQColorGroup &cg,
                                       SFlags sf,
                                       SCFlags sc,
                                       SCFlags sa,
                                       const TQStyleOption &o,
                                       const TQWidget *w) const
{
	switch (cc) {
	/*	TQ_ComplexControls available are:

		CC_SpinWidget
		CC_ScrollBar
		CC_Slider
		CC_ToolButton
		CC_TitleBar
		CC_ListView
	 */
#ifndef TQT_NO_LISTVIEW
		case CC_ListView:
			{
			if ( sc & SC_ListView ) {
				TQCommonStyle::drawComplexControl( cc, p, ceData, elementFlags, r, cg, sf, sc, sa, o, w );
			}
			if ( sc & ( SC_ListViewBranch | SC_ListViewExpand ) ) {
				if (o.isDefault())
				break;
		
				TQListViewItem *item = o.listViewItem(),
					*child = item->firstChild();
		
				int y = r.y();
				int c;
				int dotoffset = 0;
				TQPointArray dotlines;
				if ( sa == (uint)SC_All && sc == SC_ListViewExpand ) {
				c = 2;
				dotlines.resize(2);
				dotlines[0] = TQPoint( r.right(), r.top() );
				dotlines[1] = TQPoint( r.right(), r.bottom() );
				} else {
				int linetop = 0, linebot = 0;
				// each branch needs at most two lines, ie. four end points
				dotoffset = (item->itemPos() + item->height() - y) %2;
				dotlines.resize( item->childCount() * 4 );
				c = 0;
		
				// skip the stuff above the exposed rectangle
				while ( child && y + child->height() <= 0 ) {
					y += child->totalHeight();
					child = child->nextSibling();
				}
		
				int bx = r.width() / 2;
		
				// paint stuff in the magical area
				TQListView* v = item->listView();
				while ( child && y < r.height() ) {
					if (child->isVisible()) {
					int lh;
					if ( !item->multiLinesEnabled() )
						lh = child->height();
					else
						lh = p->fontMetrics().height() + 2 * v->itemMargin();
					lh = TQMAX( lh, TQApplication::globalStrut().height() );
					if ( lh % 2 > 0 )
						lh++;
					linebot = y + lh/2;
					if ( (child->isExpandable() || child->childCount()) &&
						(child->height() > 0) ) {
						// needs a box
						p->setPen( cg.mid() );
						p->drawRect( bx-4, linebot-4, 9, 9 );
						// plus or minus
						p->setPen( cg.text() );
						p->drawLine( bx - 2, linebot, bx + 2, linebot );
						if ( !child->isOpen() )
						p->drawLine( bx, linebot - 2, bx, linebot + 2 );
						// dotlinery
						p->setPen( cg.mid() );
						dotlines[c++] = TQPoint( bx, linetop );
						dotlines[c++] = TQPoint( bx, linebot - 4 );
						dotlines[c++] = TQPoint( bx + 5, linebot );
						dotlines[c++] = TQPoint( r.width(), linebot );
						linetop = linebot + 5;
					} else {
						// just dotlinery
						dotlines[c++] = TQPoint( bx+1, linebot -1);
						dotlines[c++] = TQPoint( r.width(), linebot -1);
					}
					y += child->totalHeight();
					}
					child = child->nextSibling();
				}
		
				// Expand line height to edge of rectangle if there's any
				// visible child below
				while ( child && child->height() <= 0)
					child = child->nextSibling();
				if ( child )
					linebot = r.height();
		
				if ( linetop < linebot ) {
					dotlines[c++] = TQPoint( bx, linetop );
					dotlines[c++] = TQPoint( bx, linebot );
				}
				}
				p->setPen( cg.text() );
		
				static TQBitmap *verticalLine = 0, *horizontalLine = 0;
				static TQCleanupHandler<TQBitmap> qlv_cleanup_bitmap;
				if ( !verticalLine ) {
				// make 128*1 and 1*128 bitmaps that can be used for
				// drawing the right sort of lines.
				verticalLine = new TQBitmap( 1, 129, TRUE );
				horizontalLine = new TQBitmap( 128, 1, TRUE );
				TQPointArray a( 64 );
				TQPainter p;
				p.begin( verticalLine );
				int i;
				for( i=0; i<64; i++ )
					a.setPoint( i, 0, i*2+1 );
				p.setPen( TQt::color1 );
				p.drawPoints( a );
				p.end();
				TQApplication::flushX();
				verticalLine->setMask( *verticalLine );
				p.begin( horizontalLine );
				for( i=0; i<64; i++ )
					a.setPoint( i, i*2+1, 0 );
				p.setPen( TQt::color1 );
				p.drawPoints( a );
				p.end();
				TQApplication::flushX();
				horizontalLine->setMask( *horizontalLine );
				qlv_cleanup_bitmap.add( &verticalLine );
				qlv_cleanup_bitmap.add( &horizontalLine );
				}
		
				int line; // index into dotlines
				if ( sc & SC_ListViewBranch ) for( line = 0; line < c; line += 2 ) {
				// assumptions here: lines are horizontal or vertical.
				// lines always start with the numerically lowest
				// coordinate.
		
				// point ... relevant coordinate of current point
				// end ..... same coordinate of the end of the current line
				// other ... the other coordinate of the current point/line
				if ( dotlines[line].y() == dotlines[line+1].y() ) {
					int end = dotlines[line+1].x();
					int point = dotlines[line].x();
					int other = dotlines[line].y();
					while( point < end ) {
					int i = 128;
					if ( i+point > end )
						i = end-point;
					p->drawPixmap( point, other, *horizontalLine,
							0, 0, i, 1 );
					point += i;
					}
				} else {
					int end = dotlines[line+1].y();
					int point = dotlines[line].y();
					int other = dotlines[line].x();
					int pixmapoffset = ((point & 1) != dotoffset ) ? 1 : 0;
					while( point < end ) {
					int i = 128;
					if ( i+point > end )
						i = end-point;
					p->drawPixmap( other, point, *verticalLine,
							0, pixmapoffset, 1, i );
					point += i;
					}
				}
				}
			}
			}
			break;
#endif //TQT_NO_LISTVIEW

		case CC_ComboBox: {
			int x, y, x2, y2, sw, sh;
			r.rect(&x, &y, &sw, &sh);
			r.coords(&x, &y, &x2, &y2);

			if (sa & Style_Sunken) {
				sf |= Style_Sunken;
			}

			static const unsigned char downarrow_bits[] = {
				0x7f, 0xbe, 0x9c, 0x08, 0x00, 0x00, 0x00, 0x28,
				0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0xb8
			};

			static const unsigned int handle_width = 16;
			static const unsigned int handle_offset = handle_width + 1;

			TQBitmap downArrow = TQBitmap(7, 4, downarrow_bits, true);
			downArrow.setMask(downArrow);

		//	Draw the frame around the text.
			p->setPen(cg.mid());
			p->setBrush(cg.dark());
			p->drawRect(r);
			p->setPen(cg.light());
			p->drawLine(x2, y2, x2, y);
			p->drawLine(x2, y2, x, y2);
			p->setPen(cg.background());
			p->drawLine(x2-1, y2-1, x2-1, y+1);
			p->drawLine(x2-1, y2-1, x+1, y2-1);

			// Fill in the area behind the text.
			p->fillRect(querySubControlMetrics(cc, ceData, elementFlags, SC_ComboBoxEditField, TQStyleOption::Default, w), cg.base());
			p->setBrush(cg.background());

			// Draw the box on the right.
			TQRect hr(sw - handle_offset-1, y+2, handle_width, sh-4);
			int hrx, hry, hrx2, hry2;
			hr.coords(&hrx, &hry, &hrx2, &hry2);
			if (backwards) { hr = visualRect(hr, r); }

			p->drawRect(hr);

			bool draw_skinny_frame = false;
// 			if (!ceData.widgetObjectTypes.contains("TQComboBox")) draw_skinny_frame = true;
			if (ceData.comboBoxListBoxFlags & CEF_IsVisible) draw_skinny_frame = true;
			if (draw_skinny_frame) {
				p->setPen(cg.mid());
				p->drawRect(hr);
			} else {
				p->setPen(cg.light());
				p->drawLine(hrx+1, hry+1, hrx2-1, hry+1);
				p->drawLine(hrx+1, hry+1, hrx+1, hry2-1);

				p->setPen(cg.mid());
				p->drawLine(hrx2-1, hry2-1, hrx+1, hry2-1);
				p->drawLine(hrx2-1, hry2-1, hrx2-1, hry+1);

				p->setPen(cg.dark());
				p->drawLine(hrx2, hry2, hrx, hry2);
				p->drawLine(hrx2, hry2, hrx2, hry);
			}

			TQRect cr(sw - handle_offset-1, y+2, handle_width, sh - 4);
			TQRect pmr(0, 0, 7, 4);
			pmr.moveCenter(cr.center());
			if (ceData.comboBoxListBoxFlags & CEF_IsVisible) {
				pmr.moveBy(1, 1);
			}

			if (backwards) { pmr = visualRect(pmr, r); }

			p->setPen(cg.foreground());
			p->drawPixmap(pmr.topLeft(), downArrow);

			break;
		}

#ifndef TQT_NO_TOOLBUTTON
		case CC_ToolButton:
		{
			TQColorGroup c = cg;
			if ( ceData.backgroundMode != TQt::PaletteButton )
				c.setBrush( TQColorGroup::Button, ceData.paletteBgColor );
			TQRect button, menuarea;
			button   = visualRect( querySubControlMetrics(cc, ceData, elementFlags, SC_ToolButton, o, w), ceData, elementFlags );
			menuarea = visualRect( querySubControlMetrics(cc, ceData, elementFlags, SC_ToolButtonMenu, o, w), ceData, elementFlags );
		
			SFlags bflags = sf;
			SFlags mflags = sf;
		
			if (sa & SC_ToolButton)
				bflags |= Style_Down;
			if (sa & SC_ToolButtonMenu)
				mflags |= Style_Down;

			if (sc & SC_ToolButton) {
				if (bflags & (Style_Down | Style_On | Style_Raised)) {
					drawPrimitive(TQStyle::PE_ButtonTool, p, ceData, elementFlags, button, c, bflags, o);
				} else if ( ! ceData.parentWidgetData.bgPixmap.isNull() ) {
					TQPixmap pixmap = ceData.parentWidgetData.bgPixmap;
			
					p->drawTiledPixmap( r, pixmap, ceData.pos );
				}
			}
		
			if (sc & SC_ToolButtonMenu) {
				if (mflags & (Style_Down | Style_On | Style_Raised))
					drawPrimitive(TQStyle::PE_ButtonDropDown, p, ceData, elementFlags, menuarea, c, mflags, o);
				drawPrimitive(TQStyle::PE_ArrowDown, p, ceData, elementFlags, menuarea, c, mflags, o);
			}
		
			if ((elementFlags & CEF_HasFocus) && !(elementFlags & CEF_HasFocusProxy)) {
				TQRect fr = ceData.rect;
				fr.addCoords(3, 3, -3, -3);
				drawPrimitive(TQStyle::PE_FocusRect, p, ceData, elementFlags, fr, c);
			}
		
			break;
		}
#endif // TQT_NO_TOOLBUTTON

		case CC_Slider: {
			TQRect groove = querySubControlMetrics(CC_Slider, ceData, elementFlags, SC_SliderGroove, o, w);
			TQRect handle = querySubControlMetrics(CC_Slider, ceData, elementFlags, SC_SliderHandle, o, w);

			// Double-buffer slider for no flicker
			TQPixmap pix(ceData.rect.size());
			TQPainter p2;
			p2.begin(&pix);

			if ( !ceData.parentWidgetData.bgPixmap.isNull() ) {
				TQPixmap pixmap = ceData.parentWidgetData.bgPixmap;
				p2.drawTiledPixmap(r, pixmap, ceData.pos);
			} else
				pix.fill(cg.background());

			// Draw slider groove
			if ((sc & SC_SliderGroove) && groove.isValid()) {
				drawTDEStylePrimitive( KPE_SliderGroove, &p2, ceData, elementFlags, groove, cg, sf, o, w );

				// Draw the focus rect around the groove
				if (elementFlags & CEF_HasFocus)
					drawPrimitive(PE_FocusRect, &p2, ceData, elementFlags, groove, cg);
			}


                        /* Turn these off for now */
                        // Draw the tickmarks
			/*if (controls & SC_SliderTickmarks)
				TQCommonStyle::drawComplexControl(control, &p2,
						r, cg, flags, SC_SliderTickmarks, sa, o);*/

			// Draw the slider handle
			if ((sc & SC_SliderHandle) && handle.isValid()) {
				if (sa == SC_SliderHandle)
					sf |= Style_Active;
				drawTDEStylePrimitive( KPE_SliderHandle, &p2, ceData, elementFlags, handle, cg, sf, o, w );
			}

			p2.end();
			if (w) {
				bitBlt((TQWidget*)w, r.x(), r.y(), &pix);
			}
			else {
				p->drawPixmap(r.topLeft(), pix);
			}
			break;
		}

		case CC_SpinWidget: {
			int x, y, x2, y2, w, h;
			int aw = SPINBOX_BUTTON_WIDTH;  // arrow button width
			r.coords(&x, &y, &x2, &y2);
			r.rect(&x, &y, &w, &h);

			TQRect arrowup(x2-aw, y+2, aw, h/2-1);
			TQRect arrowdn(x2-aw, h/2+1, aw, h/2-2);

			// Draw the frame around the text
			p->setPen(cg.mid());
			p->setBrush(cg.dark());
			p->drawRect(r);
			p->setPen(cg.light());
			p->drawLine(x2, y2, x2, y);
			p->drawLine(x2, y2, x, y2);
			p->setPen(cg.background());
			p->drawLine(x2-1, y2-1, x2-1, y+2);
			p->drawLine(x2-1, y2-1, x+1, y2-1);
			p->drawLine(x2-aw-1, y+2, x2-aw-1, y2-1);

			// Draw the arrow buttons
			SFlags upflags = Style_Default;
			SFlags downflags = Style_Default;
			if (ceData.spinWidgetData.upEnabled) {
				upflags |= Style_Enabled;
			}
			if (ceData.spinWidgetData.downEnabled) {
				downflags |= Style_Enabled;
			}
			if (sa == SC_SpinWidgetUp) {
				upflags |= Style_On;
				upflags |= Style_Sunken;
				downflags |= Style_Raised;
			}
			if (sa == SC_SpinWidgetDown) {
				downflags |= Style_On;
				downflags |= Style_Sunken;
				upflags |= Style_Raised;
			}

			p->setPen(TQt::NoPen);
			drawPrimitive(PE_ButtonBevel, p, ceData, elementFlags, arrowup, cg, upflags, o);
			p->setPen(TQt::NoPen);
			drawPrimitive(PE_ButtonBevel, p, ceData, elementFlags, arrowdn, cg, downflags, o);
			p->setPen(TQt::NoPen);
			drawPrimitive(PE_SpinWidgetUp, p, ceData, elementFlags, arrowup, cg, upflags, o);
			p->setPen(TQt::NoPen);
			drawPrimitive(PE_SpinWidgetDown, p, ceData, elementFlags, arrowdn, cg, downflags, o);

			break;

		}



		default: {
			TDEStyle::drawComplexControl(cc, p, ceData, elementFlags, r, cg, sf, sc, sa, o, w);
		}
	}
}

void AsteroidStyle::drawComplexControlMask(ComplexControl cc,
                                           TQPainter *p,
                                           const TQStyleControlElementData &ceData,
                                           const ControlElementFlags elementFlags,
                                           const TQRect &r,
                                           const TQStyleOption &o,
                                           const TQWidget *w) const
{
	switch (cc) {
	/*	TQ_ComplexControls available are:

		CC_SpinWidget
		CC_ComboBox
		CC_ScrollBar
		CC_Slider
		CC_ToolButton
		CC_TitleBar
		CC_ListView
	 */
			default: {
			TDEStyle::drawComplexControlMask(cc, p, ceData, elementFlags, r, o, w);
		}
	}
}

int AsteroidStyle::pixelMetric(PixelMetric pm, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQWidget *w) const
{
	switch (pm) {
	/*	PixelMetrics available are:

		PM_ButtonMargin
		PM_ButtonDefaultIndicator

		PM_SpinBoxFrameWidth
		PM_MDIFrameWidth
		PM_MDIMinimizedWidth

		PM_MaximumDragDistance

		PM_ScrollBarExtent
		PM_ScrollBarSliderMin

		PM_SliderThickness
		PM_SliderControlThickness
		PM_SliderLength
		PM_SliderTickmarkOffset
		PM_SliderSpaceAvailable

		PM_DockWindowSeparatorExtent
		PM_DockWindowHandleExtent

		PM_TabBarTabOverlap
		PM_TabBarTabHSpace
		PM_TabBarTabVSpace
		PM_TabBarBaseHeight
		PM_TabBarBaseOverlap

		PM_ProgressBarChunkWidth

		PM_SplitterWidth

		PM_TitleBarHeight

		PM_CheckListButtonSize
		PM_CheckListControllerSize

		PM_PopupMenuFrameHorizontalExtra
		PM_PopupMenuFrameVerticalExtra
		PM_PopupMenuScrollerHeight

		PM_HeaderMarkSize

		PM_TabBarTabShiftHorizontal
		PM_TabBarTabShiftVertical
	 */

		case PM_ExclusiveIndicatorWidth:
		case PM_ExclusiveIndicatorHeight: {
			return 12;
		}

		case PM_IndicatorWidth:
		case PM_IndicatorHeight: {
			return 13;
		}

		case PM_MenuBarFrameWidth: {
			return 2;
		}

		case PM_ButtonShiftHorizontal:
		case PM_ButtonShiftVertical: {
			return 1;
		}

		case PM_HeaderGripMargin:
		case PM_HeaderMargin: {
			return 3;
		}

		case PM_DialogButtonsButtonWidth: {
			return MINIMUM_PUSHBUTTON_WIDTH;
		}

		case PM_DialogButtonsButtonHeight: {
			return MINIMUM_PUSHBUTTON_HEIGHT;
		}

		case PM_DockWindowSeparatorExtent:
		case PM_MenuButtonIndicator:
		case PM_DockWindowFrameWidth:
		case PM_DialogButtonsSeparator: {
			return 6;
		}

		case PM_DefaultFrameWidth: {
			if (ceData.widgetObjectTypes.contains("TQPopupMenu")) {
				return 3;
			} else {
				return TDEStyle::pixelMetric(pm, ceData, elementFlags, w);
			}
		}

		case PM_TabBarTabOverlap:
			return 4;
		
		case PM_TabBarBaseHeight:
			return 0;
		
		case PM_TabBarBaseOverlap:
			return 0;
		
		case PM_TabBarTabHSpace:
			return 24;
		
		case PM_TabBarTabShiftHorizontal:
			return 4;

		case PM_TabBarTabShiftVertical:
			return 2;

		case PM_SplitterWidth:
			return TQMAX( 4, TQApplication::globalStrut().width() );

		case PM_MenuIndicatorFrameHBorder:
		case PM_MenuIndicatorFrameVBorder:
		case PM_MenuIconIndicatorFrameHBorder:
		case PM_MenuIconIndicatorFrameVBorder:
			return 0;

		default: {
			return TDEStyle::pixelMetric(pm, ceData, elementFlags, w);
		}
	}
}

TQRect AsteroidStyle::subRect(SubRect sr, const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, const TQWidget *w) const
{
	switch (sr) {
	/*	SubRects to calculate are:

		SR_PushButtonContents
		SR_PushButtonFocusRect

		SR_CheckBoxIndicator
		SR_CheckBoxContents
		SR_CheckBoxFocusRect

		SR_RadioButtonIndicator
		SR_RadioButtonContents
		SR_RadioButtonFocusRect

		SR_ComboBoxFocusRect
		SR_SliderFocusRect

		SR_DockWindowHandleRect

		SR_ProgressBarGroove
		SR_ProgressBarContents
		SR_ProgressBarLabel

		SR_ToolButtonContents
		SR_ToolBoxTabContents

		SR_DialogButtonAccept
		SR_DialogButtonReject
		SR_DialogButtonApply
		SR_DialogButtonHelp
		SR_DialogButtonAll
		SR_DialogButtonAbort
		SR_DialogButtonIgnore
		SR_DialogButtonRetry
		SR_DialogButtonCustom
	 */
		default: {
			return TDEStyle::subRect(sr, ceData, elementFlags, w);
		}
	}
}

TQRect AsteroidStyle::querySubControlMetrics(ComplexControl cc,
                                            const TQStyleControlElementData &ceData,
                                            ControlElementFlags elementFlags,
                                            SubControl sc,
                                            const TQStyleOption &o,
                                            const TQWidget *w) const
{
	switch (cc) {
	/*	Available SubControls are:

		SC_ScrollBarAddLine
		SC_ScrollBarSubLine
		SC_ScrollBarAddPage
		SC_ScrollBarSubPage
		SC_ScrollBarFirst
		SC_ScrollBarLast
		SC_ScrollBarSlider
		SC_ScrollBarGroove

		SC_SpinWidgetUp
		SC_SpinWidgetDown
		SC_SpinWidgetFrame
		SC_SpinWidgetEditField
		SC_SpinWidgetButtonField

		SC_ComboBoxFrame
		SC_ComboBoxArrow
		SC_ComboBoxListBoxPopup

		SC_SliderGroove
		SC_SliderHandle
		SC_SliderTickmarks

		SC_ToolButton
		SC_ToolButtonMenu

		SC_TitleBarLabel
		SC_TitleBarSysMenu
		SC_TitleBarMinButton
		SC_TitleBarMaxButton
		SC_TitleBarCloseButton
		SC_TitleBarNormalButton
		SC_TitleBarShadeButton
		SC_TitleBarUnshadeButton

		SC_ListView
		SC_ListViewBranch
		SC_ListViewExpand
	 */
		case CC_ComboBox: {
			TQRect r(ceData.rect);

			switch (sc) {
				case SC_ComboBoxEditField: {
					return TQRect(r.x()+2, r.y()+2, r.width()-20, r.height()-4);
				}
				default: {
					return TDEStyle::querySubControlMetrics(cc, ceData, elementFlags, sc, o, w);
				}
			}

			break;
		}

		case CC_SpinWidget: {
			int fw = pixelMetric(PM_SpinBoxFrameWidth, ceData, elementFlags, w);
			TQSize bs;
			bs.setHeight(ceData.rect.height()/2 - fw);
			if (bs.height() < 8) {
				bs.setHeight(8);
			}
			bs.setWidth(SPINBOX_BUTTON_WIDTH);
			bs = bs.expandedTo(TQApplication::globalStrut());
			int y = fw;
			int x, lx, rx;
			x = ceData.rect.width() - y - bs.width();
			lx = fw;
			rx = x - fw;
			switch ( sc ) {
				case SC_SpinWidgetUp:
					return TQRect(x, y, bs.width(), bs.height());
				case SC_SpinWidgetDown:
					return TQRect(x, y + bs.height(), bs.width(), bs.height());
				case SC_SpinWidgetButtonField:
					return TQRect(x, y, bs.width(), ceData.rect.height() - (2*fw));
				case SC_SpinWidgetEditField:
					return TQRect(lx, fw, rx, ceData.rect.height() - (2*fw));
				case SC_SpinWidgetFrame:
					return ceData.rect;
				default:
					return TDEStyle::querySubControlMetrics(cc, ceData, elementFlags, sc, o, w);
					break;
			}

			break;
		}

		default: {
			return TDEStyle::querySubControlMetrics(cc, ceData, elementFlags, sc, o, w);
		}
	}
}

TQSize AsteroidStyle::sizeFromContents(ContentsType ct,
                                      const TQStyleControlElementData &ceData,
                                      ControlElementFlags elementFlags,
                                      const TQSize &s,
                                      const TQStyleOption &o,
                                      const TQWidget *w) const
{
	switch (ct) {
	/*	ContentsType values can be:

		CT_CheckBox
		CT_RadioButton
		CT_Splitter
		CT_DockWindow
		CT_ProgressBar
		CT_TabBarTab
		CT_Slider
		CT_LineEdit
		CT_SpinBox
		CT_SizeGrip
		CT_TabWidget
		CT_DialogButtons
	 */
		case CT_ToolButton: {
			return TQSize(s.width() + 8, s.height() + 8);
		}

		case CT_PushButton: {
			const TQPushButton *pb = dynamic_cast<const TQPushButton *>(w);
			const TQSize ret = TDEStyle::sizeFromContents(ct, ceData, elementFlags, s, o, w);
			int rw = ret.width(), rh = ret.height();
			int mw;
			int mh;
			if (pb->text().length() > 0) {
				mw = MINIMUM_PUSHBUTTON_WIDTH;
				mh = MINIMUM_PUSHBUTTON_HEIGHT;
			}
			else {
				mw = 1;
				mh = 1;
			}

			if (pb->text().length() > 0) {
				if (pb->iconSet() && !pb->iconSet()->isNull()) {
					rw += ((pb->iconSet()->pixmap(TQIconSet::Small, TQIconSet::Normal).width())*2.0);
				} else if (pb->pixmap() && !pb->pixmap()->isNull()) {
					rw += ((pb->pixmap()->width())*2.0);
				}
			}
			else {
				if (pb->iconSet() && !pb->iconSet()->isNull()) {
					rw += ((pb->iconSet()->pixmap(TQIconSet::Small, TQIconSet::Normal).width())*0.0);
				} else if (pb->pixmap() && !pb->pixmap()->isNull()) {
					rw += ((pb->pixmap()->width())*0.0);
				}
			}

			return TQSize((rw < mw ? mw : rw), (rh < mh ? mh : rh));
		}

		case CT_ComboBox: {
			int padding = (pixelMetric(PM_DefaultFrameWidth, ceData, elementFlags, w) * 2) + 1;
			return TQSize(s.width() + 21, s.height() + padding);
		}

		case CT_PopupMenuItem: {
			if (o.isDefault()) {
				return TQSize(0, 0);
			}

			int sw = s.width(), sh = s.height();
			TQMenuItem *mi = o.menuItem();

			if (mi->custom()) {
				sw = mi->custom()->sizeHint().width();
				sh = mi->custom()->sizeHint().height();
			} else if (mi->widget()) {
			/* This is a do-nothing branch */
			} else if (mi->isSeparator()) {
				sw = 0, sh = 9;
			} else {
				if (mi->pixmap()) {
					sh = TQMAX(sh, mi->pixmap()->height() + 2);
				} else if (mi->iconSet()) {
					sh = TQMAX(sh, mi->iconSet()->pixmap(TQIconSet::Small, TQIconSet::Normal).height() + 2);
				}

				sh = TQMAX(sh, TQFontMetrics(ceData.font).height() + 4);
			}

			if (!mi->text().isNull()) {
				if ((mi->text().find('\t') >= 0)) {
					sw += 16;
				} else if (mi->popup()) {
					sw += 8;
				}
			}

			int miw = o.maxIconWidth();
			if (miw) {
				sw += miw;
				if (elementFlags & CEF_IsCheckable) {
					sw += 20 - miw;
				}
			}

			sw += 20;

			return TQSize(sw, sh);
		}

		case CT_MenuBar: {
			// Fall through is intentional
// 			const TQMenuBar *mb = dynamic_cast<const TQMenuBar *>(w);
// 			int sh = TQFontInfo(mb->font()).pixelSize() + 4;
// 			int sw = 10;
// 			return TQSize(sw, sh);
		}

		case CT_Header: {
			// Fall through is intentional
// 			const TQHeader *hw = dynamic_cast<const TQHeader *>(w);
// 			int sh = TQFontInfo(ceData.font).pixelSize() + 8;
// 			int sw = 10;
// 			return TQSize(sw, sh);
		}

		default: {
			return TDEStyle::sizeFromContents(ct, ceData, elementFlags, s, o, w);
		}
	}
}

void AsteroidStyle::paletteChanged()
{
}

bool AsteroidStyle::objectEventHandler( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void* source, TQEvent *e )
{
	/*	Win2K has this interesting behaviour where it sets the current
		default button to whatever pushbutton the user presses the mouse
		on. I _think_ this emulates that properly. -clee */
	if (ceData.widgetObjectTypes.contains("TQPushButton")) {
		if (e->type() == TQEvent::MouseButtonPress) {
			widgetActionRequest(ceData, elementFlags, source, WAR_SetDefault);
		}
	}

	return false;
}
