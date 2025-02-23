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

#ifndef _XML_Tokenizer_h_
#define _XML_Tokenizer_h_

#include <tqxml.h>
#include <tqptrlist.h>
#include <tqptrstack.h>
#include <tqvaluestack.h>
#include <tqobject.h>
#include "misc/loader_client.h"
#include "misc/stringit.h"

class TDEHTMLView;

namespace tdehtml {
    class CachedObject;
    class CachedScript;
}

namespace DOM {
    class DocumentImpl;
    class NodeImpl;
    class HTMLScriptElementImpl;
    class DocumentImpl;
    class HTMLScriptElementImpl;
}

namespace tdehtml {

class XMLHandler : public TQXmlDefaultHandler
{
public:
    XMLHandler(DOM::DocumentImpl *_doc, TDEHTMLView *_view);
    virtual ~XMLHandler();

    // return the error protocol if parsing failed
    TQString errorProtocol();

    // overloaded handler functions
    bool startDocument();
    bool startElement(const TQString& namespaceURI, const TQString& localName, const TQString& qName, const TQXmlAttributes& atts);
    bool endElement(const TQString& namespaceURI, const TQString& localName, const TQString& qName);
    bool startCDATA();
    bool endCDATA();
    bool characters(const TQString& ch);
    bool comment(const TQString & ch);
    bool processingInstruction(const TQString &target, const TQString &data);
    
    // namespace handling, to workaround problem in QXML where some attributes
    // do not get the namespace resolved properly
    bool startPrefixMapping(const TQString& prefix, const TQString& uri);
    bool endPrefixMapping(const TQString& prefix);
    void fixUpNSURI(TQString& uri, const TQString& qname);
    TQMap<TQString, TQValueStack<TQString> > namespaceInfo;


    // from QXmlDeclHandler
    bool attributeDecl(const TQString &eName, const TQString &aName, const TQString &type, const TQString &valueDefault, const TQString &value);
    bool externalEntityDecl(const TQString &name, const TQString &publicId, const TQString &systemId);
    bool internalEntityDecl(const TQString &name, const TQString &value);

    // from QXmlDTDHandler
    bool notationDecl(const TQString &name, const TQString &publicId, const TQString &systemId);
    bool unparsedEntityDecl(const TQString &name, const TQString &publicId, const TQString &systemId, const TQString &notationName);

    bool enterText();
    void exitText();

    TQString errorString();

    bool fatalError( const TQXmlParseException& exception );

    unsigned long errorLine;
    unsigned long errorCol;

private:
    void pushNode( DOM::NodeImpl *node );
    DOM::NodeImpl *popNode();
    DOM::NodeImpl *currentNode() const;
private:
    TQString errorProt;
    DOM::DocumentImpl *m_doc;
    TDEHTMLView *m_view;
    TQPtrStack<DOM::NodeImpl> m_nodes;
    DOM::NodeImpl *m_rootNode;

    enum State {
        StateInit,
        StateDocument,
        StateQuote,
        StateLine,
        StateHeading,
        StateP
    };
    State state;
};

class Tokenizer : public TQObject
{
    TQ_OBJECT
public:
    virtual void begin() = 0;
    // script output must be prepended, while new data
    // received during executing a script must be appended, hence the
    // extra bool to be able to distinguish between both cases. document.write()
    // always uses false, while tdehtmlpart uses true
    virtual void write( const TokenizerString &str, bool appendData) = 0;
    virtual void end() = 0;
    virtual void finish() = 0;
    virtual void setOnHold(bool /*_onHold*/) {}
    virtual bool isWaitingForScripts() const = 0;
    virtual bool isExecutingScript() const = 0;
    virtual void abort() {}
    virtual void setAutoClose(bool b=true) = 0;

signals:
    void finishedParsing();

};

class XMLIncrementalSource : public TQXmlInputSource
{
public:
    XMLIncrementalSource();
    virtual void fetchData();
    virtual TQChar next();
    virtual void setData( const TQString& str );
    virtual void setData( const TQByteArray& data );
    virtual TQString data();

    void appendXML( const TQString& str );
    void setFinished( bool );

private:
    TQString      m_data;
    uint         m_pos;
    const TQChar *m_unicode;
    bool         m_finished;
};

class XMLTokenizer : public Tokenizer, public tdehtml::CachedObjectClient
{
public:
    XMLTokenizer(DOM::DocumentImpl *, TDEHTMLView * = 0);
    virtual ~XMLTokenizer();
    virtual void begin();
    virtual void write( const TokenizerString &str, bool );
    virtual void end();
    virtual void finish();
    virtual void setAutoClose(bool b=true) { tqWarning("XMLTokenizer::setAutoClose: stub."); (void)b; }

    // from CachedObjectClient
    void notifyFinished(tdehtml::CachedObject *finishedObj);

    virtual bool isWaitingForScripts() const;
    virtual bool isExecutingScript() const { return false; }

protected:
    DOM::DocumentImpl *m_doc;
    TDEHTMLView *m_view;

    void executeScripts();
    void addScripts(DOM::NodeImpl *n);

    TQPtrList<DOM::HTMLScriptElementImpl> m_scripts;
    TQPtrListIterator<DOM::HTMLScriptElementImpl> *m_scriptsIt;
    tdehtml::CachedScript *m_cachedScript;

    XMLHandler m_handler;
    TQXmlSimpleReader m_reader;
    XMLIncrementalSource m_source;
    bool m_noErrors;
};

} // end namespace

#endif
