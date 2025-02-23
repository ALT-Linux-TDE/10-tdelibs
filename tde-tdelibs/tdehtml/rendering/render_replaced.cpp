/**
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2000-2003 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003 Apple Computer, Inc.
 * Copyright (C) 2004 Germain Garand (germain@ebooksfrance.org)
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
#include "render_replaced.h"
#include "render_layer.h"
#include "render_canvas.h"
#include "render_line.h"

#include "render_arena.h"

#include <assert.h>
#include <tqwidget.h>
#include <tqpainter.h>
#include <tqevent.h>
#include <tqapplication.h>
#include <tqlineedit.h>
#include <tdeglobalsettings.h>
#include <tqobjectlist.h>
#include <tqvaluevector.h>

#include "tdehtml_ext.h"
#include "tdehtmlview.h"
#include "xml/dom2_eventsimpl.h"
#include "tdehtml_part.h"
#include "xml/dom_docimpl.h"
#include <kdebug.h>

bool tdehtml::allowWidgetPaintEvents = false;

using namespace tdehtml;
using namespace DOM;


RenderReplaced::RenderReplaced(DOM::NodeImpl* node)
    : RenderBox(node)
{
    // init RenderObject attributes
    setReplaced(true);

    m_intrinsicWidth = 300;
    m_intrinsicHeight = 150;
}

void RenderReplaced::calcMinMaxWidth()
{
    TDEHTMLAssert( !minMaxKnown());

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "RenderReplaced::calcMinMaxWidth() known=" << minMaxKnown() << endl;
#endif

    m_width = calcReplacedWidth();
    m_width = calcBoxWidth( m_width );

    if ( style()->width().isPercent() || style()->height().isPercent() || 
		    style()->maxWidth().isPercent() || style()->maxHeight().isPercent() ||
		    style()->minWidth().isPercent() || style()->minHeight().isPercent() ) {
        m_minWidth = 0;
        m_maxWidth = m_width;
    }
    else
        m_minWidth = m_maxWidth = m_width;

    setMinMaxKnown();
}

void RenderReplaced::position(InlineBox* box, int /*from*/, int /*len*/, bool /*reverse*/)
{
    setPos( box->xPos(), box->yPos() );
}

// -----------------------------------------------------------------------------

RenderWidget::RenderWidget(DOM::NodeImpl* node)
        : RenderReplaced(node)
{
    m_widget = 0;
    // a widget doesn't support being anonymous
    assert(!isAnonymous());
    m_view  = node->getDocument()->view();
    m_arena.reset(renderArena());
    m_resizePending = false;
    m_discardResizes = false;
    m_isTDEHTMLWidget = false;
    m_needsMask = false;

    // this is no real reference counting, its just there
    // to make sure that we're not deleted while we're recursed
    // in an eventFilter of the widget
    ref();
}

void RenderWidget::detach()
{
    remove();
    deleteInlineBoxes();

    if ( m_widget ) {
        if ( m_view ) {
            m_view->setWidgetVisible(this, false);
            m_view->removeChild( m_widget );
        }

        m_widget->removeEventFilter( this );
        m_widget->setMouseTracking( false );
    }

    deref();
}

RenderWidget::~RenderWidget()
{
    TDEHTMLAssert( refCount() <= 0 );

    if(m_widget) {
        m_widget->hide();
        m_widget->deleteLater();
    }
}

class TQWidgetResizeEvent : public TQEvent
{
public:
    enum { Type = TQEvent::User + 0xbee };
    TQWidgetResizeEvent( int _w,  int _h ) :
        TQEvent( ( TQEvent::Type ) Type ),  w( _w ), h( _h ) {}
    int w;
    int h;
};

void  RenderWidget::resizeWidget( int w, int h )
{
    // ugly hack to limit the maximum size of the widget ( as X11 has problems if
    // its bigger )
    h = kMin( h, 3072 );
    w = kMin( w, 2000 );

    if (m_widget->width() != w || m_widget->height() != h) {
        m_resizePending = isTDEHTMLWidget();
        ref();
        element()->ref();
        TQApplication::postEvent( this, new TQWidgetResizeEvent( w, h ) );
        element()->deref();
        deref();
    }
}

