/*
 * $Id$
 *
 * This file is part of the KDE project, module tdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * with minor additions and based on ideas from
 * Torsten Rahn <torsten@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#include <config.h>
#include <unistd.h>
#include <math.h>

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqbitmap.h>
#include <tqpixmap.h>
#include <tqimage.h>
#include <tqcolor.h>
#include <tqwidget.h>
#include <tqpainter.h>
#include <tqpen.h>
#include <tqapplication.h>
#include <tqpoint.h>
#include <tqrect.h>

#include <kdebug.h>
#include <tdeglobal.h>
#include <tdeconfig.h>
#include <tdeglobalsettings.h>
#include <kicontheme.h>
#include "kiconeffect.h"

#if defined(TQ_WS_WIN) || defined(TQ_WS_MACX)
static bool tqt_use_xrender=true;
static bool tqt_has_xft=true;
#else
extern bool tqt_use_xrender;
extern bool tqt_has_xft;
#endif
class TDEIconEffectPrivate
{
public:
	TQString mKey[6][3];
	TQColor  mColor2[6][3];
};

TDEIconEffect::TDEIconEffect()
{
    d = new TDEIconEffectPrivate;
    init();
}

TDEIconEffect::~TDEIconEffect()
{
    delete d;
    d = 0L;
}

void TDEIconEffect::init()
{
    TDEConfig *config = TDEGlobal::config();

    int i, j, effect=-1;
    TQStringList groups;
    groups += "Desktop";
    groups += "Toolbar";
    groups += "MainToolbar";
    groups += "Small";
    groups += "Panel";

    TQStringList states;
    states += "Default";
    states += "Active";
    states += "Disabled";

    TQStringList::ConstIterator it, it2;
    TQString _togray("togray");
    TQString _colorize("colorize");
    TQString _desaturate("desaturate");
    TQString _togamma("togamma");
    TQString _none("none");
    TQString _tomonochrome("tomonochrome");

    TDEConfigGroupSaver cs(config, "default");

    for (it=groups.begin(), i=0; it!=groups.end(); it++, i++)
    {
	// Default effects
	mEffect[i][0] = NoEffect;
	mEffect[i][1] =  ((i==0)||(i==4)) ? ToGamma : NoEffect;
	mEffect[i][2] = ToGray; 
	
	mTrans[i][0] = false;
	mTrans[i][1] = false;
	mTrans[i][2] = true;
        mValue[i][0] = 1.0;
        mValue[i][1] = ((i==0)||(i==4)) ? 0.7 : 1.0;
        mValue[i][2] = 1.0;
        mColor[i][0] = TQColor(144,128,248);
        mColor[i][1] = TQColor(169,156,255);
        mColor[i][2] = TQColor(34,202,0);
        d->mColor2[i][0] = TQColor(0,0,0);
        d->mColor2[i][1] = TQColor(0,0,0);
        d->mColor2[i][2] = TQColor(0,0,0);

	config->setGroup(*it + "Icons");
	for (it2=states.begin(), j=0; it2!=states.end(); it2++, j++)
	{
	    TQString tmp = config->readEntry(*it2 + "Effect");
	    if (tmp == _togray)
		effect = ToGray;
	    else if (tmp == _colorize)
		effect = Colorize;
	    else if (tmp == _desaturate)
		effect = DeSaturate;
	    else if (tmp == _togamma)
		effect = ToGamma;
	    else if (tmp == _tomonochrome)
		effect = ToMonochrome;
            else if (tmp == _none)
		effect = NoEffect;
	    else
		continue;
	    if(effect != -1)
                mEffect[i][j] = effect;
	    mValue[i][j] = config->readDoubleNumEntry(*it2 + "Value");
	    mColor[i][j] = config->readColorEntry(*it2 + "Color");
	    d->mColor2[i][j] = config->readColorEntry(*it2 + "Color2");
	    mTrans[i][j] = config->readBoolEntry(*it2 + "SemiTransparent");

	}
    }    
}

bool TDEIconEffect::hasEffect(int group, int state) const
{
    return mEffect[group][state] != NoEffect;
}

TQString TDEIconEffect::fingerprint(int group, int state) const
{
    if ( group >= TDEIcon::LastGroup ) return "";
    TQString cached = d->mKey[group][state];
    if (cached.isEmpty())
    {
        TQString tmp;
        cached = tmp.setNum(mEffect[group][state]);
        cached += ':';
        cached += tmp.setNum(mValue[group][state]);
        cached += ':';
        cached += mTrans[group][state] ? TQString::fromLatin1("trans")
            : TQString::fromLatin1("notrans");
        if (mEffect[group][state] == Colorize || mEffect[group][state] == ToMonochrome)
        {
            cached += ':';
            cached += mColor[group][state].name();
        }
        if (mEffect[group][state] == ToMonochrome)
        {
            cached += ':';
            cached += d->mColor2[group][state].name();
        }
    
        d->mKey[group][state] = cached;    
    }
    
    return cached;
}

TQImage TDEIconEffect::apply(TQImage image, int group, int state) const
{
    if (state >= TDEIcon::LastState)
    {
	kdDebug(265) << "Illegal icon state: " << state << "\n";
	return image;
    }
    if (group >= TDEIcon::LastGroup)
    {
	kdDebug(265) << "Illegal icon group: " << group << "\n";
	return image;
    }
    return apply(image, mEffect[group][state], mValue[group][state],
	    mColor[group][state], d->mColor2[group][state], mTrans[group][state]);
}

TQImage TDEIconEffect::apply(TQImage image, int effect, float value, const TQColor col, bool trans) const
{
    return apply (image, effect, value, col, TDEGlobalSettings::baseColor(), trans);
}

TQImage TDEIconEffect::apply(TQImage image, int effect, float value, const TQColor col, const TQColor col2, bool trans) const
{
    if (effect >= LastEffect )
    {
	kdDebug(265) << "Illegal icon effect: " << effect << "\n";
	return image;
    }
    if (value > 1.0)
	value = 1.0;
    else if (value < 0.0)
	value = 0.0;
    switch (effect)
    {
    case ToGray:
	toGray(image, value);
	break;
    case DeSaturate:
	deSaturate(image, value);
	break;
    case Colorize:
        colorize(image, col, value);
        break;
    case ToGamma:
        toGamma(image, value);
        break;
    case ToMonochrome:
        toMonochrome(image, col, col2, value);
        break;
    }
    if (trans == true)
    {
	semiTransparent(image);
    }
    return image;
}

TQPixmap TDEIconEffect::apply(TQPixmap pixmap, int group, int state) const
{
    if (state >= TDEIcon::LastState)
    {
	kdDebug(265) << "Illegal icon state: " << state << "\n";
	return pixmap;
    }
    if (group >= TDEIcon::LastGroup)
    {
	kdDebug(265) << "Illegal icon group: " << group << "\n";
	return pixmap;
    }
    return apply(pixmap, mEffect[group][state], mValue[group][state],
	    mColor[group][state], d->mColor2[group][state], mTrans[group][state]);
}

TQPixmap TDEIconEffect::apply(TQPixmap pixmap, int effect, float value,
	const TQColor col, bool trans) const
{
    return apply (pixmap, effect, value, col, TDEGlobalSettings::baseColor(), trans);
}

TQPixmap TDEIconEffect::apply(TQPixmap pixmap, int effect, float value,
	const TQColor col, const TQColor col2, bool trans) const
{
    TQPixmap result;

    if (effect >= LastEffect )
    {
	kdDebug(265) << "Illegal icon effect: " << effect << "\n";
	return result;
    }

    if ((trans == true) && (effect == NoEffect))
    {
        result = pixmap;
        semiTransparent(result);
    }
    else if ( effect != NoEffect )
    {
        TQImage tmpImg = pixmap.convertToImage();
        tmpImg = apply(tmpImg, effect, value, col, col2, trans);
        result.convertFromImage(tmpImg);
    }
    else
        result = pixmap;

    return result;
}

// Taken from KImageEffect. We don't want to link tdecore to tdeui! As long
// as this code is not too big, it doesn't seem much of a problem to me.

void TDEIconEffect::toGray(TQImage &img, float value)
{
    int pixels = (img.depth() > 8) ? img.width()*img.height()
	    : img.numColors();
    unsigned int *data = img.depth() > 8 ? (unsigned int *) img.bits()
	    : (unsigned int *) img.colorTable();
    int rval, gval, bval, val, alpha, i;
    for (i=0; i<pixels; i++)
    {
	val = tqGray(data[i]);
	alpha = tqAlpha(data[i]);
	if (value < 1.0)
	{
	    rval = static_cast<int>(value*val+(1.0-value)*tqRed(data[i]));
	    gval = static_cast<int>(value*val+(1.0-value)*tqGreen(data[i]));
	    bval = static_cast<int>(value*val+(1.0-value)*tqBlue(data[i]));
	    data[i] = tqRgba(rval, gval, bval, alpha);
	} else
	    data[i] = tqRgba(val, val, val, alpha);
    }
}

void TDEIconEffect::colorize(TQImage &img, const TQColor &col, float value)
{
    int pixels = (img.depth() > 8) ? img.width()*img.height()
	    : img.numColors();
    unsigned int *data = img.depth() > 8 ? (unsigned int *) img.bits()
	    : (unsigned int *) img.colorTable();
    int rval, gval, bval, val, alpha, i;
    float rcol = col.red(), gcol = col.green(), bcol = col.blue();
    for (i=0; i<pixels; i++)
    {
        val = tqGray(data[i]);
        if (val < 128)
        {
             rval = static_cast<int>(rcol/128*val);
             gval = static_cast<int>(gcol/128*val);
             bval = static_cast<int>(bcol/128*val);
        }
        else if (val > 128)
        {
             rval = static_cast<int>((val-128)*(2-rcol/128)+rcol-1);
             gval = static_cast<int>((val-128)*(2-gcol/128)+gcol-1);
             bval = static_cast<int>((val-128)*(2-bcol/128)+bcol-1);
        }
	else // val == 128
	{
             rval = static_cast<int>(rcol);
             gval = static_cast<int>(gcol);
             bval = static_cast<int>(bcol);
	}
	if (value < 1.0)
	{
	    rval = static_cast<int>(value*rval+(1.0 - value)*tqRed(data[i]));
	    gval = static_cast<int>(value*gval+(1.0 - value)*tqGreen(data[i]));
	    bval = static_cast<int>(value*bval+(1.0 - value)*tqBlue(data[i]));
	}

	alpha = tqAlpha(data[i]);
	data[i] = tqRgba(rval, gval, bval, alpha);
    }
}

void TDEIconEffect::toMonochrome(TQImage &img, const TQColor &black, const TQColor &white, float value) {
   int pixels = (img.depth() > 8) ? img.width()*img.height() : img.numColors();
   unsigned int *data = img.depth() > 8 ? (unsigned int *) img.bits()
         : (unsigned int *) img.colorTable();
   int rval, gval, bval, alpha, i;
   int rw = white.red(), gw = white.green(), bw = white.blue();
   int rb = black.red(), gb = black.green(), bb = black.blue();
   
   double values = 0, sum = 0;
   bool grayscale = true;
   // Step 1: determine the average brightness
   for (i=0; i<pixels; i++) {
       sum += tqGray(data[i])*tqAlpha(data[i]) + 255*(255-tqAlpha(data[i]));
       values += 255;
       if ((tqRed(data[i]) != tqGreen(data[i]) ) || (tqGreen(data[i]) != tqBlue(data[i]) ))
           grayscale = false;
   }
   double medium = sum/values;

   // Step 2: Modify the image
   if (grayscale) {
       for (i=0; i<pixels; i++) {
           int v = tqRed(data[i]);
           rval = static_cast<int>( ((255-v)*rb + v*rw)*value/255 + (1.0-value)*tqRed(data[i]));
           gval = static_cast<int>( ((255-v)*gb + v*gw)*value/255 + (1.0-value)*tqGreen(data[i]));
           bval = static_cast<int>( ((255-v)*bb + v*bw)*value/255 + (1.0-value)*tqBlue(data[i]));

           alpha = tqAlpha(data[i]);
           data[i] = tqRgba(rval, gval, bval, alpha);
       }
   }
   else {
      for (i=0; i<pixels; i++) {
         if (tqGray(data[i]) <= medium) {
            rval = static_cast<int>(value*rb+(1.0-value)*tqRed(data[i]));
            gval = static_cast<int>(value*gb+(1.0-value)*tqGreen(data[i]));
            bval = static_cast<int>(value*bb+(1.0-value)*tqBlue(data[i]));
         }
         else {
            rval = static_cast<int>(value*rw+(1.0-value)*tqRed(data[i]));
            gval = static_cast<int>(value*gw+(1.0-value)*tqGreen(data[i]));
            bval = static_cast<int>(value*bw+(1.0-value)*tqBlue(data[i]));
         }

         alpha = tqAlpha(data[i]);
         data[i] = tqRgba(rval, gval, bval, alpha);
      }
   }
}

void TDEIconEffect::deSaturate(TQImage &img, float value)
{
    int pixels = (img.depth() > 8) ? img.width()*img.height()
	    : img.numColors();
    unsigned int *data = (img.depth() > 8) ? (unsigned int *) img.bits()
	    : (unsigned int *) img.colorTable();
    TQColor color;
    int h, s, v, i;
    for (i=0; i<pixels; i++)
    {
        color.setRgb(data[i]);
        color.hsv(&h, &s, &v);
        color.setHsv(h, (int) (s * (1.0 - value) + 0.5), v);
	data[i] = tqRgba(color.red(), color.green(), color.blue(),
		tqAlpha(data[i]));
    }
}

void TDEIconEffect::toGamma(TQImage &img, float value)
{
    int pixels = (img.depth() > 8) ? img.width()*img.height()
	    : img.numColors();
    unsigned int *data = (img.depth() > 8) ? (unsigned int *) img.bits()
	    : (unsigned int *) img.colorTable();
    TQColor color;
    int i, rval, gval, bval;
    float gamma;
    gamma = 1/(2*value+0.5);

    for (i=0; i<pixels; i++)
    {
        color.setRgb(data[i]);
        color.rgb(&rval, &gval, &bval);
        rval = static_cast<int>(pow(static_cast<float>(rval)/255 , gamma)*255);
        gval = static_cast<int>(pow(static_cast<float>(gval)/255 , gamma)*255);
        bval = static_cast<int>(pow(static_cast<float>(bval)/255 , gamma)*255);
	data[i] = tqRgba(rval, gval, bval, tqAlpha(data[i]));
    }
}

void TDEIconEffect::semiTransparent(TQImage &img)
{
    img.setAlphaBuffer(true);

    int x, y;
    if (img.depth() == 32)
    {
	int width  = img.width();
	int height = img.height();
	
	if (tqt_use_xrender && tqt_has_xft )
	  for (y=0; y<height; y++)
	  {
#ifdef WORDS_BIGENDIAN
	    uchar *line = (uchar*) img.scanLine(y);
#else
	    uchar *line = (uchar*) img.scanLine(y) + 3;
#endif
	    for (x=0; x<width; x++)
	    {
		*line >>= 1;
		line += 4;
	    }
	  }
	else
	  for (y=0; y<height; y++)
	  {
	    TQRgb *line = (TQRgb *) img.scanLine(y);
	    for (x=(y%2); x<width; x+=2)
		line[x] &= 0x00ffffff;
	  }

    } else
    {
	// Insert transparent pixel into the clut.
	int transColor = -1;

        // search for a color that is already transparent
        for (x=0; x<img.numColors(); x++)
        {
            // try to find already transparent pixel
            if (tqAlpha(img.color(x)) < 127)
            {
                transColor = x;
                break;
            }
        }


        // FIXME: image must have transparency
        if(transColor < 0 || transColor >= img.numColors())
            return;

	img.setColor(transColor, 0);
        if(img.depth() == 8)
        {
            for (y=0; y<img.height(); y++)
            {
                unsigned char *line = img.scanLine(y);
                for (x=(y%2); x<img.width(); x+=2)
                    line[x] = transColor;
            }
	}
        else
        {
            // SLOOW, but simple, as we would have to
            // deal with endianess etc on our own here
            for (y=0; y<img.height(); y++)
                for (x=(y%2); x<img.width(); x+=2)
                    img.setPixel(x, y, transColor);
        }
    }
}

void TDEIconEffect::semiTransparent(TQPixmap &pix)
{
    if ( tqt_use_xrender && tqt_has_xft )
    {
	TQImage img=pix.convertToImage();
	semiTransparent(img);
	pix.convertFromImage(img);
	return;
    }

    TQImage img;
    if (pix.mask() != 0L)
	img = pix.mask()->convertToImage();
    else
    {
	img.create(pix.size(), 1, 2, TQImage::BigEndian);
	img.fill(1);
    }

    for (int y=0; y<img.height(); y++)
    {
	TQRgb *line = (TQRgb *) img.scanLine(y);
	TQRgb pattern = (y % 2) ? 0x55555555 : 0xaaaaaaaa;
	for (int x=0; x<(img.width()+31)/32; x++)
	    line[x] &= pattern;
    }
    TQBitmap mask;
    mask.convertFromImage(img);
    pix.setMask(mask);
}

TQImage TDEIconEffect::doublePixels(TQImage src) const
{
    TQImage dst;
    if (src.depth() == 1)
    {
	kdDebug(265) << "image depth 1 not supported\n";
	return dst;
    }

    int w = src.width();
    int h = src.height();
    dst.create(w*2, h*2, src.depth());
    dst.setAlphaBuffer(src.hasAlphaBuffer());

    int x, y;
    if (src.depth() == 32)
    {
	TQRgb *l1, *l2;
	for (y=0; y<h; y++)
	{
	    l1 = (TQRgb *) src.scanLine(y);
	    l2 = (TQRgb *) dst.scanLine(y*2);
	    for (x=0; x<w; x++)
	    {
		l2[x*2] = l2[x*2+1] = l1[x];
	    }
	    memcpy(dst.scanLine(y*2+1), l2, dst.bytesPerLine());
	}
    } else
    {
	for (x=0; x<src.numColors(); x++)
	    dst.setColor(x, src.color(x));

	unsigned char *l1, *l2;
	for (y=0; y<h; y++)
	{
	    l1 = src.scanLine(y);
	    l2 = dst.scanLine(y*2);
	    for (x=0; x<w; x++)
	    {
		l2[x*2] = l1[x];
		l2[x*2+1] = l1[x];
	    }
	    memcpy(dst.scanLine(y*2+1), l2, dst.bytesPerLine());
	}
    }
    return dst;
}

void TDEIconEffect::overlay(TQImage &src, TQImage &overlay)
{
    if (src.depth() != overlay.depth())
    {
	kdDebug(265) << "Image depth src != overlay!\n";
	return;
    }
    if (src.size() != overlay.size())
    {
	kdDebug(265) << "Image size src != overlay\n";
	return;
    }
    if (!overlay.hasAlphaBuffer())
    {
	kdDebug(265) << "Overlay doesn't have alpha buffer!\n";
	return;
    }

    int i, j;

    // We don't do 1 bpp

    if (src.depth() == 1)
    {
	kdDebug(265) << "1bpp not supported!\n";
	return;
    }

    // Overlay at 8 bpp doesn't use alpha blending

    if (src.depth() == 8)
    {
	if (src.numColors() + overlay.numColors() > 255)
	{
	    kdDebug(265) << "Too many colors in src + overlay!\n";
	    return;
	}

	// Find transparent pixel in overlay
	int trans;
	for (trans=0; trans<overlay.numColors(); trans++)
	{
	    if (tqAlpha(overlay.color(trans)) == 0)
	    {
		kdDebug(265) << "transparent pixel found at " << trans << "\n";
		break;
	    }
	}
	if (trans == overlay.numColors())
	{
	    kdDebug(265) << "transparent pixel not found!\n";
	    return;
	}

	// Merge color tables
	int nc = src.numColors();
	src.setNumColors(nc + overlay.numColors());
	for (i=0; i<overlay.numColors(); i++)
	{
	    src.setColor(nc+i, overlay.color(i));
	}

	// Overwrite nontransparent pixels.
	unsigned char *oline, *sline;
	for (i=0; i<src.height(); i++)
	{
	    oline = overlay.scanLine(i);
	    sline = src.scanLine(i);
	    for (j=0; j<src.width(); j++)
	    {
		if (oline[j] != trans)
		    sline[j] = oline[j]+nc;
	    }
	}
    }

    // Overlay at 32 bpp does use alpha blending

    if (src.depth() == 32)
    {
	TQRgb *oline, *sline;
	int r1, g1, b1, a1;
	int r2, g2, b2, a2;

	for (i=0; i<src.height(); i++)
	{
	    oline = (TQRgb *) overlay.scanLine(i);
	    sline = (TQRgb *) src.scanLine(i);

	    for (j=0; j<src.width(); j++)
	    {
		r1 = tqRed(oline[j]);
		g1 = tqGreen(oline[j]);
		b1 = tqBlue(oline[j]);
		a1 = tqAlpha(oline[j]);

		r2 = tqRed(sline[j]);
		g2 = tqGreen(sline[j]);
		b2 = tqBlue(sline[j]);
		a2 = tqAlpha(sline[j]);

		r2 = (a1 * r1 + (0xff - a1) * r2) >> 8;
		g2 = (a1 * g1 + (0xff - a1) * g2) >> 8;
		b2 = (a1 * b1 + (0xff - a1) * b2) >> 8;
		a2 = TQMAX(a1, a2);

		sline[j] = tqRgba(r2, g2, b2, a2);
	    }
	}
    }

    return;
}

    void
TDEIconEffect::visualActivate(TQWidget * widget, TQRect rect)
{
    if (!TDEGlobalSettings::visualActivate())
        return;

    uint actSpeed = TDEGlobalSettings::visualActivateSpeed();

    uint actCount = TQMIN(rect.width(), rect.height()) / 2;

    // Clip actCount to range 1..10.

    if (actCount < 1)
        actCount = 1;

    else if (actCount > 10)
        actCount = 10;

    // Clip actSpeed to range 1..100.

    if (actSpeed < 1)
        actSpeed = 1;

    else if (actSpeed > 100)
        actSpeed = 100;

    // actSpeed needs to be converted to actDelay.
    // actDelay is inversely proportional to actSpeed and needs to be
    // divided up into actCount portions.
    // We also convert the us value to ms.

    unsigned int actDelay = (1000 * (100 - actSpeed)) / actCount;

    //kdDebug() << "actCount=" << actCount << " actDelay=" << actDelay << endl;

    TQPoint c = rect.center();

    TQPainter p(widget);

    // Use NotROP to avoid having to repaint the pixmap each time.
    p.setPen(TQPen(TQt::black, 2, TQt::DotLine));
    p.setRasterOp(TQt::NotROP);

    // The spacing between the rects we draw.
    // Use the minimum of width and height to avoid painting outside the
    // pixmap area.
    //unsigned int delta(TQMIN(rect.width() / actCount, rect.height() / actCount));

    // Support for rectangles by David
    unsigned int deltaX = rect.width() / actCount;
    unsigned int deltaY = rect.height() / actCount;

    for (unsigned int i = 1; i < actCount; i++) {

        int w = i * deltaX;
        int h = i * deltaY;

        rect.setRect(c.x() - w / 2, c.y() - h / 2, w, h);

        p.drawRect(rect);
        p.flush();

        usleep(actDelay);

        p.drawRect(rect);
    }
}

void
TDEIconEffect::visualActivate(TQWidget * widget, TQRect rect, TQPixmap *pixmap)
{
    if (!TDEGlobalSettings::visualActivate())
        return;

    // Image too big to display smoothly
    if ((rect.width() > 160) || (rect.height() > 160)) {
	visualActivate(widget, rect); // call old effect
	return;
    }

    uint actSpeed = TDEGlobalSettings::visualActivateSpeed();
    uint actCount = TQMIN(rect.width(), rect.height()) / 4;


    // Clip actCount to range 1..10.
    if (actCount < 1)
        actCount = 1;

    else if (actCount > 10)
        actCount = 10;

    // Clip actSpeed to range 1..100.
    if (actSpeed < 1)
        actSpeed = 1;

    else if (actSpeed > 100)
        actSpeed = 100;

    // actSpeed needs to be converted to actDelay.
    // actDelay is inversely proportional to actSpeed and needs to be
    // divided up into actCount portions.
    // We also convert the us value to ms.

    unsigned int actDelay = (1000 * (100 - actSpeed)) / actCount;

    unsigned int deltaX = rect.width() / actCount * 1.5;
    unsigned int deltaY = rect.height() / actCount * 1.5;

    TQPoint c = rect.center();
    TQRect maxRect(c.x() - (actCount * 2) * deltaX /2,
	          c.y() - (actCount * 2) * deltaY /2,
		  actCount * 2 * deltaX,
		  actCount * 2 * deltaY);

    // convert rect to global coordinates if needed
    if ((widget->rect().width() <= maxRect.width())
       || (widget->rect().height() <= maxRect.height()))
    {
	TQPoint topLeft(rect.x(), rect.y());
	rect.moveLeft(widget->mapToGlobal(topLeft).x());
	rect.moveTop(widget->mapToGlobal(topLeft).y());
	c = rect.center();
	maxRect.setRect(c.x() - (actCount * 2) * deltaX /2,
	        	c.y() - (actCount * 2) * deltaY /2,
			actCount * 2 * deltaX,
			actCount * 2 * deltaY);
    }

    TQPainter *p;
    TQImage img = pixmap->convertToImage();
    TQPixmap pix;
    TQPixmap composite(maxRect.width(), maxRect.height(), -1, TQPixmap::BestOptim);
    TQPainter cPainter(&composite);
    TQPoint cComposite = composite.rect().center();

    // enable alpha blending
    img.setAlphaBuffer(true);
    
    // Ugly hack... Get "Screenshot" to blt into and even do that on the
    // root window if the display area of <widget> is too small
    if ((widget->rect().width() <= maxRect.width())
       || (widget->rect().height() <= maxRect.height()))
    {
//	p = new TQPainter(TQApplication::desktop()->screen( -1 ), TRUE);	// WARNING: This was done in Qt3.  It only worked in this placement due to a glitch in Qt3; it has therefore been moved below grabWidget, where it should have been in the first place.
	pix = TQPixmap::grabWindow((TQApplication::desktop()->screen( -1 ))->winId(),
		    		      maxRect.x(),
				      maxRect.y(),
				      maxRect.width(),
				      maxRect.height());
        p = new TQPainter(TQApplication::desktop()->screen( -1 ), TRUE);
    } else
    {
	// not as ugly as drawing directly to the screen
//	p = new TQPainter(widget);	// WARNING: This was done in Qt3.  See above.
	pix = TQPixmap::grabWidget(widget,
			              maxRect.x(),
				      maxRect.y(),
				      maxRect.width(),
				      maxRect.height());
	p = new TQPainter(widget);
    }
    uchar deltaAlpha = 255 / (actCount * 1.2);
    
    // Activate effect like MacOS X
    for (unsigned int i = actCount; i < actCount * 2; i++) {

        int w = i * deltaX;
        int h = i * deltaY;

        rect.setRect(cComposite.x() - w / 2, cComposite.y() - h / 2, w, h);

	// draw offscreen
	cPainter.drawPixmap(0, 0, pix, 0, 0, pix.width(), pix.height());
	cPainter.drawImage(rect, img);
	cPainter.flush();

	// put onscreen
	p->drawPixmap(maxRect, composite);
        p->flush();

	// Fade out Icon a bit more
        int x, y;
        if ((img.depth() == 32) && tqt_use_xrender && tqt_has_xft)
        {
    	    int width  = img.width();
	    int height = img.height();
	
	    for (y=0; y<height; y++)
	    {
#ifdef WORDS_BIGENDIAN
		uchar *line = (uchar*) img.scanLine(y);
#else
		uchar *line = (uchar*) img.scanLine(y) + 3;
#endif
		for (x=0; x<width; x++)
		{
		    *line = (*line < deltaAlpha) ? 0 : *line - deltaAlpha;
		    line += 4;
		}
	    }
	}
        usleep(actDelay*3);
    }

    // remove traces of the effect
    if ((widget->rect().width() <= maxRect.width())
       || (widget->rect().height() <= maxRect.height()))
	p->drawPixmap(maxRect, pix);
    else {
	 p->drawPixmap(maxRect, pix);
        widget->update(rect);
    }

    delete p;
}
