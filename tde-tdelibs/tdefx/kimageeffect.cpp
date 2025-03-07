/* This file is part of the KDE libraries
    Copyright (C) 1998, 1999, 2001, 2002 Daniel M. Duley <mosfet@kde.org>
    (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
    (C) 1998, 1999 Dirk Mueller <mueller@kde.org>
    (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
    (C) 2000 Josef Weidendorfer <weidendo@in.tum.de>
    (C) 2004 Zack Rusin <zack@kde.org>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

// $Id$

#include <math.h>
#include <assert.h>

#include <tqimage.h>
#include <stdlib.h>
#include <iostream>

#include "kimageeffect.h"
#include "kcpuinfo.h"

#include <config.h>

#if 0
//disabled until #74478 fixed.

#if defined(__i386__) && ( defined(__GNUC__) || defined(__INTEL_COMPILER) )
#  if defined( HAVE_X86_MMX )
#    define USE_MMX_INLINE_ASM
#  endif
#  if defined( HAVE_X86_SSE2 )
#    define USE_SSE2_INLINE_ASM
#  endif
#endif

#endif
//======================================================================
//
// Utility stuff for effects ported from ImageMagick to TQImage
//
//======================================================================
#define MaxRGB 255L
#define DegreesToRadians(x) ((x)*M_PI/180.0)
#define MagickSQ2PI 2.50662827463100024161235523934010416269302368164062
#define MagickEpsilon  1.0e-12
#define MagickPI  3.14159265358979323846264338327950288419716939937510
#define MOD(x, y) ((x) < 0 ? ((y) - 1 - ((y) - 1 - (x)) % (y)) : (x) % (y))

/**
 * \relates TDEGlobal
 * A typesafe function that returns x if it's between low and high values.
 * low if x is smaller than then low and high if x is bigger than high.
 */
#define FXCLAMP(x,low,high) fxClamp(x,low,high)
template<class T>
inline const T& fxClamp( const T& x, const T& low, const T& high )
{
    if ( x < low )       return low;
    else if ( x > high ) return high;
    else                 return x;
}

static inline unsigned int intensityValue(unsigned int color)
{
    return((unsigned int)((0.299*tqRed(color) +
                           0.587*tqGreen(color) +
                           0.1140000000000001*tqBlue(color))));
}

template<typename T>
static inline void liberateMemory(T **memory)
{
    assert(memory != NULL);
    if(*memory == NULL) return;
    free((char*)*memory);
    *memory=NULL;
}

struct double_packet
{
    double red;
    double green;
    double blue;
    double alpha;
};

struct short_packet
{
    unsigned short int red;
    unsigned short int green;
    unsigned short int blue;
    unsigned short int alpha;
};


//======================================================================
//
// Gradient effects
//
//======================================================================

TQImage KImageEffect::gradient(const TQSize &size, const TQColor &ca,
	const TQColor &cb, GradientType eff, int ncols)
{
    int rDiff, gDiff, bDiff;
    int rca, gca, bca, rcb, gcb, bcb;

    TQImage image(size, 32);

    if (size.width() == 0 || size.height() == 0) {
#ifndef NDEBUG
      std::cerr << "WARNING: KImageEffect::gradient: invalid image" << std::endl;
#endif
      return image;
    }

    int x, y;

    rDiff = (rcb = cb.red())   - (rca = ca.red());
    gDiff = (gcb = cb.green()) - (gca = ca.green());
    bDiff = (bcb = cb.blue())  - (bca = ca.blue());

    if( eff == VerticalGradient || eff == HorizontalGradient ){

        uint *p;
        uint rgb;

        int rl = rca << 16;
        int gl = gca << 16;
        int bl = bca << 16;

        if( eff == VerticalGradient ) {

            int rcdelta = ((1<<16) / size.height()) * rDiff;
            int gcdelta = ((1<<16) / size.height()) * gDiff;
            int bcdelta = ((1<<16) / size.height()) * bDiff;

            for ( y = 0; y < size.height(); y++ ) {
                p = (uint *) image.scanLine(y);

                rl += rcdelta;
                gl += gcdelta;
                bl += bcdelta;

                rgb = tqRgb( (rl>>16), (gl>>16), (bl>>16) );

                for( x = 0; x < size.width(); x++ ) {
                    *p = rgb;
                    p++;
                }
            }

        }
        else {                  // must be HorizontalGradient

            unsigned int *o_src = (unsigned int *)image.scanLine(0);
            unsigned int *src = o_src;

            int rcdelta = ((1<<16) / size.width()) * rDiff;
            int gcdelta = ((1<<16) / size.width()) * gDiff;
            int bcdelta = ((1<<16) / size.width()) * bDiff;

            for( x = 0; x < size.width(); x++) {

                rl += rcdelta;
                gl += gcdelta;
                bl += bcdelta;

                *src++ = tqRgb( (rl>>16), (gl>>16), (bl>>16));
            }

            src = o_src;

            // Believe it or not, manually copying in a for loop is faster
            // than calling memcpy for each scanline (on the order of ms...).
            // I think this is due to the function call overhead (mosfet).

            for (y = 1; y < size.height(); ++y) {

                p = (unsigned int *)image.scanLine(y);
                src = o_src;
                for(x=0; x < size.width(); ++x)
                    *p++ = *src++;
            }
        }
    }

    else {

        float rfd, gfd, bfd;
        float rd = rca, gd = gca, bd = bca;

        unsigned char *xtable[3];
        unsigned char *ytable[3];

        unsigned int w = size.width(), h = size.height();
        xtable[0] = new unsigned char[w];
        xtable[1] = new unsigned char[w];
        xtable[2] = new unsigned char[w];
        ytable[0] = new unsigned char[h];
        ytable[1] = new unsigned char[h];
        ytable[2] = new unsigned char[h];
        w*=2, h*=2;

        if ( eff == DiagonalGradient || eff == CrossDiagonalGradient) {
            // Diagonal dgradient code inspired by BlackBox (mosfet)
            // BlackBox dgradient is (C) Brad Hughes, <bhughes@tcac.net> and
            // Mike Cole <mike@mydot.com>.

            rfd = (float)rDiff/w;
            gfd = (float)gDiff/w;
            bfd = (float)bDiff/w;

            int dir;
            for (x = 0; x < size.width(); x++, rd+=rfd, gd+=gfd, bd+=bfd) {
                dir = eff == DiagonalGradient? x : size.width() - x - 1;
                xtable[0][dir] = (unsigned char) rd;
                xtable[1][dir] = (unsigned char) gd;
                xtable[2][dir] = (unsigned char) bd;
            }
            rfd = (float)rDiff/h;
            gfd = (float)gDiff/h;
            bfd = (float)bDiff/h;
            rd = gd = bd = 0;
            for (y = 0; y < size.height(); y++, rd+=rfd, gd+=gfd, bd+=bfd) {
                ytable[0][y] = (unsigned char) rd;
                ytable[1][y] = (unsigned char) gd;
                ytable[2][y] = (unsigned char) bd;
            }

            for (y = 0; y < size.height(); y++) {
                unsigned int *scanline = (unsigned int *)image.scanLine(y);
                for (x = 0; x < size.width(); x++) {
                    scanline[x] = tqRgb(xtable[0][x] + ytable[0][y],
                                       xtable[1][x] + ytable[1][y],
                                       xtable[2][x] + ytable[2][y]);
                }
            }
        }

        else if (eff == RectangleGradient ||
                 eff == PyramidGradient ||
                 eff == PipeCrossGradient ||
                 eff == EllipticGradient)
        {
            int rSign = rDiff>0? 1: -1;
            int gSign = gDiff>0? 1: -1;
            int bSign = bDiff>0? 1: -1;

            rfd = (float)rDiff / size.width();
            gfd = (float)gDiff / size.width();
            bfd = (float)bDiff / size.width();

            rd = (float)rDiff/2;
            gd = (float)gDiff/2;
            bd = (float)bDiff/2;

            for (x = 0; x < size.width(); x++, rd-=rfd, gd-=gfd, bd-=bfd)
            {
                xtable[0][x] = (unsigned char) abs((int)rd);
                xtable[1][x] = (unsigned char) abs((int)gd);
                xtable[2][x] = (unsigned char) abs((int)bd);
            }

            rfd = (float)rDiff/size.height();
            gfd = (float)gDiff/size.height();
            bfd = (float)bDiff/size.height();

            rd = (float)rDiff/2;
            gd = (float)gDiff/2;
            bd = (float)bDiff/2;

            for (y = 0; y < size.height(); y++, rd-=rfd, gd-=gfd, bd-=bfd)
            {
                ytable[0][y] = (unsigned char) abs((int)rd);
                ytable[1][y] = (unsigned char) abs((int)gd);
                ytable[2][y] = (unsigned char) abs((int)bd);
            }

            int h = (size.height()+1)>>1;
            for (y = 0; y < h; y++) {
                unsigned int *sl1 = (unsigned int *)image.scanLine(y);
                unsigned int *sl2 = (unsigned int *)image.scanLine(TQMAX(size.height()-y-1, y));

                int w = (size.width()+1)>>1;
                int x2 = size.width()-1;

                for (x = 0; x < w; x++, x2--) {
		    unsigned int rgb = 0;
                    if (eff == PyramidGradient) {
                        rgb = tqRgb(rcb-rSign*(xtable[0][x]+ytable[0][y]),
                                   gcb-gSign*(xtable[1][x]+ytable[1][y]),
                                   bcb-bSign*(xtable[2][x]+ytable[2][y]));
                    }
                    if (eff == RectangleGradient) {
                        rgb = tqRgb(rcb - rSign *
                                   TQMAX(xtable[0][x], ytable[0][y]) * 2,
                                   gcb - gSign *
                                   TQMAX(xtable[1][x], ytable[1][y]) * 2,
                                   bcb - bSign *
                                   TQMAX(xtable[2][x], ytable[2][y]) * 2);
                    }
                    if (eff == PipeCrossGradient) {
                        rgb = tqRgb(rcb - rSign *
                                   TQMIN(xtable[0][x], ytable[0][y]) * 2,
                                   gcb - gSign *
                                   TQMIN(xtable[1][x], ytable[1][y]) * 2,
                                   bcb - bSign *
                                   TQMIN(xtable[2][x], ytable[2][y]) * 2);
                    }
                    if (eff == EllipticGradient) {
                        rgb = tqRgb(rcb - rSign *
                                   (int)sqrt((xtable[0][x]*xtable[0][x] +
                                              ytable[0][y]*ytable[0][y])*2.0),
                                   gcb - gSign *
                                   (int)sqrt((xtable[1][x]*xtable[1][x] +
                                              ytable[1][y]*ytable[1][y])*2.0),
                                   bcb - bSign *
                                   (int)sqrt((xtable[2][x]*xtable[2][x] +
                                              ytable[2][y]*ytable[2][y])*2.0));
                    }

                    sl1[x] = sl2[x] = rgb;
                    sl1[x2] = sl2[x2] = rgb;
                }
            }
        }

        delete [] xtable[0];
        delete [] xtable[1];
        delete [] xtable[2];
        delete [] ytable[0];
        delete [] ytable[1];
        delete [] ytable[2];
    }

    // dither if necessary
    if (ncols && (TQPixmap::defaultDepth() < 15 )) {
	if ( ncols < 2 || ncols > 256 )
	    ncols = 3;
	TQColor *dPal = new TQColor[ncols];
	for (int i=0; i<ncols; i++) {
	    dPal[i].setRgb ( rca + rDiff * i / ( ncols - 1 ),
			     gca + gDiff * i / ( ncols - 1 ),
			     bca + bDiff * i / ( ncols - 1 ) );
	}
        dither(image, dPal, ncols);
        delete [] dPal;
    }

    return image;
}


// -----------------------------------------------------------------------------

//CT this was (before Dirk A. Mueller's speedup changes)
//   merely the same code as in the above method, but it's supposedly
//   way less performant since it introduces a lot of supplementary tests
//   and simple math operations for the calculus of the balance.
//      (surprizingly, it isn't less performant, in the contrary :-)
//   Yes, I could have merged them, but then the excellent performance of
//   the balanced code would suffer with no other gain than a mere
//   source code and byte code size economy.

TQImage KImageEffect::unbalancedGradient(const TQSize &size, const TQColor &ca,
	const TQColor &cb, GradientType eff, int xfactor, int yfactor,
	int ncols)
{
    int dir; // general parameter used for direction switches

    bool _xanti = false , _yanti = false;

    if (xfactor < 0) _xanti = true; // negative on X direction
    if (yfactor < 0) _yanti = true; // negative on Y direction

    xfactor = abs(xfactor);
    yfactor = abs(yfactor);

    if (!xfactor) xfactor = 1;
    if (!yfactor) yfactor = 1;

    if (xfactor > 200 ) xfactor = 200;
    if (yfactor > 200 ) yfactor = 200;


    //    float xbal = xfactor/5000.;
    //    float ybal = yfactor/5000.;
    float xbal = xfactor/30./size.width();
    float ybal = yfactor/30./size.height();
    float rat;

    int rDiff, gDiff, bDiff;
    int rca, gca, bca, rcb, gcb, bcb;

    TQImage image(size, 32);

    if (size.width() == 0 || size.height() == 0) {
#ifndef NDEBUG
      std::cerr << "WARNING: KImageEffect::unbalancedGradient : invalid image\n";
#endif
      return image;
    }

    int x, y;
    unsigned int *scanline;

    rDiff = (rcb = cb.red())   - (rca = ca.red());
    gDiff = (gcb = cb.green()) - (gca = ca.green());
    bDiff = (bcb = cb.blue())  - (bca = ca.blue());

    if( eff == VerticalGradient || eff == HorizontalGradient){
        TQColor cRow;

        uint *p;
        uint rgbRow;

	if( eff == VerticalGradient) {
	  for ( y = 0; y < size.height(); y++ ) {
	    dir = _yanti ? y : size.height() - 1 - y;
            p = (uint *) image.scanLine(dir);
            rat =  1 - exp( - (float)y  * ybal );

            cRow.setRgb( rcb - (int) ( rDiff * rat ),
                         gcb - (int) ( gDiff * rat ),
                         bcb - (int) ( bDiff * rat ) );

            rgbRow = cRow.rgb();

            for( x = 0; x < size.width(); x++ ) {
	      *p = rgbRow;
	      p++;
            }
	  }
	}
	else {

	  unsigned int *src = (unsigned int *)image.scanLine(0);
	  for(x = 0; x < size.width(); x++ )
          {
	      dir = _xanti ? x : size.width() - 1 - x;
	      rat = 1 - exp( - (float)x  * xbal );

              src[dir] = tqRgb(rcb - (int) ( rDiff * rat ),
                              gcb - (int) ( gDiff * rat ),
                              bcb - (int) ( bDiff * rat ));
          }

	  // Believe it or not, manually copying in a for loop is faster
	  // than calling memcpy for each scanline (on the order of ms...).
	  // I think this is due to the function call overhead (mosfet).

	  for(y = 1; y < size.height(); ++y)
          {
	      scanline = (unsigned int *)image.scanLine(y);
	      for(x=0; x < size.width(); ++x)
                  scanline[x] = src[x];
          }
	}
    }

    else {
      int w=size.width(), h=size.height();

      unsigned char *xtable[3];
      unsigned char *ytable[3];
      xtable[0] = new unsigned char[w];
      xtable[1] = new unsigned char[w];
      xtable[2] = new unsigned char[w];
      ytable[0] = new unsigned char[h];
      ytable[1] = new unsigned char[h];
      ytable[2] = new unsigned char[h];

      if ( eff == DiagonalGradient || eff == CrossDiagonalGradient)
      {
	  for (x = 0; x < w; x++) {
              dir = _xanti ? x : w - 1 - x;
              rat = 1 - exp( - (float)x * xbal );

              xtable[0][dir] = (unsigned char) ( rDiff/2 * rat );
              xtable[1][dir] = (unsigned char) ( gDiff/2 * rat );
              xtable[2][dir] = (unsigned char) ( bDiff/2 * rat );
          }

	  for (y = 0; y < h; y++) {
              dir = _yanti ? y : h - 1 - y;
              rat =  1 - exp( - (float)y  * ybal );

              ytable[0][dir] = (unsigned char) ( rDiff/2 * rat );
              ytable[1][dir] = (unsigned char) ( gDiff/2 * rat );
              ytable[2][dir] = (unsigned char) ( bDiff/2 * rat );
          }

	  for (y = 0; y < h; y++) {
              unsigned int *scanline = (unsigned int *)image.scanLine(y);
              for (x = 0; x < w; x++) {
                  scanline[x] = tqRgb(rcb - (xtable[0][x] + ytable[0][y]),
                                     gcb - (xtable[1][x] + ytable[1][y]),
                                     bcb - (xtable[2][x] + ytable[2][y]));
              }
          }
      }

      else if (eff == RectangleGradient ||
               eff == PyramidGradient ||
               eff == PipeCrossGradient ||
               eff == EllipticGradient)
      {
          int rSign = rDiff>0? 1: -1;
          int gSign = gDiff>0? 1: -1;
          int bSign = bDiff>0? 1: -1;

          for (x = 0; x < w; x++)
          {
              dir = _xanti ? x : w - 1 - x;
              rat =  1 - exp( - (float)x * xbal );

              xtable[0][dir] = (unsigned char) abs((int)(rDiff*(0.5-rat)));
              xtable[1][dir] = (unsigned char) abs((int)(gDiff*(0.5-rat)));
              xtable[2][dir] = (unsigned char) abs((int)(bDiff*(0.5-rat)));
          }

          for (y = 0; y < h; y++)
          {
              dir = _yanti ? y : h - 1 - y;

              rat =  1 - exp( - (float)y * ybal );

              ytable[0][dir] = (unsigned char) abs((int)(rDiff*(0.5-rat)));
              ytable[1][dir] = (unsigned char) abs((int)(gDiff*(0.5-rat)));
              ytable[2][dir] = (unsigned char) abs((int)(bDiff*(0.5-rat)));
          }

          for (y = 0; y < h; y++) {
              unsigned int *scanline = (unsigned int *)image.scanLine(y);
              for (x = 0; x < w; x++) {
                  if (eff == PyramidGradient)
                  {
                      scanline[x] = tqRgb(rcb-rSign*(xtable[0][x]+ytable[0][y]),
                                         gcb-gSign*(xtable[1][x]+ytable[1][y]),
                                         bcb-bSign*(xtable[2][x]+ytable[2][y]));
                  }
                  else if (eff == RectangleGradient)
                  {
                      scanline[x] = tqRgb(rcb - rSign *
                                         TQMAX(xtable[0][x], ytable[0][y]) * 2,
                                         gcb - gSign *
                                         TQMAX(xtable[1][x], ytable[1][y]) * 2,
                                         bcb - bSign *
                                         TQMAX(xtable[2][x], ytable[2][y]) * 2);
                  }
                  else if (eff == PipeCrossGradient)
                  {
                      scanline[x] = tqRgb(rcb - rSign *
                                         TQMIN(xtable[0][x], ytable[0][y]) * 2,
                                         gcb - gSign *
                                         TQMIN(xtable[1][x], ytable[1][y]) * 2,
                                         bcb - bSign *
                                         TQMIN(xtable[2][x], ytable[2][y]) * 2);
                  }
                  else if (eff == EllipticGradient)
                  {
                      scanline[x] = tqRgb(rcb - rSign *
                                         (int)sqrt((xtable[0][x]*xtable[0][x] +
                                                    ytable[0][y]*ytable[0][y])*2.0),
                                         gcb - gSign *
                                         (int)sqrt((xtable[1][x]*xtable[1][x] +
                                                    ytable[1][y]*ytable[1][y])*2.0),
                                         bcb - bSign *
                                         (int)sqrt((xtable[2][x]*xtable[2][x] +
                                                    ytable[2][y]*ytable[2][y])*2.0));
                  }
              }
          }
      }

      if (ncols && (TQPixmap::defaultDepth() < 15 )) {
          if ( ncols < 2 || ncols > 256 )
              ncols = 3;
          TQColor *dPal = new TQColor[ncols];
          for (int i=0; i<ncols; i++) {
              dPal[i].setRgb ( rca + rDiff * i / ( ncols - 1 ),
                               gca + gDiff * i / ( ncols - 1 ),
                               bca + bDiff * i / ( ncols - 1 ) );
          }
          dither(image, dPal, ncols);
          delete [] dPal;
      }

      delete [] xtable[0];
      delete [] xtable[1];
      delete [] xtable[2];
      delete [] ytable[0];
      delete [] ytable[1];
      delete [] ytable[2];

    }

    return image;
}

/**
Types for MMX and SSE packing of colors, for safe constraints
*/
namespace {

struct KIE4Pack
{
    TQ_UINT16 data[4];
};

struct KIE8Pack
{
    TQ_UINT16 data[8];
};

}

//======================================================================
//
// Intensity effects
//
//======================================================================


/* This builds a 256 byte unsigned char lookup table with all
 * the possible percent values prior to applying the effect, then uses
 * integer math for the pixels. For any image larger than 9x9 this will be
 * less expensive than doing a float operation on the 3 color components of
 * each pixel. (mosfet)
 */
