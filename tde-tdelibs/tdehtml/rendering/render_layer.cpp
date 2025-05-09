/*
 * Copyright (C) 2003 Apple Computer, Inc.
 *           (C) 2006 Germain Garand <germain@ebooksfrance.org>
 *           (C) 2006 Allan Sandfeld Jense <kde@carewolf.com>
 *
 * Portions are Copyright (C) 1998 Netscape Communications Corporation.
 *
 * Other contributors:
 *   Robert O'Callahan <roc+@cs.cmu.edu>
 *   David Baron <dbaron@fas.harvard.edu>
 *   Christian Biesinger <cbiesinger@web.de>
 *   Randall Jesup <rjesup@wgate.com>
 *   Roland Mainz <roland.mainz@informatik.med.uni-giessen.de>
 *   Josh Soref <timeless@mac.com>
 *   Boris Zbarsky <bzbarsky@mit.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Alternatively, the contents of this file may be used under the terms
 * of either the Mozilla Public License Version 1.1, found at
 * http://www.mozilla.org/MPL/ (the "MPL") or the GNU General Public
 * License Version 2.0, found at http://www.fsf.org/copyleft/gpl.html
 * (the "GPL"), in which case the provisions of the MPL or the GPL are
 * applicable instead of those above.  If you wish to allow use of your
 * version of this file only under the terms of one of those two
 * licenses (the MPL or the GPL) and not to allow others to use your
 * version of this file under the LGPL, indicate your decision by
 * deletingthe provisions above and replace them with the notice and
 * other provisions required by the MPL or the GPL, as the case may be.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under any of the LGPL, the MPL or the GPL.
 */

//#define BOX_DEBUG

#include "render_layer.h"
#include <kdebug.h>
#include <assert.h>
#include "tdehtmlview.h"
#include "render_canvas.h"
#include "render_arena.h"
#include "render_replaced.h"
#include "xml/dom_docimpl.h"
#include "xml/dom2_eventsimpl.h"
#include "misc/htmltags.h"
#include "html/html_blockimpl.h"
#include "xml/dom_restyler.h"

#include <tqscrollbar.h>
#include <tqptrvector.h>
#include <tqstyle.h>

using namespace DOM;
using namespace tdehtml;

#ifdef APPLE_CHANGES
TQScrollBar* RenderLayer::gScrollBar = 0;
#endif

#ifndef NDEBUG
static bool inRenderLayerDetach;
#endif

void
RenderScrollMediator::slotValueChanged()
{
    m_layer->updateScrollPositionFromScrollbars();
}

RenderLayer::RenderLayer(RenderObject* object)
: m_object( object ),
m_parent( 0 ),
m_previous( 0 ),
m_next( 0 ),
m_first( 0 ),
m_last( 0 ),
m_x( 0 ),
m_y( 0 ),
m_scrollX( 0 ),
m_scrollY( 0 ),
m_scrollWidth( 0 ),
m_scrollHeight( 0 ),
m_hBar( 0 ),
m_vBar( 0 ),
m_scrollMediator( 0 ),
m_posZOrderList( 0 ),
m_negZOrderList( 0 ),
m_overflowList(0),
m_zOrderListsDirty( true ),
m_overflowListDirty(true),
m_isOverflowOnly( shouldBeOverflowOnly() ),
m_markedForRepaint( false ),
m_hasOverlaidWidgets( false ),
m_marquee( 0 )
{
}

RenderLayer::~RenderLayer()
{
    // Child layers will be deleted by their corresponding render objects, so
    // our destructor doesn't have to do anything.
    delete m_hBar;
    delete m_vBar;
    delete m_scrollMediator;
    delete m_posZOrderList;
    delete m_negZOrderList;
    delete m_overflowList;
    delete m_marquee;
}

void RenderLayer::updateLayerPosition()
{

    // The canvas is sized to the docWidth/Height over in RenderCanvas::layout, so we
    // don't need to ever update our layer position here.
    if (renderer()->isCanvas())
        return;

    int x = m_object->xPos();
    int y = m_object->yPos() - m_object->borderTopExtra();

    if (!m_object->isPositioned()) {
        // We must adjust our position by walking up the render tree looking for the
        // nearest enclosing object with a layer.
        RenderObject* curr = m_object->parent();
        while (curr && !curr->layer()) {
            x += curr->xPos();
            y += curr->yPos();
            curr = curr->parent();
        }
        if (curr)
          y += curr->borderTopExtra();
    }

    if (m_object->isRelPositioned())
        static_cast<RenderBox*>(m_object)->relativePositionOffset(x, y);

    // Subtract our parent's scroll offset.
    if (m_object->isPositioned() && enclosingPositionedAncestor()) {
        RenderLayer* positionedParent = enclosingPositionedAncestor();

        // For positioned layers, we subtract out the enclosing positioned layer's scroll offset.
        positionedParent->subtractScrollOffset(x, y);
        positionedParent->checkInlineRelOffset(m_object, x, y);
    }
    else if (parent())
        parent()->subtractScrollOffset(x, y);

    setPos(x,y);
}

TQRegion RenderLayer::paintedRegion(RenderLayer* rootLayer)
{
    updateZOrderLists();
    TQRegion r;
    if (m_negZOrderList) {
        uint count = m_negZOrderList->count();
        for (uint i = 0; i < count; i++) {
            RenderLayer* child = m_negZOrderList->at(i);
            r += child->paintedRegion(rootLayer);
        }
    }
    const RenderStyle *s= renderer()->style();
    if (s->visibility() == VISIBLE) {
        int x = 0; int y = 0;
        convertToLayerCoords(rootLayer,x,y);
        TQRect cr(x,y,width(),height());
        if ( s->backgroundImage() || s->backgroundColor().isValid() || s->hasBorder() ||
             renderer()->scrollsOverflow() || renderer()->isReplaced() ) {
            r += cr;
        } else {
            r += renderer()->visibleFlowRegion(x, y);
        }
    }

    if (m_posZOrderList) {
        uint count = m_posZOrderList->count();
        for (uint i = 0; i < count; i++) {
            RenderLayer* child = m_posZOrderList->at(i);
            r += child->paintedRegion(rootLayer);
        }
    }
    return r;
}

void RenderLayer::repaint( Priority p, bool markForRepaint )
{
    if (markForRepaint && m_markedForRepaint)
        return;
    for (RenderLayer* child = firstChild(); child; child = child->nextSibling())
        child->repaint( p, markForRepaint );
    TQRect layerBounds, damageRect, fgrect;
    calculateRects(renderer()->canvas()->layer(), renderer()->viewRect(), layerBounds, damageRect, fgrect);
    m_visibleRect = damageRect.intersect( layerBounds );
    if (m_visibleRect.isValid())
        renderer()->canvas()->repaintViewRectangle( m_visibleRect.x(), m_visibleRect.y(), m_visibleRect.width(), m_visibleRect.height(), (p > NormalPriority) );
    if (markForRepaint)
        m_markedForRepaint = true;
}

void RenderLayer::updateLayerPositions(RenderLayer* rootLayer, bool doFullRepaint, bool checkForRepaint)
{
    if (doFullRepaint) {
        m_object->repaint();
        checkForRepaint = doFullRepaint = false;
    }

    updateLayerPosition(); // For relpositioned layers or non-positioned layers,
                           // we need to keep in sync, since we may have shifted relative
                           // to our parent layer.

    if (m_hBar || m_vBar) {
        // Need to position the scrollbars.
        int x = 0;
        int y = 0;
        convertToLayerCoords(rootLayer, x, y);
        TQRect layerBounds = TQRect(x,y,width(),height());
        positionScrollbars(layerBounds);
    }

#ifdef APPLE_CHANGES
    // FIXME: Child object could override visibility.
    if (checkForRepaint && (m_object->style()->visibility() == VISIBLE))
        m_object->repaintAfterLayoutIfNeeded(m_repaintRect, m_fullRepaintRect);
#else
    if (checkForRepaint && m_markedForRepaint) {
        TQRect layerBounds, damageRect, fgrect;
        calculateRects(rootLayer, renderer()->viewRect(), layerBounds, damageRect, fgrect);
        TQRect vr = damageRect.intersect( layerBounds );
        if (vr != m_visibleRect && vr.isValid()) {
            renderer()->canvas()->repaintViewRectangle( vr.x(), vr.y(), vr.width(), vr.height() );
            m_visibleRect = vr;
        }
    }
    m_markedForRepaint = false;
#endif

    for	(RenderLayer* child = firstChild(); child; child = child->nextSibling())
        child->updateLayerPositions(rootLayer, doFullRepaint, checkForRepaint);

    // With all our children positioned, now update our marquee if we need to.
    if (m_marquee)
        m_marquee->updateMarqueePosition();
}

