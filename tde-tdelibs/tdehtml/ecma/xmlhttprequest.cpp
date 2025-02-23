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

#include "xmlhttprequest.h"
#include "xmlhttprequest.lut.h"
#include "kjs_window.h"
#include "kjs_events.h"

#include "dom/dom_doc.h"
#include "dom/dom_exception.h"
#include "dom/dom_string.h"
#include "misc/loader.h"
#include "html/html_documentimpl.h"
#include "xml/dom2_eventsimpl.h"

#include "tdehtml_part.h"
#include "tdehtmlview.h"

#include <tdeio/scheduler.h>
#include <tdeio/job.h>
#include <tqobject.h>
#include <kdebug.h>

#ifdef APPLE_CHANGES
#include "KWQLoader.h"
#else
#include <tdeio/netaccess.h>
using TDEIO::NetAccess;
#endif

#define BANNED_HTTP_HEADERS "authorization,proxy-authorization,"\
                            "content-length,host,connect,copy,move,"\
                            "delete,head,trace,put,propfind,proppatch,"\
                            "mkcol,lock,unlock,options,via,"\
                            "accept-charset,accept-encoding,expect,date,"\
                            "keep-alive,te,trailer,"\
                            "transfer-encoding,upgrade"

using tdehtml::Decoder;