TQImage& KImageEffect::intensity(TQImage &image, float percent)
{
    if (image.width() == 0 || image.height() == 0) {
#ifndef NDEBUG
      std::cerr << "WARNING: KImageEffect::intensity : invalid image\n";
#endif
      return image;
    }

    int segColors = image.depth() > 8 ? 256 : image.numColors();
    int pixels = image.depth() > 8 ? image.width()*image.height() :
                 image.numColors();
    unsigned int *data = image.depth() > 8 ? (unsigned int *)image.bits() :
                         (unsigned int *)image.colorTable();

    bool brighten = (percent >= 0);
    if(percent < 0)
        percent = -percent;

#ifdef USE_MMX_INLINE_ASM
    bool haveMMX = KCPUInfo::haveExtension( KCPUInfo::IntelMMX );

    if(haveMMX)
    {
        TQ_UINT16 p = TQ_UINT16(256.0f*(percent));
        KIE4Pack mult = {{p,p,p,0}};

        __asm__ __volatile__(
        "pxor %%mm7, %%mm7\n\t"                // zero mm7 for unpacking
        "movq  (%0), %%mm6\n\t"                // copy intensity change to mm6
        : : "r"(&mult), "m"(mult));

        unsigned int rem = pixels % 4;
        pixels -= rem;
        TQ_UINT32 *end = ( data + pixels );

        if (brighten)
        {
            while ( data != end ) {
                __asm__ __volatile__(
                "movq       (%0), %%mm0\n\t"
                "movq      8(%0), %%mm4\n\t"   // copy 4 pixels of data to mm0 and mm4
                "movq      %%mm0, %%mm1\n\t"
                "movq      %%mm0, %%mm3\n\t"
                "movq      %%mm4, %%mm5\n\t"   // copy to registers for unpacking
                "punpcklbw %%mm7, %%mm0\n\t"
                "punpckhbw %%mm7, %%mm1\n\t"   // unpack the two pixels from mm0
                "pmullw    %%mm6, %%mm0\n\t"
                "punpcklbw %%mm7, %%mm4\n\t"
                "pmullw    %%mm6, %%mm1\n\t"   // multiply by intensity*256
                "psrlw        $8, %%mm0\n\t"   // divide by 256
                "pmullw    %%mm6, %%mm4\n\t"
                "psrlw        $8, %%mm1\n\t"
                "psrlw        $8, %%mm4\n\t"
                "packuswb  %%mm1, %%mm0\n\t"   // pack solution into mm0. saturates at 255
                "movq      %%mm5, %%mm1\n\t"

                "punpckhbw %%mm7, %%mm1\n\t"   // unpack 4th pixel in mm1

                "pmullw    %%mm6, %%mm1\n\t"
                "paddusb   %%mm3, %%mm0\n\t"   // add intesity result to original of mm0
                "psrlw        $8, %%mm1\n\t"
                "packuswb  %%mm1, %%mm4\n\t"   // pack upper two pixels into mm4

                "movq      %%mm0, (%0)\n\t"    // rewrite to memory lower two pixels
                "paddusb   %%mm5, %%mm4\n\t"
                "movq      %%mm4, 8(%0)\n\t"   // rewrite upper two pixels
                : : "r"(data) );
                data += 4;
            }

            end += rem;
            while ( data != end ) {
                __asm__ __volatile__(
                "movd       (%0), %%mm0\n\t"   // repeat above but for
                "punpcklbw %%mm7, %%mm0\n\t"   // one pixel at a time
                "movq      %%mm0, %%mm3\n\t"
                "pmullw    %%mm6, %%mm0\n\t"
                "psrlw        $8, %%mm0\n\t"
                "paddw     %%mm3, %%mm0\n\t"
                "packuswb  %%mm0, %%mm0\n\t"
                "movd      %%mm0, (%0)\n\t"
                : : "r"(data) );
		data++;
            }
        }
        else
        {
            while ( data != end ) {
                __asm__ __volatile__(
                "movq       (%0), %%mm0\n\t"
                "movq      8(%0), %%mm4\n\t"
                "movq      %%mm0, %%mm1\n\t"
                "movq      %%mm0, %%mm3\n\t"

                "movq      %%mm4, %%mm5\n\t"

                "punpcklbw %%mm7, %%mm0\n\t"
                "punpckhbw %%mm7, %%mm1\n\t"
                "pmullw    %%mm6, %%mm0\n\t"
                "punpcklbw %%mm7, %%mm4\n\t"
                "pmullw    %%mm6, %%mm1\n\t"
                "psrlw        $8, %%mm0\n\t"
                "pmullw    %%mm6, %%mm4\n\t"
                "psrlw        $8, %%mm1\n\t"
                "psrlw        $8, %%mm4\n\t"
                "packuswb  %%mm1, %%mm0\n\t"
                "movq      %%mm5, %%mm1\n\t"

                "punpckhbw %%mm7, %%mm1\n\t"

                "pmullw    %%mm6, %%mm1\n\t"
                "psubusb   %%mm0, %%mm3\n\t"   // subtract darkening amount
                "psrlw        $8, %%mm1\n\t"
                "packuswb  %%mm1, %%mm4\n\t"

                "movq      %%mm3, (%0)\n\t"
                "psubusb   %%mm4, %%mm5\n\t"   // only change for this version is
                "movq      %%mm5, 8(%0)\n\t"   // subtraction here as we are darkening image
                : : "r"(data) );
                data += 4;
            }

            end += rem;
            while ( data != end ) {
                __asm__ __volatile__(
                "movd       (%0), %%mm0\n\t"
                "punpcklbw %%mm7, %%mm0\n\t"
                "movq      %%mm0, %%mm3\n\t"
                "pmullw    %%mm6, %%mm0\n\t"
                "psrlw        $8, %%mm0\n\t"
                "psubusw   %%mm0, %%mm3\n\t"
                "packuswb  %%mm3, %%mm3\n\t"
                "movd      %%mm3, (%0)\n\t"
                : : "r"(data) );
                data++;
            }
        }
        __asm__ __volatile__("emms");          // clear mmx state
    }
    else
#endif // USE_MMX_INLINE_ASM
    {
        unsigned char *segTbl = new unsigned char[segColors];
        int tmp;
        if(brighten){ // keep overflow check out of loops
            for(int i=0; i < segColors; ++i){
                tmp = (int)(i*percent);
                if(tmp > 255)
                    tmp = 255;
                segTbl[i] = tmp;
            }
        }
        else{
            for(int i=0; i < segColors; ++i){
                tmp = (int)(i*percent);
                if(tmp < 0)
                    tmp = 0;
                 segTbl[i] = tmp;
            }
        }

        if(brighten){ // same here
            for(int i=0; i < pixels; ++i){
                int r = tqRed(data[i]);
                int g = tqGreen(data[i]);
                int b = tqBlue(data[i]);
                int a = tqAlpha(data[i]);
                r = r + segTbl[r] > 255 ? 255 : r + segTbl[r];
                g = g + segTbl[g] > 255 ? 255 : g + segTbl[g];
                b = b + segTbl[b] > 255 ? 255 : b + segTbl[b];
                data[i] = tqRgba(r, g, b,a);
            }
        }
        else{
            for(int i=0; i < pixels; ++i){
                int r = tqRed(data[i]);
                int g = tqGreen(data[i]);
                int b = tqBlue(data[i]);
                int a = tqAlpha(data[i]);
                r = r - segTbl[r] < 0 ? 0 : r - segTbl[r];
                g = g - segTbl[g] < 0 ? 0 : g - segTbl[g];
                b = b - segTbl[b] < 0 ? 0 : b - segTbl[b];
                data[i] = tqRgba(r, g, b, a);
            }
        }
        delete [] segTbl;
    }

    return image;
}

TQImage& KImageEffect::channelIntensity(TQImage &image, float percent,
                                       RGBComponent channel)
{
    if (image.width() == 0 || image.height() == 0) {
#ifndef NDEBUG
      std::cerr << "WARNING: KImageEffect::channelIntensity : invalid image\n";
#endif
      return image;
    }

    int segColors = image.depth() > 8 ? 256 : image.numColors();
    unsigned char *segTbl = new unsigned char[segColors];
    int pixels = image.depth() > 8 ? image.width()*image.height() :
        image.numColors();
    unsigned int *data = image.depth() > 8 ? (unsigned int *)image.bits() :
        (unsigned int *)image.colorTable();
    bool brighten = (percent >= 0);
    if(percent < 0)
        percent = -percent;

    if(brighten){ // keep overflow check out of loops
        for(int i=0; i < segColors; ++i){
            int tmp = (int)(i*percent);
            if(tmp > 255)
                tmp = 255;
            segTbl[i] = tmp;
        }
    }
    else{
        for(int i=0; i < segColors; ++i){
            int tmp = (int)(i*percent);
            if(tmp < 0)
                tmp = 0;
            segTbl[i] = tmp;
        }
    }

    if(brighten){ // same here
        if(channel == Red){ // and here ;-)
            for(int i=0; i < pixels; ++i){
                int c = tqRed(data[i]);
                c = c + segTbl[c] > 255 ? 255 : c + segTbl[c];
                data[i] = tqRgba(c, tqGreen(data[i]), tqBlue(data[i]), tqAlpha(data[i]));
            }
        }
        else if(channel == Green){
            for(int i=0; i < pixels; ++i){
                int c = tqGreen(data[i]);
                c = c + segTbl[c] > 255 ? 255 : c + segTbl[c];
                data[i] = tqRgba(tqRed(data[i]), c, tqBlue(data[i]), tqAlpha(data[i]));
            }
        }
        else{
            for(int i=0; i < pixels; ++i){
                int c = tqBlue(data[i]);
                c = c + segTbl[c] > 255 ? 255 : c + segTbl[c];
                data[i] = tqRgba(tqRed(data[i]), tqGreen(data[i]), c, tqAlpha(data[i]));
            }
        }

    }
    else{
        if(channel == Red){
            for(int i=0; i < pixels; ++i){
                int c = tqRed(data[i]);
                c = c - segTbl[c] < 0 ? 0 : c - segTbl[c];
                data[i] = tqRgba(c, tqGreen(data[i]), tqBlue(data[i]), tqAlpha(data[i]));
            }
        }
        else if(channel == Green){
            for(int i=0; i < pixels; ++i){
                int c = tqGreen(data[i]);
                c = c - segTbl[c] < 0 ? 0 : c - segTbl[c];
                data[i] = tqRgba(tqRed(data[i]), c, tqBlue(data[i]), tqAlpha(data[i]));
            }
        }
        else{
            for(int i=0; i < pixels; ++i){
                int c = tqBlue(data[i]);
                c = c - segTbl[c] < 0 ? 0 : c - segTbl[c];
                data[i] = tqRgba(tqRed(data[i]), tqGreen(data[i]), c, tqAlpha(data[i]));
            }
        }
    }
    delete [] segTbl;

    return image;
}

// Modulate an image with an RBG channel of another image
//
TQImage& KImageEffect::modulate(TQImage &image, TQImage &modImage, bool reverse,
	ModulationType type, int factor, RGBComponent channel)
{
    if (image.width() == 0 || image.height() == 0 ||
        modImage.width() == 0 || modImage.height() == 0) {
#ifndef NDEBUG
      std::cerr << "WARNING: KImageEffect::modulate : invalid image\n";
#endif
      return image;
    }

    int r, g, b, h, s, v, a;
    TQColor clr;
    int mod=0;
    unsigned int x1, x2, y1, y2;
    int x, y;

    // for image, we handle only depth 32
    if (image.depth()<32) image = image.convertDepth(32);

    // for modImage, we handle depth 8 and 32
    if (modImage.depth()<8) modImage = modImage.convertDepth(8);

    unsigned int *colorTable2 = (modImage.depth()==8) ?
				 modImage.colorTable():0;
    unsigned int *data1, *data2;
    unsigned char *data2b;
    unsigned int color1, color2;

    x1 = image.width();    y1 = image.height();
    x2 = modImage.width(); y2 = modImage.height();

    for (y = 0; y < (int)y1; y++) {
        data1 =  (unsigned int *) image.scanLine(y);
	data2 =  (unsigned int *) modImage.scanLine( y%y2 );
	data2b = (unsigned char *) modImage.scanLine( y%y2 );

	x=0;
	while(x < (int)x1) {
	  color2 = (colorTable2) ? colorTable2[*data2b] : *data2;
	  if (reverse) {
	      color1 = color2;
	      color2 = *data1;
	  }
	  else
	      color1 = *data1;

	  if (type == Intensity || type == Contrast) {
              r = tqRed(color1);
	      g = tqGreen(color1);
	      b = tqBlue(color1);
	      if (channel != All) {
      	        mod = (channel == Red) ? tqRed(color2) :
		    (channel == Green) ? tqGreen(color2) :
	    	    (channel == Blue) ? tqBlue(color2) :
		    (channel == Gray) ? tqGray(color2) : 0;
	        mod = mod*factor/50;
	      }

	      if (type == Intensity) {
	        if (channel == All) {
	          r += r * factor/50 * tqRed(color2)/256;
	          g += g * factor/50 * tqGreen(color2)/256;
	          b += b * factor/50 * tqBlue(color2)/256;
	        }
	        else {
	          r += r * mod/256;
	          g += g * mod/256;
	          b += b * mod/256;
	        }
	      }
	      else { // Contrast
	        if (channel == All) {
		  r += (r-128) * factor/50 * tqRed(color2)/128;
	          g += (g-128) * factor/50 * tqGreen(color2)/128;
	          b += (b-128) * factor/50 * tqBlue(color2)/128;
	        }
	        else {
	          r += (r-128) * mod/128;
	          g += (g-128) * mod/128;
	          b += (b-128) * mod/128;
	        }
	      }

	      if (r<0) r=0; if (r>255) r=255;
	      if (g<0) g=0; if (g>255) g=255;
	      if (b<0) b=0; if (b>255) b=255;
	      a = tqAlpha(*data1);
	      *data1 = tqRgba(r, g, b, a);
	  }
	  else if (type == Saturation || type == HueShift) {
	      clr.setRgb(color1);
	      clr.hsv(&h, &s, &v);
      	      mod = (channel == Red) ? tqRed(color2) :
		    (channel == Green) ? tqGreen(color2) :
	    	    (channel == Blue) ? tqBlue(color2) :
		    (channel == Gray) ? tqGray(color2) : 0;
	      mod = mod*factor/50;

	      if (type == Saturation) {
		  s -= s * mod/256;
		  if (s<0) s=0; if (s>255) s=255;
	      }
	      else { // HueShift
	        h += mod;
		while(h<0) h+=360;
		h %= 360;
	      }

	      clr.setHsv(h, s, v);
	      a = tqAlpha(*data1);
	      *data1 = clr.rgb() | ((uint)(a & 0xff) << 24);
	  }
	  data1++; data2++; data2b++; x++;
	  if ( (x%x2) ==0) { data2 -= x2; data2b -= x2; }
        }
    }
    return image;
}



//======================================================================
//
// Blend effects
//
//======================================================================


// Nice and fast direct pixel manipulation
TQImage& KImageEffect::blend(const TQColor& clr, TQImage& dst, float opacity)
{
    if (dst.width() <= 0 || dst.height() <= 0)
        return dst;

    if (opacity < 0.0 || opacity > 1.0) {
#ifndef NDEBUG
        std::cerr << "WARNING: KImageEffect::blend : invalid opacity. Range [0, 1]\n";
#endif
        return dst;
    }

    if (dst.depth() != 32)
        dst = dst.convertDepth(32);

    int pixels = dst.width() * dst.height();

#ifdef USE_SSE2_INLINE_ASM
    if ( KCPUInfo::haveExtension( KCPUInfo::IntelSSE2 ) && pixels > 16 ) {
        TQ_UINT16 alpha = TQ_UINT16( ( 1.0 - opacity ) * 256.0 );

        KIE8Pack packedalpha = { { alpha, alpha, alpha, 256,
                                   alpha, alpha, alpha, 256 } };

        TQ_UINT16 red   = TQ_UINT16( clr.red()   * 256 * opacity );
        TQ_UINT16 green = TQ_UINT16( clr.green() * 256 * opacity );
        TQ_UINT16 blue  = TQ_UINT16( clr.blue()  * 256 * opacity );

        KIE8Pack packedcolor = { { blue, green, red, 0,
                                   blue, green, red, 0 } };

        // Prepare the XMM5, XMM6 and XMM7 registers for unpacking and blending
        __asm__ __volatile__(
        "pxor        %%xmm7,  %%xmm7\n\t" // Zero out XMM7 for unpacking
        "movdqu        (%0),  %%xmm6\n\t" // Set up (1 - alpha) * 256 in XMM6
        "movdqu        (%1),  %%xmm5\n\t" // Set up color * alpha * 256 in XMM5
        : : "r"(&packedalpha), "r"(&packedcolor),
            "m"(packedcolor),  "m"(packedalpha) );

        TQ_UINT32 *data = reinterpret_cast<TQ_UINT32*>( dst.bits() );

        // Check how many pixels we need to process to achieve 16 byte alignment
        int offset = (16 - (TQ_UINT32( data ) & 0x0f)) / 4;

        // The main loop processes 8 pixels / iteration
        int remainder = (pixels - offset) % 8;
        pixels -= remainder;

        // Alignment loop
        for ( int i = 0; i < offset; i++ ) {
            __asm__ __volatile__(
            "movd         (%0,%1,4),      %%xmm0\n\t"  // Load one pixel to XMM1
            "punpcklbw       %%xmm7,      %%xmm0\n\t"  // Unpack the pixel
            "pmullw          %%xmm6,      %%xmm0\n\t"  // Multiply the pixel with (1 - alpha) * 256
            "paddw           %%xmm5,      %%xmm0\n\t"  // Add color * alpha * 256 to the result
            "psrlw               $8,      %%xmm0\n\t"  // Divide by 256
            "packuswb        %%xmm1,      %%xmm0\n\t"  // Pack the pixel to a dword
            "movd            %%xmm0,   (%0,%1,4)\n\t"  // Write the pixel to the image
            : : "r"(data), "r"(i) );
        }

        // Main loop
        for ( int i = offset; i < pixels; i += 8 ) {
            __asm__ __volatile(
            // Load 8 pixels to XMM registers 1 - 4
            "movq         (%0,%1,4),      %%xmm0\n\t"  // Load pixels 1 and 2 to XMM1
            "movq        8(%0,%1,4),      %%xmm1\n\t"  // Load pixels 3 and 4 to XMM2
            "movq       16(%0,%1,4),      %%xmm2\n\t"  // Load pixels 5 and 6 to XMM3
            "movq       24(%0,%1,4),      %%xmm3\n\t"  // Load pixels 7 and 8 to XMM4

            // Prefetch the pixels for next iteration
            "prefetchnta 32(%0,%1,4)            \n\t"

            // Blend pixels 1 and 2
            "punpcklbw       %%xmm7,      %%xmm0\n\t"  // Unpack the pixels
            "pmullw          %%xmm6,      %%xmm0\n\t"  // Multiply the pixels with (1 - alpha) * 256
            "paddw           %%xmm5,      %%xmm0\n\t"  // Add color * alpha * 256 to the result
            "psrlw               $8,      %%xmm0\n\t"  // Divide by 256

            // Blend pixels 3 and 4
            "punpcklbw       %%xmm7,      %%xmm1\n\t"  // Unpack the pixels
            "pmullw          %%xmm6,      %%xmm1\n\t"  // Multiply the pixels with (1 - alpha) * 256
            "paddw           %%xmm5,      %%xmm1\n\t"  // Add color * alpha * 256 to the result
            "psrlw               $8,      %%xmm1\n\t"  // Divide by 256

            // Blend pixels 5 and 6
            "punpcklbw       %%xmm7,      %%xmm2\n\t"  // Unpack the pixels
            "pmullw          %%xmm6,      %%xmm2\n\t"  // Multiply the pixels with (1 - alpha) * 256
            "paddw           %%xmm5,      %%xmm2\n\t"  // Add color * alpha * 256 to the result
            "psrlw               $8,      %%xmm2\n\t"  // Divide by 256

            // Blend pixels 7 and 8
            "punpcklbw       %%xmm7,      %%xmm3\n\t"  // Unpack the pixels
            "pmullw          %%xmm6,      %%xmm3\n\t"  // Multiply the pixels with (1 - alpha) * 256
            "paddw           %%xmm5,      %%xmm3\n\t"  // Add color * alpha * 256 to the result
            "psrlw               $8,      %%xmm3\n\t"  // Divide by 256

            // Pack the pixels into 2 double quadwords
            "packuswb        %%xmm1,      %%xmm0\n\t"  // Pack pixels 1 - 4 to a double qword
            "packuswb        %%xmm3,      %%xmm2\n\t"  // Pack pixles 5 - 8 to a double qword

            // Write the pixels back to the image
            "movdqa          %%xmm0,   (%0,%1,4)\n\t"  // Store pixels 1 - 4
            "movdqa          %%xmm2, 16(%0,%1,4)\n\t"  // Store pixels 5 - 8
            : : "r"(data), "r"(i) );
        }

        // Cleanup loop
        for ( int i = pixels; i < pixels + remainder; i++ ) {
            __asm__ __volatile__(
            "movd         (%0,%1,4),      %%xmm0\n\t"  // Load one pixel to XMM1
            "punpcklbw       %%xmm7,      %%xmm0\n\t"  // Unpack the pixel
            "pmullw          %%xmm6,      %%xmm0\n\t"  // Multiply the pixel with (1 - alpha) * 256
            "paddw           %%xmm5,      %%xmm0\n\t"  // Add color * alpha * 256 to the result
            "psrlw               $8,      %%xmm0\n\t"  // Divide by 256
            "packuswb        %%xmm1,      %%xmm0\n\t"  // Pack the pixel to a dword
            "movd            %%xmm0,   (%0,%1,4)\n\t"  // Write the pixel to the image
            : : "r"(data), "r"(i) );
        }
    } else
#endif

#ifdef USE_MMX_INLINE_ASM
    if ( KCPUInfo::haveExtension( KCPUInfo::IntelMMX ) && pixels > 1 ) {
        TQ_UINT16 alpha = TQ_UINT16( ( 1.0 - opacity ) * 256.0 );
        KIE4Pack packedalpha = { { alpha, alpha, alpha, 256 } };

        TQ_UINT16 red   = TQ_UINT16( clr.red()   * 256 * opacity );
        TQ_UINT16 green = TQ_UINT16( clr.green() * 256 * opacity );
        TQ_UINT16 blue  = TQ_UINT16( clr.blue()  * 256 * opacity );

        KIE4Pack packedcolor = { { blue, green, red, 0 } };

        __asm__ __volatile__(
        "pxor        %%mm7,    %%mm7\n\t"       // Zero out MM7 for unpacking
        "movq         (%0),    %%mm6\n\t"       // Set up (1 - alpha) * 256 in MM6
        "movq         (%1),    %%mm5\n\t"       // Set up color * alpha * 256 in MM5
        : : "r"(&packedalpha), "r"(&packedcolor), "m"(packedcolor), "m"(packedalpha) );

        TQ_UINT32 *data = reinterpret_cast<TQ_UINT32*>( dst.bits() );

        // The main loop processes 4 pixels / iteration
        int remainder = pixels % 4;
        pixels -= remainder;

        // Main loop
        for ( int i = 0; i < pixels; i += 4 ) {
            __asm__ __volatile__(
            // Load 4 pixels to MM registers 1 - 4
            "movd         (%0,%1,4),      %%mm0\n\t"  // Load the 1st pixel to MM0
            "movd        4(%0,%1,4),      %%mm1\n\t"  // Load the 2nd pixel to MM1
            "movd        8(%0,%1,4),      %%mm2\n\t"  // Load the 3rd pixel to MM2
            "movd       12(%0,%1,4),      %%mm3\n\t"  // Load the 4th pixel to MM3

            // Blend the first pixel
            "punpcklbw        %%mm7,      %%mm0\n\t"  // Unpack the pixel
            "pmullw           %%mm6,      %%mm0\n\t"  // Multiply the pixel with (1 - alpha) * 256
            "paddw            %%mm5,      %%mm0\n\t"  // Add color * alpha * 256 to the result
            "psrlw               $8,      %%mm0\n\t"  // Divide by 256

            // Blend the second pixel
            "punpcklbw        %%mm7,      %%mm1\n\t"  // Unpack the pixel
            "pmullw           %%mm6,      %%mm1\n\t"  // Multiply the pixel with (1 - alpha) * 256
            "paddw            %%mm5,      %%mm1\n\t"  // Add color * alpha * 256 to the result
            "psrlw               $8,      %%mm1\n\t"  // Divide by 256

            // Blend the third pixel
            "punpcklbw        %%mm7,      %%mm2\n\t"  // Unpack the pixel
            "pmullw           %%mm6,      %%mm2\n\t"  // Multiply the pixel with (1 - alpha) * 256
            "paddw            %%mm5,      %%mm2\n\t"  // Add color * alpha * 256 to the result
            "psrlw               $8,      %%mm2\n\t"  // Divide by 256

            // Blend the fourth pixel
            "punpcklbw        %%mm7,      %%mm3\n\t"  // Unpack the pixel
            "pmullw           %%mm6,      %%mm3\n\t"  // Multiply the pixel with (1 - alpha) * 256
            "paddw            %%mm5,      %%mm3\n\t"  // Add color * alpha * 256 to the result
            "psrlw               $8,      %%mm3\n\t"  // Divide by 256

            // Pack the pixels into 2 quadwords
            "packuswb         %%mm1,      %%mm0\n\t"  // Pack pixels 1 and 2 to a qword
            "packuswb         %%mm3,      %%mm2\n\t"  // Pack pixels 3 and 4 to a qword

            // Write the pixels back to the image
            "movq             %%mm0,  (%0,%1,4)\n\t"  // Store pixels 1 and 2
            "movq             %%mm2, 8(%0,%1,4)\n\t"  // Store pixels 3 and 4
            : : "r"(data), "r"(i) );
        }

        // Cleanup loop
        for ( int i = pixels; i < pixels + remainder; i++ ) {
            __asm__ __volatile__(
            "movd         (%0,%1,4),      %%mm0\n\t"  // Load one pixel to MM1
            "punpcklbw        %%mm7,      %%mm0\n\t"  // Unpack the pixel
            "pmullw           %%mm6,      %%mm0\n\t"  // Multiply the pixel with 1 - alpha * 256
            "paddw            %%mm5,      %%mm0\n\t"  // Add color * alpha * 256 to the result
            "psrlw               $8,      %%mm0\n\t"  // Divide by 256
            "packuswb         %%mm0,      %%mm0\n\t"  // Pack the pixel to a dword
            "movd             %%mm0,  (%0,%1,4)\n\t"  // Write the pixel to the image
            : : "r"(data), "r"(i) );
        }

        // Empty the MMX state
        __asm__ __volatile__("emms");
    } else
#endif // USE_MMX_INLINE_ASM

    {
        int rcol, gcol, bcol;
        clr.rgb(&rcol, &gcol, &bcol);

#ifdef WORDS_BIGENDIAN   // ARGB (skip alpha)
        unsigned char *data = (unsigned char *)dst.bits() + 1;
#else                    // BGRA
        unsigned char *data = (unsigned char *)dst.bits();
#endif

        for (int i=0; i<pixels; i++)
        {
#ifdef WORDS_BIGENDIAN
            *data += (unsigned char)((rcol - *data) * opacity);
            data++;
            *data += (unsigned char)((gcol - *data) * opacity);
            data++;
            *data += (unsigned char)((bcol - *data) * opacity);
            data++;
#else
            *data += (unsigned char)((bcol - *data) * opacity);
            data++;
            *data += (unsigned char)((gcol - *data) * opacity);
            data++;
            *data += (unsigned char)((rcol - *data) * opacity);
            data++;
#endif
            data++; // skip alpha
        }
    }

    return dst;
}

