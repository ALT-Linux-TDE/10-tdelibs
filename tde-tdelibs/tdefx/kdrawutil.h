/* This file is part of the KDE libraries
   Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

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
#ifndef __KDRAWUTIL_H
#define __KDRAWUTIL_H

#include <tqnamespace.h>
#include <tqpainter.h>
#include <tqbitmap.h>
#include <tqpalette.h>

#include <tdelibs_export.h>

/*
 * Various drawing routines. Also see Qt's tqdrawutil.h for some more routines
 * contained in Qt.
 *
 * (C) Daniel M. Duley <mosfet@kde.org>
 */

/**
 * @relates TDEStyle
 * @c \#include @c <kdrawutil.h>
 *
 * Draws a Next-style button (solid black shadow with light and midlight highlight).
 * 
 * @param p       The painter to use for drawing the button.
 * @param r       Specifies the rect in which to draw the button.
 * @param g       Specifies the shading colors.
 * @param sunken  Whether to draw the button as sunken (pressed) or not.
 * @param fill    The brush to use for filling the interior of the button.
 *                Pass @a null to prevent the button from being filled.
 */
TDEFX_EXPORT void kDrawNextButton(TQPainter *p, const TQRect &r, const TQColorGroup &g,
                     bool sunken=false, const TQBrush *fill=0);

/**
 * @relates TDEStyle
 * @overload
 */
TDEFX_EXPORT void kDrawNextButton(TQPainter *p, int x, int y, int w, int h,
                     const TQColorGroup &g, bool sunken=false, 
                     const TQBrush *fill=0);

/**
 * @relates TDEStyle
 * @c \#include @c <kdrawutil.h>
 *
 * Draws a Be-style button.
 *
 * @param p       The painter to use for drawing the button.
 * @param r       Specifies the rect in which to draw the button.
 * @param g       Specifies the shading colors.
 * @param sunken  Whether to draw the button as sunken (pressed) or not.
 * @param fill    The brush to use for filling the interior of the button.
 *                Pass @a null to prevent the button from being filled.
 */
TDEFX_EXPORT void kDrawBeButton(TQPainter *p, TQRect &r, const TQColorGroup &g,
                   bool sunken=false, const TQBrush *fill=0);

/**
 * @relates TDEStyle
 * @c \#include @c <kdrawutil.h>
 * @overload
 */
TDEFX_EXPORT void kDrawBeButton(TQPainter *p, int x, int y, int w, int h,
                   const TQColorGroup &g, bool sunken=false, 
                   const TQBrush *fill=0);

/**
 * @relates TDEStyle
 * @c \#include @c <kdrawutil.h>
 *
 * Draws a rounded oval button. This function doesn't fill the button.
 * See kRoundMaskRegion() for setting masks for fills.
 *
 * @param p       The painter to use for drawing the button.
 * @param r       Specifies the rect in which to draw the button.
 * @param g       Specifies the shading colors.
 * @param sunken  Whether to draw the button as sunken (pressed) or not.
 */
TDEFX_EXPORT void kDrawRoundButton(TQPainter *p, const TQRect &r, const TQColorGroup &g,
                      bool sunken=false);

/**
 * @relates TDEStyle
 * @overload
 */
TDEFX_EXPORT void kDrawRoundButton(TQPainter *p, int x, int y, int w, int h,
                      const TQColorGroup &g, bool sunken=false);

/**
 * @relates TDEStyle
 * @c \#include @c <kdrawutil.h>
 *
 * Sets a region to the pixels covered by a round button of the given
 * size. You can use this to set clipping regions.
 * 
 * @param r  Reference to the region to set.
 * @param x  The X coordinate of the button.
 * @param y  The Y coordinate of the button.
 * @param w  The width of the button.
 * @param h  The height of the button.
 *
 * @see kDrawRoundButton() and kDrawRoundMask()
 */
TDEFX_EXPORT void kRoundMaskRegion(TQRegion &r, int x, int y, int w, int h);

/**
 * @relates TDEStyle
 * @c \#include @c <kdrawutil.h>
 *
 * Paints the pixels covered by a round button of the given size with
 * TQt::color1. This function is useful in TQStyle::drawControlMask().
 *
 * @param p      The painter to use for drawing the button.
 * @param x      The X coordinate of the button.
 * @param y      The Y coordinate of the button.
 * @param w      The width of the button.
 * @param h      The height of the button.
 * @param clear  Whether to clear the rectangle specified by @p (x, y, w, h) to
 *               TQt::color0 before drawing the mask.
 */
TDEFX_EXPORT void kDrawRoundMask(TQPainter *p, int x, int y, int w, int h, bool clear=false);

/**
 * @relates TDEStyle
 * @c \#include @c <kdrawutil.h>
 *
 * Paints the provided bitmaps in the painter, using the supplied colorgroup for
 * the foreground colors. There's one bitmap for each color. If you want to skip
 * a color, pass @a null for the corresponding bitmap.
 *
 * @note The bitmaps will be self-masked automatically if not masked
 *       prior to calling this routine.
 *
 * @param p             The painter to use for drawing the bitmaps.
 * @param g             Specifies the shading colors.
 * @param x             The X coordinate at which to draw the bitmaps.
 * @param y             The Y coordinate at which to draw the bitmaps.
 * @param lightColor    The bitmap to use for the light part.
 * @param midColor      The bitmap to use for the mid part.
 * @param midlightColor The bitmap to use for the midlight part.
 * @param darkColor     The bitmap to use for the dark part.
 * @param blackColor    The bitmap to use for the black part.
 * @param whiteColor    The bitmap to use for the white part.
 *
 * @see QColorGroup
 */
TDEFX_EXPORT void kColorBitmaps(TQPainter *p, const TQColorGroup &g, int x, int y,
                   TQBitmap *lightColor=0, TQBitmap *midColor=0,
                   TQBitmap *midlightColor=0, TQBitmap *darkColor=0,
                   TQBitmap *blackColor=0, TQBitmap *whiteColor=0);

/**
 * @relates TDEStyle
 * @c \#include @c <kdrawutil.h>
 * @overload
 */
 TDEFX_EXPORT void kColorBitmaps(TQPainter *p, const TQColorGroup &g, int x, int y, int w,
                   int h, bool isXBitmaps=true, const uchar *lightColor = 0,
                   const uchar *midColor=0, const uchar *midlightColor=0,
                   const uchar *darkColor=0, const uchar *blackColor=0,
                   const uchar *whiteColor=0);

#endif
