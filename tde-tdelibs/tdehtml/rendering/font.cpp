/**
 * This file is part of the html renderer for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
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

#include "config.h"

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include "font.h"
#include "tdehtml_factory.h"
#include "tdehtml_settings.h"

#include <kdebug.h>
#include <tdeglobal.h>

#include <tqpainter.h>
#include <tqfontdatabase.h>
#include <tqpaintdevicemetrics.h>

using namespace tdehtml;

/** closes the current word and returns its width in pixels
 * @param fm metrics of font to be used
 * @param str string
 * @param pos zero-indexed position within @p str upon which all other
 *	indices are based
 * @param wordStart relative index pointing to the position where the word started
 * @param wordEnd relative index pointing one position after the word ended
 * @return the width in pixels. May be 0 if @p wordStart and @p wordEnd were
 *	equal.
 */
inline int closeWordAndGetWidth(const TQFontMetrics &fm, const TQChar *str, int pos,
	int wordStart, int wordEnd)
{
    if (wordEnd <= wordStart) return 0;

    TQConstString s(str + pos + wordStart, wordEnd - wordStart);
    return fm.width(s.string());
}

/** closes the current word and draws it
 * @param p painter
 * @param d text direction
 * @param x current x position, will be inc-/decremented correctly according
 *	to text direction
 * @param y baseline of text
 * @param widths list of widths; width of word is expected at position
 *		wordStart
 * @param str string
 * @param pos zero-indexed position within @p str upon which all other
 *	indices are based
 * @param wordStart relative index pointing to the position where the word started,
 *	will be set to wordEnd after function
 * @param wordEnd relative index pointing one position after the word ended
 */
inline void closeAndDrawWord(TQPainter *p, TQPainter::TextDirection d,
	int &x, int y, const short widths[], const TQChar *str, int pos,
	int &wordStart, int wordEnd)
{
    if (wordEnd <= wordStart) return;

    int width = widths[wordStart];
    if (d == TQPainter::RTL)
      x -= width;

    TQConstString s(str + pos + wordStart, wordEnd - wordStart);
    p->drawText(x, y, s.string(), -1, d);

    if (d != TQPainter::RTL)
      x += width;

    wordStart = wordEnd;
}