void RenderWidget::cancelPendingResize()
{
    if (!m_widget)
        return;
    m_discardResizes = true;
    TQApplication::sendPostedEvents(this, TQWidgetResizeEvent::Type);
    m_discardResizes = false;
}

bool RenderWidget::event( TQEvent *e )
{
    if ( m_widget && (e->type() == (TQEvent::Type)TQWidgetResizeEvent::Type) ) {
        m_resizePending = false;
        if (m_discardResizes)
            return true;
        TQWidgetResizeEvent *re = static_cast<TQWidgetResizeEvent *>(e);
        m_widget->resize( re->w,  re->h );
        repaint();
    }
    // eat all events - except if this is a frame (in which case TDEHTMLView handles it all)
    if ( ::tqt_cast<TDEHTMLView *>( m_widget ) )
        return TQObject::event( e );
    return true;
}

void RenderWidget::flushWidgetResizes() //static
{
    TQApplication::sendPostedEvents( 0, TQWidgetResizeEvent::Type );
}

void RenderWidget::setQWidget(TQWidget *widget)
{
    if (widget != m_widget)
    {
        if (m_widget) {
            m_widget->removeEventFilter(this);
            disconnect( m_widget, TQ_SIGNAL( destroyed()), this, TQ_SLOT( slotWidgetDestructed()));
            m_widget->hide();
            m_widget->deleteLater(); //Might happen due to event on the widget, so be careful
            m_widget = 0;
        }
        m_widget = widget;
        if (m_widget) {
            connect( m_widget, TQ_SIGNAL( destroyed()), this, TQ_SLOT( slotWidgetDestructed()));
            m_widget->installEventFilter(this);

            if ( (m_isTDEHTMLWidget = !strcmp(m_widget->name(), "__tdehtml")) && !::tqt_cast<TQFrame*>(m_widget))
                m_widget->setBackgroundMode( TQWidget::NoBackground );

            if (m_widget->focusPolicy() > TQWidget::StrongFocus)
                m_widget->setFocusPolicy(TQWidget::StrongFocus);
            // if we've already received a layout, apply the calculated space to the
            // widget immediately, but we have to have really been full constructed (with a non-null
            // style pointer).
            if (!needsLayout() && style()) {
                resizeWidget( m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
                              m_height-borderTop()-borderBottom()-paddingTop()-paddingBottom() );
            }
            else
                setPos(xPos(), -500000);
        }
        m_view->setWidgetVisible(this, false);
        m_view->addChild( m_widget, 0, -500000);
        if ( m_widget ) m_widget->hide();
        m_resizePending = false;
    }
}

void RenderWidget::layout( )
{
    TDEHTMLAssert( needsLayout() );
    TDEHTMLAssert( minMaxKnown() );
    if ( m_widget ) {
        resizeWidget( m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
                      m_height-borderTop()-borderBottom()-paddingTop()-paddingBottom() );
        if (!isTDEHTMLWidget() && !isFrame() && !m_needsMask) {
            m_needsMask = true;
            RenderLayer* rl = enclosingStackingContext();
            RenderLayer* el = enclosingLayer();
            while (rl && el && el != rl) {
                if (el->renderer()->style()->position() != STATIC) {
                    m_needsMask = false;
                    break;
                }
                el = el->parent();
            }                                                                                                                                      
            if (m_needsMask) {
                if (rl) rl->setHasOverlaidWidgets();
                canvas()->setNeedsWidgetMasks();
            }
        }
    }

    setNeedsLayout(false);
}

