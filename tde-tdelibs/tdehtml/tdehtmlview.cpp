/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *                     2000-2004 Dirk Mueller <mueller@kde.org>
 *                     2003 Leo Savernik <l.savernik@aon.at>
 *                     2003-2004 Apple Computer, Inc.
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
 */


#include "tdehtmlview.moc"

#include "tdehtmlview.h"

#include "tdehtml_part.h"
#include "tdehtml_events.h"

#include "html/html_documentimpl.h"
#include "html/html_inlineimpl.h"
#include "html/html_formimpl.h"
#include "rendering/render_arena.h"
#include "rendering/render_canvas.h"
#include "rendering/render_frames.h"
#include "rendering/render_replaced.h"
#include "rendering/render_layer.h"
#include "rendering/render_line.h"
#include "rendering/render_table.h"
// removeme
#define protected public
#include "rendering/render_text.h"
#undef protected
#include "xml/dom2_eventsimpl.h"
#include "css/cssstyleselector.h"
#include "css/csshelper.h"
#include "misc/htmlhashes.h"
#include "misc/helper.h"
#include "misc/loader.h"
#include "tdehtml_settings.h"
#include "tdehtml_printsettings.h"

#include "tdehtmlpart_p.h"

#ifndef TDEHTML_NO_CARET
#include "tdehtml_caret_p.h"
#include "xml/dom2_rangeimpl.h"
#endif

#include <tdeapplication.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <tdelocale.h>
#include <knotifyclient.h>
#include <kprinter.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <tdestdaccel.h>
#include <kstringhandler.h>
#include <kurldrag.h>

#include <tqbitmap.h>
#include <tqlabel.h>
#include <tqobjectlist.h>
#include <tqpaintdevicemetrics.h>
#include <tqpainter.h>
#include <tqptrdict.h>
#include <tqtooltip.h>
#include <tqstring.h>
#include <tqstylesheet.h>
#include <tqtimer.h>
#include <tqvaluevector.h>

//#define DEBUG_NO_PAINT_BUFFER

//#define DEBUG_FLICKER

//#define DEBUG_PIXEL

#ifdef TQ_WS_X11
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

#define PAINT_BUFFER_HEIGHT 128

#if 0
namespace tdehtml {
    void dumpLineBoxes(RenderFlow *flow);
}
#endif

using namespace DOM;
using namespace tdehtml;
class TDEHTMLToolTip;


#ifndef TQT_NO_TOOLTIP

class TDEHTMLToolTip : public TQToolTip
{
public:
    TDEHTMLToolTip(TDEHTMLView *view,  TDEHTMLViewPrivate* vp) : TQToolTip(view->viewport())
    {
        m_view = view;
        m_viewprivate = vp;
    };

protected:
    virtual void maybeTip(const TQPoint &);

private:
    TDEHTMLView *m_view;
    TDEHTMLViewPrivate* m_viewprivate;
};

#endif

class TDEHTMLViewPrivate {
    friend class TDEHTMLToolTip;
public:

    enum PseudoFocusNodes {
	PFNone,
	PFTop,
	PFBottom
    };

    enum CompletedState {
        CSNone = 0,
        CSFull,
        CSActionPending
    };

    TDEHTMLViewPrivate()
        : underMouse( 0 ), underMouseNonShared( 0 ), visibleWidgets( 107 )
#ifndef NO_SMOOTH_SCROLL_HACK
          , dx(0), dy(0), ddx(0), ddy(0), rdx(0), rdy(0), scrolling(false)
#endif
    {
#ifndef TDEHTML_NO_CARET
	m_caretViewContext = 0;
	m_editorContext = 0;
#endif // TDEHTML_NO_CARET
        postponed_autorepeat = NULL;
        reset();
        vmode = TQScrollView::Auto;
 	hmode = TQScrollView::Auto;
        tp=0;
        paintBuffer=0;
        vertPaintBuffer=0;
        formCompletions=0;
        prevScrollbarVisible = true;
	tooltip = 0;
        possibleTripleClick = false;
        emitCompletedAfterRepaint = CSNone;
	cursor_icon_widget = NULL;
        m_mouseScrollTimer = 0;
        m_mouseScrollIndicator = 0;
    }
    ~TDEHTMLViewPrivate()
    {
        delete formCompletions;
        delete tp; tp = 0;
        delete paintBuffer; paintBuffer =0;
        delete vertPaintBuffer;
        delete postponed_autorepeat;
        if (underMouse)
	    underMouse->deref();
        if (underMouseNonShared)
	    underMouseNonShared->deref();
	delete tooltip;
#ifndef TDEHTML_NO_CARET
	delete m_caretViewContext;
	delete m_editorContext;
#endif // TDEHTML_NO_CARET
        delete cursor_icon_widget;
        delete m_mouseScrollTimer;
        delete m_mouseScrollIndicator;
    }
    void reset()
    {
        if (underMouse)
	    underMouse->deref();
	underMouse = 0;
        if (underMouseNonShared)
	    underMouseNonShared->deref();
	underMouseNonShared = 0;
        linkPressed = false;
        useSlowRepaints = false;
	tabMovePending = false;
	lastTabbingDirection = true;
	pseudoFocusNode = PFNone;
#ifndef TDEHTML_NO_SCROLLBARS
        //We don't turn off the toolbars here
	//since if the user turns them
	//off, then chances are they want them turned
	//off always - even after a reset.
#else
        vmode = TQScrollView::AlwaysOff;
        hmode = TQScrollView::AlwaysOff;
#endif
#ifdef DEBUG_PIXEL
        timer.start();
        pixelbooth = 0;
        repaintbooth = 0;
#endif
        scrollBarMoved = false;
        contentsMoving = false;
        ignoreWheelEvents = false;
	borderX = 30;
	borderY = 30;
        paged = false;
	clickX = -1;
	clickY = -1;
        prevMouseX = -1;
        prevMouseY = -1;
	clickCount = 0;
	isDoubleClick = false;
	scrollingSelf = false;
        delete postponed_autorepeat;
        postponed_autorepeat = NULL;
	layoutTimerId = 0;
        repaintTimerId = 0;
        scrollTimerId = 0;
        scrollSuspended = false;
        scrollSuspendPreActivate = false;
        complete = false;
        firstRelayout = true;
        needsFullRepaint = true;
        dirtyLayout = false;
        layoutSchedulingEnabled = true;
        painting = false;
        updateRegion = TQRegion();
        m_dialogsAllowed = true;
#ifndef TDEHTML_NO_CARET
        if (m_caretViewContext) {
          m_caretViewContext->caretMoved = false;
	  m_caretViewContext->keyReleasePending = false;
        }/*end if*/
#endif // TDEHTML_NO_CARET
#ifndef TDEHTML_NO_TYPE_AHEAD_FIND
        typeAheadActivated = false;
#endif // TDEHTML_NO_TYPE_AHEAD_FIND
	accessKeysActivated = false;
	accessKeysPreActivate = false;

        // We ref/deref to ensure defaultHTMLSettings is available
        TDEHTMLFactory::ref();
        accessKeysEnabled = TDEHTMLFactory::defaultHTMLSettings()->accessKeysEnabled();
        TDEHTMLFactory::deref();

        emitCompletedAfterRepaint = CSNone;
    }
    void newScrollTimer(TQWidget *view, int tid)
    {
        //kdDebug(6000) << "newScrollTimer timer " << tid << endl;
        view->killTimer(scrollTimerId);
        scrollTimerId = tid;
        scrollSuspended = false;
    }
    enum ScrollDirection { ScrollLeft, ScrollRight, ScrollUp, ScrollDown };

    void adjustScroller(TQWidget *view, ScrollDirection direction, ScrollDirection oppositedir)
    {
        static const struct { int msec, pixels; } timings [] = {
            {320,1}, {224,1}, {160,1}, {112,1}, {80,1}, {56,1}, {40,1},
            {28,1}, {20,1}, {20,2}, {20,3}, {20,4}, {20,6}, {20,8}, {0,0}
        };
        if (!scrollTimerId ||
            (static_cast<int>(scrollDirection) != direction &&
             (static_cast<int>(scrollDirection) != oppositedir || scrollSuspended))) {
            scrollTiming = 6;
            scrollBy = timings[scrollTiming].pixels;
            scrollDirection = direction;
            newScrollTimer(view, view->startTimer(timings[scrollTiming].msec));
        } else if (scrollDirection == direction &&
                   timings[scrollTiming+1].msec && !scrollSuspended) {
            scrollBy = timings[++scrollTiming].pixels;
            newScrollTimer(view, view->startTimer(timings[scrollTiming].msec));
        } else if (scrollDirection == oppositedir) {
            if (scrollTiming) {
                scrollBy = timings[--scrollTiming].pixels;
                newScrollTimer(view, view->startTimer(timings[scrollTiming].msec));
            }
        }
        scrollSuspended = false;
    }

#ifndef TDEHTML_NO_CARET
    /** this function returns an instance of the caret view context. If none
     * exists, it will be instantiated.
     */
    CaretViewContext *caretViewContext() {
      if (!m_caretViewContext) m_caretViewContext = new CaretViewContext();
      return m_caretViewContext;
    }
    /** this function returns an instance of the editor context. If none
     * exists, it will be instantiated.
     */
    EditorContext *editorContext() {
      if (!m_editorContext) m_editorContext = new EditorContext();
      return m_editorContext;
    }
#endif // TDEHTML_NO_CARET

#ifdef DEBUG_PIXEL
    TQTime timer;
    unsigned int pixelbooth;
    unsigned int repaintbooth;
#endif

    TQPainter *tp;
    TQPixmap  *paintBuffer;
    TQPixmap  *vertPaintBuffer;
    NodeImpl *underMouse;
    NodeImpl *underMouseNonShared;

    bool tabMovePending:1;
    bool lastTabbingDirection:1;
    PseudoFocusNodes pseudoFocusNode:2;
    bool scrollBarMoved:1;
    bool contentsMoving:1;

    TQScrollView::ScrollBarMode vmode;
    TQScrollView::ScrollBarMode hmode;
    bool prevScrollbarVisible:1;
    bool linkPressed:1;
    bool useSlowRepaints:1;
    bool ignoreWheelEvents:1;

    int borderX, borderY;
    KSimpleConfig *formCompletions;

    bool paged;

    int clickX, clickY, clickCount;
    bool isDoubleClick;

    int prevMouseX, prevMouseY;
    bool scrollingSelf;
    int layoutTimerId;
    TQKeyEvent* postponed_autorepeat;

    int repaintTimerId;
    int scrollTimerId;
    int scrollTiming;
    int scrollBy;
    ScrollDirection scrollDirection		:2;
    bool scrollSuspended			:1;
    bool scrollSuspendPreActivate		:1;
    bool complete				:1;
    bool firstRelayout				:1;
    bool layoutSchedulingEnabled		:1;
    bool needsFullRepaint			:1;
    bool painting				:1;
    bool possibleTripleClick			:1;
    bool dirtyLayout                           :1;
    bool m_dialogsAllowed			:1;
    TQRegion updateRegion;
    TDEHTMLToolTip *tooltip;
    TQPtrDict<TQWidget> visibleWidgets;
#ifndef TDEHTML_NO_CARET
    CaretViewContext *m_caretViewContext;
    EditorContext *m_editorContext;
#endif // TDEHTML_NO_CARET
#ifndef TDEHTML_NO_TYPE_AHEAD_FIND
    TQString findString;
    TQTimer timer;
    bool findLinksOnly;
    bool typeAheadActivated;
#endif // TDEHTML_NO_TYPE_AHEAD_FIND
    bool accessKeysEnabled;
    bool accessKeysActivated;
    bool accessKeysPreActivate;
    CompletedState emitCompletedAfterRepaint;

    TQWidget* cursor_icon_widget;

    // scrolling activated by MMB
    short m_mouseScroll_byX;
    short m_mouseScroll_byY;
    TQTimer *m_mouseScrollTimer;
    TQWidget *m_mouseScrollIndicator;
#ifndef NO_SMOOTH_SCROLL_HACK
    TQTimer timer2;
    int dx;
    int dy;
    // Step size * 16 and residual to avoid huge difference between 1px/step and 2px/step
    int ddx;
    int ddy;
    int rdx;
    int rdy;
    bool scrolling;
#endif
};

#ifndef TQT_NO_TOOLTIP

/** calculates the client-side image map rectangle for the given image element
 * @param img image element
 * @param scrollOfs scroll offset of viewport in content coordinates
 * @param p position to be probed in viewport coordinates
 * @param r returns the bounding rectangle in content coordinates
 * @param s returns the title string
 * @return true if an appropriate area was found -- only in this case r and
 *	s are valid, false otherwise
 */
static bool findImageMapRect(HTMLImageElementImpl *img, const TQPoint &scrollOfs,
			const TQPoint &p, TQRect &r, TQString &s)
{
    HTMLMapElementImpl* map;
    if (img && img->getDocument()->isHTMLDocument() &&
        (map = static_cast<HTMLDocumentImpl*>(img->getDocument())->getMap(img->imageMap()))) {
        RenderObject::NodeInfo info(true, false);
        RenderObject *rend = img->renderer();
        int ax, ay;
        if (!rend || !rend->absolutePosition(ax, ay))
            return false;
        // we're a client side image map
        bool inside = map->mapMouseEvent(p.x() - ax + scrollOfs.x(),
                p.y() - ay + scrollOfs.y(), rend->contentWidth(),
                rend->contentHeight(), info);
        if (inside && info.URLElement()) {
            HTMLAreaElementImpl *area = static_cast<HTMLAreaElementImpl *>(info.URLElement());
            Q_ASSERT(area->id() == ID_AREA);
            s = area->getAttribute(ATTR_TITLE).string();
            TQRegion reg = area->cachedRegion();
            if (!s.isEmpty() && !reg.isEmpty()) {
                r = reg.boundingRect();
                r.moveBy(ax, ay);
                return true;
            }
        }
    }
    return false;
}

void TDEHTMLToolTip::maybeTip(const TQPoint& p)
{
    DOM::NodeImpl *node = m_viewprivate->underMouseNonShared;
    TQRect region;
    while ( node ) {
        if ( node->isElementNode() ) {
            DOM::ElementImpl *e = static_cast<DOM::ElementImpl*>( node );
            TQRect r;
            TQString s;
            bool found = false;
            // for images, check if it is part of a client-side image map,
            // and query the <area>s' title attributes, too
            if (e->id() == ID_IMG && !e->getAttribute( ATTR_USEMAP ).isEmpty()) {
                found = findImageMapRect(static_cast<HTMLImageElementImpl *>(e),
                    		m_view->viewportToContents(TQPoint(0, 0)), p, r, s);
            }
            if (!found) {
                s = e->getAttribute( ATTR_TITLE ).string();
                r = node->getRect();
            }
            region |= TQRect( m_view->contentsToViewport( r.topLeft() ), r.size() );
            if ( !s.isEmpty() ) {
                tip( region, TQStyleSheet::convertFromPlainText( s, TQStyleSheetItem::WhiteSpaceNormal ) );
                break;
            }
        }
        node = node->parentNode();
    }
}
#endif

TDEHTMLView::TDEHTMLView( TDEHTMLPart *part, TQWidget *parent, const char *name)
    : TQScrollView( parent, name, (WFlags)(WResizeNoErase | WRepaintNoErase) )
{
    m_medium = "screen";

    m_part = part;
    d = new TDEHTMLViewPrivate;
    TQScrollView::setVScrollBarMode(d->vmode);
    TQScrollView::setHScrollBarMode(d->hmode);
    connect(kapp, TQ_SIGNAL(tdedisplayPaletteChanged()), this, TQ_SLOT(slotPaletteChanged()));
    connect(this, TQ_SIGNAL(contentsMoving(int, int)), this, TQ_SLOT(slotScrollBarMoved()));

    // initialize QScrollView
    enableClipper(true);
    // hack to get unclipped painting on the viewport.
    static_cast<TDEHTMLView *>(viewport())->setWFlags(WPaintUnclipped);

    setResizePolicy(Manual);
    viewport()->setMouseTracking(true);
    viewport()->setBackgroundMode(NoBackground);

    KImageIO::registerFormats();

#ifndef TQT_NO_TOOLTIP
    d->tooltip = new TDEHTMLToolTip( this, d );
#endif

#ifndef TDEHTML_NO_TYPE_AHEAD_FIND
    connect(&d->timer, TQ_SIGNAL(timeout()), this, TQ_SLOT(findTimeout()));
#endif // TDEHTML_NO_TYPE_AHEAD_FIND

    init();

    viewport()->show();
#ifndef NO_SMOOTH_SCROLL_HACK
#define timer timer2
    connect(&d->timer, TQ_SIGNAL(timeout()), this, TQ_SLOT(scrollTick()));
#undef timer
#endif
}

TDEHTMLView::~TDEHTMLView()
{
    closeChildDialogs();
    if (m_part)
    {
        //WABA: Is this Ok? Do I need to deref it as well?
        //Does this need to be done somewhere else?
        DOM::DocumentImpl *doc = m_part->xmlDocImpl();
        if (doc)
            doc->detach();
    }
    delete d; d = 0;
}

void TDEHTMLView::init()
{
    if(!d->paintBuffer) d->paintBuffer = new TQPixmap(PAINT_BUFFER_HEIGHT, PAINT_BUFFER_HEIGHT);
    if(!d->vertPaintBuffer)
        d->vertPaintBuffer = new TQPixmap(10, PAINT_BUFFER_HEIGHT);
    if(!d->tp) d->tp = new TQPainter();

    setFocusPolicy(TQWidget::StrongFocus);
    viewport()->setFocusProxy(this);

    _marginWidth = -1; // undefined
    _marginHeight = -1;
    _width = 0;
    _height = 0;

    installEventFilter(this);

    setAcceptDrops(true);
    TQSize s = viewportSize(4095, 4095);
    resizeContents(s.width(), s.height());
}

void TDEHTMLView::clear()
{
    // work around QScrollview's unbelievable bugginess
    setStaticBackground(true);
#ifndef TDEHTML_NO_CARET
    if (!m_part->isCaretMode() && !m_part->isEditable()) caretOff();
#endif

#ifndef TDEHTML_NO_TYPE_AHEAD_FIND
    if( d->typeAheadActivated )
        findTimeout();
#endif
    if (d->accessKeysEnabled && d->accessKeysActivated)
        accessKeysTimeout();
    viewport()->unsetCursor();
    if ( d->cursor_icon_widget )
        d->cursor_icon_widget->hide();
    d->reset();
    this->killTimers();
    emit cleared();

    TQScrollView::setHScrollBarMode(d->hmode);
    TQScrollView::setVScrollBarMode(d->vmode);
    verticalScrollBar()->setEnabled( false );
    horizontalScrollBar()->setEnabled( false );
}

void TDEHTMLView::hideEvent(TQHideEvent* e)
{
    TQScrollView::hideEvent(e);
    if ( m_part && m_part->xmlDocImpl() )
        m_part->xmlDocImpl()->docLoader()->pauseAnimations();
}

void TDEHTMLView::showEvent(TQShowEvent* e)
{
    TQScrollView::showEvent(e);
    if ( m_part && m_part->xmlDocImpl() )
        m_part->xmlDocImpl()->docLoader()->resumeAnimations();
}

void TDEHTMLView::resizeEvent (TQResizeEvent* e)
{
    int dw = e->oldSize().width() - e->size().width();
    int dh = e->oldSize().height() - e->size().height();

    // if we are shrinking the view, don't allow the content to overflow
    // before the layout occurs - we don't know if we need scrollbars yet
    dw = dw>0 ? kMax(0, contentsWidth()-dw) : contentsWidth();
    dh = dh>0 ? kMax(0, contentsHeight()-dh) : contentsHeight();

    resizeContents(dw, dh);

    TQScrollView::resizeEvent(e);

    if ( m_part && m_part->xmlDocImpl() )
        m_part->xmlDocImpl()->dispatchWindowEvent( EventImpl::RESIZE_EVENT, false, false );
}

void TDEHTMLView::viewportResizeEvent (TQResizeEvent* e)
{
    TQScrollView::viewportResizeEvent(e);

    //int w = visibleWidth();
    //int h = visibleHeight();

    if (d->layoutSchedulingEnabled)
        layout();
#ifndef TDEHTML_NO_CARET
    else {
        hideCaret();
        recalcAndStoreCaretPos();
	showCaret();
    }/*end if*/
#endif

    TDEApplication::sendPostedEvents(viewport(), TQEvent::Paint);
}

// this is to get rid of a compiler virtual overload mismatch warning. do not remove
void TDEHTMLView::drawContents( TQPainter*)
{
}