// Nice and fast direct pixel manipulation
TQImage& KImageEffect::blend(TQImage& src, TQImage& dst, float opacity)
{
    if (src.width() <= 0 || src.height() <= 0)
        return dst;
    if (dst.width() <= 0 || dst.height() <= 0)
        return dst;

    if (src.width() != dst.width() || src.height() != dst.height()) {
#ifndef NDEBUG
        std::cerr << "WARNING: KImageEffect::blend : src and destination images are not the same size\n";
#endif
        return dst;
    }

    if (opacity < 0.0 || opacity > 1.0) {
#ifndef NDEBUG
        std::cerr << "WARNING: KImageEffect::blend : invalid opacity. Range [0, 1]\n";
#endif
        return dst;
    }

    if (src.depth() != 32) src = src.convertDepth(32);
    if (dst.depth() != 32) dst = dst.convertDepth(32);

    int pixels = src.width() * src.height();

#ifdef USE_SSE2_INLINE_ASM
    if ( KCPUInfo::haveExtension( KCPUInfo::IntelSSE2 ) && pixels > 16 ) {
        TQ_UINT16 alpha = TQ_UINT16( opacity * 256.0 );
        KIE8Pack packedalpha = { { alpha, alpha, alpha, 0,
                                   alpha, alpha, alpha, 0 } };

        // Prepare the XMM6 and XMM7 registers for unpacking and blending
        __asm__ __volatile__(
        "pxor      %%xmm7, %%xmm7\n\t" // Zero out XMM7 for unpacking
        "movdqu      (%0), %%xmm6\n\t" // Set up alpha * 256 in XMM6
        : : "r"(&packedalpha), "m"(packedalpha) );

        TQ_UINT32 *data1 = reinterpret_cast<TQ_UINT32*>( src.bits() );
        TQ_UINT32 *data2 = reinterpret_cast<TQ_UINT32*>( dst.bits() );

        // Check how many pixels we need to process to achieve 16 byte alignment
        int offset = (16 - (TQ_UINT32( data2 ) & 0x0f)) / 4;

        // The main loop processes 4 pixels / iteration
        int remainder = (pixels - offset) % 4;
        pixels -= remainder;

        // Alignment loop
        for ( int i = 0; i < offset; i++ ) {
            __asm__ __volatile__(
            "movd       (%1,%2,4),    %%xmm1\n\t"  // Load one dst pixel to XMM1
            "punpcklbw     %%xmm7,    %%xmm1\n\t"  // Unpack the pixel
            "movd       (%0,%2,4),    %%xmm0\n\t"  // Load one src pixel to XMM0
            "punpcklbw     %%xmm7,    %%xmm0\n\t"  // Unpack the pixel
            "psubw         %%xmm1,    %%xmm0\n\t"  // Subtract dst from src
            "pmullw        %%xmm6,    %%xmm0\n\t"  // Multiply the result with alpha * 256
            "psllw             $8,    %%xmm1\n\t"  // Multiply dst with 256
            "paddw         %%xmm1,    %%xmm0\n\t"  // Add dst to result
            "psrlw             $8,    %%xmm0\n\t"  // Divide by 256
            "packuswb      %%xmm1,    %%xmm0\n\t"  // Pack the pixel to a dword
            "movd          %%xmm0, (%1,%2,4)\n\t"  // Write the pixel to the image
            : : "r"(data1), "r"(data2), "r"(i) );
        }

        // Main loop
        for ( int i = offset; i < pixels; i += 4 ) {
            __asm__ __volatile__(
            // Load 4 src pixels to XMM0 and XMM2 and 4 dst pixels to XMM1 and XMM3
            "movq       (%0,%2,4),    %%xmm0\n\t"  // Load two src pixels to XMM0
            "movq       (%1,%2,4),    %%xmm1\n\t"  // Load two dst pixels to XMM1
            "movq      8(%0,%2,4),    %%xmm2\n\t"  // Load two src pixels to XMM2
            "movq      8(%1,%2,4),    %%xmm3\n\t"  // Load two dst pixels to XMM3

            // Prefetch the pixels for the iteration after the next one
            "prefetchnta 32(%0,%2,4)        \n\t"
            "prefetchnta 32(%1,%2,4)        \n\t"

            // Blend the first two pixels
            "punpcklbw     %%xmm7,    %%xmm1\n\t"  // Unpack the dst pixels
            "punpcklbw     %%xmm7,    %%xmm0\n\t"  // Unpack the src pixels
            "psubw         %%xmm1,    %%xmm0\n\t"  // Subtract dst from src
            "pmullw        %%xmm6,    %%xmm0\n\t"  // Multiply the result with alpha * 256
            "psllw             $8,    %%xmm1\n\t"  // Multiply dst with 256
            "paddw         %%xmm1,    %%xmm0\n\t"  // Add dst to the result
            "psrlw             $8,    %%xmm0\n\t"  // Divide by 256

            // Blend the next two pixels
            "punpcklbw     %%xmm7,    %%xmm3\n\t"  // Unpack the dst pixels
            "punpcklbw     %%xmm7,    %%xmm2\n\t"  // Unpack the src pixels
            "psubw         %%xmm3,    %%xmm2\n\t"  // Subtract dst from src
            "pmullw        %%xmm6,    %%xmm2\n\t"  // Multiply the result with alpha * 256
            "psllw             $8,    %%xmm3\n\t"  // Multiply dst with 256
            "paddw         %%xmm3,    %%xmm2\n\t"  // Add dst to the result
            "psrlw             $8,    %%xmm2\n\t"  // Divide by 256

            // Write the pixels back to the image
            "packuswb      %%xmm2,    %%xmm0\n\t"  // Pack the pixels to a double qword
            "movdqa        %%xmm0, (%1,%2,4)\n\t"  // Store the pixels
            : : "r"(data1), "r"(data2), "r"(i) );
        }

        // Cleanup loop
        for ( int i = pixels; i < pixels + remainder; i++ ) {
            __asm__ __volatile__(
            "movd       (%1,%2,4),    %%xmm1\n\t"  // Load one dst pixel to XMM1
            "punpcklbw     %%xmm7,    %%xmm1\n\t"  // Unpack the pixel
            "movd       (%0,%2,4),    %%xmm0\n\t"  // Load one src pixel to XMM0
            "punpcklbw     %%xmm7,    %%xmm0\n\t"  // Unpack the pixel
            "psubw         %%xmm1,    %%xmm0\n\t"  // Subtract dst from src
            "pmullw        %%xmm6,    %%xmm0\n\t"  // Multiply the result with alpha * 256
            "psllw             $8,    %%xmm1\n\t"  // Multiply dst with 256
            "paddw         %%xmm1,    %%xmm0\n\t"  // Add dst to result
            "psrlw             $8,    %%xmm0\n\t"  // Divide by 256
            "packuswb      %%xmm1,    %%xmm0\n\t"  // Pack the pixel to a dword
            "movd          %%xmm0, (%1,%2,4)\n\t"  // Write the pixel to the image
            : : "r"(data1), "r"(data2), "r"(i) );
        }
    } else
#endif // USE_SSE2_INLINE_ASM

#ifdef USE_MMX_INLINE_ASM
    if ( KCPUInfo::haveExtension( KCPUInfo::IntelMMX ) && pixels > 1 ) {
        TQ_UINT16 alpha = TQ_UINT16( opacity * 256.0 );
        KIE4Pack packedalpha = { { alpha, alpha, alpha, 0 } };

        // Prepare the MM6 and MM7 registers for blending and unpacking
        __asm__ __volatile__(
        "pxor       %%mm7,   %%mm7\n\t"      // Zero out MM7 for unpacking
        "movq        (%0),   %%mm6\n\t"      // Set up alpha * 256 in MM6
        : : "r"(&packedalpha), "m"(packedalpha) );

        TQ_UINT32 *data1 = reinterpret_cast<TQ_UINT32*>( src.bits() );
        TQ_UINT32 *data2 = reinterpret_cast<TQ_UINT32*>( dst.bits() );

        // The main loop processes 2 pixels / iteration
        int remainder = pixels % 2;
        pixels -= remainder;

        // Main loop
        for ( int i = 0; i < pixels; i += 2 ) {
            __asm__ __volatile__(
            // Load 2 src pixels to MM0 and MM2 and 2 dst pixels to MM1 and MM3
            "movd        (%0,%2,4),     %%mm0\n\t"  // Load the 1st src pixel to MM0
            "movd        (%1,%2,4),     %%mm1\n\t"  // Load the 1st dst pixel to MM1
            "movd       4(%0,%2,4),     %%mm2\n\t"  // Load the 2nd src pixel to MM2
            "movd       4(%1,%2,4),     %%mm3\n\t"  // Load the 2nd dst pixel to MM3

            // Blend the first pixel
            "punpcklbw       %%mm7,     %%mm0\n\t"  // Unpack the src pixel
            "punpcklbw       %%mm7,     %%mm1\n\t"  // Unpack the dst pixel
            "psubw           %%mm1,     %%mm0\n\t"  // Subtract dst from src
            "pmullw          %%mm6,     %%mm0\n\t"  // Multiply the result with alpha * 256
            "psllw              $8,     %%mm1\n\t"  // Multiply dst with 256
            "paddw           %%mm1,     %%mm0\n\t"  // Add dst to the result
            "psrlw              $8,     %%mm0\n\t"  // Divide by 256

            // Blend the second pixel
            "punpcklbw       %%mm7,     %%mm2\n\t"  // Unpack the src pixel
            "punpcklbw       %%mm7,     %%mm3\n\t"  // Unpack the dst pixel
            "psubw           %%mm3,     %%mm2\n\t"  // Subtract dst from src
            "pmullw          %%mm6,     %%mm2\n\t"  // Multiply the result with alpha * 256
            "psllw              $8,     %%mm3\n\t"  // Multiply dst with 256
            "paddw           %%mm3,     %%mm2\n\t"  // Add dst to the result
            "psrlw              $8,     %%mm2\n\t"  // Divide by 256

            // Write the pixels back to the image
            "packuswb        %%mm2,     %%mm0\n\t"  // Pack the pixels to a qword
            "movq            %%mm0, (%1,%2,4)\n\t"  // Store the pixels
            : : "r"(data1), "r"(data2), "r"(i) );
        }

        // Blend the remaining pixel (if there is one)
        if ( remainder ) {
             __asm__ __volatile__(
            "movd             (%0),     %%mm0\n\t"  // Load one src pixel to MM0
            "punpcklbw       %%mm7,     %%mm0\n\t"  // Unpack the src pixel
            "movd             (%1),     %%mm1\n\t"  // Load one dst pixel to MM1
            "punpcklbw       %%mm7,     %%mm1\n\t"  // Unpack the dst pixel
            "psubw           %%mm1,     %%mm0\n\t"  // Subtract dst from src
            "pmullw          %%mm6,     %%mm0\n\t"  // Multiply the result with alpha * 256
            "psllw              $8,     %%mm1\n\t"  // Multiply dst with 256
            "paddw           %%mm1,     %%mm0\n\t"  // Add dst to result
            "psrlw              $8,     %%mm0\n\t"  // Divide by 256
            "packuswb        %%mm0,     %%mm0\n\t"  // Pack the pixel to a dword
            "movd            %%mm0,      (%1)\n\t"  // Write the pixel to the image
            : : "r"(data1 + pixels), "r"(data2 + pixels) );
        }

        // Empty the MMX state
        __asm__ __volatile__("emms");
    } else
#endif // USE_MMX_INLINE_ASM

    {
#ifdef WORDS_BIGENDIAN   // ARGB (skip alpha)
        unsigned char *data1 = (unsigned char *)dst.bits() + 1;
        unsigned char *data2 = (unsigned char *)src.bits() + 1;
#else                    // BGRA
        unsigned char *data1 = (unsigned char *)dst.bits();
        unsigned char *data2 = (unsigned char *)src.bits();
#endif

        for (int i=0; i<pixels; i++)
        {
#ifdef WORDS_BIGENDIAN
            *data1 += (unsigned char)((*(data2++) - *data1) * opacity);
            data1++;
            *data1 += (unsigned char)((*(data2++) - *data1) * opacity);
            data1++;
            *data1 += (unsigned char)((*(data2++) - *data1) * opacity);
            data1++;
#else
            *data1 += (unsigned char)((*(data2++) - *data1) * opacity);
            data1++;
            *data1 += (unsigned char)((*(data2++) - *data1) * opacity);
            data1++;
            *data1 += (unsigned char)((*(data2++) - *data1) * opacity);
            data1++;
#endif
            data1++; // skip alpha
            data2++;
        }
    }

    return dst;
}


TQImage& KImageEffect::blend(TQImage &image, float initial_intensity,
                            const TQColor &bgnd, GradientType eff,
                            bool anti_dir)
{
    if (image.width() == 0 || image.height() == 0 || image.depth()!=32 ) {
#ifndef NDEBUG
      std::cerr << "WARNING: KImageEffect::blend : invalid image\n";
#endif
      return image;
    }

    int r_bgnd = bgnd.red(), g_bgnd = bgnd.green(), b_bgnd = bgnd.blue();
    int r, g, b;
    int ind;

    unsigned int xi, xf, yi, yf;
    unsigned int a;

    // check the boundaries of the initial intesity param
    float unaffected = 1;
    if (initial_intensity >  1) initial_intensity =  1;
    if (initial_intensity < -1) initial_intensity = -1;
    if (initial_intensity < 0) {
        unaffected = 1. + initial_intensity;
        initial_intensity = 0;
    }


    float intensity = initial_intensity;
    float var = 1. - initial_intensity;

    if (anti_dir) {
        initial_intensity = intensity = 1.;
        var = -var;
    }

    int x, y;

    unsigned int *data =  (unsigned int *)image.bits();

    int image_width = image.width(); //Those can't change
    int image_height = image.height();


    if( eff == VerticalGradient || eff == HorizontalGradient ) {

        // set the image domain to apply the effect to
        xi = 0, xf = image_width;
        yi = 0, yf = image_height;
        if (eff == VerticalGradient) {
            if (anti_dir) yf = (int)(image_height * unaffected);
            else yi = (int)(image_height * (1 - unaffected));
        }
        else {
            if (anti_dir) xf = (int)(image_width * unaffected);
            else xi = (int)(image_height * (1 - unaffected));
        }

        var /= (eff == VerticalGradient?yf-yi:xf-xi);

        int ind_base;
        for (y = yi; y < (int)yf; y++) {
            intensity = eff == VerticalGradient? intensity + var :
                initial_intensity;
            ind_base = image_width  * y ;
            for (x = xi; x < (int)xf ; x++) {
                if (eff == HorizontalGradient) intensity += var;
                ind = x + ind_base;
                r = tqRed  (data[ind]) + (int)(intensity *
                                              (r_bgnd - tqRed  (data[ind])));
                g = tqGreen(data[ind]) + (int)(intensity *
                                              (g_bgnd - tqGreen(data[ind])));
                b = tqBlue (data[ind]) + (int)(intensity *
                                              (b_bgnd - tqBlue (data[ind])));
                if (r > 255) r = 255; if (r < 0 ) r = 0;
                if (g > 255) g = 255; if (g < 0 ) g = 0;
                if (b > 255) b = 255; if (b < 0 ) b = 0;
                a = tqAlpha(data[ind]);
                data[ind] = tqRgba(r, g, b, a);
            }
        }
    }
    else if (eff == DiagonalGradient  || eff == CrossDiagonalGradient) {
        float xvar = var / 2 / image_width;  // / unaffected;
        float yvar = var / 2 / image_height; // / unaffected;
        float tmp;

        for (x = 0; x < image_width ; x++) {
            tmp =  xvar * (eff == DiagonalGradient? x : image.width()-x-1);
            ind = x;
            for (y = 0; y < image_height ; y++) {
                intensity = initial_intensity + tmp + yvar * y;

                r = tqRed  (data[ind]) + (int)(intensity *
                                              (r_bgnd - tqRed  (data[ind])));
                g = tqGreen(data[ind]) + (int)(intensity *
                                              (g_bgnd - tqGreen(data[ind])));
                b = tqBlue (data[ind]) + (int)(intensity *
                                              (b_bgnd - tqBlue (data[ind])));
                if (r > 255) r = 255; if (r < 0 ) r = 0;
                if (g > 255) g = 255; if (g < 0 ) g = 0;
                if (b > 255) b = 255; if (b < 0 ) b = 0;
                a = tqAlpha(data[ind]);
                data[ind] = tqRgba(r, g, b, a);

                ind += image_width;
            }
        }
    }

    else if (eff == RectangleGradient || eff == EllipticGradient) {
        float xvar;
        float yvar;

        for (x = 0; x < image_width / 2 + image_width % 2; x++) {
            xvar = var / image_width  * (image_width - x*2/unaffected-1);
            for (y = 0; y < image_height / 2 + image_height % 2; y++) {
                yvar = var / image_height   * (image_height - y*2/unaffected -1);

                if (eff == RectangleGradient)
                    intensity = initial_intensity + TQMAX(xvar, yvar);
                else
                    intensity = initial_intensity + sqrt(xvar * xvar + yvar * yvar);
                if (intensity > 1) intensity = 1;
                if (intensity < 0) intensity = 0;

                //NW
                ind = x + image_width  * y ;
                r = tqRed  (data[ind]) + (int)(intensity *
                                              (r_bgnd - tqRed  (data[ind])));
                g = tqGreen(data[ind]) + (int)(intensity *
                                              (g_bgnd - tqGreen(data[ind])));
                b = tqBlue (data[ind]) + (int)(intensity *
                                              (b_bgnd - tqBlue (data[ind])));
                if (r > 255) r = 255; if (r < 0 ) r = 0;
                if (g > 255) g = 255; if (g < 0 ) g = 0;
                if (b > 255) b = 255; if (b < 0 ) b = 0;
                a = tqAlpha(data[ind]);
                data[ind] = tqRgba(r, g, b, a);

                //NE
                ind = image_width - x - 1 + image_width  * y ;
                r = tqRed  (data[ind]) + (int)(intensity *
                                              (r_bgnd - tqRed  (data[ind])));
                g = tqGreen(data[ind]) + (int)(intensity *
                                              (g_bgnd - tqGreen(data[ind])));
                b = tqBlue (data[ind]) + (int)(intensity *
                                              (b_bgnd - tqBlue (data[ind])));
                if (r > 255) r = 255; if (r < 0 ) r = 0;
                if (g > 255) g = 255; if (g < 0 ) g = 0;
                if (b > 255) b = 255; if (b < 0 ) b = 0;
                a = tqAlpha(data[ind]);
                data[ind] = tqRgba(r, g, b, a);
            }
        }

        //CT  loop is doubled because of stupid central row/column issue.
        //    other solution?
        for (x = 0; x < image_width / 2; x++) {
            xvar = var / image_width  * (image_width - x*2/unaffected-1);
            for (y = 0; y < image_height / 2; y++) {
                yvar = var / image_height   * (image_height - y*2/unaffected -1);

                if (eff == RectangleGradient)
                    intensity = initial_intensity + TQMAX(xvar, yvar);
                else
                    intensity = initial_intensity + sqrt(xvar * xvar + yvar * yvar);
                if (intensity > 1) intensity = 1;
                if (intensity < 0) intensity = 0;

                //SW
                ind = x + image_width  * (image_height - y -1) ;
                r = tqRed  (data[ind]) + (int)(intensity *
                                              (r_bgnd - tqRed  (data[ind])));
                g = tqGreen(data[ind]) + (int)(intensity *
                                              (g_bgnd - tqGreen(data[ind])));
                b = tqBlue (data[ind]) + (int)(intensity *
                                              (b_bgnd - tqBlue (data[ind])));
                if (r > 255) r = 255; if (r < 0 ) r = 0;
                if (g > 255) g = 255; if (g < 0 ) g = 0;
                if (b > 255) b = 255; if (b < 0 ) b = 0;
                a = tqAlpha(data[ind]);
                data[ind] = tqRgba(r, g, b, a);

                //SE
                ind = image_width-x-1 + image_width * (image_height - y - 1) ;
                r = tqRed  (data[ind]) + (int)(intensity *
                                              (r_bgnd - tqRed  (data[ind])));
                g = tqGreen(data[ind]) + (int)(intensity *
                                              (g_bgnd - tqGreen(data[ind])));
                b = tqBlue (data[ind]) + (int)(intensity *
                                              (b_bgnd - tqBlue (data[ind])));
                if (r > 255) r = 255; if (r < 0 ) r = 0;
                if (g > 255) g = 255; if (g < 0 ) g = 0;
                if (b > 255) b = 255; if (b < 0 ) b = 0;
                a = tqAlpha(data[ind]);
                data[ind] = tqRgba(r, g, b, a);
            }
        }
    }
#ifndef NDEBUG
    else std::cerr << "KImageEffect::blend effect not implemented" << std::endl;
#endif
    return image;
}