void RenderWidget::updateFromElement()
{
    if (m_widget) {
        // Color:
        TQColor color = style()->color();
        TQColor backgroundColor = style()->backgroundColor();

        if ( color.isValid() || backgroundColor.isValid() ) {
            TQPalette pal(TQApplication::palette(m_widget));

            int contrast_ = TDEGlobalSettings::contrast();
            int highlightVal = 100 + (2*contrast_+4)*16/10;
            int lowlightVal = 100 + (2*contrast_+4)*10;

            if (backgroundColor.isValid()) {
                if (!isTDEHTMLWidget())
                    widget()->setEraseColor(backgroundColor );
                for ( int i = 0; i < TQPalette::NColorGroups; ++i ) {
                    pal.setColor( (TQPalette::ColorGroup)i, TQColorGroup::Background, backgroundColor );
                    pal.setColor( (TQPalette::ColorGroup)i, TQColorGroup::Light, backgroundColor.light(highlightVal) );
                    pal.setColor( (TQPalette::ColorGroup)i, TQColorGroup::Dark, backgroundColor.dark(lowlightVal) );
                    pal.setColor( (TQPalette::ColorGroup)i, TQColorGroup::Mid, backgroundColor.dark(120) );
                    pal.setColor( (TQPalette::ColorGroup)i, TQColorGroup::Midlight, backgroundColor.light(110) );
                    pal.setColor( (TQPalette::ColorGroup)i, TQColorGroup::Button, backgroundColor );
                    pal.setColor( (TQPalette::ColorGroup)i, TQColorGroup::Base, backgroundColor );
            }
            }
            if ( color.isValid() ) {
                struct ColorSet {
                    TQPalette::ColorGroup cg;
                    TQColorGroup::ColorRole cr;
                };
                const struct ColorSet toSet [] = {
                    { TQPalette::Active, TQColorGroup::Foreground },
                    { TQPalette::Active, TQColorGroup::ButtonText },
                    { TQPalette::Active, TQColorGroup::Text },
                    { TQPalette::Inactive, TQColorGroup::Foreground },
                    { TQPalette::Inactive, TQColorGroup::ButtonText },
                    { TQPalette::Inactive, TQColorGroup::Text },
                    { TQPalette::Disabled,TQColorGroup::ButtonText },
                    { TQPalette::NColorGroups, TQColorGroup::NColorRoles },
                };
                const ColorSet *set = toSet;
                while( set->cg != TQPalette::NColorGroups ) {
                    pal.setColor( set->cg, set->cr, color );
                    ++set;
                }

                TQColor disfg = color;
                int h, s, v;
                disfg.hsv( &h, &s, &v );
                if (v > 128)
                    // dark bg, light fg - need a darker disabled fg
                    disfg = disfg.dark(lowlightVal);
                else if (disfg != TQt::black)
                    // light bg, dark fg - need a lighter disabled fg - but only if !black
                    disfg = disfg.light(highlightVal);
                else
                    // black fg - use darkgray disabled fg
                    disfg = TQt::darkGray;
                pal.setColor(TQPalette::Disabled,TQColorGroup::Foreground,disfg);
            }

            m_widget->setPalette(pal);
        }
        else
            m_widget->unsetPalette();
        // Border:
        TQFrame* frame = ::tqt_cast<TQFrame*>(m_widget);
        if (frame) {
            if (shouldPaintBackgroundOrBorder())
            {
                frame->setFrameShape(TQFrame::NoFrame);
            }
        }

    }

    RenderReplaced::updateFromElement();
}

void RenderWidget::slotWidgetDestructed()
{
    if (m_view)
       m_view->setWidgetVisible(this, false);
    m_widget = 0;
}

void RenderWidget::setStyle(RenderStyle *_style)
{
    RenderReplaced::setStyle(_style);
    if(m_widget)
    {
        m_widget->setFont(style()->font());
        if (style()->visibility() != VISIBLE) {
            if (m_view)
                m_view->setWidgetVisible(this, false);
            m_widget->hide();
        }
    }

    // Don't paint borders if the border-style is native
    // or borders are not supported on this widget
    if (!canHaveBorder() ||
        (style()->borderLeftStyle()   == BNATIVE &&
         style()->borderRightStyle()  == BNATIVE &&
         style()->borderTopStyle()    == BNATIVE &&
         style()->borderBottomStyle() == BNATIVE))
    {
        setShouldPaintBackgroundOrBorder(false);
    }
}