void TDEHTMLView::drawContents( TQPainter *p, int ex, int ey, int ew, int eh )
{
#ifdef DEBUG_PIXEL

    if ( d->timer.elapsed() > 5000 ) {
        tqDebug( "drawed %d pixels in %d repaints the last %d milliseconds",
                d->pixelbooth, d->repaintbooth,  d->timer.elapsed() );
        d->timer.restart();
        d->pixelbooth = 0;
        d->repaintbooth = 0;
    }
    d->pixelbooth += ew*eh;
    d->repaintbooth++;
#endif

    //kdDebug( 6000 ) << "drawContents this="<< this <<" x=" << ex << ",y=" << ey << ",w=" << ew << ",h=" << eh << endl;
    if(!m_part || !m_part->xmlDocImpl() || !m_part->xmlDocImpl()->renderer()) {
        p->fillRect(ex, ey, ew, eh, palette().active().brush(TQColorGroup::Base));
        return;
    } else if ( d->complete && static_cast<RenderCanvas*>(m_part->xmlDocImpl()->renderer())->needsLayout() ) {
        // an external update request happens while we have a layout scheduled
        unscheduleRelayout();
        layout();
    }

    if (d->painting) {
        kdDebug( 6000 ) << "WARNING: drawContents reentered! " << endl;
        return;
    }
    d->painting = true;

    TQPoint pt = contentsToViewport(TQPoint(ex, ey));
    TQRegion cr = TQRect(pt.x(), pt.y(), ew, eh);

    // kdDebug(6000) << "clip rect: " << TQRect(pt.x(), pt.y(), ew, eh) << endl;
    for (TQPtrDictIterator<TQWidget> it(d->visibleWidgets); it.current(); ++it) {
	TQWidget *w = it.current();
	RenderWidget* rw = static_cast<RenderWidget*>( it.currentKey() );
	if (w && rw && !rw->isTDEHTMLWidget()) {
            int x, y;
            rw->absolutePosition(x, y);
            contentsToViewport(x, y, x, y);
            int pbx = rw->borderLeft()+rw->paddingLeft();
            int pby = rw->borderTop()+rw->paddingTop();
            TQRect g = TQRect(x+pbx, y+pby,
                            rw->width()-pbx-rw->borderRight()-rw->paddingRight(),
                            rw->height()-pby-rw->borderBottom()-rw->paddingBottom());
            if ( !rw->isFrame() && ((g.top() > pt.y()+eh) || (g.bottom() <= pt.y()) ||
                                    (g.right() <= pt.x()) || (g.left() > pt.x()+ew) ))
                continue;
            RenderLayer* rl = rw->needsMask() ? rw->enclosingStackingContext() : 0;
            TQRegion mask = rl ? rl->getMask() : TQRegion();
            if (!mask.isNull()) {
                TQPoint o(0,0);
                o = contentsToViewport(o);
                mask.translate(o.x(),o.y());
                mask = mask.intersect( TQRect(g.x(),g.y(),g.width(),g.height()) );
                cr -= mask;
            } else {
                cr -= g;
            }
        }
    }

#if 0
    // this is commonly the case with framesets. we still do
    // want to paint them, otherwise the widgets don't get placed.
    if (cr.isEmpty()) {
        d->painting = false;
	return;
    }
#endif

#ifndef DEBUG_NO_PAINT_BUFFER
    p->setClipRegion(cr);

    if (eh > PAINT_BUFFER_HEIGHT && ew <= 10) {
        if ( d->vertPaintBuffer->height() < visibleHeight() )
            d->vertPaintBuffer->resize(10, visibleHeight());
        d->tp->begin(d->vertPaintBuffer);
        d->tp->translate(-ex, -ey);
        d->tp->fillRect(ex, ey, ew, eh, palette().active().brush(TQColorGroup::Base));
        m_part->xmlDocImpl()->renderer()->layer()->paint(d->tp, TQRect(ex, ey, ew, eh));
        d->tp->end();
	p->drawPixmap(ex, ey, *d->vertPaintBuffer, 0, 0, ew, eh);
    }
    else {
        if ( d->paintBuffer->width() < visibleWidth() )
            d->paintBuffer->resize(visibleWidth(),PAINT_BUFFER_HEIGHT);

        int py=0;
        while (py < eh) {
            int ph = eh-py < PAINT_BUFFER_HEIGHT ? eh-py : PAINT_BUFFER_HEIGHT;
            d->tp->begin(d->paintBuffer);
            d->tp->translate(-ex, -ey-py);
            d->tp->fillRect(ex, ey+py, ew, ph, palette().active().brush(TQColorGroup::Base));
            m_part->xmlDocImpl()->renderer()->layer()->paint(d->tp, TQRect(ex, ey+py, ew, ph));
            d->tp->end();

	    p->drawPixmap(ex, ey+py, *d->paintBuffer, 0, 0, ew, ph);
            py += PAINT_BUFFER_HEIGHT;
        }
    }
#else // !DEBUG_NO_PAINT_BUFFER
static int cnt=0;
	ex = contentsX(); ey = contentsY();
	ew = visibleWidth(); eh = visibleHeight();
	TQRect pr(ex,ey,ew,eh);
	kdDebug() << "[" << ++cnt << "]" << " clip region: " << pr << endl;
//	p->setClipRegion(TQRect(0,0,ew,eh));
//        p->translate(-ex, -ey);
        p->fillRect(ex, ey, ew, eh, palette().active().brush(TQColorGroup::Base));
        m_part->xmlDocImpl()->renderer()->layer()->paint(p, pr);
#endif // DEBUG_NO_PAINT_BUFFER

#ifndef TDEHTML_NO_CARET
    if (d->m_caretViewContext && d->m_caretViewContext->visible) {
        TQRect pos(d->m_caretViewContext->x, d->m_caretViewContext->y,
		d->m_caretViewContext->width, d->m_caretViewContext->height);
        if (pos.intersects(TQRect(ex, ey, ew, eh))) {
            p->setRasterOp(XorROP);
	    p->setPen(white);
	    if (pos.width() == 1)
              p->drawLine(pos.topLeft(), pos.bottomRight());
	    else {
	      p->fillRect(pos, white);
	    }/*end if*/
	}/*end if*/
    }/*end if*/
#endif // TDEHTML_NO_CARET

//    p->setPen(TQPen(magenta,0,DashDotDotLine));
//    p->drawRect(dbg_paint_rect);

    tdehtml::DrawContentsEvent event( p, ex, ey, ew, eh );
    TQApplication::sendEvent( m_part, &event );

    d->painting = false;
}

void TDEHTMLView::setMarginWidth(int w)
{
    // make it update the rendering area when set
    _marginWidth = w;
}

void TDEHTMLView::setMarginHeight(int h)
{
    // make it update the rendering area when set
    _marginHeight = h;
}

void TDEHTMLView::layout()
{
    if( m_part && m_part->xmlDocImpl() ) {
        DOM::DocumentImpl *document = m_part->xmlDocImpl();

        tdehtml::RenderCanvas* canvas = static_cast<tdehtml::RenderCanvas *>(document->renderer());
        if ( !canvas ) return;

        d->layoutSchedulingEnabled=false;

        // the reference object for the overflow property on canvas
        RenderObject * ref = 0;
        RenderObject* root = document->documentElement() ? document->documentElement()->renderer() : 0;

        if (document->isHTMLDocument()) {
             NodeImpl *body = static_cast<HTMLDocumentImpl*>(document)->body();
             if(body && body->renderer() && body->id() == ID_FRAMESET) {
                 TQScrollView::setVScrollBarMode(AlwaysOff);
                 TQScrollView::setHScrollBarMode(AlwaysOff);
                 body->renderer()->setNeedsLayout(true);
//                  if (d->tooltip) {
//                      delete d->tooltip;
//                      d->tooltip = 0;
//                  }
             }
             else {
                 if (!d->tooltip)
                     d->tooltip = new TDEHTMLToolTip( this, d );
                 // only apply body's overflow to canvas if root as a visible overflow
                 if (root)
                     ref = (!body || root->style()->hidesOverflow()) ? root : body->renderer();
             }
        } else {
            ref = root;
        }
        if (ref) {
            if( ref->style()->overflowX() == OHIDDEN ) {
                if (d->hmode == Auto) TQScrollView::setHScrollBarMode(AlwaysOff);
            } else if (ref->style()->overflowX() == OSCROLL ) {
                if (d->hmode == Auto) TQScrollView::setHScrollBarMode(AlwaysOn);
            } else {
                if (TQScrollView::hScrollBarMode() == AlwaysOff) TQScrollView::setHScrollBarMode(d->hmode);
            } if ( ref->style()->overflowY() == OHIDDEN ) {
                if (d->vmode == Auto) TQScrollView::setVScrollBarMode(AlwaysOff);
            } else if (ref->style()->overflowY() == OSCROLL ) {
                if (d->vmode == Auto) TQScrollView::setVScrollBarMode(AlwaysOn);
            } else {
                if (TQScrollView::vScrollBarMode() == AlwaysOff) TQScrollView::setVScrollBarMode(d->vmode);
            }
        }
        d->needsFullRepaint = d->firstRelayout;
        if (_height !=  visibleHeight() || _width != visibleWidth()) {;
            d->needsFullRepaint = true;
            _height = visibleHeight();
            _width = visibleWidth();
        }
        //TQTime qt;
        //qt.start();
        canvas->layout();

        emit finishedLayout();
        if (d->firstRelayout) {
            // make sure firstRelayout is set to false now in case this layout
            // wasn't scheduled
            d->firstRelayout = false;
            verticalScrollBar()->setEnabled( true );
            horizontalScrollBar()->setEnabled( true );
        }
#if 0
    ElementImpl *listitem = m_part->xmlDocImpl()->getElementById("__test_element__");
    if (listitem) kdDebug(6000) << "after layout, before repaint" << endl;
    if (listitem) dumpLineBoxes(static_cast<RenderFlow *>(listitem->renderer()));
#endif
#ifndef TDEHTML_NO_CARET
        hideCaret();
        if ((m_part->isCaretMode() || m_part->isEditable())
        	&& !d->complete && d->m_caretViewContext
                && !d->m_caretViewContext->caretMoved) {
            initCaret();
        } else {
	    recalcAndStoreCaretPos();
	    showCaret();
        }/*end if*/
#endif
        if (d->accessKeysEnabled && d->accessKeysActivated) {
            emit hideAccessKeys();
            displayAccessKeys();
        }
        //kdDebug( 6000 ) << "TIME: layout() dt=" << qt.elapsed() << endl;
    }
    else
       _width = visibleWidth();

    killTimer(d->layoutTimerId);
    d->layoutTimerId = 0;
    d->layoutSchedulingEnabled=true;
}

void TDEHTMLView::closeChildDialogs()
{
    TQObjectList *dlgs = queryList("TQDialog");
    for (TQObject *dlg = dlgs->first(); dlg; dlg = dlgs->next())
    {
        KDialogBase* dlgbase = dynamic_cast<KDialogBase *>( dlg );
        if ( dlgbase ) {
            if ( dlgbase->testWFlags( WShowModal ) ) {
                kdDebug(6000) << "closeChildDialogs: closing dialog " << dlgbase << endl;
                // close() ends up calling TQButton::animateClick, which isn't immediate
                // we need something the exits the event loop immediately (#49068)
                dlgbase->cancel();
            }
        }
        else
        {
            kdWarning() << "closeChildDialogs: not a KDialogBase! Don't use QDialogs in KDE! " << static_cast<TQWidget*>(dlg) << endl;
            static_cast<TQWidget*>(dlg)->hide();
        }
    }
    delete dlgs;
    d->m_dialogsAllowed = false;
}

bool TDEHTMLView::dialogsAllowed() {
    bool allowed = d->m_dialogsAllowed;
    TDEHTMLPart* p = m_part->parentPart();
    if (p && p->view())
        allowed &= p->view()->dialogsAllowed();
    return allowed;
}

void TDEHTMLView::closeEvent( TQCloseEvent* ev )
{
    closeChildDialogs();
    TQScrollView::closeEvent( ev );
}

//
// Event Handling
//
/////////////////

void TDEHTMLView::viewportMousePressEvent( TQMouseEvent *_mouse )
{
    if (!m_part->xmlDocImpl()) return;
    if (d->possibleTripleClick && ( _mouse->button() & TQt::MouseButtonMask ) == TQt::LeftButton)
    {
        viewportMouseDoubleClickEvent( _mouse ); // it handles triple clicks too
        return;
    }

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);
    //kdDebug( 6000 ) << "mousePressEvent: viewport=("<<_mouse->x()<<"/"<<_mouse->y()<<"), contents=(" << xm << "/" << ym << ")\n";

    d->isDoubleClick = false;

    DOM::NodeImpl::MouseEvent mev( _mouse->stateAfter(), DOM::NodeImpl::MousePress );
    m_part->xmlDocImpl()->prepareMouseEvent( false, xm, ym, &mev );

    //kdDebug(6000) << "innerNode="<<mev.innerNode.nodeName().string()<<endl;

    if ( (_mouse->button() == TQt::MidButton) &&
          !m_part->d->m_bOpenMiddleClick && !d->m_mouseScrollTimer &&
          mev.url.isNull() && (mev.innerNode.elementId() != ID_INPUT) ) {
        TQPoint point = mapFromGlobal( _mouse->globalPos() );

        d->m_mouseScroll_byX = 0;
        d->m_mouseScroll_byY = 0;

        d->m_mouseScrollTimer = new TQTimer( this );
        connect( d->m_mouseScrollTimer, TQ_SIGNAL(timeout()), this, TQ_SLOT(slotMouseScrollTimer()) );

        if ( !d->m_mouseScrollIndicator ) {
            TQPixmap pixmap, icon;
            pixmap.resize( 48, 48 );
            pixmap.fill( TQColor( tqRgba( 127, 127, 127, 127 ) ) );

            TQPainter p( &pixmap );
            icon = TDEGlobal::iconLoader()->loadIcon( "1uparrow", TDEIcon::Small );
            p.drawPixmap( 16, 0, icon );
            icon = TDEGlobal::iconLoader()->loadIcon( "1leftarrow", TDEIcon::Small );
            p.drawPixmap( 0, 16, icon );
            icon = TDEGlobal::iconLoader()->loadIcon( "1downarrow", TDEIcon::Small );
            p.drawPixmap( 16, 32,icon  );
            icon = TDEGlobal::iconLoader()->loadIcon( "1rightarrow", TDEIcon::Small );
            p.drawPixmap( 32, 16, icon );
            p.drawEllipse( 23, 23, 2, 2 );

            d->m_mouseScrollIndicator = new TQWidget( this, 0 );
            d->m_mouseScrollIndicator->setFixedSize( 48, 48 );
            d->m_mouseScrollIndicator->setPaletteBackgroundPixmap( pixmap );
        }
        d->m_mouseScrollIndicator->move( point.x()-24, point.y()-24 );

        bool hasHorBar = visibleWidth() < contentsWidth();
        bool hasVerBar = visibleHeight() < contentsHeight();

        TDEConfig *config = TDEGlobal::config();
        TDEConfigGroupSaver saver( config, "HTML Settings" );
        if ( config->readBoolEntry( "ShowMouseScrollIndicator", true ) ) {
            d->m_mouseScrollIndicator->show();
            d->m_mouseScrollIndicator->unsetCursor();

            TQBitmap mask = d->m_mouseScrollIndicator->paletteBackgroundPixmap()->createHeuristicMask( true );

	    if ( hasHorBar && !hasVerBar ) {
                TQBitmap bm( 16, 16, true );
                bitBlt( &mask, 16,  0, &bm, 0, 0, -1, -1 );
                bitBlt( &mask, 16, 32, &bm, 0, 0, -1, -1 );
                d->m_mouseScrollIndicator->setCursor( KCursor::SizeHorCursor );
            }
            else if ( !hasHorBar && hasVerBar ) {
                TQBitmap bm( 16, 16, true );
                bitBlt( &mask,  0, 16, &bm, 0, 0, -1, -1 );
                bitBlt( &mask, 32, 16, &bm, 0, 0, -1, -1 );
                d->m_mouseScrollIndicator->setCursor( KCursor::SizeVerCursor );
            }
            else
                d->m_mouseScrollIndicator->setCursor( KCursor::SizeAllCursor );

            d->m_mouseScrollIndicator->setMask( mask );
        }
        else {
            if ( hasHorBar && !hasVerBar )
                viewport()->setCursor( KCursor::SizeHorCursor );
            else if ( !hasHorBar && hasVerBar )
                viewport()->setCursor( KCursor::SizeVerCursor );
            else
                viewport()->setCursor( KCursor::SizeAllCursor );
        }

        return;
    }
    else if ( d->m_mouseScrollTimer ) {
        delete d->m_mouseScrollTimer;
        d->m_mouseScrollTimer = 0;

        if ( d->m_mouseScrollIndicator )
            d->m_mouseScrollIndicator->hide();
    }

	d->clickCount = 1;
	d->clickX = xm;
	d->clickY = ym;

    bool swallowEvent = dispatchMouseEvent(EventImpl::MOUSEDOWN_EVENT,mev.innerNode.handle(),mev.innerNonSharedNode.handle(),true,
                                           d->clickCount,_mouse,true,DOM::NodeImpl::MousePress);

    tdehtml::RenderObject* r = mev.innerNode.handle() ? mev.innerNode.handle()->renderer() : 0;
    if (r && r->isWidget())
	_mouse->ignore();

    if (!swallowEvent) {
	emit m_part->nodeActivated(mev.innerNode);

	tdehtml::MousePressEvent event( _mouse, xm, ym, mev.url, mev.target, mev.innerNode );
        TQApplication::sendEvent( m_part, &event );
        // we might be deleted after this
    }
}

void TDEHTMLView::viewportMouseDoubleClickEvent( TQMouseEvent *_mouse )
{
    if(!m_part->xmlDocImpl()) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    kdDebug( 6000 ) << "mouseDblClickEvent: x=" << xm << ", y=" << ym << endl;

    d->isDoubleClick = true;

    DOM::NodeImpl::MouseEvent mev( _mouse->stateAfter(), DOM::NodeImpl::MouseDblClick );
    m_part->xmlDocImpl()->prepareMouseEvent( false, xm, ym, &mev );

    // We do the same thing as viewportMousePressEvent() here, since the DOM does not treat
    // single and double-click events as separate (only the detail, i.e. number of clicks differs)
    if (d->clickCount > 0 &&
        TQPoint(d->clickX-xm,d->clickY-ym).manhattanLength() <= TQApplication::startDragDistance())
	d->clickCount++;
    else { // shouldn't happen, if Qt has the same criterias for double clicks.
	d->clickCount = 1;
	d->clickX = xm;
	d->clickY = ym;
    }
    bool swallowEvent = dispatchMouseEvent(EventImpl::MOUSEDOWN_EVENT,mev.innerNode.handle(),mev.innerNonSharedNode.handle(),true,
                                           d->clickCount,_mouse,true,DOM::NodeImpl::MouseDblClick);

    tdehtml::RenderObject* r = mev.innerNode.handle() ? mev.innerNode.handle()->renderer() : 0;
    if (r && r->isWidget())
	_mouse->ignore();

    if (!swallowEvent) {
	tdehtml::MouseDoubleClickEvent event( _mouse, xm, ym, mev.url, mev.target, mev.innerNode, d->clickCount );
	TQApplication::sendEvent( m_part, &event );
    }

    d->possibleTripleClick=true;
    TQTimer::singleShot(TQApplication::doubleClickInterval(),this,TQ_SLOT(tripleClickTimeout()));
}

void TDEHTMLView::tripleClickTimeout()
{
    d->possibleTripleClick = false;
    d->clickCount = 0;
}

static inline void forwardPeripheralEvent(tdehtml::RenderWidget* r, TQMouseEvent* me, int x, int y)
{
    int absx = 0;
    int absy = 0;
    r->absolutePosition(absx, absy);
    TQPoint p(x-absx, y-absy);
    TQMouseEvent fw(me->type(), p, me->button(), me->state());
    TQWidget* w = r->widget();
    TQScrollView* sc = ::tqt_cast<TQScrollView*>(w);
    if (sc && !::tqt_cast<TQListBox*>(w))
        static_cast<tdehtml::RenderWidget::ScrollViewEventPropagator*>(sc)->sendEvent(static_cast<TQEvent*>(&fw));
    else if(w)
        static_cast<tdehtml::RenderWidget::EventPropagator*>(w)->sendEvent(static_cast<TQEvent*>(&fw));
}


static bool targetOpensNewWindow(TDEHTMLPart *part, TQString target)
{
    if (!target.isEmpty() && (target.lower() != "_top") &&
       (target.lower() != "_self") && (target.lower() != "_parent")) {
        if (target.lower() == "_blank")
            return true;
        else {
            while (part->parentPart())
                part = part->parentPart();
            if (!part->frameExists(target))
                return true;
        }
    }
    return false;
}

