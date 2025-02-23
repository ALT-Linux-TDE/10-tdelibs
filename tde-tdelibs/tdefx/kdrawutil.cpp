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
#include "kdrawutil.h"
#include <tqdrawutil.h>

TDEFX_EXPORT void kDrawNextButton(TQPainter *p, int x, int y, int w, int h,
                     const TQColorGroup &g, bool sunken,
                     const TQBrush *fill)
{
    TQPen oldPen = p->pen();
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->fillRect(x+1, y+1, w-2, h-2,
                fill ? *fill : g.brush(TQColorGroup::Button));
    p->setPen(sunken ? TQt::black : g.light());
    p->drawLine(x, y, x2-1, y);
    p->drawLine(x, y, x, y2-1);
    p->setPen(sunken ? g.midlight() : g.mid());
    p->drawLine(x+1, y2-1, x2-1, y2-1);
    p->drawLine(x2-1, y+1, x2-1, y2-1);
    p->setPen(sunken ? g.light() : TQt::black);
    p->drawLine(x, y2, x2, y2);
    p->drawLine(x2, y, x2, y2);
    p->setPen(oldPen);
}


TDEFX_EXPORT void kDrawNextButton(TQPainter *p, const TQRect &r, const TQColorGroup &g,
                     bool sunken, const TQBrush *fill)
{
    kDrawNextButton(p, r.x(), r.y(), r.width(), r.height(), g, sunken, fill);
}

TDEFX_EXPORT void kDrawBeButton(TQPainter *p, int x, int y, int w, int h,
                   const TQColorGroup &g, bool sunken, const TQBrush *fill)
{
    TQPen oldPen = p->pen();
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->setPen(g.dark());
    p->drawLine(x+1, y, x2-1, y);
    p->drawLine(x, y+1, x, y2-1);
    p->drawLine(x+1, y2, x2-1, y2);
    p->drawLine(x2, y+1, x2, y2-1);


    if(!sunken){
        p->setPen(g.light());
        p->drawLine(x+2, y+2, x2-1, y+2);
        p->drawLine(x+2, y+3, x2-2, y+3);
        p->drawLine(x+2, y+4, x+2, y2-1);
        p->drawLine(x+3, y+4, x+3, y2-2);
    }
    else{
        p->setPen(g.mid());
        p->drawLine(x+2, y+2, x2-1, y+2);
        p->drawLine(x+2, y+3, x2-2, y+3);
        p->drawLine(x+2, y+4, x+2, y2-1);
        p->drawLine(x+3, y+4, x+3, y2-2);
    }


    p->setPen(sunken? g.light() : g.mid());
    p->drawLine(x2-1, y+2, x2-1, y2-1);
    p->drawLine(x+2, y2-1, x2-1, y2-1);

    p->setPen(g.mid());
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->drawLine(x+1, y+2, x+1, y2-1);
    p->drawLine(x2-2, y+3, x2-2, y2-2);

    if(fill)
        p->fillRect(x+4, y+4, w-6, h-6, *fill);
    
    p->setPen(oldPen);
}

TDEFX_EXPORT void kDrawBeButton(TQPainter *p, TQRect &r, const TQColorGroup &g, bool sunken,
                   const TQBrush *fill)
{
    kDrawBeButton(p, r.x(), r.y(), r.width(), r.height(), g, sunken, fill);
}

TDEFX_EXPORT void kDrawRoundButton(TQPainter *p, const TQRect &r, const TQColorGroup &g,
                      bool sunken)
{
    int x, y, x2, y2;
    r.coords(&x, &y, &x2, &y2);
    if(r.width() > 16 && r.height() > 16){
        TQPen oldPen = p->pen();
        TQPointArray hPntArray, lPntArray;
        hPntArray.putPoints(0, 12, x+4,y+1, x+5,y+1, // top left
                            x+3,y+2, x+2,y+3, x+1,y+4, x+1,y+5,
                            x+1,y2-5, x+1,y2-4, x+2,y2-3, // half corners
                            x2-5,y+1, x2-4,y+1, x2-3,y+2);

        lPntArray.putPoints(0, 17, x2-5,y2-1, x2-4,y2-1, // btm right
                            x2-3,y2-2, x2-2,y2-3, x2-1,y2-5, x2-1,y2-4,
 
                            x+3,y2-2, x+4,y2-1, x+5,y2-1, //half corners
                            x2-2,y+3, x2-1,y+4, x2-1,y+5,

                            x2-5,y2-2, x2-4,y2-2, // testing
                            x2-3,y2-3,
                            x2-2,y2-5, x2-2,y2-4);

        p->setPen(sunken ? g.dark() : g.light());
        p->drawLine(x+6, y, x2-6, y);
        p->drawLine(0, y+6, 0, y2-6);
        p->drawPoints(hPntArray);

        p->setPen(sunken ? g.light() : g.dark());
        p->drawLine(x+6, y2, x2-6, y2);
        p->drawLine(x+6, y2-1, x2-6, y2-1);
        p->drawLine(x2, y+6, x2, y2-6);
        p->drawLine(x2-1, y+6, x2-1, y2-6);
        p->drawPoints(lPntArray);
        p->setPen(oldPen);
    }
    else
        qDrawWinPanel(p, x, y, r.width(), r.height(), g, sunken);
}

TDEFX_EXPORT void kDrawRoundButton(TQPainter *p, int x, int y, int w, int h,
                      const TQColorGroup &g, bool sunken)
{
    TQRect r(x, y, w, h);
    kDrawRoundButton(p, r, g, sunken);
}

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(TQCOORD)*2)