void RenderWidget::paint(PaintInfo& paintInfo, int _tx, int _ty)
{
    _tx += m_x;
    _ty += m_y;
 
    if (shouldPaintBackgroundOrBorder() && 
          (paintInfo.phase == PaintActionChildBackground || paintInfo.phase == PaintActionChildBackgrounds))
        paintBoxDecorations(paintInfo, _tx, _ty);

    if (!m_widget || !m_view || paintInfo.phase != PaintActionForeground)
        return;

    // not visible or not even once layouted
    if (style()->visibility() != VISIBLE || m_y <= -500000 || m_resizePending )
        return;

    if ( (_ty > paintInfo.r.bottom()) || (_ty + m_height <= paintInfo.r.top()) ||
         (_tx + m_width <= paintInfo.r.left()) || (_tx > paintInfo.r.right()) )
        return;

    int xPos = _tx+borderLeft()+paddingLeft();
    int yPos = _ty+borderTop()+paddingTop();

    bool tdehtmlw = isTDEHTMLWidget();
    int childw = m_widget->width();
    int childh = m_widget->height();
    if ( (childw == 2000 || childh == 3072) && m_widget->inherits( "TDEHTMLView" ) ) {
        TDEHTMLView *vw = static_cast<TDEHTMLView *>(m_widget);
        int cy = m_view->contentsY();
        int ch = m_view->visibleHeight();


        int childx = m_view->childX( m_widget );
        int childy = m_view->childY( m_widget );

        int xNew = xPos;
        int yNew = childy;

        //         tqDebug("cy=%d, ch=%d, childy=%d, childh=%d", cy, ch, childy, childh );
        if ( childh == 3072 ) {
            if ( cy + ch > childy + childh ) {
                yNew = cy + ( ch - childh )/2;
            } else if ( cy < childy ) {
                yNew = cy + ( ch - childh )/2;
            }
//             tqDebug("calculated yNew=%d", yNew);
        }
        yNew = kMin( yNew, yPos + m_height - childh );
        yNew = kMax( yNew, yPos );
        if ( yNew != childy || xNew != childx ) {
            if ( vw->contentsHeight() < yNew - yPos + childh )
                vw->resizeContents( vw->contentsWidth(), yNew - yPos + childh );
            vw->setContentsPos( xNew - xPos, yNew - yPos );
        }
        xPos = xNew;
        yPos = yNew;
    }
    m_view->setWidgetVisible(this, true);
    m_view->addChild(m_widget, xPos, yPos );
    m_widget->show();
    if (tdehtmlw)
        paintWidget(paintInfo, m_widget, xPos, yPos);
}

#include <tqinternal_p.h>

// The PaintBuffer class provides a shared buffer for widget painting.
//
// It will grow to encompass the biggest widget encountered, in order to avoid
// constantly resizing.
// When it grows over maxPixelBuffering, it periodically checks if such a size
// is still needed.  If not, it shrinks down to the biggest size < maxPixelBuffering
// that was requested during the overflow lapse.

class PaintBuffer: public TQObject
{
public:
    static const int maxPixelBuffering = 320*200;
    static const int leaseTime = 20*1000;

    static TQPixmap *grab( TQSize s = TQSize() ) {
        if (!m_inst)
            m_inst = new PaintBuffer;
        return m_inst->getBuf( s );
    }
    static void release() { m_inst->m_grabbed = false; }
protected:
    PaintBuffer(): m_overflow(false), m_grabbed(false),
                   m_timer(0), m_resetWidth(0), m_resetHeight(0) {};
    void timerEvent(TQTimerEvent* e) {
        assert( m_timer == e->timerId() );
        if (m_grabbed)
            return;
        m_buf.resize(m_resetWidth, m_resetHeight);
        m_resetWidth = m_resetHeight = 0;
        killTimer( m_timer );
        m_timer = 0;
    }