// Not very efficient as we create a third big image...
//
TQImage& KImageEffect::blend(TQImage &image1, TQImage &image2,
			    GradientType gt, int xf, int yf)
{
  if (image1.width() == 0 || image1.height() == 0 ||
      image2.width() == 0 || image2.height() == 0)
    return image1;

  TQImage image3;

  image3 = KImageEffect::unbalancedGradient(image1.size(),
				    TQColor(0,0,0), TQColor(255,255,255),
				    gt, xf, yf, 0);

  return blend(image1,image2,image3, Red); // Channel to use is arbitrary
}

// Blend image2 into image1, using an RBG channel of blendImage
//
TQImage& KImageEffect::blend(TQImage &image1, TQImage &image2,
			    TQImage &blendImage, RGBComponent channel)
{
    if (image1.width() == 0 || image1.height() == 0 ||
        image2.width() == 0 || image2.height() == 0 ||
        blendImage.width() == 0 || blendImage.height() == 0) {
#ifndef NDEBUG
        std::cerr << "KImageEffect::blend effect invalid image" << std::endl;
#endif
      return image1;
    }

    int r, g, b;
    int ind1, ind2, ind3;

    unsigned int x1, x2, x3, y1, y2, y3;
    unsigned int a;

    int x, y;

    // for image1 and image2, we only handle depth 32
    if (image1.depth()<32) image1 = image1.convertDepth(32);
    if (image2.depth()<32) image2 = image2.convertDepth(32);

    // for blendImage, we handle depth 8 and 32
    if (blendImage.depth()<8) blendImage = blendImage.convertDepth(8);

    unsigned int *colorTable3 = (blendImage.depth()==8) ?
				 blendImage.colorTable():0;

    unsigned int *data1 =  (unsigned int *)image1.bits();
    unsigned int *data2 =  (unsigned int *)image2.bits();
    unsigned int *data3   =  (unsigned int *)blendImage.bits();
    unsigned char *data3b =  (unsigned char *)blendImage.bits();
    unsigned int color3;

    x1 = image1.width();     y1 = image1.height();
    x2 = image2.width();     y2 = image2.height();
    x3 = blendImage.width(); y3 = blendImage.height();

    for (y = 0; y < (int)y1; y++) {
	ind1 = x1*y;
	ind2 = x2*(y%y2);
	ind3 = x3*(y%y3);

	x=0;
	while(x < (int)x1) {
	  color3 = (colorTable3) ? colorTable3[data3b[ind3]] : data3[ind3];

          a = (channel == Red) ? tqRed(color3) :
              (channel == Green) ? tqGreen(color3) :
	      (channel == Blue) ? tqBlue(color3) : tqGray(color3);

	  r = (a*tqRed(data1[ind1]) + (256-a)*tqRed(data2[ind2]))/256;
	  g = (a*tqGreen(data1[ind1]) + (256-a)*tqGreen(data2[ind2]))/256;
	  b = (a*tqBlue(data1[ind1]) + (256-a)*tqBlue(data2[ind2]))/256;

	  a = tqAlpha(data1[ind1]);
	  data1[ind1] = tqRgba(r, g, b, a);

	  ind1++; ind2++; ind3++; x++;
	  if ( (x%x2) ==0) ind2 -= x2;
	  if ( (x%x3) ==0) ind3 -= x3;
        }
    }
    return image1;
}


//======================================================================
//
// Hash effects
//
//======================================================================

unsigned int KImageEffect::lHash(unsigned int c)
{
    unsigned char r = tqRed(c), g = tqGreen(c), b = tqBlue(c), a = tqAlpha(c);
    unsigned char nr, ng, nb;
    nr =(r >> 1) + (r >> 2); nr = nr > r ? 0 : nr;
    ng =(g >> 1) + (g >> 2); ng = ng > g ? 0 : ng;
    nb =(b >> 1) + (b >> 2); nb = nb > b ? 0 : nb;

    return tqRgba(nr, ng, nb, a);
}


// -----------------------------------------------------------------------------

unsigned int KImageEffect::uHash(unsigned int c)
{
    unsigned char r = tqRed(c), g = tqGreen(c), b = tqBlue(c), a = tqAlpha(c);
    unsigned char nr, ng, nb;
    nr = r + (r >> 3); nr = nr < r ? ~0 : nr;
    ng = g + (g >> 3); ng = ng < g ? ~0 : ng;
    nb = b + (b >> 3); nb = nb < b ? ~0 : nb;

    return tqRgba(nr, ng, nb, a);
}


// -----------------------------------------------------------------------------

TQImage& KImageEffect::hash(TQImage &image, Lighting lite, unsigned int spacing)
{
    if (image.width() == 0 || image.height() == 0) {
#ifndef NDEBUG
        std::cerr << "KImageEffect::hash effect invalid image" << std::endl;
#endif
      return image;
    }

    int x, y;
    unsigned int *data =  (unsigned int *)image.bits();
    unsigned int ind;

    //CT no need to do it if not enough space
    if ((lite == NorthLite ||
         lite == SouthLite)&&
        (unsigned)image.height() < 2+spacing) return image;
    if ((lite == EastLite ||
         lite == WestLite)&&
        (unsigned)image.height() < 2+spacing) return image;

    if (lite == NorthLite || lite == SouthLite) {
        for (y = 0 ; y < image.height(); y = y + 2 + spacing) {
            for (x = 0; x < image.width(); x++) {
                ind = x + image.width() * y;
                data[ind] = lite==NorthLite?uHash(data[ind]):lHash(data[ind]);

                ind = ind + image.width();
                data[ind] = lite==NorthLite?lHash(data[ind]):uHash(data[ind]);
            }
        }
    }

    else if (lite == EastLite || lite == WestLite) {
        for (y = 0 ; y < image.height(); y++) {
            for (x = 0; x < image.width(); x = x + 2 + spacing) {
                ind = x + image.width() * y;
                data[ind] = lite==EastLite?uHash(data[ind]):lHash(data[ind]);

                ind++;
                data[ind] = lite==EastLite?lHash(data[ind]):uHash(data[ind]);
            }
        }
    }

    else if (lite == NWLite || lite == SELite) {
        for (y = 0 ; y < image.height(); y++) {
            for (x = 0;
                 x < (int)(image.width() - ((y & 1)? 1 : 0) * spacing);
                 x = x + 2 + spacing) {
                ind = x + image.width() * y + ((y & 1)? 1 : 0);
                data[ind] = lite==NWLite?uHash(data[ind]):lHash(data[ind]);

                ind++;
                data[ind] = lite==NWLite?lHash(data[ind]):uHash(data[ind]);
            }
        }
    }

    else if (lite == SWLite || lite == NELite) {
        for (y = 0 ; y < image.height(); y++) {
            for (x = 0  + ((y & 1)? 1 : 0); x < image.width(); x = x + 2 + spacing) {
                ind = x + image.width() * y - ((y & 1)? 1 : 0);
                data[ind] = lite==SWLite?uHash(data[ind]):lHash(data[ind]);

                ind++;
                data[ind] = lite==SWLite?lHash(data[ind]):uHash(data[ind]);
            }
        }
    }

    return image;
}


//======================================================================
//
// Flatten effects
//
//======================================================================

TQImage& KImageEffect::flatten(TQImage &img, const TQColor &ca,
                            const TQColor &cb, int ncols)
{
    if (img.width() == 0 || img.height() == 0)
      return img;

    // a bitmap is easy...
    if (img.depth() == 1) {
	img.setColor(0, ca.rgb());
	img.setColor(1, cb.rgb());
	return img;
    }

    int r1 = ca.red(); int r2 = cb.red();
    int g1 = ca.green(); int g2 = cb.green();
    int b1 = ca.blue(); int b2 = cb.blue();
    int min = 0, max = 255;

    TQRgb col;

    // Get minimum and maximum greylevel.
    if (img.numColors()) {
	// pseudocolor
	for (int i = 0; i < img.numColors(); i++) {
	    col = img.color(i);
	    int mean = (tqRed(col) + tqGreen(col) + tqBlue(col)) / 3;
	    min = TQMIN(min, mean);
	    max = TQMAX(max, mean);
	}
    } else {
	// truecolor
	for (int y=0; y < img.height(); y++)
	    for (int x=0; x < img.width(); x++) {
		col = img.pixel(x, y);
		int mean = (tqRed(col) + tqGreen(col) + tqBlue(col)) / 3;
		min = TQMIN(min, mean);
		max = TQMAX(max, mean);
	    }
    }

    // Conversion factors
    float sr = ((float) r2 - r1) / (max - min);
    float sg = ((float) g2 - g1) / (max - min);
    float sb = ((float) b2 - b1) / (max - min);


    // Repaint the image
    if (img.numColors()) {
	for (int i=0; i < img.numColors(); i++) {
	    col = img.color(i);
	    int mean = (tqRed(col) + tqGreen(col) + tqBlue(col)) / 3;
	    int r = (int) (sr * (mean - min) + r1 + 0.5);
	    int g = (int) (sg * (mean - min) + g1 + 0.5);
	    int b = (int) (sb * (mean - min) + b1 + 0.5);
	    img.setColor(i, tqRgba(r, g, b, tqAlpha(col)));
	}
    } else {
	for (int y=0; y < img.height(); y++)
	    for (int x=0; x < img.width(); x++) {
		col = img.pixel(x, y);
		int mean = (tqRed(col) + tqGreen(col) + tqBlue(col)) / 3;
		int r = (int) (sr * (mean - min) + r1 + 0.5);
		int g = (int) (sg * (mean - min) + g1 + 0.5);
		int b = (int) (sb * (mean - min) + b1 + 0.5);
		img.setPixel(x, y, tqRgba(r, g, b, tqAlpha(col)));
	    }
    }


    // Dither if necessary
    if ( (ncols <= 0) || ((img.numColors() != 0) && (img.numColors() <= ncols)))
	return img;

    if (ncols == 1) ncols++;
    if (ncols > 256) ncols = 256;

    TQColor *pal = new TQColor[ncols];
    sr = ((float) r2 - r1) / (ncols - 1);
    sg = ((float) g2 - g1) / (ncols - 1);
    sb = ((float) b2 - b1) / (ncols - 1);

    for (int i=0; i<ncols; i++)
	pal[i] = TQColor(r1 + int(sr*i), g1 + int(sg*i), b1 + int(sb*i));

    dither(img, pal, ncols);

    delete[] pal;
    return img;
}


//======================================================================
//
// Fade effects
//
//======================================================================

TQImage& KImageEffect::fade(TQImage &img, float val, const TQColor &color)
{
    if (img.width() == 0 || img.height() == 0)
      return img;

    // We don't handle bitmaps
    if (img.depth() == 1)
	return img;

    unsigned char tbl[256];
    for (int i=0; i<256; i++)
	tbl[i] = (int) (val * i + 0.5);

    int red = color.red();
    int green = color.green();
    int blue = color.blue();

    TQRgb col;
    int r, g, b, cr, cg, cb;

    if (img.depth() <= 8) {
	// pseudo color
	for (int i=0; i<img.numColors(); i++) {
	    col = img.color(i);
	    cr = tqRed(col); cg = tqGreen(col); cb = tqBlue(col);
	    if (cr > red)
		r = cr - tbl[cr - red];
	    else
		r = cr + tbl[red - cr];
	    if (cg > green)
		g = cg - tbl[cg - green];
	    else
		g = cg + tbl[green - cg];
	    if (cb > blue)
		b = cb - tbl[cb - blue];
	    else
		b = cb + tbl[blue - cb];
	    img.setColor(i, tqRgba(r, g, b, tqAlpha(col)));
	}

    } else {
	// truecolor
        for (int y=0; y<img.height(); y++) {
            TQRgb *data = (TQRgb *) img.scanLine(y);
            for (int x=0; x<img.width(); x++) {
                col = *data;
                cr = tqRed(col); cg = tqGreen(col); cb = tqBlue(col);
                if (cr > red)
                    r = cr - tbl[cr - red];
                else
                    r = cr + tbl[red - cr];
                if (cg > green)
                    g = cg - tbl[cg - green];
                else
                    g = cg + tbl[green - cg];
                if (cb > blue)
                    b = cb - tbl[cb - blue];
                else
                    b = cb + tbl[blue - cb];
                *data++ = tqRgba(r, g, b, tqAlpha(col));
            }
        }
    }

    return img;
}

//======================================================================
//
// Color effects
//
//======================================================================

// This code is adapted from code (C) Rik Hemsley <rik@kde.org>
//
// The formula used (r + b + g) /3 is different from the tqGray formula
// used by Qt.  This is because our formula is much much faster.  If,
// however, it turns out that this is producing sub-optimal images,
// then it will have to change (kurt)
//
// It does produce lower quality grayscale ;-) Use fast == true for the fast
// algorithm, false for the higher quality one (mosfet).
TQImage& KImageEffect::toGray(TQImage &img, bool fast)
{
    if (img.width() == 0 || img.height() == 0)
      return img;

    if(fast){
        if (img.depth() == 32) {
            uchar * r(img.bits());
            uchar * g(img.bits() + 1);
            uchar * b(img.bits() + 2);

            uchar * end(img.bits() + img.numBytes());

            while (r != end) {

                *r = *g = *b = (((*r + *g) >> 1) + *b) >> 1; // (r + b + g) / 3

                r += 4;
                g += 4;
                b += 4;
            }
        }
        else
        {
            for (int i = 0; i < img.numColors(); i++)
            {
                uint r = tqRed(img.color(i));
                uint g = tqGreen(img.color(i));
                uint b = tqBlue(img.color(i));

                uint gray = (((r + g) >> 1) + b) >> 1;
                img.setColor(i, tqRgba(gray, gray, gray, tqAlpha(img.color(i))));
            }
        }
    }
    else{
        int pixels = img.depth() > 8 ? img.width()*img.height() :
            img.numColors();
        unsigned int *data = img.depth() > 8 ? (unsigned int *)img.bits() :
            (unsigned int *)img.colorTable();
        int val, i;
        for(i=0; i < pixels; ++i){
            val = tqGray(data[i]);
            data[i] = tqRgba(val, val, val, tqAlpha(data[i]));
        }
    }
    return img;
}

// CT 29Jan2000 - desaturation algorithms
TQImage& KImageEffect::desaturate(TQImage &img, float desat)
{
    if (img.width() == 0 || img.height() == 0)
      return img;

    if (desat < 0) desat = 0.;
    if (desat > 1) desat = 1.;
    int pixels = img.depth() > 8 ? img.width()*img.height() :
        img.numColors();
    unsigned int *data = img.depth() > 8 ? (unsigned int *)img.bits() :
        (unsigned int *)img.colorTable();
    int h, s, v, i;
    TQColor clr; // keep constructor out of loop (mosfet)
    for(i=0; i < pixels; ++i){
        clr.setRgb(data[i]);
	clr.hsv(&h, &s, &v);
	clr.setHsv(h, (int)(s * (1. - desat)), v);
	data[i] = clr.rgb();
    }
    return img;
}

// Contrast stuff (mosfet)
TQImage& KImageEffect::contrast(TQImage &img, int c)
{
    if (img.width() == 0 || img.height() == 0)
      return img;

    if(c > 255)
        c = 255;
    if(c < -255)
        c =  -255;
    int pixels = img.depth() > 8 ? img.width()*img.height() :
        img.numColors();
    unsigned int *data = img.depth() > 8 ? (unsigned int *)img.bits() :
        (unsigned int *)img.colorTable();
    int i, r, g, b;
    for(i=0; i < pixels; ++i){
        r = tqRed(data[i]);
        g = tqGreen(data[i]);
        b = tqBlue(data[i]);
        if(tqGray(data[i]) <= 127){
            if(r - c > 0)
                r -= c;
            else
                r = 0;
            if(g - c > 0)
                g -= c;
            else
                g = 0;
            if(b - c > 0)
                b -= c;
            else
                b = 0;
        }
        else{
            if(r + c <= 255)
                r += c;
            else
                r = 255;
            if(g + c <= 255)
                g += c;
            else
                g = 255;
            if(b + c <= 255)
                b += c;
            else
                b = 255;
        }
        data[i] = tqRgba(r, g, b, tqAlpha(data[i]));
    }
    return(img);
}

//======================================================================
//
// Dithering effects
//
//======================================================================

// adapted from kFSDither (C) 1997 Martin Jones (mjones@kde.org)
//
// Floyd-Steinberg dithering
// Ref: Bitmapped Graphics Programming in C++
//      Marv Luse, Addison-Wesley Publishing, 1993.
TQImage& KImageEffect::dither(TQImage &img, const TQColor *palette, int size)
{
    if (img.width() == 0 || img.height() == 0 ||
        palette == 0 || img.depth() <= 8)
      return img;

    TQImage dImage( img.width(), img.height(), 8, size );
    int i;

    dImage.setNumColors( size );
    for ( i = 0; i < size; i++ )
        dImage.setColor( i, palette[ i ].rgb() );

    int *rerr1 = new int [ img.width() * 2 ];
    int *gerr1 = new int [ img.width() * 2 ];
    int *berr1 = new int [ img.width() * 2 ];

    memset( rerr1, 0, sizeof( int ) * img.width() * 2 );
    memset( gerr1, 0, sizeof( int ) * img.width() * 2 );
    memset( berr1, 0, sizeof( int ) * img.width() * 2 );

    int *rerr2 = rerr1 + img.width();
    int *gerr2 = gerr1 + img.width();
    int *berr2 = berr1 + img.width();

    for ( int j = 0; j < img.height(); j++ )
    {
        uint *ip = (uint * )img.scanLine( j );
        uchar *dp = dImage.scanLine( j );

        for ( i = 0; i < img.width(); i++ )
        {
            rerr1[i] = rerr2[i] + tqRed( *ip );
            rerr2[i] = 0;
            gerr1[i] = gerr2[i] + tqGreen( *ip );
            gerr2[i] = 0;
            berr1[i] = berr2[i] + tqBlue( *ip );
            berr2[i] = 0;
            ip++;
        }

        *dp++ = nearestColor( rerr1[0], gerr1[0], berr1[0], palette, size );

        for ( i = 1; i < img.width()-1; i++ )
        {
            int indx = nearestColor( rerr1[i], gerr1[i], berr1[i], palette, size );
            *dp = indx;

            int rerr = rerr1[i];
            rerr -= palette[indx].red();
            int gerr = gerr1[i];
            gerr -= palette[indx].green();
            int berr = berr1[i];
            berr -= palette[indx].blue();

            // diffuse red error
            rerr1[ i+1 ] += ( rerr * 7 ) >> 4;
            rerr2[ i-1 ] += ( rerr * 3 ) >> 4;
            rerr2[  i  ] += ( rerr * 5 ) >> 4;
            rerr2[ i+1 ] += ( rerr ) >> 4;

            // diffuse green error
            gerr1[ i+1 ] += ( gerr * 7 ) >> 4;
            gerr2[ i-1 ] += ( gerr * 3 ) >> 4;
            gerr2[  i  ] += ( gerr * 5 ) >> 4;
            gerr2[ i+1 ] += ( gerr ) >> 4;

            // diffuse red error
            berr1[ i+1 ] += ( berr * 7 ) >> 4;
            berr2[ i-1 ] += ( berr * 3 ) >> 4;
            berr2[  i  ] += ( berr * 5 ) >> 4;
            berr2[ i+1 ] += ( berr ) >> 4;

            dp++;
        }

        *dp = nearestColor( rerr1[i], gerr1[i], berr1[i], palette, size );
    }

    delete [] rerr1;
    delete [] gerr1;
    delete [] berr1;

    img = dImage;
    return img;
}

