/*
 * Copyright 2003, Sandro Giessl <ceebx@users.sourceforge.net>
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

#include <tqcolor.h>
#include "misc.h"

TQColor alphaBlendColors(const TQColor &bgColor, const TQColor &fgColor, const int a)
{

    // normal button...
    TQRgb rgb = bgColor.rgb();
    TQRgb rgb_b = fgColor.rgb();
    int alpha = a;
    if(alpha>255) alpha = 255;
    if(alpha<0) alpha = 0;
    int inv_alpha = 255 - alpha;

    TQColor result  = TQColor( tqRgb(tqRed(rgb_b)*inv_alpha/255 + tqRed(rgb)*alpha/255,
                                  tqGreen(rgb_b)*inv_alpha/255 + tqGreen(rgb)*alpha/255,
                                  tqBlue(rgb_b)*inv_alpha/255 + tqBlue(rgb)*alpha/255) );

    return result;
}