namespace KJS {

////////////////////// XMLHttpRequest Object ////////////////////////

/* Source for XMLHttpRequestProtoTable.
@begin XMLHttpRequestProtoTable 7
  abort			XMLHttpRequest::Abort			DontDelete|Function 0
  getAllResponseHeaders	XMLHttpRequest::GetAllResponseHeaders	DontDelete|Function 0
  getResponseHeader	XMLHttpRequest::GetResponseHeader	DontDelete|Function 1
  open			XMLHttpRequest::Open			DontDelete|Function 5
  overrideMimeType	XMLHttpRequest::OverrideMIMEType	DontDelete|Function 1
  send			XMLHttpRequest::Send			DontDelete|Function 1
  setRequestHeader	XMLHttpRequest::SetRequestHeader	DontDelete|Function 2
@end
*/
KJS_DEFINE_PROTOTYPE(XMLHttpRequestProto)
IMPLEMENT_PROTOFUNC_DOM(XMLHttpRequestProtoFunc)
KJS_IMPLEMENT_PROTOTYPE("XMLHttpRequest", XMLHttpRequestProto,XMLHttpRequestProtoFunc)


XMLHttpRequestQObject::XMLHttpRequestQObject(XMLHttpRequest *_jsObject)
{
  jsObject = _jsObject;
}

#ifdef APPLE_CHANGES
void XMLHttpRequestQObject::slotData( TDEIO::Job* job, const char *data, int size )
{
  jsObject->slotData(job, data, size);
}
#else
void XMLHttpRequestQObject::slotData( TDEIO::Job* job, const TQByteArray &data )
{
  jsObject->slotData(job, data);
}
#endif

void XMLHttpRequestQObject::slotFinished( TDEIO::Job* job )
{
  jsObject->slotFinished(job);
}

void XMLHttpRequestQObject::slotRedirection( TDEIO::Job* job, const KURL& url)
{
  jsObject->slotRedirection( job, url );
}

XMLHttpRequestConstructorImp::XMLHttpRequestConstructorImp(ExecState *, const DOM::Document &d)
    : ObjectImp(), doc(d)
{
}

bool XMLHttpRequestConstructorImp::implementsConstruct() const
{
  return true;
}

Object XMLHttpRequestConstructorImp::construct(ExecState *exec, const List &)
{
  return Object(new XMLHttpRequest(exec, doc));
}

const ClassInfo XMLHttpRequest::info = { "XMLHttpRequest", 0, &XMLHttpRequestTable, 0 };


/* Source for XMLHttpRequestTable.
@begin XMLHttpRequestTable 7
  readyState		XMLHttpRequest::ReadyState		DontDelete|ReadOnly
  responseText		XMLHttpRequest::ResponseText		DontDelete|ReadOnly
  responseXML		XMLHttpRequest::ResponseXML		DontDelete|ReadOnly
  status		XMLHttpRequest::Status			DontDelete|ReadOnly
  statusText		XMLHttpRequest::StatusText		DontDelete|ReadOnly
  onreadystatechange	XMLHttpRequest::Onreadystatechange	DontDelete
  onload		XMLHttpRequest::Onload			DontDelete
@end
*/

Value XMLHttpRequest::tryGet(ExecState *exec, const Identifier &propertyName) const
{
  return DOMObjectLookupGetValue<XMLHttpRequest,DOMObject>(exec, propertyName, &XMLHttpRequestTable, this);
}

Value XMLHttpRequest::getValueProperty(ExecState *exec, int token) const
{
  switch (token) {
  case ReadyState:
    return Number(state);
  case ResponseText:
    return getString(DOM::DOMString(response));
  case ResponseXML:
    if (state != Completed) {
      return Null();
    }
    if (!createdDocument) {
      TQString mimeType = "text/xml";

      if (!m_mimeTypeOverride.isEmpty()) {
        mimeType = m_mimeTypeOverride;
      } else {
	  Value header = getResponseHeader("Content-Type");
          if (header.type() != UndefinedType) {
            mimeType = TQStringList::split(";", header.toString(exec).qstring())[0].stripWhiteSpace();
	  }
      }

      if (mimeType == "text/xml" || mimeType == "application/xml" || mimeType == "application/xhtml+xml") {
	responseXML = DOM::Document(doc->implementation()->createDocument());

	DOM::DocumentImpl *docImpl = static_cast<DOM::DocumentImpl *>(responseXML.handle());

	docImpl->open();
	docImpl->write(response);
	docImpl->finishParsing();
	docImpl->close();

	typeIsXML = true;
      } else {
	typeIsXML = false;
      }
      createdDocument = true;
    }

    if (!typeIsXML) {
      return Undefined();
    }

    return getDOMNode(exec,responseXML);
  case Status:
    return getStatus();
  case StatusText:
    return getStatusText();
  case Onreadystatechange:
   if (onReadyStateChangeListener && onReadyStateChangeListener->listenerObjImp()) {
     return onReadyStateChangeListener->listenerObj();
   } else {
     return Null();
   }
  case Onload:
   if (onLoadListener && onLoadListener->listenerObjImp()) {
     return onLoadListener->listenerObj();
   } else {
    return Null();
   }
  default:
    kdWarning() << "XMLHttpRequest::getValueProperty unhandled token " << token << endl;
    return Value();
  }
}

void XMLHttpRequest::tryPut(ExecState *exec, const Identifier &propertyName, const Value& value, int attr)
{
  DOMObjectLookupPut<XMLHttpRequest,DOMObject>(exec, propertyName, value, attr, &XMLHttpRequestTable, this );
}

void XMLHttpRequest::putValueProperty(ExecState *exec, int token, const Value& value, int /*attr*/)
{
  JSEventListener* newListener;
  switch(token) {
  case Onreadystatechange:
    newListener = Window::retrieveActive(exec)->getJSEventListener(value, true);
    if (newListener != onReadyStateChangeListener) {
      if (onReadyStateChangeListener) onReadyStateChangeListener->deref();
      onReadyStateChangeListener = newListener;
      if (onReadyStateChangeListener) onReadyStateChangeListener->ref();
    }
    break;
  case Onload:
    newListener = Window::retrieveActive(exec)->getJSEventListener(value, true);
    if (newListener != onLoadListener) {
      if (onLoadListener) onLoadListener->deref();
      onLoadListener = newListener;
      if (onLoadListener) onLoadListener->ref();
    }
    break;
  default:
    kdWarning() << "XMLHttpRequest::putValue unhandled token " << token << endl;
  }
}

XMLHttpRequest::XMLHttpRequest(ExecState *exec, const DOM::Document &d)
  : DOMObject(XMLHttpRequestProto::self(exec)),
    qObject(new XMLHttpRequestQObject(this)),
    doc(static_cast<DOM::DocumentImpl*>(d.handle())),
    async(true),
    contentType(TQString::null),
    job(0),
    state(Uninitialized),
    onReadyStateChangeListener(0),
    onLoadListener(0),
    decoder(0),
    createdDocument(false),
    aborted(false)
{
}

XMLHttpRequest::~XMLHttpRequest()
{
  if (onReadyStateChangeListener)
    onReadyStateChangeListener->deref();
  if (onLoadListener)
    onLoadListener->deref();
  delete qObject;
  qObject = 0;
  delete decoder;
  decoder = 0;
}

void XMLHttpRequest::changeState(XMLHttpRequestState newState)
{
  if (state != newState) {
    state = newState;

    ref();

    if (onReadyStateChangeListener != 0 && doc->view() && doc->view()->part()) {
      DOM::Event ev = doc->view()->part()->document().createEvent("HTMLEvents");
      ev.initEvent("readystatechange", true, true);
      onReadyStateChangeListener->handleEvent(ev);
    }

    if (state == Completed && onLoadListener != 0 && doc->view() && doc->view()->part()) {
      DOM::Event ev = doc->view()->part()->document().createEvent("HTMLEvents");
      ev.initEvent("load", true, true);
      onLoadListener->handleEvent(ev);
    }

    deref();
  }
}

bool XMLHttpRequest::urlMatchesDocumentDomain(const KURL& _url) const
{
  // No need to do work if _url is not valid...
  if (!_url.isValid())
    return false;

  KURL documentURL(doc->URL());

  // a local file can load anything
  if (documentURL.protocol().lower() == "file") {
    return true;
  }

  // but a remote document can only load from the same port on the server
  if (documentURL.protocol().lower() == _url.protocol().lower() &&
      documentURL.host().lower() == _url.host().lower() &&
      documentURL.port() == _url.port()) {
    return true;
  }

  return false;
}

void XMLHttpRequest::open(const TQString& _method, const KURL& _url, bool _async)
{
  abort();
  aborted = false;

  // clear stuff from possible previous load
  requestHeaders.clear();
  responseHeaders = TQString();
  response = TQString();
  createdDocument = false;
  responseXML = DOM::Document();

  changeState(Uninitialized);

  if (aborted) {
    return;
  }

  if (!urlMatchesDocumentDomain(_url)) {
    return;
  }


  method = _method.lower();
  url = _url;
  async = _async;

  changeState(Loading);
}

void XMLHttpRequest::send(const TQString& _body)
{
  aborted = false;

  const TQString protocol = url.protocol().lower();
  // Abandon the request when the protocol is other than "http",
  // instead of blindly doing a TDEIO::get on other protocols like file:/.
  if (!protocol.startsWith("http") && !protocol.startsWith("webdav"))
  {
    abort();
    return;
  }

  if (method == "post") {

    // FIXME: determine post encoding correctly by looking in headers
    // for charset.
    TQByteArray buf;
    TQCString str = _body.utf8();
    buf.duplicate(str.data(), str.size() - 1);

    job = TDEIO::http_post( url, buf, false );
    if(contentType.isNull())
      job->addMetaData( "content-type", "Content-type: text/plain" );
    else
      job->addMetaData( "content-type", contentType );
  }
  else {
    job = TDEIO::get( url, false, false );
  }

  if (!requestHeaders.isEmpty()) {
    TQString rh;
    TQMap<TQString, TQString>::ConstIterator begin = requestHeaders.begin();
    TQMap<TQString, TQString>::ConstIterator end = requestHeaders.end();
    for (TQMap<TQString, TQString>::ConstIterator i = begin; i != end; ++i) {
      TQString key = i.key();
      TQString value = i.data();
      if (key == "accept") {
        // The HTTP TDEIO slave supports an override this way
        job->addMetaData("accept", value);
      } else {
        if (i != begin)
          rh += "\r\n";
        rh += key + ": " + value;
      }
    }

    job->addMetaData("customHTTPHeader", rh);
  }

  job->addMetaData("PropagateHttpHeader", "true");

  // Set the default referrer if one is not already supplied
  // through setRequestHeader. NOTE: the user can still disable
  // this feature at the protocol level (tdeio_http).
  // ### does find() ever succeed? the headers are stored in lower case!
  if (requestHeaders.find("Referer") == requestHeaders.end()) {
    KURL documentURL(doc->URL());
    documentURL.setPass(TQString::null);
    documentURL.setUser(TQString::null);
    job->addMetaData("referrer", documentURL.url());
    // kdDebug() << "Adding referrer: " << documentURL << endl;
  }

  if (!async) {
    TQByteArray data;
    KURL finalURL;
    TQString headers;

#ifdef APPLE_CHANGES
    data = KWQServeSynchronousRequest(tdehtml::Cache::loader(), doc->docLoader(), job, finalURL, headers);
#else
    TQMap<TQString, TQString> metaData;
    if ( NetAccess::synchronousRun( job, 0, &data, &finalURL, &metaData ) ) {
      headers = metaData[ "HTTP-Headers" ];
    }
#endif
    job = 0;
    processSyncLoadResults(data, finalURL, headers);
    return;
  }

  qObject->connect( job, TQ_SIGNAL( result( TDEIO::Job* ) ),
		    TQ_SLOT( slotFinished( TDEIO::Job* ) ) );
#ifdef APPLE_CHANGES
  qObject->connect( job, TQ_SIGNAL( data( TDEIO::Job*, const char*, int ) ),
		    TQ_SLOT( slotData( TDEIO::Job*, const char*, int ) ) );
#else
  qObject->connect( job, TQ_SIGNAL( data( TDEIO::Job*, const TQByteArray& ) ),
		    TQ_SLOT( slotData( TDEIO::Job*, const TQByteArray& ) ) );
#endif
  qObject->connect( job, TQ_SIGNAL(redirection(TDEIO::Job*, const KURL& ) ),
		    TQ_SLOT( slotRedirection(TDEIO::Job*, const KURL&) ) );

#ifdef APPLE_CHANGES
  KWQServeRequest(tdehtml::Cache::loader(), doc->docLoader(), job);
#else
  TDEIO::Scheduler::scheduleJob( job );
#endif
}

void XMLHttpRequest::abort()
{
  if (job) {
    job->kill();
    job = 0;
  }
  delete decoder;
  decoder = 0;
  aborted = true;
}

void XMLHttpRequest::overrideMIMEType(const TQString& override)
{
    m_mimeTypeOverride = override;
}

void XMLHttpRequest::setRequestHeader(const TQString& _name, const TQString &value)
{
  TQString name = _name.lower().stripWhiteSpace();

  // Content-type needs to be set seperately from the other headers
  if(name == "content-type") {
    contentType = "Content-type: " + value;
    return;
  }

  // Sanitize the referrer header to protect against spoofing...
  if(name == "referer") {
    KURL referrerURL(value);
    if (urlMatchesDocumentDomain(referrerURL))
      requestHeaders[name] = referrerURL.url();
    return;
  }

  // Sanitize the request headers below and handle them as if they are
  // calls to open. Otherwise, we will end up ignoring them all together!
  // TODO: Do something about "put" which tdeio_http sort of supports and
  // the webDAV headers such as PROPFIND etc...
  if (name == "get"  || name == "post") {
    KURL reqURL (doc->URL(), value.stripWhiteSpace());
    open(name, reqURL, async);
    return;
  }

  // Reject all banned headers. See BANNED_HTTP_HEADERS above.
  // kdDebug() << "Banned HTTP Headers: " << BANNED_HTTP_HEADERS << endl;
  TQStringList bannedHeaders = TQStringList::split(',',
                                  TQString::fromLatin1(BANNED_HTTP_HEADERS));

  if (bannedHeaders.contains(name))
    return;   // Denied

  requestHeaders[name] = value.stripWhiteSpace();
}

Value XMLHttpRequest::getAllResponseHeaders() const
{
  if (responseHeaders.isEmpty()) {
    return Undefined();
  }

  int endOfLine = responseHeaders.find("\n");

  if (endOfLine == -1) {
    return Undefined();
  }

  return String(responseHeaders.mid(endOfLine + 1) + "\n");
}

Value XMLHttpRequest::getResponseHeader(const TQString& name) const
{
  if (responseHeaders.isEmpty()) {
    return Undefined();
  }

  TQRegExp headerLinePattern(name + ":", false);

  int matchLength;
  int headerLinePos = headerLinePattern.search(responseHeaders, 0);
  matchLength = headerLinePattern.matchedLength();
  while (headerLinePos != -1) {
    if (headerLinePos == 0 || responseHeaders[headerLinePos-1] == '\n') {
      break;
    }

    headerLinePos = headerLinePattern.search(responseHeaders, headerLinePos + 1);
    matchLength = headerLinePattern.matchedLength();
  }


  if (headerLinePos == -1) {
    return Undefined();
  }

  int endOfLine = responseHeaders.find("\n", headerLinePos + matchLength);

  return String(responseHeaders.mid(headerLinePos + matchLength, endOfLine - (headerLinePos + matchLength)).stripWhiteSpace());
}

static Value httpStatus(const TQString& response, bool textStatus = false)
{
  if (response.isEmpty()) {
    return Undefined();
  }

  int endOfLine = response.find("\n");
  TQString firstLine = (endOfLine == -1) ? response : response.left(endOfLine);
  int codeStart = firstLine.find(" ");
  int codeEnd = firstLine.find(" ", codeStart + 1);

  if (codeStart == -1 || codeEnd == -1) {
    return Undefined();
  }

  if (textStatus) {
    TQString statusText = firstLine.mid(codeEnd + 1, endOfLine - (codeEnd + 1)).stripWhiteSpace();
    return String(statusText);
  }

  TQString number = firstLine.mid(codeStart + 1, codeEnd - (codeStart + 1));

  bool ok = false;
  int code = number.toInt(&ok);
  if (!ok) {
    return Undefined();
  }

  return Number(code);
}

Value XMLHttpRequest::getStatus() const
{
  return httpStatus(responseHeaders);
}

Value XMLHttpRequest::getStatusText() const
{
  return httpStatus(responseHeaders, true);
}

void XMLHttpRequest::processSyncLoadResults(const TQByteArray &data, const KURL &finalURL, const TQString &headers)
{
  if (!urlMatchesDocumentDomain(finalURL)) {
    abort();
    return;
  }

  responseHeaders = headers;
  changeState(Loaded);
  if (aborted) {
    return;
  }

#ifdef APPLE_CHANGES
  const char *bytes = (const char *)data.data();
  int len = (int)data.size();

  slotData(0, bytes, len);
#else
  slotData(0, data);
#endif

  if (aborted) {
    return;
  }

  slotFinished(0);
}

void XMLHttpRequest::slotFinished(TDEIO::Job *)
{
  if (decoder) {
    response += decoder->flush();
  }

  // make sure to forget about the job before emitting completed,
  // since changeState triggers JS code, which might e.g. call abort.
  job = 0;
  changeState(Completed);

  delete decoder;
  decoder = 0;
}

void XMLHttpRequest::slotRedirection(TDEIO::Job*, const KURL& url)
{
  if (!urlMatchesDocumentDomain(url)) {
    abort();
  }
}

#ifdef APPLE_CHANGES
void XMLHttpRequest::slotData( TDEIO::Job*, const char *data, int len )
#else
void XMLHttpRequest::slotData(TDEIO::Job*, const TQByteArray &_data)
#endif
{
  if (state < Loaded ) {
    responseHeaders = job->queryMetaData("HTTP-Headers");

    // NOTE: Replace a 304 response with a 200! Both IE and Mozilla do this.
    // Problem first reported through bug# 110272.
    int codeStart = responseHeaders.find("304");
    if ( codeStart != -1) {
      int codeEnd = responseHeaders.find("\n", codeStart+3);
      if (codeEnd != -1)
        responseHeaders.replace(codeStart, (codeEnd-codeStart), "200 OK");
    }

    changeState(Loaded);
  }

#ifndef APPLE_CHANGES
  const char *data = (const char *)_data.data();
  int len = (int)_data.size();
#endif

  if ( decoder == NULL ) {
    int pos = responseHeaders.find("content-type:", 0, false);

    if ( pos > -1 ) {
      pos += 13;
      int index = responseHeaders.find('\n', pos);
      TQString type = responseHeaders.mid(pos, (index-pos));
      index = type.find (';');
      if (index > -1)
        encoding = TQString(type.mid( index+1 ).remove(TQRegExp("charset[ ]*=[ ]*", false))).stripWhiteSpace();
    }

    decoder = new Decoder;
    if (!encoding.isNull())
      decoder->setEncoding(encoding.latin1(), Decoder::EncodingFromHTTPHeader);
    else {
      // Per section 2 of W3C working draft spec, fall back to "UTF-8".
      decoder->setEncoding("UTF-8", Decoder::DefaultEncoding);
    }
  }
  if (len == 0)
    return;

  if (len == -1)
    len = strlen(data);

  TQString decoded = decoder->decode(data, len);

  response += decoded;

  if (!aborted) {
    changeState(Interactive);
  }
}

Value XMLHttpRequestProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  if (!thisObj.inherits(&XMLHttpRequest::info)) {
    Object err = Error::create(exec,TypeError);
    exec->setException(err);
    return err;
  }

