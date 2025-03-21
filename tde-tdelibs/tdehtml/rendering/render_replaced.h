/*
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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
#ifndef render_replaced_h
#define render_replaced_h

#include "rendering/render_block.h"
#include <tqobject.h>
#include <tqscrollview.h>

class TDEHTMLView;
class TQWidget;

namespace DOM
{
    class MouseEventImpl;
}

namespace tdehtml {

class RenderReplaced : public RenderBox
{
public:
    RenderReplaced(DOM::NodeImpl* node);

    virtual const char *renderName() const { return "RenderReplaced"; }
    virtual bool isRenderReplaced() const { return true; }

    virtual bool childAllowed() const { return false; }

    virtual void calcMinMaxWidth();

    virtual short intrinsicWidth() const { return m_intrinsicWidth; }
    virtual int intrinsicHeight() const { return m_intrinsicHeight; }

    void setIntrinsicWidth(int w) {  m_intrinsicWidth = w; }
    void setIntrinsicHeight(int h) { m_intrinsicHeight = h; }

    virtual void position(InlineBox*, int, int, bool);

    // Return before, after (offset set to max), or inside the replaced element,
    // at @p offset
    virtual FindSelectionResult checkSelectionPoint( int _x, int _y, int _tx, int _ty,
                                                     DOM::NodeImpl*& node, int & offset,
						     SelPointState & );

    /** returns the lowest possible value the caret offset may have to
     * still point to a valid position.
     *
     * Returns 0.
     */
    virtual long minOffset() const { return 0; }
    /** returns the highest possible value the caret offset may have to
     * still point to a valid position.
     *
     * Returns 0.
     */
    virtual long maxOffset() const { return 0; }

protected:
    short m_intrinsicWidth;
    short m_intrinsicHeight;
};


class RenderWidget : public TQObject, public RenderReplaced, public tdehtml::Shared<RenderWidget>
{
    TQ_OBJECT
public:
    RenderWidget(DOM::NodeImpl* node);
    virtual ~RenderWidget();

    virtual void setStyle(RenderStyle *style);
    virtual void paint( PaintInfo& i, int tx, int ty );
    virtual bool isWidget() const { return true; };

    virtual bool isFrame() const { return false; }

    virtual void detach( );
    virtual void layout( );

    virtual void updateFromElement();

    TQWidget *widget() const { return m_widget; }
    TDEHTMLView* view() const { return m_view; }

    void deref();

    void cancelPendingResize();
    bool needsMask() const { return m_needsMask; }

    static void paintWidget(PaintInfo& pI, TQWidget *widget, int tx, int ty);
    virtual bool handleEvent(const DOM::EventImpl& ev);

#ifdef ENABLE_DUMP
    virtual void dump(TQTextStream &stream, const TQString &ind) const;
#endif

    // for ECMA to flush all pending resizes
    TDE_EXPORT static void flushWidgetResizes();

public slots:
    void slotWidgetDestructed();
    bool isTDEHTMLWidget() const { return m_isTDEHTMLWidget; }

protected:
    virtual bool canHaveBorder() const { return false; }

    virtual bool acceptsSyntheticEvents() const { return true; }

    virtual void handleFocusOut() {}
    bool event( TQEvent *e );

    bool eventFilter(TQObject* /*o*/, TQEvent* e);
    void setQWidget(TQWidget *widget);
    void resizeWidget( int w, int h );

    TQWidget *m_widget;
    TDEHTMLView* m_view;

    //Because we mess with normal detach due to ref/deref,
    //we need to keep track of the arena ourselves
    //so it doesn't get yanked from us, etc.
    SharedPtr<RenderArena> m_arena; 

    bool m_resizePending;
    bool m_discardResizes;
    bool m_isTDEHTMLWidget;
    bool m_needsMask;

public:
    virtual int borderTop() const { return canHaveBorder() ? RenderReplaced::borderTop() : 0; }
    virtual int borderBottom() const { return canHaveBorder() ? RenderReplaced::borderBottom() : 0; }
    virtual int borderLeft() const { return canHaveBorder() ? RenderReplaced::borderLeft() : 0; }
    virtual int borderRight() const { return canHaveBorder() ? RenderReplaced::borderRight() : 0; }

    class EventPropagator : public TQWidget {
    public:
        void sendEvent(TQEvent *e);
    };
    class ScrollViewEventPropagator : public TQScrollView {
    public:
        void sendEvent(TQEvent *e);
    };
};

extern bool allowWidgetPaintEvents;

}

#endif
