/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2000-2003 Dirk Mueller (mueller@kde.org)
 *           (C) 2003 Apple Computer, Inc.
 *           (C) 2004-2005 Allan Sandfeld Jensen (kde@carewolf.com)
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

// #define DEBUG_LAYOUT
// #define BIDI_DEBUG

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "rendering/render_text.h"
#include "rendering/render_canvas.h"
#include "rendering/break_lines.h"
#include "rendering/render_arena.h"
#include "xml/dom_nodeimpl.h"

#include "misc/loader.h"
#include "misc/helper.h"

#include <tqbitmap.h>
#include <tqimage.h>
#include <tqpainter.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <assert.h>
#include <limits.h>
#include <math.h>

#ifdef HAVE_ALLOCA_H
// explicitly included for systems that don't provide it in stdlib.h
#include <alloca.h>
#else
#include <stdlib.h>
#endif

using namespace tdehtml;
using namespace DOM;

#ifndef NDEBUG
static bool inInlineTextBoxDetach;
#endif

void InlineTextBox::detach(RenderArena* renderArena)
{
    if (m_parent)
        m_parent->removeFromLine(this);

#ifndef NDEBUG
    inInlineTextBoxDetach = true;
#endif
    delete this;
#ifndef NDEBUG
    inInlineTextBoxDetach = false;
#endif

    // Recover the size left there for us by operator delete and free the memory.
    renderArena->free(*(size_t *)this, this);
}

void* InlineTextBox::operator new(size_t sz, RenderArena* renderArena) throw()
{
    return renderArena->allocate(sz);
}

void InlineTextBox::operator delete(void* ptr, size_t sz)
{
    assert(inInlineTextBoxDetach);

    // Stash size where detach can find it.
    *(size_t *)ptr = sz;
}

void InlineTextBox::selectionStartEnd(int& sPos, int& ePos)
{
    int startPos, endPos;
    if (object()->selectionState() == RenderObject::SelectionInside) {
        startPos = 0;
        endPos = renderText()->string()->l;
    } else {
        renderText()->selectionStartEnd(startPos, endPos);
        if (object()->selectionState() == RenderObject::SelectionStart)
            endPos = renderText()->string()->l;
        else if (object()->selectionState() == RenderObject::SelectionEnd)
            startPos = 0;
    }

    sPos = kMax(startPos - m_start, 0);
    ePos = kMin(endPos - m_start, (int)m_len);
}

RenderObject::SelectionState InlineTextBox::selectionState()
{
    RenderObject::SelectionState state = object()->selectionState();
    if (state == RenderObject::SelectionStart || state == RenderObject::SelectionEnd ||
        state == RenderObject::SelectionBoth) {
        int startPos, endPos;
        renderText()->selectionStartEnd(startPos, endPos);
        
        bool start = (state != RenderObject::SelectionEnd && startPos >= m_start && startPos < m_start + m_len);
        bool end = (state != RenderObject::SelectionStart && endPos > m_start && endPos <= m_start + m_len);
        if (start && end)
            state = RenderObject::SelectionBoth;
        else if (start)
            state = RenderObject::SelectionStart;
        else if (end)
            state = RenderObject::SelectionEnd;
        else if ((state == RenderObject::SelectionEnd || startPos < m_start) &&
                 (state == RenderObject::SelectionStart || endPos > m_start + m_len))
            state = RenderObject::SelectionInside;
    }
    return state;
}

void InlineTextBox::paint(RenderObject::PaintInfo& i, int tx, int ty)
{
    if (object()->isBR() || object()->style()->visibility() != VISIBLE ||
        m_truncation == cFullTruncation || i.phase == PaintActionOutline)
        return;

    if (i.phase == PaintActionSelection && object()->selectionState() == RenderObject::SelectionNone)
        // When only painting the selection, don't bother to paint if there is none.
        return;

    int xPos = tx + m_x;
    int w = width();
    if ((xPos >= i.r.x() + i.r.width()) || (xPos + w <= i.r.x()))
        return;

    // Set our font.
    RenderStyle* styleToUse = object()->style(m_firstLine);
    int d = styleToUse->textDecorationsInEffect();
    if (styleToUse->font() != i.p->font())
        i.p->setFont(styleToUse->font());
    const Font *font = &styleToUse->htmlFont();
    bool haveSelection = selectionState() != RenderObject::SelectionNone;
    
    // Now calculate startPos and endPos, for painting selection.
    // We paint selection while endPos > 0
    int ePos = 0, sPos = 0;
    if (haveSelection && !object()->canvas()->staticMode()) {
        selectionStartEnd(sPos, ePos);
    }
    if (styleToUse->color() != i.p->pen().color())
        i.p->setPen(styleToUse->color());

    if (m_len > 0 && i.phase != PaintActionSelection) {
        int endPoint = m_len;
        if (m_truncation != cNoTruncation)
            endPoint = m_truncation - m_start;
        if (styleToUse->textShadow())
            paintShadow(i.p, font, tx, ty, styleToUse->textShadow());
        if (!haveSelection || sPos != 0 || ePos != m_len) {
            font->drawText(i.p, m_x + tx, m_y + ty + m_baseline, renderText()->string()->s, renderText()->string()->l, m_start, endPoint,
                           m_toAdd, m_reversed ? TQPainter::RTL : TQPainter::LTR);
        }
    }

    if (d != TDNONE && i.phase != PaintActionSelection && styleToUse->htmlHacks()) {
        i.p->setPen(styleToUse->color());
        paintDecoration(i.p, font, tx, ty, d);
    }

    if (haveSelection && i.phase == PaintActionSelection) {
        //kdDebug(6040) << this << " paintSelection with startPos=" << sPos << " endPos=" << ePos << endl;
        if ( sPos < ePos )
	    paintSelection(font, renderText(), i.p, styleToUse, tx, ty, sPos, ePos, d);
    }
}

/** returns the proper ::selection pseudo style for the given element
 * @return the style or 0 if no ::selection pseudo applies.
 */
inline const RenderStyle *retrieveSelectionPseudoStyle(const RenderObject *obj)
{
  // http://www.w3.org/Style/CSS/Test/CSS3/Selectors/20021129/html/tests/css3-modsel-162.html
  // is of the opinion that ::selection of parent elements is also to be applied
  // to children, so let's do it.
  while (obj) {
    const RenderStyle *style = obj->style()->getPseudoStyle(RenderStyle::SELECTION);
    if (style) return style;

    obj = obj->parent();
  }/*wend*/
  return 0;
}

