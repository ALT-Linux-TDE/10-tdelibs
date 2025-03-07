/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000-2003 Dirk Mueller (mueller@kde.org)
 *           (C) 2003 Apple Computer, Inc.
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
//#define DEBUG_LAYOUT

#include "render_image.h"
#include "render_canvas.h"

#include <tqdrawutil.h>
#include <tqpainter.h>

#include <tdeapplication.h>
#include <kdebug.h>
#include <tdeglobalsettings.h>

#include "css/csshelper.h"
#include "misc/helper.h"
#include "misc/htmlattrs.h"
#include "misc/loader.h"
#include "misc/htmltags.h"
#include "html/html_formimpl.h"
#include "html/html_imageimpl.h"
#include "html/dtd.h"
#include "xml/dom2_eventsimpl.h"
#include "html/html_documentimpl.h"
#include "html/html_objectimpl.h"
#include "tdehtmlview.h"
#include "tdehtml_part.h"
#include <math.h>

#include "loading_icon.cpp"

using namespace DOM;
using namespace tdehtml;

// -------------------------------------------------------------------------

RenderImage::RenderImage(NodeImpl *_element)
    : RenderReplaced(_element)
{
    m_oldImage = m_cachedImage = 0;

    m_selectionState = SelectionNone;
    berrorPic = false;

    const TDEHTMLSettings *settings = _element->getDocument()->view()->part()->settings();
    bUnfinishedImageFrame = settings->unfinishedImageFrame();

    setIntrinsicWidth( 0 );
    setIntrinsicHeight( 0 );
}

RenderImage::~RenderImage()
{
    if(m_cachedImage) m_cachedImage->deref( this );
    if(m_oldImage) m_oldImage->deref( this );
}

TQPixmap RenderImage::pixmap() const
{
    return m_cachedImage ? m_cachedImage->pixmap() : TQPixmap();
}

void RenderImage::setStyle(RenderStyle* _style)
{
    RenderReplaced::setStyle(_style);
    // init RenderObject attributes
    //setOverhangingContents(style()->height().isPercent());
    setShouldPaintBackgroundOrBorder(true);
}

void RenderImage::setContentObject(CachedObject* co )
{
    if (co && m_cachedImage != co)
        updateImage( static_cast<CachedImage*>( co ) );
}

void RenderImage::setPixmap( const TQPixmap &p, const TQRect& r, CachedImage *o)
{
    if ( o == m_oldImage )
        return;

    if(o != m_cachedImage) {
        RenderReplaced::setPixmap(p, r, o);
        return;
    }

    bool iwchanged = false;

    if(o->isErrorImage()) {
        int iw = p.width() + 8;
        int ih = p.height() + 8;

        // we have an alt and the user meant it (its not a text we invented)
        if ( element() && !alt.isEmpty() && !element()->getAttribute( ATTR_ALT ).isNull()) {
            const TQFontMetrics &fm = style()->fontMetrics();
            TQRect br = fm.boundingRect (  0, 0, 1024, 256, TQt::AlignAuto|TQt::WordBreak, alt.string() );
            if ( br.width() > iw )
                iw = br.width();
            if ( br.height() > ih )
                ih = br.height();
        }

        if ( iw != intrinsicWidth() ) {
            setIntrinsicWidth( iw );
            iwchanged = true;
        }
        if ( ih != intrinsicHeight() ) {
            setIntrinsicHeight( ih );
            iwchanged = true;
        }
        if ( element() && element()->id() == ID_OBJECT ) {
            static_cast<HTMLObjectElementImpl*>(  element() )->renderAlternative();
            return;
        }
    }
    berrorPic = o->isErrorImage();

    bool needlayout = false;

    // Image dimensions have been changed, see what needs to be done
    if( o->pixmap_size().width() != intrinsicWidth() ||
       o->pixmap_size().height() != intrinsicHeight() || iwchanged )
    {
//           tqDebug("image dimensions have been changed, old: %d/%d  new: %d/%d",
//                  intrinsicWidth(), intrinsicHeight(),
//               o->pixmap_size().width(), o->pixmap_size().height());

        if(!o->isErrorImage()) {
            setIntrinsicWidth( o->pixmap_size().width() );
            setIntrinsicHeight( o->pixmap_size().height() );
        }

         // lets see if we need to relayout at all..
         int oldwidth = m_width;
         int oldheight = m_height;
         int oldminwidth = m_minWidth;
         m_minWidth = 0;

         if ( parent() ) {
             calcWidth();
             calcHeight();
         }

         if(iwchanged || m_width != oldwidth || m_height != oldheight)
             needlayout = true;

         m_minWidth = oldminwidth;
         m_width = oldwidth;
         m_height = oldheight;
    }

    // we're not fully integrated in the tree yet.. we'll come back.
    if ( !parent() )
        return;

    if(needlayout)
    {
        if (!selfNeedsLayout())
            setNeedsLayout(true);
        if (minMaxKnown())
            setMinMaxKnown(false);
    }
    else
    {
        bool completeRepaint = !resizeCache.isNull();
        int cHeight = contentHeight();
        int scaledHeight = intrinsicHeight() ? ((o->valid_rect().height()*cHeight)/intrinsicHeight()) : 0;

        // don't bog down X server doing xforms
        if(completeRepaint && cHeight >= 5 &&  o->valid_rect().height() < intrinsicHeight() &&
           (scaledHeight / (cHeight/5) == resizeCache.height() / (cHeight/5)))
            return;

        resizeCache = TQPixmap(); // for resized animations

        if(completeRepaint)
            repaintRectangle(borderLeft()+paddingLeft(), borderTop()+paddingTop(), contentWidth(), contentHeight());
        else
        {
            repaintRectangle(r.x() + borderLeft() + paddingLeft(), r.y() + borderTop() + paddingTop(),
                             r.width(), r.height());
        }
    }
}