void TDEHTMLView::viewportMouseMoveEvent( TQMouseEvent * _mouse )
{
    if ( d->m_mouseScrollTimer ) {
        TQPoint point = mapFromGlobal( _mouse->globalPos() );

        int deltaX = point.x() - d->m_mouseScrollIndicator->x() - 24;
        int deltaY = point.y() - d->m_mouseScrollIndicator->y() - 24;

        (deltaX > 0) ? d->m_mouseScroll_byX = 1 : d->m_mouseScroll_byX = -1;
        (deltaY > 0) ? d->m_mouseScroll_byY = 1 : d->m_mouseScroll_byY = -1;

        double adX = TQABS(deltaX)/30.0;
        double adY = TQABS(deltaY)/30.0;

        d->m_mouseScroll_byX = kMax(kMin(d->m_mouseScroll_byX * int(adX*adX), SHRT_MAX), SHRT_MIN);
        d->m_mouseScroll_byY = kMax(kMin(d->m_mouseScroll_byY * int(adY*adY), SHRT_MAX), SHRT_MIN);

        if (d->m_mouseScroll_byX == 0 && d->m_mouseScroll_byY == 0) {
            d->m_mouseScrollTimer->stop();
        }
        else if (!d->m_mouseScrollTimer->isActive()) {
            d->m_mouseScrollTimer->changeInterval( 20 );
        }
    }

    if(!m_part->xmlDocImpl()) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    DOM::NodeImpl::MouseEvent mev( _mouse->stateAfter(), DOM::NodeImpl::MouseMove );
    // Do not modify :hover/:active state while mouse is pressed.
    m_part->xmlDocImpl()->prepareMouseEvent( _mouse->state() & TQt::MouseButtonMask /*readonly ?*/, xm, ym, &mev );

//     kdDebug(6000) << "mouse move: " << _mouse->pos()
// 		  << " button " << _mouse->button()
// 		  << " state " << _mouse->state() << endl;

    bool swallowEvent = dispatchMouseEvent(EventImpl::MOUSEMOVE_EVENT,mev.innerNode.handle(),mev.innerNonSharedNode.handle(),false,
                                           0,_mouse,true,DOM::NodeImpl::MouseMove);

    if (d->clickCount > 0 &&
        TQPoint(d->clickX-xm,d->clickY-ym).manhattanLength() > TQApplication::startDragDistance()) {
	d->clickCount = 0;  // moving the mouse outside the threshold invalidates the click
    }

    // execute the scheduled script. This is to make sure the mouseover events come after the mouseout events
    m_part->executeScheduledScript();

    DOM::NodeImpl* fn = m_part->xmlDocImpl()->focusNode();
    if (fn && fn != mev.innerNode.handle() &&
        fn->renderer() && fn->renderer()->isWidget()) {
        forwardPeripheralEvent(static_cast<tdehtml::RenderWidget*>(fn->renderer()), _mouse, xm, ym);
    }

    tdehtml::RenderObject* r = mev.innerNode.handle() ? mev.innerNode.handle()->renderer() : 0;
    tdehtml::RenderStyle* style = (r && r->style()) ? r->style() : 0;
    TQCursor c;
    bool mailtoCursor = false;
    bool newWindowCursor = false;
    switch ( style ? style->cursor() : CURSOR_AUTO) {
    case CURSOR_AUTO:
        if ( r && r->isText() )
            c = KCursor::ibeamCursor();
        if ( mev.url.length() && m_part->settings()->changeCursor() ) {
            c = m_part->urlCursor();
	    if (mev.url.string().startsWith("mailto:") && mev.url.string().find('@')>0)
                mailtoCursor = true;
            else
                newWindowCursor = targetOpensNewWindow( m_part, mev.target.string() );
        }

        if (r && r->isFrameSet() && !static_cast<RenderFrameSet*>(r)->noResize())
            c = TQCursor(static_cast<RenderFrameSet*>(r)->cursorShape());

        break;
    case CURSOR_CROSS:
        c = KCursor::crossCursor();
        break;
    case CURSOR_POINTER:
        c = m_part->urlCursor();
	if (mev.url.string().startsWith("mailto:") && mev.url.string().find('@')>0)
            mailtoCursor = true;
        else
            newWindowCursor = targetOpensNewWindow( m_part, mev.target.string() );
        break;
    case CURSOR_PROGRESS:
        c = KCursor::workingCursor();
        break;
    case CURSOR_MOVE:
        c = KCursor::sizeAllCursor();
        break;
    case CURSOR_E_RESIZE:
    case CURSOR_W_RESIZE:
        c = KCursor::sizeHorCursor();
        break;
    case CURSOR_N_RESIZE:
    case CURSOR_S_RESIZE:
        c = KCursor::sizeVerCursor();
        break;
    case CURSOR_NE_RESIZE:
    case CURSOR_SW_RESIZE:
        c = KCursor::sizeBDiagCursor();
        break;
    case CURSOR_NW_RESIZE:
    case CURSOR_SE_RESIZE:
        c = KCursor::sizeFDiagCursor();
        break;
    case CURSOR_TEXT:
        c = KCursor::ibeamCursor();
        break;
    case CURSOR_WAIT:
        c = KCursor::waitCursor();
        break;
    case CURSOR_HELP:
        c = KCursor::whatsThisCursor();
        break;
    case CURSOR_DEFAULT:
        break;
    }

    if ( viewport()->cursor().handle() != c.handle() ) {
        if( c.handle() == KCursor::arrowCursor().handle()) {
            for (TDEHTMLPart* p = m_part; p; p = p->parentPart())
                p->view()->viewport()->unsetCursor();
        }
        else {
            viewport()->setCursor( c );
        }
    }

    if ( ( mailtoCursor || newWindowCursor ) && isVisible() && hasFocus() ) {
#ifdef TQ_WS_X11
        TQPixmap icon_pixmap = TDEGlobal::iconLoader()->loadIcon( mailtoCursor ? "mail_generic" : "window-new", TDEIcon::Small, 0, TDEIcon::DefaultState, 0, true );

        if (d->cursor_icon_widget) {
            const TQPixmap *pm = d->cursor_icon_widget->backgroundPixmap();
            if (!pm || pm->serialNumber()!=icon_pixmap.serialNumber()) {
                delete d->cursor_icon_widget;
                d->cursor_icon_widget = 0;
            }
        }

        if( !d->cursor_icon_widget ) {
            d->cursor_icon_widget = new TQWidget( NULL, NULL, WX11BypassWM );
            XSetWindowAttributes attr;
            attr.save_under = True;
            XChangeWindowAttributes( tqt_xdisplay(), d->cursor_icon_widget->winId(), CWSaveUnder, &attr );
            d->cursor_icon_widget->resize( icon_pixmap.width(), icon_pixmap.height());
            if( icon_pixmap.mask() )
                d->cursor_icon_widget->setMask( *icon_pixmap.mask());
            else
                d->cursor_icon_widget->clearMask();
            d->cursor_icon_widget->setBackgroundPixmap( icon_pixmap );
            d->cursor_icon_widget->erase();
        }
        TQPoint c_pos = TQCursor::pos();
        d->cursor_icon_widget->move( c_pos.x() + 15, c_pos.y() + 15 );
        XRaiseWindow( tqt_xdisplay(), d->cursor_icon_widget->winId());
        TQApplication::flushX();
        d->cursor_icon_widget->show();
#endif
    }
    else if ( d->cursor_icon_widget )
        d->cursor_icon_widget->hide();

    if (r && r->isWidget()) {
	_mouse->ignore();
    }


    d->prevMouseX = xm;
    d->prevMouseY = ym;

    if (!swallowEvent) {
        tdehtml::MouseMoveEvent event( _mouse, xm, ym, mev.url, mev.target, mev.innerNode );
        TQApplication::sendEvent( m_part, &event );
    }
}

void TDEHTMLView::viewportMouseReleaseEvent( TQMouseEvent * _mouse )
{
    bool swallowEvent = false;
    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);
    DOM::NodeImpl::MouseEvent mev( _mouse->stateAfter(), DOM::NodeImpl::MouseRelease );

    if ( m_part->xmlDocImpl() )
    {
        m_part->xmlDocImpl()->prepareMouseEvent( false, xm, ym, &mev );

        swallowEvent = dispatchMouseEvent(EventImpl::MOUSEUP_EVENT,mev.innerNode.handle(),mev.innerNonSharedNode.handle(),true,
                                          d->clickCount,_mouse,false,DOM::NodeImpl::MouseRelease);

        if (d->clickCount > 0 &&
            TQPoint(d->clickX-xm,d->clickY-ym).manhattanLength() <= TQApplication::startDragDistance()) {
            TQMouseEvent me(d->isDoubleClick ? TQEvent::MouseButtonDblClick : TQEvent::MouseButtonRelease,
                           _mouse->pos(), _mouse->button(), _mouse->state());
            dispatchMouseEvent(EventImpl::CLICK_EVENT, mev.innerNode.handle(),mev.innerNonSharedNode.handle(),true,
                               d->clickCount, &me, true, DOM::NodeImpl::MouseRelease);
        }

        DOM::NodeImpl* fn = m_part->xmlDocImpl()->focusNode();
        if (fn && fn != mev.innerNode.handle() &&
            fn->renderer() && fn->renderer()->isWidget() &&
            _mouse->button() != TQt::MidButton) {
            forwardPeripheralEvent(static_cast<tdehtml::RenderWidget*>(fn->renderer()), _mouse, xm, ym);
        }

        tdehtml::RenderObject* r = mev.innerNode.handle() ? mev.innerNode.handle()->renderer() : 0;
        if (r && r->isWidget())
            _mouse->ignore();
    }

    if (!swallowEvent) {
	tdehtml::MouseReleaseEvent event( _mouse, xm, ym, mev.url, mev.target, mev.innerNode );
	TQApplication::sendEvent( m_part, &event );
    }
}

// returns true if event should be swallowed
bool TDEHTMLView::dispatchKeyEvent( TQKeyEvent *_ke )
{
    if (!m_part->xmlDocImpl())
        return false;
    // Pressing and releasing a key should generate keydown, keypress and keyup events
    // Holding it down should generated keydown, keypress (repeatedly) and keyup events
    // The problem here is that Qt generates two autorepeat events (keyrelease+keypress)
    // for autorepeating, while DOM wants only one autorepeat event (keypress), so one
    // of the Qt events shouldn't be passed to DOM, but it should be still filtered
    // out if DOM would filter the autorepeat event. Additional problem is that Qt keyrelease
    // events don't have text() set (Qt bug?), so DOM often would ignore the keypress event
    // if it was created using Qt keyrelease, but Qt autorepeat keyrelease comes
    // before Qt autorepeat keypress (i.e. problem whether to filter it out or not).
    // The solution is to filter out and postpone the Qt autorepeat keyrelease until
    // the following Qt keypress event comes. If DOM accepts the DOM keypress event,
    // the postponed event will be simply discarded. If not, it will be passed to keyPressEvent()
    // again, and here it will be ignored.
    //
    //  Qt:      Press      | Release(autorepeat) Press(autorepeat) etc. |   Release
    //  DOM:   Down + Press |      (nothing)           Press             |     Up

    // It's also possible to get only Releases. E.g. the release of alt-tab,
    // or when the keypresses get captured by an accel.

    if( _ke == d->postponed_autorepeat ) // replayed event
    {
        return false;
    }

    if( _ke->type() == TQEvent::KeyPress )
    {
        if( !_ke->isAutoRepeat())
        {
            bool ret = dispatchKeyEventHelper( _ke, false ); // keydown
            // don't send keypress even if keydown was blocked, like IE (and unlike Mozilla)
            if( !ret && dispatchKeyEventHelper( _ke, true )) // keypress
                ret = true;
            return ret;
        }
        else // autorepeat
        {
            bool ret = dispatchKeyEventHelper( _ke, true ); // keypress
            if( !ret && d->postponed_autorepeat )
                keyPressEvent( d->postponed_autorepeat );
            delete d->postponed_autorepeat;
            d->postponed_autorepeat = NULL;
            return ret;
        }
    }
    else // TQEvent::KeyRelease
    {
        // Discard postponed "autorepeat key-release" events that didn't see
        // a keypress after them (e.g. due to TQAccel)
        if ( d->postponed_autorepeat ) {
            delete d->postponed_autorepeat;
            d->postponed_autorepeat = 0;
        }

        if( !_ke->isAutoRepeat()) {
            return dispatchKeyEventHelper( _ke, false ); // keyup
        }
        else
        {
            d->postponed_autorepeat = new TQKeyEvent( _ke->type(), _ke->key(), _ke->ascii(), _ke->state(),
                _ke->text(), _ke->isAutoRepeat(), _ke->count());
            if( _ke->isAccepted())
                d->postponed_autorepeat->accept();
            else
                d->postponed_autorepeat->ignore();
            return true;
        }
    }
}

// returns true if event should be swallowed
bool TDEHTMLView::dispatchKeyEventHelper( TQKeyEvent *_ke, bool keypress )
{
    DOM::NodeImpl* keyNode = m_part->xmlDocImpl()->focusNode();
    if (keyNode) {
        return keyNode->dispatchKeyEvent(_ke, keypress);
    } else { // no focused node, send to document
        return m_part->xmlDocImpl()->dispatchKeyEvent(_ke, keypress);
    }
}

void TDEHTMLView::keyPressEvent( TQKeyEvent *_ke )
{
#ifndef TDEHTML_NO_TYPE_AHEAD_FIND
	if(d->typeAheadActivated)
	{
		// type-ahead find aka find-as-you-type
		if(_ke->key() == Key_BackSpace)
		{
			d->findString = d->findString.left(d->findString.length() - 1);

			if(!d->findString.isEmpty())
			{
				findAhead(false);
			}
			else
			{
				findTimeout();
			}

			d->timer.start(3000, true);
			_ke->accept();
			return;
		}
		else if(_ke->key() == Key_Escape)
		{
			findTimeout();

			_ke->accept();
			return;
		}
		else if(_ke->key() == Key_Space || !TQString(_ke->text()).stripWhiteSpace().isEmpty())
		{
			d->findString += _ke->text();

			findAhead(true);

			d->timer.start(3000, true);
			_ke->accept();
			return;
		}
	}
#endif // TDEHTML_NO_TYPE_AHEAD_FIND

#ifndef TDEHTML_NO_CARET
    if (m_part->isEditable() || m_part->isCaretMode()
        || (m_part->xmlDocImpl() && m_part->xmlDocImpl()->focusNode()
	    && m_part->xmlDocImpl()->focusNode()->contentEditable())) {
      d->caretViewContext()->keyReleasePending = true;
      caretKeyPressEvent(_ke);
      return;
    }
#endif // TDEHTML_NO_CARET

    // If CTRL was hit, be prepared for access keys
    if (d->accessKeysEnabled && _ke->key() == Key_Control && _ke->state()==0 && !d->accessKeysActivated)
    {
        d->accessKeysPreActivate=true;
        _ke->accept();
        return;
    }

    if (_ke->key() == Key_Shift && _ke->state()==0)
	    d->scrollSuspendPreActivate=true;

    // accesskey handling needs to be done before dispatching, otherwise e.g. lineedits
    // may eat the event

    if (d->accessKeysEnabled && d->accessKeysActivated)
    {
        int state = ( _ke->state() & ( ShiftButton | ControlButton | AltButton | MetaButton ));
        if ( state==0 || state==ShiftButton) {
	if (_ke->key() != Key_Shift) accessKeysTimeout();
        handleAccessKey( _ke );
        _ke->accept();
        return;
    	}
	accessKeysTimeout();
    }

    if ( dispatchKeyEvent( _ke )) {
        // If either keydown or keypress was accepted by a widget, or canceled by JS, stop here.
        _ke->accept();
        return;
    }

    int offs = (clipper()->height() < 30) ? clipper()->height() : 30;
    if (_ke->state() & TQt::ShiftButton)
      switch(_ke->key())
        {
        case Key_Space:
            scrollBy( 0, -clipper()->height() + offs );
            if(d->scrollSuspended)
                d->newScrollTimer(this, 0);
            break;

        case Key_Down:
        case Key_J:
            d->adjustScroller(this, TDEHTMLViewPrivate::ScrollDown, TDEHTMLViewPrivate::ScrollUp);
            break;

        case Key_Up:
        case Key_K:
            d->adjustScroller(this, TDEHTMLViewPrivate::ScrollUp, TDEHTMLViewPrivate::ScrollDown);
            break;

        case Key_Left:
        case Key_H:
            d->adjustScroller(this, TDEHTMLViewPrivate::ScrollLeft, TDEHTMLViewPrivate::ScrollRight);
            break;

        case Key_Right:
        case Key_L:
            d->adjustScroller(this, TDEHTMLViewPrivate::ScrollRight, TDEHTMLViewPrivate::ScrollLeft);
            break;
        }
    else
        switch ( _ke->key() )
        {
        case Key_Down:
        case Key_J:
            if (!d->scrollTimerId || d->scrollSuspended)
                scrollBy( 0, 10 * _ke->count() );
            if (d->scrollTimerId)
                d->newScrollTimer(this, 0);
            break;

        case Key_Space:
        case Key_Next:
            scrollBy( 0, clipper()->height() - offs );
            if(d->scrollSuspended)
                d->newScrollTimer(this, 0);
            break;

        case Key_Up:
        case Key_K:
            if (!d->scrollTimerId || d->scrollSuspended)
                scrollBy( 0, -10 * _ke->count());
            if (d->scrollTimerId)
                d->newScrollTimer(this, 0);
            break;

        case Key_Prior:
            scrollBy( 0, -clipper()->height() + offs );
            if(d->scrollSuspended)
                d->newScrollTimer(this, 0);
            break;
        case Key_Right:
        case Key_L:
            if (!d->scrollTimerId || d->scrollSuspended)
                scrollBy( 10 * _ke->count(), 0 );
            if (d->scrollTimerId)
                d->newScrollTimer(this, 0);
            break;
        case Key_Left:
        case Key_H:
            if (!d->scrollTimerId || d->scrollSuspended)
                scrollBy( -10 * _ke->count(), 0 );
            if (d->scrollTimerId)
                d->newScrollTimer(this, 0);
            break;
        case Key_Enter:
        case Key_Return:
	    // ### FIXME:
	    // or even better to HTMLAnchorElementImpl::event()
            if (m_part->xmlDocImpl()) {
		NodeImpl *n = m_part->xmlDocImpl()->focusNode();
		if (n)
		    n->setActive();
	    }
            break;
        case Key_Home:
            setContentsPos( 0, 0 );
            if(d->scrollSuspended)
                d->newScrollTimer(this, 0);
            break;
        case Key_End:
            setContentsPos( 0, contentsHeight() - visibleHeight() );
            if(d->scrollSuspended)
                d->newScrollTimer(this, 0);
            break;
        case Key_Shift:
            // what are you doing here?
	    _ke->ignore();
            return;
        default:
            if (d->scrollTimerId)
                d->newScrollTimer(this, 0);
	    _ke->ignore();
            return;
        }

    _ke->accept();
}

void TDEHTMLView::findTimeout()
{
#ifndef TDEHTML_NO_TYPE_AHEAD_FIND
	d->typeAheadActivated = false;
	d->findString = "";
	m_part->setStatusBarText(i18n("Find stopped."), TDEHTMLPart::BarDefaultText);
	m_part->enableFindAheadActions( true );
#endif // TDEHTML_NO_TYPE_AHEAD_FIND
}

#ifndef TDEHTML_NO_TYPE_AHEAD_FIND
void TDEHTMLView::startFindAhead( bool linksOnly )
{
	if( linksOnly )
	{
		d->findLinksOnly = true;
		m_part->setStatusBarText(i18n("Starting -- find links as you type"),
		                         TDEHTMLPart::BarDefaultText);
	}
	else
	{
		d->findLinksOnly = false;
		m_part->setStatusBarText(i18n("Starting -- find text as you type"),
		                         TDEHTMLPart::BarDefaultText);
	}

	m_part->findTextBegin();
	d->typeAheadActivated = true;
        // disable, so that the shortcut ( / or ' by default ) doesn't interfere
	m_part->enableFindAheadActions( false );
	d->timer.start(3000, true);
}

void TDEHTMLView::findAhead(bool increase)
{
	TQString status;

	if(d->findLinksOnly)
	{
		m_part->findText(d->findString, TDEHTMLPart::FindNoPopups |
		                 TDEHTMLPart::FindLinksOnly, this);
		if(m_part->findTextNext())
		{
			status = i18n("Link found: \"%1\".");
		}
		else
		{
			if(increase) KNotifyClient::beep();
			status = i18n("Link not found: \"%1\".");
		}
	}
	else
	{
		m_part->findText(d->findString, TDEHTMLPart::FindNoPopups, this);
		if(m_part->findTextNext())
		{
			status = i18n("Text found: \"%1\".");
		}
		else
		{
			if(increase) KNotifyClient::beep();
			status = i18n("Text not found: \"%1\".");
		}
	}

	m_part->setStatusBarText(status.arg(d->findString.lower()),
	                         TDEHTMLPart::BarDefaultText);
}

void TDEHTMLView::updateFindAheadTimeout()
{
    if( d->typeAheadActivated )
        d->timer.start( 3000, true );
}

#endif // TDEHTML_NO_TYPE_AHEAD_FIND