void InlineTextBox::paintSelection(const Font *f, RenderText *text, TQPainter *p, RenderStyle* style, int tx, int ty, int startPos, int endPos, int deco)
{
    if(startPos > m_len) return;
    if(startPos < 0) startPos = 0;

    TQColor hc;
    TQColor hbg;
    const RenderStyle* pseudoStyle = retrieveSelectionPseudoStyle(text);
    if (pseudoStyle) {
        // ### support outline (mandated by CSS3)
	// ### support background-image? (optional by CSS3)
        if (pseudoStyle->backgroundColor().isValid())
            hbg = pseudoStyle->backgroundColor();
        hc = pseudoStyle->color();
    } else {
        const TQColorGroup &grp = style->palette().active();
        hc = grp.highlightedText();
        hbg = grp.highlight();
	// ### should be at most retrieved once per render text
	TQColor bg = tdehtml::retrieveBackgroundColor(text);
	// It may happen that the contrast is -- well -- virtually non existent.
	// In this case, simply swap the colors, thus in compliance with
	// NN4 (win32 only), IE, and Mozilla.
	if (!tdehtml::hasSufficientContrast(hbg, bg))
	    tqSwap(hc, hbg);
    }

    p->setPen(hc);

    //kdDebug( 6040 ) << "textRun::painting(" << TQConstString(text->str->s + m_start, m_len).string().left(30) << ") at(" << m_x+tx << "/" << m_y+ty << ")" << endl;

    const bool needClipping = startPos != 0 || endPos != m_len;

    if (needClipping) {
        p->save();

        int visualSelectionStart = f->width(text->str->s, text->str->l, m_start, startPos, m_start, m_start + m_len, m_toAdd);
        int visualSelectionEnd = f->width(text->str->s, text->str->l, m_start, endPos, m_start, m_start + m_len, m_toAdd);
        int visualSelectionWidth = visualSelectionEnd - visualSelectionStart;
        if (m_reversed) {
            visualSelectionStart = f->width(text->str->s, text->str->l, m_start, m_len) - visualSelectionEnd;
        }

        TQRect selectionRect(m_x + tx + visualSelectionStart, m_y + ty, visualSelectionWidth, height());
        TQRegion r(selectionRect);
        if (p->hasClipping())
            r &= p->clipRegion(TQPainter::CoordPainter);
        p->setClipRegion(r, TQPainter::CoordPainter);
    }

    f->drawText(p, m_x + tx, m_y + ty + m_baseline, text->str->s, text->str->l,
                m_start, m_len, m_toAdd,
                m_reversed ? TQPainter::RTL : TQPainter::LTR,
                needClipping ? 0 : startPos, needClipping ? m_len : endPos,
		hbg, m_y + ty, height(), deco);

    if (needClipping) p->restore();
}

void InlineTextBox::paintDecoration( TQPainter *pt, const Font *f, int _tx, int _ty, int deco)
{
    _tx += m_x;
    _ty += m_y;

    if (m_truncation == cFullTruncation)
        return;

    int width = m_width - 1;
    if (m_truncation != cNoTruncation) {
        width = static_cast<RenderText*>(m_object)->width(m_start, m_truncation - m_start, m_firstLine);
    }

    RenderObject *p = object();

    TQColor underline, overline, linethrough;
    p->getTextDecorationColors(deco, underline, overline, linethrough, p->style()->htmlHacks());

    if(deco & UNDERLINE){
        pt->setPen(underline);
        f->drawDecoration(pt, _tx, _ty, baseline(), width, height(), Font::UNDERLINE);
    }
    if (deco & OVERLINE) {
        pt->setPen(overline);
        f->drawDecoration(pt, _tx, _ty, baseline(), width, height(), Font::OVERLINE);
    }
    if(deco & LINE_THROUGH) {
        pt->setPen(linethrough);
        f->drawDecoration(pt, _tx, _ty, baseline(), width, height(), Font::LINE_THROUGH);
    }
    // NO! Do NOT add BLINK! It is the most annoying feature of Netscape, and IE has a reason not to
    // support it. Lars
}

void InlineTextBox::paintShadow(TQPainter *pt, const Font *f, int _tx, int _ty, const ShadowData *shadow )
{
    int x = m_x + _tx + shadow->x;
    int y = m_y + _ty + shadow->y;
    const RenderText* text = renderText();

    if (shadow->blur <= 0) {
        TQColor c = pt->pen().color();
        pt->setPen(shadow->color);
        f->drawText(pt, x, y+m_baseline, text->str->s, text->str->l,
                    m_start, m_len, m_toAdd,
                    m_reversed ? TQPainter::RTL : TQPainter::LTR);
        pt->setPen(c);

    }
    else {
        const int thickness = shadow->blur;
        const int w = m_width+2*thickness;
        const int h = m_height+2*thickness;
        const TQRgb color = shadow->color.rgb();
        const int gray = tqGray(color);
        const bool inverse = (gray < 100);
        const TQRgb bgColor = (inverse) ? tqRgb(255,255,255) : tqRgb(0,0,0);
        TQPixmap pixmap(w, h);
        pixmap.fill(bgColor);
        TQPainter p;

        p.begin(&pixmap);
        p.setPen(shadow->color);
        p.setFont(pt->font());
        f->drawText(&p, thickness, thickness+m_baseline, text->str->s, text->str->l,
                    m_start, m_len, m_toAdd,
                    m_reversed ? TQPainter::RTL : TQPainter::LTR);

        p.end();
        TQImage img = pixmap.convertToImage().convertDepth(32);

        int md = thickness*thickness; // max-dist^2

        // blur map (division cache)
        float *bmap = (float*)alloca(sizeof(float)*(md+1));
        for(int n=0; n<=md; n++) {
            float f;
            f = n/(float)(md+1);
            f = 1.0 - f*f;
            bmap[n] = f;
        }

        float factor = 0.0; // maximal potential opacity-sum
        for(int n=-thickness; n<=thickness; n++)
            for(int m=-thickness; m<=thickness; m++) {
                int d = n*n+m*m;
                if (d<=md)
                    factor += bmap[d];
            }

        // arbitratry factor adjustment to make shadows solid.
        factor = factor/1.333;

        // alpha map
        float* amap = (float*)alloca(sizeof(float)*(h*w));
        memset(amap, 0, h*w*(sizeof(float)));
        for(int j=thickness; j<h-thickness; j++) {
            for(int i=thickness; i<w-thickness; i++) {
                TQRgb col= img.pixel(i,j);
                if (col == bgColor) continue;
                float g = tqGray(col);
                if (inverse)
                    g = (255-g)/(255-gray);
                else
                    g = g/gray;
                for(int n=-thickness; n<=thickness; n++) {
                    for(int m=-thickness; m<=thickness; m++) {
                        int d = n*n+m*m;
                        if (d>md) continue;
                        float f = bmap[d];
                        amap[(i+m)+(j+n)*w] += (g*f);
                    }
                }
            }
        }

        TQImage res(w,h,32);
        res.setAlphaBuffer(true);
        int r = tqRed(color);
        int g = tqGreen(color);
        int b = tqBlue(color);

        // divide by factor
        factor = 1.0/factor;

        for(int j=0; j<h; j++) {
            for(int i=0; i<w; i++) {
                int a = (int)(amap[i+j*w] * factor * 255.0);
                if (a > 255) a = 255;
                res.setPixel(i,j, tqRgba(r,g,b,a));
            }
        }

        pt->drawImage(x-thickness, y-thickness, res, 0, 0, -1, -1, TQt::DiffuseAlphaDither | TQt::ColorOnly | TQt::PreferDither);
    }
    // Paint next shadow effect
    if (shadow->next) paintShadow(pt, f, _tx, _ty, shadow->next);
}

