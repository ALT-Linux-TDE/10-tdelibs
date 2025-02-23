/*
 * $Id$
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

#ifndef __TDESTYLE_H
#define __TDESTYLE_H

// W A R N I N G
// -------------
// This API is still subject to change.
// I will remove this warning when I feel the API is sufficiently flexible.

#include <tqcommonstyle.h>

#include <tdelibs_export.h>

class KPixmap;

struct TDEStylePrivate;
/** 
 * Simplifies and extends the TQStyle API to make style coding easier.
 *  
 * The TDEStyle class provides a simple internal menu transparency engine
 * which attempts to use XRender for accelerated blending where requested,
 * or falls back to fast internal software tinting/blending routines.
 * It also simplifies more complex portions of the TQStyle API, such as
 * the PopupMenuItems, ScrollBars and Sliders by providing extra "primitive
 * elements" which are simple to implement by the style writer.
 *
 * @see TQStyle::QStyle
 * @see TQCommonStyle::QCommonStyle
 * @author Karol Szwed (gallium@kde.org)
 * @version $Id$
 */
class TDEFX_EXPORT TDEStyle: public TQCommonStyle
{
	TQ_OBJECT
	

	public:

		/**
		 * TDEStyle Flags:
		 * 
		 * @li Default - Default style setting, where menu transparency
		 * and the FilledFrameWorkaround are disabled.
		 * 
		 * @li AllowMenuTransparency - Enable this flag to use TDEStyle's 
		 * internal menu transparency engine.
		 * 
		 * @li FilledFrameWorkaround - Enable this flag to facilitate 
		 * proper repaints of QMenuBars and QToolBars when the style chooses 
		 * to paint the interior of a TQFrame. The style primitives in question 
		 * are PE_PanelMenuBar and PE_PanelDockWindow. The HighColor style uses
		 * this workaround to enable painting of gradients in menubars and 
		 * toolbars.
		 */
		typedef uint TDEStyleFlags;
		enum TDEStyleOption {
			Default 	      =		0x00000000, //!< All options disabled
			AllowMenuTransparency =		0x00000001, //!< Internal transparency enabled
			FilledFrameWorkaround = 	0x00000002  //!< Filled frames enabled
		};

		/**
		 * TDEStyle ScrollBarType:
		 *
		 * Allows the style writer to easily select what type of scrollbar
		 * should be used without having to duplicate large amounts of source
		 * code by implementing the complex control CC_ScrollBar.
		 *
		 * @li WindowsStyleScrollBar - Two button scrollbar with the previous
		 * button at the top/left, and the next button at the bottom/right.
		 *
		 * @li PlatinumStyleScrollBar - Two button scrollbar with both the 
		 * previous and next buttons at the bottom/right.
		 *
		 * @li ThreeButtonScrollBar - %KDE style three button scrollbar with
		 * two previous buttons, and one next button. The next button is always
		 * at the bottom/right, whilst the two previous buttons are on either 
		 * end of the scrollbar.
		 *
		 * @li NextStyleScrollBar - Similar to the PlatinumStyle scroll bar, but
		 * with the buttons grouped on the opposite end of the scrollbar.
		 *
		 * @see TDEStyle::TDEStyle()
		 */
		enum TDEStyleScrollBarType {
			WindowsStyleScrollBar  = 	0x00000000, //!< two button, windows style
			PlatinumStyleScrollBar = 	0x00000001, //!< two button, platinum style
			ThreeButtonScrollBar   = 	0x00000002, //!< three buttons, %KDE style
			NextStyleScrollBar     = 	0x00000004  //!< two button, NeXT style
		};

		/** 
		 * Constructs a TDEStyle object.
		 *
		 * Select the appropriate TDEStyle flags and scrollbar type
		 * for your style. The user's style preferences selected in KControl
		 * are read by using TQSettings and are automatically applied to the style.
		 * As a fallback, TDEStyle paints progressbars and tabbars. It inherits from
		 * TQCommonStyle for speed, so don't expect much to be implemented. 
		 *
		 * It is advisable to use a currently implemented style such as the HighColor
		 * style as a foundation for any new TDEStyle, so the limited number of
		 * drawing fallbacks should not prove problematic.
		 *
		 * @param flags the style to be applied
		 * @param sbtype the scroll bar type
		 * @see TDEStyle::TDEStyleFlags
		 * @see TDEStyle::TDEStyleScrollBarType
		 * @author Karol Szwed (gallium@kde.org)
		 */
		TDEStyle( TDEStyleFlags flags = TDEStyle::Default, 
			TDEStyleScrollBarType sbtype = TDEStyle::WindowsStyleScrollBar );

