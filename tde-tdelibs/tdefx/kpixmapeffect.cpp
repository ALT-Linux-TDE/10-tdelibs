/* This file is part of the KDE libraries
    Copyright (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
              (C) 1998, 1999 Daniel M. Duley <mosfet@kde.org>
              (C) 1998, 1999 Dirk A. Mueller <mueller@kde.org>

*/

// $Id$

#include <tqimage.h>
#include <tqpainter.h>

#include "kpixmapeffect.h"
#include "kpixmap.h"
#include "kimageeffect.h"

//======================================================================
//
// Gradient effects
//
//======================================================================


KPixmap& KPixmapEffect::gradient(KPixmap &pixmap, const TQColor &ca,
	const TQColor &cb, GradientType eff, int ncols)
{
    if(pixmap.depth() > 8 &&
       (eff == VerticalGradient || eff == HorizontalGradient)) {

        int rDiff, gDiff, bDiff;
        int rca, gca, bca /*, rcb, gcb, bcb*/;

        int x, y;

        rDiff = (/*rcb = */ cb.red())   - (rca = ca.red());
        gDiff = (/*gcb = */ cb.green()) - (gca = ca.green());
        bDiff = (/*bcb = */ cb.blue())  - (bca = ca.blue());

        int rl = rca << 16;
        int gl = gca << 16;
        int bl = bca << 16;

        int rcdelta = ((1<<16) / (eff == VerticalGradient ? pixmap.height() : pixmap.width())) * rDiff;
        int gcdelta = ((1<<16) / (eff == VerticalGradient ? pixmap.height() : pixmap.width())) * gDiff;
        int bcdelta = ((1<<16) / (eff == VerticalGradient ? pixmap.height() : pixmap.width())) * bDiff;

        TQPainter p(&pixmap);

        // these for-loops could be merged, but the if's in the inner loop
        // would make it slow
        switch(eff) {
        case VerticalGradient:
            for ( y = 0; y < pixmap.height(); y++ ) {
                rl += rcdelta;
                gl += gcdelta;
                bl += bcdelta;

                p.setPen(TQColor(rl>>16, gl>>16, bl>>16));
                p.drawLine(0, y, pixmap.width()-1, y);
            }
            break;
        case HorizontalGradient:
            for( x = 0; x < pixmap.width(); x++) {
                rl += rcdelta;
                gl += gcdelta;
                bl += bcdelta;

                p.setPen(TQColor(rl>>16, gl>>16, bl>>16));
                p.drawLine(x, 0, x, pixmap.height()-1);
            }
            break;
        default:
            ;
        }
    }
    else {
        TQImage image = KImageEffect::gradient(pixmap.size(), ca, cb,
                                              (KImageEffect::GradientType) eff, ncols);
        pixmap.convertFromImage(image);
    }

    return pixmap;
}


// -----------------------------------------------------------------------------

KPixmap& KPixmapEffect::unbalancedGradient(KPixmap &pixmap, const TQColor &ca,
         const TQColor &cb, GradientType eff, int xfactor, int yfactor,
         int ncols)
{
    TQImage image = KImageEffect::unbalancedGradient(pixmap.size(), ca, cb,
                                 (KImageEffect::GradientType) eff,
                                 xfactor, yfactor, ncols);
    pixmap.convertFromImage(image);

    return pixmap;
}


//======================================================================
//
// Intensity effects
//
//======================================================================



KPixmap& KPixmapEffect::intensity(KPixmap &pixmap, float percent)
{
    TQImage image = pixmap.convertToImage();
    KImageEffect::intensity(image, percent);
    pixmap.convertFromImage(image);

    return pixmap;
}


// -----------------------------------------------------------------------------

KPixmap& KPixmapEffect::channelIntensity(KPixmap &pixmap, float percent,
                                     RGBComponent channel)
{
    TQImage image = pixmap.convertToImage();
    KImageEffect::channelIntensity(image, percent,
                   (KImageEffect::RGBComponent) channel);
    pixmap.convertFromImage(image);

    return pixmap;
}


//======================================================================
//
// Blend effects
//
//======================================================================


