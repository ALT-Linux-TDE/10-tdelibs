/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 2000 Peter Kelly (pmk@post.com)
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

#ifndef _DOM_XmlImpl_h_
#define _DOM_XmlImpl_h_

#include "xml/dom_nodeimpl.h"
#include "misc/loader_client.h"

#include <tqxml.h>

namespace tdehtml {
class CachedCSSStyleSheet;
}

namespace DOM {

class DocumentImpl;
class CSSStyleSheetImpl;
class StyleSheetImpl;
class DOMString;

class EntityImpl : public NodeBaseImpl
{
public:
    EntityImpl(DocumentImpl *doc);
    EntityImpl(DocumentImpl *doc, DOMString _name);
    EntityImpl(DocumentImpl *doc, DOMString _publicId, DOMString _systemId, DOMString _notationName);
    virtual ~EntityImpl();

    // DOM methods & attributes for Entity

    virtual DOMString publicId() const;
    virtual DOMString systemId() const;
    virtual DOMString notationName() const;

    // DOM methods overridden from  parent classes

    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual NodeImpl *cloneNode ( bool deep );

    // Other methods (not part of DOM)

    virtual bool childTypeAllowed( unsigned short type );

    virtual DOMString toString() const;

protected:
    DOMStringImpl *m_publicId;
    DOMStringImpl *m_systemId;
    DOMStringImpl *m_notationName;
    DOMStringImpl *m_name;
};


class EntityReferenceImpl : public NodeBaseImpl
{
public:
    EntityReferenceImpl(DocumentImpl *doc);
    EntityReferenceImpl(DocumentImpl *doc, DOMStringImpl *_entityName);
    virtual ~EntityReferenceImpl();

    // DOM methods overridden from  parent classes

    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual NodeImpl *cloneNode ( bool deep );

    // Other methods (not part of DOM)

    virtual bool childTypeAllowed( unsigned short type );

    virtual DOMString toString() const;
protected:
    DOMStringImpl *m_entityName;
};

class NotationImpl : public NodeBaseImpl
{
public:
    NotationImpl(DocumentImpl *doc);
    NotationImpl(DocumentImpl *doc, DOMString _name, DOMString _publicId, DOMString _systemId);
    virtual ~NotationImpl();

    // DOM methods & attributes for Notation

    virtual DOMString publicId() const;
    virtual DOMString systemId() const;

    // DOM methods overridden from  parent classes

    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual NodeImpl *cloneNode ( bool deep );

    // Other methods (not part of DOM)

    virtual bool childTypeAllowed( unsigned short type );
protected:
    DOMStringImpl *m_name;
    DOMStringImpl *m_publicId;
    DOMStringImpl *m_systemId;
};


class ProcessingInstructionImpl : public NodeBaseImpl, private tdehtml::CachedObjectClient
{
public:
    ProcessingInstructionImpl(DocumentImpl *doc);
    ProcessingInstructionImpl(DocumentImpl *doc, DOMString _target, DOMString _data);
    virtual ~ProcessingInstructionImpl();

    // DOM methods & attributes for Notation

    virtual DOMString target() const;
    DOMString data() const { return m_data; }
    virtual void setData( const DOMString &_data, int &exceptioncode );

    // DOM methods overridden from  parent classes

    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual DOMString nodeValue() const;
    virtual void setNodeValue( const DOMString &_nodeValue, int &exceptioncode );
    virtual NodeImpl *cloneNode ( bool deep );

    // Other methods (not part of DOM)

    virtual DOMString localHref() const;
    virtual bool childTypeAllowed( unsigned short type );
    StyleSheetImpl *sheet() const;
    void checkStyleSheet();
    virtual void setStyleSheet(const DOM::DOMString &url, const DOM::DOMString &sheet, const DOM::DOMString &charset);
    virtual void setStyleSheet(CSSStyleSheetImpl* sheet);

    virtual DOMString toString() const;

protected:
    DOMStringImpl *m_target;
    DOMStringImpl *m_data;
    DOMStringImpl *m_localHref;
    tdehtml::CachedCSSStyleSheet *m_cachedSheet;
    CSSStyleSheetImpl *m_sheet;
};

class XMLAttributeReader : public TQXmlDefaultHandler
{
public:
    XMLAttributeReader(const TQString& _attrString);
    virtual ~XMLAttributeReader();
    TQXmlAttributes readAttrs(bool &ok);
    bool startElement(const TQString& namespaceURI, const TQString& localName, const TQString& qName, const TQXmlAttributes& atts);

protected:
    TQXmlAttributes attrs;
    TQString m_attrString;
};

} //namespace

#endif