void TDEHTMLView::keyReleaseEvent(TQKeyEvent *_ke)
{
#ifndef TDEHTML_NO_TYPE_AHEAD_FIND
    if(d->typeAheadActivated) {
        _ke->accept();
        return;
    }
#endif
    if (d->m_caretViewContext && d->m_caretViewContext->keyReleasePending) {
        //caretKeyReleaseEvent(_ke);
	d->m_caretViewContext->keyReleasePending = false;
	return;
    }

    if( d->scrollSuspendPreActivate && _ke->key() != Key_Shift )
        d->scrollSuspendPreActivate = false;
    if( _ke->key() == Key_Shift && d->scrollSuspendPreActivate && _ke->state() == TQt::ShiftButton
        && !(TDEApplication::keyboardMouseState() & TQt::ShiftButton))
    {
        if (d->scrollTimerId)
        {
            d->scrollSuspended = !d->scrollSuspended;
#ifndef NO_SMOOTH_SCROLL_HACK
            if( d->scrollSuspended )
                stopScrolling();
#endif
        }
    }

    if (d->accessKeysEnabled)
    {
        if (d->accessKeysPreActivate && _ke->key() != Key_Control)
            d->accessKeysPreActivate=false;
        if (d->accessKeysPreActivate && _ke->state() == TQt::ControlButton && !(TDEApplication::keyboardMouseState() & TQt::ControlButton))
        {
	    displayAccessKeys();
	    m_part->setStatusBarText(i18n("Access Keys activated"),TDEHTMLPart::BarOverrideText);
	    d->accessKeysActivated = true;
	    d->accessKeysPreActivate = false;
            _ke->accept();
            return;
        }
	else if (d->accessKeysActivated)
        {
            accessKeysTimeout();
            _ke->accept();
            return;
        }
    }

    // Send keyup event
    if ( dispatchKeyEvent( _ke ) )
    {
        _ke->accept();
        return;
    }

    TQScrollView::keyReleaseEvent(_ke);
}

void TDEHTMLView::contentsContextMenuEvent ( TQContextMenuEvent * /*ce*/ )
{
// ### what kind of c*** is that ?
#if 0
    if (!m_part->xmlDocImpl()) return;
    int xm = _ce->x();
    int ym = _ce->y();

    DOM::NodeImpl::MouseEvent mev( _ce->state(), DOM::NodeImpl::MouseMove ); // ### not a mouse event!
    m_part->xmlDocImpl()->prepareMouseEvent( xm, ym, &mev );

    NodeImpl *targetNode = mev.innerNode.handle();
    if (targetNode && targetNode->renderer() && targetNode->renderer()->isWidget()) {
        int absx = 0;
        int absy = 0;
        targetNode->renderer()->absolutePosition(absx,absy);
        TQPoint pos(xm-absx,ym-absy);

        TQWidget *w = static_cast<RenderWidget*>(targetNode->renderer())->widget();
        TQContextMenuEvent cme(_ce->reason(),pos,_ce->globalPos(),_ce->state());
        setIgnoreEvents(true);
        TQApplication::sendEvent(w,&cme);
        setIgnoreEvents(false);
    }
#endif
}

bool TDEHTMLView::focusNextPrevChild( bool next )
{
    // Now try to find the next child
    if (m_part->xmlDocImpl() && focusNextPrevNode(next))
    {
	if (m_part->xmlDocImpl()->focusNode())
	    kdDebug() << "focusNode.name: "
		      << m_part->xmlDocImpl()->focusNode()->nodeName().string() << endl;
	return true; // focus node found
    }

    // If we get here, pass tabbing control up to the next/previous child in our parent
    d->pseudoFocusNode = TDEHTMLViewPrivate::PFNone;
    if (m_part->parentPart() && m_part->parentPart()->view())
        return m_part->parentPart()->view()->focusNextPrevChild(next);

    return TQWidget::focusNextPrevChild(next);
}

void TDEHTMLView::doAutoScroll()
{
    TQPoint pos = TQCursor::pos();
    pos = viewport()->mapFromGlobal( pos );

    int xm, ym;
    viewportToContents(pos.x(), pos.y(), xm, ym);

    pos = TQPoint(pos.x() - viewport()->x(), pos.y() - viewport()->y());
    if ( (pos.y() < 0) || (pos.y() > visibleHeight()) ||
         (pos.x() < 0) || (pos.x() > visibleWidth()) )
    {
        ensureVisible( xm, ym, 0, 5 );

#ifndef TDEHTML_NO_SELECTION
        // extend the selection while scrolling
	DOM::Node innerNode;
	if (m_part->isExtendingSelection()) {
            RenderObject::NodeInfo renderInfo(true/*readonly*/, false/*active*/);
            m_part->xmlDocImpl()->renderer()->layer()
				->nodeAtPoint(renderInfo, xm, ym);
            innerNode = renderInfo.innerNode();
	}/*end if*/

        if (innerNode.handle() && innerNode.handle()->renderer()) {
            int absX, absY;
            innerNode.handle()->renderer()->absolutePosition(absX, absY);

            m_part->extendSelectionTo(xm, ym, absX, absY, innerNode);
        }/*end if*/
#endif // TDEHTML_NO_SELECTION
    }
}


class HackWidget : public TQWidget
{
 public:
    inline void setNoErase() { setWFlags(getWFlags()|WRepaintNoErase); }
};

bool TDEHTMLView::eventFilter(TQObject *o, TQEvent *e)
{
    if ( e->type() == TQEvent::AccelOverride ) {
	TQKeyEvent* ke = (TQKeyEvent*) e;
//kdDebug(6200) << "TQEvent::AccelOverride" << endl;
	if (m_part->isEditable() || m_part->isCaretMode()
	    || (m_part->xmlDocImpl() && m_part->xmlDocImpl()->focusNode()
		&& m_part->xmlDocImpl()->focusNode()->contentEditable())) {
//kdDebug(6200) << "editable/navigable" << endl;
	    if ( (ke->state() & ControlButton) || (ke->state() & ShiftButton) ) {
		switch ( ke->key() ) {
		case Key_Left:
		case Key_Right:
		case Key_Up:
		case Key_Down:
		case Key_Home:
		case Key_End:
		    ke->accept();
//kdDebug(6200) << "eaten" << endl;
		    return true;
		default:
		    break;
		}
	    }
	}
    }

    if ( e->type() == TQEvent::Leave ) {
      if ( d->cursor_icon_widget )
        d->cursor_icon_widget->hide();
      m_part->resetHoverText();
    }

    TQWidget *view = viewport();

    if (o == view) {
	// we need to install an event filter on all children of the viewport to
	// be able to get correct stacking of children within the document.
	if(e->type() == TQEvent::ChildInserted) {
	    TQObject *c = static_cast<TQChildEvent*>(e)->child();
	    if (c->isWidgetType()) {
		TQWidget *w = static_cast<TQWidget*>(c);
		// don't install the event filter on toplevels
		if (w->parentWidget(true) == view) {
		    if (!strcmp(w->name(), "__tdehtml")) {
			w->installEventFilter(this);
			w->unsetCursor();
			if (!::tqt_cast<TQFrame*>(w))
			    w->setBackgroundMode( TQWidget::NoBackground );
			static_cast<HackWidget *>(w)->setNoErase();
			if (!w->childrenListObject().isEmpty()) {
			    TQObjectListIterator it(w->childrenListObject());
			    for (; it.current(); ++it) {
				TQWidget *widget = ::tqt_cast<TQWidget *>(it.current());
				if (widget && !widget->isTopLevel()) {
				    if (!::tqt_cast<TQFrame*>(w))
				        widget->setBackgroundMode( TQWidget::NoBackground );
				    static_cast<HackWidget *>(widget)->setNoErase();
				    widget->installEventFilter(this);
				}
			    }
			}
		    }
		}
	    }
	}
    } else if (o->isWidgetType()) {
	TQWidget *v = static_cast<TQWidget*>(o);
        TQWidget *c = v;
	while (v && v != view) {
            c = v;
	    v = v->parentWidget(true);
	}

	if (v && !strcmp(c->name(), "__tdehtml")) {
	    bool block = false;
	    TQWidget *w = static_cast<TQWidget*>(o);
	    switch(e->type()) {
	    case TQEvent::Paint:
		if (!allowWidgetPaintEvents) {
		    // eat the event. Like this we can control exactly when the widget
		    // get's repainted.
		    block = true;
		    int x = 0, y = 0;
		    TQWidget *v = w;
		    while (v && v != view) {
			x += v->x();
			y += v->y();
			v = v->parentWidget();
		    }
		    viewportToContents( x, y, x, y );
		    TQPaintEvent *pe = static_cast<TQPaintEvent*>(e);
		    bool asap = !d->contentsMoving && ::tqt_cast<TQScrollView *>(c);

		    // TQScrollView needs fast repaints
		    if ( asap && !d->painting && m_part->xmlDocImpl() && m_part->xmlDocImpl()->renderer() &&
		         !static_cast<tdehtml::RenderCanvas *>(m_part->xmlDocImpl()->renderer())->needsLayout() ) {
		        repaintContents(x + pe->rect().x(), y + pe->rect().y(),
	                                        pe->rect().width(), pe->rect().height(), true);
                    } else {
 		        scheduleRepaint(x + pe->rect().x(), y + pe->rect().y(),
 				    pe->rect().width(), pe->rect().height(), asap);
                    }
		}
		break;
	    case TQEvent::MouseMove:
	    case TQEvent::MouseButtonPress:
	    case TQEvent::MouseButtonRelease:
	    case TQEvent::MouseButtonDblClick: {
		if ( (w->parentWidget() == view || ::tqt_cast<TQScrollView*>(c)) && !::tqt_cast<TQScrollBar *>(w)) {
		    TQMouseEvent *me = static_cast<TQMouseEvent*>(e);
		    TQPoint pt = w->mapTo( view, me->pos());
		    TQMouseEvent me2(me->type(), pt, me->button(), me->state());

		    if (e->type() == TQEvent::MouseMove)
			viewportMouseMoveEvent(&me2);
		    else if(e->type() == TQEvent::MouseButtonPress)
			viewportMousePressEvent(&me2);
		    else if(e->type() == TQEvent::MouseButtonRelease)
			viewportMouseReleaseEvent(&me2);
		    else
			viewportMouseDoubleClickEvent(&me2);
		    block = true;
                }
		break;
	    }
	    case TQEvent::KeyPress:
	    case TQEvent::KeyRelease:
		if (w->parentWidget() == view && !::tqt_cast<TQScrollBar *>(w)) {
		    TQKeyEvent *ke = static_cast<TQKeyEvent*>(e);
		    if (e->type() == TQEvent::KeyPress)
			keyPressEvent(ke);
		    else
			keyReleaseEvent(ke);
		    block = true;
		}
	    default:
		break;
	    }
	    if (block) {
 		//tqDebug("eating event");
		return true;
	    }
	}
    }

//    kdDebug(6000) <<"passing event on to sv event filter object=" << o->className() << " event=" << e->type() << endl;
    return TQScrollView::eventFilter(o, e);
}


DOM::NodeImpl *TDEHTMLView::nodeUnderMouse() const
{
    return d->underMouse;
}

DOM::NodeImpl *TDEHTMLView::nonSharedNodeUnderMouse() const
{
    return d->underMouseNonShared;
}

bool TDEHTMLView::scrollTo(const TQRect &bounds)
{
    d->scrollingSelf = true; // so scroll events get ignored

    int x, y, xe, ye;
    x = bounds.left();
    y = bounds.top();
    xe = bounds.right();
    ye = bounds.bottom();

    //kdDebug(6000)<<"scrolling coords: x="<<x<<" y="<<y<<" width="<<xe-x<<" height="<<ye-y<<endl;

    int deltax;
    int deltay;

    int curHeight = visibleHeight();
    int curWidth = visibleWidth();

    if (ye-y>curHeight-d->borderY)
	ye  = y + curHeight - d->borderY;

    if (xe-x>curWidth-d->borderX)
	xe = x + curWidth - d->borderX;

    // is xpos of target left of the view's border?
    if (x < contentsX() + d->borderX )
            deltax = x - contentsX() - d->borderX;
    // is xpos of target right of the view's right border?
    else if (xe + d->borderX > contentsX() + curWidth)
            deltax = xe + d->borderX - ( contentsX() + curWidth );
    else
        deltax = 0;

    // is ypos of target above upper border?
    if (y < contentsY() + d->borderY)
            deltay = y - contentsY() - d->borderY;
    // is ypos of target below lower border?
    else if (ye + d->borderY > contentsY() + curHeight)
            deltay = ye + d->borderY - ( contentsY() + curHeight );
    else
        deltay = 0;

    int maxx = curWidth-d->borderX;
    int maxy = curHeight-d->borderY;

    int scrollX,scrollY;

    scrollX = deltax > 0 ? (deltax > maxx ? maxx : deltax) : deltax == 0 ? 0 : (deltax>-maxx ? deltax : -maxx);
    scrollY = deltay > 0 ? (deltay > maxy ? maxy : deltay) : deltay == 0 ? 0 : (deltay>-maxy ? deltay : -maxy);

    if (contentsX() + scrollX < 0)
	scrollX = -contentsX();
    else if (contentsWidth() - visibleWidth() - contentsX() < scrollX)
	scrollX = contentsWidth() - visibleWidth() - contentsX();

    if (contentsY() + scrollY < 0)
	scrollY = -contentsY();
    else if (contentsHeight() - visibleHeight() - contentsY() < scrollY)
	scrollY = contentsHeight() - visibleHeight() - contentsY();

    scrollBy(scrollX, scrollY);

    d->scrollingSelf = false;

    if ( (abs(deltax)<=maxx) && (abs(deltay)<=maxy) )
	return true;
    else return false;

}

bool TDEHTMLView::focusNextPrevNode(bool next)
{
    // Sets the focus node of the document to be the node after (or if
    // next is false, before) the current focus node.  Only nodes that
    // are selectable (i.e. for which isFocusable() returns true) are
    // taken into account, and the order used is that specified in the
    // HTML spec (see DocumentImpl::nextFocusNode() and
    // DocumentImpl::previousFocusNode() for details).

    DocumentImpl *doc = m_part->xmlDocImpl();
    NodeImpl *oldFocusNode = doc->focusNode();

    // See whether we're in the middle of detach. If so, we want to
    // clear focus... The document code will be careful to not
    // emit events in that case..
    if (oldFocusNode && oldFocusNode->renderer() &&
        !oldFocusNode->renderer()->parent()) {
        doc->setFocusNode(0);
        return true;
    }

#if 1
    // If the user has scrolled the document, then instead of picking
    // the next focusable node in the document, use the first one that
    // is within the visible area (if possible).
    if (d->scrollBarMoved)
    {
	NodeImpl *toFocus;
	if (next)
	    toFocus = doc->nextFocusNode(oldFocusNode);
	else
	    toFocus = doc->previousFocusNode(oldFocusNode);

	if (!toFocus && oldFocusNode)
	    if (next)
		toFocus = doc->nextFocusNode(NULL);
	    else
		toFocus = doc->previousFocusNode(NULL);

	while (toFocus && toFocus != oldFocusNode)
	{

	    TQRect focusNodeRect = toFocus->getRect();
	    if ((focusNodeRect.left() > contentsX()) && (focusNodeRect.right() < contentsX() + visibleWidth()) &&
		(focusNodeRect.top() > contentsY()) && (focusNodeRect.bottom() < contentsY() + visibleHeight())) {
		{
		    TQRect r = toFocus->getRect();
		    ensureVisible( r.right(), r.bottom());
		    ensureVisible( r.left(), r.top());
		    d->scrollBarMoved = false;
		    d->tabMovePending = false;
		    d->lastTabbingDirection = next;
		    d->pseudoFocusNode = TDEHTMLViewPrivate::PFNone;
		    m_part->xmlDocImpl()->setFocusNode(toFocus);
		    Node guard(toFocus);
		    if (!toFocus->hasOneRef() )
		    {
			emit m_part->nodeActivated(Node(toFocus));
		    }
		    return true;
		}
	    }
	    if (next)
		toFocus = doc->nextFocusNode(toFocus);
	    else
		toFocus = doc->previousFocusNode(toFocus);

	    if (!toFocus && oldFocusNode)
		if (next)
		    toFocus = doc->nextFocusNode(NULL);
		else
		    toFocus = doc->previousFocusNode(NULL);
	}

	d->scrollBarMoved = false;
    }
#endif

    if (!oldFocusNode && d->pseudoFocusNode == TDEHTMLViewPrivate::PFNone)
    {
	ensureVisible(contentsX(), next?0:contentsHeight());
	d->scrollBarMoved = false;
	d->pseudoFocusNode = next?TDEHTMLViewPrivate::PFTop:TDEHTMLViewPrivate::PFBottom;
	return true;
    }

    NodeImpl *newFocusNode = NULL;

    if (d->tabMovePending && next != d->lastTabbingDirection)
    {
	//kdDebug ( 6000 ) << " tab move pending and tabbing direction changed!\n";
	newFocusNode = oldFocusNode;
    }
    else if (next)
    {
	if (oldFocusNode || d->pseudoFocusNode == TDEHTMLViewPrivate::PFTop )
	    newFocusNode = doc->nextFocusNode(oldFocusNode);
    }
    else
    {
	if (oldFocusNode || d->pseudoFocusNode == TDEHTMLViewPrivate::PFBottom )
	    newFocusNode = doc->previousFocusNode(oldFocusNode);
    }

    bool targetVisible = false;
    if (!newFocusNode)
    {
	if ( next )
	{
	    targetVisible = scrollTo(TQRect(contentsX()+visibleWidth()/2,contentsHeight()-d->borderY,0,0));
	}
	else
	{
	    targetVisible = scrollTo(TQRect(contentsX()+visibleWidth()/2,d->borderY,0,0));
	}
    }
    else
    {
#ifndef TDEHTML_NO_CARET
        // if it's an editable element, activate the caret
        if (!m_part->isCaretMode() && !m_part->isEditable()
		&& newFocusNode->contentEditable()) {
	    d->caretViewContext();
	    moveCaretTo(newFocusNode, 0L, true);
        } else {
	    caretOff();
	}
#endif // TDEHTML_NO_CARET

	targetVisible = scrollTo(newFocusNode->getRect());
    }

    if (targetVisible)
    {
	//kdDebug ( 6000 ) << " target reached.\n";
	d->tabMovePending = false;

	m_part->xmlDocImpl()->setFocusNode(newFocusNode);
	if (newFocusNode)
	{
	    Node guard(newFocusNode);
	    if (!newFocusNode->hasOneRef() )
	    {
		emit m_part->nodeActivated(Node(newFocusNode));
	    }
	    return true;
	}
	else
	{
	    d->pseudoFocusNode = next?TDEHTMLViewPrivate::PFBottom:TDEHTMLViewPrivate::PFTop;
	    return false;
	}
    }
    else
    {
	if (!d->tabMovePending)
	    d->lastTabbingDirection = next;
	d->tabMovePending = true;
	return true;
    }
}

void TDEHTMLView::displayAccessKeys()
{
    TQValueVector< TQChar > taken;
    displayAccessKeys( NULL, this, taken, false );
    displayAccessKeys( NULL, this, taken, true );
}

void TDEHTMLView::displayAccessKeys( TDEHTMLView* caller, TDEHTMLView* origview, TQValueVector< TQChar >& taken, bool use_fallbacks )
{
    TQMap< ElementImpl*, TQChar > fallbacks;
    if( use_fallbacks )
        fallbacks = buildFallbackAccessKeys();
    for( NodeImpl* n = m_part->xmlDocImpl(); n != NULL; n = n->traverseNextNode()) {
        if( n->isElementNode()) {
            ElementImpl* en = static_cast< ElementImpl* >( n );
            DOMString s = en->getAttribute( ATTR_ACCESSKEY );
            TQString accesskey;
            if( s.length() == 1 ) {
                TQChar a = s.string()[ 0 ].upper();
                if( tqFind( taken.begin(), taken.end(), a ) == taken.end()) // !contains
                    accesskey = a;
            }
            if( accesskey.isNull() && fallbacks.contains( en )) {
                TQChar a = fallbacks[ en ].upper();
                if( tqFind( taken.begin(), taken.end(), a ) == taken.end()) // !contains
                    accesskey = TQString( "<qt><i>" ) + a + "</i></qt>";
            }
            if( !accesskey.isNull()) {
	        TQRect rec=en->getRect();
	        TQLabel *lab=new TQLabel(accesskey,viewport(),0,(WFlags)WDestructiveClose);
	        connect( origview, TQ_SIGNAL(hideAccessKeys()), lab, TQ_SLOT(close()) );
	        connect( this, TQ_SIGNAL(repaintAccessKeys()), lab, TQ_SLOT(repaint()));
	        lab->setPalette(TQToolTip::palette());
	        lab->setLineWidth(2);
	        lab->setFrameStyle(TQFrame::Box | TQFrame::Plain);
	        lab->setMargin(3);
	        lab->adjustSize();
	        addChild(lab,
                    KMIN(rec.left()+rec.width()/2, contentsWidth() - lab->width()),
                    KMIN(rec.top()+rec.height()/2, contentsHeight() - lab->height()));
	        showChild(lab);
                taken.append( accesskey[ 0 ] );
	    }
        }
    }
    if( use_fallbacks )
        return;
    TQPtrList<KParts::ReadOnlyPart> frames = m_part->frames();
    for( TQPtrListIterator<KParts::ReadOnlyPart> it( frames );
         it != NULL;
         ++it ) {
        if( !(*it)->inherits( "TDEHTMLPart" ))
            continue;
        TDEHTMLPart* part = static_cast< TDEHTMLPart* >( *it );
        if( part->view() && part->view() != caller )
            part->view()->displayAccessKeys( this, origview, taken, use_fallbacks );
    }
    // pass up to the parent
    if (m_part->parentPart() && m_part->parentPart()->view()
        && m_part->parentPart()->view() != caller)
        m_part->parentPart()->view()->displayAccessKeys( this, origview, taken, use_fallbacks );
}