void RenderImage::paint(PaintInfo& paintInfo, int _tx, int _ty)
{
    if (paintInfo.phase == PaintActionOutline && style()->outlineWidth() && style()->visibility() == VISIBLE)
        paintOutline(paintInfo.p, _tx + m_x, _ty + m_y, width(), height(), style());

    if (paintInfo.phase != PaintActionForeground && paintInfo.phase != PaintActionSelection)
        return;

    // not visible or not even once layouted?
    if (style()->visibility() != VISIBLE || m_y <=  -500000)  return;

    _tx += m_x;
    _ty += m_y;

    if((_ty > paintInfo.r.bottom()) || (_ty + m_height <= paintInfo.r.top())) return;

    if(shouldPaintBackgroundOrBorder())
        paintBoxDecorations(paintInfo, _tx, _ty);

    int cWidth = contentWidth();
    int cHeight = contentHeight();
    int leftBorder = borderLeft();
    int topBorder = borderTop();
    int leftPad = paddingLeft();
    int topPad = paddingTop();

    if (!canvas()->printImages())
        return;

    CachedImage* i = m_oldImage && m_oldImage->valid_rect().size() == m_oldImage->pixmap_size() &&
                     m_oldImage->pixmap_size().width()  == intrinsicWidth() &&
                     m_oldImage->pixmap_size().height() == intrinsicHeight()
                     ? m_oldImage : m_cachedImage;

    // paint frame around image as long as it is not completely loaded from web.
    if (bUnfinishedImageFrame && paintInfo.phase == PaintActionForeground && cWidth > 2 && cHeight > 2 && !complete()) {
        static TQPixmap *loadingIcon;
        TQColor bg = tdehtml::retrieveBackgroundColor(this);
        TQColor fg = tdehtml::hasSufficientContrast(TQt::gray, bg) ? TQt::gray :
                    (hasSufficientContrast(TQt::white, bg) ? TQt::white : TQt::black);
	paintInfo.p->setPen(TQPen(fg, 1));
	paintInfo.p->setBrush( TQt::NoBrush );
	paintInfo.p->drawRect(_tx, _ty, m_width, m_height);
        if (!(m_width <= 5 || m_height <= 5)) {
            if (!loadingIcon) {
                loadingIcon = new TQPixmap();
                loadingIcon->loadFromData(loading_icon_data, loading_icon_len);
            }
            paintInfo.p->drawPixmap(_tx + 4, _ty + 4, *loadingIcon, 0, 0, m_width - 5, m_height - 5);
        }

    }

    //kdDebug( 6040 ) << "    contents (" << contentWidth << "/" << contentHeight << ") border=" << borderLeft() << " padding=" << paddingLeft() << endl;
    if ( !i || berrorPic)
    {
        if(cWidth > 2 && cHeight > 2)
        {
            if ( !berrorPic ) {
                //tqDebug("qDrawShadePanel %d/%d/%d/%d", _tx + leftBorder, _ty + topBorder, cWidth, cHeight);
                qDrawShadePanel( paintInfo.p, _tx + leftBorder + leftPad, _ty + topBorder + topPad, cWidth, cHeight,
                                 TDEApplication::palette().inactive(), true, 1 );
            }
            TQPixmap const* pix = i ? &i->pixmap() : 0;
            if(berrorPic && pix && (cWidth >= pix->width()+4) && (cHeight >= pix->height()+4) )
            {
                TQRect r(pix->rect());
                r = r.intersect(TQRect(0, 0, cWidth-4, cHeight-4));
                paintInfo.p->drawPixmap( TQPoint( _tx + leftBorder + leftPad+2, _ty + topBorder + topPad+2), *pix, r );
            }
            if(!alt.isEmpty()) {
                TQString text = alt.string();
                paintInfo.p->setFont(style()->font());
                paintInfo.p->setPen( style()->color() );
                int ax = _tx + leftBorder + leftPad + 2;
                int ay = _ty + topBorder + topPad + 2;
                const TQFontMetrics &fm = style()->fontMetrics();
                if (cWidth>5 && cHeight>=fm.height())
                    paintInfo.p->drawText(ax, ay+1, cWidth - 4, cHeight - 4, TQt::WordBreak, text );
            }
        }
    }
    else if (i && !i->isTransparent())
    {
        paintInfo.p->setPen( TQt::black ); // used for bitmaps
        const TQPixmap& pix = i->pixmap();
        if ( (cWidth != intrinsicWidth() ||  cHeight != intrinsicHeight()) &&
             pix.width() > 0 && pix.height() > 0 && i->valid_rect().isValid())
        {
            if (resizeCache.isNull() && cWidth && cHeight && intrinsicWidth() && intrinsicHeight())
            {
                TQRect scaledrect(i->valid_rect());
//                 kdDebug(6040) << "time elapsed: " << dt->elapsed() << endl;
//                  kdDebug( 6040 ) << "have to scale: " << endl;
//                  tqDebug("cw=%d ch=%d  pw=%d ph=%d  rcw=%d, rch=%d",
//                          cWidth, cHeight, intrinsicWidth(), intrinsicHeight(), resizeCache.width(), resizeCache.height());
                TQWMatrix matrix;
                matrix.scale( (float)(cWidth)/intrinsicWidth(),
                              (float)(cHeight)/intrinsicHeight() );
                resizeCache = pix.xForm( matrix );
                scaledrect.setWidth( ( cWidth*scaledrect.width() ) / intrinsicWidth() );
                scaledrect.setHeight( ( cHeight*scaledrect.height() ) / intrinsicHeight() );
//                   tqDebug("resizeCache size: %d/%d", resizeCache.width(), resizeCache.height());
//                   tqDebug("valid: %d/%d, scaled: %d/%d",
//                          i->valid_rect().width(), i->valid_rect().height(),
//                          scaledrect.width(), scaledrect.height());

                // sometimes scaledrect.width/height are off by one because
                // of rounding errors. if the i is fully loaded, we
                // make sure that we don't do unnecessary resizes during painting
                TQSize s(scaledrect.size());
                if(i->valid_rect().size() == TQSize( intrinsicWidth(), intrinsicHeight() )) // fully loaded
                    s = TQSize(cWidth, cHeight);
                if(kAbs(s.width() - cWidth) < 2) // rounding errors
                    s.setWidth(cWidth);
                if(resizeCache.size() != s)
                    resizeCache.resize(s);

                paintInfo.p->drawPixmap( TQPoint( _tx + leftBorder + leftPad, _ty + topBorder + topPad),
                               resizeCache, scaledrect );
            }
            else
                paintInfo.p->drawPixmap( TQPoint( _tx + leftBorder + leftPad, _ty + topBorder + topPad), resizeCache );
        }
        else
        {
            // we might be just about switching images
            TQRect rect(i->valid_rect().isValid() ? i->valid_rect()
                       : TQRect(0, 0, intrinsicWidth(), intrinsicHeight()));

            TQPoint offs( _tx + leftBorder + leftPad, _ty + topBorder + topPad);
//             tqDebug("normal paint rect %d/%d/%d/%d", rect.x(), rect.y(), rect.width(), rect.height());
//             rect = rect & TQRect( 0 , y - offs.y() - 10, w, 10 + y + h  - offs.y());

//             tqDebug("normal paint rect after %d/%d/%d/%d", rect.x(), rect.y(), rect.width(), rect.height());
//             tqDebug("normal paint: offs.y(): %d, y: %d, diff: %d", offs.y(), y, y - offs.y());
//             tqDebug("");

//           p->setClipRect(TQRect(x,y,w,h));


//             p->drawPixmap( offs.x(), y, pix, rect.x(), rect.y(), rect.width(), rect.height() );
             paintInfo.p->drawPixmap(offs, pix, rect);

        }
    }
    if (m_selectionState != SelectionNone) {
//    kdDebug(6040) << "_tx " << _tx << " _ty " << _ty << " _x " << _x << " _y " << _y << endl;
        // Draw in any case if inside selection. For selection borders, the
	// offset will decide whether to draw selection or not
	bool draw = true;
	if (m_selectionState != SelectionInside) {
	    int startPos, endPos;
            selectionStartEnd(startPos, endPos);
            if(selectionState() == SelectionStart)
                endPos = 1;
            else if(selectionState() == SelectionEnd)
                startPos = 0;
	    draw = endPos - startPos > 0;
	}
	if (draw) {
    	    // setting the brush origin is important for compatibility,
	    // don't touch it unless you know what you're doing
    	    paintInfo.p->setBrushOrigin(_tx, _ty - paintInfo.r.y());
            paintInfo.p->fillRect(_tx, _ty, width(), height(),
		    TQBrush(style()->palette().active().highlight(),
		    TQt::Dense4Pattern));
	}
    }
}