    TQPixmap *getBuf( TQSize s ) {
        assert( !m_grabbed );
        if (s.isEmpty())
            return 0;

        m_grabbed = true;
        bool cur_overflow = false;

        int nw = kMax(m_buf.width(), s.width());
        int nh = kMax(m_buf.height(), s.height());

        if (!m_overflow && (nw*nh > maxPixelBuffering))
            cur_overflow = true;

        if (nw != m_buf.width() || nh != m_buf.height())
            m_buf.resize(nw, nh);

        if (cur_overflow) {
            m_overflow = true;
            m_timer = startTimer( leaseTime );
        } else if (m_overflow) {
            if( s.width()*s.height() > maxPixelBuffering ) {
                killTimer( m_timer );
                m_timer = startTimer( leaseTime );
            } else {
                if (s.width() > m_resetWidth)
                    m_resetWidth = s.width();
                if (s.height() > m_resetHeight)
                    m_resetHeight = s.height();
            }
        }
        return &m_buf;
    }
private:
    static PaintBuffer* m_inst;
    TQPixmap m_buf;
    bool m_overflow;
    bool m_grabbed;
    int m_timer;
    int m_resetWidth;
    int m_resetHeight;
};

PaintBuffer *PaintBuffer::m_inst = 0;

static void copyWidget(const TQRect& r, TQPainter *p, TQWidget *widget, int tx, int ty)
{
    if (r.isNull() || r.isEmpty() )
        return;
    TQRegion blit(r);
    TQValueVector<TQWidget*> cw;
    TQValueVector<TQRect> cr;

    if (!widget->childrenListObject().isEmpty()) {
        // build region
        TQObjectList childWidgets = widget->childrenListObject();
        TQObjectListIterator it(childWidgets);
        for (; it.current(); ++it) {
            TQWidget* const w = ::tqt_cast<TQWidget *>(it.current());
	    if ( w && !w->isTopLevel() && !w->isHidden()) {
	        TQRect r2 = w->geometry();
	        blit -= r2;
	        r2 = r2.intersect( r );
	        r2.moveBy(-w->x(), -w->y());
	        cr.append(r2);
	        cw.append(w);
            }
        }
    }
    TQMemArray<TQRect> br = blit.rects();

    const int cnt = br.size();
    const bool external = p->device()->isExtDev();
    TQPixmap* const pm = PaintBuffer::grab( widget->size() );
    if (!pm)
    {
        kdWarning(6040) << "Rendering widget [ " << widget->className() << " ] failed due to invalid size." << endl;
        return;
    }

    // fill background
    if ( external ) {
	// even hackier!
        TQPainter pt( pm );
        const TQColor c = widget->colorGroup().base();
        for (int i = 0; i < cnt; ++i)
            pt.fillRect( br[i], c );
    } else {
        TQRect dr;
        for (int i = 0; i < cnt; ++i ) {
            dr = br[i];
	    dr.moveBy( tx, ty );
	    dr = p->xForm( dr );
	    bitBlt(pm, br[i].topLeft(), p->device(), dr);
        }
    }

    // send paint event
    TQPainter::redirect(widget, pm);
    TQPaintEvent e( r, false );
    TQApplication::sendEvent( widget, &e );
    TQPainter::redirect(widget, 0);

    // transfer result
    if ( external )
        for ( int i = 0; i < cnt; ++i )
            p->drawPixmap(TQPoint(tx+br[i].x(), ty+br[i].y()), *pm, br[i]);
    else
        for ( int i = 0; i < cnt; ++i )
            bitBlt(p->device(), p->xForm( TQPoint(tx, ty) + br[i].topLeft() ), pm, br[i]);

    // cleanup and recurse
    PaintBuffer::release();
    TQValueVector<TQWidget*>::iterator cwit = cw.begin();
    TQValueVector<TQWidget*>::iterator cwitEnd = cw.end();
    TQValueVector<TQRect>::const_iterator crit = cr.begin();
    for (; cwit != cwitEnd; ++cwit, ++crit)
        copyWidget(*crit, p, *cwit, tx+(*cwit)->x(), ty+(*cwit)->y());
}

void RenderWidget::paintWidget(PaintInfo& pI, TQWidget *widget, int tx, int ty)
{
    TQPainter* const p = pI.p;
    allowWidgetPaintEvents = true;

    const bool dsbld = TQSharedDoubleBuffer::isDisabled();
    TQSharedDoubleBuffer::setDisabled(true);
    TQRect rr = pI.r;
    rr.moveBy(-tx, -ty);
    const TQRect r = widget->rect().intersect( rr );
    copyWidget(r, p, widget, tx, ty);
    TQSharedDoubleBuffer::setDisabled(dsbld);

    allowWidgetPaintEvents = false;
}