void TDEHTMLView::accessKeysTimeout()
{
d->accessKeysActivated=false;
d->accessKeysPreActivate = false;
m_part->setStatusBarText(TQString::null, TDEHTMLPart::BarOverrideText);
emit hideAccessKeys();
}

// Handling of the HTML accesskey attribute.
bool TDEHTMLView::handleAccessKey( const TQKeyEvent* ev )
{
// Qt interprets the keyevent also with the modifiers, and ev->text() matches that,
// but this code must act as if the modifiers weren't pressed
    TQChar c;
    if( ev->key() >= Key_A && ev->key() <= Key_Z )
        c = 'A' + ev->key() - Key_A;
    else if( ev->key() >= Key_0 && ev->key() <= Key_9 )
        c = '0' + ev->key() - Key_0;
    else {
        // TODO fake XKeyEvent and XLookupString ?
        // This below seems to work e.g. for eacute though.
        if( ev->text().length() == 1 )
            c = ev->text()[ 0 ];
    }
    if( c.isNull())
        return false;
    return focusNodeWithAccessKey( c );
}

bool TDEHTMLView::focusNodeWithAccessKey( TQChar c, TDEHTMLView* caller )
{
    DocumentImpl *doc = m_part->xmlDocImpl();
    if( !doc )
        return false;
    ElementImpl* node = doc->findAccessKeyElement( c );
    if( !node ) {
        TQPtrList<KParts::ReadOnlyPart> frames = m_part->frames();
        for( TQPtrListIterator<KParts::ReadOnlyPart> it( frames );
             it != NULL;
             ++it ) {
            if( !(*it)->inherits( "TDEHTMLPart" ))
                continue;
            TDEHTMLPart* part = static_cast< TDEHTMLPart* >( *it );
            if( part->view() && part->view() != caller
                && part->view()->focusNodeWithAccessKey( c, this ))
                return true;
        }
        // pass up to the parent
        if (m_part->parentPart() && m_part->parentPart()->view()
            && m_part->parentPart()->view() != caller
            && m_part->parentPart()->view()->focusNodeWithAccessKey( c, this ))
            return true;
        if( caller == NULL ) { // the active frame (where the accesskey was pressed)
            TQMap< ElementImpl*, TQChar > fallbacks = buildFallbackAccessKeys();
            for( TQMap< ElementImpl*, TQChar >::ConstIterator it = fallbacks.begin();
                 it != fallbacks.end();
                 ++it )
                if( *it == c ) {
                    node = it.key();
                    break;
                }
        }
        if( node == NULL )
            return false;
    }

    // Scroll the view as necessary to ensure that the new focus node is visible
#ifndef TDEHTML_NO_CARET
    // if it's an editable element, activate the caret
    if (!m_part->isCaretMode() && !m_part->isEditable()
	&& node->contentEditable()) {
        d->caretViewContext();
        moveCaretTo(node, 0L, true);
    } else {
        caretOff();
    }
#endif // TDEHTML_NO_CARET

    TQRect r = node->getRect();
    ensureVisible( r.right(), r.bottom());
    ensureVisible( r.left(), r.top());

    Node guard( node );
    if( node->isFocusable()) {
	if (node->id()==ID_LABEL) {
	    // if Accesskey is a label, give focus to the label's referrer.
	    node=static_cast<ElementImpl *>(static_cast< HTMLLabelElementImpl* >( node )->getFormElement());
	    if (!node) return true;
            guard = node;
	}
        // Set focus node on the document
        TQFocusEvent::setReason( TQFocusEvent::Shortcut );
        m_part->xmlDocImpl()->setFocusNode(node);
        TQFocusEvent::resetReason();
        if( node != NULL && node->hasOneRef()) // deleted, only held by guard
            return true;
        emit m_part->nodeActivated(Node(node));
        if( node != NULL && node->hasOneRef())
            return true;
    }

    switch( node->id()) {
        case ID_A:
            static_cast< HTMLAnchorElementImpl* >( node )->click();
          break;
        case ID_INPUT:
            static_cast< HTMLInputElementImpl* >( node )->click();
          break;
        case ID_BUTTON:
            static_cast< HTMLButtonElementImpl* >( node )->click();
          break;
        case ID_AREA:
            static_cast< HTMLAreaElementImpl* >( node )->click();
          break;
        case ID_TEXTAREA:
	  break; // just focusing it is enough
        case ID_LEGEND:
            // TODO
          break;
    }
    return true;
}

static TQString getElementText( NodeImpl* start, bool after )
{
    TQString ret;             // nextSibling(), to go after e.g. </select>
    for( NodeImpl* n = after ? start->nextSibling() : start->traversePreviousNode();
         n != NULL;
         n = after ? n->traverseNextNode() : n->traversePreviousNode()) {
        if( n->isTextNode()) {
            if( after )
                ret += static_cast< TextImpl* >( n )->toString().string();
            else
                ret.prepend( static_cast< TextImpl* >( n )->toString().string());
        } else {
            switch( n->id()) {
                case ID_A:
                case ID_FONT:
                case ID_TT:
                case ID_U:
                case ID_B:
                case ID_I:
                case ID_S:
                case ID_STRIKE:
                case ID_BIG:
                case ID_SMALL:
                case ID_EM:
                case ID_STRONG:
                case ID_DFN:
                case ID_CODE:
                case ID_SAMP:
                case ID_KBD:
                case ID_VAR:
                case ID_CITE:
                case ID_ABBR:
                case ID_ACRONYM:
                case ID_SUB:
                case ID_SUP:
                case ID_SPAN:
                case ID_NOBR:
                case ID_WBR:
                    break;
                case ID_TD:
                    if( ret.stripWhiteSpace().isEmpty())
                        break;
                    // fall through
                default:
                    return ret.simplifyWhiteSpace();
            }
        }
    }
    return ret.simplifyWhiteSpace();
}

static TQMap< NodeImpl*, TQString > buildLabels( NodeImpl* start )
{
    TQMap< NodeImpl*, TQString > ret;
    for( NodeImpl* n = start;
         n != NULL;
         n = n->traverseNextNode()) {
        if( n->id() == ID_LABEL ) {
            HTMLLabelElementImpl* label = static_cast< HTMLLabelElementImpl* >( n );
            NodeImpl* labelfor = label->getFormElement();
            if( labelfor )
                ret[ labelfor ] = label->innerText().string().simplifyWhiteSpace();
        }
    }
    return ret;
}

namespace tdehtml {
struct AccessKeyData {
    ElementImpl* element;
    TQString text;
    TQString url;
    int priority; // 10(highest) - 0(lowest)
};
}

TQMap< ElementImpl*, TQChar > TDEHTMLView::buildFallbackAccessKeys() const
{
    // build a list of all possible candidate elements that could use an accesskey
    TQValueList< AccessKeyData > data;
    TQMap< NodeImpl*, TQString > labels = buildLabels( m_part->xmlDocImpl());
    for( NodeImpl* n = m_part->xmlDocImpl();
         n != NULL;
         n = n->traverseNextNode()) {
        if( n->isElementNode()) {
            ElementImpl* element = static_cast< ElementImpl* >( n );
            if( element->getAttribute( ATTR_ACCESSKEY ).length() == 1 )
                continue; // has accesskey set, ignore
            if( element->renderer() == NULL )
                continue; // not visible
            TQString text;
            TQString url;
            int priority = 0;
            bool ignore = false;
            bool text_after = false;
            bool text_before = false;
            switch( element->id()) {
                case ID_A:
                    url = tdehtml::parseURL(element->getAttribute(ATTR_HREF)).string();
                    if( url.isEmpty()) // doesn't have href, it's only an anchor
                        continue;
                    text = static_cast< HTMLElementImpl* >( element )->innerText().string().simplifyWhiteSpace();
                    priority = 2;
                    break;
                case ID_INPUT: {
                    HTMLInputElementImpl* in = static_cast< HTMLInputElementImpl* >( element );
                    switch( in->inputType()) {
                        case HTMLInputElementImpl::SUBMIT:
                            text = in->value().string();
                            if( text.isEmpty())
                                text = i18n( "Submit" );
                            priority = 7;
                            break;
                        case HTMLInputElementImpl::IMAGE:
                            text = in->altText().string();
                            priority = 7;
                            break;
                        case HTMLInputElementImpl::BUTTON:
                            text = in->value().string();
                            priority = 5;
                            break;
                        case HTMLInputElementImpl::RESET:
                            text = in->value().string();
                            if( text.isEmpty())
                                text = i18n( "Reset" );
                            priority = 5;
                            break;
                        case HTMLInputElementImpl::HIDDEN:
                            ignore = true;
                            break;
                        case HTMLInputElementImpl::CHECKBOX:
                        case HTMLInputElementImpl::RADIO:
                            text_after = true;
                            priority = 5;
                            break;
                        case HTMLInputElementImpl::TEXT:
                        case HTMLInputElementImpl::PASSWORD:
                        case HTMLInputElementImpl::FILE:
                            text_before = true;
                            priority = 5;
                            break;
                        default:
                            priority = 5;
                            break;
                    }
                    break;
                }
                case ID_BUTTON:
                    text = static_cast< HTMLElementImpl* >( element )->innerText().string().simplifyWhiteSpace();
                    switch( static_cast< HTMLButtonElementImpl* >( element )->buttonType()) {
                        case HTMLButtonElementImpl::SUBMIT:
                            if( text.isEmpty())
                                text = i18n( "Submit" );
                            priority = 7;
                            break;
                        case HTMLButtonElementImpl::RESET:
                            if( text.isEmpty())
                                text = i18n( "Reset" );
                            priority = 5;
                            break;
                        default:
                            priority = 5;
                            break;
                    break;
                    }
                case ID_SELECT: // these don't have accesskey attribute, but quick access may be handy
                    text_before = true;
                    text_after = true;
                    priority = 5;
                    break;
                case ID_FRAME:
                    ignore = true;
                    break;
                default:
                    ignore = !element->isFocusable();
                    priority = 2;
                    break;
            }
            if( ignore )
                continue;
            if( text.isNull() && labels.contains( element ))
                text = labels[ element ];
            if( text.isNull() && text_before )
                text = getElementText( element, false );
            if( text.isNull() && text_after )
                text = getElementText( element, true );
            text = text.stripWhiteSpace();
            // increase priority of items which have explicitly specified accesskeys in the config
            TQValueList< TQPair< TQString, TQChar > > priorities
                = m_part->settings()->fallbackAccessKeysAssignments();
            for( TQValueList< TQPair< TQString, TQChar > >::ConstIterator it = priorities.begin();
                 it != priorities.end();
                 ++it ) {
                if( text == (*it).first )
                    priority = 10;
            }
            AccessKeyData tmp = { element, text, url, priority };
            data.append( tmp );
        }
    }

    TQValueList< TQChar > keys;
    for( char c = 'A'; c <= 'Z'; ++c )
        keys << c;
    for( char c = '0'; c <= '9'; ++c )
        keys << c;
    for( NodeImpl* n = m_part->xmlDocImpl();
         n != NULL;
         n = n->traverseNextNode()) {
        if( n->isElementNode()) {
            ElementImpl* en = static_cast< ElementImpl* >( n );
            DOMString s = en->getAttribute( ATTR_ACCESSKEY );
            if( s.length() == 1 ) {
                TQChar c = s.string()[ 0 ].upper();
                keys.remove( c ); // remove manually assigned accesskeys
            }
        }
    }

    TQMap< ElementImpl*, TQChar > ret;
    for( int priority = 10;
         priority >= 0;
         --priority ) {
        for( TQValueList< AccessKeyData >::Iterator it = data.begin();
             it != data.end();
             ) {
            if( (*it).priority != priority ) {
                ++it;
                continue;
            }
            if( keys.isEmpty())
                break;
            TQString text = (*it).text;
            TQChar key;
            if( key.isNull() && !text.isEmpty()) {
                TQValueList< TQPair< TQString, TQChar > > priorities
                    = m_part->settings()->fallbackAccessKeysAssignments();
                for( TQValueList< TQPair< TQString, TQChar > >::ConstIterator it = priorities.begin();
                     it != priorities.end();
                     ++it )
                    if( text == (*it).first && keys.contains( (*it).second )) {
                        key = (*it).second;
                        break;
                    }
            }
            // try first to select the first character as the accesskey,
            // then first character of the following words,
            // and then simply the first free character
            if( key.isNull() && !text.isEmpty()) {
                TQStringList words = TQStringList::split( ' ', text );
                for( TQStringList::ConstIterator it = words.begin();
                     it != words.end();
                     ++it ) {
                    if( keys.contains( (*it)[ 0 ].upper())) {
                        key = (*it)[ 0 ].upper();
                        break;
                    }
                }
            }
            if( key.isNull() && !text.isEmpty()) {
                for( unsigned int i = 0;
                     i < text.length();
                     ++i ) {
                    if( keys.contains( text[ i ].upper())) {
                        key = text[ i ].upper();
                        break;
                    }
                }
            }
            if( key.isNull())
                key = keys.front();
            ret[ (*it).element ] = key;
            keys.remove( key );
            TQString url = (*it).url;
            it = data.remove( it );
            // assign the same accesskey also to other elements pointing to the same url
            if( !url.isEmpty() && !url.startsWith( "javascript:", false )) {
                for( TQValueList< AccessKeyData >::Iterator it2 = data.begin();
                     it2 != data.end();
                     ) {
                    if( (*it2).url == url ) {
                        ret[ (*it2).element ] = key;
                        if( it == it2 )
                            ++it;
                        it2 = data.remove( it2 );
                    } else
                        ++it2;
                }
            }
        }
    }
    return ret;
}

void TDEHTMLView::setMediaType( const TQString &medium )
{
    m_medium = medium;
}

TQString TDEHTMLView::mediaType() const
{
    return m_medium;
}

bool TDEHTMLView::pagedMode() const
{
    return d->paged;
}

void TDEHTMLView::setWidgetVisible(RenderWidget* w, bool vis)
{
    if (vis) {
        d->visibleWidgets.replace(w, w->widget());
    }
    else
        d->visibleWidgets.remove(w);
}

bool TDEHTMLView::needsFullRepaint() const
{
    return d->needsFullRepaint;
}

void TDEHTMLView::print()
{
    print( false );
}

void TDEHTMLView::print(bool quick)
{
    if(!m_part->xmlDocImpl()) return;
    tdehtml::RenderCanvas *root = static_cast<tdehtml::RenderCanvas *>(m_part->xmlDocImpl()->renderer());
    if(!root) return;

    KPrinter *printer = new KPrinter(true, TQPrinter::ScreenResolution);
    printer->addDialogPage(new TDEHTMLPrintSettings());
    TQString docname = m_part->xmlDocImpl()->URL().prettyURL();
    if ( !docname.isEmpty() )
        docname = KStringHandler::csqueeze(docname, 80);
    if(quick || printer->setup(this, i18n("Print %1").arg(docname))) {
        viewport()->setCursor( TQt::waitCursor ); // only viewport(), no TQApplication::, otherwise we get the busy cursor in tdeprint's dialogs
        // set up KPrinter
        printer->setFullPage(false);
        printer->setCreator(TQString("TDE %1.%2.%3 HTML Library").arg(TDE_VERSION_MAJOR).arg(TDE_VERSION_MINOR).arg(TDE_VERSION_RELEASE));
        printer->setDocName(docname);

        TQPainter *p = new TQPainter;
        p->begin( printer );
        tdehtml::setPrintPainter( p );

        m_part->xmlDocImpl()->setPaintDevice( printer );
        TQString oldMediaType = mediaType();
        setMediaType( "print" );
        // We ignore margin settings for html and body when printing
        // and use the default margins from the print-system
        // (In Qt 3.0.x the default margins are hardcoded in Qt)
        m_part->xmlDocImpl()->setPrintStyleSheet( printer->option("app-khtml-printfriendly") == "true" ?
                                                  "* { background-image: none !important;"
                                                  "    background-color: white !important;"
                                                  "    color: black !important; }"
						  "body { margin: 0px !important; }"
						  "html { margin: 0px !important; }" :
						  "body { margin: 0px !important; }"
						  "html { margin: 0px !important; }"
						  );

        TQPaintDeviceMetrics metrics( printer );

        kdDebug(6000) << "printing: physical page width = " << metrics.width()
                      << " height = " << metrics.height() << endl;
        root->setStaticMode(true);
        root->setPagedMode(true);
        root->setWidth(metrics.width());
//         root->setHeight(metrics.height());
        root->setPageTop(0);
        root->setPageBottom(0);
        d->paged = true;

        m_part->xmlDocImpl()->styleSelector()->computeFontSizes(&metrics, 100);
        m_part->xmlDocImpl()->updateStyleSelector();
        root->setPrintImages( printer->option("app-khtml-printimages") == "true");
        root->makePageBreakAvoidBlocks();

        root->setNeedsLayoutAndMinMaxRecalc();
        root->layout();
        tdehtml::RenderWidget::flushWidgetResizes(); // make sure widgets have their final size

        // check sizes ask for action.. (scale or clip)

        bool printHeader = (printer->option("app-khtml-printheader") == "true");

        int headerHeight = 0;
        TQFont headerFont("Sans Serif", 8);

        TQString headerLeft = TDEGlobal::locale()->formatDate(TQDate::currentDate(),true);
        TQString headerMid = docname;
        TQString headerRight;

        if (printHeader)
        {
           p->setFont(headerFont);
           headerHeight = (p->fontMetrics().lineSpacing() * 3) / 2;
        }

        // ok. now print the pages.
        kdDebug(6000) << "printing: html page width = " << root->docWidth()
                      << " height = " << root->docHeight() << endl;
        kdDebug(6000) << "printing: margins left = " << printer->margins().width()
                      << " top = " << printer->margins().height() << endl;
        kdDebug(6000) << "printing: paper width = " << metrics.width()
                      << " height = " << metrics.height() << endl;
        // if the width is too large to fit on the paper we just scale
        // the whole thing.
        int pageWidth = metrics.width();
        int pageHeight = metrics.height();
        p->setClipRect(0,0, pageWidth, pageHeight);

        pageHeight -= headerHeight;

        bool scalePage = false;
        double scale = 0.0;
#ifndef TQT_NO_TRANSFORMATIONS
        if(root->docWidth() > metrics.width()) {
            scalePage = true;
            scale = ((double) metrics.width())/((double) root->docWidth());
            pageHeight = (int) (pageHeight/scale);
            pageWidth = (int) (pageWidth/scale);
            headerHeight = (int) (headerHeight/scale);
        }
#endif
        kdDebug(6000) << "printing: scaled html width = " << pageWidth
                      << " height = " << pageHeight << endl;

        root->setHeight(pageHeight);
        root->setPageBottom(pageHeight);
        root->setNeedsLayout(true);
        root->layoutIfNeeded();
//         m_part->slotDebugRenderTree();

        // Squeeze header to make it it on the page.
        if (printHeader)
        {
            int available_width = metrics.width() - 10 -
                2 * kMax(p->boundingRect(0, 0, metrics.width(), p->fontMetrics().lineSpacing(), TQt::AlignLeft, headerLeft).width(),
                         p->boundingRect(0, 0, metrics.width(), p->fontMetrics().lineSpacing(), TQt::AlignLeft, headerRight).width());
            if (available_width < 150)
               available_width = 150;
            int mid_width;
            int squeeze = 120;
            do {
                headerMid = KStringHandler::csqueeze(docname, squeeze);
                mid_width = p->boundingRect(0, 0, metrics.width(), p->fontMetrics().lineSpacing(), TQt::AlignLeft, headerMid).width();
                squeeze -= 10;
            } while (mid_width > available_width);
        }

        int top = 0;
        int bottom = 0;
        int page = 1;
        while(top < root->docHeight()) {
            if(top > 0) printer->newPage();
            p->setClipRect(0, 0, pageWidth, headerHeight, TQPainter::CoordDevice);
            if (printHeader)
            {
                int dy = p->fontMetrics().lineSpacing();
                p->setPen(TQt::black);
                p->setFont(headerFont);

                headerRight = TQString("#%1").arg(page);

                p->drawText(0, 0, metrics.width(), dy, TQt::AlignLeft, headerLeft);
                p->drawText(0, 0, metrics.width(), dy, TQt::AlignHCenter, headerMid);
                p->drawText(0, 0, metrics.width(), dy, TQt::AlignRight, headerRight);
            }


#ifndef TQT_NO_TRANSFORMATIONS
            if (scalePage)
                p->scale(scale, scale);
#endif

            p->setClipRect(0, headerHeight, pageWidth, pageHeight, TQPainter::CoordDevice);
            p->translate(0, headerHeight-top);

            bottom = top+pageHeight;

            root->setPageTop(top);
            root->setPageBottom(bottom);
            root->setPageNumber(page);

            root->layer()->paint(p, TQRect(0, top, pageWidth, pageHeight));
//             m_part->xmlDocImpl()->renderer()->layer()->paint(p, TQRect(0, top, pageWidth, pageHeight));
//             root->repaint();
//             p->flush();
            kdDebug(6000) << "printed: page " << page <<" bottom At = " << bottom << endl;

            top = bottom;
            p->resetXForm();
            page++;
        }

        p->end();
        delete p;

        // and now reset the layout to the usual one...
        root->setPagedMode(false);
        root->setStaticMode(false);
        d->paged = false;
        tdehtml::setPrintPainter( 0 );
        setMediaType( oldMediaType );
        m_part->xmlDocImpl()->setPaintDevice( this );
        m_part->xmlDocImpl()->styleSelector()->computeFontSizes(m_part->xmlDocImpl()->paintDeviceMetrics(), m_part->zoomFactor());
        m_part->xmlDocImpl()->updateStyleSelector();
        viewport()->unsetCursor();
    }
    delete printer;
}