TDEFX_EXPORT void kDrawRoundMask(TQPainter *p, int x, int y, int w, int h, bool clear)
{
    // round edge fills
    static const TQCOORD btm_left_fill[]={ 0,0,1,0,2,0,3,0,4,0,0,1,1,1,2,1,3,1,4,1,
    1,2,2,2,3,2,4,2,2,3,3,3,4,3,3,4,4,4 };

    static const TQCOORD btm_right_fill[]={ 0,0,1,0,2,0,3,0,4,0,0,1,1,1,2,1,3,1,4,
    1,0,2,1,2,2,2,3,2,0,3,1,3,2,3,0,4,1,4 };

    static const TQCOORD top_left_fill[]={ 3,0,4,0,2,1,3,1,4,1,1,2,2,2,3,2,4,2,0,3,
    1,3,2,3,3,3,4,3,0,4,1,4,2,4,3,4,4,4 };

    static const TQCOORD top_right_fill[]={ 0,0,1,0,0,1,1,1,2,1,0,2,1,2,2,2,3,2,0,
    3,1,3,2,3,3,3,4,3,0,4,1,4,2,4,3,4,4,4 };

    if(clear)
        p->fillRect(x, y, w, h, TQBrush(TQt::color0, TQt::SolidPattern));
    
    TQBrush fillBrush(TQt::color1, TQt::SolidPattern);
    p->setPen(TQt::color1);
    if(w > 16 && h > 16){
        int x2 = x+w-1;
        int y2 = y+h-1;
        TQPointArray a(QCOORDARRLEN(top_left_fill), top_left_fill);
        a.translate(1, 1);
        p->drawPoints(a);
        a.setPoints(QCOORDARRLEN(btm_left_fill), btm_left_fill);
        a.translate(1, h-6);
        p->drawPoints(a);
        a.setPoints(QCOORDARRLEN(top_right_fill), top_right_fill);
        a.translate(w-6, 1);
        p->drawPoints(a);
        a.setPoints(QCOORDARRLEN(btm_right_fill), btm_right_fill);
        a.translate(w-6, h-6);
        p->drawPoints(a);

        p->fillRect(x+6, y, w-12, h, fillBrush);
        p->fillRect(x, y+6, x+6, h-12, fillBrush);
        p->fillRect(x2-6, y+6, x2, h-12, fillBrush);
        p->drawLine(x+6, y, x2-6, y);
        p->drawLine(x+6, y2, x2-6, y2);
        p->drawLine(x, y+6, x, y2-6);
        p->drawLine(x2, y+6, x2, y2-6);

    }
    else
        p->fillRect(x, y, w, h, fillBrush);
}

TDEFX_EXPORT void kRoundMaskRegion(TQRegion &r, int x, int y, int w, int h)
{
    // using a bunch of TQRect lines seems much more efficient than bitmaps or
    // point arrays, even tho it uses more statements
    r += TQRect(x+6, y+0, w-12, h);
    r += TQRect(x+5, y+1, 1, h-2); // left
    r += TQRect(x+4, y+1, 1, h-2);
    r += TQRect(x+3, y+2, 1, h-4);
    r += TQRect(x+2, y+3, 1, h-6);
    r += TQRect(x+1, y+4, 1, h-8);
    r += TQRect(x, y+6, 1, h-12);
    int x2 = x+w-1;
    r += TQRect(x2-5, y+1, 1, h-2); // right
    r += TQRect(x2-4, y+1, 1, h-2);
    r += TQRect(x2-3, y+2, 1, h-4);
    r += TQRect(x2-2, y+3, 1, h-6);
    r += TQRect(x2-1, y+4, 1, h-8);
    r += TQRect(x2, y+6, 1, h-12);
}

TDEFX_EXPORT void kColorBitmaps(TQPainter *p, const TQColorGroup &g, int x, int y,
                   TQBitmap *lightColor, TQBitmap *midColor,
                   TQBitmap *midlightColor, TQBitmap *darkColor,
                   TQBitmap *blackColor, TQBitmap *whiteColor)
{
    TQBitmap *bitmaps[]={lightColor, midColor, midlightColor, darkColor,
        blackColor, whiteColor};

    TQColor colors[]={g.light(), g.mid(), g.midlight(), g.dark(),
        TQt::black, TQt::white};

    int i;
    for(i=0; i < 6; ++i){
        if(bitmaps[i]){
            if(!bitmaps[i]->mask())
                bitmaps[i]->setMask(*bitmaps[i]);
            p->setPen(colors[i]);
            p->drawPixmap(x, y, *bitmaps[i]);
        }
    }
}

TDEFX_EXPORT void kColorBitmaps(TQPainter *p, const TQColorGroup &g, int x, int y, int w,
                   int h, bool isXBitmaps, const uchar *lightColor,
                   const uchar *midColor, const uchar *midlightColor,
                   const uchar *darkColor, const uchar *blackColor,
                   const uchar *whiteColor)
{
    const uchar *data[]={lightColor, midColor, midlightColor, darkColor,
        blackColor, whiteColor};

    TQColor colors[]={g.light(), g.mid(), g.midlight(), g.dark(),
        TQt::black, TQt::white};

    int i;
    TQBitmap b;
    for(i=0; i < 6; ++i){
        if(data[i]){
            b = TQBitmap(w, h, data[i], isXBitmaps);
            b.setMask(b);
            p->setPen(colors[i]);
            p->drawPixmap(x, y, b);
        }
    }
}