void RenderLayer::updateWidgetMasks(RenderLayer* rootLayer)
{
    if (hasOverlaidWidgets() && !renderer()->canvas()->pagedMode()) {
        updateZOrderLists();
        uint count = m_posZOrderList ? m_posZOrderList->count() : 0;
        bool needUpdate = (count || !m_region.isNull());
        if (count) {
            TQScrollView* sv = m_object->document()->view();
            m_region = TQRect(0,0,sv->contentsWidth(),sv->contentsHeight());

            for (uint i = 0; i < count; i++) {
                RenderLayer* child = m_posZOrderList->at(i);
                if (child->zIndex() == 0 && child->renderer()->style()->position() == STATIC)
                    continue; // we don't know the widget's exact stacking position within flow
                m_region -= child->paintedRegion(rootLayer);
            }
        } else {
            m_region = TQRegion();
        }
        if (needUpdate)
            renderer()->updateWidgetMasks();
    }
    for	(RenderLayer* child = firstChild(); child; child = child->nextSibling())
        child->updateWidgetMasks(rootLayer);
}

short RenderLayer::width() const
{
    int w = m_object->width();
    if (!m_object->hasOverflowClip())
        w = kMax(m_object->overflowWidth(), w);
    return w;
}

int RenderLayer::height() const
{
    int h = m_object->height() + m_object->borderTopExtra() + m_object->borderBottomExtra();
    if (!m_object->hasOverflowClip())
        h = kMax(m_object->overflowHeight(), h);
    return h;
}


RenderLayer *RenderLayer::stackingContext() const
{
    RenderLayer* curr = parent();
    for ( ; curr && !curr->m_object->isCanvas() &&
          curr->m_object->style()->hasAutoZIndex();
          curr = curr->parent());
    return curr;
}

RenderLayer* RenderLayer::enclosingPositionedAncestor() const
{
    RenderLayer* curr = parent();
    for ( ; curr && !curr->m_object->isCanvas() &&
         !curr->m_object->isPositioned() && !curr->m_object->isRelPositioned();
         curr = curr->parent());

    return curr;
}

#ifdef APPLE_CHANGES
bool RenderLayer::isTransparent()
{
    return m_object->style()->opacity() < 1.0f;
}

RenderLayer* RenderLayer::transparentAncestor()
{
    RenderLayer* curr = parent();
    for ( ; curr && curr->m_object->style()->opacity() == 1.0f; curr = curr->parent());
    return curr;
}
#endif

void* RenderLayer::operator new(size_t sz, RenderArena* renderArena) throw()
{
    return renderArena->allocate(sz);
}

void RenderLayer::operator delete(void* ptr, size_t sz)
{
    assert(inRenderLayerDetach);

    // Stash size where detach can find it.
    *(size_t *)ptr = sz;
}

void RenderLayer::detach(RenderArena* renderArena)
{
#ifndef NDEBUG
    inRenderLayerDetach = true;
#endif
    delete this;
#ifndef NDEBUG
    inRenderLayerDetach = false;
#endif

    // Recover the size left there for us by operator delete and free the memory.
    renderArena->free(*(size_t *)this, this);
}

void RenderLayer::addChild(RenderLayer *child, RenderLayer* beforeChild)
{
    RenderLayer* prevSibling = beforeChild ? beforeChild->previousSibling() : lastChild();
    if (prevSibling) {
        child->setPreviousSibling(prevSibling);
        prevSibling->setNextSibling(child);
    }
    else
        setFirstChild(child);

    if (beforeChild) {
        beforeChild->setPreviousSibling(child);
        child->setNextSibling(beforeChild);
    }
    else
        setLastChild(child);

    child->setParent(this);

    if (child->isOverflowOnly())
        dirtyOverflowList();
    else {
        // Dirty the z-order list in which we are contained.  The stackingContext() can be null in the
        // case where we're building up generated content layers.  This is ok, since the lists will start
        // off dirty in that case anyway.
        RenderLayer* stackingContext = child->stackingContext();
        if (stackingContext)
            stackingContext->dirtyZOrderLists();
    }
}

RenderLayer* RenderLayer::removeChild(RenderLayer* oldChild)
{
    // remove the child
    if (oldChild->previousSibling())
        oldChild->previousSibling()->setNextSibling(oldChild->nextSibling());
    if (oldChild->nextSibling())
        oldChild->nextSibling()->setPreviousSibling(oldChild->previousSibling());

    if (m_first == oldChild)
        m_first = oldChild->nextSibling();
    if (m_last == oldChild)
        m_last = oldChild->previousSibling();

    if (oldChild->isOverflowOnly())
        dirtyOverflowList();
    else {
        // Dirty the z-order list in which we are contained.  When called via the
        // reattachment process in removeOnlyThisLayer, the layer may already be disconnected
        // from the main layer tree, so we need to null-check the |stackingContext| value.
        RenderLayer* stackingContext = oldChild->stackingContext();
        if (stackingContext)
            stackingContext->dirtyZOrderLists();
    }

    oldChild->setPreviousSibling(0);
    oldChild->setNextSibling(0);
    oldChild->setParent(0);

    return oldChild;
}

void RenderLayer::removeOnlyThisLayer()
{
    if (!m_parent)
        return;

    // Remove us from the parent.
    RenderLayer* parent = m_parent;
    RenderLayer* nextSib = nextSibling();
    parent->removeChild(this);

    // Now walk our kids and reattach them to our parent.
    RenderLayer* current = m_first;
    while (current) {
        RenderLayer* next = current->nextSibling();
        removeChild(current);
        parent->addChild(current, nextSib);
        current = next;
    }

    detach(renderer()->renderArena());
}

void RenderLayer::insertOnlyThisLayer()
{
    if (!m_parent && renderer()->parent()) {
        // We need to connect ourselves when our renderer() has a parent.
        // Find our enclosingLayer and add ourselves.
        RenderLayer* parentLayer = renderer()->parent()->enclosingLayer();
        if (parentLayer)
            parentLayer->addChild(this,
                                  renderer()->parent()->findNextLayer(parentLayer, renderer()));
    }

    // Remove all descendant layers from the hierarchy and add them to the new position.
    for (RenderObject* curr = renderer()->firstChild(); curr; curr = curr->nextSibling())
        curr->moveLayers(m_parent, this);
}

void RenderLayer::convertToLayerCoords(const RenderLayer* ancestorLayer, int& x, int& y) const
{
    if (ancestorLayer == this)
        return;

    if (m_object->style()->position() == FIXED) {
        // Add in the offset of the view.  We can obtain this by calling
        // absolutePosition() on the RenderCanvas.
        int xOff, yOff;
        m_object->absolutePosition(xOff, yOff, true);
        x += xOff;
        y += yOff;
        return;
    }

    RenderLayer* parentLayer;
    if (m_object->style()->position() == ABSOLUTE)
        parentLayer = enclosingPositionedAncestor();
    else
        parentLayer = parent();

    if (!parentLayer) return;

    parentLayer->convertToLayerCoords(ancestorLayer, x, y);

    x += xPos();
    y += yPos();
}

void RenderLayer::scrollOffset(int& x, int& y)
{
    x += scrollXOffset();
    y += scrollYOffset();
}