bool RenderWidget::eventFilter(TQObject* /*o*/, TQEvent* e)
{
    // no special event processing if this is a frame (in which case TDEHTMLView handles it all)
    if ( ::tqt_cast<TDEHTMLView *>( m_widget ) )
        return false;
    if ( !element() ) return true;


    static bool directToWidget = false;
    if (directToWidget) {
      //We're trying to get the event to the widget 
      //promptly. So get out of here..
      return false;
    }

    ref();
    element()->ref();

    bool filtered = false;

    //kdDebug() << "RenderWidget::eventFilter type=" << e->type() << endl;
    switch(e->type()) {
    case TQEvent::FocusOut:
        // First, forward it to the widget, so that Qt gets a precise
        // state of the focus before pesky JS can try changing it..
        directToWidget = true;
        TQApplication::sendEvent(m_widget, e);
        directToWidget = false;
        filtered       = true; //We already delivered it!
        
        // Don't count popup as a valid reason for losing the focus
        // (example: opening the options of a select combobox shouldn't emit onblur)
        if ( static_cast<TQFocusEvent*>(e)->reason() != TQFocusEvent::Popup )
            handleFocusOut();
        break;
    case TQEvent::FocusIn:
        //As above, forward to the widget first...
        directToWidget = true;
        TQApplication::sendEvent(m_widget, e);
        directToWidget = false;
        filtered       = true; //We already delivered it!

        //kdDebug(6000) << "RenderWidget::eventFilter captures FocusIn" << endl;
        element()->getDocument()->setFocusNode(element());
//         if ( isEditable() ) {
//             TDEHTMLPartBrowserExtension *ext = static_cast<TDEHTMLPartBrowserExtension *>( element()->view->part()->browserExtension() );
//             if ( ext )  ext->editableWidgetFocused( m_widget );
//         }
        break;
    case TQEvent::KeyPress:
    case TQEvent::KeyRelease:
    // TODO this seems wrong - Qt events are not correctly translated to DOM ones,
    // like in TDEHTMLView::dispatchKeyEvent()
        if (element()->dispatchKeyEvent(static_cast<TQKeyEvent*>(e),false))
            filtered = true;
        break;

    case TQEvent::Wheel:
        if (widget()->parentWidget() == view()->viewport()) {
            // don't allow the widget to react to wheel event unless its
            // currently focused. this avoids accidentally changing a select box
            // or something while wheeling a webpage.
            if (tqApp->focusWidget() != widget() &&
                widget()->focusPolicy() <= TQWidget::StrongFocus)  {
                static_cast<TQWheelEvent*>(e)->ignore();
                TQApplication::sendEvent(view(), e);
                filtered = true;
            }
        }
        break;
    default:
        break;
    };

    element()->deref();

    // stop processing if the widget gets deleted, but continue in all other cases
    if (hasOneRef())
        filtered = true;
    deref();

    return filtered;
}

void RenderWidget::EventPropagator::sendEvent(TQEvent *e) {
    switch(e->type()) {
    case TQEvent::MouseButtonPress:
        mousePressEvent(static_cast<TQMouseEvent*>(e));
        break;
    case TQEvent::MouseButtonRelease:
        mouseReleaseEvent(static_cast<TQMouseEvent*>(e));
        break;
    case TQEvent::MouseButtonDblClick:
        mouseDoubleClickEvent(static_cast<TQMouseEvent*>(e));
        break;
    case TQEvent::MouseMove:
        mouseMoveEvent(static_cast<TQMouseEvent*>(e));
        break;
    case TQEvent::KeyPress:
        keyPressEvent(static_cast<TQKeyEvent*>(e));
        break;
    case TQEvent::KeyRelease:
        keyReleaseEvent(static_cast<TQKeyEvent*>(e));
        break;
    default:
        break;
    }
}

