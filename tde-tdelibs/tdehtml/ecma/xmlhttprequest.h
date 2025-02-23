/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Apple Computer, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _XMLHTTPREQUEST_H_
#define _XMLHTTPREQUEST_H_

#include "ecma/kjs_binding.h"
#include "ecma/kjs_dom.h"
#include "misc/decoder.h"
#include "tdeio/jobclasses.h"

namespace KJS {

  class JSEventListener;
  class XMLHttpRequestQObject;

  // these exact numeric values are important because JS expects them
  enum XMLHttpRequestState {
    Uninitialized = 0,
    Loading = 1,
    Loaded = 2,
    Interactive = 3,
    Completed = 4
  };

  class XMLHttpRequestConstructorImp : public ObjectImp {
  public:
    XMLHttpRequestConstructorImp(ExecState *exec, const DOM::Document &d);
    virtual bool implementsConstruct() const;
    virtual Object construct(ExecState *exec, const List &args);
  private:
    DOM::Document doc;
  };

  class XMLHttpRequest : public DOMObject {
  public:
    XMLHttpRequest(ExecState *, const DOM::Document &d);
    ~XMLHttpRequest();
    virtual Value tryGet(ExecState *exec, const Identifier &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    virtual void tryPut(ExecState *exec, const Identifier &propertyName, const Value& value, int attr = None);
    void putValueProperty(ExecState *exec, int token, const Value& value, int /*attr*/);
    virtual bool toBoolean(ExecState *) const { return true; }
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Onload, Onreadystatechange, ReadyState, ResponseText, ResponseXML, Status, StatusText, Abort,
           GetAllResponseHeaders, GetResponseHeader, Open, Send, SetRequestHeader,
           OverrideMIMEType };

  private:
    friend class XMLHttpRequestProtoFunc;
    friend class XMLHttpRequestQObject;

    Value getStatusText() const;
    Value getStatus() const;
    bool urlMatchesDocumentDomain(const KURL&) const;

    XMLHttpRequestQObject *qObject;

#ifdef APPLE_CHANGES
    void slotData( TDEIO::Job* job, const char *data, int size );
#else
    void slotData( TDEIO::Job* job, const TQByteArray &data );
#endif
    void slotFinished( TDEIO::Job* );
    void slotRedirection( TDEIO::Job*, const KURL& );

    void processSyncLoadResults(const TQByteArray &data, const KURL &finalURL, const TQString &headers);

    void open(const TQString& _method, const KURL& _url, bool _async);
    void send(const TQString& _body);
    void abort();
    void setRequestHeader(const TQString& name, const TQString &value);
    void overrideMIMEType(const TQString& override);
    Value getAllResponseHeaders() const;
    Value getResponseHeader(const TQString& name) const;

    void changeState(XMLHttpRequestState newState);

    TQGuardedPtr<DOM::DocumentImpl> doc;

    KURL url;
    TQString method;
    bool async;
    TQMap<TQString,TQString> requestHeaders;
    TQString m_mimeTypeOverride;
    TQString contentType;

    TDEIO::TransferJob * job;

    XMLHttpRequestState state;
    JSEventListener *onReadyStateChangeListener;
    JSEventListener *onLoadListener;

    tdehtml::Decoder *decoder;
    TQString encoding;
    TQString responseHeaders;

    TQString response;
    mutable bool createdDocument;
    mutable bool typeIsXML;
    mutable DOM::Document responseXML;

    bool aborted;
  };


  class XMLHttpRequestQObject : public TQObject {
    TQ_OBJECT

  public:
    XMLHttpRequestQObject(XMLHttpRequest *_jsObject);

  public slots:
    void slotData( TDEIO::Job* job, const TQByteArray &data );
    void slotFinished( TDEIO::Job* job );
    void slotRedirection( TDEIO::Job* job, const KURL& url);

  private:
    XMLHttpRequest *jsObject;
  };

} // namespace

#endif