void RenderLayer::subtractScrollOffset(int& x, int& y)
{
    x -= scrollXOffset();
    y -= scrollYOffset();
}

void RenderLayer::checkInlineRelOffset(const RenderObject* o, int& x, int& y)
{
    if(o->style()->position() != ABSOLUTE || !renderer()->isRelPositioned() || !renderer()->isInlineFlow())
        return;

    // Our renderer is an enclosing relpositioned inline, we need to add in the offset of the first line
    // box from the rest of the content, but only in the cases where we know our descendant is positioned
    // relative to the inline itself.
    assert( o->container() == m_object );

    RenderFlow* flow = static_cast<RenderFlow*>(m_object);
    int sx = 0, sy = 0;
    if (flow->firstLineBox()) {
        if (flow->style()->direction() == LTR)
            sx = flow->firstLineBox()->xPos();
        else
            sx = flow->lastLineBox()->xPos();
        sy = flow->firstLineBox()->yPos();
    } else {
        sx = flow->staticX(); // ###
        sy = flow->staticY();
    }
    bool isInlineType = o->style()->isOriginalDisplayInlineType();

    if (!o->hasStaticX())
        x += sx;

    // Despite the positioned child being a block display type inside an inline, we still keep
    // its x locked to our left.  Arguably the correct behavior would be to go flush left to
    // the block that contains us, but that isn't what other browsers do.
    if (o->hasStaticX() && !isInlineType)
        // Avoid adding in the left border/padding of the containing block twice.  Subtract it out.
        x += sx - (o->containingBlock()->borderLeft() + o->containingBlock()->paddingLeft());

    if (!o->hasStaticY())
        y += sy;
}

void RenderLayer::scrollToOffset(int x, int y, bool updateScrollbars, bool repaint)
{
    if (renderer()->style()->overflowX() != OMARQUEE || !renderer()->hasOverflowClip()) {
        if (x < 0) x = 0;
        if (y < 0) y = 0;

        // Call the scrollWidth/Height functions so that the dimensions will be computed if they need
        // to be (for overflow:hidden blocks).
        // ### merge the scrollWidth()/scrollHeight() methods
        int maxX = m_scrollWidth - m_object->clientWidth();
        int maxY = m_scrollHeight - m_object->clientHeight();

        if (x > maxX) x = maxX;
        if (y > maxY) y = maxY;
    }

    // FIXME: Eventually, we will want to perform a blit.  For now never
    // blit, since the check for blitting is going to be very
    // complicated (since it will involve testing whether our layer
    // is either occluded by another layer or clipped by an enclosing
    // layer or contains fixed backgrounds, etc.).
    m_scrollX = x;
    m_scrollY = y;

    // Update the positions of our child layers.
    RenderLayer* rootLayer = root();
    for (RenderLayer* child = firstChild(); child; child = child->nextSibling())
        child->updateLayerPositions(rootLayer);

    // Fire the scroll DOM event.
    m_object->element()->dispatchHTMLEvent(EventImpl::SCROLL_EVENT, true, false);

    // Just schedule a full repaint of our object.
    if (repaint)
        m_object->repaint(RealtimePriority);

    if (updateScrollbars) {
        if (m_hBar)
            m_hBar->setValue(m_scrollX);
        if (m_vBar)
            m_vBar->setValue(m_scrollY);
    }
}

void RenderLayer::updateScrollPositionFromScrollbars()
{
    bool needUpdate = false;
    int newX = m_scrollX;
    int newY = m_scrollY;

    if (m_hBar) {
        newX = m_hBar->value();
        if (newX != m_scrollX)
           needUpdate = true;
    }

    if (m_vBar) {
        newY = m_vBar->value();
        if (newY != m_scrollY)
           needUpdate = true;
    }

    if (needUpdate)
        scrollToOffset(newX, newY, false);
}

void
RenderLayer::showScrollbar(TQt::Orientation o, bool show)
{
    TQScrollBar *sb = (o == TQt::Horizontal) ? m_hBar : m_vBar;

    if (show && !sb) {
        TQScrollView* scrollView = m_object->document()->view();
        sb = new TQScrollBar(o, scrollView, "__tdehtml");
        scrollView->addChild(sb, 0, -50000);
	sb->setBackgroundMode(TQWidget::NoBackground);
        sb->show();
        if (!m_scrollMediator)
            m_scrollMediator = new RenderScrollMediator(this);
        m_scrollMediator->connect(sb, TQ_SIGNAL(valueChanged(int)), TQ_SLOT(slotValueChanged()));
    }
    else if (!show && sb) {
        delete sb;
        sb = 0;
    }

    if (o == TQt::Horizontal)
	m_hBar = sb;
    else
	m_vBar = sb;
}

int RenderLayer::verticalScrollbarWidth()
{
    if (!m_vBar)
        return 0;

#ifdef APPLE_CHANGES
    return m_vBar->width();
#else
    return m_vBar->style().pixelMetric(TQStyle::PM_ScrollBarExtent);
#endif

}

int RenderLayer::horizontalScrollbarHeight()
{
    if (!m_hBar)
        return 0;

#ifdef APPLE_CHANGES
    return m_hBar->height();
#else
    return m_hBar->style().pixelMetric(TQStyle::PM_ScrollBarExtent);
#endif

}

void RenderLayer::positionScrollbars(const TQRect& absBounds)
{
#ifdef APPLE_CHANGES
    if (m_vBar) {
        scrollView->addChild(m_vBar, absBounds.x()+absBounds.width()-m_object->borderRight()-m_vBar->width(),
                     absBounds.y()+m_object->borderTop());
        m_vBar->resize(m_vBar->width(), absBounds.height() -
                       (m_object->borderTop()+m_object->borderBottom()) -
                       (m_hBar ? m_hBar->height()-1 : 0));
    }

    if (m_hBar) {
        scrollView->addChild(m_hBar, absBounds.x()+m_object->borderLeft(),
                     absBounds.y()+absBounds.height()-m_object->borderBottom()-m_hBar->height());
        m_hBar->resize(absBounds.width() - (m_object->borderLeft()+m_object->borderRight()) -
                       (m_vBar ? m_vBar->width()-1 : 0), m_hBar->height());
    }
#else
    int tx = absBounds.x();
    int ty = absBounds.y();
    int bl = m_object->borderLeft();
    int bt = m_object->borderTop();
    int w = width() - bl - m_object->borderRight();
    int h = height() - bt - m_object->borderBottom();

    if (w <= 0 || h <= 0 || (!m_vBar && !m_hBar))
	return;

    TQScrollView* scrollView = m_object->document()->view();

    tx += bl;
    ty += bt;

    TQScrollBar *b = m_hBar;
    if (!m_hBar)
	b = m_vBar;
    int sw = b->style().pixelMetric(TQStyle::PM_ScrollBarExtent);

    if (m_vBar) {
	TQRect vBarRect = TQRect(tx + w - sw + 1, ty, sw, h - (m_hBar ? sw : 0) + 1);
        m_vBar->resize(vBarRect.width(), vBarRect.height());
        scrollView->addChild(m_vBar, vBarRect.x(), vBarRect.y());
    }

    if (m_hBar) {
	TQRect hBarRect = TQRect(tx, ty + h - sw + 1, w - (m_vBar ? sw : 0) + 1, sw);
        m_hBar->resize(hBarRect.width(), hBarRect.height());
        scrollView->addChild(m_hBar, hBarRect.x(), hBarRect.y());
    }
#endif
}

#define LINE_STEP   10
#define PAGE_KEEP   40