int KImageEffect::nearestColor( int r, int g, int b, const TQColor *palette, int size )
{
    if (palette == 0)
      return 0;

    int dr = palette[0].red() - r;
    int dg = palette[0].green() - g;
    int db = palette[0].blue() - b;

    int minDist =  dr*dr + dg*dg + db*db;
    int nearest = 0;

    for (int i = 1; i < size; i++ )
    {
        dr = palette[i].red() - r;
        dg = palette[i].green() - g;
        db = palette[i].blue() - b;

        int dist = dr*dr + dg*dg + db*db;

        if ( dist < minDist )
        {
            minDist = dist;
            nearest = i;
        }
    }

    return nearest;
}

bool KImageEffect::blend(
    const TQImage & upper,
    const TQImage & lower,
    TQImage & output
)
{
  if (
      upper.width()  > lower.width()  ||
      upper.height() > lower.height() ||
      upper.depth() != 32             ||
      lower.depth() != 32
  )
  {
#ifndef NDEBUG
    std::cerr << "KImageEffect::blend : Sizes not correct\n" ;
#endif
    return false;
  }

  output = lower.copy();

  uchar *i, *o;
  int a;
  int col;
  int w = upper.width();
  int row(upper.height() - 1);

  do {

    i = const_cast<TQImage&>(upper).scanLine(row);
    o = const_cast<TQImage&>(output).scanLine(row);

    col = w << 2;
    --col;

    do {

      while (!(a = i[col]) && (col != 3)) {
        --col; --col; --col; --col;
      }

      --col;
      o[col] += ((i[col] - o[col]) * a) >> 8;

      --col;
      o[col] += ((i[col] - o[col]) * a) >> 8;

      --col;
      o[col] += ((i[col] - o[col]) * a) >> 8;

    } while (col--);

  } while (row--);

  return true;
}

#if 0
// Not yet...
bool KImageEffect::blend(
    const TQImage & upper,
    const TQImage & lower,
    TQImage & output,
    const TQRect & destRect
)
{
  output = lower.copy();
  return output;
}

#endif

bool KImageEffect::blend(
    int &x, int &y,
    const TQImage & upper,
    const TQImage & lower,
    TQImage & output
)
{
  int cx=0, cy=0, cw=upper.width(), ch=upper.height();

  if ( upper.width() + x > lower.width()  ||
      upper.height() + y > lower.height() ||
      x < 0 || y < 0 ||
      upper.depth() != 32 || lower.depth() != 32 )
  {
    if ( x > lower.width() || y > lower.height() ) return false;
    if ( upper.width()<=0 || upper.height() <= 0 ) return false;
    if ( lower.width()<=0 || lower.height() <= 0 ) return false;

    if (x<0) {cx=-x; cw+=x; x=0; };
    if (cw + x > lower.width()) { cw=lower.width()-x; };
    if (y<0) {cy=-y; ch+=y; y=0; };
    if (ch + y > lower.height()) { ch=lower.height()-y; };

    if ( cx >= upper.width() || cy >= upper.height() ) return true;
    if ( cw <= 0 || ch <= 0 ) return true;
  }

  output.create(cw,ch,32);
//  output.setAlphaBuffer(true); // I should do some benchmarks to see if
	// this is worth the effort

  TQRgb *i, *o, *b;

  int a;
  int j,k;
  for (j=0; j<ch; j++)
  {
    b=reinterpret_cast<TQRgb *>(&const_cast<TQImage&>(lower).scanLine(y+j) [ (x+cw) << 2 ]);
    i=reinterpret_cast<TQRgb *>(&const_cast<TQImage&>(upper).scanLine(cy+j)[ (cx+cw) << 2 ]);
    o=reinterpret_cast<TQRgb *>(&const_cast<TQImage&>(output).scanLine(j)  [ cw << 2 ]);

    k=cw-1;
    --b; --i; --o;
    do
    {
      while ( !(a=tqAlpha(*i)) && k>0 )
      {
        i--;
//	*o=0;
	*o=*b;
	--o; --b;
	k--;
      };
//      *o=0xFF;
      *o = tqRgb(tqRed(*b) + (((tqRed(*i) - tqRed(*b)) * a) >> 8),
                tqGreen(*b) + (((tqGreen(*i) - tqGreen(*b)) * a) >> 8),
                tqBlue(*b) + (((tqBlue(*i) - tqBlue(*b)) * a) >> 8));
      --i; --o; --b;
    } while (k--);
  }

  return true;
}

bool KImageEffect::blendOnLower(
    int x, int y,
    const TQImage & upper,
    const TQImage & lower
)
{
  int cx=0, cy=0, cw=upper.width(), ch=upper.height();

  if ( upper.depth() != 32 || lower.depth() != 32 ) return false;
  if ( x + cw > lower.width()  ||
      y + ch > lower.height() ||
      x < 0 || y < 0 )
  {
    if ( x > lower.width() || y > lower.height() ) return true;
    if ( upper.width()<=0 || upper.height() <= 0 ) return true;
    if ( lower.width()<=0 || lower.height() <= 0 ) return true;

    if (x<0) {cx=-x; cw+=x; x=0; };
    if (cw + x > lower.width()) { cw=lower.width()-x; };
    if (y<0) {cy=-y; ch+=y; y=0; };
    if (ch + y > lower.height()) { ch=lower.height()-y; };

    if ( cx >= upper.width() || cy >= upper.height() ) return true;
    if ( cw <= 0 || ch <= 0 ) return true;
  }

  uchar *i, *b;
  int a;
  int k;

  for (int j=0; j<ch; j++)
  {
    b=&const_cast<TQImage&>(lower).scanLine(y+j) [ (x+cw) << 2 ];
    i=&const_cast<TQImage&>(upper).scanLine(cy+j)[ (cx+cw) << 2 ];

    k=cw-1;
    --b; --i;
    do
    {
#ifndef WORDS_BIGENDIAN
      while ( !(a=*i) && k>0 )
#else
      while ( !(a=*(i-3)) && k>0 )
#endif
      {
        i-=4; b-=4; k--;
      };

#ifndef WORDS_BIGENDIAN
      --i; --b;
      *b += ( ((*i - *b) * a) >> 8 );
      --i; --b;
      *b += ( ((*i - *b) * a) >> 8 );
      --i; --b;
      *b += ( ((*i - *b) * a) >> 8 );
      --i; --b;
#else
      *b += ( ((*i - *b) * a) >> 8 );
      --i; --b;
      *b += ( ((*i - *b) * a) >> 8 );
      --i; --b;
      *b += ( ((*i - *b) * a) >> 8 );
      i -= 2; b -= 2;
#endif
    } while (k--);
  }

  return true;
}

void KImageEffect::blendOnLower(const TQImage &upper, const TQPoint &upperOffset,
                                TQImage &lower, const TQRect &lowerRect)
{
    // clip rect
    TQRect lr =  lowerRect & lower.rect();
    lr.setWidth( TQMIN(lr.width(), upper.width()-upperOffset.x()) );
    lr.setHeight( TQMIN(lr.height(), upper.height()-upperOffset.y()) );
    if ( !lr.isValid() ) return;

    // blend
    for (int y = 0; y < lr.height(); y++) {
        for (int x = 0; x < lr.width(); x++) {
            TQRgb *b = reinterpret_cast<TQRgb*>(const_cast<TQImage&>(lower).scanLine(lr.y() + y)+ (lr.x() + x) * sizeof(TQRgb));
            TQRgb *d = reinterpret_cast<TQRgb*>(const_cast<TQImage&>(upper).scanLine(upperOffset.y() + y) + (upperOffset.x() + x) * sizeof(TQRgb));
            int a = tqAlpha(*d);
            *b = tqRgb(tqRed(*b) - (((tqRed(*b) - tqRed(*d)) * a) >> 8),
                      tqGreen(*b) - (((tqGreen(*b) - tqGreen(*d)) * a) >> 8),
                      tqBlue(*b) - (((tqBlue(*b) - tqBlue(*d)) * a) >> 8));
        }
    }
}

void KImageEffect::blendOnLower(const TQImage &upper, const TQPoint &upperOffset,
                          TQImage &lower, const TQRect &lowerRect, float opacity)
{
    // clip rect
    TQRect lr =  lowerRect & lower.rect();
    lr.setWidth( TQMIN(lr.width(), upper.width()-upperOffset.x()) );
    lr.setHeight( TQMIN(lr.height(), upper.height()-upperOffset.y()) );
    if ( !lr.isValid() ) return;

    // blend
    for (int y = 0; y < lr.height(); y++) {
        for (int x = 0; x < lr.width(); x++) {
            TQRgb *b = reinterpret_cast<TQRgb*>(const_cast<TQImage&>(lower).scanLine(lr.y() + y)+ (lr.x() + x) * sizeof(TQRgb));
            TQRgb *d = reinterpret_cast<TQRgb*>(const_cast<TQImage&>(upper).scanLine(upperOffset.y() + y) + (upperOffset.x() + x) * sizeof(TQRgb));
            int a = tqRound(opacity * tqAlpha(*d));
            *b = tqRgb(tqRed(*b) - (((tqRed(*b) - tqRed(*d)) * a) >> 8),
                      tqGreen(*b) - (((tqGreen(*b) - tqGreen(*d)) * a) >> 8),
                      tqBlue(*b) - (((tqBlue(*b) - tqBlue(*d)) * a) >> 8));
        }
    }
}

TQRect KImageEffect::computeDestinationRect(const TQSize &lowerSize,
                                       Disposition disposition, TQImage &upper)
{
    int w = lowerSize.width();
    int h = lowerSize.height();
    int ww = upper.width();
    int wh = upper.height();
    TQRect d;

    switch (disposition) {
    case NoImage:
        break;
    case Centered:
        d.setRect((w - ww) / 2, (h - wh) / 2, ww, wh);
        break;
    case Tiled:
        d.setRect(0, 0, w, h);
        break;
    case CenterTiled:
        d.setCoords(-ww + ((w - ww) / 2) % ww, -wh + ((h - wh) / 2) % wh,
                    w-1, h-1);
        break;
    case Scaled:
        upper = upper.smoothScale(w, h);
        d.setRect(0, 0, w, h);
        break;
    case CenteredAutoFit:
        if( ww <= w && wh <= h ) {
            d.setRect((w - ww) / 2, (h - wh) / 2, ww, wh); // like Centered
            break;
        }
        // fall through
    case CenteredMaxpect: {
        double sx = (double) w / ww;
        double sy = (double) h / wh;
        if (sx > sy) {
            ww = (int)(sy * ww);
            wh = h;
        } else {
            wh = (int)(sx * wh);
            ww = w;
        }
        upper = upper.smoothScale(ww, wh);
        d.setRect((w - ww) / 2, (h - wh) / 2, ww, wh);
        break;
    }
    case TiledMaxpect: {
        double sx = (double) w / ww;
        double sy = (double) h / wh;
        if (sx > sy) {
            ww = (int)(sy * ww);
            wh = h;
        } else {
            wh = (int)(sx * wh);
            ww = w;
        }
        upper = upper.smoothScale(ww, wh);
        d.setRect(0, 0, w, h);
        break;
    }
    }

    return d;
}

void KImageEffect::blendOnLower(TQImage &upper, TQImage &lower,
                                Disposition disposition, float opacity)
{
    TQRect r = computeDestinationRect(lower.size(), disposition, upper);
    for (int y = r.top(); y<r.bottom(); y += upper.height())
        for (int x = r.left(); x<r.right(); x += upper.width())
            blendOnLower(upper, TQPoint(-TQMIN(x, 0), -TQMIN(y, 0)),
                   lower, TQRect(x, y, upper.width(), upper.height()), opacity);
}


// For selected icons
TQImage& KImageEffect::selectedImage( TQImage &img, const TQColor &col )
{
    return blend( col, img, 0.5);
}

//
// ===================================================================
// Effects originally ported from ImageMagick for PixiePlus, plus a few
// new ones. (mosfet 05/26/2003)
// ===================================================================
//
/*
 Portions of this software are based on ImageMagick. Such portions are clearly
marked as being ported from ImageMagick. ImageMagick is copyrighted under the
following conditions:

Copyright (C) 2003 ImageMagick Studio, a non-profit organization dedicated to
making software imaging solutions freely available.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files ("ImageMagick"), to deal
in ImageMagick without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense,  and/or sell
copies of ImageMagick, and to permit persons to whom the ImageMagick is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of ImageMagick.

The software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement.  In no event shall
ImageMagick Studio be liable for any claim, damages or other liability,
whether in an action of contract, tort or otherwise, arising from, out of or
in connection with ImageMagick or the use or other dealings in ImageMagick.

Except as contained in this notice, the name of the ImageMagick Studio shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in ImageMagick without prior written authorization from the
ImageMagick Studio.
*/

TQImage KImageEffect::sample(TQImage &src, int w, int h)
{
    if(w == src.width() && h == src.height())
        return(src);

    int depth = src.depth();
    TQImage dest(w, h, depth, depth <= 8 ? src.numColors() : 0,
    depth == 1 ? TQImage::LittleEndian : TQImage::IgnoreEndian);
    int *x_offset = (int *)malloc(w*sizeof(int));
    int *y_offset = (int *)malloc(h*sizeof(int));
    if(!x_offset || !y_offset){
#ifndef NDEBUG
        tqWarning("KImageEffect::sample(): Unable to allocate pixel buffer");
#endif
        free(x_offset);
        free(y_offset);
        return(src);
    }

    // init pixel offsets
    for(int x=0; x < w; ++x)
        x_offset[x] = (int)(x*src.width()/((double)w));
    for(int y=0; y < h; ++y)
        y_offset[y] = (int)(y*src.height()/((double)h));

    if(depth > 8){ // DirectClass source image
        for(int y=0; y < h; ++y){
            unsigned int *destData = (unsigned int *)dest.scanLine(y);
            unsigned int *srcData = (unsigned int *)src.scanLine(y_offset[y]);
            for(int x=0; x < w; ++x)
                destData[x] = srcData[x_offset[x]];
        }
    }
    else if(depth == 1) {
        int r = src.bitOrder() == TQImage::LittleEndian;
        memcpy(dest.colorTable(), src.colorTable(), src.numColors()*sizeof(TQRgb));
        for(int y=0; y < h; ++y){
            unsigned char *destData = dest.scanLine(y);
            unsigned char *srcData = src.scanLine(y_offset[y]);
            for(int x=0; x < w; ++x){
                int k = x_offset[x];
                int l = r ? (k & 7) : (7 - (k&7));
                if(srcData[k >> 3] & (1 << l))
                    destData[x >> 3] |= 1 << (x & 7);
                else
                    destData[x >> 3] &= ~(1 << (x & 7));
            }
        }
    }
    else{ // PseudoClass source image
        memcpy(dest.colorTable(), src.colorTable(), src.numColors()*sizeof(TQRgb));
        for(int y=0; y < h; ++y){
            unsigned char *destData = dest.scanLine(y);
            unsigned char *srcData = src.scanLine(y_offset[y]);
            for(int x=0; x < w; ++x)
                destData[x] = srcData[x_offset[x]];
        }
    }
    free(x_offset);
    free(y_offset);
    return(dest);
}

void KImageEffect::threshold(TQImage &img, unsigned int threshold)
{
    int i, count;
    unsigned int *data;
    if(img.depth() > 8){ // DirectClass
        count = img.width()*img.height();
        data = (unsigned int *)img.bits();
    }
    else{ // PsudeoClass
        count = img.numColors();
        data = (unsigned int *)img.colorTable();
    }
    for(i=0; i < count; ++i)
        data[i] = intensityValue(data[i]) < threshold ? TQColor(TQt::black).rgb() : TQColor(TQt::white).rgb();
}

void KImageEffect::hull(const int x_offset, const int y_offset,
                        const int polarity, const int columns,
                        const int rows,
                        unsigned int *f, unsigned int *g)
{
    int x, y;

    unsigned int *p, *q, *r, *s;
    unsigned int v;
    if(f == NULL || g == NULL)
        return;
    p=f+(columns+2);
    q=g+(columns+2);
    r=p+(y_offset*(columns+2)+x_offset);
    for (y=0; y < rows; y++){
        p++;
        q++;
        r++;
        if(polarity > 0)
            for (x=0; x < columns; x++){
                v=(*p);
                if (*r > v)
                    v++;
                *q=v;
                p++;
                q++;
                r++;
            }
        else
            for(x=0; x < columns; x++){
                v=(*p);
                if (v > (unsigned int) (*r+1))
                    v--;
                *q=v;
                p++;
                q++;
                r++;
            }
        p++;
        q++;
        r++;
    }
    p=f+(columns+2);
    q=g+(columns+2);
    r=q+(y_offset*(columns+2)+x_offset);
    s=q-(y_offset*(columns+2)+x_offset);
    for(y=0; y < rows; y++){
        p++;
        q++;
        r++;
        s++;
        if(polarity > 0)
            for(x=0; x < (int) columns; x++){
                v=(*q);
                if (((unsigned int) (*s+1) > v) && (*r > v))
                    v++;
                *p=v;
                p++;
                q++;
                r++;
                s++;
            }
        else
            for (x=0; x < columns; x++){
                v=(*q);
                if (((unsigned int) (*s+1) < v) && (*r < v))
                    v--;
                *p=v;
                p++;
                q++;
                r++;
                s++;
            }
        p++;
        q++;
        r++;
        s++;
    }
}

TQImage KImageEffect::despeckle(TQImage &src)
{
    int i, j, x, y;
    unsigned int *blue_channel, *red_channel, *green_channel, *buffer,
        *alpha_channel;
    int packets;
    static const int
    X[4]= {0, 1, 1,-1},
    Y[4]= {1, 0, 1, 1};

    unsigned int *destData;
    TQImage dest(src.width(), src.height(), 32);

    packets = (src.width()+2)*(src.height()+2);
    red_channel = (unsigned int *)calloc(packets, sizeof(unsigned int));
    green_channel = (unsigned int *)calloc(packets, sizeof(unsigned int));
    blue_channel = (unsigned int *)calloc(packets, sizeof(unsigned int));
    alpha_channel = (unsigned int *)calloc(packets, sizeof(unsigned int));
    buffer = (unsigned int *)calloc(packets, sizeof(unsigned int));
    if(!red_channel || ! green_channel || ! blue_channel || ! alpha_channel ||
       !buffer){
        free(red_channel);
        free(green_channel);
        free(blue_channel);
        free(alpha_channel);
        free(buffer);
        return(src);
    }

    // copy image pixels to color component buffers
    j = src.width()+2;
    if(src.depth() > 8){ // DirectClass source image
        unsigned int *srcData;
        for(y=0; y < src.height(); ++y){
            srcData = (unsigned int *)src.scanLine(y);
            ++j;
            for(x=0; x < src.width(); ++x){
                red_channel[j] = tqRed(srcData[x]);
                green_channel[j] = tqGreen(srcData[x]);
                blue_channel[j] = tqBlue(srcData[x]);
                alpha_channel[j] = tqAlpha(srcData[x]);
                ++j;
            }
            ++j;
        }
    }
    else{ // PsudeoClass source image
        unsigned char *srcData;
        unsigned int *cTable = src.colorTable();
        unsigned int pixel;
        for(y=0; y < src.height(); ++y){
            srcData = (unsigned char *)src.scanLine(y);
            ++j;
            for(x=0; x < src.width(); ++x){
                pixel = *(cTable+srcData[x]);
                red_channel[j] = tqRed(pixel);
                green_channel[j] = tqGreen(pixel);
                blue_channel[j] = tqBlue(pixel);
                alpha_channel[j] = tqAlpha(pixel);
                ++j;
            }
            ++j;
        }
    }
    // reduce speckle in red channel
    for(i=0; i < 4; i++){
        hull(X[i],Y[i],1,src.width(),src.height(),red_channel,buffer);
        hull(-X[i],-Y[i],1,src.width(),src.height(),red_channel,buffer);
        hull(-X[i],-Y[i],-1,src.width(),src.height(),red_channel,buffer);
        hull(X[i],Y[i],-1,src.width(),src.height(),red_channel,buffer);
    }
    // reduce speckle in green channel
    for (i=0; i < packets; i++)
        buffer[i]=0;
    for (i=0; i < 4; i++){
        hull(X[i],Y[i],1,src.width(),src.height(),green_channel,buffer);
        hull(-X[i],-Y[i],1,src.width(),src.height(),green_channel,buffer);
        hull(-X[i],-Y[i],-1,src.width(),src.height(),green_channel,buffer);
        hull(X[i],Y[i],-1,src.width(),src.height(),green_channel,buffer);
    }
    // reduce speckle in blue channel
    for (i=0; i < packets; i++)
        buffer[i]=0;
    for (i=0; i < 4; i++){
        hull(X[i],Y[i],1,src.width(),src.height(),blue_channel,buffer);
        hull(-X[i],-Y[i],1,src.width(),src.height(),blue_channel,buffer);
        hull(-X[i],-Y[i],-1,src.width(),src.height(),blue_channel,buffer);
        hull(X[i],Y[i],-1,src.width(),src.height(),blue_channel,buffer);
    }
    // copy color component buffers to despeckled image
    j = dest.width()+2;
    for(y=0; y < dest.height(); ++y)
    {
        destData = (unsigned int *)dest.scanLine(y);
        ++j;
        for (x=0; x < dest.width(); ++x)
        {
            destData[x] = tqRgba(red_channel[j], green_channel[j],
                                blue_channel[j], alpha_channel[j]);
            ++j;
        }
        ++j;
    }
    free(buffer);
    free(red_channel);
    free(green_channel);
    free(blue_channel);
    free(alpha_channel);
    return(dest);
}