void RenderWidget::ScrollViewEventPropagator::sendEvent(TQEvent *e) {
    switch(e->type()) {
    case TQEvent::MouseButtonPress:
        viewportMousePressEvent(static_cast<TQMouseEvent*>(e));
        break;
    case TQEvent::MouseButtonRelease:
        viewportMouseReleaseEvent(static_cast<TQMouseEvent*>(e));
        break;
    case TQEvent::MouseButtonDblClick:
        viewportMouseDoubleClickEvent(static_cast<TQMouseEvent*>(e));
        break;
    case TQEvent::MouseMove:
        viewportMouseMoveEvent(static_cast<TQMouseEvent*>(e));
        break;
    case TQEvent::KeyPress:
        keyPressEvent(static_cast<TQKeyEvent*>(e));
        break;
    case TQEvent::KeyRelease:
        keyReleaseEvent(static_cast<TQKeyEvent*>(e));
        break;
    default:
        break;
    }
}

bool RenderWidget::handleEvent(const DOM::EventImpl& ev)
{
    bool ret = false;
    switch(ev.id()) {
    case EventImpl::MOUSEDOWN_EVENT:
    case EventImpl::MOUSEUP_EVENT:
    case EventImpl::MOUSEMOVE_EVENT: {
        if (!ev.isMouseEvent()) break;
        const MouseEventImpl &me = static_cast<const MouseEventImpl &>(ev);
        TQMouseEvent* const qme = me.qEvent();

        int absx = 0;
        int absy = 0;

        absolutePosition(absx, absy);
        TQPoint p(me.clientX() - absx + m_view->contentsX(),
                 me.clientY() - absy + m_view->contentsY());
        TQMouseEvent::Type type;
        int button = 0;
        int state = 0;

        if (qme) {
            button = qme->button();
            state = qme->state();
            type = qme->type();
        } else {
            switch(me.id())  {
            case EventImpl::MOUSEDOWN_EVENT:
                type = TQMouseEvent::MouseButtonPress;
                break;
            case EventImpl::MOUSEUP_EVENT:
                type = TQMouseEvent::MouseButtonRelease;
                break;
            case EventImpl::MOUSEMOVE_EVENT:
            default:
                type = TQMouseEvent::MouseMove;
                break;
            }
            switch (me.button()) {
            case 0:
                button = TQt::LeftButton;
                break;
            case 1:
                button = TQt::MidButton;
                break;
            case 2:
                button = TQt::RightButton;
                break;
            default:
                break;
            }
            if (me.ctrlKey())
                state |= ControlButton;
            if (me.altKey())
                state |= AltButton;
            if (me.shiftKey())
                state |= ShiftButton;
            if (me.metaKey())
                state |= MetaButton;
        }

//     kdDebug(6000) << "sending event to widget "
//                   << " pos=" << p << " type=" << type
//                   << " button=" << button << " state=" << state << endl;
        TQMouseEvent e(type, p, button, state);
        TQScrollView * sc = ::tqt_cast<TQScrollView*>(m_widget);
        if (sc && !::tqt_cast<TQListBox*>(m_widget))
            static_cast<ScrollViewEventPropagator *>(sc)->sendEvent(static_cast<TQEvent*>(&e));
        else
            static_cast<EventPropagator *>(m_widget)->sendEvent(static_cast<TQEvent*>(&e));
        ret = e.isAccepted();
        break;
    }
    case EventImpl::KEYDOWN_EVENT:
        // do nothing; see the mapping table below
        break;
    case EventImpl::KEYUP_EVENT: {
        if (!ev.isKeyRelatedEvent()) break;

        const KeyEventBaseImpl& domKeyEv = static_cast<const KeyEventBaseImpl &>(ev);
        if (domKeyEv.isSynthetic() && !acceptsSyntheticEvents()) break;

        TQKeyEvent* const ke = domKeyEv.qKeyEvent();
        static_cast<EventPropagator *>(m_widget)->sendEvent(static_cast<TQEvent*>(ke));
        ret = ke->isAccepted();
        break;
    }
    case EventImpl::KEYPRESS_EVENT: {
        if (!ev.isKeyRelatedEvent()) break;

        const KeyEventBaseImpl& domKeyEv = static_cast<const KeyEventBaseImpl &>(ev);
        if (domKeyEv.isSynthetic() && !acceptsSyntheticEvents()) break;

        // See TDEHTMLView::dispatchKeyEvent: autorepeat is just keypress in the DOM
        // but it's keyrelease+keypress in Qt. So here we do the inverse mapping as
        // the one done in TDEHTMLView: generate two events for one DOM auto-repeat keypress.
        // Similarly, DOM keypress events with non-autorepeat Qt event do nothing here,
        // because the matching Qt keypress event was already sent from DOM keydown event.

        // Reverse drawing as the one in TDEHTMLView:
        //  DOM:   Down      Press   |       Press                             |     Up
        //  Qt:    (nothing) Press   | Release(autorepeat) + Press(autorepeat) |   Release
        //
        // TQt::KeyPress is sent for DOM keypress and not DOM keydown to allow
        // sites to block a key with onkeypress, #99749

        TQKeyEvent* const ke = domKeyEv.qKeyEvent();
        if (ke->isAutoRepeat()) {
            TQKeyEvent releaseEv( TQEvent::KeyRelease, ke->key(), ke->ascii(), ke->state(),
                               ke->text(), ke->isAutoRepeat(), ke->count() );
            static_cast<EventPropagator *>(m_widget)->sendEvent(static_cast<TQEvent*>(&releaseEv));
        }
        static_cast<EventPropagator *>(m_widget)->sendEvent(static_cast<TQEvent*>(ke));
        ret = ke->isAccepted();
	break;
    }
    case EventImpl::MOUSEOUT_EVENT: {
	TQEvent moe( TQEvent::Leave );
	TQApplication::sendEvent(m_widget, &moe);
	break;
    }
    case EventImpl::MOUSEOVER_EVENT: {
	TQEvent moe( TQEvent::Enter );
	TQApplication::sendEvent(m_widget, &moe);
	view()->part()->resetHoverText();
	break;
    }
    default:
        break;
    }
    return ret;
}