void RenderImage::layout()
{
    TDEHTMLAssert( needsLayout());
    TDEHTMLAssert( minMaxKnown() );

    short oldwidth = m_width;
    int oldheight = m_height;

    // minimum height
    m_height = m_cachedImage && m_cachedImage->isErrorImage() ? intrinsicHeight() : 0;

    calcWidth();
    calcHeight();

    // if they are variable width and we calculate a huge height or width, we assume they
    // actually wanted the intrinsic width.
    if ( m_width > 4096 && !style()->width().isFixed() )
	m_width = intrinsicWidth() + paddingLeft() + paddingRight() + borderLeft() + borderRight();
    if ( m_height > 2048 && !style()->height().isFixed() )
	m_height = intrinsicHeight() + paddingTop() + paddingBottom() + borderTop() + borderBottom();

    // limit total size to not run out of memory when doing the xform call.
    if ( ( m_width * m_height > 4096*2048 ) &&
         ( contentWidth() > intrinsicWidth() || contentHeight() > intrinsicHeight() ) ) {
	float scale = sqrt( m_width*m_height / ( 4096.*2048. ) );
	m_width = (int) (m_width/scale);
	m_height = (int) (m_height/scale);
    }

    if ( m_width != oldwidth || m_height != oldheight )
        resizeCache = TQPixmap();

    setNeedsLayout(false);
}