/**
 * Distributes pixels to justify text.
 * @param numSpaces spaces left, will be decremented by one
 * @param toAdd number of pixels left to be distributed, will have the
 *	amount of pixels distributed during this call subtracted.
 * @return number of pixels to distribute
 */
static inline int justifyWidth(int &numSpaces, int &toAdd) {
  int a = 0;
  if ( numSpaces ) {
    a = toAdd/numSpaces;
    toAdd -= a;
    numSpaces--;
  }/*end if*/
  return a;
}

FindSelectionResult InlineTextBox::checkSelectionPoint(int _x, int _y, int _tx, int _ty, const Font *f, RenderText *text, int & offset, short lineHeight)
{
//       kdDebug(6040) << "InlineTextBox::checkSelectionPoint " << this << " _x=" << _x << " _y=" << _y
//                     << " _tx+m_x=" << _tx+m_x << " _ty+m_y=" << _ty+m_y << endl;
    offset = 0;

    if ( _y < _ty + m_y )
        return SelectionPointBefore; // above -> before

    if ( _y > _ty + m_y + lineHeight ) {
        // below -> after
        // Set the offset to the max
        offset = m_len;
        return SelectionPointAfter;
    }
    if ( _x > _tx + m_x + m_width ) {
	// to the right
	return  SelectionPointAfterInLine;
    }

    // The Y matches, check if we're on the left
    if ( _x < _tx + m_x ) {
        return SelectionPointBeforeInLine;
    }

    // consider spacing for justified text
    int toAdd = m_toAdd;
    bool justified = text->style()->textAlign() == JUSTIFY && toAdd > 0;
    int numSpaces = 0;
    if (justified) {

        for( int i = 0; i < m_len; i++ )
            if ( text->str->s[m_start+i].category() == TQChar::Separator_Space )
	        numSpaces++;

    }/*end if*/

    int delta = _x - (_tx + m_x);
    //kdDebug(6040) << "InlineTextBox::checkSelectionPoint delta=" << delta << endl;
    int pos = 0;
    if ( m_reversed ) {
	delta -= m_width;
	while(pos < m_len) {
	    int w = f->width( text->str->s, text->str->l, m_start + pos);
	    if (justified && text->str->s[m_start + pos].category() == TQChar::Separator_Space)
	        w += justifyWidth(numSpaces, toAdd);
	    int w2 = w/2;
	    w -= w2;
	    delta += w2;
	    if(delta >= 0) break;
	    pos++;
	    delta += w;
	}
    } else {
	while(pos < m_len) {
	    int w = f->width( text->str->s, text->str->l, m_start + pos);
	    if (justified && text->str->s[m_start + pos].category() == TQChar::Separator_Space)
	        w += justifyWidth(numSpaces, toAdd);
	    int w2 = w/2;
	    w -= w2;
	    delta -= w2;
	    if(delta <= 0) break;
	    pos++;
	    delta -= w;
	}
    }
//     kdDebug( 6040 ) << " Text  --> inside at position " << pos << endl;
    offset = pos;
    return SelectionPointInside;
}

int InlineTextBox::offsetForPoint(int _x, int &ax) const
{
  // Do binary search for finding out offset, saves some time for long
  // runs.
  int start = 0;
  int end = m_len;
  ax = m_x;
  int offset = (start + end) / 2;
  while (end - start > 0) {
    // always snap to the right column. This makes up for "jumpy" vertical
    // navigation.
    if (end - start == 1) start = end;

    offset = (start + end) / 2;
    ax = m_x + widthFromStart(offset);
    if (ax > _x) end = offset;
    else if (ax < _x) start = offset;
    else break;
  }
  return m_start + offset;
}

int InlineTextBox::widthFromStart(int pos) const
{
  // gasp! sometimes pos is i < 0 which crashes Font::width
  pos = kMax(pos, 0);

  const RenderText *t = renderText();
  Q_ASSERT(t->isText());
  const Font *f = t->htmlFont(m_firstLine);
  const TQFontMetrics &fm = t->fontMetrics(m_firstLine);

  int numSpaces = 0;
  // consider spacing for justified text
  bool justified = t->style()->textAlign() == JUSTIFY;
  //kdDebug(6000) << "InlineTextBox::width(int)" << endl;
  if (justified && m_toAdd > 0) do {
    //kdDebug(6000) << "justify" << endl;

//    TQConstString cstr = TQConstString(t->str->s + m_start, m_len);
    for( int i = 0; i < m_len; i++ )
      if ( t->str->s[m_start+i].category() == TQChar::Separator_Space )
	numSpaces++;
    if (numSpaces == 0) break;

    int toAdd = m_toAdd;
    int w = 0;		// accumulated width
    int start = 0;	// start of non-space sequence
    int current = 0;	// current position
    while (current < pos) {
      // add spacing
      while (current < pos && t->str->s[m_start + current].category() == TQChar::Separator_Space) {
	w += f->getWordSpacing();
	w += f->getLetterSpacing();
	w += justifyWidth(numSpaces, toAdd);
        w += fm.width(' ');	// ### valid assumption? (LS)
        current++; start++;
      }/*wend*/
      if (current >= pos) break;

      // seek next space
      while (current < pos && t->str->s[m_start + current].category() != TQChar::Separator_Space)
        current++;

      // check run without spaces
      if ( current > start ) {
          w += f->width(t->str->s + m_start, m_len, start, current - start);
          start = current;
      }
    }

    return w;

  } while(false);/*end if*/

  //kdDebug(6000) << "default" << endl;
  // else use existing width function
  return f->width(t->str->s + m_start, m_len, 0, pos);

}

