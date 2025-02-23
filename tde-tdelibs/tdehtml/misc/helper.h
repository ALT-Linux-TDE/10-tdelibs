/*
 * This file is part of the CSS implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
 *
 */
#ifndef html_helper_h
#define html_helper_h

#include <tqcolor.h>
class TQPainter;
#include <tqfontmetrics.h>
#include <tqfont.h>



namespace tdehtml
{
    class RenderObject;
     const TQRgb transparentColor = 0x00000000;
     const TQRgb invertedColor    = 0x00000002;

    extern TQPainter *printpainter;
    void setPrintPainter( TQPainter *printer );
    
    bool hasSufficientContrast(const TQColor &c1, const TQColor &c2);
    TQColor retrieveBackgroundColor(const RenderObject *obj);
    TQRgb tqRgbaFromHsla(double h, double s, double l, double a);

    //enumerator for findSelectionNode
    enum FindSelectionResult { SelectionPointBefore,
			       SelectionPointAfter,
			       SelectionPointInside,
			       // the next two are only used inside one line in RenderText
			       // to get BiDi contexts right.
			       SelectionPointBeforeInLine,
			       SelectionPointAfterInLine };
}

#endif