void TDEHTMLView::slotPaletteChanged()
{
    if(!m_part->xmlDocImpl()) return;
    DOM::DocumentImpl *document = m_part->xmlDocImpl();
    if (!document->isHTMLDocument()) return;
    tdehtml::RenderCanvas *root = static_cast<tdehtml::RenderCanvas *>(document->renderer());
    if(!root) return;
    root->style()->resetPalette();
    NodeImpl *body = static_cast<HTMLDocumentImpl*>(document)->body();
    if(!body) return;
    body->setChanged(true);
    body->recalcStyle( NodeImpl::Force );
}

void TDEHTMLView::paint(TQPainter *p, const TQRect &rc, int yOff, bool *more)
{
    if(!m_part->xmlDocImpl()) return;
    tdehtml::RenderCanvas *root = static_cast<tdehtml::RenderCanvas *>(m_part->xmlDocImpl()->renderer());
    if(!root) return;

    m_part->xmlDocImpl()->setPaintDevice(p->device());
    root->setPagedMode(true);
    root->setStaticMode(true);
    root->setWidth(rc.width());

    p->save();
    p->setClipRect(rc);
    p->translate(rc.left(), rc.top());
    double scale = ((double) rc.width()/(double) root->docWidth());
    int height = (int) ((double) rc.height() / scale);
#ifndef TQT_NO_TRANSFORMATIONS
    p->scale(scale, scale);
#endif
    root->setPageTop(yOff);
    root->setPageBottom(yOff+height);

    root->layer()->paint(p, TQRect(0, yOff, root->docWidth(), height));
    if (more)
        *more = yOff + height < root->docHeight();
    p->restore();

    root->setPagedMode(false);
    root->setStaticMode(false);
    m_part->xmlDocImpl()->setPaintDevice( this );
}


void TDEHTMLView::useSlowRepaints()
{
    d->useSlowRepaints = true;
    setStaticBackground(true);
}


void TDEHTMLView::setVScrollBarMode ( ScrollBarMode mode )
{
#ifndef TDEHTML_NO_SCROLLBARS
    d->vmode = mode;
    TQScrollView::setVScrollBarMode(mode);
#else
    Q_UNUSED( mode );
#endif
}

void TDEHTMLView::setHScrollBarMode ( ScrollBarMode mode )
{
#ifndef TDEHTML_NO_SCROLLBARS
    d->hmode = mode;
    TQScrollView::setHScrollBarMode(mode);
#else
    Q_UNUSED( mode );
#endif
}

void TDEHTMLView::restoreScrollBar()
{
    int ow = visibleWidth();
    TQScrollView::setVScrollBarMode(d->vmode);
    if (visibleWidth() != ow)
        layout();
    d->prevScrollbarVisible = verticalScrollBar()->isVisible();
}

TQStringList TDEHTMLView::formCompletionItems(const TQString &name) const
{
    if (!m_part->settings()->isFormCompletionEnabled())
        return TQStringList();
    if (!d->formCompletions)
        d->formCompletions = new KSimpleConfig(locateLocal("data", "tdehtml/formcompletions"));
    return d->formCompletions->readListEntry(name);
}

void TDEHTMLView::clearCompletionHistory(const TQString& name)
{
    if (!d->formCompletions)
    {
        d->formCompletions = new KSimpleConfig(locateLocal("data", "tdehtml/formcompletions"));
    }
    d->formCompletions->writeEntry(name, "");
    d->formCompletions->sync();
}

void TDEHTMLView::addFormCompletionItem(const TQString &name, const TQString &value)
{
    if (!m_part->settings()->isFormCompletionEnabled())
        return;
    // don't store values that are all numbers or just numbers with
    // dashes or spaces as those are likely credit card numbers or
    // something similar
    bool cc_number(true);
    for (unsigned int i = 0; i < value.length(); ++i)
    {
      TQChar c(value[i]);
      if (!c.isNumber() && c != '-' && !c.isSpace())
      {
        cc_number = false;
        break;
      }
    }
    if (cc_number)
      return;
    TQStringList items = formCompletionItems(name);
    if (!items.contains(value))
        items.prepend(value);
    while ((int)items.count() > m_part->settings()->maxFormCompletionItems())
        items.remove(items.fromLast());
    d->formCompletions->writeEntry(name, items);
}

void TDEHTMLView::removeFormCompletionItem(const TQString &name, const TQString &value)
{
    if (!m_part->settings()->isFormCompletionEnabled())
        return;

    TQStringList items = formCompletionItems(name);
    if (items.remove(value))
        d->formCompletions->writeEntry(name, items);
}

void TDEHTMLView::addNonPasswordStorableSite(const TQString& host)
{
    if (!d->formCompletions) {
        d->formCompletions = new KSimpleConfig(locateLocal("data", "tdehtml/formcompletions"));
    }

    d->formCompletions->setGroup("NonPasswordStorableSites");
    TQStringList sites = d->formCompletions->readListEntry("Sites");
    sites.append(host);
    d->formCompletions->writeEntry("Sites", sites);
    d->formCompletions->sync();
    d->formCompletions->setGroup(TQString::null);//reset
}

bool TDEHTMLView::nonPasswordStorableSite(const TQString& host) const
{
    if (!d->formCompletions) {
        d->formCompletions = new KSimpleConfig(locateLocal("data", "tdehtml/formcompletions"));
    }
    d->formCompletions->setGroup("NonPasswordStorableSites");
    TQStringList sites =  d->formCompletions->readListEntry("Sites");
    d->formCompletions->setGroup(TQString::null);//reset

    return (sites.find(host) != sites.end());
}

// returns true if event should be swallowed
bool TDEHTMLView::dispatchMouseEvent(int eventId, DOM::NodeImpl *targetNode,
				   DOM::NodeImpl *targetNodeNonShared, bool cancelable,
				   int detail,TQMouseEvent *_mouse, bool setUnder,
				   int mouseEventType)
{
    // if the target node is a text node, dispatch on the parent node - rdar://4196646 (and #76948)
    if (targetNode && targetNode->isTextNode())
        targetNode = targetNode->parentNode();

    if (d->underMouse)
	d->underMouse->deref();
    d->underMouse = targetNode;
    if (d->underMouse)
	d->underMouse->ref();

    if (d->underMouseNonShared)
	d->underMouseNonShared->deref();
    d->underMouseNonShared = targetNodeNonShared;
    if (d->underMouseNonShared)
	d->underMouseNonShared->ref();

    int exceptioncode = 0;
    int pageX = 0;
    int pageY = 0;
    viewportToContents(_mouse->x(), _mouse->y(), pageX, pageY);
    int clientX = pageX - contentsX();
    int clientY = pageY - contentsY();
    int screenX = _mouse->globalX();
    int screenY = _mouse->globalY();
    int button = -1;
    switch (_mouse->button()) {
	case TQt::LeftButton:
	    button = 0;
	    break;
	case TQt::MidButton:
	    button = 1;
	    break;
	case TQt::RightButton:
	    button = 2;
	    break;
	default:
	    break;
    }
    if (d->accessKeysEnabled && d->accessKeysPreActivate && button!=-1)
    	d->accessKeysPreActivate=false;

    bool ctrlKey = (_mouse->state() & ControlButton);
    bool altKey = (_mouse->state() & AltButton);
    bool shiftKey = (_mouse->state() & ShiftButton);
    bool metaKey = (_mouse->state() & MetaButton);

    // mouseout/mouseover
    if (setUnder && (d->prevMouseX != pageX || d->prevMouseY != pageY)) {

        // ### this code sucks. we should save the oldUnder instead of calculating
        // it again. calculating is expensive! (Dirk)
        NodeImpl *oldUnder = 0;
	if (d->prevMouseX >= 0 && d->prevMouseY >= 0) {
	    NodeImpl::MouseEvent mev( _mouse->stateAfter(), static_cast<NodeImpl::MouseEventType>(mouseEventType));
	    m_part->xmlDocImpl()->prepareMouseEvent( true, d->prevMouseX, d->prevMouseY, &mev );
	    oldUnder = mev.innerNode.handle();

            if (oldUnder && oldUnder->isTextNode())
                oldUnder = oldUnder->parentNode();
	}
// 	tqDebug("oldunder=%p (%s), target=%p (%s) x/y=%d/%d", oldUnder, oldUnder ? oldUnder->renderer()->renderName() : 0, targetNode,  targetNode ? targetNode->renderer()->renderName() : 0, _mouse->x(), _mouse->y());
	if (oldUnder != targetNode) {
	    // send mouseout event to the old node
	    if (oldUnder){
		oldUnder->ref();
		MouseEventImpl *me = new MouseEventImpl(EventImpl::MOUSEOUT_EVENT,
							true,true,m_part->xmlDocImpl()->defaultView(),
							0,screenX,screenY,clientX,clientY,pageX, pageY,
							ctrlKey,altKey,shiftKey,metaKey,
							button,targetNode);
		me->ref();
		oldUnder->dispatchEvent(me,exceptioncode,true);
		me->deref();
	    }

	    // send mouseover event to the new node
	    if (targetNode) {
		MouseEventImpl *me = new MouseEventImpl(EventImpl::MOUSEOVER_EVENT,
							true,true,m_part->xmlDocImpl()->defaultView(),
							0,screenX,screenY,clientX,clientY,pageX, pageY,
							ctrlKey,altKey,shiftKey,metaKey,
							button,oldUnder);

		me->ref();
		targetNode->dispatchEvent(me,exceptioncode,true);
		me->deref();
	    }

            if (oldUnder)
                oldUnder->deref();
        }
    }

    bool swallowEvent = false;

    if (targetNode) {
        // send the actual event
        bool dblclick = ( eventId == EventImpl::CLICK_EVENT &&
                          _mouse->type() == TQEvent::MouseButtonDblClick );
        MouseEventImpl *me = new MouseEventImpl(static_cast<EventImpl::EventId>(eventId),
						true,cancelable,m_part->xmlDocImpl()->defaultView(),
						detail,screenX,screenY,clientX,clientY,pageX, pageY,
						ctrlKey,altKey,shiftKey,metaKey,
						button,0, _mouse, dblclick );
        me->ref();
        targetNode->dispatchEvent(me,exceptioncode,true);
	bool defaultHandled = me->defaultHandled();
        if (defaultHandled || me->defaultPrevented())
            swallowEvent = true;
        me->deref();

        if (eventId == EventImpl::MOUSEDOWN_EVENT) {
            // Focus should be shifted on mouse down, not on a click.  -dwh
            // Blur current focus node when a link/button is clicked; this
            // is expected by some sites that rely on onChange handlers running
            // from form fields before the button click is processed.
            DOM::NodeImpl* nodeImpl = targetNode;
            for ( ; nodeImpl && !nodeImpl->isFocusable(); nodeImpl = nodeImpl->parentNode());
            if (nodeImpl && nodeImpl->isMouseFocusable())
                m_part->xmlDocImpl()->setFocusNode(nodeImpl);
            else if (!nodeImpl || !nodeImpl->focused())
                m_part->xmlDocImpl()->setFocusNode(0);
        }
    }

    return swallowEvent;
}

void TDEHTMLView::setIgnoreWheelEvents( bool e )
{
    d->ignoreWheelEvents = e;
}

#ifndef TQT_NO_WHEELEVENT

void TDEHTMLView::viewportWheelEvent(TQWheelEvent* e)
{
    if (d->accessKeysEnabled && d->accessKeysPreActivate) d->accessKeysPreActivate=false;

    if ( ( e->state() & ControlButton) == ControlButton )
    {
        emit zoomView( - e->delta() );
        e->accept();
    }
    else if (d->firstRelayout)
    {
        e->accept();
    }
    else if( (   (e->orientation() == TQt::Vertical &&
                   ((d->ignoreWheelEvents && !verticalScrollBar()->isVisible())
                     || e->delta() > 0 && contentsY() <= 0
                     || e->delta() < 0 && contentsY() >= contentsHeight() - visibleHeight()))
              ||
                 (e->orientation() == TQt::Horizontal &&
                    ((d->ignoreWheelEvents && !horizontalScrollBar()->isVisible())
                     || e->delta() > 0 && contentsX() <=0
                     || e->delta() < 0 && contentsX() >= contentsWidth() - visibleWidth())))
            && m_part->parentPart())
    {
        if ( m_part->parentPart()->view() )
            m_part->parentPart()->view()->wheelEvent( e );
        e->ignore();
    }
    else
    {
        d->scrollBarMoved = true;
#ifndef NO_SMOOTH_SCROLL_HACK
        scrollViewWheelEvent( e );
#else
        TQScrollView::viewportWheelEvent( e );
#endif

        TQMouseEvent *tempEvent = new TQMouseEvent( TQEvent::MouseMove, TQPoint(-1,-1), TQPoint(-1,-1), TQt::NoButton, e->state() );
        emit viewportMouseMoveEvent ( tempEvent );
        delete tempEvent;
    }

}
#endif

void TDEHTMLView::dragEnterEvent( TQDragEnterEvent* ev )
{
    // Handle drops onto frames (#16820)
    // Drops on the main html part is handled by Konqueror (and shouldn't do anything
    // in e.g. kmail, so not handled here).
    if ( m_part->parentPart() )
    {
    	TQApplication::sendEvent(m_part->parentPart()->widget(), ev);
	return;
    }
    TQScrollView::dragEnterEvent( ev );
}

void TDEHTMLView::dropEvent( TQDropEvent *ev )
{
    // Handle drops onto frames (#16820)
    // Drops on the main html part is handled by Konqueror (and shouldn't do anything
    // in e.g. kmail, so not handled here).
    if ( m_part->parentPart() )
    {
    	TQApplication::sendEvent(m_part->parentPart()->widget(), ev);
	return;
    }
    TQScrollView::dropEvent( ev );
}

void TDEHTMLView::focusInEvent( TQFocusEvent *e )
{
#ifndef TDEHTML_NO_TYPE_AHEAD_FIND
    m_part->enableFindAheadActions( true );
#endif
    DOM::NodeImpl* fn = m_part->xmlDocImpl() ? m_part->xmlDocImpl()->focusNode() : 0;
    if (fn && fn->renderer() && fn->renderer()->isWidget() &&
        (e->reason() != TQFocusEvent::Mouse) &&
        static_cast<tdehtml::RenderWidget*>(fn->renderer())->widget())
        static_cast<tdehtml::RenderWidget*>(fn->renderer())->widget()->setFocus();
#ifndef TDEHTML_NO_CARET
    // Restart blink frequency timer if it has been killed, but only on
    // editable nodes
    if (d->m_caretViewContext &&
        d->m_caretViewContext->freqTimerId == -1 &&
        fn) {
        if (m_part->isCaretMode()
		|| m_part->isEditable()
     		|| (fn && fn->renderer()
			&& fn->renderer()->style()->userInput()
				== UI_ENABLED)) {
            d->m_caretViewContext->freqTimerId = startTimer(500);
	    d->m_caretViewContext->visible = true;
        }/*end if*/
    }/*end if*/
    showCaret();
#endif // TDEHTML_NO_CARET
    TQScrollView::focusInEvent( e );
}

void TDEHTMLView::focusOutEvent( TQFocusEvent *e )
{
    if(m_part) m_part->stopAutoScroll();

#ifndef TDEHTML_NO_TYPE_AHEAD_FIND
    if(d->typeAheadActivated)
    {
        findTimeout();
    }
    m_part->enableFindAheadActions( false );
#endif // TDEHTML_NO_TYPE_AHEAD_FIND

#ifndef TDEHTML_NO_CARET
    if (d->m_caretViewContext) {
        switch (d->m_caretViewContext->displayNonFocused) {
	case TDEHTMLPart::CaretInvisible:
            hideCaret();
	    break;
	case TDEHTMLPart::CaretVisible: {
	    killTimer(d->m_caretViewContext->freqTimerId);
	    d->m_caretViewContext->freqTimerId = -1;
            NodeImpl *caretNode = m_part->xmlDocImpl()->focusNode();
	    if (!d->m_caretViewContext->visible && (m_part->isCaretMode()
		|| m_part->isEditable()
     		|| (caretNode && caretNode->renderer()
			&& caretNode->renderer()->style()->userInput()
				== UI_ENABLED))) {
	        d->m_caretViewContext->visible = true;
	        showCaret(true);
	    }/*end if*/
	    break;
	}
	case TDEHTMLPart::CaretBlink:
	    // simply leave as is
	    break;
	}/*end switch*/
    }/*end if*/
#endif // TDEHTML_NO_CARET

    if ( d->cursor_icon_widget )
        d->cursor_icon_widget->hide();

    TQScrollView::focusOutEvent( e );
}

void TDEHTMLView::slotScrollBarMoved()
{
    if ( !d->firstRelayout && !d->complete && m_part->xmlDocImpl() &&
          d->layoutSchedulingEnabled) {
        // contents scroll while we are not complete: we need to check our layout *now*
        tdehtml::RenderCanvas* root = static_cast<tdehtml::RenderCanvas *>( m_part->xmlDocImpl()->renderer() );
        if (root && root->needsLayout()) {
            unscheduleRelayout();
            layout();
        }
    }
    if (!d->scrollingSelf) {
        d->scrollBarMoved = true;
        d->contentsMoving = true;
        // ensure quick reset of contentsMoving flag
        scheduleRepaint(0, 0, 0, 0);
    }

    if (m_part->xmlDocImpl() && m_part->xmlDocImpl()->documentElement())
        m_part->xmlDocImpl()->documentElement()->dispatchHTMLEvent(EventImpl::SCROLL_EVENT, true, false);
}