void RenderLayer::checkScrollbarsAfterLayout()
{
    int rightPos = m_object->rightmostPosition(true);
    int bottomPos = m_object->lowestPosition(true);

/*  TODO
    m_scrollLeft = m_object->leftmostPosition(true);
    m_scrollTop = m_object->highestPosition(true);
*/

    int clientWidth = m_object->clientWidth();
    int clientHeight = m_object->clientHeight();
    m_scrollWidth = clientWidth;
    m_scrollHeight = clientHeight;

    if (rightPos - m_object->borderLeft() > m_scrollWidth)
        m_scrollWidth = rightPos - m_object->borderLeft();
    if (bottomPos - m_object->borderTop() > m_scrollHeight)
        m_scrollHeight = bottomPos - m_object->borderTop();

    bool needHorizontalBar = rightPos > width();
    bool needVerticalBar = bottomPos > height();

    bool haveHorizontalBar = m_hBar && m_hBar->isEnabled();
    bool haveVerticalBar = m_vBar && m_vBar->isEnabled();
    
    bool hasOvf = m_object->hasOverflowClip();

    // overflow:scroll should just enable/disable.
    if (hasOvf && m_object->style()->overflowX() == OSCROLL)
        m_hBar->setEnabled(needHorizontalBar);
    if (hasOvf && m_object->style()->overflowY() == OSCROLL)
        m_vBar->setEnabled(needVerticalBar);

    // overflow:auto may need to lay out again if scrollbars got added/removed.
    bool scrollbarsChanged = (hasOvf && m_object->style()->overflowX() == OAUTO && haveHorizontalBar != needHorizontalBar)
                          || (hasOvf && m_object->style()->overflowY() == OAUTO && haveVerticalBar != needVerticalBar);
    if (scrollbarsChanged) {
        if (m_object->style()->overflowX() == OAUTO) {
            showScrollbar(TQt::Horizontal, needHorizontalBar);
            if (m_hBar) 
                m_hBar->setEnabled(true);
        }
        if (m_object->style()->overflowY() == OAUTO) {
            showScrollbar(TQt::Vertical, needVerticalBar);
            if (m_vBar)
                m_vBar->setEnabled(true);
        }

        m_object->setNeedsLayout(true);
	if (m_object->isRenderBlock())
            static_cast<RenderBlock*>(m_object)->layoutBlock(true);
        else
            m_object->layout();
	return;
    }

    // Set up the range (and page step/line step).
    if (m_hBar) {
        int pageStep = (clientWidth-PAGE_KEEP);
        if (pageStep < 0) pageStep = clientWidth;
        m_hBar->setSteps(LINE_STEP, pageStep);
#ifdef APPLE_CHANGES
        m_hBar->setKnobProportion(clientWidth, m_scrollWidth);
#else
        m_hBar->setRange(0, needHorizontalBar ? m_scrollWidth-clientWidth : 0);
#endif
    }
    if (m_vBar) {
        int pageStep = (clientHeight-PAGE_KEEP);
        if (pageStep < 0) pageStep = clientHeight;
        m_vBar->setSteps(LINE_STEP, pageStep);
#ifdef APPLE_CHANGES
        m_vBar->setKnobProportion(clientHeight, m_scrollHeight);
#else
        m_vBar->setRange(0, needVerticalBar ? m_scrollHeight-clientHeight : 0);
#endif
    }
}

void RenderLayer::paintScrollbars(RenderObject::PaintInfo& pI)
{
#ifdef APPLE_CHANGES
    if (m_hBar)
        m_hBar->paint(p, damageRect);
    if (m_vBar)
        m_vBar->paint(p, damageRect);
#else
    if (!m_object->element())
       return;

    TQScrollView* scrollView = m_object->document()->view();
    if (m_hBar) {
	int x = m_hBar->x();
	int y = m_hBar->y();
	scrollView->viewportToContents(x, y, x, y);
	RenderWidget::paintWidget(pI, m_hBar, x, y);
    }
    if (m_vBar) {
	int x = m_vBar->x();
	int y = m_vBar->y();
	scrollView->viewportToContents(x, y, x, y);
	RenderWidget::paintWidget(pI, m_vBar, x, y);
    }
#endif
}

void RenderLayer::paint(TQPainter *p, const TQRect& damageRect, bool selectionOnly)
{
    paintLayer(this, p, damageRect, selectionOnly);
}

static void setClip(TQPainter* p, const TQRect& paintDirtyRect, const TQRect& clipRect)
{
    if (paintDirtyRect == clipRect)
        return;
    p->save();

#ifdef APPLE_CHANGES
    p->addClip(clipRect);
#else

    TQRect clippedRect = p->xForm(clipRect);
    TQRegion creg(clippedRect);
    TQRegion old = p->clipRegion();
    if (!old.isNull())
        creg = old.intersect(creg);
    p->setClipRegion(creg);
#endif

}

static void restoreClip(TQPainter* p, const TQRect& paintDirtyRect, const TQRect& clipRect)
{
    if (paintDirtyRect == clipRect)
        return;
    p->restore();
}

void RenderLayer::paintLayer(RenderLayer* rootLayer, TQPainter *p,
                        const TQRect& paintDirtyRect, bool selectionOnly)
{
    // Calculate the clip rects we should use.
    TQRect layerBounds, damageRect, clipRectToApply;
    calculateRects(rootLayer, paintDirtyRect, layerBounds, damageRect, clipRectToApply);
    int x = layerBounds.x();
    int y = layerBounds.y();

    // Ensure our lists are up-to-date.
    updateZOrderLists();
    updateOverflowList();

#ifdef APPLE_CHANGES
    // Set our transparency if we need to.
    if (isTransparent())
        p->beginTransparencyLayer(renderer()->style()->opacity());
#endif

    // We want to paint our layer, but only if we intersect the damage rect.
    bool shouldPaint = intersectsDamageRect(layerBounds, damageRect);
    if (shouldPaint && !selectionOnly) {
        // Paint our background first, before painting any child layers.
        if (!damageRect.isEmpty()) {
            // Establish the clip used to paint our background.
            setClip(p, paintDirtyRect, damageRect);

            // Paint the background.
            RenderObject::PaintInfo paintInfo(p, damageRect, PaintActionElementBackground);
            renderer()->paint(paintInfo,
                              x - renderer()->xPos(), y - renderer()->yPos() + renderer()->borderTopExtra());

            // Position our scrollbars.
            positionScrollbars(layerBounds);

            // Our scrollbar widgets paint exactly when we tell them to, so that they work properly with
            // z-index.  We paint after we painted the background/border, so that the scrollbars will
            // sit above the background/border.
            paintScrollbars(paintInfo);

            // Restore the clip.
            restoreClip(p, paintDirtyRect, damageRect);
        }
    }

    // Now walk the sorted list of children with negative z-indices.
    if (m_negZOrderList) {
        uint count = m_negZOrderList->count();
        for (uint i = 0; i < count; i++) {
            RenderLayer* child = m_negZOrderList->at(i);
            child->paintLayer(rootLayer, p, paintDirtyRect, selectionOnly);
        }
    }

    // Now establish the appropriate clip and paint our child RenderObjects.
    if (shouldPaint && !clipRectToApply.isEmpty()) {
        // Set up the clip used when painting our children.
        setClip(p, paintDirtyRect, clipRectToApply);

        RenderObject::PaintInfo paintInfo(p, clipRectToApply, PaintActionSelection);

        int tx = x - renderer()->xPos();
        int ty = y - renderer()->yPos() + renderer()->borderTopExtra();

        if (selectionOnly)
            renderer()->paint(paintInfo, tx, ty);
        else {
            paintInfo.phase = PaintActionChildBackgrounds;
            renderer()->paint(paintInfo, tx, ty);
            paintInfo.phase = PaintActionFloat;
            renderer()->paint(paintInfo, tx, ty);
            paintInfo.phase = PaintActionForeground;
            renderer()->paint(paintInfo, tx, ty);
            RenderCanvas *rc = static_cast<RenderCanvas*>(renderer()->document()->renderer());
            if (rc->maximalOutlineSize()) {
                paintInfo.phase = PaintActionOutline;
                renderer()->paint(paintInfo, tx, ty);
            }
            if (rc->selectionStart() && rc->selectionEnd()) {
                paintInfo.phase = PaintActionSelection;
                renderer()->paint(paintInfo, tx, ty);
            }
        }

        // Now restore our clip.
        restoreClip(p, paintDirtyRect, clipRectToApply);
    }

    // Paint any child layers that have overflow.
    if (m_overflowList)
        for (TQValueList<RenderLayer*>::iterator it = m_overflowList->begin(); it != m_overflowList->end(); ++it)
            (*it)->paintLayer(rootLayer, p, paintDirtyRect, selectionOnly);

    // Now walk the sorted list of children with positive z-indices.
    if (m_posZOrderList) {
        uint count = m_posZOrderList->count();
        for (uint i = 0; i < count; i++) {
            RenderLayer* child = m_posZOrderList->at(i);
            child->paintLayer(rootLayer, p, paintDirtyRect, selectionOnly);
        }
    }

#ifdef BOX_DEBUG
    {
        int ax=0;
        int ay=0;
        renderer()->absolutePosition( ax, ay );
        p->setPen(TQPen(TQColor("yellow"), 1, TQt::DotLine));
        p->setBrush( TQt::NoBrush );
        p->drawRect(ax, ay, width(), height());
    }
#endif

#ifdef APPLE_CHANGES
    // End our transparency layer
    if (isTransparent())
        p->endTransparencyLayer();
#endif
}

