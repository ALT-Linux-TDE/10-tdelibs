/* Keramik Style for KDE3
   Copyright (c) 2002 Malte Starostik <malte@kde.org>

   based on the KDE3 HighColor Style

   Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
             (C) 2001-2002 Fredrik H�glund  <fredrik@kde.org>

   Drawing routines adapted from the KDE2 HCStyle,
   Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
             (C) 2000 Dirk Mueller          <mueller@kde.org>
             (C) 2001 Martijn Klingens      <klingens@kde.org>

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

#ifndef __keramik_h__
#define __keramik_h__

#include <tqframe.h>
#include <tdestyle.h>

#include "pixmaploader.h"

class TQProgressBar;

class KeramikStyle : public TDEStyle
{
	TQ_OBJECT

public:
	KeramikStyle();
	virtual ~KeramikStyle();

	void renderMenuBlendPixmap( KPixmap& pix, const TQColorGroup &cg, const TQPopupMenu* ) const;
	TQPixmap stylePixmap(StylePixmap stylepixmap, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQStyleOption& opt, const TQWidget* widget = 0) const;

	void polish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void * );
	void unPolish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void * );
	void polish( TQPalette& );
	void applicationPolish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void * );

	void drawTDEStylePrimitive( TDEStylePrimitive kpe,
	                          TQPainter* p,
	                          const TQStyleControlElementData &ceData,
	                          ControlElementFlags elementFlags,
	                          const TQRect& r,
	                          const TQColorGroup& cg,
	                          SFlags flags = Style_Default,
	                          const TQStyleOption& = TQStyleOption::Default,
	                          const TQWidget* widget = 0 ) const;

	void drawPrimitive( PrimitiveElement pe,
	                    TQPainter* p,
	                    const TQStyleControlElementData &ceData,
	                    ControlElementFlags elementFlags,
	                    const TQRect& r,
	                    const TQColorGroup& cg,
	                    SFlags flags = Style_Default,
	                    const TQStyleOption& = TQStyleOption::Default ) const;

	void drawControl( ControlElement element,
	                  TQPainter* p,
	                  const TQStyleControlElementData &ceData,
	                  ControlElementFlags elementFlags,
	                  const TQRect& r,
	                  const TQColorGroup& cg,
	                  SFlags flags = Style_Default,
	                  const TQStyleOption& opt = TQStyleOption::Default,
	                  const TQWidget* widget = 0 ) const;

	void drawControlMask( ControlElement element,
	                      TQPainter* p,
	                      const TQStyleControlElementData &ceData,
	                      ControlElementFlags elementFlags,
	                      const TQRect& r,
	                      const TQStyleOption& opt = TQStyleOption::Default,
	                      const TQWidget* widget = 0 ) const;

	void drawComplexControl( ComplexControl control,
	                         TQPainter* p,
	                         const TQStyleControlElementData &ceData,
	                         ControlElementFlags elementFlags,
	                         const TQRect& r,
	                         const TQColorGroup& cg,
	                         SFlags flags = Style_Default,
	                         SCFlags controls = SC_All,
	                         SCFlags active = SC_None,
	                         const TQStyleOption& = TQStyleOption::Default,
	                         const TQWidget* widget = 0 ) const;

	void drawComplexControlMask( ComplexControl control,
	                             TQPainter* p,
	                             const TQStyleControlElementData &ceData,
	                             const ControlElementFlags elementFlags,
	                             const TQRect& r,
	                             const TQStyleOption& = TQStyleOption::Default,
	                             const TQWidget* widget = 0 ) const;

	int pixelMetric( PixelMetric m, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQWidget* widget = 0 ) const;

	TQSize sizeFromContents( ContentsType contents,
	                        const TQStyleControlElementData &ceData,
	                        ControlElementFlags elementFlags,
	                        const TQSize& contentSize,
	                        const TQStyleOption& opt,
	                        const TQWidget* widget = 0 ) const;

	SubControl querySubControl( ComplexControl control,
	                            const TQStyleControlElementData &ceData,
	                            ControlElementFlags elementFlags,
	                            const TQPoint& point,
						        const TQStyleOption& opt = TQStyleOption::Default,
	                            const TQWidget* widget = 0 ) const;

	TQRect querySubControlMetrics( ComplexControl control,
	                              const TQStyleControlElementData &ceData,
	                              ControlElementFlags elementFlags,
	                              SubControl subcontrol,
	                              const TQStyleOption& opt = TQStyleOption::Default,
	                              const TQWidget* widget = 0 ) const;

	int styleHint(StyleHint, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags,
			const TQStyleOption & = TQStyleOption::Default,
			TQStyleHintReturn * = 0, const TQWidget * = 0 ) const;

private slots:
	//Animation slots.
	void updateProgressPos();
	void progressBarDestroyed(TQObject* bar);

private:

	bool isSizeConstrainedCombo(const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, const TQComboBox* widget) const;
	bool isFormWidget          (const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, const TQWidget* widget) const;

	///Configuration settings
	bool animateProgressBar;
	bool highlightScrollBar;

	//Rendering flags
	mutable bool forceSmallMode;
	mutable bool maskMode;   //Ugly round trip flag to permit masking with little code;
	mutable bool formMode;   //Set when rendering form widgets

	mutable const TQWidget* toolbarBlendWidget;  //Ditto for blending with toolbars

	enum TitleBarMode
	{
		None = 0,
		Regular,
		Maximized
	};

	mutable TitleBarMode titleBarMode; //Set when passing back CC_TilteBar modes to handle
	//PE_ButtonTool properly for them, as well as when handling CC_ToolButton from
	//The maximized window controls.

	mutable bool flatMode; //Set when calling PE_PushButton or PE_ButtonDefault
	// on a flat button.

	mutable bool customScrollMode; //Set when drawing scrollbars with custom colors.
	
	bool firstComboPopupRelease;

	//Animation support.
	TQMap<TQProgressBar*, int> progAnimWidgets;

	virtual bool objectEventHandler( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void* source, TQEvent *e );

	Keramik::TilePainter::PaintMode pmode() const
	{
		if (formMode)
		{
			//If we're a form widget, we blend on painting, and consider ourselves
			//not to have a mask (so we don't get clipped to it)
			if (maskMode)
				return Keramik::TilePainter::PaintTrivialMask;
			else
				return Keramik::TilePainter::PaintFullBlend;
		}
		else
		{
			if (maskMode)
				return Keramik::TilePainter::PaintMask;
			else
				return Keramik::TilePainter::PaintNormal;
		}
	}

	Keramik::TilePainter::PaintMode pmodeFullBlend() const
	{
		return maskMode?Keramik::TilePainter::PaintMask : Keramik::TilePainter::PaintFullBlend;
	}

	bool kickerMode;
	
	// For progress bar animation
	TQTimer *animationTimer;

	TQRect subRect(SubRect r, const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, const TQWidget *widget) const;

	// Disable copy constructor and = operator
	KeramikStyle( const KeramikStyle&  );
	KeramikStyle& operator=( const KeramikStyle&  );
};

#endif
