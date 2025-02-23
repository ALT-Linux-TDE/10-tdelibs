/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Simon Hausmann <hausmann@kde.org>
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
#ifndef __render_frames_h__
#define __render_frames_h__

#include "rendering/render_replaced.h"
#include "xml/dom_nodeimpl.h"
#include "html/html_baseimpl.h"
class TDEHTMLView;

namespace DOM
{
  class HTMLFrameElementImpl;
  class HTMLElementImpl;
  class MouseEventImpl;
}

namespace tdehtml
{
    class ChildFrame;

class RenderFrameSet : public RenderBox
{
    friend class DOM::HTMLFrameSetElementImpl;
public:
    RenderFrameSet( DOM::HTMLFrameSetElementImpl *frameSet );

    virtual ~RenderFrameSet();

    virtual const char *renderName() const { return "RenderFrameSet"; }
    virtual bool isFrameSet() const { return true; }

    virtual void layout();

    void positionFrames( );

    bool resizing() const { return m_resizing; }
    bool noResize() const { return element()->noResize(); }

    bool userResize( DOM::MouseEventImpl *evt );
    bool canResize( int _x, int _y);
    void setResizing(bool e);

    TQt::CursorShape cursorShape() const { return m_cursor; }

    bool nodeAtPoint(NodeInfo& info, int x, int y, int tx, int ty, HitTestAction hitTestAction, bool inside);

    DOM::HTMLFrameSetElementImpl *element() const
    { return static_cast<DOM::HTMLFrameSetElementImpl*>(RenderObject::element()); }

#ifdef ENABLE_DUMP
    virtual void dump(TQTextStream &stream, const TQString &ind) const;
#endif

private:
    TQt::CursorShape m_cursor;
    int m_oldpos;
    int m_gridLen[2];
    int* m_gridDelta[2];
    int* m_gridLayout[2];

    bool *m_hSplitVar; // is this split variable?
    bool *m_vSplitVar;

    int m_hSplit;     // the split currently resized
    int m_vSplit;
    int m_hSplitPos;
    int m_vSplitPos;

    bool m_resizing;
    bool m_clientresizing;
};

class RenderPart : public tdehtml::RenderWidget
{
    TQ_OBJECT
public:
    RenderPart(DOM::HTMLElementImpl* node);

    virtual const char *renderName() const { return "RenderPart"; }

    virtual void setWidget( TQWidget *widget );

    /**
     * Called by TDEHTMLPart to notify the frame object that loading the
     * part was not successfuly. (called either asyncroniously after a
     * after the servicetype of the given url (the one passed with requestObject)
     * has been determined or syncroniously from within requestObject)
     *
     * The default implementation does nothing.
     *
     * Return false in the normal case, return true if a fallback was found
     * and the url was successfully opened.
     */
    virtual bool partLoadingErrorNotify( tdehtml::ChildFrame *childFrame, const KURL& url, const TQString& serviceType );

    virtual short intrinsicWidth() const;
    virtual int intrinsicHeight() const;

public slots:
    virtual void slotViewCleared();
};

class RenderFrame : public tdehtml::RenderPart
{
    TQ_OBJECT
public:
    RenderFrame( DOM::HTMLFrameElementImpl *frame );

    virtual const char *renderName() const { return "RenderFrame"; }
    virtual bool isFrame() const { return true; }

    // frames never have padding
    virtual int paddingTop() const { return 0; }
    virtual int paddingBottom() const { return 0; }
    virtual int paddingLeft() const { return 0; }
    virtual int paddingRight() const { return 0; }

    DOM::HTMLFrameElementImpl *element() const
    { return static_cast<DOM::HTMLFrameElementImpl*>(RenderObject::element()); }

public slots:
    void slotViewCleared();
};

// I can hardly call the class RenderObject ;-)
class RenderPartObject : public tdehtml::RenderPart
{
    TQ_OBJECT
public:
    RenderPartObject( DOM::HTMLElementImpl * );

    virtual const char *renderName() const { return "RenderPartObject"; }

    virtual void close();

    virtual void layout( );
    virtual void updateWidget();
    
    virtual bool canHaveBorder() const { return true; }

    virtual bool partLoadingErrorNotify( tdehtml::ChildFrame *childFrame, const KURL& url, const TQString& serviceType );

public slots:
    void slotViewCleared();
private slots:
    void slotPartLoadingErrorNotify();
};

}

#endif