bool RenderLayer::nodeAtPoint(RenderObject::NodeInfo& info, int x, int y)
{
#ifdef APPLE_CHANGES
    // Clear our our scrollbar variable
    RenderLayer::gScrollBar = 0;
#endif

    int stx = m_x;
    int sty = m_y;

#ifdef __GNUC__
#warning HACK
#endif
    if (renderer()->isCanvas()) {
        stx += static_cast<RenderCanvas*>(renderer())->view()->contentsX();
        sty += static_cast<RenderCanvas*>(renderer())->view()->contentsY();
    }

    TQRect damageRect(stx,sty, width(), height());
    RenderLayer* insideLayer = nodeAtPointForLayer(this, info, x, y, damageRect);

    // Now determine if the result is inside an anchor.
    DOM::NodeImpl* node = info.innerNode();
    while (node) {
        if (node->hasAnchor() && !info.URLElement())
            info.setURLElement(node);
        node = node->parentNode();
    }

    // Next set up the correct :hover/:active state along the new chain.
    updateHoverActiveState(info);

    // Now return whether we were inside this layer (this will always be true for the root
    // layer).
    return insideLayer;
}

RenderLayer* RenderLayer::nodeAtPointForLayer(RenderLayer* rootLayer, RenderObject::NodeInfo& info,
                                 int xMousePos, int yMousePos, const TQRect& hitTestRect)
{
    // Calculate the clip rects we should use.
    TQRect layerBounds, bgRect, fgRect;
    calculateRects(rootLayer, hitTestRect, layerBounds, bgRect, fgRect);

    // Ensure our lists are up-to-date.
    updateZOrderLists();
    updateOverflowList();

    // This variable tracks which layer the mouse ends up being inside.  The minute we find an insideLayer,
    // we are done and can return it.
    RenderLayer* insideLayer = 0;

    // Begin by walking our list of positive layers from highest z-index down to the lowest
    // z-index.
    if (m_posZOrderList) {
        uint count = m_posZOrderList->count();
        for (int i = count-1; i >= 0; i--) {
            RenderLayer* child = m_posZOrderList->at(i);
            insideLayer = child->nodeAtPointForLayer(rootLayer, info, xMousePos, yMousePos, hitTestRect);
            if (insideLayer)
                return insideLayer;
        }
    }

    // Now check our overflow objects.
    if (m_overflowList) {
        TQValueList<RenderLayer*>::iterator it = m_overflowList->end();
        for (--it; it != m_overflowList->end(); --it) {
            insideLayer = (*it)->nodeAtPointForLayer(rootLayer, info,  xMousePos, yMousePos, hitTestRect);
            if (insideLayer)
                return insideLayer;
        }
    }

    // Next we want to see if the mouse pos is inside the child RenderObjects of the layer.
    if (containsPoint(xMousePos, yMousePos, fgRect) &&
        renderer()->nodeAtPoint(info, xMousePos, yMousePos,
                            layerBounds.x() - renderer()->xPos(),
                            layerBounds.y() - renderer()->yPos() + m_object->borderTopExtra(),
                                HitTestChildrenOnly)) {
	if (info.innerNode() != m_object->element())
	    return this;
    }

    // Now check our negative z-index children.
    if (m_negZOrderList) {
        uint count = m_negZOrderList->count();
        for (int i = count-1; i >= 0; i--) {
            RenderLayer* child = m_negZOrderList->at(i);
            insideLayer = child->nodeAtPointForLayer(rootLayer, info, xMousePos, yMousePos, hitTestRect);
            if (insideLayer)
                return insideLayer;
        }
    }

    // Next we want to see if the mouse pos is inside this layer but not any of its children.
    if (containsPoint(xMousePos, yMousePos, bgRect) &&
        renderer()->nodeAtPoint(info, xMousePos, yMousePos,
                                layerBounds.x() - renderer()->xPos(),
                                layerBounds.y() - renderer()->yPos() + m_object->borderTopExtra(),
                                HitTestSelfOnly))
        return this;

    // No luck.
    return 0;
}

void RenderLayer::calculateClipRects(const RenderLayer* rootLayer, TQRect& overflowClipRect,
                                     TQRect& posClipRect, TQRect& fixedClipRect)
{
    if (parent())
        parent()->calculateClipRects(rootLayer, overflowClipRect, posClipRect, fixedClipRect);

    switch (m_object->style()->position()) {
      // A fixed object is essentially the root of its containing block hierarchy, so when
      // we encounter such an object, we reset our clip rects to the fixedClipRect.
      case FIXED:
         posClipRect = fixedClipRect;
         overflowClipRect = fixedClipRect;
        break;
      case ABSOLUTE:
        overflowClipRect = posClipRect;
        break;
      case RELATIVE:
        posClipRect = overflowClipRect;
        break;
      default:
        break;
    }

    // Update the clip rects that will be passed to child layers.
    if (m_object->hasOverflowClip() || m_object->hasClip()) {
        // This layer establishes a clip of some kind.
        int x = 0;
        int y = 0;
        convertToLayerCoords(rootLayer, x, y);

        if (m_object->hasOverflowClip()) {
            TQRect newOverflowClip = m_object->getOverflowClipRect(x,y);
            overflowClipRect  = newOverflowClip.intersect(overflowClipRect);
            if (m_object->isPositioned() || m_object->isRelPositioned())
                posClipRect = newOverflowClip.intersect(posClipRect);
        }
        if (m_object->hasClip()) {
            TQRect newPosClip = m_object->getClipRect(x,y);
            posClipRect = posClipRect.intersect(newPosClip);
            overflowClipRect = overflowClipRect.intersect(newPosClip);
            fixedClipRect = fixedClipRect.intersect(newPosClip);
        }
    }
}

void RenderLayer::calculateRects(const RenderLayer* rootLayer, const TQRect& paintDirtyRect, TQRect& layerBounds,
                                 TQRect& backgroundRect, TQRect& foregroundRect)
{
    TQRect overflowClipRect = paintDirtyRect;
    TQRect posClipRect = paintDirtyRect;
    TQRect fixedClipRect = paintDirtyRect;
    if (parent())
        parent()->calculateClipRects(rootLayer, overflowClipRect, posClipRect, fixedClipRect);

    int x = 0;
    int y = 0;
    convertToLayerCoords(rootLayer, x, y);
    layerBounds = TQRect(x,y,width(),height());

    backgroundRect = m_object->style()->position() == FIXED ? fixedClipRect :
        (m_object->isPositioned() ? posClipRect : overflowClipRect);
    foregroundRect = backgroundRect;

    // Update the clip rects that will be passed to child layers.
    if (m_object->hasOverflowClip() || m_object->hasClip()) {
        // This layer establishes a clip of some kind.
        if (m_object->hasOverflowClip())
            foregroundRect = foregroundRect.intersect(m_object->getOverflowClipRect(x,y));

        if (m_object->hasClip()) {
            // Clip applies to *us* as well, so go ahead and update the damageRect.
            TQRect newPosClip = m_object->getClipRect(x,y);
            backgroundRect = backgroundRect.intersect(newPosClip);
            foregroundRect = foregroundRect.intersect(newPosClip);
        }

        // If we establish a clip at all, then go ahead and make sure our background
        // rect is intersected with our layer's bounds.
        backgroundRect = backgroundRect.intersect(layerBounds);
    }
}