void Font::drawText( TQPainter *p, int x, int y, TQChar *str, int slen, int pos, int len,
        int toAdd, TQPainter::TextDirection d, int from, int to, TQColor bg, int uy, int h, int deco ) const
{
    if (!str) return;
    TQConstString cstr = TQConstString(str, slen);
    TQString qstr = cstr.string();

    // ### fixme for RTL
    if ( !scFont && !letterSpacing && !wordSpacing && !toAdd && from==-1 ) {
	// simply draw it
	// Due to some unfounded cause TQPainter::drawText traverses the
        // *whole* string when painting, not only the specified
        // [pos, pos + len) segment. This makes painting *extremely* slow for
        // long render texts (in the order of several megabytes).
        // Hence, only hand over the piece of text of the actual inline text box
	TQConstString cstr = TQConstString(str + pos, len);
	p->drawText( x, y, cstr.string(), 0, len, d );
    } else {
	if (from < 0) from = 0;
	if (to < 0) to = len;

	int numSpaces = 0;
	if ( toAdd ) {
	    for( int i = 0; i < len; ++i )
		if ( str[i+pos].category() == TQChar::Separator_Space )
		    ++numSpaces;
	}

	const int totWidth = width( str, slen, pos, len );
	if ( d == TQPainter::RTL ) {
	    x += totWidth + toAdd;
	}
	TQString upper = qstr;
	TQFontMetrics sc_fm = fm;
	if ( scFont ) {
	    // draw in small caps
	    upper = qstr.upper();
	    sc_fm = TQFontMetrics( *scFont );
	}

	// ### sc could be optimized by only painting uppercase letters extra,
	// and treat the rest WordWise, but I think it's not worth it.
	// Somebody else may volunteer, and implement this ;-) (LS)

	// The mode determines whether the text is displayed character by
	// character, word by word, or as a whole
	enum { CharacterWise, WordWise, Whole }
	mode = Whole;
	if (!letterSpacing && !scFont && (wordSpacing || toAdd > 0))
	  mode = WordWise;
	else if (letterSpacing || scFont)
	  mode = CharacterWise;

	if (mode == Whole) {	// most likely variant is treated extra

	    if (to < 0) to = len;
	    const TQConstString cstr(str + pos, len);
	    const TQConstString segStr(str + pos + from, to - from);
	    const int preSegmentWidth = fm.width(cstr.string(), from);
	    const int segmentWidth = fm.width(segStr.string());
	    const int eff_x = d == TQPainter::RTL ? x - preSegmentWidth - segmentWidth
					: x + preSegmentWidth;

	    // draw whole string segment, with optional background
	    if ( bg.isValid() )
		p->fillRect( eff_x, uy, segmentWidth, h, bg );
	    p->drawText(eff_x, y, segStr.string(), -1, d);
	    if (deco)
	        drawDecoration(p, eff_x, uy, y - uy, segmentWidth - 1, h, deco);
	    return;
	}

	// We are using two passes. In the first pass, the widths are collected,
	// and stored. In the second, the actual characters are drawn.

	// For each letter in the text box, save the width of the character.
	// When word-wise, only the first letter contains the width, but of the
	// whole word.
        short* const widthList = (short *)alloca(to*sizeof(short));

	// First pass: gather widths
	int preSegmentWidth = 0;
	int segmentWidth = 0;
        int lastWordBegin = 0;
	bool onSegment = from == 0;
	for( int i = 0; i < to; ++i ) {
	    if (i == from) {
                // Also close words on visibility boundary
	        if (mode == WordWise) {
	            const int width = closeWordAndGetWidth(fm, str, pos, lastWordBegin, i);

		    if (lastWordBegin < i) {
		        widthList[lastWordBegin] = (short)width;
		        lastWordBegin = i;
		        preSegmentWidth += width;
		    }
		}
		onSegment = true;
	    }

	    const TQChar ch = str[pos+i];
	    bool lowercase = (ch.category() == TQChar::Letter_Lowercase);
	    bool is_space = (ch.category() == TQChar::Separator_Space);
	    int chw = 0;
	    if ( letterSpacing )
		chw += letterSpacing;
	    if ( (wordSpacing || toAdd) && is_space ) {
	        if (mode == WordWise) {
		    const int width = closeWordAndGetWidth(fm, str, pos, lastWordBegin, i);
		    if (lastWordBegin < i) {
		        widthList[lastWordBegin] = (short)width;
			lastWordBegin = i;
		        (onSegment ? segmentWidth : preSegmentWidth) += width;
		    }
		    ++lastWordBegin;		// ignore this space
		}
		chw += wordSpacing;
		if ( numSpaces ) {
		    const int a = toAdd/numSpaces;
		    chw += a;
		    toAdd -= a;
		    --numSpaces;
		}
	    }
	    if (is_space || mode == CharacterWise) {
	        chw += lowercase ? sc_fm.charWidth( upper, pos+i ) : fm.charWidth( qstr, pos+i );
		widthList[i] = (short)chw;

		(onSegment ? segmentWidth : preSegmentWidth) += chw;
	    }

	}

	// close last word
	Q_ASSERT(onSegment);
	if (mode == WordWise) {
	    const int width = closeWordAndGetWidth(fm, str, pos, lastWordBegin, to);
	    segmentWidth += width;
	    widthList[lastWordBegin] = (short)width;
	}

        if (d == TQPainter::RTL) x -= preSegmentWidth;
	else x += preSegmentWidth;

        const int startx = d == TQPainter::RTL ? x-segmentWidth : x;

	// optionally draw background
	if ( bg.isValid() )
	    p->fillRect( startx, uy, segmentWidth, h, bg );

	// second pass: do the actual drawing
        lastWordBegin = from;
	for( int i = from; i < to; ++i ) {
	    const TQChar ch = str[pos+i];
	    bool lowercase = (ch.category() == TQChar::Letter_Lowercase);
	    bool is_space = ch.category() == TQChar::Separator_Space;
	    if ( is_space ) {
	        if (mode == WordWise) {
		    closeAndDrawWord(p, d, x, y, widthList, str, pos, lastWordBegin, i);
		    ++lastWordBegin;	// jump over space
		}
	    }
	    if (is_space || mode == CharacterWise) {
	        const int chw = widthList[i];
	        if (d == TQPainter::RTL)
		    x -= chw;

		if ( scFont )
		    p->setFont( lowercase ? *scFont : f );
		p->drawText( x, y, (lowercase ? upper : qstr), pos+i, 1, d );

	        if (d != TQPainter::RTL)
		    x += chw;
	    }

	}

	// don't forget to draw last word
	if (mode == WordWise) {
	    closeAndDrawWord(p, d, x, y, widthList, str, pos, lastWordBegin, to);
	}

	if (deco)
	    drawDecoration(p, startx, uy, y - uy, segmentWidth - 1, h, deco);

	if ( scFont )
	    p->setFont( f );
    }
}