void TDEHTMLView::timerEvent ( TQTimerEvent *e )
{
//    kdDebug() << "timer event " << e->timerId() << endl;
    if ( e->timerId() == d->scrollTimerId ) {
        if( d->scrollSuspended )
            return;
        switch (d->scrollDirection) {
            case TDEHTMLViewPrivate::ScrollDown:
                if (contentsY() + visibleHeight () >= contentsHeight())
                    d->newScrollTimer(this, 0);
                else
                    scrollBy( 0, d->scrollBy );
                break;
            case TDEHTMLViewPrivate::ScrollUp:
                if (contentsY() <= 0)
                    d->newScrollTimer(this, 0);
                else
                    scrollBy( 0, -d->scrollBy );
                break;
            case TDEHTMLViewPrivate::ScrollRight:
                if (contentsX() + visibleWidth () >= contentsWidth())
                    d->newScrollTimer(this, 0);
                else
                    scrollBy( d->scrollBy, 0 );
                break;
            case TDEHTMLViewPrivate::ScrollLeft:
                if (contentsX() <= 0)
                    d->newScrollTimer(this, 0);
                else
                    scrollBy( -d->scrollBy, 0 );
                break;
        }
        return;
    }
    else if ( e->timerId() == d->layoutTimerId ) {
        d->dirtyLayout = true;
        layout();
        if (d->firstRelayout) {
            d->firstRelayout = false;
            verticalScrollBar()->setEnabled( true );
            horizontalScrollBar()->setEnabled( true );
        }
    }
#ifndef TDEHTML_NO_CARET
    else if (d->m_caretViewContext
    	     && e->timerId() == d->m_caretViewContext->freqTimerId) {
        d->m_caretViewContext->visible = !d->m_caretViewContext->visible;
	if (d->m_caretViewContext->displayed) {
	    updateContents(d->m_caretViewContext->x, d->m_caretViewContext->y,
			d->m_caretViewContext->width,
			d->m_caretViewContext->height);
	}/*end if*/
//	if (d->m_caretViewContext->visible) cout << "|" << flush;
//	else cout << "" << flush;
	return;
    }
#endif

    d->contentsMoving = false;
    if( m_part->xmlDocImpl() ) {
	DOM::DocumentImpl *document = m_part->xmlDocImpl();
	tdehtml::RenderCanvas* root = static_cast<tdehtml::RenderCanvas *>(document->renderer());

	if ( root && root->needsLayout() ) {
	    killTimer(d->repaintTimerId);
	    d->repaintTimerId = 0;
	    scheduleRelayout();
	    return;
	}
    }

    setStaticBackground(d->useSlowRepaints);

//        kdDebug() << "scheduled repaint "<< d->repaintTimerId  << endl;
    killTimer(d->repaintTimerId);
    d->repaintTimerId = 0;

    TQRect updateRegion;
    TQMemArray<TQRect> rects = d->updateRegion.rects();

    d->updateRegion = TQRegion();

    if ( rects.size() )
        updateRegion = rects[0];

    for ( unsigned i = 1; i < rects.size(); ++i ) {
        TQRect newRegion = updateRegion.unite(rects[i]);
        if (2*newRegion.height() > 3*updateRegion.height() )
        {
            repaintContents( updateRegion );
            updateRegion = rects[i];
        }
        else
            updateRegion = newRegion;
    }

    if ( !updateRegion.isNull() )
        repaintContents( updateRegion );

    // As widgets can only be accurately positioned during painting, every layout might
    // dissociate a widget from its RenderWidget. E.g: if a RenderWidget was visible before layout, but the layout
    // pushed it out of the viewport, it will not be repainted, and consequently it's assocoated widget won't be repositioned!
    // Thus we need to check each supposedly 'visible' widget at the end of each layout, and remove it in case it's no more in sight.

    if (d->dirtyLayout && !d->visibleWidgets.isEmpty()) {
        TQWidget* w;
        d->dirtyLayout = false;

        TQRect visibleRect(contentsX(), contentsY(), visibleWidth(), visibleHeight());
        TQPtrList<RenderWidget> toRemove;
        for (TQPtrDictIterator<TQWidget> it(d->visibleWidgets); it.current(); ++it) {
            int xp = 0, yp = 0;
            w = it.current();
            RenderWidget* rw = static_cast<RenderWidget*>( it.currentKey() );
            if (!rw->absolutePosition(xp, yp) ||
                !visibleRect.intersects(TQRect(xp, yp, w->width(), w->height())))
                toRemove.append(rw);
        }
        for (RenderWidget* r = toRemove.first(); r; r = toRemove.next())
            if ( (w = d->visibleWidgets.take(r) ) )
                addChild(w, 0, -500000);
    }

    emit repaintAccessKeys();
    if (d->emitCompletedAfterRepaint) {
        bool full = d->emitCompletedAfterRepaint == TDEHTMLViewPrivate::CSFull;
        d->emitCompletedAfterRepaint = TDEHTMLViewPrivate::CSNone;
        if ( full )
            emit m_part->completed();
        else
            emit m_part->completed(true);
    }
}

void TDEHTMLView::scheduleRelayout(tdehtml::RenderObject * /*clippedObj*/)
{
    if (!d->layoutSchedulingEnabled || d->layoutTimerId)
        return;

    d->layoutTimerId = startTimer( m_part->xmlDocImpl() && m_part->xmlDocImpl()->parsing()
                             ? 1000 : 0 );
}

void TDEHTMLView::unscheduleRelayout()
{
    if (!d->layoutTimerId)
        return;

    killTimer(d->layoutTimerId);
    d->layoutTimerId = 0;
}

void TDEHTMLView::unscheduleRepaint()
{
    if (!d->repaintTimerId)
        return;

    killTimer(d->repaintTimerId);
    d->repaintTimerId = 0;
}

void TDEHTMLView::scheduleRepaint(int x, int y, int w, int h, bool asap)
{
    bool parsing = !m_part->xmlDocImpl() || m_part->xmlDocImpl()->parsing();

//     kdDebug() << "parsing " << parsing << endl;
//     kdDebug() << "complete " << d->complete << endl;

    int time = parsing ? 300 : (!asap ? ( !d->complete ? 100 : 20 ) : 0);

#ifdef DEBUG_FLICKER
    TQPainter p;
    p.begin( viewport() );

    int vx, vy;
    contentsToViewport( x, y, vx, vy );
    p.fillRect( vx, vy, w, h, TQt::red );
    p.end();
#endif

    d->updateRegion = d->updateRegion.unite(TQRect(x,y,w,h));

    if (asap && !parsing)
        unscheduleRepaint();

    if ( !d->repaintTimerId )
        d->repaintTimerId = startTimer( time );

//     kdDebug() << "starting timer " << time << endl;
}

void TDEHTMLView::complete( bool pendingAction )
{
//     kdDebug() << "TDEHTMLView::complete()" << endl;

    d->complete = true;

    // is there a relayout pending?
    if (d->layoutTimerId)
    {
//         kdDebug() << "requesting relayout now" << endl;
        // do it now
        killTimer(d->layoutTimerId);
        d->layoutTimerId = startTimer( 0 );
        d->emitCompletedAfterRepaint = pendingAction ?
            TDEHTMLViewPrivate::CSActionPending : TDEHTMLViewPrivate::CSFull;
    }

    // is there a repaint pending?
    if (d->repaintTimerId)
    {
//         kdDebug() << "requesting repaint now" << endl;
        // do it now
        killTimer(d->repaintTimerId);
        d->repaintTimerId = startTimer( 20 );
        d->emitCompletedAfterRepaint = pendingAction ?
            TDEHTMLViewPrivate::CSActionPending : TDEHTMLViewPrivate::CSFull;
    }

    if (!d->emitCompletedAfterRepaint)
    {
        if (!pendingAction)
	    emit m_part->completed();
        else
            emit m_part->completed(true);
    }

}

void TDEHTMLView::slotMouseScrollTimer()
{
    scrollBy( d->m_mouseScroll_byX, d->m_mouseScroll_byY );
}

#ifndef TDEHTML_NO_CARET

// ### the dependencies on static functions are a nightmare. just be
// hacky and include the implementation here. Clean me up, please.

#include "tdehtml_caret.cpp"

void TDEHTMLView::initCaret(bool keepSelection)
{
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "begin initCaret" << endl;
#endif
  // save caretMoved state as moveCaretTo changes it
  if (m_part->xmlDocImpl()) {
#if 0
    ElementImpl *listitem = m_part->xmlDocImpl()->getElementById("__test_element__");
    if (listitem) dumpLineBoxes(static_cast<RenderFlow *>(listitem->renderer()));
#endif
    d->caretViewContext();
    bool cmoved = d->m_caretViewContext->caretMoved;
    if (m_part->d->caretNode().isNull()) {
      // set to document, position will be sanitized anyway
      m_part->d->caretNode() = m_part->document();
      m_part->d->caretOffset() = 0L;
      // This sanity check is necessary for the not so unlikely case that
      // setEditable or setCaretMode is called before any render objects have
      // been created.
      if (!m_part->d->caretNode().handle()->renderer()) return;
    }/*end if*/
//    kdDebug(6200) << "d->m_selectionStart " << m_part->d->m_selectionStart.handle()
//    		<< " d->m_selectionEnd " << m_part->d->m_selectionEnd.handle() << endl;
    // ### does not repaint the selection on keepSelection!=false
    moveCaretTo(m_part->d->caretNode().handle(), m_part->d->caretOffset(), !keepSelection);
//    kdDebug(6200) << "d->m_selectionStart " << m_part->d->m_selectionStart.handle()
//    		<< " d->m_selectionEnd " << m_part->d->m_selectionEnd.handle() << endl;
    d->m_caretViewContext->caretMoved = cmoved;
  }/*end if*/
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "end initCaret" << endl;
#endif
}

bool TDEHTMLView::caretOverrides() const
{
    bool cm = m_part->isCaretMode();
    bool dm = m_part->isEditable();
    return cm && !dm ? false
    	: (dm || m_part->d->caretNode().handle()->contentEditable())
	  && d->editorContext()->override;
}

void TDEHTMLView::ensureNodeHasFocus(NodeImpl *node)
{
  if (m_part->isCaretMode() || m_part->isEditable()) return;
  if (node->focused()) return;

  // Find first ancestor whose "user-input" is "enabled"
  NodeImpl *firstAncestor = 0;
  while (node) {
    if (node->renderer()
       && node->renderer()->style()->userInput() != UI_ENABLED)
      break;
    firstAncestor = node;
    node = node->parentNode();
  }/*wend*/

  if (!node) firstAncestor = 0;

  DocumentImpl *doc = m_part->xmlDocImpl();
  // ensure that embedded widgets don't lose their focus
  if (!firstAncestor && doc->focusNode() && doc->focusNode()->renderer()
  	&& doc->focusNode()->renderer()->isWidget())
    return;

  // Set focus node on the document
#if DEBUG_CARETMODE > 1
  kdDebug(6200) << k_funcinfo << "firstAncestor " << firstAncestor << ": "
  	<< (firstAncestor ? firstAncestor->nodeName().string() : TQString::null) << endl;
#endif
  doc->setFocusNode(firstAncestor);
  emit m_part->nodeActivated(Node(firstAncestor));
}

void TDEHTMLView::recalcAndStoreCaretPos(CaretBox *hintBox)
{
    if (!m_part || m_part->d->caretNode().isNull()) return;
    d->caretViewContext();
    NodeImpl *caretNode = m_part->d->caretNode().handle();
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "recalcAndStoreCaretPos: caretNode=" << caretNode << (caretNode ? " "+caretNode->nodeName().string() : TQString::null) << " r@" << caretNode->renderer() << (caretNode->renderer() && caretNode->renderer()->isText() ? " \"" + TQConstString(static_cast<RenderText *>(caretNode->renderer())->str->s, kMin(static_cast<RenderText *>(caretNode->renderer())->str->l, 15u)).string() + "\"" : TQString::null) << endl;
#endif
    caretNode->getCaret(m_part->d->caretOffset(), caretOverrides(),
    		d->m_caretViewContext->x, d->m_caretViewContext->y,
		d->m_caretViewContext->width,
		d->m_caretViewContext->height);

    if (hintBox && d->m_caretViewContext->x == -1) {
#if DEBUG_CARETMODE > 1
        kdDebug(6200) << "using hint inline box coordinates" << endl;
#endif
	RenderObject *r = caretNode->renderer();
	const TQFontMetrics &fm = r->style()->fontMetrics();
        int absx, absy;
	r->containingBlock()->absolutePosition(absx, absy,
						false);	// ### what about fixed?
	d->m_caretViewContext->x = absx + hintBox->xPos();
	d->m_caretViewContext->y = absy + hintBox->yPos();
// 				+ hintBox->baseline() - fm.ascent();
	d->m_caretViewContext->width = 1;
	// ### firstline not regarded. But I think it can be safely neglected
	// as hint boxes are only used for empty lines.
	d->m_caretViewContext->height = fm.height();
    }/*end if*/

#if DEBUG_CARETMODE > 4
//    kdDebug(6200) << "freqTimerId: "<<d->m_caretViewContext->freqTimerId<<endl;
#endif
#if DEBUG_CARETMODE > 0
    kdDebug(6200) << "caret: ofs="<<m_part->d->caretOffset()<<" "
    	<<" x="<<d->m_caretViewContext->x<<" y="<<d->m_caretViewContext->y
	<<" h="<<d->m_caretViewContext->height<<endl;
#endif
}

void TDEHTMLView::caretOn()
{
    if (d->m_caretViewContext) {
        killTimer(d->m_caretViewContext->freqTimerId);

	if (hasFocus() || d->m_caretViewContext->displayNonFocused
			== TDEHTMLPart::CaretBlink) {
            d->m_caretViewContext->freqTimerId = startTimer(500);
	} else {
	    d->m_caretViewContext->freqTimerId = -1;
	}/*end if*/

        d->m_caretViewContext->visible = true;
        if ((d->m_caretViewContext->displayed = (hasFocus()
		|| d->m_caretViewContext->displayNonFocused
			!= TDEHTMLPart::CaretInvisible))) {
	    updateContents(d->m_caretViewContext->x, d->m_caretViewContext->y,
	    		d->m_caretViewContext->width,
			d->m_caretViewContext->height);
	}/*end if*/
//        kdDebug(6200) << "caret on" << endl;
    }/*end if*/
}

void TDEHTMLView::caretOff()
{
    if (d->m_caretViewContext) {
        killTimer(d->m_caretViewContext->freqTimerId);
	d->m_caretViewContext->freqTimerId = -1;
        d->m_caretViewContext->displayed = false;
        if (d->m_caretViewContext->visible) {
            d->m_caretViewContext->visible = false;
	    updateContents(d->m_caretViewContext->x, d->m_caretViewContext->y,
	    		d->m_caretViewContext->width,
	    		d->m_caretViewContext->height);
	}/*end if*/
//        kdDebug(6200) << "caret off" << endl;
    }/*end if*/
}

void TDEHTMLView::showCaret(bool forceRepaint)
{
    if (d->m_caretViewContext) {
        d->m_caretViewContext->displayed = true;
        if (d->m_caretViewContext->visible) {
	    if (!forceRepaint) {
	    	updateContents(d->m_caretViewContext->x, d->m_caretViewContext->y,
	    		d->m_caretViewContext->width,
			d->m_caretViewContext->height);
            } else {
	    	repaintContents(d->m_caretViewContext->x, d->m_caretViewContext->y,
	    		d->m_caretViewContext->width,
	    		d->m_caretViewContext->height);
	    }/*end if*/
   	}/*end if*/
//        kdDebug(6200) << "caret shown" << endl;
    }/*end if*/
}

bool TDEHTMLView::foldSelectionToCaret(NodeImpl *startNode, long startOffset,
    				NodeImpl *endNode, long endOffset)
{
  m_part->d->m_selectionStart = m_part->d->m_selectionEnd = m_part->d->caretNode();
  m_part->d->m_startOffset = m_part->d->m_endOffset = m_part->d->caretOffset();
  m_part->d->m_extendAtEnd = true;

  bool folded = startNode != endNode || startOffset != endOffset;

  // Only clear the selection if there has been one.
  if (folded) {
    m_part->xmlDocImpl()->clearSelection();
  }/*end if*/

  return folded;
}

void TDEHTMLView::hideCaret()
{
    if (d->m_caretViewContext) {
        if (d->m_caretViewContext->visible) {
//            kdDebug(6200) << "redraw caret hidden" << endl;
	    d->m_caretViewContext->visible = false;
	    // force repaint, otherwise the event won't be handled
	    // before the focus leaves the window
	    repaintContents(d->m_caretViewContext->x, d->m_caretViewContext->y,
	    		d->m_caretViewContext->width,
	    		d->m_caretViewContext->height);
	    d->m_caretViewContext->visible = true;
	}/*end if*/
        d->m_caretViewContext->displayed = false;
//        kdDebug(6200) << "caret hidden" << endl;
    }/*end if*/
}

int TDEHTMLView::caretDisplayPolicyNonFocused() const
{
  if (d->m_caretViewContext)
    return d->m_caretViewContext->displayNonFocused;
  else
    return TDEHTMLPart::CaretInvisible;
}

void TDEHTMLView::setCaretDisplayPolicyNonFocused(int policy)
{
  d->caretViewContext();
//  int old = d->m_caretViewContext->displayNonFocused;
  d->m_caretViewContext->displayNonFocused = (TDEHTMLPart::CaretDisplayPolicy)policy;

  // make change immediately take effect if not focused
  if (!hasFocus()) {
    switch (d->m_caretViewContext->displayNonFocused) {
      case TDEHTMLPart::CaretInvisible:
        hideCaret();
	break;
      case TDEHTMLPart::CaretBlink:
	if (d->m_caretViewContext->freqTimerId != -1) break;
	d->m_caretViewContext->freqTimerId = startTimer(500);
	// fall through
      case TDEHTMLPart::CaretVisible:
        d->m_caretViewContext->displayed = true;
        showCaret();
	break;
    }/*end switch*/
  }/*end if*/
}

bool TDEHTMLView::placeCaret(CaretBox *hintBox)
{
  CaretViewContext *cv = d->caretViewContext();
  caretOff();
  NodeImpl *caretNode = m_part->d->caretNode().handle();
  // ### why is it sometimes null?
  if (!caretNode || !caretNode->renderer()) return false;
  ensureNodeHasFocus(caretNode);
  if (m_part->isCaretMode() || m_part->isEditable()
     || caretNode->renderer()->style()->userInput() == UI_ENABLED) {
    recalcAndStoreCaretPos(hintBox);

    cv->origX = cv->x;

    caretOn();
    return true;
  }/*end if*/
  return false;
}

void TDEHTMLView::ensureCaretVisible()
{
  CaretViewContext *cv = d->m_caretViewContext;
  if (!cv) return;
  ensureVisible(cv->x, cv->y, cv->width, cv->height);
  d->scrollBarMoved = false;
}

bool TDEHTMLView::extendSelection(NodeImpl *oldStartSel, long oldStartOfs,
				NodeImpl *oldEndSel, long oldEndOfs)
{
  bool changed = false;
  if (m_part->d->m_selectionStart == m_part->d->m_selectionEnd
      && m_part->d->m_startOffset == m_part->d->m_endOffset) {
    changed = foldSelectionToCaret(oldStartSel, oldStartOfs, oldEndSel, oldEndOfs);
    m_part->d->m_extendAtEnd = true;
  } else do {
    changed = m_part->d->m_selectionStart.handle() != oldStartSel
    		|| m_part->d->m_startOffset != oldStartOfs
		|| m_part->d->m_selectionEnd.handle() != oldEndSel
		|| m_part->d->m_endOffset != oldEndOfs;
    if (!changed) break;

    // determine start position -- caret position is always at end.
    NodeImpl *startNode;
    long startOffset;
    if (m_part->d->m_extendAtEnd) {
      startNode = m_part->d->m_selectionStart.handle();
      startOffset = m_part->d->m_startOffset;
    } else {
      startNode = m_part->d->m_selectionEnd.handle();
      startOffset = m_part->d->m_endOffset;
      m_part->d->m_selectionEnd = m_part->d->m_selectionStart;
      m_part->d->m_endOffset = m_part->d->m_startOffset;
      m_part->d->m_extendAtEnd = true;
    }/*end if*/

    bool swapNeeded = false;
    if (!m_part->d->m_selectionEnd.isNull() && startNode) {
      swapNeeded = RangeImpl::compareBoundaryPoints(startNode, startOffset,
      			m_part->d->m_selectionEnd.handle(),
			m_part->d->m_endOffset) >= 0;
    }/*end if*/

    m_part->d->m_selectionStart = startNode;
    m_part->d->m_startOffset = startOffset;

    if (swapNeeded) {
      m_part->xmlDocImpl()->setSelection(m_part->d->m_selectionEnd.handle(),
		m_part->d->m_endOffset, m_part->d->m_selectionStart.handle(),
		m_part->d->m_startOffset);
    } else {
      m_part->xmlDocImpl()->setSelection(m_part->d->m_selectionStart.handle(),
		m_part->d->m_startOffset, m_part->d->m_selectionEnd.handle(),
		m_part->d->m_endOffset);
    }/*end if*/
  } while(false);/*end if*/
  return changed;
}

void TDEHTMLView::updateSelection(NodeImpl *oldStartSel, long oldStartOfs,
				NodeImpl *oldEndSel, long oldEndOfs)
{
  if (m_part->d->m_selectionStart == m_part->d->m_selectionEnd
      && m_part->d->m_startOffset == m_part->d->m_endOffset) {
    if (foldSelectionToCaret(oldStartSel, oldStartOfs, oldEndSel, oldEndOfs)) {
      m_part->emitSelectionChanged();
    }/*end if*/
    m_part->d->m_extendAtEnd = true;
  } else {
    // check if the extending end has passed the immobile end
    if (!m_part->d->m_selectionEnd.isNull() && !m_part->d->m_selectionEnd.isNull()) {
      bool swapNeeded = RangeImpl::compareBoundaryPoints(
      			m_part->d->m_selectionStart.handle(), m_part->d->m_startOffset,
			m_part->d->m_selectionEnd.handle(), m_part->d->m_endOffset) >= 0;
      if (swapNeeded) {
        DOM::Node tmpNode = m_part->d->m_selectionStart;
        long tmpOffset = m_part->d->m_startOffset;
        m_part->d->m_selectionStart = m_part->d->m_selectionEnd;
        m_part->d->m_startOffset = m_part->d->m_endOffset;
        m_part->d->m_selectionEnd = tmpNode;
        m_part->d->m_endOffset = tmpOffset;
        m_part->d->m_startBeforeEnd = true;
        m_part->d->m_extendAtEnd = !m_part->d->m_extendAtEnd;
      }/*end if*/
    }/*end if*/

    m_part->xmlDocImpl()->setSelection(m_part->d->m_selectionStart.handle(),
		m_part->d->m_startOffset, m_part->d->m_selectionEnd.handle(),
		m_part->d->m_endOffset);
    m_part->emitSelectionChanged();
  }/*end if*/
}