bool RenderLayer::intersectsDamageRect(const TQRect& layerBounds, const TQRect& damageRect) const
{
    return (renderer()->isCanvas() || renderer()->isRoot() || renderer()->isBody() ||
            (renderer()->hasOverhangingFloats() && !renderer()->hasOverflowClip()) ||
            (renderer()->isInline() && !renderer()->isReplaced()) ||
            layerBounds.intersects(damageRect));
}

bool RenderLayer::containsPoint(int x, int y, const TQRect& damageRect) const
{
    return (renderer()->isCanvas() || renderer()->isRoot() || renderer()->isBody() ||
            renderer()->hasOverhangingFloats() ||
            (renderer()->isInline() && !renderer()->isReplaced()) ||
            damageRect.contains(x, y));
}

// This code has been written to anticipate the addition of CSS3-::outside and ::inside generated
// content (and perhaps XBL).  That's why it uses the render tree and not the DOM tree.
static RenderObject* hoverAncestor(RenderObject* obj)
{
    return (!obj->isInline() && obj->continuation()) ? obj->continuation() : obj->parent();
}

static RenderObject* commonAncestor(RenderObject* obj1, RenderObject* obj2)
{
    if (!obj1 || !obj2)
        return 0;

    for (RenderObject* currObj1 = obj1; currObj1; currObj1 = hoverAncestor(currObj1))
        for (RenderObject* currObj2 = obj2; currObj2; currObj2 = hoverAncestor(currObj2))
            if (currObj1 == currObj2)
                return currObj1;

    return 0;
}


void RenderLayer::updateHoverActiveState(RenderObject::NodeInfo& info)
{
    // We don't update :hover/:active state when the info is marked as readonly.
    if (info.readonly())
        return;

    DOM::NodeImpl *e = m_object->element();
    DOM::DocumentImpl *doc = e ? e->getDocument() : 0;
    if (!doc) return;

    // Check to see if the hovered node has changed.  If not, then we don't need to
    // do anything.
    DOM::NodeImpl* oldHoverNode = doc->hoverNode();
    DOM::NodeImpl* newHoverNode = info.innerNode();

    if (oldHoverNode == newHoverNode && (!oldHoverNode || oldHoverNode->active() == info.active()))
        return;

    // Update our current hover node.
    doc->setHoverNode(newHoverNode);
    if (info.active())
        doc->setActiveNode(newHoverNode);
    else
        doc->setActiveNode(0);

    // We have two different objects.  Fetch their renderers.
    RenderObject* oldHoverObj = oldHoverNode ? oldHoverNode->renderer() : 0;
    RenderObject* newHoverObj = newHoverNode ? newHoverNode->renderer() : 0;

    // Locate the common ancestor render object for the two renderers.
    RenderObject* ancestor = commonAncestor(oldHoverObj, newHoverObj);

    // The old hover path only needs to be cleared up to (and not including) the common ancestor;
    for (RenderObject* curr = oldHoverObj; curr && curr != ancestor; curr = hoverAncestor(curr)) {
        curr->setMouseInside(false);
        if (curr->element()) {
            curr->element()->setActive(false);
            curr->element()->setHovered(false);
        }
    }

    // Now set the hover state for our new object up to the root.
    for (RenderObject* curr = newHoverObj; curr; curr = hoverAncestor(curr)) {
        curr->setMouseInside(true);
        if (curr->element()) {
            curr->element()->setActive(info.active());
            curr->element()->setHovered(true);
        }
    }
}

// Sort the buffer from lowest z-index to highest.  The common scenario will have
// most z-indices equal, so we optimize for that case (i.e., the list will be mostly
// sorted already).
static void sortByZOrder(TQPtrVector<RenderLayer>* buffer,
                         TQPtrVector<RenderLayer>* mergeBuffer,
                         uint start, uint end)
{
    if (start >= end)
        return; // Sanity check.

    if (end - start <= 6) {
        // Apply a bubble sort for smaller lists.
        for (uint i = end-1; i > start; i--) {
            bool sorted = true;
            for (uint j = start; j < i; j++) {
                RenderLayer* elt = buffer->at(j);
                RenderLayer* elt2 = buffer->at(j+1);
                if (elt->zIndex() > elt2->zIndex()) {
                    sorted = false;
                    buffer->insert(j, elt2);
                    buffer->insert(j+1, elt);
                }
            }
            if (sorted)
                return;
        }
    }
    else {
        // Peform a merge sort for larger lists.
        uint mid = (start+end)/2;
        sortByZOrder(buffer, mergeBuffer, start, mid);
        sortByZOrder(buffer, mergeBuffer, mid, end);

        RenderLayer* elt = buffer->at(mid-1);
        RenderLayer* elt2 = buffer->at(mid);

        // Handle the fast common case (of equal z-indices).  The list may already
        // be completely sorted.
        if (elt->zIndex() <= elt2->zIndex())
            return;

        // We have to merge sort.  Ensure our merge buffer is big enough to hold
        // all the items.
        mergeBuffer->resize(end - start);
        uint i1 = start;
        uint i2 = mid;

        elt = buffer->at(i1);
        elt2 = buffer->at(i2);

        while (i1 < mid || i2 < end) {
            if (i1 < mid && (i2 == end || elt->zIndex() <= elt2->zIndex())) {
                mergeBuffer->insert(mergeBuffer->count(), elt);
                i1++;
                if (i1 < mid)
                    elt = buffer->at(i1);
            }
            else {
                mergeBuffer->insert(mergeBuffer->count(), elt2);
                i2++;
                if (i2 < end)
                    elt2 = buffer->at(i2);
            }
        }

        for (uint i = start; i < end; i++)
            buffer->insert(i, mergeBuffer->at(i-start));

        mergeBuffer->clear();
    }
}

void RenderLayer::dirtyZOrderLists()
{
    if (m_posZOrderList)
        m_posZOrderList->clear();
    if (m_negZOrderList)
        m_negZOrderList->clear();
    m_zOrderListsDirty = true;
}

void RenderLayer::dirtyOverflowList()
{
    if (m_overflowList)
        m_overflowList->clear();
    m_overflowListDirty = true;
}

void RenderLayer::updateZOrderLists()
{
    if (!isStackingContext() || !m_zOrderListsDirty)
        return;

    for (RenderLayer* child = firstChild(); child; child = child->nextSibling())
        child->collectLayers(m_posZOrderList, m_negZOrderList);

    // Sort the two lists.
    if (m_posZOrderList) {
        TQPtrVector<RenderLayer> mergeBuffer;
        sortByZOrder(m_posZOrderList, &mergeBuffer, 0, m_posZOrderList->count());
    }
    if (m_negZOrderList) {
        TQPtrVector<RenderLayer> mergeBuffer;
        sortByZOrder(m_negZOrderList, &mergeBuffer, 0, m_negZOrderList->count());
    }

    m_zOrderListsDirty = false;
}

void RenderLayer::updateOverflowList()
{
    if (!m_overflowListDirty)
        return;

    for (RenderLayer* child = firstChild(); child; child = child->nextSibling()) {
        if (child->isOverflowOnly()) {
            if (!m_overflowList)
                m_overflowList = new TQValueList<RenderLayer*>;
            m_overflowList->append(child);
        }
    }

    m_overflowListDirty = false;
}