int Font::width( TQChar *chs, int, int pos, int len, int start, int end, int toAdd ) const
{
    const TQConstString cstr(chs+pos, len);
    int w = 0;

    const TQString qstr = cstr.string();
    if ( scFont ) {
	const TQString upper = qstr.upper();
	const TQChar *uc = qstr.unicode();
	const TQFontMetrics sc_fm( *scFont );
	for ( int i = 0; i < len; ++i ) {
	    if ( (uc+i)->category() == TQChar::Letter_Lowercase )
		w += sc_fm.charWidth( upper, i );
	    else
		w += fm.charWidth( qstr, i );
	}
    } else {
	// ### might be a little inaccurate
	w = fm.width( qstr );
    }

    if ( letterSpacing )
	w += len*letterSpacing;

    if ( wordSpacing )
	// add amount
	for( int i = 0; i < len; ++i ) {
	    if( chs[i+pos].category() == TQChar::Separator_Space )
		w += wordSpacing;
	}

    if ( toAdd ) {
        // first gather count of spaces
        int numSpaces = 0;
        for( int i = start; i != end; ++i )
            if ( chs[i].category() == TQChar::Separator_Space )
                ++numSpaces;
        // distribute pixels evenly among spaces, but count only those within
        // [pos, pos+len)
        for ( int i = start; numSpaces && i != pos + len; i++ )
            if ( chs[i].category() == TQChar::Separator_Space ) {
                const int a = toAdd/numSpaces;
                if ( i >= pos ) w += a;
                toAdd -= a;
                --numSpaces;
            }
    }

    return w;
}

int Font::width( TQChar *chs, int slen, int pos ) const
{
    int w;
	if ( scFont && chs[pos].category() == TQChar::Letter_Lowercase ) {
	    TQString str( chs, slen );
	    str[pos] = chs[pos].upper();
	    w = TQFontMetrics( *scFont ).charWidth( str, pos );
	} else {
	    const TQConstString cstr( chs, slen );
	    w = fm.charWidth( cstr.string(), pos );
	}
    if ( letterSpacing )
	w += letterSpacing;

    if ( wordSpacing && (chs+pos)->category() == TQChar::Separator_Space )
		w += wordSpacing;
    return w;
}

/** Querying QFontDB whether something is scalable is expensive, so we cache. */
struct Font::ScalKey 
{
    TQString family;
    int     weight;
    int     italic;

    ScalKey(const TQFont& font) : 
            family(font.family()), weight(font.weight()), italic(font.italic())
    {}

    ScalKey() {}

    bool operator<(const ScalKey& other) const {
        if (weight < other.weight)
            return true;
        if (weight > other.weight)
            return false;

        if (italic < other.italic)
            return true;
        if (italic > other.italic)
            return false;

        return family < other.family;
    }

    bool operator==(const ScalKey& other) const {
        return (weight == other.weight &&
                italic == other.italic && 
                family == other.family);
    }
};

TQMap<Font::ScalKey, Font::ScalInfo>*   Font::scalCache;
TQMap<Font::ScalKey, TQValueList<int> >* Font::scalSizesCache;