  XMLHttpRequest *request = static_cast<XMLHttpRequest *>(thisObj.imp());
  switch (id) {
  case XMLHttpRequest::Abort:
    request->abort();
    return Undefined();
  case XMLHttpRequest::GetAllResponseHeaders:
    if (args.size() != 0) {
    return Undefined();
    }

    return request->getAllResponseHeaders();
  case XMLHttpRequest::GetResponseHeader:
    if (args.size() != 1) {
    return Undefined();
    }

    return request->getResponseHeader(args[0].toString(exec).qstring());
  case XMLHttpRequest::Open:
    {
      if (args.size() < 2 || args.size() > 5) {
        return Undefined();
      }

      TQString method = args[0].toString(exec).qstring();
      TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(Window::retrieveActive(exec)->part());
      if (!part)
        return Undefined();
      KURL url = KURL(part->document().completeURL(args[1].toString(exec).qstring()).string());

      bool async = true;
      if (args.size() >= 3) {
	async = args[2].toBoolean(exec);
      }

      if (args.size() >= 4) {
	url.setUser(args[3].toString(exec).qstring());
      }

      if (args.size() >= 5) {
	url.setPass(args[4].toString(exec).qstring());
      }

      request->open(method, url, async);

      return Undefined();
    }
  case XMLHttpRequest::Send:
    {
      if (args.size() > 1) {
        return Undefined();
      }

      if (request->state != Loading) {
	return Undefined();
      }

      TQString body;
      if (args.size() >= 1) {
        Object obj = Object::dynamicCast(args[0]);
        if (obj.isValid() && obj.inherits(&DOMDocument::info)) {
          DOM::Node docNode = static_cast<KJS::DOMDocument *>(obj.imp())->toNode();
          DOM::DocumentImpl *doc = static_cast<DOM::DocumentImpl *>(docNode.handle());

          try {
            body = doc->toString().string();
            // FIXME: also need to set content type, including encoding!

          } catch(DOM::DOMException& e) {
            Object err = Error::create(exec, GeneralError, "Exception serializing document");
            exec->setException(err);
          }
        } else {
          body = args[0].toString(exec).qstring();
        }
      }

      request->send(body);

      return Undefined();
    }
  case XMLHttpRequest::SetRequestHeader:
    if (args.size() != 2) {
      return Undefined();
    }

    request->setRequestHeader(args[0].toString(exec).qstring(), args[1].toString(exec).qstring());

    return Undefined();

  case XMLHttpRequest::OverrideMIMEType:
    if (args.size() < 1) {
       Object err = Error::create(exec, SyntaxError, "Not enough arguments");
       exec->setException(err);
       return err;
    }

    request->overrideMIMEType(args[0].toString(exec).qstring());
    return Undefined();
  }

  return Undefined();
}

} // end namespace


#include "xmlhttprequest.moc"