unsigned int KImageEffect::generateNoise(unsigned int pixel,
                                         NoiseType noise_type)
{
#define NoiseEpsilon  1.0e-5
#define NoiseMask  0x7fff
#define SigmaUniform  4.0
#define SigmaGaussian  4.0
#define SigmaImpulse  0.10
#define SigmaLaplacian 10.0
#define SigmaMultiplicativeGaussian  0.5
#define SigmaPoisson  0.05
#define TauGaussian  20.0

    double alpha, beta, sigma, value;
    alpha=(double) (rand() & NoiseMask)/NoiseMask;
    if (alpha == 0.0)
        alpha=1.0;
    switch(noise_type){
    case UniformNoise:
    default:
        {
            value=(double) pixel+SigmaUniform*(alpha-0.5);
            break;
        }
    case GaussianNoise:
        {
            double tau;

            beta=(double) (rand() & NoiseMask)/NoiseMask;
            sigma=sqrt(-2.0*log(alpha))*cos(2.0*M_PI*beta);
            tau=sqrt(-2.0*log(alpha))*sin(2.0*M_PI*beta);
            value=(double) pixel+
                (sqrt((double) pixel)*SigmaGaussian*sigma)+(TauGaussian*tau);
            break;
        }
    case MultiplicativeGaussianNoise:
        {
            if (alpha <= NoiseEpsilon)
                sigma=MaxRGB;
            else
                sigma=sqrt(-2.0*log(alpha));
            beta=(rand() & NoiseMask)/NoiseMask;
            value=(double) pixel+
                pixel*SigmaMultiplicativeGaussian*sigma*cos(2.0*M_PI*beta);
            break;
        }
    case ImpulseNoise:
        {
            if (alpha < (SigmaImpulse/2.0))
                value=0;
            else
                if (alpha >= (1.0-(SigmaImpulse/2.0)))
                    value=MaxRGB;
                else
                    value=pixel;
            break;
        }
    case LaplacianNoise:
        {
            if (alpha <= 0.5)
            {
                if (alpha <= NoiseEpsilon)
                    value=(double) pixel-MaxRGB;
                else
                    value=(double) pixel+SigmaLaplacian*log(2.0*alpha);
                break;
            }
            beta=1.0-alpha;
            if (beta <= (0.5*NoiseEpsilon))
                value=(double) pixel+MaxRGB;
            else
                value=(double) pixel-SigmaLaplacian*log(2.0*beta);
            break;
        }
    case PoissonNoise:
        {
            int
                i;

            for (i=0; alpha > exp(-SigmaPoisson*pixel); i++)
            {
                beta=(double) (rand() & NoiseMask)/NoiseMask;
                alpha=alpha*beta;
            }
            value=i/SigmaPoisson;
            break;
        }
    }
    if(value < 0.0)
        return(0);
    if(value > MaxRGB)
        return(MaxRGB);
    return((unsigned int) (value+0.5));
}

TQImage KImageEffect::addNoise(TQImage &src, NoiseType noise_type)
{
    int x, y;
    TQImage dest(src.width(), src.height(), 32);
    unsigned int *destData;

    if(src.depth() > 8){ // DirectClass source image
        unsigned int *srcData;
        for(y=0; y < src.height(); ++y){
            srcData = (unsigned int *)src.scanLine(y);
            destData = (unsigned int *)dest.scanLine(y);
            for(x=0; x < src.width(); ++x){
                destData[x] = tqRgba(generateNoise(tqRed(srcData[x]), noise_type),
                                    generateNoise(tqGreen(srcData[x]), noise_type),
                                    generateNoise(tqBlue(srcData[x]), noise_type),
                                    tqAlpha(srcData[x]));
            }
        }
    }
    else{ // PsudeoClass source image
        unsigned char *srcData;
        unsigned int *cTable = src.colorTable();
        unsigned int pixel;
        for(y=0; y < src.height(); ++y){
            srcData = (unsigned char *)src.scanLine(y);
            destData = (unsigned int *)dest.scanLine(y);
            for(x=0; x < src.width(); ++x){
                pixel = *(cTable+srcData[x]);
                destData[x] = tqRgba(generateNoise(tqRed(pixel), noise_type),
                                    generateNoise(tqGreen(pixel), noise_type),
                                    generateNoise(tqBlue(pixel), noise_type),
                                    tqAlpha(pixel));
            }
        }

    }
    return(dest);
}

unsigned int KImageEffect::interpolateColor(TQImage *image, double x_offset,
                                            double y_offset,
                                            unsigned int background)
{
    double alpha, beta;
    unsigned int p, q, r, s;
    int x, y;

    x = (int)x_offset;
    y = (int)y_offset;
    if((x < -1) || (x >= image->width()) || (y < -1) || (y >= image->height()))
        return(background);
    if(image->depth() > 8){
        if((x >= 0) && (y >= 0) && (x < (image->width()-1)) && (y < (image->height()-1)))    {
            unsigned int *t = (unsigned int *)image->scanLine(y);
            p = t[x];
            q = t[x+1];
            r = t[x+image->width()];
            s = t[x+image->width()+1];
        }
        else{
            unsigned int *t = (unsigned int *)image->scanLine(y);
            p = background;
            if((x >= 0) && (y >= 0)){
                p = t[x];
            }
            q = background;
            if(((x+1) < image->width()) && (y >= 0)){
                q = t[x+1];
            }
            r = background;
            if((x >= 0) && ((y+1) < image->height())){
                t = (unsigned int *)image->scanLine(y+1);
                r = t[x+image->width()];
            }
            s = background;
            if(((x+1) < image->width()) && ((y+1) < image->height())){
                t = (unsigned int *)image->scanLine(y+1);
                s = t[x+image->width()+1];
            }

        }
    }
    else{
        unsigned int *colorTable = (unsigned int *)image->colorTable();
        if((x >= 0) && (y >= 0) && (x < (image->width()-1)) && (y < (image->height()-1)))    {
            unsigned char *t;
            t = (unsigned char *)image->scanLine(y);
            p = *(colorTable+t[x]);
            q = *(colorTable+t[x+1]);
            t = (unsigned char *)image->scanLine(y+1);
            r = *(colorTable+t[x]);
            s = *(colorTable+t[x+1]);
        }
        else{
            unsigned char *t;
            p = background;
            if((x >= 0) && (y >= 0)){
                t = (unsigned char *)image->scanLine(y);
                p = *(colorTable+t[x]);
            }
            q = background;
            if(((x+1) < image->width()) && (y >= 0)){
                t = (unsigned char *)image->scanLine(y);
                q = *(colorTable+t[x+1]);
            }
            r = background;
            if((x >= 0) && ((y+1) < image->height())){
                t = (unsigned char *)image->scanLine(y+1);
                r = *(colorTable+t[x]);
            }
            s = background;
            if(((x+1) < image->width()) && ((y+1) < image->height())){
                t = (unsigned char *)image->scanLine(y+1);
                s = *(colorTable+t[x+1]);
            }

        }

    }
    x_offset -= floor(x_offset);
    y_offset -= floor(y_offset);
    alpha = 1.0-x_offset;
    beta = 1.0-y_offset;

    return(tqRgba((unsigned char)(beta*(alpha*tqRed(p)+x_offset*tqRed(q))+y_offset*(alpha*tqRed(r)+x_offset*tqRed(s))),
                 (unsigned char)(beta*(alpha*tqGreen(p)+x_offset*tqGreen(q))+y_offset*(alpha*tqGreen(r)+x_offset*tqGreen(s))),
                 (unsigned char)(beta*(alpha*tqBlue(p)+x_offset*tqBlue(q))+y_offset*(alpha*tqBlue(r)+x_offset*tqBlue(s))),
                 (unsigned char)(beta*(alpha*tqAlpha(p)+x_offset*tqAlpha(q))+y_offset*(alpha*tqAlpha(r)+x_offset*tqAlpha(s)))));
}

TQImage KImageEffect::implode(TQImage &src, double factor,
                             unsigned int background)
{
    double amount, distance, radius;
    double x_center, x_distance, x_scale;
    double y_center, y_distance, y_scale;
    unsigned int *destData;
    int x, y;

    TQImage dest(src.width(), src.height(), 32);

    // compute scaling factor
    x_scale = 1.0;
    y_scale = 1.0;
    x_center = (double)0.5*src.width();
    y_center = (double)0.5*src.height();
    radius=x_center;
    if(src.width() > src.height())
        y_scale = (double)src.width()/src.height();
    else if(src.width() < src.height()){
        x_scale = (double) src.height()/src.width();
        radius = y_center;
    }
    amount=factor/10.0;
    if(amount >= 0)
        amount/=10.0;
    if(src.depth() > 8){ // DirectClass source image
        unsigned int *srcData;
        for(y=0; y < src.height(); ++y){
            srcData = (unsigned int *)src.scanLine(y);
            destData = (unsigned int *)dest.scanLine(y);
            y_distance=y_scale*(y-y_center);
            for(x=0; x < src.width(); ++x){
                destData[x] = srcData[x];
                x_distance = x_scale*(x-x_center);
                distance= x_distance*x_distance+y_distance*y_distance;
                if(distance < (radius*radius)){
                    double factor;
                    // Implode the pixel.
                    factor=1.0;
                    if(distance > 0.0)
                        factor=
                            pow(sin(0.5000000000000001*M_PI*sqrt(distance)/radius),-amount);
                    destData[x] = interpolateColor(&src, factor*x_distance/x_scale+x_center,
                                                   factor*y_distance/y_scale+y_center,
                                                   background);
                }
            }
        }
    }
    else{ // PsudeoClass source image
        unsigned char *srcData;
        unsigned char idx;
        unsigned int *cTable = src.colorTable();
        for(y=0; y < src.height(); ++y){
            srcData = (unsigned char *)src.scanLine(y);
            destData = (unsigned int *)dest.scanLine(y);
            y_distance=y_scale*(y-y_center);
            for(x=0; x < src.width(); ++x){
                idx = srcData[x];
                destData[x] = cTable[idx];
                x_distance = x_scale*(x-x_center);
                distance= x_distance*x_distance+y_distance*y_distance;
                if(distance < (radius*radius)){
                    double factor;
                    // Implode the pixel.
                    factor=1.0;
                    if(distance > 0.0)
                        factor=
                            pow(sin(0.5000000000000001*M_PI*sqrt(distance)/radius),-amount);
                    destData[x] = interpolateColor(&src, factor*x_distance/x_scale+x_center,
                                                   factor*y_distance/y_scale+y_center,
                                                   background);
                }
            }
        }

    }
    return(dest);
}

TQImage KImageEffect::rotate(TQImage &img, RotateDirection r)
{
    TQImage dest;
    int x, y;
    if(img.depth() > 8){
        unsigned int *srcData, *destData;
        switch(r){
        case Rotate90:
            dest.create(img.height(), img.width(), img.depth());
            for(y=0; y < img.height(); ++y){
                srcData = (unsigned int *)img.scanLine(y);
                for(x=0; x < img.width(); ++x){
                    destData = (unsigned int *)dest.scanLine(x);
                    destData[img.height()-y-1] = srcData[x];
                }
            }
            break;
        case Rotate180:
            dest.create(img.width(), img.height(), img.depth());
            for(y=0; y < img.height(); ++y){
                srcData = (unsigned int *)img.scanLine(y);
                destData = (unsigned int *)dest.scanLine(img.height()-y-1);
                for(x=0; x < img.width(); ++x)
                    destData[img.width()-x-1] = srcData[x];
            }
            break;
        case Rotate270:
            dest.create(img.height(), img.width(), img.depth());
            for(y=0; y < img.height(); ++y){
                srcData = (unsigned int *)img.scanLine(y);
                for(x=0; x < img.width(); ++x){
                    destData = (unsigned int *)dest.scanLine(img.width()-x-1);
                    destData[y] = srcData[x];
                }
            }
            break;
        default:
            dest = img;
            break;
        }
    }
    else{
        unsigned char *srcData, *destData;
        unsigned int *srcTable, *destTable;
        switch(r){
        case Rotate90:
            dest.create(img.height(), img.width(), img.depth());
            dest.setNumColors(img.numColors());
            srcTable = (unsigned int *)img.colorTable();
            destTable = (unsigned int *)dest.colorTable();
            for(x=0; x < img.numColors(); ++x)
                destTable[x] = srcTable[x];
            for(y=0; y < img.height(); ++y){
                srcData = (unsigned char *)img.scanLine(y);
                for(x=0; x < img.width(); ++x){
                    destData = (unsigned char *)dest.scanLine(x);
                    destData[img.height()-y-1] = srcData[x];
                }
            }
            break;
        case Rotate180:
            dest.create(img.width(), img.height(), img.depth());
            dest.setNumColors(img.numColors());
            srcTable = (unsigned int *)img.colorTable();
            destTable = (unsigned int *)dest.colorTable();
            for(x=0; x < img.numColors(); ++x)
                destTable[x] = srcTable[x];
            for(y=0; y < img.height(); ++y){
                srcData = (unsigned char *)img.scanLine(y);
                destData = (unsigned char *)dest.scanLine(img.height()-y-1);
                for(x=0; x < img.width(); ++x)
                    destData[img.width()-x-1] = srcData[x];
            }
            break;
        case Rotate270:
            dest.create(img.height(), img.width(), img.depth());
            dest.setNumColors(img.numColors());
            srcTable = (unsigned int *)img.colorTable();
            destTable = (unsigned int *)dest.colorTable();
            for(x=0; x < img.numColors(); ++x)
                destTable[x] = srcTable[x];
            for(y=0; y < img.height(); ++y){
                srcData = (unsigned char *)img.scanLine(y);
                for(x=0; x < img.width(); ++x){
                    destData = (unsigned char *)dest.scanLine(img.width()-x-1);
                    destData[y] = srcData[x];
                }
            }
            break;
        default:
            dest = img;
            break;
        }

    }
    return(dest);
}

void KImageEffect::solarize(TQImage &img, double factor)
{
    int i, count;
    int threshold;
    unsigned int *data;

    threshold = (int)(factor*(MaxRGB+1)/100.0);
    if(img.depth() < 32){
        data = (unsigned int *)img.colorTable();
        count = img.numColors();
    }
    else{
        data = (unsigned int *)img.bits();
        count = img.width()*img.height();
    }
    for(i=0; i < count; ++i){
        data[i] = tqRgba(tqRed(data[i]) > threshold ? MaxRGB-tqRed(data[i]) : tqRed(data[i]),
                        tqGreen(data[i]) > threshold ? MaxRGB-tqGreen(data[i]) : tqGreen(data[i]),
                        tqBlue(data[i]) > threshold ? MaxRGB-tqBlue(data[i]) : tqBlue(data[i]),
                        tqAlpha(data[i]));
    }
}

TQImage KImageEffect::spread(TQImage &src, unsigned int amount)
{
    int quantum, x, y;
    int x_distance, y_distance;
    if(src.width() < 3 || src.height() < 3)
        return(src);
    TQImage dest(src);
    dest.detach();
    quantum=(amount+1) >> 1;
    if(src.depth() > 8){ // DirectClass source image
        unsigned int *p, *q;
        for(y=0; y < src.height(); y++){
            q = (unsigned int *)dest.scanLine(y);
            for(x=0; x < src.width(); x++){
                x_distance = x + ((rand() & (amount+1))-quantum);
                y_distance = y + ((rand() & (amount+1))-quantum);
                x_distance = TQMIN(x_distance, src.width()-1);
                y_distance = TQMIN(y_distance, src.height()-1);
                if(x_distance < 0)
                    x_distance = 0;
                if(y_distance < 0)
                    y_distance = 0;
                p = (unsigned int *)src.scanLine(y_distance);
                p += x_distance;
                *q++=(*p);
            }
        }
    }
    else{ // PsudeoClass source image
        // just do colortable values
        unsigned char *p, *q;
        for(y=0; y < src.height(); y++){
            q = (unsigned char *)dest.scanLine(y);
            for(x=0; x < src.width(); x++){
                x_distance = x + ((rand() & (amount+1))-quantum);
                y_distance = y + ((rand() & (amount+1))-quantum);
                x_distance = TQMIN(x_distance, src.width()-1);
                y_distance = TQMIN(y_distance, src.height()-1);
                if(x_distance < 0)
                    x_distance = 0;
                if(y_distance < 0)
                    y_distance = 0;
                p = (unsigned char *)src.scanLine(y_distance);
                p += x_distance;
                *q++=(*p);
            }
        }
    }
    return(dest);
}

TQImage KImageEffect::swirl(TQImage &src, double degrees,
                           unsigned int background)
{
    double cosine, distance, factor, radius, sine, x_center, x_distance,
        x_scale, y_center, y_distance, y_scale;
    int x, y;
    unsigned int *q;
    TQImage dest(src.width(), src.height(), 32);

    // compute scaling factor
    x_center = src.width()/2.0;
    y_center = src.height()/2.0;
    radius = TQMAX(x_center,y_center);
    x_scale=1.0;
    y_scale=1.0;
    if(src.width() > src.height())
        y_scale=(double)src.width()/src.height();
    else if(src.width() < src.height())
        x_scale=(double)src.height()/src.width();
    degrees=DegreesToRadians(degrees);
    // swirl each row
    if(src.depth() > 8){ // DirectClass source image
        unsigned int *p;
        for(y=0; y < src.height(); y++){
            p = (unsigned int *)src.scanLine(y);
            q = (unsigned int *)dest.scanLine(y);
            y_distance = y_scale*(y-y_center);
            for(x=0; x < src.width(); x++){
                // determine if the pixel is within an ellipse
                *q=(*p);
                x_distance = x_scale*(x-x_center);
                distance = x_distance*x_distance+y_distance*y_distance;
                if (distance < (radius*radius)){
                    // swirl
                    factor = 1.0-sqrt(distance)/radius;
                    sine = sin(degrees*factor*factor);
                    cosine = cos(degrees*factor*factor);
                    *q = interpolateColor(&src,
                                          (cosine*x_distance-sine*y_distance)/x_scale+x_center,
                                          (sine*x_distance+cosine*y_distance)/y_scale+y_center,
                                          background);
                }
                p++;
                q++;
            }
        }
    }
    else{ // PsudeoClass source image
        unsigned char *p;
        unsigned int *cTable = (unsigned int *)src.colorTable();
        for(y=0; y < src.height(); y++){
            p = (unsigned char *)src.scanLine(y);
            q = (unsigned int *)dest.scanLine(y);
            y_distance = y_scale*(y-y_center);
            for(x=0; x < src.width(); x++){
                // determine if the pixel is within an ellipse
                *q = *(cTable+(*p));
                x_distance = x_scale*(x-x_center);
                distance = x_distance*x_distance+y_distance*y_distance;
                if (distance < (radius*radius)){
                    // swirl
                    factor = 1.0-sqrt(distance)/radius;
                    sine = sin(degrees*factor*factor);
                    cosine = cos(degrees*factor*factor);
                    *q = interpolateColor(&src,
                                          (cosine*x_distance-sine*y_distance)/x_scale+x_center,
                                          (sine*x_distance+cosine*y_distance)/y_scale+y_center,
                                          background);
                }
                p++;
                q++;
            }
        }

    }
    return(dest);
}

TQImage KImageEffect::wave(TQImage &src, double amplitude, double wavelength,
                          unsigned int background)
{
    double *sine_map;
    int x, y;
    unsigned int *q;

    TQImage dest(src.width(), src.height() + (int)(2*fabs(amplitude)), 32);
    // allocate sine map
    sine_map = (double *)malloc(dest.width()*sizeof(double));
    if(!sine_map)
        return(src);
    for(x=0; x < dest.width(); ++x)
        sine_map[x]=fabs(amplitude)+amplitude*sin((2*M_PI*x)/wavelength);
    // wave image
    for(y=0; y < dest.height(); ++y){
        q = (unsigned int *)dest.scanLine(y);
        for (x=0; x < dest.width(); x++){
            *q=interpolateColor(&src, x, (int)(y-sine_map[x]), background);
            ++q;
        }
    }
    free(sine_map);
    return(dest);
}

//
// The following methods work by computing a value from neighboring pixels
// (mosfet 05/26/03)
//

// New algorithms based on ImageMagick 5.5.6 (05/26/03)

TQImage KImageEffect::oilPaint(TQImage &src, int /*radius*/)
{
    /* binary compat method - remove me when possible! */
    return(oilPaintConvolve(src, 0));
}

TQImage KImageEffect::oilPaintConvolve(TQImage &src, double radius)
{
    unsigned long count /*,*histogram*/;
    unsigned long histogram[256];
    unsigned int k;
    int width;
    int x, y, mx, my, sx, sy;
    int mcx, mcy;
    unsigned int *s=0, *q;

    if(src.depth() < 32)
        src.convertDepth(32);
    TQImage dest(src);
    dest.detach();

    width = getOptimalKernelWidth(radius, 0.5);
    if(src.width() < width){
        tqWarning("KImageEffect::oilPaintConvolve(): Image is smaller than radius!");
        return(dest);
    }
    /*
    histogram = (unsigned long *)malloc(256*sizeof(unsigned long));
    if(!histogram){
        tqWarning("KImageEffect::oilPaintColvolve(): Unable to allocate memory!");
        return(dest);
    }
    */
    unsigned int **jumpTable = (unsigned int **)src.jumpTable();
    for(y=0; y < dest.height(); ++y){
        sy = y-(width/2);
        q = (unsigned int *)dest.scanLine(y);
        for(x=0; x < dest.width(); ++x){
            count = 0;
            memset(histogram, 0, 256*sizeof(unsigned long));
            //memset(histogram, 0, 256);
            sy = y-(width/2);
            for(mcy=0; mcy < width; ++mcy, ++sy){
                my = sy < 0 ? 0 : sy > src.height()-1 ?
                    src.height()-1 : sy;
                sx = x+(-width/2);
                for(mcx=0; mcx < width; ++mcx, ++sx){
                    mx = sx < 0 ? 0 : sx > src.width()-1 ?
                        src.width()-1 : sx;

                    k = intensityValue(jumpTable[my][mx]);
                    if(k > 255){
                        tqWarning("KImageEffect::oilPaintConvolve(): k is %d",
                                 k);
                        k = 255;
                    }
                    histogram[k]++;
                    if(histogram[k] > count){
                        count = histogram[k];
                        s = jumpTable[my]+mx;
                    }
                }
            }
            if (s)
                *q++ = (*s);
        }
    }
    /* liberateMemory((histogram); */
    return(dest);
}