void RenderImage::notifyFinished(CachedObject *finishedObj)
{
    if ( ( m_cachedImage == finishedObj || m_oldImage == finishedObj ) && m_oldImage ) {
        m_oldImage->deref( this );
        m_oldImage = 0;
        repaint();
    }

    RenderReplaced::notifyFinished(finishedObj);
}

bool RenderImage::nodeAtPoint(NodeInfo& info, int _x, int _y, int _tx, int _ty, HitTestAction hitTestAction, bool inside)
{
    inside |= RenderReplaced::nodeAtPoint(info, _x, _y, _tx, _ty, hitTestAction, inside);

    if (inside && element()) {
        int tx = _tx + m_x;
        int ty = _ty + m_y;
        if (isRelPositioned())
            relativePositionOffset(tx, ty);

        HTMLImageElementImpl* i = element()->id() == ID_IMG ? static_cast<HTMLImageElementImpl*>(element()) : 0;
        HTMLMapElementImpl* map;
        if (i && i->getDocument()->isHTMLDocument() &&
            (map = static_cast<HTMLDocumentImpl*>(i->getDocument())->getMap(i->imageMap()))) {
            // we're a client side image map
            inside = map->mapMouseEvent(_x - tx, _y - ty, contentWidth(), contentHeight(), info);
            info.setInnerNonSharedNode(element());
        }
    }

    return inside;
}

