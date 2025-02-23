/*
$Id$

This file is part of the KDE libraries
Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

KDE3 port (C) 2001-2002 Maksim Orlovich <mo002j@mail.rochester.edu>
Port version 0.9.7

Includes code portions from the dotNET style, and the KDE HighColor style.

dotNET Style
 Copyright (C) 2001, Chris Lee        <lee@azsites.com>
                   Carsten Pfeiffer <pfeiffer@kde.org>

KDE3 HighColor Style
Copyright (C) 2001 Karol Szwed       <gallium@kde.org>
  (C) 2001 Fredrik H�glund   <fredrik@kde.org>

Drawing routines adapted from the KDE2 HCStyle,
Copyright (C) 2000 Daniel M. Duley   <mosfet@kde.org>
  (C) 2000 Dirk Mueller      <mueller@kde.org>
  (C) 2001 Martijn Klingens  <klingens@kde.org>


 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License version 2 as published by the Free Software Foundation.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*/
#ifndef KTHEMESTYLE_H
#define KTHEMESTYLE_H

#include <tqglobal.h>

#include "kthemebase.h"
#include <tqwindowdefs.h>
#include <tqobject.h>
#include <tqbutton.h>
#include <tqpushbutton.h>
#include <tqscrollbar.h>
#include <tqtabbar.h>
#include <tqstring.h>
#include <tqintdict.h>
#include <tqmap.h>


/**
 * KDE themed styles.
 *
 * It provides methods for
 * drawing most widgets with user-specified borders, highlights, pixmaps,
 * etc. It also handles various other settings such as scrollbar types,
 * rounded buttons, and shading types. For a full list of parameters this
 * class handles refer to the KDE theme configuration documentation.
 *
 */

class KThemeStyle: public KThemeBase
{
    TQ_OBJECT
public:
    /**
     * Constructs a new KThemeStyle object.
     *
     * @param configDir The directory which has the TDEConfig file.
     * @param configFile A TDEConfig file to use as the theme configuration.
     * Defaults to ~/.tderc.
     */
    KThemeStyle( const TQString& configDir, const TQString &configFile = TQString::null );
    ~KThemeStyle();

    virtual int pixelMetric ( PixelMetric metric, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQWidget * widget = 0 ) const;

    virtual void drawPrimitive ( PrimitiveElement pe, TQPainter * p, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQRect & r, const TQColorGroup & cg,
                                 SFlags flags = Style_Default,
                                 const TQStyleOption& = TQStyleOption::Default ) const;

    virtual void drawControl( ControlElement element,
                              TQPainter *p,
                              const TQStyleControlElementData &ceData,
                              ControlElementFlags elementFlags,
                              const TQRect &r,
                              const TQColorGroup &cg,
                              SFlags how = Style_Default,
                              const TQStyleOption& = TQStyleOption::Default,
                              const TQWidget *widget = 0 ) const;

    virtual void drawControlMask( ControlElement element,
                                  TQPainter *p,
                                  const TQStyleControlElementData &ceData,
                                  ControlElementFlags elementFlags,
                                  const TQRect &r,
                                  const TQStyleOption& = TQStyleOption::Default,
                                  const TQWidget *widget = 0 ) const;


    virtual void drawComplexControl( ComplexControl control,
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

    virtual void drawTDEStylePrimitive( TDEStylePrimitive kpe,
                                      TQPainter* p,
                                      const TQStyleControlElementData &ceData,
                                      ControlElementFlags elementFlags,
                                      const TQRect &r,
                                      const TQColorGroup &cg,
                                      SFlags flags = Style_Default,
                                      const TQStyleOption& = TQStyleOption::Default,
                                      const TQWidget* widget = 0 ) const;


    virtual int styleHint( StyleHint sh,
                           const TQStyleControlElementData &ceData,
                           ControlElementFlags elementFlags,
                           const TQStyleOption& = TQStyleOption::Default,
                           TQStyleHintReturn* returnData = 0,
                           const TQWidget *widget = 0 ) const;

    virtual TQSize sizeFromContents( ContentsType contents,
                                    const TQStyleControlElementData &ceData,
                                    ControlElementFlags elementFlags,
                                    const TQSize &contentsSize,
                                    const TQStyleOption& = TQStyleOption::Default,
                                    const TQWidget *widget = 0 ) const;

    virtual TQRect subRect(SubRect, const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, const TQWidget *) const;

    virtual void polish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void * );
    virtual void unPolish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void * );
    virtual bool objectEventHandler( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void* source, TQEvent *e );
    /**
     * By default this just sets the background brushes to the pixmapped
     * background.
     */
    virtual void applicationPolish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void * );
    virtual void applicationUnPolish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void * );

    /** \internal */
    // to make it possible for derived classes to overload this function
    virtual void polish( TQPalette& pal );

    /**
     * This is a convenience method for drawing widgets with
     * borders, highlights, pixmaps, colors, etc...
     * You specify the widget type and it will draw it according to the
     * config file settings.
     *
     * @param x The x coordinate of the button's upper left hand corner.
     * @param y The y coordinate of the buttons' upper left hand corner.
     * @param w The button width.
     * @param h The button height.
     * @param p The TQPainter to draw on.
     * @param g The color group to use.
     * @param sunken The button is drawn with a sunken style if @p true
     * @param rounded @p true if the widget is rounded, @p false if rectangular.
     * @param type The widget type to paint.
     */
    virtual void drawBaseButton( TQPainter *p, int x, int y, int w, int h,
                                 const TQColorGroup &g, bool sunken = false,
                                 bool rounded = false, WidgetType type = Bevel ) const;
    /**
     * Draw a mask with for widgets that may be rounded.
     *
     *Currently used
     * by pushbuttons and comboboxes.
     *
     * @param p The TQPainter to draw on.
     * @param x The x coordinate of the widget's upper left hand corner.
     * @param y The y coordinate of the widget's upper left hand corner.
     * @param w The widget width.
     * @param h The widget height.
     * @param rounded @p true if the widget is rounded, @p false if rectangular.
     */
    virtual void drawBaseMask( TQPainter *p, int x, int y, int w, int h,
                               bool rounded ) const;



    /**
     * Draw a shaded rectangle using the given style.
     *
     * @param p The painter to draw on.
     * @param g The color group to use.
     * @param x The x coordinate of the rectangle's upper left hand corner.
     * @param y The y coordinate of the rectangle's upper left hand corner.
     * @param w The rectangle width.
     * @param h The rectangle height.
     * @param sunken Draws a sunken style if @p true.
     * @param rounded Draws a rounded shape if @p true. Requires bWidth to be
     * at least 1.
     * @param hWidth The highlight width.
     * @param bWidth The border width.
     * @param style The shading style to use.
     */
    virtual void drawShade( TQPainter *p, int x, int y, int w, int h,
                            const TQColorGroup &g, bool sunken, bool rounded,
                            int hWidth, int bWidth, ShadeStyle style ) const;
    int popupMenuItemHeight( bool checkable, TQMenuItem *mi,
                             const TQFontMetrics &fm );

protected:
    TQPalette oldPalette, popupPalette, indiPalette, exIndiPalette;
    bool paletteSaved;
    bool polishLock;
    TQStyle *mtfstyle;

    TQPixmap* makeMenuBarCache(int w, int h) const;

    mutable TQPixmap* menuCache;
    mutable TQPixmap* vsliderCache;

    TQt::HANDLE brushHandle;
    bool brushHandleSet;
    bool kickerMode;

protected slots:
    void paletteChanged();



};


#endif