TQImage KImageEffect::charcoal(TQImage &src, double /*factor*/)
{
    /* binary compat method - remove me when possible! */
    return(charcoal(src, 0, 1));
}

TQImage KImageEffect::charcoal(TQImage &src, double radius, double sigma)
{
    TQImage img(edge(src, radius));
    img = blur(img, radius, sigma);
    normalize(img);
    img.invertPixels(false);
    KImageEffect::toGray(img);
    return(img);
}

void KImageEffect::normalize(TQImage &image)
{
    struct double_packet high, low, intensity, *histogram;
    struct short_packet *normalize_map;
    TQ_INT64 number_pixels;
    int x, y;
    unsigned int *p, *q;
    long i;
    unsigned long threshold_intensity;
    unsigned char r, g, b, a;

    if(image.depth() < 32) // result will always be 32bpp
        image = image.convertDepth(32);

    histogram = (struct double_packet *)
        malloc(256*sizeof(struct double_packet));
    normalize_map = (struct short_packet *)
        malloc(256*sizeof(struct short_packet));

    if(!histogram || !normalize_map){
        if(histogram)
            liberateMemory(&histogram);
        if(normalize_map)
            liberateMemory(&normalize_map);
        tqWarning("KImageEffect::normalize(): Unable to allocate memory!");
        return;
    }

    /*
    Form histogram.
    */
    memset(histogram, 0, 256*sizeof(struct double_packet));
    for(y=0; y < image.height(); ++y){
        p = (unsigned int *)image.scanLine(y);
        for(x=0; x < image.width(); ++x){
            histogram[(unsigned char)(tqRed(*p))].red++;
            histogram[(unsigned char)(tqGreen(*p))].green++;
            histogram[(unsigned char)(tqBlue(*p))].blue++;
            histogram[(unsigned char)(tqAlpha(*p))].alpha++;
            p++;
        }
    }

    /*
    Find the histogram boundaries by locating the 0.1 percent levels.
    */
    number_pixels = (TQ_INT64)image.width()*image.height();
    threshold_intensity = number_pixels/1000;

    /* red */
    memset(&intensity, 0, sizeof(struct double_packet));
    memset(&high, 0, sizeof(struct double_packet));
    memset(&low, 0, sizeof(struct double_packet));
    for(high.red=255; high.red != 0; high.red--){
        intensity.red+=histogram[(unsigned char)high.red].red;
        if(intensity.red > threshold_intensity)
            break;
    }
    if(low.red == high.red){
        threshold_intensity = 0;
        memset(&intensity, 0, sizeof(struct double_packet));
        for(low.red=0; low.red < 255; low.red++){
            intensity.red+=histogram[(unsigned char)low.red].red;
            if(intensity.red > threshold_intensity)
                break;
        }
        memset(&intensity, 0, sizeof(struct double_packet));
        for(high.red=255; high.red != 0; high.red--){
            intensity.red+=histogram[(unsigned char)high.red].red;
            if(intensity.red > threshold_intensity)
                break;
        }
    }

    /* green */
    memset(&intensity, 0, sizeof(struct double_packet));
    for(high.green=255; high.green != 0; high.green--){
        intensity.green+=histogram[(unsigned char)high.green].green;
        if(intensity.green > threshold_intensity)
            break;
    }
    if(low.green == high.green){
        threshold_intensity = 0;
        memset(&intensity, 0, sizeof(struct double_packet));
        for(low.green=0; low.green < 255; low.green++){
            intensity.green+=histogram[(unsigned char)low.green].green;
            if(intensity.green > threshold_intensity)
                break;
        }
        memset(&intensity,0,sizeof(struct double_packet));
        for(high.green=255; high.green != 0; high.green--){
            intensity.green+=histogram[(unsigned char)high.green].green;
            if(intensity.green > threshold_intensity)
                break;
        }
    }

    /* blue */
    memset(&intensity, 0, sizeof(struct double_packet));
    for(high.blue=255; high.blue != 0; high.blue--){
        intensity.blue+=histogram[(unsigned char)high.blue].blue;
        if(intensity.blue > threshold_intensity)
            break;
    }
    if(low.blue == high.blue){
        threshold_intensity = 0;
        memset(&intensity, 0, sizeof(struct double_packet));
        for(low.blue=0; low.blue < 255; low.blue++){
            intensity.blue+=histogram[(unsigned char)low.blue].blue;
            if(intensity.blue > threshold_intensity)
                break;
        }
        memset(&intensity,0,sizeof(struct double_packet));
        for(high.blue=255; high.blue != 0; high.blue--){
            intensity.blue+=histogram[(unsigned char)high.blue].blue;
            if(intensity.blue > threshold_intensity)
                break;
        }
    }

    /* alpha */
    memset(&intensity, 0, sizeof(struct double_packet));
    for(high.alpha=255; high.alpha != 0; high.alpha--){
        intensity.alpha+=histogram[(unsigned char)high.alpha].alpha;
        if(intensity.alpha > threshold_intensity)
            break;
    }
    if(low.alpha == high.alpha){
        threshold_intensity = 0;
        memset(&intensity, 0, sizeof(struct double_packet));
        for(low.alpha=0; low.alpha < 255; low.alpha++){
            intensity.alpha+=histogram[(unsigned char)low.alpha].alpha;
            if(intensity.alpha > threshold_intensity)
                break;
        }
        memset(&intensity,0,sizeof(struct double_packet));
        for(high.alpha=255; high.alpha != 0; high.alpha--){
            intensity.alpha+=histogram[(unsigned char)high.alpha].alpha;
            if(intensity.alpha > threshold_intensity)
                break;
        }
    }
    liberateMemory(&histogram);

    /*
     Stretch the histogram to create the normalized image mapping.
     */

    // should the maxes be 65535?
    memset(normalize_map, 0 ,256*sizeof(struct short_packet));
    for(i=0; i <= (long) 255; i++){
        if(i < (long) low.red)
            normalize_map[i].red=0;
        else if (i > (long) high.red)
            normalize_map[i].red=65535;
        else if (low.red != high.red)
            normalize_map[i].red =
                (unsigned short)((65535*(i-low.red))/(high.red-low.red));

        if(i < (long) low.green)
            normalize_map[i].green=0;
        else if (i > (long) high.green)
            normalize_map[i].green=65535;
        else if (low.green != high.green)
            normalize_map[i].green =
                (unsigned short)((65535*(i-low.green))/(high.green-low.green));

        if(i < (long) low.blue)
            normalize_map[i].blue=0;
        else if (i > (long) high.blue)
            normalize_map[i].blue=65535;
        else if (low.blue != high.blue)
            normalize_map[i].blue =
                (unsigned short)((65535*(i-low.blue))/(high.blue-low.blue));

        if(i < (long) low.alpha)
            normalize_map[i].alpha=0;
        else if (i > (long) high.alpha)
            normalize_map[i].alpha=65535;
        else if (low.alpha != high.alpha)
            normalize_map[i].alpha =
                (unsigned short)((65535*(i-low.alpha))/(high.alpha-low.alpha));

    }

    for(y=0; y < image.height(); ++y){
        q = (unsigned int *)image.scanLine(y);
        for(x=0; x < image.width(); ++x){
            if(low.red != high.red)
                r = (normalize_map[(unsigned short)(tqRed(q[x]))].red)/257;
            else
                r = tqRed(q[x]);
            if(low.green != high.green)
                g = (normalize_map[(unsigned short)(tqGreen(q[x]))].green)/257;
            else
                g = tqGreen(q[x]);
            if(low.blue != high.blue)
                b = (normalize_map[(unsigned short)(tqBlue(q[x]))].blue)/257;
            else
                b = tqBlue(q[x]);
            if(low.alpha != high.alpha)
                a = (normalize_map[(unsigned short)(tqAlpha(q[x]))].alpha)/257;
            else
                a = tqAlpha(q[x]);
            q[x] = tqRgba(r, g, b, a);
        }
    }
    liberateMemory(&normalize_map);
}

void KImageEffect::equalize(TQImage &image)
{
    struct double_packet high, low, intensity, *map, *histogram;
    struct short_packet *equalize_map;
    int x, y;
    unsigned int *p, *q;
    long i;
    unsigned char r, g, b, a;

    if(image.depth() < 32) // result will always be 32bpp
        image = image.convertDepth(32);

    histogram=(struct double_packet *) malloc(256*sizeof(struct double_packet));
    map=(struct double_packet *) malloc(256*sizeof(struct double_packet));
    equalize_map=(struct short_packet *)malloc(256*sizeof(struct short_packet));
    if(!histogram || !map || !equalize_map){
        if(histogram)
            liberateMemory(&histogram);
        if(map)
            liberateMemory(&map);
        if(equalize_map)
            liberateMemory(&equalize_map);
        tqWarning("KImageEffect::equalize(): Unable to allocate memory!");
        return;
    }

    /*
    Form histogram.
    */
    memset(histogram, 0, 256*sizeof(struct double_packet));
    for(y=0; y < image.height(); ++y){
        p = (unsigned int *)image.scanLine(y);
        for(x=0; x < image.width(); ++x){
            histogram[(unsigned char)(tqRed(*p))].red++;
            histogram[(unsigned char)(tqGreen(*p))].green++;
            histogram[(unsigned char)(tqBlue(*p))].blue++;
            histogram[(unsigned char)(tqAlpha(*p))].alpha++;
            p++;
        }
    }
    /*
     Integrate the histogram to get the equalization map.
     */
    memset(&intensity, 0 ,sizeof(struct double_packet));
    for(i=0; i <= 255; ++i){
        intensity.red += histogram[i].red;
        intensity.green += histogram[i].green;
        intensity.blue += histogram[i].blue;
        intensity.alpha += histogram[i].alpha;
        map[i]=intensity;
    }
    low=map[0];
    high=map[255];
    memset(equalize_map, 0, 256*sizeof(short_packet));
    for(i=0; i <= 255; ++i){
        if(high.red != low.red)
            equalize_map[i].red=(unsigned short)
                ((65535*(map[i].red-low.red))/(high.red-low.red));
        if(high.green != low.green)
            equalize_map[i].green=(unsigned short)
                ((65535*(map[i].green-low.green))/(high.green-low.green));
        if(high.blue != low.blue)
            equalize_map[i].blue=(unsigned short)
                ((65535*(map[i].blue-low.blue))/(high.blue-low.blue));
        if(high.alpha != low.alpha)
            equalize_map[i].alpha=(unsigned short)
                ((65535*(map[i].alpha-low.alpha))/(high.alpha-low.alpha));
    }
    liberateMemory(&histogram);
    liberateMemory(&map);

    /*
     Stretch the histogram.
     */
    for(y=0; y < image.height(); ++y){
        q = (unsigned int *)image.scanLine(y);
        for(x=0; x < image.width(); ++x){
            if(low.red != high.red)
                r = (equalize_map[(unsigned short)(tqRed(q[x]))].red/257);
            else
                r = tqRed(q[x]);
            if(low.green != high.green)
                g = (equalize_map[(unsigned short)(tqGreen(q[x]))].green/257);
            else
                g = tqGreen(q[x]);
            if(low.blue != high.blue)
                b = (equalize_map[(unsigned short)(tqBlue(q[x]))].blue/257);
            else
                b = tqBlue(q[x]);
            if(low.alpha != high.alpha)
                a = (equalize_map[(unsigned short)(tqAlpha(q[x]))].alpha/257);
            else
                a = tqAlpha(q[x]);
            q[x] = tqRgba(r, g, b, a);
        }
    }
    liberateMemory(&equalize_map);

}

TQImage KImageEffect::edge(TQImage &image, double radius)
{
    double *kernel;
    int width;
    long i;
    TQImage dest;

    if(radius == 50.0){
        /* For binary compatability! Remove me when possible! This used to
         * take a different parameter, a factor, and this was the default
         * value */
        radius = 0.0;
    }

    width = getOptimalKernelWidth(radius, 0.5);
    if(image.width() < width || image.height() < width){
        tqWarning("KImageEffect::edge(): Image is smaller than radius!");
        return(dest);
    }
    kernel= (double *)malloc(width*width*sizeof(double));
    if(!kernel){
        tqWarning("KImageEffect::edge(): Unable to allocate memory!");
        return(dest);
    }
    for(i=0; i < (width*width); i++)
        kernel[i]=(-1.0);
    kernel[i/2]=width*width-1.0;
    convolveImage(&image, &dest, width, kernel);
    free(kernel);
    return(dest);
}

TQImage KImageEffect::emboss(TQImage &src)
{
    /* binary compat method - remove me when possible! */
    return(emboss(src, 0, 1));
}

TQImage KImageEffect::emboss(TQImage &image, double radius, double sigma)
{
    double alpha, *kernel;
    int j, width;
    long i, u, v;
    TQImage dest;

    if(sigma == 0.0){
        tqWarning("KImageEffect::emboss(): Zero sigma is not permitted!");
        return(dest);
    }

    width = getOptimalKernelWidth(radius, sigma);
    if(image.width() < width || image.height() < width){
        tqWarning("KImageEffect::emboss(): Image is smaller than radius!");
        return(dest);
    }
    kernel= (double *)malloc(width*width*sizeof(double));
    if(!kernel){
        tqWarning("KImageEffect::emboss(): Unable to allocate memory!");
        return(dest);
    }
    if(image.depth() < 32)
        image = image.convertDepth(32);

    i=0;
    j=width/2;
    for(v=(-width/2); v <= (width/2); v++){
        for(u=(-width/2); u <= (width/2); u++){
            alpha=exp(-((double) u*u+v*v)/(2.0*sigma*sigma));
            kernel[i]=((u < 0) || (v < 0) ? -8.0 : 8.0)*alpha/
                (2.0*MagickPI*sigma*sigma);
            if (u == j)
                kernel[i]=0.0;
            i++;
        }
        j--;
    }
    convolveImage(&image, &dest, width, kernel);
    liberateMemory(&kernel);

    equalize(dest);
    return(dest);
}

void KImageEffect::blurScanLine(double *kernel, int width,
                                unsigned int *src, unsigned int *dest,
                                int columns)
{
    double *p;
    unsigned int *q;
    int x;
    long i;
    double red, green, blue, alpha;
    double scale = 0.0;

    if(width > columns){
        for(x=0; x < columns; ++x){
            scale = 0.0;
            red = blue = green = alpha = 0.0;
            p = kernel;
            q = src;
            for(i=0; i < columns; ++i){
                if((i >= (x-width/2)) && (i <= (x+width/2))){
                    red += (*p)*(tqRed(*q)*257);
                    green += (*p)*(tqGreen(*q)*257);
                    blue += (*p)*(tqBlue(*q)*257);
                    alpha += (*p)*(tqAlpha(*q)*257);
                }
                if(((i+width/2-x) >= 0) && ((i+width/2-x) < width))
                    scale+=kernel[i+width/2-x];
                p++;
                q++;
            }
            scale = 1.0/scale;
            red = scale*(red+0.5);
            green = scale*(green+0.5);
            blue = scale*(blue+0.5);
            alpha = scale*(alpha+0.5);

            red = red < 0 ? 0 : red > 65535 ? 65535 : red;
            green = green < 0 ? 0 : green > 65535 ? 65535 : green;
            blue = blue < 0 ? 0 : blue > 65535 ? 65535 : blue;
            alpha = alpha < 0 ? 0 : alpha > 65535 ? 65535 : alpha;

            dest[x] = tqRgba((unsigned char)(red/257UL),
                            (unsigned char)(green/257UL),
                            (unsigned char)(blue/257UL),
                            (unsigned char)(alpha/257UL));
        }
        return;
    }

    for(x=0; x < width/2; ++x){
        scale = 0.0;
        red = blue = green = alpha = 0.0;
        p = kernel+width/2-x;
        q = src;
        for(i=width/2-x; i < width; ++i){
            red += (*p)*(tqRed(*q)*257);
            green += (*p)*(tqGreen(*q)*257);
            blue += (*p)*(tqBlue(*q)*257);
            alpha += (*p)*(tqAlpha(*q)*257);
            scale += (*p);
            p++;
            q++;
        }
        scale=1.0/scale;

        red = scale*(red+0.5);
        green = scale*(green+0.5);
        blue = scale*(blue+0.5);
        alpha = scale*(alpha+0.5);

        red = red < 0 ? 0 : red > 65535 ? 65535 : red;
        green = green < 0 ? 0 : green > 65535 ? 65535 : green;
        blue = blue < 0 ? 0 : blue > 65535 ? 65535 : blue;
        alpha = alpha < 0 ? 0 : alpha > 65535 ? 65535 : alpha;

        dest[x] = tqRgba((unsigned char)(red/257UL),
                        (unsigned char)(green/257UL),
                        (unsigned char)(blue/257UL),
                        (unsigned char)(alpha/257UL));
    }

    for(; x < columns-width/2; ++x){
        red = blue = green = alpha = 0.0;
        p = kernel;
        q = src+(x-width/2);
        for (i=0; i < (long) width; ++i){
            red += (*p)*(tqRed(*q)*257);
            green += (*p)*(tqGreen(*q)*257);
            blue += (*p)*(tqBlue(*q)*257);
            alpha += (*p)*(tqAlpha(*q)*257);
            p++;
            q++;
        }
        red = scale*(red+0.5);
        green = scale*(green+0.5);
        blue = scale*(blue+0.5);
        alpha = scale*(alpha+0.5);

        red = red < 0 ? 0 : red > 65535 ? 65535 : red;
        green = green < 0 ? 0 : green > 65535 ? 65535 : green;
        blue = blue < 0 ? 0 : blue > 65535 ? 65535 : blue;
        alpha = alpha < 0 ? 0 : alpha > 65535 ? 65535 : alpha;

        dest[x] = tqRgba((unsigned char)(red/257UL),
                        (unsigned char)(green/257UL),
                        (unsigned char)(blue/257UL),
                        (unsigned char)(alpha/257UL));
    }

    for(; x < columns; ++x){
        red = blue = green = alpha = 0.0;
        scale=0;
        p = kernel;
        q = src+(x-width/2);
        for(i=0; i < columns-x+width/2; ++i){
            red += (*p)*(tqRed(*q)*257);
            green += (*p)*(tqGreen(*q)*257);
            blue += (*p)*(tqBlue(*q)*257);
            alpha += (*p)*(tqAlpha(*q)*257);
            scale += (*p);
            p++;
            q++;
        }
        scale=1.0/scale;
        red = scale*(red+0.5);
        green = scale*(green+0.5);
        blue = scale*(blue+0.5);
        alpha = scale*(alpha+0.5);

        red = red < 0 ? 0 : red > 65535 ? 65535 : red;
        green = green < 0 ? 0 : green > 65535 ? 65535 : green;
        blue = blue < 0 ? 0 : blue > 65535 ? 65535 : blue;
        alpha = alpha < 0 ? 0 : alpha > 65535 ? 65535 : alpha;

        dest[x] = tqRgba((unsigned char)(red/257UL),
                        (unsigned char)(green/257UL),
                        (unsigned char)(blue/257UL),
                        (unsigned char)(alpha/257UL));
    }
}

int KImageEffect::getBlurKernel(int width, double sigma, double **kernel)
{
#define KernelRank 3
    double alpha, normalize;
    long i;
    int bias;

    assert(sigma != 0.0);
    if(width == 0)
        width = 3;
    *kernel=(double *)malloc(width*sizeof(double));
    if(*kernel == (double *)NULL)
        return(0);
    memset(*kernel, 0, width*sizeof(double));
    bias = KernelRank*width/2;
    for(i=(-bias); i <= bias; i++){
        alpha=exp(-((double) i*i)/(2.0*KernelRank*KernelRank*sigma*sigma));
        (*kernel)[(i+bias)/KernelRank]+=alpha/(MagickSQ2PI*sigma);
    }
    normalize=0;
    for(i=0; i < width; i++)
        normalize+=(*kernel)[i];
    for(i=0; i < width; i++)
        (*kernel)[i]/=normalize;

    return(width);
}

TQImage KImageEffect::blur(TQImage &src, double /*factor*/)
{
    /* binary compat method - remove me when possible! */
    return(blur(src, 0, 1));
}

TQImage KImageEffect::blur(TQImage &src, double radius, double sigma)
{
    double *kernel;
    TQImage dest;
    int width;
    int x, y;
    unsigned int *scanline, *temp;
    unsigned int *p, *q;

    if(sigma == 0.0){
        tqWarning("KImageEffect::blur(): Zero sigma is not permitted!");
        return(dest);
    }
    if(src.depth() < 32)
        src = src.convertDepth(32);

    kernel=(double *) NULL;
    if(radius > 0)
        width=getBlurKernel((int) (2*ceil(radius)+1),sigma,&kernel);
    else{
        double *last_kernel;
        last_kernel=(double *) NULL;
        width=getBlurKernel(3,sigma,&kernel);

        while ((long) (MaxRGB*kernel[0]) > 0){
            if(last_kernel != (double *)NULL){
                liberateMemory(&last_kernel);
            }
            last_kernel=kernel;
            kernel = (double *)NULL;
            width = getBlurKernel(width+2, sigma, &kernel);
        }
        if(last_kernel != (double *) NULL){
            liberateMemory(&kernel);
            width-=2;
            kernel = last_kernel;
        }
    }

    if(width < 3){
        tqWarning("KImageEffect::blur(): Kernel radius is too small!");
        liberateMemory(&kernel);
        return(dest);
    }

    dest.create(src.width(), src.height(), 32);

    // Horizontal convolution
    scanline = (unsigned int *)malloc(sizeof(unsigned int)*src.height());
    temp = (unsigned int *)malloc(sizeof(unsigned int)*src.height());
    for(y=0; y < src.height(); ++y){
        p = (unsigned int *)src.scanLine(y);
        q = (unsigned int *)dest.scanLine(y);
        blurScanLine(kernel, width, p, q, src.width());
    }

    TQImage partial = dest;

    // Vertical convolution
    unsigned int **srcTable = (unsigned int **)partial.jumpTable();
    unsigned int **destTable = (unsigned int **)dest.jumpTable();
    for(x=0; x < partial.width(); ++x){
        for(y=0; y < partial.height(); ++y){
            scanline[y] = srcTable[y][x];
        }
        blurScanLine(kernel, width, scanline, temp, partial.height());
        for(y=0; y < partial.height(); ++y){
            destTable[y][x] = temp[y];
        }
    }
    free(scanline);
    free(temp);
    free(kernel);
    return(dest);
}