KPixmap& KPixmapEffect::blend(KPixmap &pixmap, float initial_intensity,
			  const TQColor &bgnd, GradientType eff,
			  bool anti_dir, int ncols)
{

    TQImage image = pixmap.convertToImage();
    if (image.depth() <=8)
        image = image.convertDepth(32); //Sloww..

    KImageEffect::blend(image, initial_intensity, bgnd,
                  (KImageEffect::GradientType) eff, anti_dir);

    unsigned int tmp;

    if(pixmap.depth() <= 8 ) {
        if ( ncols < 2 || ncols > 256 )
            ncols = 3;
        TQColor *dPal = new TQColor[ncols];
        for (int i=0; i<ncols; i++) {
            tmp = 0 + 255 * i / ( ncols - 1 );
            dPal[i].setRgb ( tmp, tmp, tmp );
        }
        KImageEffect::dither(image, dPal, ncols);
        pixmap.convertFromImage(image);
        delete [] dPal;
    }
    else
        pixmap.convertFromImage(image);

    return pixmap;
}


//======================================================================
//
// Hash effects
//
//======================================================================

KPixmap& KPixmapEffect::hash(KPixmap &pixmap, Lighting lite,
			 unsigned int spacing, int ncols)
{
    TQImage image = pixmap.convertToImage();
    KImageEffect::hash(image, (KImageEffect::Lighting) lite, spacing);

    unsigned int tmp;

    if(pixmap.depth() <= 8 ) {
        if ( ncols < 2 || ncols > 256 )
            ncols = 3;
        TQColor *dPal = new TQColor[ncols];
        for (int i=0; i<ncols; i++) {
            tmp = 0 + 255 * i / ( ncols - 1 );
            dPal[i].setRgb ( tmp, tmp, tmp );
        }
        KImageEffect::dither(image, dPal, ncols);
        pixmap.convertFromImage(image);
        delete [] dPal;
    }
    else
        pixmap.convertFromImage(image);

    return pixmap;
}


//======================================================================
//
// Pattern effects
//
//======================================================================

#if 0
void KPixmapEffect::pattern(KPixmap &pixmap, const TQColor &ca,
	const TQColor &cb, unsigned pat[8])
{
    TQImage img = pattern(pixmap.size(), ca, cb, pat);
    pixmap.convertFromImage(img);
}
#endif

// -----------------------------------------------------------------------------

KPixmap KPixmapEffect::pattern(const KPixmap& pmtile, TQSize size,
                       const TQColor &ca, const TQColor &cb, int ncols)
{
    if (pmtile.depth() > 8)
	ncols = 0;

    TQImage img = pmtile.convertToImage();
    KImageEffect::flatten(img, ca, cb, ncols);
    KPixmap pixmap;
    pixmap.convertFromImage(img);

    return KPixmapEffect::createTiled(pixmap, size);
}


// -----------------------------------------------------------------------------

KPixmap KPixmapEffect::createTiled(const KPixmap& pixmap, TQSize size)
{
    KPixmap pix(size);

    TQPainter p(&pix);
    p.drawTiledPixmap(0, 0, size.width(), size.height(), pixmap);

    return pix;
}


//======================================================================
//
// Fade effects
//
//======================================================================

KPixmap& KPixmapEffect::fade(KPixmap &pixmap, double val, const TQColor &color)
{
    TQImage img = pixmap.convertToImage();
    KImageEffect::fade(img, val, color);
    pixmap.convertFromImage(img);

    return pixmap;
}


// -----------------------------------------------------------------------------
KPixmap& KPixmapEffect::toGray(KPixmap &pixmap, bool fast)
{
    TQImage img = pixmap.convertToImage();
    KImageEffect::toGray(img, fast);
    pixmap.convertFromImage(img);

    return pixmap;
}

// -----------------------------------------------------------------------------
KPixmap& KPixmapEffect::desaturate(KPixmap &pixmap, float desat)
{
    TQImage img = pixmap.convertToImage();
    KImageEffect::desaturate(img, desat);
    pixmap.convertFromImage(img);

    return pixmap;
}
// -----------------------------------------------------------------------------
KPixmap& KPixmapEffect::contrast(KPixmap &pixmap, int c)
{
    TQImage img = pixmap.convertToImage();
    KImageEffect::contrast(img, c);
    pixmap.convertFromImage(img);

    return pixmap;
}

//======================================================================
//
// Dither effects
//
//======================================================================

// -----------------------------------------------------------------------------
KPixmap& KPixmapEffect::dither(KPixmap &pixmap, const TQColor *palette, int size)
{
    TQImage img = pixmap.convertToImage();
    KImageEffect::dither(img, palette, size);
    pixmap.convertFromImage(img);

    return pixmap;
}

//======================================================================
//
// Other effects
//
//======================================================================

KPixmap KPixmapEffect::selectedPixmap( const KPixmap &pix, const TQColor &col )
{
    TQImage img = pix.convertToImage();
    KImageEffect::selectedImage(img, col);
    KPixmap outPix;
    outPix.convertFromImage(img);
    return outPix;
}