bool Font::isFontScalable(TQFontDatabase& db, const TQFont& font) 
{
    if (!scalCache)
        scalCache = new TQMap<ScalKey, ScalInfo>;

    ScalKey key(font);

    ScalInfo &s = (*scalCache)[key];
    if (s == Unknown) {
        s = db.isSmoothlyScalable(font.family(), db.styleString(font)) ? Yes : No;

        if (s == No) {
            /* Cache size info */
            if (!scalSizesCache)
                scalSizesCache = new TQMap<ScalKey, TQValueList<int> >;
            (*scalSizesCache)[key] = db.smoothSizes(font.family(), db.styleString(font));
        }
    }

    return (s == Yes);
}


void Font::update( TQPaintDeviceMetrics* devMetrics ) const
{
    f.setFamily( fontDef.family.isEmpty() ? TDEHTMLFactory::defaultHTMLSettings()->stdFontName() : fontDef.family );
    f.setItalic( fontDef.italic );
    f.setWeight( fontDef.weight );

    TQFontDatabase db;

    int size = fontDef.size;
    const int lDpiY = kMax(devMetrics->logicalDpiY(), 96);

    // ok, now some magic to get a nice unscaled font
    // all other font properties should be set before this one!!!!
    if( !isFontScalable(db, f) )
    {
        const TQValueList<int>& pointSizes = (*scalSizesCache)[ScalKey(f)];
        // lets see if we find a nice looking font, which is not too far away
        // from the requested one.
        // kdDebug(6080) << "tdehtml::setFontSize family = " << f.family() << " size requested=" << size << endl;


        float diff = 1; // ### 100% deviation
        float bestSize = 0;

        TQValueList<int>::ConstIterator it = pointSizes.begin();
        const TQValueList<int>::ConstIterator itEnd = pointSizes.end();

        for( ; it != itEnd; ++it )
        {
            float newDiff = ((*it)*(lDpiY/72.) - float(size))/float(size);
            //kdDebug( 6080 ) << "smooth font size: " << *it << " diff=" << newDiff << endl;
            if(newDiff < 0) newDiff = -newDiff;
            if(newDiff < diff)
            {
                diff = newDiff;
                bestSize = *it;
            }
        }
        //kdDebug( 6080 ) << "best smooth font size: " << bestSize << " diff=" << diff << endl;
        if ( bestSize != 0 && diff < 0.2 ) // 20% deviation, otherwise we use a scaled font...
            size = (int)((bestSize*lDpiY) / 72);
    }

    // make sure we don't bust up X11
    // Also, Qt does not support sizing a TQFont to zero.
    size = kMax(1, kMin(255, size));

//       tqDebug("setting font to %s, italic=%d, weight=%d, size=%d", fontDef.family.latin1(), fontDef.italic,
//    	   fontDef.weight, size );


    f.setPixelSize( size );

    fm = TQFontMetrics( f );

    // small caps
    delete scFont;
    scFont = 0;

    if ( fontDef.smallCaps ) {
	scFont = new TQFont( f );
	scFont->setPixelSize( kMax(1, f.pixelSize()*7/10) );
    }
}

void Font::drawDecoration(TQPainter *pt, int _tx, int _ty, int baseline, int width, int height, int deco) const
{
    Q_UNUSED(height);
    // thick lines on small fonts look ugly
    const int thickness = fm.height() > 20 ? fm.lineWidth() : 1;
    const TQBrush brush = pt->pen().color();
    if (deco & UNDERLINE) {
        int underlineOffset = ( fm.height() + baseline ) / 2;
        if (underlineOffset <= baseline) underlineOffset = baseline+1;

        pt->fillRect(_tx, _ty + underlineOffset, width + 1, thickness, brush );
    }
    if (deco & OVERLINE) {
        pt->fillRect(_tx, _ty, width + 1, thickness, brush );
    }
    if (deco & LINE_THROUGH) {
        pt->fillRect(_tx, _ty + 2*baseline/3, width + 1, thickness, brush );
    }
}