long InlineTextBox::minOffset() const
{
  return m_start;
}

long InlineTextBox::maxOffset() const
{
  return m_start + m_len;
}

int InlineTextBox::placeEllipsisBox(bool ltr, int blockEdge, int ellipsisWidth, bool& foundBox)
{
    if (foundBox) {
        m_truncation = cFullTruncation;
        return -1;
    }

    int ellipsisX = ltr ? blockEdge - ellipsisWidth : blockEdge + ellipsisWidth;

    // For LTR, if the left edge of the ellipsis is to the left of our text run, then we are the run that will get truncated.
    if (ltr) {
        if (ellipsisX <= m_x) {
            // Too far.  Just set full truncation, but return -1 and let the ellipsis just be placed at the edge of the box.
            m_truncation = cFullTruncation;
            foundBox = true;
            return -1;
        }

        if (ellipsisX < m_x + m_width) {
            if (m_reversed)
                return -1; // FIXME: Support LTR truncation when the last run is RTL someday.

            foundBox = true;

            int ax;
            int offset = offsetForPoint(ellipsisX, ax) - 1;
            if (offset <= m_start) {
                // No characters should be rendered.  Set ourselves to full truncation and place the ellipsis at the min of our start
                // and the ellipsis edge.
                m_truncation = cFullTruncation;
                return kMin(ellipsisX, (int)m_x);
            }

            // Set the truncation index on the text run.  The ellipsis needs to be placed just after the last visible character.
            m_truncation = offset;
            return widthFromStart(offset - m_start);
        }
    }
    else {
        // FIXME: Support RTL truncation someday, including both modes (when the leftmost run on the line is either RTL or LTR)
    }
    return -1;
}

// -----------------------------------------------------------------------------

InlineTextBoxArray::InlineTextBoxArray()
{
    setAutoDelete(false);
}

int InlineTextBoxArray::compareItems( Item d1, Item d2 )
{
    assert(d1);
    assert(d2);

    return static_cast<InlineTextBox*>(d1)->m_y - static_cast<InlineTextBox*>(d2)->m_y;
}

// remove this once TQPtrVector::bsearch is fixed
int InlineTextBoxArray::findFirstMatching(Item d) const
{
    int len = count();

    if ( !len )
	return -1;
    if ( !d )
	return -1;
    int n1 = 0;
    int n2 = len - 1;
    int mid = 0;
    bool found = false;
    while ( n1 <= n2 ) {
	int  res;
	mid = (n1 + n2)/2;
	if ( (*this)[mid] == 0 )			// null item greater
	    res = -1;
	else
	    res = ((TQGVector*)this)->compareItems( d, (*this)[mid] );
	if ( res < 0 )
	    n2 = mid - 1;
	else if ( res > 0 )
	    n1 = mid + 1;
	else {					// found it
	    found = true;
	    break;
	}
    }
    /* if ( !found )
	return -1; */
    // search to first one equal or bigger
    while ( found && (mid > 0) && !((TQGVector*)this)->compareItems(d, (*this)[mid-1]) )
	mid--;
    return mid;
}

// -------------------------------------------------------------------------------------

RenderText::RenderText(DOM::NodeImpl* node, DOMStringImpl *_str)
    : RenderObject(node)
{
    // init RenderObject attributes
    setRenderText();   // our object inherits from RenderText

    m_minWidth = -1;
    m_maxWidth = -1;
    str = _str;
    if(str) str->ref();
    TDEHTMLAssert(!str || !str->l || str->s);

    m_selectionState = SelectionNone;
    m_hasReturn = true;

#ifdef DEBUG_LAYOUT
    TQConstString cstr(str->s, str->l);
    kdDebug( 6040 ) << "RenderText ctr( "<< cstr.string().length() << " )  '" << cstr.string() << "'" << endl;
#endif
}

void RenderText::setStyle(RenderStyle *_style)
{
    if ( style() != _style ) {
        bool changedText = ((!style() && ( _style->textTransform() != TTNONE ||
                                           !_style->preserveLF() || !_style->preserveWS() )) ||
                            (style() && (style()->textTransform() != _style->textTransform() ||
                                         style()->whiteSpace() != _style->whiteSpace())));

        RenderObject::setStyle( _style );
        m_lineHeight = RenderObject::lineHeight(false);

        if (!isBR() && changedText) {
            DOM::DOMStringImpl* textToTransform = originalString();
            if (textToTransform)
                setText(textToTransform, true);
        }
    }
}

RenderText::~RenderText()
{
    TDEHTMLAssert(m_lines.count() == 0);
    if(str) str->deref();
}

void RenderText::deleteInlineBoxes(RenderArena* arena)
{
    // this is a slight variant of TQArray::clear().
    // We don't delete the array itself here because its
    // likely to be used in the same size later again, saves
    // us resize() calls
    unsigned int len = m_lines.size();
    if (len) {
        if (!arena)
            arena = renderArena();
        for(unsigned int i=0; i < len; i++) {
            InlineTextBox* s = m_lines.at(i);
            if (s)
                s->detach(arena);
            m_lines.remove(i);
        }
    }

    TDEHTMLAssert(m_lines.count() == 0);
}

bool RenderText::isTextFragment() const
{
    return false;
}

DOM::DOMStringImpl* RenderText::originalString() const
{
    return element() ? element()->string() : 0;
}

