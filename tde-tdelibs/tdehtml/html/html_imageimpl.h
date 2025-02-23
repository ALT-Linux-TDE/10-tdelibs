/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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
#ifndef HTML_IMAGEIMPL_H
#define HTML_IMAGEIMPL_H

#include "html/html_inlineimpl.h"
#include "misc/tdehtmllayout.h"
#include "misc/loader_client.h"
#include "rendering/render_object.h"

#include <tqregion.h>

namespace DOM {

class DOMString;
class HTMLFormElementImpl;

class HTMLImageElementImpl
    : public HTMLElementImpl, public tdehtml::CachedObjectClient
{
    friend class HTMLFormElementImpl;
public:
    HTMLImageElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f = 0);
    ~HTMLImageElementImpl();

    virtual Id id() const;

    virtual void parseAttribute(AttributeImpl *);

    virtual void attach();
    virtual void removedFromDocument();
    virtual void insertedIntoDocument();
    virtual void addId(const TQString& id);
    virtual void removeId(const TQString& id);


    long width() const;
    long height() const;

    bool isServerMap() const { return ( ismap && !usemap.length() );  }
    /** Return the image for this element.
     *  This has to convert the pixmap into an image first.
     *  This will return undefined results if complete() is not true.
     */
    TQImage currentImage() const;
    /** Return the pixmap for this element.
     *  This will return undefined results if complete() is not true.
     */
    TQPixmap currentPixmap() const;

    DOMString altText() const;

    DOMString imageMap() const { return usemap; }
    /** See if the image has been completely downloaded. 
     * @return True if and only if the image is completely downloaded yet*/
    bool complete() const;

    virtual void notifyFinished(tdehtml::CachedObject *finishedObj);
    void dispatchLoadEvent();
protected:
    DOMString usemap;
    bool ismap : 1;
    bool loadEventSent : 1;
    tdehtml::CachedImage  *m_image;
    HTMLFormElementImpl *m_form;
    DOMString            m_name;
};


//------------------------------------------------------------------

class HTMLAreaElementImpl : public HTMLAnchorElementImpl
{
public:

    enum Shape { Default, Poly, Rect, Circle, Unknown };

    HTMLAreaElementImpl(DocumentImpl *doc);
    ~HTMLAreaElementImpl();

    virtual Id id() const;

    virtual void parseAttribute(AttributeImpl *attr);

    bool isDefault() const { return shape==Default; }

    bool mapMouseEvent(int x_, int y_, int width_, int height_,
                       tdehtml::RenderObject::NodeInfo& info);

    virtual TQRect getRect() const;

    TQRegion cachedRegion() const { return region; }

protected:
    TQRegion getRegion(int width_, int height) const;
    TQRegion region;
    tdehtml::Length* m_coords;
    int m_coordsLen;
    int lastw, lasth;
    Shape shape  : 3;
    bool nohref  : 1;
};


// -------------------------------------------------------------------------

class HTMLMapElementImpl : public HTMLElementImpl
{
public:
    HTMLMapElementImpl(DocumentImpl *doc);

    ~HTMLMapElementImpl();

    virtual Id id() const;

    virtual DOMString getName() const { return name; }

    virtual void parseAttribute(AttributeImpl *attr);

    bool mapMouseEvent(int x_, int y_, int width_, int height_,
                       tdehtml::RenderObject::NodeInfo& info);
private:

    TQString name;
};


} //namespace

#endif