void RenderImage::updateImage(CachedImage* new_image)
{
    CachedImage* tempimage = m_oldImage;
    m_oldImage = m_cachedImage;
    m_cachedImage = new_image;
    assert( m_cachedImage != m_oldImage );

    if ( m_cachedImage )
        m_cachedImage->ref(this);

    if ( tempimage )
        tempimage->deref(this);

    // if the loading finishes we might get an error and then the image is deleted
    if ( m_cachedImage )
        berrorPic = m_cachedImage->isErrorImage();
    else
        berrorPic = true;
}

void RenderImage::updateFromElement()
{
    if (element()->id() == ID_INPUT)
        alt = static_cast<HTMLInputElementImpl*>(element())->altText();
    else if (element()->id() == ID_IMG)
        alt = static_cast<HTMLImageElementImpl*>(element())->altText();

    DOMString u = element()->id() == ID_OBJECT ?
                  element()->getAttribute(ATTR_DATA) : element()->getAttribute(ATTR_SRC);

    if (!u.isEmpty() &&
        ( !m_cachedImage || m_cachedImage->url() != u ) ) {
        CachedImage *new_image = element()->getDocument()->docLoader()->
                                 requestImage(tdehtml::parseURL(u));

        if(new_image && new_image != m_cachedImage)
            updateImage( new_image );
    }
}

bool RenderImage::complete() const
{
     // "complete" means that the image has been loaded
     // but also that its width/height (contentWidth(),contentHeight()) have been calculated.
     return m_cachedImage && m_cachedImage->valid_rect().size() == m_cachedImage->pixmap_size() && !needsLayout();
}

bool RenderImage::isWidthSpecified() const
{
    switch (style()->width().type()) {
        case Fixed:
        case Percent:
            return true;
        default:
            return false;
    }
    assert(false);
    return false;
}

bool RenderImage::isHeightSpecified() const
{
    switch (style()->height().type()) {
        case Fixed:
        case Percent:
            return true;
        default:
            return false;
    }
    assert(false);
    return false;
}

short RenderImage::calcAspectRatioWidth() const
{
    if (intrinsicHeight() == 0)
        return 0;
    if (!m_cachedImage || m_cachedImage->isErrorImage())
        return intrinsicWidth(); // Don't bother scaling.
    return RenderReplaced::calcReplacedHeight() * intrinsicWidth() / intrinsicHeight();
}

int RenderImage::calcAspectRatioHeight() const
{
    if (intrinsicWidth() == 0)
        return 0;
    if (!m_cachedImage || m_cachedImage->isErrorImage())
        return intrinsicHeight(); // Don't bother scaling.
    return RenderReplaced::calcReplacedWidth() * intrinsicHeight() / intrinsicWidth();
}

short RenderImage::calcReplacedWidth() const
{
    int width;
    if (isWidthSpecified())
        width = calcReplacedWidthUsing(Width);
    else
        width = calcAspectRatioWidth();
    int minW = calcReplacedWidthUsing(MinWidth);
    int maxW = style()->maxWidth().value() == UNDEFINED ? width : calcReplacedWidthUsing(MaxWidth);

    if (width > maxW)
        width = maxW;

    if (width < minW)
        width = minW;

    return width;
}

int RenderImage::calcReplacedHeight() const
{
    int height;
    if (isHeightSpecified())
        height = calcReplacedHeightUsing(Height);
    else
        height = calcAspectRatioHeight();

    int minH = calcReplacedHeightUsing(MinHeight);
    int maxH = style()->maxHeight().value() == UNDEFINED ? height : calcReplacedHeightUsing(MaxHeight);

    if (height > maxH)
        height = maxH;

    if (height < minH)
        height = minH;

    return height;
}

#if 0
void RenderImage::caretPos(int offset, int flags, int &_x, int &_y, int &width, int &height)
{
    RenderReplaced::caretPos(offset, flags, _x, _y, width, height);

#if 0	// doesn't work reliably
    height = intrinsicHeight();
    width = override && offset == 0 ? intrinsicWidth() : 0;
    _x = xPos();
    _y = yPos();
    if (offset > 0) _x += intrinsicWidth();

    RenderObject *cb = containingBlock();

    int absx, absy;
    if (cb && cb != this && cb->absolutePosition(absx,absy))
    {
        _x += absx;
        _y += absy;
    } else {
        // we don't know our absolute position, and there is no point returning
        // just a relative one
        _x = _y = -1;
    }
#endif
}
#endif