void RenderWidget::deref()
{
    if (_ref) _ref--;
//     tqDebug( "deref(%p): width get count is %d", this, _ref);
    if (!_ref) {
        tdehtml::SharedPtr<RenderArena> guard(m_arena); //Since delete on us gets called -first-,
                                                      //before the arena free
        arenaDelete(m_arena.get());
    }
}

FindSelectionResult RenderReplaced::checkSelectionPoint(int _x, int _y, int _tx, int _ty, DOM::NodeImpl*& node, int &offset, SelPointState &)
{
#if 0
    kdDebug(6040) << "RenderReplaced::checkSelectionPoint(_x="<<_x<<",_y="<<_y<<",_tx="<<_tx<<",_ty="<<_ty<<")" << endl
                    << "xPos: " << xPos() << " yPos: " << yPos() << " width: " << width() << " height: " << height() << endl
                << "_ty + yPos: " << (_ty + yPos()) << " + height: " << (_ty + yPos() + height()) << "; _tx + xPos: " << (_tx + xPos()) << " + width: " << (_tx + xPos() + width()) << endl;
#endif
    node = element();
    offset = 0;

    if ( _y < _ty + yPos() )
        return SelectionPointBefore; // above -> before

    if ( _y > _ty + yPos() + height() ) {
        // below -> after
        // Set the offset to the max
        offset = 1;
        return SelectionPointAfter;
    }
    if ( _x > _tx + xPos() + width() ) {
        // to the right
        // ### how to regard bidi in replaced elements? (LS)
        offset = 1;
        return SelectionPointAfterInLine;
    }

    // The Y matches, check if we're on the left
    if ( _x < _tx + xPos() ) {
        // ### how to regard bidi in replaced elements? (LS)
        return SelectionPointBeforeInLine;
    }

    offset = _x > _tx + xPos() + width()/2;
    return SelectionPointInside;
}

#ifdef ENABLE_DUMP
void RenderWidget::dump(TQTextStream &stream, const TQString &ind) const
{
    RenderReplaced::dump(stream,ind);
    if ( widget() )
        stream << " color=" << widget()->foregroundColor().name()
               << " bg=" << widget()->backgroundColor().name();
    else
        stream << " null widget";
}
#endif

#include "render_replaced.moc"

