/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __tdehtml_events_h__
#define __tdehtml_events_h__

#include <tdeparts/event.h>

#include "dom/dom_node.h"
#include "dom/dom_string.h"

namespace tdehtml
{

class MouseEvent : public KParts::Event
{
public:
  MouseEvent( const char *name, TQMouseEvent *qmouseEvent, int x, int y,
              const DOM::DOMString &url, const DOM::DOMString& target,
              const DOM::Node &innerNode);
  virtual ~MouseEvent();

  TQMouseEvent *qmouseEvent() const { return m_qmouseEvent; }
  int x() const { return m_x; }
  int y() const { return m_y; }
  int absX() const { return m_nodeAbsX; }
  int absY() const { return m_nodeAbsY; }

  DOM::DOMString url() const { return m_url; }
  DOM::DOMString target() const { return m_target; }
  DOM::Node innerNode() const { return m_innerNode; }

  // return the offset of innerNode
  long offset() const;

private:
  TQMouseEvent *m_qmouseEvent;
  int m_x;
  int m_y;
  int m_nodeAbsX, m_nodeAbsY;
  DOM::DOMString m_url;
  DOM::DOMString m_target;
  DOM::Node m_innerNode;
  class MouseEventPrivate;
  MouseEventPrivate *d;
};

class MousePressEvent : public MouseEvent
{
public:
  MousePressEvent( TQMouseEvent *mouseEvent, int x, int y,
                   const DOM::DOMString &url, const DOM::DOMString& target,
                   const DOM::Node &innerNode)
  : MouseEvent( s_strMousePressEvent, mouseEvent, x, y, url, target, innerNode )
  {}

  static bool test( const TQEvent *event ) { return KParts::Event::test( event, s_strMousePressEvent ); }


private:
  static const char *s_strMousePressEvent;
};

class MouseDoubleClickEvent : public MouseEvent
{
public:
  // clickCount is 3 for a triple-click event
  MouseDoubleClickEvent( TQMouseEvent *mouseEvent, int x, int y,
                         const DOM::DOMString &url, const DOM::DOMString& target,
		         const DOM::Node &innerNode, int clickCount = 2 )
  : MouseEvent( s_strMouseDoubleClickEvent, mouseEvent, x, y, url, target, innerNode ),
    m_clickCount( clickCount )
  {}

  static bool test( const TQEvent *event )
  { return KParts::Event::test( event, s_strMouseDoubleClickEvent ); }

  int clickCount() const { return m_clickCount; }

private:
  int m_clickCount;
  static const char *s_strMouseDoubleClickEvent;
};

class MouseMoveEvent : public MouseEvent
{
public:
  MouseMoveEvent( TQMouseEvent *mouseEvent, int x, int y,
                  const DOM::DOMString &url, const DOM::DOMString& target,
		   const DOM::Node &innerNode)
  : MouseEvent( s_strMouseMoveEvent, mouseEvent, x, y, url, target, innerNode )
  {}

  static bool test( const TQEvent *event ) { return KParts::Event::test( event, s_strMouseMoveEvent ); }

private:
  static const char *s_strMouseMoveEvent;
};

class MouseReleaseEvent : public MouseEvent
{
public:
  MouseReleaseEvent( TQMouseEvent *mouseEvent, int x, int y,
                     const DOM::DOMString &url, const DOM::DOMString& target,
		     const DOM::Node &innerNode, long = 0 )
  : MouseEvent( s_strMouseReleaseEvent, mouseEvent, x, y, url, target, innerNode )
  {}

  static bool test( const TQEvent *event ) { return KParts::Event::test( event, s_strMouseReleaseEvent ); }

private:
  static const char *s_strMouseReleaseEvent;
};

class DrawContentsEvent : public KParts::Event
{
public:
  DrawContentsEvent( TQPainter *painter, int clipx, int clipy, int clipw, int cliph );
  virtual ~DrawContentsEvent();

  TQPainter *painter() const { return m_painter; }
  int clipx() const { return m_clipx; }
  int clipy() const { return m_clipy; }
  int clipw() const { return m_clipw; }
  int cliph() const { return m_cliph; }

  static bool test( const TQEvent *event ) { return KParts::Event::test( event, s_strDrawContentsEvent ); }

private:
  TQPainter *m_painter;
  int m_clipx;
  int m_clipy;
  int m_clipw;
  int m_cliph;
  class DrawContentsEventPrivate;
  DrawContentsEventPrivate *d;
  static const char *s_strDrawContentsEvent;
};

}

#endif