bool KImageEffect::convolveImage(TQImage *image, TQImage *dest,
                                 const unsigned int order,
                                 const double *kernel)
{
    long width;
    double red, green, blue, alpha;
    double normalize, *normal_kernel;
    const double *k;
    unsigned int *q;
    int x, y, mx, my, sx, sy;
    long i;
    int mcx, mcy;

    width = order;
    if((width % 2) == 0){
        tqWarning("KImageEffect: Kernel width must be an odd number!");
        return(false);
    }
    normal_kernel = (double *)malloc(width*width*sizeof(double));
    if(!normal_kernel){
        tqWarning("KImageEffect: Unable to allocate memory!");
        return(false);
    }
    dest->reset();
    dest->create(image->width(), image->height(), 32);
    if(image->depth() < 32)
        *image = image->convertDepth(32);

    normalize=0.0;
    for(i=0; i < (width*width); i++)
        normalize += kernel[i];
    if(fabs(normalize) <= MagickEpsilon)
        normalize=1.0;
    normalize=1.0/normalize;
    for(i=0; i < (width*width); i++)
        normal_kernel[i] = normalize*kernel[i];

    unsigned int **jumpTable = (unsigned int **)image->jumpTable();
    for(y=0; y < dest->height(); ++y){
        sy = y-(width/2);
        q = (unsigned int *)dest->scanLine(y);
        for(x=0; x < dest->width(); ++x){
            k = normal_kernel;
            red = green = blue = alpha = 0;
            sy = y-(width/2);
            for(mcy=0; mcy < width; ++mcy, ++sy){
                my = sy < 0 ? 0 : sy > image->height()-1 ?
                    image->height()-1 : sy;
                sx = x+(-width/2);
                for(mcx=0; mcx < width; ++mcx, ++sx){
                    mx = sx < 0 ? 0 : sx > image->width()-1 ?
                        image->width()-1 : sx;
                    red += (*k)*(tqRed(jumpTable[my][mx])*257);
                    green += (*k)*(tqGreen(jumpTable[my][mx])*257);
                    blue += (*k)*(tqBlue(jumpTable[my][mx])*257);
                    alpha += (*k)*(tqAlpha(jumpTable[my][mx])*257);
                    ++k;
                }
            }

            red = red < 0 ? 0 : red > 65535 ? 65535 : red+0.5;
            green = green < 0 ? 0 : green > 65535 ? 65535 : green+0.5;
            blue = blue < 0 ? 0 : blue > 65535 ? 65535 : blue+0.5;
            alpha = alpha < 0 ? 0 : alpha > 65535 ? 65535 : alpha+0.5;

            *q++ = tqRgba((unsigned char)(red/257UL),
                         (unsigned char)(green/257UL),
                         (unsigned char)(blue/257UL),
                         (unsigned char)(alpha/257UL));
        }
    }
    free(normal_kernel);
    return(true);

}

int KImageEffect::getOptimalKernelWidth(double radius, double sigma)
{
    double normalize, value;
    long width;
    long u;

    assert(sigma != 0.0);
    if(radius > 0.0)
        return((int)(2.0*ceil(radius)+1.0));
    for(width=5; ;){
        normalize=0.0;
        for(u=(-width/2); u <= (width/2); u++)
            normalize+=exp(-((double) u*u)/(2.0*sigma*sigma))/(MagickSQ2PI*sigma);
        u=width/2;
        value=exp(-((double) u*u)/(2.0*sigma*sigma))/(MagickSQ2PI*sigma)/normalize;
        if((long)(65535*value) <= 0)
            break;
        width+=2;
    }
    return((int)width-2);
}

TQImage KImageEffect::sharpen(TQImage &src, double /*factor*/)
{
    /* binary compat method - remove me when possible! */
    return(sharpen(src, 0, 1));
}

TQImage KImageEffect::sharpen(TQImage &image, double radius, double sigma)
{
    double alpha, normalize, *kernel;
    int width;
    long i, u, v;
    TQImage dest;

    if(sigma == 0.0){
        tqWarning("KImageEffect::sharpen(): Zero sigma is not permitted!");
        return(dest);
    }
    width = getOptimalKernelWidth(radius, sigma);
    if(image.width() < width){
        tqWarning("KImageEffect::sharpen(): Image is smaller than radius!");
        return(dest);
    }
    kernel = (double *)malloc(width*width*sizeof(double));
    if(!kernel){
        tqWarning("KImageEffect::sharpen(): Unable to allocate memory!");
        return(dest);
    }

    i = 0;
    normalize=0.0;
    for(v=(-width/2); v <= (width/2); v++){
        for(u=(-width/2); u <= (width/2); u++){
            alpha=exp(-((double) u*u+v*v)/(2.0*sigma*sigma));
            kernel[i]=alpha/(2.0*MagickPI*sigma*sigma);
            normalize+=kernel[i];
            i++;
        }
    }
    kernel[i/2]=(-2.0)*normalize;
    convolveImage(&image, &dest, width, kernel);
    free(kernel);
    return(dest);
}

// End of new algorithms

TQImage KImageEffect::shade(TQImage &src, bool color_shading, double azimuth,
             double elevation)
{
    struct PointInfo{
        double x, y, z;
    };

    double distance, normal_distance, shade;
    int x, y;

    struct PointInfo light, normal;

    unsigned int *q;

    TQImage dest(src.width(), src.height(), 32);

    azimuth = DegreesToRadians(azimuth);
    elevation = DegreesToRadians(elevation);
    light.x = MaxRGB*cos(azimuth)*cos(elevation);
    light.y = MaxRGB*sin(azimuth)*cos(elevation);
    light.z = MaxRGB*sin(elevation);
    normal.z= 2*MaxRGB;  // constant Z of surface normal

    if(src.depth() > 8){ // DirectClass source image
        unsigned int *p, *s0, *s1, *s2;
        for(y=0; y < src.height(); ++y){
            p = (unsigned int *)src.scanLine(TQMIN(TQMAX(y-1,0),src.height()-3));
            q = (unsigned int *)dest.scanLine(y);
            // shade this row of pixels.
            *q++=(*(p+src.width()));
            p++;
            s0 = p;
            s1 = p + src.width();
            s2 = p + 2*src.width();
            for(x=1; x < src.width()-1; ++x){
                // determine the surface normal and compute shading.
                normal.x=intensityValue(*(s0-1))+intensityValue(*(s1-1))+intensityValue(*(s2-1))-
                    (double) intensityValue(*(s0+1))-(double) intensityValue(*(s1+1))-
                    (double) intensityValue(*(s2+1));
                normal.y=intensityValue(*(s2-1))+intensityValue(*s2)+intensityValue(*(s2+1))-
                    (double) intensityValue(*(s0-1))-(double) intensityValue(*s0)-
                    (double) intensityValue(*(s0+1));
                if((normal.x == 0) && (normal.y == 0))
                    shade=light.z;
                else{
                    shade=0.0;
                    distance=normal.x*light.x+normal.y*light.y+normal.z*light.z;
                    if (distance > 0.0){
                        normal_distance=
                            normal.x*normal.x+normal.y*normal.y+normal.z*normal.z;
                        if(fabs(normal_distance) > 0.0000001)
                            shade=distance/sqrt(normal_distance);
                    }
                }
                if(!color_shading){
                    *q = tqRgba((unsigned char)(shade),
                               (unsigned char)(shade),
                               (unsigned char)(shade),
                               tqAlpha(*s1));
                }
                else{
                    *q = tqRgba((unsigned char)((shade*tqRed(*s1))/(MaxRGB+1)),
                               (unsigned char)((shade*tqGreen(*s1))/(MaxRGB+1)),
                               (unsigned char)((shade*tqBlue(*s1))/(MaxRGB+1)),
                               tqAlpha(*s1));
                }
                ++s0;
                ++s1;
                ++s2;
                q++;
            }
            *q++=(*s1);
        }
    }
    else{ // PsudeoClass source image
        unsigned char *p, *s0, *s1, *s2;
        int scanLineIdx;
        unsigned int *cTable = (unsigned int *)src.colorTable();
        for(y=0; y < src.height(); ++y){
            scanLineIdx = TQMIN(TQMAX(y-1,0),src.height()-3);
            p = (unsigned char *)src.scanLine(scanLineIdx);
            q = (unsigned int *)dest.scanLine(y);
            // shade this row of pixels.
            s0 = p;
            s1 = (unsigned char *) src.scanLine(scanLineIdx+1);
            s2 = (unsigned char *) src.scanLine(scanLineIdx+2);
            *q++=(*(cTable+(*s1)));
            ++p;
            ++s0;
            ++s1;
            ++s2;
            for(x=1; x < src.width()-1; ++x){
                // determine the surface normal and compute shading.
                normal.x=intensityValue(*(cTable+(*(s0-1))))+intensityValue(*(cTable+(*(s1-1))))+intensityValue(*(cTable+(*(s2-1))))-
                    (double) intensityValue(*(cTable+(*(s0+1))))-(double) intensityValue(*(cTable+(*(s1+1))))-
                    (double) intensityValue(*(cTable+(*(s2+1))));
                normal.y=intensityValue(*(cTable+(*(s2-1))))+intensityValue(*(cTable+(*s2)))+intensityValue(*(cTable+(*(s2+1))))-
                    (double) intensityValue(*(cTable+(*(s0-1))))-(double) intensityValue(*(cTable+(*s0)))-
                    (double) intensityValue(*(cTable+(*(s0+1))));
                if((normal.x == 0) && (normal.y == 0))
                    shade=light.z;
                else{
                    shade=0.0;
                    distance=normal.x*light.x+normal.y*light.y+normal.z*light.z;
                    if (distance > 0.0){
                        normal_distance=
                            normal.x*normal.x+normal.y*normal.y+normal.z*normal.z;
                        if(fabs(normal_distance) > 0.0000001)
                            shade=distance/sqrt(normal_distance);
                    }
                }
                if(!color_shading){
                    *q = tqRgba((unsigned char)(shade),
                               (unsigned char)(shade),
                               (unsigned char)(shade),
                               tqAlpha(*(cTable+(*s1))));
                }
                else{
                    *q = tqRgba((unsigned char)((shade*tqRed(*(cTable+(*s1))))/(MaxRGB+1)),
                               (unsigned char)((shade*tqGreen(*(cTable+(*s1))))/(MaxRGB+1)),
                               (unsigned char)((shade*tqBlue(*(cTable+(*s1))))/(MaxRGB+1)),
                               tqAlpha(*s1));
                }
                ++s0;
                ++s1;
                ++s2;
                q++;
            }
            *q++=(*(cTable+(*s1)));
        }
    }
    return(dest);
}

// High quality, expensive HSV contrast. You can do a faster one by just
// taking a grayscale threshold (ie: 128) and incrementing RGB color
// channels above it and decrementing those below it, but this gives much
// better results. (mosfet 12/28/01)
void KImageEffect::contrastHSV(TQImage &img, bool sharpen)
{
    int i, sign;
    unsigned int *data;
    int count;
    double brightness, scale, theta;
    TQColor c;
    int h, s, v;

    sign = sharpen ? 1 : -1;
    scale=0.5000000000000001;
    if(img.depth() > 8){
        count = img.width()*img.height();
        data = (unsigned int *)img.bits();
    }
    else{
        count = img.numColors();
        data = (unsigned int *)img.colorTable();
    }
    for(i=0; i < count; ++i){
        c.setRgb(data[i]);
        c.hsv(&h, &s, &v);
        brightness = v/255.0;
        theta=(brightness-0.5)*M_PI;
        brightness+=scale*(((scale*((sin(theta)+1.0)))-brightness)*sign);
        if (brightness > 1.0)
            brightness=1.0;
        else
            if (brightness < 0)
                brightness=0.0;
        v = (int)(brightness*255);
        c.setHsv(h, s, v);
        data[i] = tqRgba(c.red(), c.green(), c.blue(), tqAlpha(data[i]));
    }
}


struct BumpmapParams {
    BumpmapParams( double bm_azimuth, double bm_elevation,
                   int bm_depth, KImageEffect::BumpmapType bm_type,
                   bool invert ) {
         /* Convert to radians */
        double azimuth = DegreesToRadians( bm_azimuth );
        double elevation = DegreesToRadians( bm_elevation );

        /* Calculate the light vector */
        lx = (int)( cos(azimuth) * cos(elevation) * 255.0 );
        ly = (int)( sin(azimuth) * cos(elevation) * 255.0 );
        int lz         = (int)( sin(elevation) * 255.0 );

        /* Calculate constant Z component of surface normal */
        int nz  = (6 * 255) / bm_depth;
        nz2     = nz * nz;
        nzlz    = nz * lz;

        /* Optimize for vertical normals */
        background = lz;

        /* Calculate darkness compensation factor */
        compensation = sin(elevation);

        /* Create look-up table for map type */
        for (int i = 0; i < 256; i++)
        {
            double n = 0;
            switch (bm_type)
            {
            case KImageEffect::Spherical:
                n = i / 255.0 - 1.0;
                lut[i] = (int) (255.0 * sqrt(1.0 - n * n) + 0.5);
                break;

            case KImageEffect::Sinuosidal:
                n = i / 255.0;
                lut[i] = (int) (255.0 * (sin((-M_PI / 2.0) + M_PI * n) + 1.0) /
                                        2.0 + 0.5);
                break;

            case KImageEffect::Linear:
            default:
                lut[i] = i;
            }

            if (invert)
                lut[i] = 255 - lut[i];
        }
    }
    int lx,  ly;
    int nz2, nzlz;
    int background;
    double compensation;
    uchar lut[256];
};


static void bumpmap_convert_row( uint *row,
                                 int    width,
                                 int    bpp,
                                 int    has_alpha,
                                 uchar *lut,
                                 int waterlevel )
{
  uint *p;

  p = row;

  has_alpha = has_alpha ? 1 : 0;

  if (bpp >= 3)
      for (; width; width--)
      {
          if (has_alpha) {
              unsigned int idx = (unsigned int)(intensityValue( *row ) + 0.5);
              *p++ = lut[(unsigned int) ( waterlevel +
                                          ( ( idx -
                                              waterlevel) * tqBlue( *row )) / 255.0 )];
          } else {
              unsigned int idx = (unsigned int)(intensityValue( *row ) + 0.5);
              *p++ = lut[idx];
          }

          ++row;
      }
}

static void bumpmap_row( uint           *src,
                         uint           *dest,
                         int              width,
                         int              bpp,
                         int              has_alpha,
                         uint           *bm_row1,
                         uint           *bm_row2,
                         uint           *bm_row3,
                         int              bm_width,
                         int              bm_xofs,
                         bool          tiled,
                         bool          row_in_bumpmap,
                         int           ambient,
                         bool          compensate,
                         BumpmapParams *params )
{
    int xofs1, xofs2, xofs3;
    int shade;
    int ndotl;
    int nx, ny;
    int x;
    int tmp;

    tmp = bm_xofs;
    xofs2 = MOD(tmp, bm_width);

    for (x = 0; x < width; x++)
    {
        /* Calculate surface normal from bump map */

        if (tiled || (row_in_bumpmap &&
                      x >= - tmp && x < - tmp + bm_width)) {
            if (tiled) {
                xofs1 = MOD(xofs2 - 1, bm_width);
                xofs3 = MOD(xofs2 + 1, bm_width);
	    } else {
                xofs1 = FXCLAMP(xofs2 - 1, 0, bm_width - 1);
                xofs3 = FXCLAMP(xofs2 + 1, 0, bm_width - 1);
	    }
            nx = (bm_row1[xofs1] + bm_row2[xofs1] + bm_row3[xofs1] -
                  bm_row1[xofs3] - bm_row2[xofs3] - bm_row3[xofs3]);
            ny = (bm_row3[xofs1] + bm_row3[xofs2] + bm_row3[xofs3] -
                  bm_row1[xofs1] - bm_row1[xofs2] - bm_row1[xofs3]);
	} else {
            nx = ny = 0;
        }

      /* Shade */

        if ((nx == 0) && (ny == 0))
            shade = params->background;
        else {
            ndotl = nx * params->lx + ny * params->ly + params->nzlz;

            if (ndotl < 0)
                shade = (int)( params->compensation * ambient );
            else {
                shade = (int)( ndotl / sqrt(double(nx * nx + ny * ny + params->nz2)) );

                shade = (int)( shade + TQMAX(0.0, (255 * params->compensation - shade)) *
                               ambient / 255 );
	    }
	}

        /* Paint */

        /**
         * NOTE: if we want to work with non-32bit images the alpha handling would
         * also change
         */
        if (compensate) {
            int red = (int)((tqRed( *src ) * shade) / (params->compensation * 255));
            int green = (int)((tqGreen( *src ) * shade) / (params->compensation * 255));
            int blue = (int)((tqBlue( *src ) * shade) / (params->compensation * 255));
            int alpha = (int)((tqAlpha( *src ) * shade) / (params->compensation * 255));
            ++src;
            *dest++ = tqRgba( red, green, blue, alpha );
        } else {
            int red = tqRed( *src ) * shade / 255;
            int green = tqGreen( *src ) * shade / 255;
            int blue = tqBlue( *src ) * shade / 255;
            int alpha = tqAlpha( *src ) * shade / 255;
            ++src;
            *dest++ = tqRgba( red, green, blue, alpha );
        }

        /* Next pixel */

        if (++xofs2 == bm_width)
            xofs2 = 0;
    }
}

/**
 * A bumpmapping algorithm.
 *
 * @param img the image you want bumpmap
 * @param map the map used
 * @param azimuth azimuth
 * @param elevation elevation
 * @param depth depth (not the depth of the image, but of the map)
 * @param xofs X offset
 * @param yofs Y offset
 * @param waterlevel level that full transparency should represent
 * @param ambient ambient lighting factor
 * @param compensate compensate for darkening
 * @param invert invert bumpmap
 * @param type type of the bumpmap
 *
 * @return The destination image (dst) containing the result.
 * @author Zack Rusin <zack@kde.org>
 */
TQImage KImageEffect::bumpmap(TQImage &img, TQImage &map, double azimuth, double elevation,
                             int depth, int xofs, int yofs, int waterlevel,
                             int ambient, bool compensate, bool invert,
                             BumpmapType type, bool tiled)
{
    TQImage dst;

    if ( img.depth() != 32 || img.depth() != 32 ) {
        tqWarning( "Bump-mapping effect works only with 32 bit images");
        return dst;
    }

    dst.create( img.width(), img.height(), img.depth() );
    int bm_width  = map.width();
    int bm_height = map.height();
    int bm_bpp = map.depth();
    int bm_has_alpha = map.hasAlphaBuffer();

    int yofs1, yofs2, yofs3;

    if ( tiled ) {
        yofs2 = MOD( yofs, bm_height );
        yofs1 = MOD( yofs2 - 1, bm_height);
        yofs3 = MOD( yofs2 + 1, bm_height);
    } else {
        yofs1 = 0;
        yofs2 = 0;
        yofs3 = FXCLAMP( yofs2+1, 0, bm_height - 1 );
    }

    BumpmapParams params( azimuth, elevation, depth, type, invert );

    uint* bm_row1 = (unsigned int*)map.scanLine( yofs1 );
    uint* bm_row2 = (unsigned int*)map.scanLine( yofs2 );
    uint* bm_row3 = (unsigned int*)map.scanLine( yofs3 );

    bumpmap_convert_row( bm_row1, bm_width, bm_bpp, bm_has_alpha, params.lut, waterlevel );
    bumpmap_convert_row( bm_row2, bm_width, bm_bpp, bm_has_alpha, params.lut, waterlevel );
    bumpmap_convert_row( bm_row3, bm_width, bm_bpp, bm_has_alpha, params.lut, waterlevel );

    for (int y = 0; y < img.height(); ++y)
    {
        int row_in_bumpmap = (y >= - yofs && y < - yofs + bm_height);

        uint* src_row = (unsigned int*)img.scanLine( y );
        uint* dest_row = (unsigned int*)dst.scanLine( y );

        bumpmap_row( src_row, dest_row, img.width(), img.depth(), img.hasAlphaBuffer(),
                     bm_row1, bm_row2, bm_row3, bm_width, xofs,
                     tiled,
                     row_in_bumpmap, ambient, compensate,
                     &params );

        /* Next line */

        if (tiled || row_in_bumpmap)
	{
            uint* bm_tmprow = bm_row1;
            bm_row1   = bm_row2;
            bm_row2   = bm_row3;
            bm_row3   = bm_tmprow;

            if (++yofs2 == bm_height)
                yofs2 = 0;

            if (tiled)
                yofs3 = MOD(yofs2 + 1, bm_height);
            else
                yofs3 = FXCLAMP(yofs2 + 1, 0, bm_height - 1);

            bm_row3 = (unsigned int*)map.scanLine( yofs3 );
            bumpmap_convert_row( bm_row3, bm_width, bm_bpp, bm_has_alpha,
                                 params.lut, waterlevel );
	}
    }
    return dst;
}

/**
 * Convert an image with standard alpha to premultiplied alpha
 *
 * @param img the image you want convert
 *
 * @return The destination image (dst) containing the result.
 * @author Timothy Pearson <kb9vqf@pearsoncomputing.net>
 */
TQImage KImageEffect::convertToPremultipliedAlpha(TQImage input) {
	TQImage alphaImage = input;
	if (!alphaImage.isNull()) alphaImage = alphaImage.convertDepth( 32 );

	int w = alphaImage.width();
	int h = alphaImage.height();

	int r;
	int g;
	int b;
	int a;
	float alpha_adjust;
	TQRgb l;
	TQRgb *ls;
	for (int y = 0; y < h; ++y) {
		ls = (TQRgb *)alphaImage.scanLine( y );
		for (int x = 0; x < w; ++x) {
			l = ls[x];
			alpha_adjust = (tqAlpha( l )/255.0);
			r = int( tqRed( l ) * alpha_adjust );
			g = int( tqGreen( l ) * alpha_adjust );
			b = int( tqBlue( l ) * alpha_adjust );
			a = int( tqAlpha( l ) * 1.0 );
			ls[x] = tqRgba( r, g, b, a );
		}
	}
	return alphaImage;
}