InlineTextBox * RenderText::findInlineTextBox( int offset, int &pos, bool checkFirstLetter )
{
    // The text boxes point to parts of the rendertext's str string
    // (they don't include '\n')
    // Find the text box that includes the character at @p offset
    // and return pos, which is the position of the char in the run.

    // FIXME: make this use binary search? Dirk says it won't work :-( (LS)
    (void)checkFirstLetter;
#if 0
    if (checkFirstLetter && forcedMinOffset()) {
//        kdDebug(6040) << "checkFirstLetter: forcedMinOffset: " << forcedMinOffset() << endl;
        RenderFlow *firstLetter = static_cast<RenderFlow *>(previousSibling());
        if (firstLetter && firstLetter->isFlow() && firstLetter->isFirstLetter()) {
            RenderText *letterText = static_cast<RenderText *>(firstLetter->firstChild());
            //kdDebug(6040) << "lettertext: " << letterText << " minOfs: " << letterText->minOffset() << " maxOfs: " << letterText->maxOffset() << endl;
	    if (offset >= letterText->minOffset() && offset <= letterText->maxOffset()) {
	        InlineTextBox *result = letterText->findInlineTextBox(offset, pos, false);
            //kdDebug(6040) << "result: " << result << endl;
		if (result) return result;
	    }
        }
    }
#endif

    if ( m_lines.isEmpty() )
        return 0L;

    // The text boxes don't resemble a contiguous coverage of the text, there
    // may be holes. Therefore, we snap to the nearest previous text box if
    // the given offset happens to point to such a hole.

    InlineTextBox* s = m_lines[0];
    uint count = m_lines.count();
    uint si = 0;
    uint nearest_idx = 0;		// index of nearest text box
    int nearest = INT_MAX;		// nearest distance
//kdDebug(6040) << "s[" << si << "] m_start " << s->m_start << " m_end " << (s->m_start + s->m_len) << endl;
    while(!(offset >= s->m_start && offset <= s->m_start + s->m_len)
    		&& ++si < count)
    {
        int dist = offset - (s->m_start + s->m_len);
//kdDebug(6040) << "dist " << dist << " nearest " << nearest << endl;
	if (dist >= 0 && dist <= nearest) {
	    nearest = dist;
	    nearest_idx = si - 1;
	}/*end if*/
        s = m_lines[si];
//kdDebug(6040) << "s[" << si << "] m_start " << s->m_start << " m_end " << (s->m_start + s->m_len) << endl;
    }
//kdDebug(6040) << "nearest_idx " << nearest_idx << " count " << count << endl;
    if (si >= count) s = m_lines[nearest_idx];
    // we are now in the correct text box
    pos = kMin(offset - s->m_start, int( s->m_len ));
    //kdDebug(6040) << "offset=" << offset << " s->m_start=" << s->m_start << endl;
    return s;
}

bool RenderText::nodeAtPoint(NodeInfo& info, int _x, int _y, int _tx, int _ty, HitTestAction /*hitTestAction*/, bool /*inBox*/)
{
    assert(parent());

    bool inside = false;
    if (style()->visibility() != HIDDEN) {
        InlineTextBox *s = m_lines.count() ? m_lines[0] : 0;
        int si = 0;
        while(s) {
            if((_y >=_ty + s->m_y) && (_y < _ty + s->m_y + s->m_height) &&
               (_x >= _tx + s->m_x) && (_x <_tx + s->m_x + s->m_width) ) {
                inside = true;
                break;
            }

            s = si < (int) m_lines.count()-1 ? m_lines[++si] : 0;
        }
    }

    // #### ported over from Safari. Can this happen at all? (lars)

    if (inside && element()) {
        if (info.innerNode() && info.innerNode()->renderer() &&
            !info.innerNode()->renderer()->isInline()) {
            // Within the same layer, inlines are ALWAYS fully above blocks.  Change inner node.
            info.setInnerNode(element());

            // Clear everything else.
            info.setInnerNonSharedNode(0);
            info.setURLElement(0);
        }

        if (!info.innerNode())
            info.setInnerNode(element());

        if(!info.innerNonSharedNode())
            info.setInnerNonSharedNode(element());
    }

    return inside;
}

FindSelectionResult RenderText::checkSelectionPoint(int _x, int _y, int _tx, int _ty, DOM::NodeImpl*& node, int &offset, SelPointState &)
{
//        kdDebug(6040) << "RenderText::checkSelectionPoint " << this << " _x=" << _x << " _y=" << _y
//                     << " _tx=" << _tx << " _ty=" << _ty << endl;
//kdDebug(6040) << renderName() << "::checkSelectionPoint x=" << xPos() << " y=" << yPos() << " w=" << width() << " h=" << height() << " m_lines.count=" << m_lines.count() << endl;

    NodeImpl *lastNode = 0;
    int lastOffset = 0;
    FindSelectionResult lastResult = SelectionPointAfter;

    for(unsigned int si = 0; si < m_lines.count(); si++)
    {
        InlineTextBox* s = m_lines[si];
        FindSelectionResult result;
        const Font *f = htmlFont( si==0 );
        result = s->checkSelectionPoint(_x, _y, _tx, _ty, f, this, offset, m_lineHeight);

//         kdDebug(6040) << "RenderText::checkSelectionPoint " << this << " line " << si << " result=" << result << " offset=" << offset << endl;
        if ( result == SelectionPointInside ) // x,y is inside the textrun
        {
            offset += s->m_start; // add the offset from the previous lines
//             kdDebug(6040) << "RenderText::checkSelectionPoint inside -> " << offset << endl;
            node = element();
            return SelectionPointInside;
        } else if ( result == SelectionPointBefore ) {
	    if (!lastNode) {
                // x,y is before the textrun -> stop here
               offset = 0;
//                kdDebug(6040) << "RenderText::checkSelectionPoint " << this << "before us -> returning Before" << endl;
               node = element();
               return SelectionPointBefore;
	    }
        } else if ( result == SelectionPointBeforeInLine ) {
	    offset = s->m_start;
	    node = element();
	    return SelectionPointInside;
        } else if ( result == SelectionPointAfterInLine ) {
	    lastOffset = s->m_start + s->m_len;
	    lastNode = element();
	    lastResult = result;
	    // no return here
	}

    }

    if (lastNode) {
        offset = lastOffset;
	node = lastNode;
//         kdDebug(6040) << "RenderText::checkSelectionPoint: lastNode " << lastNode << " lastOffset " << lastOffset << endl;
	return lastResult;
    }

    // set offset to max
    offset = str->l;
    //tqDebug("setting node to %p", element());
    node = element();
//     kdDebug(6040) << "RenderText::checkSelectionPoint: node " << node << " offset " << offset << endl;
    return SelectionPointAfter;
}