		/** 
		 * Destructs the TDEStyle object.
		 */
		~TDEStyle();

		/**
		 * Returns the default widget style depending on color depth.
		 */
		static TQString defaultStyle();

		/**
		 * Modifies the scrollbar type used by the style.
		 * 
		 * This function is only provided for convenience. It allows
		 * you to make a late decision about what scrollbar type to use for the
		 * style after performing some processing in your style's constructor.
		 * In most situations however, setting the scrollbar type via the TDEStyle
		 * constructor should suffice.
		 * @param sbtype the scroll bar type
		 * @see TDEStyle::TDEStyleScrollBarType
		 */
		void setScrollBarType(TDEStyleScrollBarType sbtype);

		/**
		 * Returns the TDEStyle flags used to initialize the style.
		 *
		 * This is used solely for the kcmstyle module, and hence is internal.
		 */
		TDEStyleFlags styleFlags() const;

		// ---------------------------------------------------------------------------

		/**
		 * This virtual function defines the pixmap used to blend between the popup
		 * menu and the background to create different menu transparency effects.
		 * For example, you can fill the pixmap "pix" with a gradient based on the
		 * popup's colorGroup, a texture, or some other fancy painting routine.
		 * TDEStyle will then internally blend this pixmap with a snapshot of the
		 * background behind the popupMenu to create the illusion of transparency.
		 * 
		 * This virtual is never called if XRender/Software blending is disabled by
		 * the user in KDE's style control module.
		 */
		virtual void renderMenuBlendPixmap( KPixmap& pix, const TQColorGroup& cg, 
						    const TQPopupMenu* popup ) const;

		/**
		 * TDEStyle Primitive Elements:
		 *
		 * The TDEStyle class extends the Qt's Style API by providing certain 
		 * simplifications for parts of TQStyle. To do this, the TDEStylePrimitive
		 * elements were defined, which are very similar to Qt's PrimitiveElement.
		 * 
		 * The first three Handle primitives simplify and extend PE_DockWindowHandle, 
		 * so do not reimplement PE_DockWindowHandle if you want the TDEStyle handle 
		 * simplifications to be operable. Similarly do not reimplement CC_Slider,
		 * SC_SliderGroove and SC_SliderHandle when using the TDEStyle slider
		 * primitives. TDEStyle automatically double-buffers slider painting
		 * when they are drawn via these TDEStyle primitives to avoid flicker.
		 *
		 * @li KPE_DockWindowHandle - This primitive is already implemented in TDEStyle,
		 * and paints a bevelled rect with the DockWindow caption text. Re-implement
		 * this primitive to perform other more fancy effects when drawing the dock window
		 * handle.
		 *
		 * @li KPE_ToolBarHandle - This primitive must be reimplemented. It currently
		 * only paints a filled rectangle as default behavior. This primitive is used
		 * to render TQToolBar handles.
		 *
		 * @li KPE_GeneralHandle - This primitive must be reimplemented. It is used
		 * to render general handles that are not part of a TQToolBar or TQDockWindow, such
		 * as the applet handles used in Kicker. The default implementation paints a filled
		 * rect of arbitrary color.
		 *
		 * @li KPE_SliderGroove - This primitive must be reimplemented. It is used to 
		 * paint the slider groove. The default implementation paints a filled rect of
		 * arbitrary color.
		 *
		 * @li KPE_SliderHandle - This primitive must be reimplemented. It is used to
		 * paint the slider handle. The default implementation paints a filled rect of
		 * arbitrary color.
		 *
		 * @li KPE_ListViewExpander - This primitive is already implemented in TDEStyle. It
		 * is used to draw the Expand/Collapse element in QListViews. To indicate the 
		 * expanded state, the style flags are set to Style_Off, while Style_On implies collapsed.
		 *
		 * @li KPE_ListViewBranch - This primitive is already implemented in TDEStyle. It is
		 * used to draw the ListView branches where necessary.
		 */
		enum TDEStylePrimitive {
			KPE_DockWindowHandle,
			KPE_ToolBarHandle,
			KPE_GeneralHandle,

			KPE_SliderGroove,
			KPE_SliderHandle,