void RenderLayer::collectLayers(TQPtrVector<RenderLayer>*& posBuffer, TQPtrVector<RenderLayer>*& negBuffer)
{
    // FIXME: A child render object or layer could override visibility.  Don't remove this
    // optimization though until RenderObject's nodeAtPoint is patched to understand what to do
    // when visibility is overridden by a child.
    if (renderer()->style()->visibility() != VISIBLE)
        return;

    // Overflow layers are just painted by their enclosing layers, so they don't get put in zorder lists.
    if (!isOverflowOnly()) {

        // Determine which buffer the child should be in.
        TQPtrVector<RenderLayer>*& buffer = (zIndex() >= 0) ? posBuffer : negBuffer;

        // Create the buffer if it doesn't exist yet.
        if (!buffer)
            buffer = new TQPtrVector<RenderLayer>();

        // Resize by a power of 2 when our buffer fills up.
        if (buffer->count() == buffer->size())
            buffer->resize(2*(buffer->size()+1));

        // Append ourselves at the end of the appropriate buffer.
        buffer->insert(buffer->count(), this);
    }

    // Recur into our children to collect more layers, but only if we don't establish
    // a stacking context.
    if (!isStackingContext()) {
        for (RenderLayer* child = firstChild(); child; child = child->nextSibling())
            child->collectLayers(posBuffer, negBuffer);
    }
}

#ifdef ENABLE_DUMP
#ifndef KDE_USE_FINAL
static TQTextStream &operator<<(TQTextStream &ts, const TQRect &r)
{
    return ts << "at (" << r.x() << "," << r.y() << ") size " << r.width() << "x" << r.height();
}
#endif

static void write(TQTextStream &ts, RenderObject& o, const TQString& indent )
{
    o.dump(ts, indent);

    for (RenderObject *child = o.firstChild(); child; child = child->nextSibling()) {
        if (child->layer()) continue;
        write( ts, *child, indent + "   " );
    }
}

static void write(TQTextStream &ts, const RenderLayer &l,
                  const TQRect& layerBounds, const TQRect& backgroundClipRect, const TQRect& clipRect,
                  int layerType = 0, const TQString& indent = TQString::null)

{
    ts << indent << "layer";

    ts << " at (" << l.xPos() << "," << l.yPos() << ") size " << l.width() << "x" << l.height();

    if (layerBounds != layerBounds.intersect(backgroundClipRect)) {
        ts << " backgroundClip " << backgroundClipRect;
    }
    if (layerBounds != layerBounds.intersect(clipRect)) {
        ts << " clip " << clipRect;
    }

    if (layerType == -1)
         ts << " layerType: background only";
    else if (layerType == 1)
        ts << " layerType: foreground only";

    ts << "\n";

    if (layerType != -1)
        write( ts, *l.renderer(), indent + "   " );

    ts << "\n";
}

static void writeLayers(TQTextStream &ts, const RenderLayer* rootLayer, RenderLayer* l,
                        const TQRect& paintDirtyRect, const TQString& indent)
{
    // Calculate the clip rects we should use.
    TQRect layerBounds, damageRect, clipRectToApply;
    l->calculateRects(rootLayer, paintDirtyRect, layerBounds, damageRect, clipRectToApply);

    // Ensure our lists are up-to-date.
    l->updateZOrderLists();
    l->updateOverflowList();

    bool shouldPaint = l->intersectsDamageRect(layerBounds, damageRect);
    TQPtrVector<RenderLayer>* negList = l->negZOrderList();
    TQValueList<RenderLayer*>* ovfList = l->overflowList();
    if (shouldPaint && negList && negList->count() > 0)
        write(ts, *l, layerBounds, damageRect, clipRectToApply, -1, indent);

    if (negList) {
        for (unsigned i = 0; i != negList->count(); ++i)
            writeLayers(ts, rootLayer, negList->at(i), paintDirtyRect, indent );
    }

    if (shouldPaint)
        write(ts, *l, layerBounds, damageRect, clipRectToApply, negList && negList->count() > 0, indent);

    if (ovfList) {
        for (TQValueList<RenderLayer*>::iterator it = ovfList->begin(); it != ovfList->end(); ++it)
            writeLayers(ts, rootLayer, *it, paintDirtyRect, indent);
    }

    TQPtrVector<RenderLayer>* posList = l->posZOrderList();
    if (posList) {
        for (unsigned i = 0; i != posList->count(); ++i)
            writeLayers(ts, rootLayer, posList->at(i), paintDirtyRect, indent);
    }
}


void RenderLayer::dump(TQTextStream &ts, const TQString &ind)
{
    assert( renderer()->isCanvas() );

    writeLayers(ts, this, this, TQRect(xPos(), yPos(), width(), height()), ind);
}


#endif

bool RenderLayer::shouldBeOverflowOnly() const
{
    return renderer()->style() && renderer()->hasOverflowClip() &&
           !renderer()->isPositioned() &&  !renderer()->isRelPositioned();
           /* && !isTransparent(); */
}

void RenderLayer::styleChanged()
{
    bool isOverflowOnly = shouldBeOverflowOnly();
    if (isOverflowOnly != m_isOverflowOnly) {
        m_isOverflowOnly = isOverflowOnly;
        RenderLayer* p = parent();
        RenderLayer* sc = stackingContext();
        if (p)
            p->dirtyOverflowList();
        if (sc)
            sc->dirtyZOrderLists();
    }

    if (m_object->hasOverflowClip() && 
         m_object->style()->overflowX() == OMARQUEE && m_object->style()->marqueeBehavior() != MNONE) {
        if (!m_marquee)
            m_marquee = new Marquee(this);
        m_marquee->updateMarqueeStyle();
    }
    else if (m_marquee) {
        delete m_marquee;
        m_marquee = 0;
    }
}

void RenderLayer::suspendMarquees()
{
    if (m_marquee)
        m_marquee->suspend();

    for (RenderLayer* curr = firstChild(); curr; curr = curr->nextSibling())
        curr->suspendMarquees();
}

// --------------------------------------------------------------------------
// Marquee implementation

Marquee::Marquee(RenderLayer* l)
:m_layer(l), m_currentLoop(0), m_totalLoops(0), m_timerId(0), m_start(0), m_end(0), m_speed(0), m_unfurlPos(0), m_reset(false),
 m_suspended(false), m_stopped(false), m_whiteSpace(NORMAL), m_direction(MAUTO)
{
}

int Marquee::marqueeSpeed() const
{
    int result = m_layer->renderer()->style()->marqueeSpeed();
    DOM::NodeImpl* elt = m_layer->renderer()->element();
    if (elt && elt->id() == ID_MARQUEE) {
        HTMLMarqueeElementImpl* marqueeElt = static_cast<HTMLMarqueeElementImpl*>(elt);
        result = kMax(result, marqueeElt->minimumDelay());
    }
    return result;
}

EMarqueeDirection Marquee::direction() const
{
    // FIXME: Support the CSS3 "auto" value for determining the direction of the marquee.
    // For now just map MAUTO to MBACKWARD
    EMarqueeDirection result = m_layer->renderer()->style()->marqueeDirection();
    EDirection dir =  m_layer->renderer()->style()->direction();
    if (result == MAUTO)
        result = MBACKWARD;
    if (result == MFORWARD)
        result = (dir == LTR) ? MRIGHT : MLEFT;
    if (result == MBACKWARD)
        result = (dir == LTR) ? MLEFT : MRIGHT;

    // Now we have the real direction.  Next we check to see if the increment is negative.
    // If so, then we reverse the direction.
    Length increment = m_layer->renderer()->style()->marqueeIncrement();
    if (increment.value() < 0)
        result = static_cast<EMarqueeDirection>(-result);

    return result;
}

bool Marquee::isHorizontal() const
{
    return direction() == MLEFT || direction() == MRIGHT;
}

bool Marquee::isUnfurlMarquee() const
{
    EMarqueeBehavior behavior = m_layer->renderer()->style()->marqueeBehavior();
    return (behavior == MUNFURL);
}