void RenderText::caretPos(int offset, int flags, int &_x, int &_y, int &width, int &height)
{
  if (!m_lines.count()) {
    _x = _y = height = -1;
    width = 1;
    return;
  }

  int pos;
  InlineTextBox * s = findInlineTextBox( offset, pos, true );
  RenderText *t = s->renderText();
//  kdDebug(6040) << "offset="<<offset << " pos="<<pos << endl;

  const TQFontMetrics &fm = t->metrics( s->m_firstLine );
  height = fm.height(); // s->m_height;

  _x = s->m_x + s->widthFromStart(pos);
  _y = s->m_y + s->baseline() - fm.ascent();
  width = 1;
  if (flags & CFOverride) {
    width = offset < maxOffset() ? fm.width(str->s[offset]) : 1;
  }/*end if*/
#if 0
  kdDebug(6040) << "_x="<<_x << " s->m_x="<<s->m_x
  		<< " s->m_start"<<s->m_start
		<< " s->m_len" << s->m_len << " _y=" << _y << endl;
#endif

  int absx, absy;

  if (absolutePosition(absx,absy))
  {
    //kdDebug(6040) << "absx=" << absx << " absy=" << absy << endl;
    _x += absx;
    _y += absy;
  } else {
    // we don't know our absolute position, and there is no point returning
    // just a relative one
    _x = _y = -1;
  }
}

long RenderText::minOffset() const
{
  if (!m_lines.count()) return 0;
  // FIXME: it is *not* guaranteed that the first run contains the lowest offset
  // Either make this a linear search (slow),
  // or maintain an index (needs much mem),
  // or calculate and store it in bidi.cpp (needs calculation even if not needed)
  // (LS)
  return m_lines[0]->m_start;
}

long RenderText::maxOffset() const
{
  int count = m_lines.count();
  if (!count) return str->l;
  // FIXME: it is *not* guaranteed that the last run contains the highest offset
  // Either make this a linear search (slow),
  // or maintain an index (needs much mem),
  // or calculate and store it in bidi.cpp (needs calculation even if not needed)
  // (LS)
  return m_lines[count - 1]->m_start + m_lines[count - 1]->m_len;
}

bool RenderText::absolutePosition(int &xPos, int &yPos, bool) const
{
    return RenderObject::absolutePosition(xPos, yPos, false);
}

bool RenderText::posOfChar(int chr, int &x, int &y)
{
    if (!parent())
        return false;
    parent()->absolutePosition( x, y, false );

    int pos;
    InlineTextBox * s = findInlineTextBox( chr, pos );

    if ( s ) {
        // s is the line containing the character
        x += s->m_x; // this is the x of the beginning of the line, but it's good enough for now
        y += s->m_y;
        return true;
    }

    return false;
}

void RenderText::paint( PaintInfo& /*pI*/, int /*tx*/, int /*ty*/)
{
    TDEHTMLAssert( false );
}

void RenderText::calcMinMaxWidth()
{
    TDEHTMLAssert( !minMaxKnown() );

    // ### calc Min and Max width...
    m_minWidth = m_beginMinWidth = m_endMinWidth = 0;
    m_maxWidth = 0;

    if (isBR())
        return;

    int currMinWidth = 0;
    int currMaxWidth = 0;
    m_hasBreakableChar = m_hasBreak = m_hasBeginWS = m_hasEndWS = false;

    // ### not 100% correct for first-line
    const Font *f = htmlFont( false );
    int wordSpacing = style()->wordSpacing();
    int len = str->l;
    bool isSpace = false;
    bool firstWord = true;
    bool firstLine = true;
    for(int i = 0; i < len; i++)
    {
        unsigned short c = str->s[i].unicode();
        bool isNewline = false;

        // If line-breaks survive to here they are preserved
        if ( c == '\n' ) {
            m_hasBreak = true;
            isNewline = true;
            isSpace = false;
        } else
            isSpace = c == ' ';

        if ((isSpace || isNewline) && i == 0)
            m_hasBeginWS = true;
        if ((isSpace || isNewline) && i == len-1)
            m_hasEndWS = true;

        if (i && c == SOFT_HYPHEN)
            continue;

        int wordlen = 0;
        while( i+wordlen < len && (i+wordlen == 0 || str->s[i+wordlen].unicode() != SOFT_HYPHEN) &&
               !(isBreakable( str->s, i+wordlen, str->l )) )
            wordlen++;

        if (wordlen)
        {
#ifndef APPLE_CHANGES
            int w = f->width(str->s, str->l, i, wordlen);
#else
            int w = widthFromCache(f, i, wordlen);
#endif
            currMinWidth += w;
            currMaxWidth += w;

            // Add in wordspacing to our maxwidth, but not if this is the last word.
            if (wordSpacing && !containsOnlyWhitespace(i+wordlen, len-(i+wordlen)))
                currMaxWidth += wordSpacing;

            if (firstWord) {
                firstWord = false;
                m_beginMinWidth = w;
            }
            m_endMinWidth = w;

            if(currMinWidth > m_minWidth) m_minWidth = currMinWidth;
            currMinWidth = 0;

            i += wordlen-1;
        }
        else {
            // Nowrap can never be broken, so don't bother setting the
            // breakable character boolean. Pre can only be broken if we encounter a newline.
            if (style()->autoWrap() || isNewline)
                m_hasBreakableChar = true;

            if(currMinWidth > m_minWidth) m_minWidth = currMinWidth;
            currMinWidth = 0;

            if (isNewline) // Only set if isPre was true and we saw a newline.
            {
                if ( firstLine ) {
                    firstLine = false;
                    m_beginMinWidth = currMaxWidth;
                }

                if(currMaxWidth > m_maxWidth) m_maxWidth = currMaxWidth;
                currMaxWidth = 0;
            }
            else
            {
                currMaxWidth += f->width( str->s, str->l, i + wordlen );
            }
        }
    }

    if(currMinWidth > m_minWidth) m_minWidth = currMinWidth;
    if(currMaxWidth > m_maxWidth) m_maxWidth = currMaxWidth;

    if (!style()->autoWrap()) {
        m_minWidth = m_maxWidth;
        if (style()->preserveLF()) {
            if (firstLine)
                m_beginMinWidth = m_maxWidth;
            m_endMinWidth = currMaxWidth;
        }
    }

    setMinMaxKnown();
    //kdDebug( 6040 ) << "Text::calcMinMaxWidth(): min = " << m_minWidth << " max = " << m_maxWidth << endl;

}