void TDEHTMLView::caretKeyPressEvent(TQKeyEvent *_ke)
{
  NodeImpl *oldStartSel = m_part->d->m_selectionStart.handle();
  long oldStartOfs = m_part->d->m_startOffset;
  NodeImpl *oldEndSel = m_part->d->m_selectionEnd.handle();
  long oldEndOfs = m_part->d->m_endOffset;

  NodeImpl *oldCaretNode = m_part->d->caretNode().handle();
  long oldOffset = m_part->d->caretOffset();

  bool ctrl = _ke->state() & ControlButton;

// FIXME: this is that widely indented because I will write ifs around it.
      switch(_ke->key()) {
        case Key_Space:
          break;

        case Key_Down:
	  moveCaretNextLine(1);
          break;

        case Key_Up:
	  moveCaretPrevLine(1);
          break;

        case Key_Left:
	  moveCaretBy(false, ctrl ? CaretByWord : CaretByCharacter, 1);
          break;

        case Key_Right:
	  moveCaretBy(true, ctrl ? CaretByWord : CaretByCharacter, 1);
          break;

        case Key_Next:
	  moveCaretNextPage();
          break;

        case Key_Prior:
	  moveCaretPrevPage();
          break;

        case Key_Home:
	  if (ctrl)
	    moveCaretToDocumentBoundary(false);
	  else
	    moveCaretToLineBegin();
          break;

        case Key_End:
	  if (ctrl)
	    moveCaretToDocumentBoundary(true);
	  else
	    moveCaretToLineEnd();
          break;

      }/*end switch*/

  if ((m_part->d->caretNode().handle() != oldCaretNode
  	|| m_part->d->caretOffset() != oldOffset)
	// node should never be null, but faulty conditions may cause it to be
	&& !m_part->d->caretNode().isNull()) {

    d->m_caretViewContext->caretMoved = true;

    if (_ke->state() & ShiftButton) {	// extend selection
      updateSelection(oldStartSel, oldStartOfs, oldEndSel, oldEndOfs);
    } else {			// clear any selection
      if (foldSelectionToCaret(oldStartSel, oldStartOfs, oldEndSel, oldEndOfs))
        m_part->emitSelectionChanged();
    }/*end if*/

    m_part->emitCaretPositionChanged(m_part->d->caretNode(), m_part->d->caretOffset());
  }/*end if*/

  _ke->accept();
}

bool TDEHTMLView::moveCaretTo(NodeImpl *node, long offset, bool clearSel)
{
  if (!node) return false;
  ElementImpl *baseElem = determineBaseElement(node);
  RenderFlow *base = static_cast<RenderFlow *>(baseElem ? baseElem->renderer() : 0);
  if (!node) return false;

  // need to find out the node's inline box. If there is none, this function
  // will snap to the next node that has one. This is necessary to make the
  // caret visible in any case.
  CaretBoxLineDeleter cblDeleter;
//   RenderBlock *cb;
  long r_ofs;
  CaretBoxIterator cbit;
  CaretBoxLine *cbl = findCaretBoxLine(node, offset, &cblDeleter, base, r_ofs, cbit);
  if(!cbl) {
      kdWarning() << "TDEHTMLView::moveCaretTo - findCaretBoxLine() returns NULL" << endl;
      return false;
  }

#if DEBUG_CARETMODE > 3
  if (cbl) kdDebug(6200) << cbl->information() << endl;
#endif
  CaretBox *box = *cbit;
  if (cbit != cbl->end() && box->object() != node->renderer()) {
    if (box->object()->element()) {
      mapRenderPosToDOMPos(box->object(), r_ofs, box->isOutside(),
      			box->isOutsideEnd(), node, offset);
      //if (!outside) offset = node->minOffset();
#if DEBUG_CARETMODE > 1
      kdDebug(6200) << "set new node " << node->nodeName().string() << "@" << node << endl;
#endif
    } else {	// box has no associated element -> do not use
      // this case should actually never happen.
      box = 0;
      kdError(6200) << "Box contains no node! Crash imminent" << endl;
    }/*end if*/
  }

  NodeImpl *oldStartSel = m_part->d->m_selectionStart.handle();
  long oldStartOfs = m_part->d->m_startOffset;
  NodeImpl *oldEndSel = m_part->d->m_selectionEnd.handle();
  long oldEndOfs = m_part->d->m_endOffset;

  // test for position change
  bool posChanged = m_part->d->caretNode().handle() != node
  		|| m_part->d->caretOffset() != offset;
  bool selChanged = false;

  m_part->d->caretNode() = node;
  m_part->d->caretOffset() = offset;
  if (clearSel || !oldStartSel || !oldEndSel) {
    selChanged = foldSelectionToCaret(oldStartSel, oldStartOfs, oldEndSel, oldEndOfs);
  } else {
    //kdDebug(6200) << "moveToCaret: extendSelection: m_extendAtEnd " << m_part->d->m_extendAtEnd << endl;
    //kdDebug(6200) << "selection: start(" << m_part->d->m_selectionStart.handle() << "," << m_part->d->m_startOffset << "), end(" << m_part->d->m_selectionEnd.handle() << "," << m_part->d->m_endOffset << "), caret(" << m_part->d->caretNode().handle() << "," << m_part->d->caretOffset() << ")" << endl;
    selChanged = extendSelection(oldStartSel, oldStartOfs, oldEndSel, oldEndOfs);
    //kdDebug(6200) << "after extendSelection: m_extendAtEnd " << m_part->d->m_extendAtEnd << endl;
    //kdDebug(6200) << "selection: start(" << m_part->d->m_selectionStart.handle() << "," << m_part->d->m_startOffset << "), end(" << m_part->d->m_selectionEnd.handle() << "," << m_part->d->m_endOffset << "), caret(" << m_part->d->caretNode().handle() << "," << m_part->d->caretOffset() << ")" << endl;
  }/*end if*/

  d->caretViewContext()->caretMoved = true;

  bool visible_caret = placeCaret(box);

  // FIXME: if the old position was !visible_caret, and the new position is
  // also, then two caretPositionChanged signals with a null Node are
  // emitted in series.
  if (posChanged) {
    m_part->emitCaretPositionChanged(visible_caret ? node : 0, offset);
  }/*end if*/

  return selChanged;
}

void TDEHTMLView::moveCaretByLine(bool next, int count)
{
  Node &caretNodeRef = m_part->d->caretNode();
  if (caretNodeRef.isNull()) return;

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long offset = m_part->d->caretOffset();

  CaretViewContext *cv = d->caretViewContext();

  ElementImpl *baseElem = determineBaseElement(caretNode);
  LinearDocument ld(m_part, caretNode, offset, LeafsOnly, baseElem);

  ErgonomicEditableLineIterator it(ld.current(), cv->origX);

  // move count lines vertically
  while (count > 0 && it != ld.end() && it != ld.preBegin()) {
    count--;
    if (next) ++it; else --it;
  }/*wend*/

  // Nothing? Then leave everything as is.
  if (it == ld.end() || it == ld.preBegin()) return;

  int x, absx, absy;
  CaretBox *caretBox = nearestCaretBox(it, d->m_caretViewContext, x, absx, absy);

  placeCaretOnLine(caretBox, x, absx, absy);
}

void TDEHTMLView::placeCaretOnLine(CaretBox *caretBox, int x, int absx, int absy)
{
  // paranoia sanity check
  if (!caretBox) return;

  RenderObject *caretRender = caretBox->object();

#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "got valid caretBox " << caretBox << endl;
  kdDebug(6200) << "xPos: " << caretBox->xPos() << " yPos: " << caretBox->yPos()
  		<< " width: " << caretBox->width() << " height: " << caretBox->height() << endl;
  InlineTextBox *tb = static_cast<InlineTextBox *>(caretBox->inlineBox());
  if (caretBox->isInlineTextBox()) { kdDebug(6200) << "contains \"" << TQString(static_cast<RenderText *>(tb->object())->str->s + tb->m_start, tb->m_len) << "\"" << endl;}
#endif
  // inquire height of caret
  int caretHeight = caretBox->height();
  bool isText = caretBox->isInlineTextBox();
  int yOfs = 0;		// y-offset for text nodes
  if (isText) {
    // text boxes need extrawurst
    RenderText *t = static_cast<RenderText *>(caretRender);
    const TQFontMetrics &fm = t->metrics(caretBox->inlineBox()->m_firstLine);
    caretHeight = fm.height();
    yOfs = caretBox->inlineBox()->baseline() - fm.ascent();
  }/*end if*/

  caretOff();

  // set new caret node
  NodeImpl *caretNode;
  long &offset = m_part->d->caretOffset();
  mapRenderPosToDOMPos(caretRender, offset, caretBox->isOutside(),
  		caretBox->isOutsideEnd(), caretNode, offset);

  // set all variables not needing special treatment
  d->m_caretViewContext->y = caretBox->yPos() + yOfs;
  d->m_caretViewContext->height = caretHeight;
  d->m_caretViewContext->width = 1; // FIXME: regard override

  int xPos = caretBox->xPos();
  int caretBoxWidth = caretBox->width();
  d->m_caretViewContext->x = xPos;

  if (!caretBox->isOutside()) {
    // before or at beginning of inline box -> place at beginning
    long r_ofs = 0;
    if (x <= xPos) {
      r_ofs = caretBox->minOffset();
  // somewhere within this block
    } else if (x > xPos && x <= xPos + caretBoxWidth) {
      if (isText) { // find out where exactly
        r_ofs = static_cast<InlineTextBox *>(caretBox->inlineBox())
      		->offsetForPoint(x, d->m_caretViewContext->x);
#if DEBUG_CARETMODE > 2
        kdDebug(6200) << "deviation from origX " << d->m_caretViewContext->x - x << endl;
#endif
#if 0
      } else {	// snap to nearest end
        if (xPos + caretBoxWidth - x < x - xPos) {
          d->m_caretViewContext->x = xPos + caretBoxWidth;
          r_ofs = caretNode ? caretNode->maxOffset() : 1;
        } else {
          d->m_caretViewContext->x = xPos;
          r_ofs = caretNode ? caretNode->minOffset() : 0;
        }/*end if*/
#endif
      }/*end if*/
    } else {		// after the inline box -> place at end
      d->m_caretViewContext->x = xPos + caretBoxWidth;
      r_ofs = caretBox->maxOffset();
    }/*end if*/
    offset = r_ofs;
  }/*end if*/
#if DEBUG_CARETMODE > 0
      kdDebug(6200) << "new offset: " << offset << endl;
#endif

  m_part->d->caretNode() = caretNode;
  m_part->d->caretOffset() = offset;

  d->m_caretViewContext->x += absx;
  d->m_caretViewContext->y += absy;

#if DEBUG_CARETMODE > 1
	kdDebug(6200) << "new caret position: x " << d->m_caretViewContext->x << " y " << d->m_caretViewContext->y << " w " << d->m_caretViewContext->width << " h " << d->m_caretViewContext->height << " absx " << absx << " absy " << absy << endl;
#endif

  ensureVisible(d->m_caretViewContext->x, d->m_caretViewContext->y,
  	d->m_caretViewContext->width, d->m_caretViewContext->height);
  d->scrollBarMoved = false;

  ensureNodeHasFocus(caretNode);
  caretOn();
}

void TDEHTMLView::moveCaretToLineBoundary(bool end)
{
  Node &caretNodeRef = m_part->d->caretNode();
  if (caretNodeRef.isNull()) return;

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long offset = m_part->d->caretOffset();

  ElementImpl *baseElem = determineBaseElement(caretNode);
  LinearDocument ld(m_part, caretNode, offset, LeafsOnly, baseElem);

  EditableLineIterator it = ld.current();
  if (it == ld.end()) return;	// should not happen, but who knows

  EditableCaretBoxIterator fbit(it, end);
  Q_ASSERT(fbit != (*it)->end() && fbit != (*it)->preBegin());
  CaretBox *b = *fbit;

  RenderObject *cb = b->containingBlock();
  int absx, absy;

  if (cb) cb->absolutePosition(absx,absy);
  else absx = absy = 0;

  int x = b->xPos() + (end && !b->isOutside() ? b->width() : 0);
  d->m_caretViewContext->origX = absx + x;
  placeCaretOnLine(b, x, absx, absy);
}

void TDEHTMLView::moveCaretToDocumentBoundary(bool end)
{
  Node &caretNodeRef = m_part->d->caretNode();
  if (caretNodeRef.isNull()) return;

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long offset = m_part->d->caretOffset();

  ElementImpl *baseElem = determineBaseElement(caretNode);
  LinearDocument ld(m_part, caretNode, offset, IndicatedFlows, baseElem);

  EditableLineIterator it(end ? ld.preEnd() : ld.begin(), end);
  if (it == ld.end() || it == ld.preBegin()) return;	// should not happen, but who knows

  EditableCaretBoxIterator fbit = it;
  Q_ASSERT(fbit != (*it)->end() && fbit != (*it)->preBegin());
  CaretBox *b = *fbit;

  RenderObject *cb = (*it)->containingBlock();
  int absx, absy;

  if (cb) cb->absolutePosition(absx, absy);
  else absx = absy = 0;

  int x = b->xPos()/* + (end ? b->width() : 0) reactivate for rtl*/;
  d->m_caretViewContext->origX = absx + x;
  placeCaretOnLine(b, x, absx, absy);
}

void TDEHTMLView::moveCaretBy(bool next, CaretMovement cmv, int count)
{
  if (!m_part) return;
  Node &caretNodeRef = m_part->d->caretNode();
  if (caretNodeRef.isNull()) return;

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long &offset = m_part->d->caretOffset();

  ElementImpl *baseElem = determineBaseElement(caretNode);
  CaretAdvancePolicy advpol = cmv != CaretByWord ? IndicatedFlows : LeafsOnly;
  LinearDocument ld(m_part, caretNode, offset, advpol, baseElem);

  EditableCharacterIterator it(&ld);
  while (!it.isEnd() && count > 0) {
    count--;
    if (cmv == CaretByCharacter) {
      if (next) ++it;
      else --it;
    } else if (cmv == CaretByWord) {
      if (next) moveItToNextWord(it);
      else moveItToPrevWord(it);
    }/*end if*/
//kdDebug(6200) << "movecaret" << endl;
  }/*wend*/
  CaretBox *hintBox = 0;	// make gcc uninit warning disappear
  if (!it.isEnd()) {
    NodeImpl *node = caretNodeRef.handle();
    hintBox = it.caretBox();
//kdDebug(6200) << "hintBox = " << hintBox << endl;
//kdDebug(6200) << " outside " << hintBox->isOutside() << " outsideEnd " << hintBox->isOutsideEnd() << " r " << it.renderer() << " ofs " << it.offset() << " cb " << hintBox->containingBlock() << endl;
    mapRenderPosToDOMPos(it.renderer(), it.offset(), hintBox->isOutside(),
    		hintBox->isOutsideEnd(), node, offset);
//kdDebug(6200) << "mapRTD" << endl;
    caretNodeRef = node;
#if DEBUG_CARETMODE > 2
    kdDebug(6200) << "set by valid node " << node << " " << (node?node->nodeName().string():TQString::null) << " offset: " << offset << endl;
#endif
  } else {
    offset = next ? caretNode->maxOffset() : caretNode->minOffset();
#if DEBUG_CARETMODE > 0
    kdDebug(6200) << "set by INvalid node. offset: " << offset << endl;
#endif
  }/*end if*/
  placeCaretOnChar(hintBox);
}

void TDEHTMLView::placeCaretOnChar(CaretBox *hintBox)
{
  caretOff();
  recalcAndStoreCaretPos(hintBox);
  ensureVisible(d->m_caretViewContext->x, d->m_caretViewContext->y,
  	d->m_caretViewContext->width, d->m_caretViewContext->height);
  d->m_caretViewContext->origX = d->m_caretViewContext->x;
  d->scrollBarMoved = false;
#if DEBUG_CARETMODE > 3
  //if (caretNode->isTextNode())  kdDebug(6200) << "text[0] = " << (int)*((TextImpl *)caretNode)->data().unicode() << " text :\"" << ((TextImpl *)caretNode)->data().string() << "\"" << endl;
#endif
  ensureNodeHasFocus(m_part->d->caretNode().handle());
  caretOn();
}

void TDEHTMLView::moveCaretByPage(bool next)
{
  Node &caretNodeRef = m_part->d->caretNode();
  if (caretNodeRef.isNull()) return;

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long offset = m_part->d->caretOffset();

  int offs = (clipper()->height() < 30) ? clipper()->height() : 30;
  // Minimum distance the caret must be moved
  int mindist = clipper()->height() - offs;

  CaretViewContext *cv = d->caretViewContext();
//  int y = cv->y;		// we always measure the top border

  ElementImpl *baseElem = determineBaseElement(caretNode);
  LinearDocument ld(m_part, caretNode, offset, LeafsOnly, baseElem);

  ErgonomicEditableLineIterator it(ld.current(), cv->origX);

  moveIteratorByPage(ld, it, mindist, next);

  int x, absx, absy;
  CaretBox *caretBox = nearestCaretBox(it, d->m_caretViewContext, x, absx, absy);

  placeCaretOnLine(caretBox, x, absx, absy);
}

void TDEHTMLView::moveCaretPrevWord()
{
  moveCaretBy(false, CaretByWord, 1);
}

void TDEHTMLView::moveCaretNextWord()
{
  moveCaretBy(true, CaretByWord, 1);
}

void TDEHTMLView::moveCaretPrevLine(int n)
{
  moveCaretByLine(false, n);
}

void TDEHTMLView::moveCaretNextLine(int n)
{
  moveCaretByLine(true, n);
}

void TDEHTMLView::moveCaretPrevPage()
{
  moveCaretByPage(false);
}

void TDEHTMLView::moveCaretNextPage()
{
  moveCaretByPage(true);
}

void TDEHTMLView::moveCaretToLineBegin()
{
  moveCaretToLineBoundary(false);
}

void TDEHTMLView::moveCaretToLineEnd()
{
  moveCaretToLineBoundary(true);
}

#endif // TDEHTML_NO_CARET

#ifndef NO_SMOOTH_SCROLL_HACK
#define timer timer2

// All scrolls must be completed within 240ms of last keypress
static const int SCROLL_TIME = 240;
// Each step is 20 ms == 50 frames/second
static const int SCROLL_TICK = 20;

void TDEHTMLView::scrollBy(int dx, int dy)
{
    TDEConfigGroup cfg( TDEGlobal::config(), "KDE" );
    if( !cfg.readBoolEntry( "SmoothScrolling", false )) {
        TQScrollView::scrollBy( dx, dy );
        return;
    }
    // scrolling destination
    int full_dx = d->dx + dx;
    int full_dy = d->dy + dy;

    // scrolling speed
    int ddx = 0;
    int ddy = 0;

    int steps = SCROLL_TIME/SCROLL_TICK;

    ddx = (full_dx*16)/steps;
    ddy = (full_dy*16)/steps;

    // don't go under 1px/step
    if (ddx > 0 && ddx < 16) ddx = 16;
    if (ddy > 0 && ddy < 16) ddy = 16;
    if (ddx < 0 && ddx > -16) ddx = -16;
    if (ddy < 0 && ddy > -16) ddy = -16;

    d->dx = full_dx;
    d->dy = full_dy;
    d->ddx = ddx;
    d->ddy = ddy;

    if (!d->scrolling) {
        scrollTick();
        startScrolling();
    }
}

void TDEHTMLView::scrollTick() {
    if (d->dx == 0 && d->dy == 0) {
        stopScrolling();
        return;
    }

    int tddx = d->ddx + d->rdx;
    int tddy = d->ddy + d->rdy;

    int ddx = tddx / 16;
    int ddy = tddy / 16;
    d->rdx = tddx % 16;
    d->rdy = tddy % 16;

    if (d->dx > 0 && ddx > d->dx) ddx = d->dx;
    else
    if (d->dx < 0 && ddx < d->dx) ddx = d->dx;

    if (d->dy > 0 && ddy > d->dy) ddy = d->dy;
    else
    if (d->dy < 0 && ddy < d->dy) ddy = d->dy;

    d->dx -= ddx;
    d->dy -= ddy;

//    TQScrollView::setContentsPos( contentsX() + ddx, contentsY() + ddy);
    kapp->syncX();
    TQScrollView::scrollBy(ddx, ddy);
// Unaccelerated X can get seriously overloaded by scrolling and for some reason
// will send KeyPress events only infrequently. This should help to reduce
// the load.
    kapp->syncX();
}

void TDEHTMLView::startScrolling()
{
    d->scrolling = true;
    d->timer.start(SCROLL_TICK, false);
}

void TDEHTMLView::stopScrolling()
{
    d->timer.stop();
    d->dx = d->dy = 0;
    d->scrolling = false;
}

// Overloaded from TQScrollView and TQScrollBar
void TDEHTMLView::scrollViewWheelEvent( TQWheelEvent *e )
{
    int pageStep = verticalScrollBar()->pageStep();
    int lineStep = verticalScrollBar()->lineStep();
    int step = TQMIN( TQApplication::wheelScrollLines()*lineStep, pageStep );
    if ( ( e->state() & ControlButton ) || ( e->state() & ShiftButton ) )
        step = pageStep;

    if(e->orientation() == TQt::Horizontal)
        scrollBy(-((e->delta()*step)/120), 0);
    else if(e->orientation() == TQt::Vertical)
        scrollBy(0,-((e->delta()*step)/120));

    e->accept();
}

#undef timer

#endif // NO_SMOOTH_SCROLL_HACK

#undef DEBUG_CARETMODE