int Marquee::computePosition(EMarqueeDirection dir, bool stopAtContentEdge)
{
    RenderObject* o = m_layer->renderer();
    RenderStyle* s = o->style();
    if (isHorizontal()) {
        bool ltr = s->direction() == LTR;
        int clientWidth = o->clientWidth();
        int contentWidth = ltr ? o->rightmostPosition(true, false) : o->leftmostPosition(true, false);
        if (ltr)
            contentWidth += (o->paddingRight() - o->borderLeft());
        else {
            contentWidth = o->width() - contentWidth;
            contentWidth += (o->paddingLeft() - o->borderRight());
        }
        if (dir == MRIGHT) {
            if (stopAtContentEdge)
                return kMax(0, ltr ? (contentWidth - clientWidth) : (clientWidth - contentWidth));
            else
                return ltr ? contentWidth : clientWidth;
        }
        else {
            if (stopAtContentEdge)
                return kMin(0, ltr ? (contentWidth - clientWidth) : (clientWidth - contentWidth));
            else
                return ltr ? -clientWidth : -contentWidth;
        }
    }
    else {
        int contentHeight = m_layer->renderer()->lowestPosition(true, false) -
                            m_layer->renderer()->borderTop() + m_layer->renderer()->paddingBottom();
        int clientHeight = m_layer->renderer()->clientHeight();
        if (dir == MUP) {
            if (stopAtContentEdge)
                 return kMin(contentHeight - clientHeight, 0);
            else
                return -clientHeight;
        }
        else {
            if (stopAtContentEdge)
                return kMax(contentHeight - clientHeight, 0);
            else
                return contentHeight;
        }
    }
}

void Marquee::start()
{
    if (m_timerId || m_layer->renderer()->style()->marqueeIncrement().value() == 0)
        return;

    if (!m_suspended && !m_stopped) {
        if (isUnfurlMarquee()) {
            bool forward = direction() == MDOWN || direction() == MRIGHT;
            bool isReversed = (forward && m_currentLoop % 2) || (!forward && !(m_currentLoop % 2));
            m_unfurlPos = isReversed ? m_end : m_start;
            m_layer->renderer()->setChildNeedsLayout(true);
        }
        else {
            if (isHorizontal())
                m_layer->scrollToOffset(m_start, 0, false, false);
            else
                m_layer->scrollToOffset(0, m_start, false, false);
        }
    }
    else
        m_suspended = false;

    m_stopped = false;
    m_timerId = startTimer(speed());
}

void Marquee::suspend()
{
    if (m_timerId) {
        killTimer(m_timerId);
        m_timerId = 0;
    }

    m_suspended = true;
}

void Marquee::stop()
{
    if (m_timerId) {
        killTimer(m_timerId);
        m_timerId = 0;
    }

    m_stopped = true;
}

void Marquee::updateMarqueePosition()
{
    bool activate = (m_totalLoops <= 0 || m_currentLoop < m_totalLoops);
    if (activate) {
        if (isUnfurlMarquee()) {
            if (m_unfurlPos < m_start) {
                m_unfurlPos = m_start;
                m_layer->renderer()->setChildNeedsLayout(true);
            }
            else if (m_unfurlPos > m_end) {
                m_unfurlPos = m_end;
                m_layer->renderer()->setChildNeedsLayout(true);
            }
        }
        else {
            EMarqueeBehavior behavior = m_layer->renderer()->style()->marqueeBehavior();
            m_start = computePosition(direction(), behavior == MALTERNATE);
            m_end = computePosition(reverseDirection(), behavior == MALTERNATE || behavior == MSLIDE);
        }
        if (!m_stopped) start();
    }
}

void Marquee::updateMarqueeStyle()
{
    RenderStyle* s = m_layer->renderer()->style();

    if (m_direction != s->marqueeDirection() || (m_totalLoops != s->marqueeLoopCount() && m_currentLoop >= m_totalLoops))
        m_currentLoop = 0; // When direction changes or our loopCount is a smaller number than our current loop, reset our loop.

    m_totalLoops = s->marqueeLoopCount();
    m_direction = s->marqueeDirection();
    m_whiteSpace = s->whiteSpace();

    if (m_layer->renderer()->isHTMLMarquee()) {
        // Hack for WinIE.  In WinIE, a value of 0 or lower for the loop count for SLIDE means to only do
        // one loop.
        if (m_totalLoops <= 0 && (s->marqueeBehavior() == MSLIDE || s->marqueeBehavior() == MUNFURL))
            m_totalLoops = 1;

        // Hack alert: Set the white-space value to nowrap for horizontal marquees with inline children, thus ensuring
        // all the text ends up on one line by default.  Limit this hack to the <marquee> element to emulate
        // WinIE's behavior.  Someone using CSS3 can use white-space: nowrap on their own to get this effect.
        // Second hack alert: Set the text-align back to auto.  WinIE completely ignores text-align on the
        // marquee element.
        // FIXME: Bring these up with the CSS WG.
        if (isHorizontal() && m_layer->renderer()->childrenInline()) {
            s->setWhiteSpace(NOWRAP);
            s->setTextAlign(TAAUTO);
        }
    }

    if (speed() != marqueeSpeed()) {
        m_speed = marqueeSpeed();
        if (m_timerId) {
            killTimer(m_timerId);
            m_timerId = startTimer(speed());
        }
    }

    // Check the loop count to see if we should now stop.
    bool activate = (m_totalLoops <= 0 || m_currentLoop < m_totalLoops);
    if (activate && !m_timerId)
        m_layer->renderer()->setNeedsLayout(true);
    else if (!activate && m_timerId) {
        // Destroy the timer.
        killTimer(m_timerId);
        m_timerId = 0;
    }
}

void Marquee::timerEvent(TQTimerEvent* /*evt*/)
{
    if (m_layer->renderer()->needsLayout())
        return;

    if (m_reset) {
        m_reset = false;
        if (isHorizontal())
            m_layer->scrollToXOffset(m_start);
        else
            m_layer->scrollToYOffset(m_start);
        return;
    }

    RenderStyle* s = m_layer->renderer()->style();

    int endPoint = m_end;
    int range = m_end - m_start;
    int newPos;
    if (range == 0)
        newPos = m_end;
    else {
        bool addIncrement = direction() == MUP || direction() == MLEFT;
        bool isReversed = s->marqueeBehavior() == MALTERNATE && m_currentLoop % 2;
        if (isUnfurlMarquee()) {
            isReversed = (!addIncrement && m_currentLoop % 2) || (addIncrement && !(m_currentLoop % 2));
            addIncrement = !isReversed;
        }
        if (isReversed) {
            // We're going in the reverse direction.
            endPoint = m_start;
            range = -range;
            if (!isUnfurlMarquee())
                addIncrement = !addIncrement;
        }
        bool positive = range > 0;
        int clientSize = isUnfurlMarquee() ? abs(range) :
            (isHorizontal() ? m_layer->renderer()->clientWidth() : m_layer->renderer()->clientHeight());
        int increment = kMax(1, abs(m_layer->renderer()->style()->marqueeIncrement().width(clientSize)));
        int currentPos = isUnfurlMarquee() ? m_unfurlPos :
            (isHorizontal() ? m_layer->scrollXOffset() : m_layer->scrollYOffset());
        newPos =  currentPos + (addIncrement ? increment : -increment);
        if (positive)
            newPos = kMin(newPos, endPoint);
        else
            newPos = kMax(newPos, endPoint);
    }

    if (newPos == endPoint) {
        m_currentLoop++;
        if (m_totalLoops > 0 && m_currentLoop >= m_totalLoops) {
            killTimer(m_timerId);
            m_timerId = 0;
        }
        else if (s->marqueeBehavior() != MALTERNATE && s->marqueeBehavior() != MUNFURL)
            m_reset = true;
    }

    if (isUnfurlMarquee()) {
        m_unfurlPos = newPos;
        m_layer->renderer()->setChildNeedsLayout(true);
    }
    else {
        if (isHorizontal())
            m_layer->scrollToXOffset(newPos);
        else
            m_layer->scrollToYOffset(newPos);
    }
}

#include "render_layer.moc"