int RenderText::minXPos() const
{
    if (!m_lines.count())
	return 0;
    int retval=6666666;
    for (unsigned i=0;i < m_lines.count(); i++)
    {
	retval = kMin ( retval, int( m_lines[i]->m_x ));
    }
    return retval;
}

int RenderText::inlineXPos() const
{
    return minXPos();
}

int RenderText::inlineYPos() const
{
    return m_lines.isEmpty() ? 0 : m_lines[0]->yPos();
}

const TQFont &RenderText::font()
{
    return style()->font();
}

void RenderText::setText(DOMStringImpl *text, bool force)
{
    if( !force && str == text ) return;

    DOMStringImpl *oldstr = str;
    if(text && style())
        str = text->collapseWhiteSpace(style()->preserveLF(), style()->preserveWS());
    else
        str = text;
    if(str) str->ref();
    if(oldstr) oldstr->deref();

    if ( str && style() ) {
        oldstr = str;
        switch(style()->textTransform()) {
        case CAPITALIZE:
        {
            RenderObject *o;
            bool runOnString = false;

            // find previous non-empty text renderer if one exists
            for (o = previousRenderer(); o; o = o->previousRenderer()) {
                if (!o->isInlineFlow()) {
                    if (!o->isText())
                        break;

                    DOMStringImpl *prevStr = static_cast<RenderText*>(o)->string();
                    // !prevStr can happen with css like "content:open-quote;"
                    if (!prevStr)
                        break;

                    if (prevStr->length() == 0)
                        continue;
                    TQChar c = (*prevStr)[prevStr->length() - 1];
                    if (!c.isSpace())
                        runOnString = true;

                    break;
                }
            }

            str = str->capitalize(runOnString);
        }
        break;

		
		
        case UPPERCASE: str = str->upper();       break;
        case LOWERCASE: str = str->lower();       break;
        case NONE:
        default:;
    }
        str->ref();
        oldstr->deref();
    }

    // ### what should happen if we change the text of a
    // RenderBR object ?
    TDEHTMLAssert(!isBR() || (str->l == 1 && (*str->s) == '\n'));
    TDEHTMLAssert(!str->l || str->s);

    setNeedsLayoutAndMinMaxRecalc();
#ifdef BIDI_DEBUG
    TQConstString cstr(str->s, str->l);
    kdDebug( 6040 ) << "RenderText::setText( " << cstr.string().length() << " ) '" << cstr.string() << "'" << endl;
#endif
}

int RenderText::height() const
{
    int retval;
    if ( m_lines.count() )
        retval = m_lines[m_lines.count()-1]->m_y + m_lineHeight - m_lines[0]->m_y;
    else
        retval = metrics( false ).height();

    return retval;
}

short RenderText::lineHeight( bool firstLine ) const
{
    if ( firstLine )
 	return RenderObject::lineHeight( firstLine );

    return m_lineHeight;
}

short RenderText::baselinePosition( bool firstLine ) const
{
    const TQFontMetrics &fm = metrics( firstLine );
    return fm.ascent() +
        ( lineHeight( firstLine ) - fm.height() ) / 2;
}

InlineBox* RenderText::createInlineBox(bool, bool isRootLineBox)
{
    TDEHTMLAssert( !isRootLineBox );
    return new (renderArena()) InlineTextBox(this);
}

void RenderText::position(InlineBox* box, int from, int len, bool reverse)
{
//kdDebug(6040) << "position: from="<<from<<" len="<<len<<endl;
    // ### should not be needed!!!
    // asserts sometimes with pre (that unibw-hamburg testcase). ### find out why
    //TDEHTMLAssert(!(len == 0 || (str->l && len == 1 && *(str->s+from) == '\n') ));
    // It is now needed. BRs need text boxes too otherwise caret navigation
    // gets stuck (LS)
    // if (len == 0 || (str->l && len == 1 && *(str->s+from) == '\n') ) return;

    reverse = reverse && !style()->visuallyOrdered();

#ifdef DEBUG_LAYOUT
    TQChar *ch = str->s+from;
    TQConstString cstr(ch, len);
#endif

    TDEHTMLAssert(box->isInlineTextBox());
    InlineTextBox *s = static_cast<InlineTextBox *>(box);
    s->m_start = from;
    s->m_len = len;
    s->m_reversed = reverse;
    //kdDebug(6040) << "m_start: " << s->m_start << " m_len: " << s->m_len << endl;

    if(m_lines.count() == m_lines.size())
        m_lines.resize(m_lines.size()*2+1);

    m_lines.insert(m_lines.count(), s);
    //kdDebug(6040) << this << " " << renderName() << "::position inserted" << endl;
}

unsigned int RenderText::width(unsigned int from, unsigned int len, bool firstLine) const
{
    if(!str->s || from > str->l ) return 0;
    if ( from + len > str->l ) len = str->l - from;

    const Font *f = htmlFont( firstLine );
    return width( from, len, f );
}

unsigned int RenderText::width(unsigned int from, unsigned int len, const Font *f) const
{
    if(!str->s || from > str->l ) return 0;
    if ( from + len > str->l ) len = str->l - from;

    if ( f == &style()->htmlFont() && from == 0 && len == str->l )
 	 return m_maxWidth;

    int w = f->width(str->s, str->l, from, len );

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "RenderText::width(" << from << ", " << len << ") = " << w << endl;
#endif
    return w;
}

short RenderText::width() const
{
    int w;
    int minx = 100000000;
    int maxx = 0;
    // slooow
    for (unsigned int si = 0; si < m_lines.count(); si++) {
        InlineTextBox* s = m_lines[si];
        if (s->m_x < minx)
            minx = s->m_x;
        if (s->m_x + s->m_width > maxx)
            maxx = s->m_x + s->m_width;
    }

    w = kMax(0, maxx-minx);

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "RenderText::width() = " << w << endl;
#endif
    return w;
}

void RenderText::repaint(Priority p)
{
    RenderObject *cb = containingBlock();
    if(cb)
        cb->repaint(p);
}

