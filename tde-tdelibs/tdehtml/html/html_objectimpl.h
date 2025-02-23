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
#ifndef HTML_OBJECTIMPL_H
#define HTML_OBJECTIMPL_H

#include "html_elementimpl.h"
#include "xml/dom_stringimpl.h"
#include <tqobject.h>
#include <tqstringlist.h>

class TDEHTMLView;

// -------------------------------------------------------------------------
namespace DOM {

class HTMLFormElementImpl;
class DOMStringImpl;

class HTMLObjectBaseElementImpl : public TQObject, public HTMLElementImpl
{
    TQ_OBJECT
public:
    HTMLObjectBaseElementImpl(DocumentImpl *doc);

    virtual void parseAttribute(AttributeImpl *attr);
    virtual void attach();

    virtual void recalcStyle( StyleChange ch );

    void renderAlternative();

    void setServiceType(const TQString &);

    TQString url;
    TQString classId;
    TQString serviceType;
    bool needWidgetUpdate;
    bool m_renderAlternative;

    virtual void insertedIntoDocument();
    virtual void removedFromDocument();
    virtual void addId(const TQString& id);
    virtual void removeId(const TQString& id);
protected slots:
    void slotRenderAlternative();
protected:
    DOMString     m_name;
};

// -------------------------------------------------------------------------

class HTMLAppletElementImpl : public HTMLObjectBaseElementImpl
{
public:
    HTMLAppletElementImpl(DocumentImpl *doc);

    ~HTMLAppletElementImpl();

    virtual Id id() const;

    virtual void parseAttribute(AttributeImpl *token);
    virtual void attach();
protected:
    tdehtml::VAlign valign;
};

// -------------------------------------------------------------------------

class HTMLEmbedElementImpl : public HTMLObjectBaseElementImpl
{
public:
    HTMLEmbedElementImpl(DocumentImpl *doc);
    ~HTMLEmbedElementImpl();

    virtual Id id() const;

    virtual void parseAttribute(AttributeImpl *attr);
    virtual void attach();

    TQString pluginPage;
    bool hidden;
};

// -------------------------------------------------------------------------

class HTMLObjectElementImpl : public HTMLObjectBaseElementImpl
{
public:
    HTMLObjectElementImpl(DocumentImpl *doc);

    ~HTMLObjectElementImpl();

    virtual Id id() const;

    HTMLFormElementImpl *form() const;

    virtual void parseAttribute(AttributeImpl *token);

    virtual void attach();

    DocumentImpl* contentDocument() const;
};

// -------------------------------------------------------------------------

class HTMLParamElementImpl : public HTMLElementImpl
{
    friend class HTMLAppletElementImpl;
public:
    HTMLParamElementImpl(DocumentImpl* _doc) : HTMLElementImpl(_doc) {}

    virtual Id id() const;

    virtual void parseAttribute(AttributeImpl *token);

    TQString name() const { return m_name; }
    TQString value() const { return m_value; }

 protected:
    TQString m_name;
    TQString m_value;
};

} // namespace
#endif
