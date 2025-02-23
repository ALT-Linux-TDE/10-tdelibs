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

#include "tdehtml_events.h"
#include "rendering/render_object.h"
#include "xml/dom_nodeimpl.h"

using namespace tdehtml;

class tdehtml::MouseEvent::MouseEventPrivate
{
};

tdehtml::MouseEvent::MouseEvent( const char *name, TQMouseEvent *qmouseEvent, int x, int y,
                               const DOM::DOMString &url, const DOM::DOMString& target,
	                const DOM::Node &innerNode )
: KParts::Event( name ), m_qmouseEvent( qmouseEvent ), m_x( x ), m_y( y ),
  m_url( url ), m_target(target), m_innerNode( innerNode )
{
  d = 0;
  if (innerNode.handle() && innerNode.handle()->renderer())
      innerNode.handle()->renderer()->absolutePosition(m_nodeAbsX, m_nodeAbsY);
}

tdehtml::MouseEvent::~MouseEvent()
{
  delete d;
}

long tdehtml::MouseEvent::offset() const
{
    int offset = 0;
    DOM::NodeImpl* tempNode = 0;
    int absX, absY;
    absX = absY = 0;
    if (innerNode().handle()->renderer()) {
        innerNode().handle()->renderer()->absolutePosition(absX, absY);
        tdehtml::RenderObject::SelPointState state;
        innerNode().handle()->renderer()->checkSelectionPoint( x(), y(), absX, absY, tempNode, offset, state );
    }
    return offset;
}

const char *tdehtml::MousePressEvent::s_strMousePressEvent = "tdehtml/Events/MousePressEvent";

const char *tdehtml::MouseDoubleClickEvent::s_strMouseDoubleClickEvent = "tdehtml/Events/MouseDoubleClickEvent";

const char *tdehtml::MouseMoveEvent::s_strMouseMoveEvent = "tdehtml/Events/MouseMoveEvent";

const char *tdehtml::MouseReleaseEvent::s_strMouseReleaseEvent = "tdehtml/Events/MouseReleaseEvent";

const char *tdehtml::DrawContentsEvent::s_strDrawContentsEvent = "tdehtml/Events/DrawContentsEvent";

class tdehtml::DrawContentsEvent::DrawContentsEventPrivate
{
public:
  DrawContentsEventPrivate()
  {
  }
  ~DrawContentsEventPrivate()
  {
  }
};

tdehtml::DrawContentsEvent::DrawContentsEvent( TQPainter *painter, int clipx, int clipy, int clipw, int cliph )
  : KParts::Event( s_strDrawContentsEvent ), m_painter( painter ), m_clipx( clipx ), m_clipy( clipy ),
    m_clipw( clipw ), m_cliph( cliph )
{
  d = new DrawContentsEventPrivate;
}

tdehtml::DrawContentsEvent::~DrawContentsEvent()
{
  delete d;
}