bool RenderText::isFixedWidthFont() const
{
    return TQFontInfo(style()->font()).fixedPitch();
}

short RenderText::verticalPositionHint( bool firstLine ) const
{
    return parent()->verticalPositionHint( firstLine );
}

const TQFontMetrics &RenderText::metrics(bool firstLine) const
{
    if( firstLine && hasFirstLine() ) {
	RenderStyle *pseudoStyle  = style()->getPseudoStyle(RenderStyle::FIRST_LINE);
	if ( pseudoStyle )
	    return pseudoStyle->fontMetrics();
    }
    return style()->fontMetrics();
}

const Font *RenderText::htmlFont(bool firstLine) const
{
    const Font *f = 0;
    if( firstLine && hasFirstLine() ) {
	RenderStyle *pseudoStyle  = style()->getPseudoStyle(RenderStyle::FIRST_LINE);
	if ( pseudoStyle )
	    f = &pseudoStyle->htmlFont();
    } else {
	f = &style()->htmlFont();
    }
    return f;
}

bool RenderText::containsOnlyWhitespace(unsigned int from, unsigned int len) const
{
    unsigned int currPos;
    for (currPos = from;
         currPos < from+len && (str->s[currPos] == '\n' || str->s[currPos].direction() == TQChar::DirWS);
         currPos++);
    return currPos >= (from+len);
}

void RenderText::trimmedMinMaxWidth(short& beginMinW, bool& beginWS,
                                    short& endMinW, bool& endWS,
                                    bool& hasBreakableChar, bool& hasBreak,
                                    short& beginMaxW, short& endMaxW,
                                    short& minW, short& maxW, bool& stripFrontSpaces)
{
    bool preserveWS = style()->preserveWS();
    bool preserveLF = style()->preserveLF();
    bool autoWrap = style()->autoWrap();
    if (preserveWS)
        stripFrontSpaces = false;

    int len = str->l;
    if (len == 0 || (stripFrontSpaces && str->containsOnlyWhitespace())) {
        maxW = 0;
        hasBreak = false;
        return;
    }

    minW = m_minWidth;
    maxW = m_maxWidth;
    beginWS = stripFrontSpaces ? false : m_hasBeginWS;
    endWS = m_hasEndWS;

    beginMinW = m_beginMinWidth;
    endMinW = m_endMinWidth;

    hasBreakableChar = m_hasBreakableChar;
    hasBreak = m_hasBreak;

    if (stripFrontSpaces && (str->s[0].direction() == TQChar::DirWS || (!preserveLF && str->s[0] == '\n'))) {
        const Font *f = htmlFont( false );
        TQChar space[1]; space[0] = ' ';
        int spaceWidth = f->width(space, 1, 0);
        maxW -= spaceWidth;
    }

    stripFrontSpaces = !preserveWS && m_hasEndWS;

    if (!autoWrap)
        minW = maxW;
    else if (minW > maxW)
        minW = maxW;

    // Compute our max widths by scanning the string for newlines.
    if (hasBreak) {
        const Font *f = htmlFont( false );
        bool firstLine = true;
        beginMaxW = endMaxW = maxW;
        for(int i = 0; i < len; i++)
        {
            int linelen = 0;
            while( i+linelen < len && str->s[i+linelen] != '\n')
                linelen++;

            if (linelen)
            {
#ifndef APPLE_CHANGES
                endMaxW = f->width(str->s, str->l, i, linelen);
#else
                endMaxW = widthFromCache(f, i, linelen);
#endif
                if (firstLine) {
                    firstLine = false;
                    beginMaxW = endMaxW;
                }
                i += linelen;
            }
            else if (firstLine) {
                beginMaxW = 0;
                firstLine = false;
            }
	    if (i == len-1)
	        // A <pre> run that ends with a newline, as in, e.g.,
	        // <pre>Some text\n\n<span>More text</pre>
	        endMaxW = 0;
        }
    }
}

#ifdef ENABLE_DUMP

static TQString quoteAndEscapeNonPrintables(const TQString &s)
{
    TQString result;
    result += '"';
    for (uint i = 0; i != s.length(); ++i) {
        TQChar c = s.at(i);
        if (c == '\\') {
            result += "\\\\";
        } else if (c == '"') {
            result += "\\\"";
        } else {
            ushort u = c.unicode();
            if (u >= 0x20 && u < 0x7F) {
                result += c;
            } else {
                TQString hex;
                hex.sprintf("\\x{%X}", u);
                result += hex;
            }
        }
    }
    result += '"';
    return result;
}

static void writeTextRun(TQTextStream &ts, const RenderText &o, const InlineTextBox &run)
{
    ts << "text run at (" << run.m_x << "," << run.m_y << ") width " << run.width() << ": "
       << quoteAndEscapeNonPrintables(o.data().string().mid(run.m_start, run.m_len));
}

void RenderText::dump(TQTextStream &stream, const TQString &ind) const
{
    RenderObject::dump( stream, ind );

    for (unsigned int i = 0; i < m_lines.count(); i++) {
        stream << endl << ind << "   ";
        writeTextRun(stream, *this, *m_lines[i]);
    }
}
#endif

RenderTextFragment::RenderTextFragment(DOM::NodeImpl* _node, DOM::DOMStringImpl* _str,
                                       int startOffset, int endOffset)
:RenderText(_node, _str->substring(startOffset, endOffset)),
m_start(startOffset), m_end(endOffset), m_generatedContentStr(0)
{}

RenderTextFragment::RenderTextFragment(DOM::NodeImpl* _node, DOM::DOMStringImpl* _str)
:RenderText(_node, _str), m_start(0)
{
    m_generatedContentStr = _str;
    if (_str) {
        _str->ref();
        m_end = _str->l;
    }
    else
        m_end = 0;
}

RenderTextFragment::~RenderTextFragment()
{
    if (m_generatedContentStr)
        m_generatedContentStr->deref();
}

bool RenderTextFragment::isTextFragment() const
{
    return true;
}

DOM::DOMStringImpl* RenderTextFragment::originalString() const
{
    DOM::DOMStringImpl* result = 0;
    if (element())
        result = element()->string();
    else
        result = contentString();
    if (result && (start() > 0 || start() < result->l))
        result = result->substring(start(), end());
    return result;
}

#undef BIDI_DEBUG
#undef DEBUG_LAYOUT