			KPE_ListViewExpander,
			KPE_ListViewBranch
		};

		// Old API
		// DEPRECATED
		virtual void drawTDEStylePrimitive( TDEStylePrimitive kpe,
					TQPainter* p,
					const TQWidget* widget,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					const TQStyleOption& = TQStyleOption::Default ) const;

		/**
		 * This function is identical to Qt's TQStyle::drawPrimitive(), except that 
		 * it adds one further parameter, 'widget', that can be used to determine 
		 * the widget state of the TDEStylePrimitive in question.
		 *
		 * @see TDEStyle::TDEStylePrimitive
		 * @see TQStyle::drawPrimitive
		 * @see TQStyle::drawComplexControl
		 */
		virtual void drawTDEStylePrimitive( TDEStylePrimitive kpe,
					TQPainter* p,
					const TQStyleControlElementData &ceData,
					ControlElementFlags elementFlags,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					const TQStyleOption& = TQStyleOption::Default,
					const TQWidget* widget = 0 ) const;


		enum TDEStylePixelMetric {
			KPM_MenuItemSeparatorHeight		= 0x00000001,
			KPM_MenuItemHMargin			= 0x00000002,
			KPM_MenuItemVMargin			= 0x00000004,
			KPM_MenuItemHFrame			= 0x00000008,
			KPM_MenuItemVFrame			= 0x00000010,
			KPM_MenuItemCheckMarkHMargin	        = 0x00000020,
			KPM_MenuItemArrowHMargin		= 0x00000040,
			KPM_MenuItemTabSpacing			= 0x00000080,
			KPM_ListViewBranchThickness		= 0x00000100
		};

		int kPixelMetric( TDEStylePixelMetric kpm, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQWidget* widget = 0 ) const;

		// ---------------------------------------------------------------------------

		void polish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void * );
		void unPolish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void * );
		void polishPopupMenu( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr );

		void drawPrimitive( PrimitiveElement pe,
					TQPainter* p,
					const TQStyleControlElementData &ceData,
					ControlElementFlags elementFlags,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					const TQStyleOption& = TQStyleOption::Default ) const;

		void drawControl( ControlElement element,
					TQPainter* p,
					const TQStyleControlElementData &ceData,
					ControlElementFlags elementFlags,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					const TQStyleOption& = TQStyleOption::Default,
					const TQWidget* widget = 0 ) const;

		void drawComplexControl( ComplexControl control,
					TQPainter *p,
					const TQStyleControlElementData &ceData,
					ControlElementFlags elementFlags,
					const TQRect &r,
					const TQColorGroup &cg,
					SFlags flags = Style_Default,
					SCFlags controls = SC_All,
					SCFlags active = SC_None,
					const TQStyleOption& = TQStyleOption::Default,
					const TQWidget* widget = 0 ) const;

		SubControl querySubControl( ComplexControl control,
					const TQStyleControlElementData &ceData,
					ControlElementFlags elementFlags,
					const TQPoint &pos,
					const TQStyleOption& = TQStyleOption::Default,
					const TQWidget* widget = 0 ) const;

		TQRect querySubControlMetrics( ComplexControl control,
					const TQStyleControlElementData &ceData,
					ControlElementFlags elementFlags,
					SubControl sc,
					const TQStyleOption& = TQStyleOption::Default,
					const TQWidget* widget = 0 ) const;

		int pixelMetric( PixelMetric m, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, 
					const TQWidget* widget = 0 ) const;

		TQRect subRect( SubRect r, const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, 
					const TQWidget* widget = 0 ) const;

		TQPixmap stylePixmap( StylePixmap stylepixmap,
					const TQStyleControlElementData &ceData,
					ControlElementFlags elementFlags,
					const TQStyleOption& = TQStyleOption::Default,
					const TQWidget* widget = 0 ) const;

		int styleHint( StyleHint sh, const TQStyleControlElementData &ceData,
					ControlElementFlags elementFlags,
					const TQStyleOption &opt = TQStyleOption::Default,
					TQStyleHintReturn* shr = 0,
					const TQWidget* w = 0 ) const;

	protected:
		virtual bool objectEventHandler( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void* source, TQEvent *e );

	private:
		// Disable copy constructor and = operator
		TDEStyle( const TDEStyle & );
		TDEStyle& operator=( const TDEStyle & );

	protected:
		virtual void virtual_hook( int id, void* data );
	private:
		TDEStylePrivate *d;
};
#endif

