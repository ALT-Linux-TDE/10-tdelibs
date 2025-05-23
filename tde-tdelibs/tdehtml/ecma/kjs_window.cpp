/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000-2003 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001-2003 David Faure (faure@kde.org)
 *  Copyright (C) 2003 Apple Computer, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "config.h"

#include "tdehtmlview.h"
#include "tdehtml_part.h"
#include "tdehtmlpart_p.h"
#include "tdehtml_settings.h"
#include "xml/dom2_eventsimpl.h"
#include "xml/dom_docimpl.h"
#include "misc/htmltags.h"
#include "html/html_documentimpl.h"
#include "rendering/render_frames.h"

#include <tqstylesheet.h>
#include <tqtimer.h>
#include <tqpaintdevicemetrics.h>
#include <tqapplication.h>
#include <kdebug.h>
#include <tdemessagebox.h>
#include <kinputdialog.h>
#include <tdelocale.h>
#include <kmdcodec.h>
#include <tdeparts/browserinterface.h>
#include <twin.h>

#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
#include <twinmodule.h> // schroder
#endif

#ifndef KONQ_EMBEDDED
#include <kbookmarkmanager.h>
#endif
#include <tdeglobalsettings.h>
#include <assert.h>
#include <tqstyle.h>
#include <tqobjectlist.h>
#include <kstringhandler.h>

#include "kjs_proxy.h"
#include "kjs_window.h"
#include "kjs_navigator.h"
#include "kjs_mozilla.h"
#include "kjs_html.h"
#include "kjs_range.h"
#include "kjs_traversal.h"
#include "kjs_css.h"
#include "kjs_events.h"
#include "kjs_views.h"
#include "xmlhttprequest.h"
#include "xmlserializer.h"
#include "domparser.h"

using namespace KJS;

namespace KJS {

  class History : public ObjectImp {
    friend class HistoryFunc;
  public:
    History(ExecState *exec, TDEHTMLPart *p)
      : ObjectImp(exec->interpreter()->builtinObjectPrototype()), part(p) { }
    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Back, Forward, Go, Length };
  private:
    TQGuardedPtr<TDEHTMLPart> part;
  };

  class External : public ObjectImp {
    friend class ExternalFunc;
  public:
    External(ExecState *exec, TDEHTMLPart *p)
      : ObjectImp(exec->interpreter()->builtinObjectPrototype()), part(p) { }
    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { AddFavorite };
  private:
    TQGuardedPtr<TDEHTMLPart> part;
  };

  class FrameArray : public ObjectImp {
  public:
    FrameArray(ExecState *exec, TDEHTMLPart *p)
      : ObjectImp(exec->interpreter()->builtinObjectPrototype()), part(p) { }
    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
    virtual Value call(ExecState *exec, Object &thisObj, const List &args);
    virtual bool implementsCall() const { return true; }
  private:
    TQGuardedPtr<TDEHTMLPart> part;
  };

#ifdef TQ_WS_QWS
  class KonquerorFunc : public DOMFunction {
  public:
    KonquerorFunc(ExecState *exec, const Konqueror* k, const char* name)
      : DOMFunction(exec), konqueror(k), m_name(name) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List &args);

  private:
    const Konqueror* konqueror;
    TQCString m_name;
  };
#endif
} // namespace KJS

#include "kjs_window.lut.h"
#include "rendering/render_replaced.h"

////////////////////// Screen Object ////////////////////////
namespace KJS {
// table for screen object
/*
@begin ScreenTable 7
  height        Screen::Height		DontEnum|ReadOnly
  width         Screen::Width		DontEnum|ReadOnly
  colorDepth    Screen::ColorDepth	DontEnum|ReadOnly
  pixelDepth    Screen::PixelDepth	DontEnum|ReadOnly
  availLeft     Screen::AvailLeft	DontEnum|ReadOnly
  availTop      Screen::AvailTop	DontEnum|ReadOnly
  availHeight   Screen::AvailHeight	DontEnum|ReadOnly
  availWidth    Screen::AvailWidth	DontEnum|ReadOnly
@end
*/

const ClassInfo Screen::info = { "Screen", 0, &ScreenTable, 0 };

// We set the object prototype so that toString is implemented
Screen::Screen(ExecState *exec)
  : ObjectImp(exec->interpreter()->builtinObjectPrototype()) {}

Value Screen::get(ExecState *exec, const Identifier &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Screen::get " << p.qstring() << endl;
#endif
  return lookupGetValue<Screen,ObjectImp>(exec,p,&ScreenTable,this);
}

Value Screen::getValueProperty(ExecState *exec, int token) const
{
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
  KWinModule info(0, KWinModule::INFO_DESKTOP);
#endif
  TQWidget *thisWidget = Window::retrieveActive(exec)->part()->widget();
  TQRect sg = TDEGlobalSettings::desktopGeometry(thisWidget);

  switch( token ) {
  case Height:
    return Number(sg.height());
  case Width:
    return Number(sg.width());
  case ColorDepth:
  case PixelDepth: {
    TQPaintDeviceMetrics m(TQApplication::desktop());
    return Number(m.depth());
  }
  case AvailLeft: {
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
    TQRect clipped = info.workArea().intersect(sg);
    return Number(clipped.x()-sg.x());
#else
    return Number(10);
#endif
  }
  case AvailTop: {
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
    TQRect clipped = info.workArea().intersect(sg);
    return Number(clipped.y()-sg.y());
#else
    return Number(10);
#endif
  }
  case AvailHeight: {
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
    TQRect clipped = info.workArea().intersect(sg);
    return Number(clipped.height());
#else
    return Number(100);
#endif
  }
  case AvailWidth: {
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
    TQRect clipped = info.workArea().intersect(sg);
    return Number(clipped.width());
#else
    return Number(100);
#endif
  }
  default:
    kdDebug(6070) << "WARNING: Screen::getValueProperty unhandled token " << token << endl;
    return Undefined();
  }
}

////////////////////// Window Object ////////////////////////

const ClassInfo Window::info = { "Window", &DOMAbstractView::info, &WindowTable, 0 };

/*
@begin WindowTable 162
  atob		Window::AToB		DontDelete|Function 1
  btoa		Window::BToA		DontDelete|Function 1
  closed	Window::Closed		DontDelete|ReadOnly
  crypto	Window::Crypto		DontDelete|ReadOnly
  defaultStatus	Window::DefaultStatus	DontDelete
  defaultstatus	Window::DefaultStatus	DontDelete
  status	Window::Status		DontDelete
  document	Window::Document	DontDelete|ReadOnly
  frameElement		Window::FrameElement		DontDelete|ReadOnly
  frames	Window::Frames		DontDelete|ReadOnly
  history	Window::_History	DontDelete|ReadOnly
  external	Window::_External	DontDelete|ReadOnly
  event		Window::Event		DontDelete|ReadOnly
  innerHeight	Window::InnerHeight	DontDelete|ReadOnly
  innerWidth	Window::InnerWidth	DontDelete|ReadOnly
  length	Window::Length		DontDelete|ReadOnly
  location	Window::_Location	DontDelete
  name		Window::Name		DontDelete
  navigator	Window::_Navigator	DontDelete|ReadOnly
  clientInformation	Window::ClientInformation	DontDelete|ReadOnly
  konqueror	Window::_Konqueror	DontDelete|ReadOnly
  offscreenBuffering	Window::OffscreenBuffering	DontDelete|ReadOnly
  opener	Window::Opener		DontDelete|ReadOnly
  outerHeight	Window::OuterHeight	DontDelete|ReadOnly
  outerWidth	Window::OuterWidth	DontDelete|ReadOnly
  pageXOffset	Window::PageXOffset	DontDelete|ReadOnly
  pageYOffset	Window::PageYOffset	DontDelete|ReadOnly
  parent	Window::Parent		DontDelete|ReadOnly
  personalbar	Window::Personalbar	DontDelete|ReadOnly
  screenX	Window::ScreenX		DontDelete|ReadOnly
  screenY	Window::ScreenY		DontDelete|ReadOnly
  scrollbars	Window::Scrollbars	DontDelete|ReadOnly
  scroll	Window::Scroll		DontDelete|Function 2
  scrollBy	Window::ScrollBy	DontDelete|Function 2
  scrollTo	Window::ScrollTo	DontDelete|Function 2
  scrollX       Window::ScrollX         DontDelete|ReadOnly
  scrollY       Window::ScrollY         DontDelete|ReadOnly
  moveBy	Window::MoveBy		DontDelete|Function 2
  moveTo	Window::MoveTo		DontDelete|Function 2
  resizeBy	Window::ResizeBy	DontDelete|Function 2
  resizeTo	Window::ResizeTo	DontDelete|Function 2
  self		Window::Self		DontDelete|ReadOnly
  window	Window::_Window		DontDelete|ReadOnly
  top		Window::Top		DontDelete|ReadOnly
  screen	Window::_Screen		DontDelete|ReadOnly
  alert		Window::Alert		DontDelete|Function 1
  confirm	Window::Confirm		DontDelete|Function 1
  prompt	Window::Prompt		DontDelete|Function 2
  open		Window::Open		DontDelete|Function 3
  setTimeout	Window::SetTimeout	DontDelete|Function 2
  clearTimeout	Window::ClearTimeout	DontDelete|Function 1
  focus		Window::Focus		DontDelete|Function 0
  blur		Window::Blur		DontDelete|Function 0
  close		Window::Close		DontDelete|Function 0
  setInterval	Window::SetInterval	DontDelete|Function 2
  clearInterval	Window::ClearInterval	DontDelete|Function 1
  captureEvents	Window::CaptureEvents	DontDelete|Function 0
  releaseEvents	Window::ReleaseEvents	DontDelete|Function 0
  print		Window::Print		DontDelete|Function 0
  addEventListener	Window::AddEventListener	DontDelete|Function 3
  removeEventListener	Window::RemoveEventListener	DontDelete|Function 3
# Normally found in prototype. Add to window object itself to make them
# accessible in closed and cross-site windows
  valueOf       Window::ValueOf		DontDelete|Function 0
  toString      Window::ToString	DontDelete|Function 0
# IE extension
  navigate	Window::Navigate	DontDelete|Function 1
# Mozilla extension
  sidebar	Window::SideBar		DontDelete|ReadOnly
  getComputedStyle	Window::GetComputedStyle	DontDelete|Function 2

# Warning, when adding a function to this object you need to add a case in Window::get

# Event handlers
# IE also has: onactivate, onbefore/afterprint, onbeforedeactivate/unload, oncontrolselect,
# ondeactivate, onhelp, onmovestart/end, onresizestart/end, onscroll.
# It doesn't have onabort, onchange, ondragdrop (but NS has that last one).
  onabort	Window::Onabort		DontDelete
  onblur	Window::Onblur		DontDelete
  onchange	Window::Onchange	DontDelete
  onclick	Window::Onclick		DontDelete
  ondblclick	Window::Ondblclick	DontDelete
  ondragdrop	Window::Ondragdrop	DontDelete
  onerror	Window::Onerror		DontDelete
  onfocus	Window::Onfocus		DontDelete
  onkeydown	Window::Onkeydown	DontDelete
  onkeypress	Window::Onkeypress	DontDelete
  onkeyup	Window::Onkeyup		DontDelete
  onload	Window::Onload		DontDelete
  onmousedown	Window::Onmousedown	DontDelete
  onmousemove	Window::Onmousemove	DontDelete
  onmouseout	Window::Onmouseout	DontDelete
  onmouseover	Window::Onmouseover	DontDelete
  onmouseup	Window::Onmouseup	DontDelete
  onmove	Window::Onmove		DontDelete
  onreset	Window::Onreset		DontDelete
  onresize	Window::Onresize	DontDelete
  onselect	Window::Onselect	DontDelete
  onsubmit	Window::Onsubmit	DontDelete
  onunload	Window::Onunload	DontDelete

# Constructors/constant tables
  Node		Window::Node		DontDelete
  Event		Window::EventCtor	DontDelete
  Range		Window::Range		DontDelete
  NodeFilter	Window::NodeFilter	DontDelete
  DOMException	Window::DOMException	DontDelete
  CSSRule	Window::CSSRule		DontDelete
  MutationEvent Window::MutationEventCtor   DontDelete
  KeyboardEvent Window::KeyboardEventCtor   DontDelete
  EventException Window::EventExceptionCtor DontDelete
  Image		Window::Image		DontDelete|ReadOnly
  Option	Window::Option		DontDelete|ReadOnly
  XMLHttpRequest Window::XMLHttpRequest DontDelete|ReadOnly
  XMLSerializer	Window::XMLSerializer	DontDelete|ReadOnly
  DOMParser	Window::DOMParser	DontDelete|ReadOnly

# Mozilla dom emulation ones.
  Element   Window::ElementCtor DontDelete
  Document  Window::DocumentCtor DontDelete
  #this one is an alias since we don't have a separate XMLDocument
  XMLDocument Window::DocumentCtor DontDelete
  HTMLElement  Window::HTMLElementCtor DontDelete
  HTMLDocument  Window::HTMLDocumentCtor DontDelete
  HTMLHtmlElement Window::HTMLHtmlElementCtor DontDelete
  HTMLHeadElement Window::HTMLHeadElementCtor DontDelete
  HTMLLinkElement Window::HTMLLinkElementCtor DontDelete
  HTMLTitleElement Window::HTMLTitleElementCtor DontDelete
  HTMLMetaElement Window::HTMLMetaElementCtor DontDelete
  HTMLBaseElement Window::HTMLBaseElementCtor DontDelete
  HTMLIsIndexElement Window::HTMLIsIndexElementCtor DontDelete
  HTMLStyleElement Window::HTMLStyleElementCtor DontDelete
  HTMLBodyElement Window::HTMLBodyElementCtor DontDelete
  HTMLFormElement Window::HTMLFormElementCtor DontDelete
  HTMLSelectElement Window::HTMLSelectElementCtor DontDelete
  HTMLOptGroupElement Window::HTMLOptGroupElementCtor DontDelete
  HTMLOptionElement Window::HTMLOptionElementCtor DontDelete
  HTMLInputElement Window::HTMLInputElementCtor DontDelete
  HTMLTextAreaElement Window::HTMLTextAreaElementCtor DontDelete
  HTMLButtonElement Window::HTMLButtonElementCtor DontDelete
  HTMLLabelElement Window::HTMLLabelElementCtor DontDelete
  HTMLFieldSetElement Window::HTMLFieldSetElementCtor DontDelete
  HTMLLegendElement Window::HTMLLegendElementCtor DontDelete
  HTMLUListElement Window::HTMLUListElementCtor DontDelete
  HTMLOListElement Window::HTMLOListElementCtor DontDelete
  HTMLDListElement Window::HTMLDListElementCtor DontDelete
  HTMLDirectoryElement Window::HTMLDirectoryElementCtor DontDelete
  HTMLMenuElement Window::HTMLMenuElementCtor DontDelete
  HTMLLIElement Window::HTMLLIElementCtor DontDelete
  HTMLDivElement Window::HTMLDivElementCtor DontDelete
  HTMLParagraphElement Window::HTMLParagraphElementCtor DontDelete
  HTMLHeadingElement Window::HTMLHeadingElementCtor DontDelete
  HTMLBlockQuoteElement Window::HTMLBlockQuoteElementCtor DontDelete
  HTMLQuoteElement Window::HTMLQuoteElementCtor DontDelete
  HTMLPreElement Window::HTMLPreElementCtor DontDelete
  HTMLBRElement Window::HTMLBRElementCtor DontDelete
  HTMLBaseFontElement Window::HTMLBaseFontElementCtor DontDelete
  HTMLFontElement Window::HTMLFontElementCtor DontDelete
  HTMLHRElement Window::HTMLHRElementCtor DontDelete
  HTMLModElement Window::HTMLModElementCtor DontDelete
  HTMLAnchorElement Window::HTMLAnchorElementCtor DontDelete
  HTMLImageElement Window::HTMLImageElementCtor DontDelete
  HTMLObjectElement Window::HTMLObjectElementCtor DontDelete
  HTMLParamElement Window::HTMLParamElementCtor DontDelete
  HTMLAppletElement Window::HTMLAppletElementCtor DontDelete
  HTMLMapElement Window::HTMLMapElementCtor DontDelete
  HTMLAreaElement Window::HTMLAreaElementCtor DontDelete
  HTMLScriptElement Window::HTMLScriptElementCtor DontDelete
  HTMLTableElement Window::HTMLTableElementCtor DontDelete
  HTMLTableCaptionElement Window::HTMLTableCaptionElementCtor DontDelete
  HTMLTableColElement Window::HTMLTableColElementCtor DontDelete
  HTMLTableSectionElement Window::HTMLTableSectionElementCtor DontDelete
  HTMLTableRowElement Window::HTMLTableRowElementCtor DontDelete
  HTMLTableCellElement Window::HTMLTableCellElementCtor DontDelete
  HTMLFrameSetElement Window::HTMLFrameSetElementCtor DontDelete
  HTMLLayerElement Window::HTMLLayerElementCtor DontDelete
  HTMLFrameElement Window::HTMLFrameElementCtor DontDelete
  HTMLIFrameElement Window::HTMLIFrameElementCtor DontDelete
  CSSStyleDeclaration Window::CSSStyleDeclarationCtor DontDelete
@end
*/
IMPLEMENT_PROTOFUNC_DOM(WindowFunc)

Window::Window(tdehtml::ChildFrame *p)
  : ObjectImp(/*no proto*/), m_frame(p), screen(0), history(0), external(0), m_frames(0), loc(0), m_evt(0)
{
  winq = new WindowQObject(this);
  //kdDebug(6070) << "Window::Window this=" << this << " part=" << m_part << " " << m_part->name() << endl;
}

Window::~Window()
{
  delete winq;
}

Window *Window::retrieveWindow(KParts::ReadOnlyPart *p)
{
  Object obj = Object::dynamicCast( retrieve( p ) );
#ifndef NDEBUG
  // obj should never be null, except when javascript has been disabled in that part.
  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(p);
  if ( part && part->jScriptEnabled() )
  {
    assert( obj.isValid() );
#ifndef QWS
    assert( dynamic_cast<KJS::Window*>(obj.imp()) ); // type checking
#endif
  }
#endif
  if ( !obj.isValid() ) // JS disabled
    return 0;
  return static_cast<KJS::Window*>(obj.imp());
}

Window *Window::retrieveActive(ExecState *exec)
{
  ValueImp *imp = exec->interpreter()->globalObject().imp();
  assert( imp );
#ifndef QWS
  assert( dynamic_cast<KJS::Window*>(imp) );
#endif
  return static_cast<KJS::Window*>(imp);
}

Value Window::retrieve(KParts::ReadOnlyPart *p)
{
  assert(p);
  TDEHTMLPart * part = ::tqt_cast<TDEHTMLPart *>(p);
  KJSProxy *proxy = 0L;
  if (!part) {
    part = ::tqt_cast<TDEHTMLPart *>(p->parent());
    if (part)
      proxy = part->framejScript(p);
  } else
    proxy = part->jScript();
  if (proxy) {
#ifdef KJS_VERBOSE
    kdDebug(6070) << "Window::retrieve part=" << part << " '" << part->name() << "' interpreter=" << proxy->interpreter() << " window=" << proxy->interpreter()->globalObject().imp() << endl;
#endif
    return proxy->interpreter()->globalObject(); // the Global object is the "window"
  } else {
#ifdef KJS_VERBOSE
    kdDebug(6070) << "Window::retrieve part=" << p << " '" << p->name() << "' no jsproxy." << endl;
#endif
    return Undefined(); // This can happen with JS disabled on the domain of that window
  }
}

Location *Window::location() const
{
  if (!loc)
    const_cast<Window*>(this)->loc = new Location(m_frame);
  return loc;
}

ObjectImp* Window::frames( ExecState* exec ) const
{
  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);
  if (part)
    return m_frames ? m_frames :
      (const_cast<Window*>(this)->m_frames = new FrameArray(exec, part));
  return 0L;
}

// reference our special objects during garbage collection
void Window::mark()
{
  ObjectImp::mark();
  if (screen && !screen->marked())
    screen->mark();
  if (history && !history->marked())
    history->mark();
  if (external && !external->marked())
    external->mark();
  if (m_frames && !m_frames->marked())
    m_frames->mark();
  //kdDebug(6070) << "Window::mark " << this << " marking loc=" << loc << endl;
  if (loc && !loc->marked())
    loc->mark();
  if (winq)
    winq->mark();
}

bool Window::hasProperty(ExecState *exec, const Identifier &p) const
{
  // we don't want any operations on a closed window
  if (m_frame.isNull() || m_frame->m_part.isNull())
    return ( p == "closed" );

  if (ObjectImp::hasProperty(exec, p))
    return true;

  if (Lookup::findEntry(&WindowTable, p))
    return true;

  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);
  if (!part)
      return false;

  TQString q = p.qstring();
  if (part->findFramePart(p.qstring()))
    return true;
  // allow window[1] or parent[1] etc. (#56983)
  bool ok;
  unsigned int i = p.toArrayIndex(&ok);
  if (ok) {
    TQPtrList<KParts::ReadOnlyPart> frames = part->frames();
    unsigned int len = frames.count();
    if (i < len)
      return true;
  }

  // allow shortcuts like 'Image1' instead of document.images.Image1
  if (part->document().isHTMLDocument()) { // might be XML
    DOM::HTMLDocument doc = part->htmlDocument();
    // Keep in sync with tryGet

    if (static_cast<DOM::DocumentImpl*>(doc.handle())->underDocNamedCache().get(p.qstring()))
      return true;

    return !doc.getElementById(p.string()).isNull();
  }

  return false;
}

UString Window::toString(ExecState *) const
{
  return "[object Window]";
}

Value Window::get(ExecState *exec, const Identifier &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Window("<<this<<")::get " << p.qstring() << endl;
#endif
  // we want only limited operations on a closed window
  if (m_frame.isNull() || m_frame->m_part.isNull()) {
    const HashEntry* entry = Lookup::findEntry(&WindowTable, p);
    if (entry) {
      switch (entry->value) {
      case Closed:
	return Boolean(true);
      case _Location:
	return Null();
      case ValueOf:
      case ToString:
      return lookupOrCreateFunction<WindowFunc>(exec,p, this, entry->value,
						entry->params, entry->attr);
      default:
	break;
      }
    }
    return Undefined();
  }

  // Look for overrides first
  ValueImp *val = getDirect(p);
  if (val) {
    //kdDebug(6070) << "Window::get found dynamic property '" << p.ascii() << "'" << endl;
    return isSafeScript(exec) ? Value(val) : Undefined();
  }

  const HashEntry* entry = Lookup::findEntry(&WindowTable, p);
  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);

  // properties that work on all windows
  if (entry) {
    // ReadOnlyPart first
    switch(entry->value) {
    case Closed:
      return Boolean( false );
    case _Location:
        // No isSafeScript test here, we must be able to _set_ location.href (#49819)
      return Value(location());
    case _Window:
    case Self:
      return retrieve(m_frame->m_part);
    default:
        break;
    }
    if (!part)
        return Undefined();
    // TDEHTMLPart next
    switch(entry->value) {
    case Frames:
      return Value(frames(exec));
    case Opener:
      if (!part->opener())
        return Null();    // ### a null Window might be better, but == null
      else                // doesn't work yet
        return retrieve(part->opener());
    case Parent:
      return retrieve(part->parentPart() ? part->parentPart() : (TDEHTMLPart*)part);
    case Top: {
      TDEHTMLPart *p = part;
      while (p->parentPart())
        p = p->parentPart();
      return retrieve(p);
    }
    case Alert:
    case Confirm:
    case Prompt:
    case Open:
    case Close:
    case Focus:
    case Blur:
    case AToB:
    case BToA:
    case GetComputedStyle:
    case ValueOf:
    case ToString:
      return lookupOrCreateFunction<WindowFunc>(exec,p,this,entry->value,entry->params,entry->attr);
    default:
      break;
    }
  } else if (!part) {
    // not a  TDEHTMLPart
    TQString rvalue;
    KParts::LiveConnectExtension::Type rtype;
    unsigned long robjid;
    if (m_frame->m_liveconnect &&
        isSafeScript(exec) &&
        m_frame->m_liveconnect->get(0, p.qstring(), rtype, robjid, rvalue))
      return getLiveConnectValue(m_frame->m_liveconnect, p.qstring(), rtype, rvalue, robjid);
    return Undefined();
  }
  // properties that only work on safe windows
  if (isSafeScript(exec) &&  entry)
  {
    //kdDebug(6070) << "token: " << entry->value << endl;
    switch( entry->value ) {
    case Crypto:
      return Undefined(); // ###
    case DefaultStatus:
      return String(UString(part->jsDefaultStatusBarText()));
    case Status:
      return String(UString(part->jsStatusBarText()));
    case Document:
      if (part->document().isNull()) {
        kdDebug(6070) << "Document.write: adding <HTML><BODY> to create document" << endl;
        part->begin();
        part->write("<HTML><BODY>");
        part->end();
      }
      return getDOMNode(exec,part->document());
    case FrameElement:
      if (m_frame->m_frame)
        return getDOMNode(exec,m_frame->m_frame->element());
      else
        return Undefined();
    case Node:
      return NodeConstructor::self(exec);
    case Range:
      return getRangeConstructor(exec);
    case NodeFilter:
      return getNodeFilterConstructor(exec);
    case DOMException:
      return getDOMExceptionConstructor(exec);
    case CSSRule:
      return getCSSRuleConstructor(exec);
    case ElementCtor:
      return ElementPseudoCtor::self(exec);
    case HTMLElementCtor:
      return HTMLElementPseudoCtor::self(exec);
    case HTMLHtmlElementCtor:
      return HTMLHtmlElementPseudoCtor::self(exec);
    case HTMLHeadElementCtor:
      return HTMLHeadElementPseudoCtor::self(exec);
    case HTMLLinkElementCtor:
      return HTMLLinkElementPseudoCtor::self(exec);
    case HTMLTitleElementCtor:
      return HTMLTitleElementPseudoCtor::self(exec);
    case HTMLMetaElementCtor:
      return HTMLMetaElementPseudoCtor::self(exec);
    case HTMLBaseElementCtor:
      return HTMLBaseElementPseudoCtor::self(exec);
    case HTMLIsIndexElementCtor:
      return HTMLIsIndexElementPseudoCtor::self(exec);
    case HTMLStyleElementCtor:
      return HTMLStyleElementPseudoCtor::self(exec);
    case HTMLBodyElementCtor:
      return HTMLBodyElementPseudoCtor::self(exec);
    case HTMLFormElementCtor:
      return HTMLFormElementPseudoCtor::self(exec);
    case HTMLSelectElementCtor:
      return HTMLSelectElementPseudoCtor::self(exec);
    case HTMLOptGroupElementCtor:
      return HTMLOptGroupElementPseudoCtor::self(exec);
    case HTMLOptionElementCtor:
      return HTMLOptionElementPseudoCtor::self(exec);
    case HTMLInputElementCtor:
      return HTMLInputElementPseudoCtor::self(exec);
    case HTMLTextAreaElementCtor:
      return HTMLTextAreaElementPseudoCtor::self(exec);
    case HTMLButtonElementCtor:
      return HTMLButtonElementPseudoCtor::self(exec);
    case HTMLLabelElementCtor:
      return HTMLLabelElementPseudoCtor::self(exec);
    case HTMLFieldSetElementCtor:
      return HTMLFieldSetElementPseudoCtor::self(exec);
    case HTMLLegendElementCtor:
      return HTMLLegendElementPseudoCtor::self(exec);
    case HTMLUListElementCtor:
      return HTMLUListElementPseudoCtor::self(exec);
    case HTMLOListElementCtor:
      return HTMLOListElementPseudoCtor::self(exec);
    case HTMLDListElementCtor:
      return HTMLDListElementPseudoCtor::self(exec);
    case HTMLDirectoryElementCtor:
      return HTMLDirectoryElementPseudoCtor::self(exec);
    case HTMLMenuElementCtor:
      return HTMLMenuElementPseudoCtor::self(exec);
    case HTMLLIElementCtor:
      return HTMLLIElementPseudoCtor::self(exec);
    case HTMLDivElementCtor:
      return HTMLDivElementPseudoCtor::self(exec);
    case HTMLParagraphElementCtor:
      return HTMLParagraphElementPseudoCtor::self(exec);
    case HTMLHeadingElementCtor:
      return HTMLHeadingElementPseudoCtor::self(exec);
    case HTMLBlockQuoteElementCtor:
      return HTMLBlockQuoteElementPseudoCtor::self(exec);
    case HTMLQuoteElementCtor:
      return HTMLQuoteElementPseudoCtor::self(exec);
    case HTMLPreElementCtor:
      return HTMLPreElementPseudoCtor::self(exec);
    case HTMLBRElementCtor:
      return HTMLBRElementPseudoCtor::self(exec);
    case HTMLBaseFontElementCtor:
      return HTMLBaseFontElementPseudoCtor::self(exec);
    case HTMLFontElementCtor:
      return HTMLFontElementPseudoCtor::self(exec);
    case HTMLHRElementCtor:
      return HTMLHRElementPseudoCtor::self(exec);
    case HTMLModElementCtor:
      return HTMLModElementPseudoCtor::self(exec);
    case HTMLAnchorElementCtor:
      return HTMLAnchorElementPseudoCtor::self(exec);
    case HTMLImageElementCtor:
      return HTMLImageElementPseudoCtor::self(exec);
    case HTMLObjectElementCtor:
      return HTMLObjectElementPseudoCtor::self(exec);
    case HTMLParamElementCtor:
      return HTMLParamElementPseudoCtor::self(exec);
    case HTMLAppletElementCtor:
      return HTMLAppletElementPseudoCtor::self(exec);
    case HTMLMapElementCtor:
      return HTMLMapElementPseudoCtor::self(exec);
    case HTMLAreaElementCtor:
      return HTMLAreaElementPseudoCtor::self(exec);
    case HTMLScriptElementCtor:
      return HTMLScriptElementPseudoCtor::self(exec);
    case HTMLTableElementCtor:
      return HTMLTableElementPseudoCtor::self(exec);
    case HTMLTableCaptionElementCtor:
      return HTMLTableCaptionElementPseudoCtor::self(exec);
    case HTMLTableColElementCtor:
      return HTMLTableColElementPseudoCtor::self(exec);
    case HTMLTableSectionElementCtor:
      return HTMLTableSectionElementPseudoCtor::self(exec);
    case HTMLTableRowElementCtor:
      return HTMLTableRowElementPseudoCtor::self(exec);
    case HTMLTableCellElementCtor:
      return HTMLTableCellElementPseudoCtor::self(exec);
    case HTMLFrameSetElementCtor:
      return HTMLFrameSetElementPseudoCtor::self(exec);
    case HTMLLayerElementCtor:
      return HTMLLayerElementPseudoCtor::self(exec);
    case HTMLFrameElementCtor:
      return HTMLFrameElementPseudoCtor::self(exec);
    case HTMLIFrameElementCtor:
      return HTMLIFrameElementPseudoCtor::self(exec);
    case DocumentCtor:
      return DocumentPseudoCtor::self(exec);
    case HTMLDocumentCtor:
      return HTMLDocumentPseudoCtor::self(exec);
    case CSSStyleDeclarationCtor:
        return CSSStyleDeclarationPseudoCtor::self(exec);
    case EventCtor:
      return EventConstructor::self(exec);
    case MutationEventCtor:
      return getMutationEventConstructor(exec);
    case KeyboardEventCtor:
      return getKeyboardEventConstructor(exec);
    case EventExceptionCtor:
      return getEventExceptionConstructor(exec);
    case _History:
      return Value(history ? history :
                   (const_cast<Window*>(this)->history = new History(exec,part)));

    case _External:
      return Value(external ? external :
                   (const_cast<Window*>(this)->external = new External(exec,part)));

    case Event:
      if (m_evt)
        return getDOMEvent(exec,*m_evt);
      else {
#ifdef KJS_VERBOSE
        kdDebug(6070) << "WARNING: window(" << this << "," << part->name() << ").event, no event!" << endl;
#endif
        return Undefined();
      }
    case InnerHeight:
      if (!part->view())
        return Undefined();
      tdehtml::RenderWidget::flushWidgetResizes(); // make sure frames have their final size
      return Number(part->view()->visibleHeight());
    case InnerWidth:
      if (!part->view())
        return Undefined();
      tdehtml::RenderWidget::flushWidgetResizes(); // make sure frames have their final size
      return Number(part->view()->visibleWidth());
    case Length:
      return Number(part->frames().count());
    case Name:
      return String(part->name());
    case SideBar:
      return Value(new MozillaSidebarExtension(exec, part));
    case _Navigator:
    case ClientInformation: {
      // Store the navigator in the object so we get the same one each time.
      Value nav( new Navigator(exec, part) );
      const_cast<Window *>(this)->put(exec, "navigator", nav, DontDelete|ReadOnly|Internal);
      const_cast<Window *>(this)->put(exec, "clientInformation", nav, DontDelete|ReadOnly|Internal);
      return nav;
    }
#ifdef TQ_WS_QWS
    case _Konqueror: {
      Value k( new Konqueror(part) );
      const_cast<Window *>(this)->put(exec, "konqueror", k, DontDelete|ReadOnly|Internal);
      return k;
    }
#endif
    case OffscreenBuffering:
      return Boolean(true);
    case OuterHeight:
    case OuterWidth:
    {
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
      if (!part->widget())
        return Number(0);
      KWin::WindowInfo inf = KWin::windowInfo(part->widget()->topLevelWidget()->winId());
      return Number(entry->value == OuterHeight ?
                    inf.geometry().height() : inf.geometry().width());
#else
      return Number(entry->value == OuterHeight ?  
		    part->view()->height() : part->view()->width());
#endif
    }
    case PageXOffset:
      return Number(part->view()->contentsX());
    case PageYOffset:
      return Number(part->view()->contentsY());
    case Personalbar:
      return Undefined(); // ###
    case ScreenLeft:
    case ScreenX: {
      if (!part->view())
        return Undefined();
      TQRect sg = TDEGlobalSettings::desktopGeometry(part->view());
      return Number(part->view()->mapToGlobal(TQPoint(0,0)).x() + sg.x());
    }
    case ScreenTop:
    case ScreenY: {
      if (!part->view())
        return Undefined();
      TQRect sg = TDEGlobalSettings::desktopGeometry(part->view());
      return Number(part->view()->mapToGlobal(TQPoint(0,0)).y() + sg.y());
    }
    case ScrollX: {
      if (!part->view())
        return Undefined();
      return Number(part->view()->contentsX());
    }
    case ScrollY: {
      if (!part->view())
        return Undefined();
      return Number(part->view()->contentsY());
    }
    case Scrollbars:
      return Undefined(); // ###
    case _Screen:
      return Value(screen ? screen :
                   (const_cast<Window*>(this)->screen = new Screen(exec)));
    case Image:
      return Value(new ImageConstructorImp(exec, part->document()));
    case Option:
      return Value(new OptionConstructorImp(exec, part->document()));
    case XMLHttpRequest:
      return Value(new XMLHttpRequestConstructorImp(exec, part->document()));
    case XMLSerializer:
      return Value(new XMLSerializerConstructorImp(exec));
    case DOMParser:
      return Value(new DOMParserConstructorImp(exec, part->xmlDocImpl()));
    case Scroll: // compatibility
    case ScrollBy:
    case ScrollTo:
    case MoveBy:
    case MoveTo:
    case ResizeBy:
    case ResizeTo:
    case CaptureEvents:
    case ReleaseEvents:
    case AddEventListener:
    case RemoveEventListener:
    case SetTimeout:
    case ClearTimeout:
    case SetInterval:
    case ClearInterval:
    case Print:
      return lookupOrCreateFunction<WindowFunc>(exec,p,this,entry->value,entry->params,entry->attr);
    // IE extension
    case Navigate:
      // Disabled in NS-compat mode. Supported by default - can't hurt, unless someone uses
      // if (navigate) to test for IE (unlikely).
      if ( exec->interpreter()->compatMode() == Interpreter::NetscapeCompat )
        return Undefined();
      return lookupOrCreateFunction<WindowFunc>(exec,p,this,entry->value,entry->params,entry->attr);
    case Onabort:
      return getListener(exec,DOM::EventImpl::ABORT_EVENT);
    case Onblur:
      return getListener(exec,DOM::EventImpl::BLUR_EVENT);
    case Onchange:
      return getListener(exec,DOM::EventImpl::CHANGE_EVENT);
    case Onclick:
      return getListener(exec,DOM::EventImpl::TDEHTML_ECMA_CLICK_EVENT);
    case Ondblclick:
      return getListener(exec,DOM::EventImpl::TDEHTML_ECMA_DBLCLICK_EVENT);
    case Ondragdrop:
      return getListener(exec,DOM::EventImpl::TDEHTML_DRAGDROP_EVENT);
    case Onerror:
      return getListener(exec,DOM::EventImpl::ERROR_EVENT);
    case Onfocus:
      return getListener(exec,DOM::EventImpl::FOCUS_EVENT);
    case Onkeydown:
      return getListener(exec,DOM::EventImpl::KEYDOWN_EVENT);
    case Onkeypress:
      return getListener(exec,DOM::EventImpl::KEYPRESS_EVENT);
    case Onkeyup:
      return getListener(exec,DOM::EventImpl::KEYUP_EVENT);
    case Onload:
      return getListener(exec,DOM::EventImpl::LOAD_EVENT);
    case Onmousedown:
      return getListener(exec,DOM::EventImpl::MOUSEDOWN_EVENT);
    case Onmousemove:
      return getListener(exec,DOM::EventImpl::MOUSEMOVE_EVENT);
    case Onmouseout:
      return getListener(exec,DOM::EventImpl::MOUSEOUT_EVENT);
    case Onmouseover:
      return getListener(exec,DOM::EventImpl::MOUSEOVER_EVENT);
    case Onmouseup:
      return getListener(exec,DOM::EventImpl::MOUSEUP_EVENT);
    case Onmove:
      return getListener(exec,DOM::EventImpl::TDEHTML_MOVE_EVENT);
    case Onreset:
      return getListener(exec,DOM::EventImpl::RESET_EVENT);
    case Onresize:
      return getListener(exec,DOM::EventImpl::RESIZE_EVENT);
    case Onselect:
      return getListener(exec,DOM::EventImpl::SELECT_EVENT);
    case Onsubmit:
      return getListener(exec,DOM::EventImpl::SUBMIT_EVENT);
    case Onunload:
      return getListener(exec,DOM::EventImpl::UNLOAD_EVENT);
    }
  }

  // doing the remainder of ObjectImp::get() that is not covered by
  // the getDirect() call above.
  // #### guessed position. move further up or down?
  Object proto = Object::dynamicCast(prototype());
  assert(proto.isValid());
  if (p == specialPrototypePropertyName)
    return isSafeScript(exec) ? Value(proto) : Undefined();
  Value val2 = proto.get(exec, p);
  if (!val2.isA(UndefinedType)) {
    return isSafeScript(exec) ? val2 : Undefined();
  }

  KParts::ReadOnlyPart *rop = part->findFramePart( p.qstring() );
  if (rop)
    return retrieve(rop);

  // allow window[1] or parent[1] etc. (#56983)
  bool ok;
  unsigned int i = p.toArrayIndex(&ok);
  if (ok) {
    TQPtrList<KParts::ReadOnlyPart> frames = part->frames();
    unsigned int len = frames.count();
    if (i < len) {
      KParts::ReadOnlyPart* frame = frames.at(i);
      if (frame)
        return Window::retrieve(frame);
    }
  }

  //Check for images, forms, objects, etc.
  if (isSafeScript(exec) && part->document().isHTMLDocument()) { // might be XML
    DOM::DocumentImpl* docImpl = part->xmlDocImpl();
    DOM::ElementMappingCache::ItemInfo* info = docImpl->underDocNamedCache().get(p.qstring());
    if (info) {
      //May be a false positive, but we can try to avoid doing it the hard way in
      //simpler cases. The trickiness here is that the cache is kept under both
      //name and id, but we sometimes ignore id for IE compat
      DOM::DOMString  propertyDOMString = p.string();
      if (info->nd && DOM::HTMLMappedNameCollectionImpl::matchesName(info->nd,
                    DOM::HTMLCollectionImpl::WINDOW_NAMED_ITEMS, propertyDOMString)) {
        return getDOMNode(exec, info->nd);
      } else {
        //Can't tell it just like that, so better go through collection and count stuff. This is the slow path...
        DOM::HTMLMappedNameCollection coll(docImpl, DOM::HTMLCollectionImpl::WINDOW_NAMED_ITEMS, propertyDOMString);
  
        if (coll.length() == 1)
          return getDOMNode(exec, coll.firstItem());
        else if (coll.length() > 1)
          return getHTMLCollection(exec, coll);
      }
    }
    DOM::Element element = part->document().getElementById(p.string());
    if ( !element.isNull() )
      return getDOMNode(exec, element );
  }

  // This isn't necessarily a bug. Some code uses if(!window.blah) window.blah=1
  // But it can also mean something isn't loaded or implemented, hence the WARNING to help grepping.
#ifdef KJS_VERBOSE
  kdDebug(6070) << "WARNING: Window::get property not found: " << p.qstring() << endl;
#endif
  return Undefined();
}

void Window::put(ExecState* exec, const Identifier &propertyName, const Value &value, int attr)
{
  // we don't want any operations on a closed window
  if (m_frame.isNull() || m_frame->m_part.isNull()) {
    // ### throw exception? allow setting of some props like location?
    return;
  }

  // Called by an internal KJS call (e.g. InterpreterImp's constructor) ?
  // If yes, save time and jump directly to ObjectImp.
  if ( (attr != None && attr != DontDelete) ||
       // Same thing if we have a local override (e.g. "var location")
       ( isSafeScript( exec ) && ObjectImp::getDirect(propertyName) ) )
  {
    ObjectImp::put( exec, propertyName, value, attr );
    return;
  }

  const HashEntry* entry = Lookup::findEntry(&WindowTable, propertyName);
  if (entry && !m_frame.isNull() && !m_frame->m_part.isNull())
  {
#ifdef KJS_VERBOSE
    kdDebug(6070) << "Window("<<this<<")::put " << propertyName.qstring() << endl;
#endif
    switch( entry->value) {
    case _Location:
      goURL(exec, value.toString(exec).qstring(), false /*don't lock history*/);
      return;
    default:
      break;
    }
    TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);
    if (part) {
    switch( entry->value ) {
    case Status: {
      if  (isSafeScript(exec) && part->settings()->windowStatusPolicy(part->url().host())
		== TDEHTMLSettings::KJSWindowStatusAllow) {
      String s = value.toString(exec);
      part->setJSStatusBarText(s.value().qstring());
      }
      return;
    }
    case DefaultStatus: {
      if (isSafeScript(exec) && part->settings()->windowStatusPolicy(part->url().host())
		== TDEHTMLSettings::KJSWindowStatusAllow) {
      String s = value.toString(exec);
      part->setJSDefaultStatusBarText(s.value().qstring());
      }
      return;
    }
    case Onabort:
      if (isSafeScript(exec))
        setListener(exec, DOM::EventImpl::ABORT_EVENT,value);
      return;
    case Onblur:
      if (isSafeScript(exec))
        setListener(exec, DOM::EventImpl::BLUR_EVENT,value);
      return;
    case Onchange:
      if (isSafeScript(exec))
        setListener(exec, DOM::EventImpl::CHANGE_EVENT,value);
      return;
    case Onclick:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::TDEHTML_ECMA_CLICK_EVENT,value);
      return;
    case Ondblclick:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::TDEHTML_ECMA_DBLCLICK_EVENT,value);
      return;
    case Ondragdrop:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::TDEHTML_DRAGDROP_EVENT,value);
      return;
    case Onerror:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::ERROR_EVENT,value);
      return;
    case Onfocus:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::FOCUS_EVENT,value);
      return;
    case Onkeydown:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KEYDOWN_EVENT,value);
      return;
    case Onkeypress:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KEYPRESS_EVENT,value);
      return;
    case Onkeyup:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KEYUP_EVENT,value);
      return;
    case Onload:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::LOAD_EVENT,value);
      return;
    case Onmousedown:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEDOWN_EVENT,value);
      return;
    case Onmousemove:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEMOVE_EVENT,value);
      return;
    case Onmouseout:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEOUT_EVENT,value);
      return;
    case Onmouseover:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEOVER_EVENT,value);
      return;
    case Onmouseup:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEUP_EVENT,value);
      return;
    case Onmove:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::TDEHTML_MOVE_EVENT,value);
      return;
    case Onreset:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::RESET_EVENT,value);
      return;
    case Onresize:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::RESIZE_EVENT,value);
      return;
    case Onselect:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::SELECT_EVENT,value);
      return;
    case Onsubmit:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::SUBMIT_EVENT,value);
      return;
    case Onunload:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::UNLOAD_EVENT,value);
      return;
    case Name:
      if (isSafeScript(exec))
        part->setName( value.toString(exec).qstring().local8Bit().data() );
      return;
    default:
      break;
    }
    }
  }
  if (m_frame->m_liveconnect &&
      isSafeScript(exec) &&
      m_frame->m_liveconnect->put(0, propertyName.qstring(), value.toString(exec).qstring()))
    return;
  if (isSafeScript(exec)) {
    //kdDebug(6070) << "Window("<<this<<")::put storing " << propertyName.qstring() << endl;
    ObjectImp::put(exec, propertyName, value, attr);
  }
}

bool Window::toBoolean(ExecState *) const
{
  return !m_frame.isNull() && !m_frame->m_part.isNull();
}

DOM::AbstractView Window::toAbstractView() const
{
  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);
  if (!part)
    return DOM::AbstractView();
  return part->document().defaultView();
}

void Window::scheduleClose()
{
  kdDebug(6070) << "Window::scheduleClose window.close() " << m_frame << endl;
  Q_ASSERT(winq);
  TQTimer::singleShot( 0, winq, TQ_SLOT( timeoutClose() ) );
}

void Window::closeNow()
{
  if (m_frame.isNull() || m_frame->m_part.isNull()) {
    kdDebug(6070) << k_funcinfo << "part is deleted already" << endl;
  } else {
    TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);
    if (!part) {
      kdDebug(6070) << "closeNow on non TDEHTML part" << endl;
    } else {
      //kdDebug(6070) << k_funcinfo << " -> closing window" << endl;
      // We want to make sure that window.open won't find this part by name.
      part->setName( 0 );
      part->deleteLater();
      part = 0;
    }
  }
}

void Window::afterScriptExecution()
{
  DOM::DocumentImpl::updateDocumentsRendering();
  TQValueList<DelayedAction> delayedActions = m_delayed;
  m_delayed.clear();
  TQValueList<DelayedAction>::Iterator it = delayedActions.begin();
  for ( ; it != delayedActions.end() ; ++it )
  {
    switch ((*it).actionId) {
    case DelayedClose:
      scheduleClose();
      return; // stop here, in case of multiple actions
    case DelayedGoHistory:
      goHistory( (*it).param.toInt() );
      break;
    case NullAction:
      // FIXME: anything needs to be done here?  This is warning anyways.
      break;
    };
  }
}

bool Window::checkIsSafeScript(KParts::ReadOnlyPart *activePart) const
{
  if (m_frame.isNull() || m_frame->m_part.isNull()) { // part deleted ? can't grant access
    kdDebug(6070) << "Window::isSafeScript: accessing deleted part !" << endl;
    return false;
  }
  if (!activePart) {
    kdDebug(6070) << "Window::isSafeScript: current interpreter's part is 0L!" << endl;
    return false;
  }
   if ( activePart == m_frame->m_part ) // Not calling from another frame, no problem.
     return true;

  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);
  if (!part)
    return true; // not a TDEHTMLPart

  if ( part->document().isNull() )
    return true; // allow to access a window that was just created (e.g. with window.open("about:blank"))

  DOM::HTMLDocument thisDocument = part->htmlDocument();
  if ( thisDocument.isNull() ) {
    kdDebug(6070) << "Window::isSafeScript: trying to access an XML document !?" << endl;
    return false;
  }

  TDEHTMLPart *activeTDEHTMLPart = ::tqt_cast<TDEHTMLPart *>(activePart);
  if (!activeTDEHTMLPart)
    return true; // not a TDEHTMLPart

  DOM::HTMLDocument actDocument = activeTDEHTMLPart->htmlDocument();
  if ( actDocument.isNull() ) {
    kdDebug(6070) << "Window::isSafeScript: active part has no document!" << endl;
    return false;
  }
  DOM::DOMString actDomain = actDocument.domain();
  DOM::DOMString thisDomain = thisDocument.domain();

  if ( actDomain == thisDomain ) {
#ifdef KJS_VERBOSE
    //kdDebug(6070) << "JavaScript: access granted, domain is '" << actDomain.string() << "'" << endl;
#endif
    return true;
  }

  kdDebug(6070) << "WARNING: JavaScript: access denied for current frame '" << actDomain.string() << "' to frame '" << thisDomain.string() << "'" << endl;
  // TODO after 3.1: throw security exception (exec->setException())
  return false;
}

void Window::setListener(ExecState *exec, int eventId, Value func)
{
  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);
  if (!part || !isSafeScript(exec))
    return;
  DOM::DocumentImpl *doc = static_cast<DOM::DocumentImpl*>(part->htmlDocument().handle());
  if (!doc)
    return;

  doc->setHTMLWindowEventListener(eventId,getJSEventListener(func,true));
}

Value Window::getListener(ExecState *exec, int eventId) const
{
  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);
  if (!part || !isSafeScript(exec))
    return Undefined();
  DOM::DocumentImpl *doc = static_cast<DOM::DocumentImpl*>(part->htmlDocument().handle());
  if (!doc)
    return Undefined();

  DOM::EventListener *listener = doc->getHTMLWindowEventListener(eventId);
  if (listener && static_cast<JSEventListener*>(listener)->listenerObjImp())
    return static_cast<JSEventListener*>(listener)->listenerObj();
  else
    return Null();
}


JSEventListener *Window::getJSEventListener(const Value& val, bool html)
{
  // This function is so hot that it's worth coding it directly with imps.
  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);
  if (!part || val.type() != ObjectType)
    return 0;

  // It's ObjectType, so it must be valid.
  Object listenerObject = Object::dynamicCast(val);
  ObjectImp *listenerObjectImp = listenerObject.imp();

  // 'listener' is not a simple ecma function. (Always use sanity checks: Better safe than sorry!)
  if (!listenerObject.implementsCall() && part && part->jScript() && part->jScript()->interpreter())
  {
    Interpreter *interpreter = part->jScript()->interpreter();

    // 'listener' probably is an EventListener object containing a 'handleEvent' function.
    Value handleEventValue = listenerObject.get(interpreter->globalExec(), Identifier("handleEvent"));
    Object handleEventObject = Object::dynamicCast(handleEventValue);

    if(handleEventObject.isValid() && handleEventObject.implementsCall())
    {
      listenerObject = handleEventObject;
      listenerObjectImp = handleEventObject.imp();
    }
  }

  JSEventListener *existingListener = jsEventListeners[listenerObjectImp];
  if (existingListener) {
     if ( existingListener->isHTMLEventListener() != html )
        // The existingListener will have the wrong type, so onclick= will behave like addEventListener or vice versa.
        kdWarning() << "getJSEventListener: event listener already found but with html=" << !html << " - please report this, we thought it would never happen" << endl;
    return existingListener;
  }

  // Note that the JSEventListener constructor adds it to our jsEventListeners list
  return new JSEventListener(listenerObject, listenerObjectImp, Object(this), html);
}

JSLazyEventListener *Window::getJSLazyEventListener(const TQString& code, const TQString& name, DOM::NodeImpl *node)
{
  return new JSLazyEventListener(code, name, Object(this), node);
}

void Window::clear( ExecState *exec )
{
  delete winq;
  winq = 0L;
  // Get rid of everything, those user vars could hold references to DOM nodes
  deleteAllProperties( exec );

  // Break the dependency between the listeners and their object
  TQPtrDictIterator<JSEventListener> it(jsEventListeners);
  for (; it.current(); ++it)
    it.current()->clear();
  // Forget about the listeners (the DOM::NodeImpls will delete them)
  jsEventListeners.clear();

  if (m_frame) {
    KJSProxy* proxy = m_frame->m_jscript;
    if (proxy) // i.e. JS not disabled
    {
      winq = new WindowQObject(this);
      // Now recreate a working global object for the next URL that will use us
      KJS::Interpreter *interpreter = proxy->interpreter();
      interpreter->initGlobalObject();
    }
  }
}

void Window::setCurrentEvent( DOM::Event *evt )
{
  m_evt = evt;
  //kdDebug(6070) << "Window " << this << " (part=" << m_part << ")::setCurrentEvent m_evt=" << evt << endl;
}

void Window::goURL(ExecState* exec, const TQString& url, bool lockHistory)
{
  Window* active = Window::retrieveActive(exec);
  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);
  TDEHTMLPart *active_part = ::tqt_cast<TDEHTMLPart *>(active->part());
  // Complete the URL using the "active part" (running interpreter)
  if (active_part && part) {
    if (url[0] == TQChar('#')) {
      part->gotoAnchor(url.mid(1));
    } else {
      TQString dstUrl = active_part->htmlDocument().completeURL(url).string();
      kdDebug(6070) << "Window::goURL dstUrl=" << dstUrl << endl;

      // check if we're allowed to inject javascript
      // SYNC check with tdehtml_part.cpp::slotRedirect!
      if ( isSafeScript(exec) ||
            dstUrl.find(TQString::fromLatin1("javascript:"), 0, false) != 0 )
        part->scheduleRedirection(-1,
                                  dstUrl,
                                  lockHistory);
    }
  } else if (!part && !m_frame->m_part.isNull()) {
    KParts::BrowserExtension *b = KParts::BrowserExtension::childObject(m_frame->m_part);
    if (b)
      b->emit openURLRequest(m_frame->m_frame->element()->getDocument()->completeURL(url));
    kdDebug() << "goURL for ROPart" << endl;
  }
}

KParts::ReadOnlyPart *Window::part() const {
    return m_frame.isNull() ? 0L : static_cast<KParts::ReadOnlyPart *>(m_frame->m_part);
}

void Window::delayedGoHistory( int steps )
{
    m_delayed.append( DelayedAction( DelayedGoHistory, steps ) );
}

void Window::goHistory( int steps )
{
  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);
  if(!part)
      // TODO history readonlypart
    return;
  KParts::BrowserExtension *ext = part->browserExtension();
  if(!ext)
    return;
  KParts::BrowserInterface *iface = ext->browserInterface();

  if ( !iface )
    return;

  iface->callMethod( "goHistory(int)", steps );
  //emit ext->goHistory(steps);
}

void KJS::Window::resizeTo(TQWidget* tl, int width, int height)
{
  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);
  if(!part)
      // TODO resizeTo readonlypart
    return;
  KParts::BrowserExtension *ext = part->browserExtension();
  if (!ext) {
    kdDebug(6070) << "Window::resizeTo found no browserExtension" << endl;
    return;
  }

  // Security check: within desktop limits and bigger than 100x100 (per spec)
  if ( width < 100 || height < 100 ) {
    kdDebug(6070) << "Window::resizeTo refused, window would be too small ("<<width<<","<<height<<")" << endl;
    return;
  }

  TQRect sg = TDEGlobalSettings::desktopGeometry(tl);

  if ( width > sg.width() || height > sg.height() ) {
    kdDebug(6070) << "Window::resizeTo refused, window would be too big ("<<width<<","<<height<<")" << endl;
    return;
  }

  kdDebug(6070) << "resizing to " << width << "x" << height << endl;

  emit ext->resizeTopLevelWidget( width, height );

  // If the window is out of the desktop, move it up/left
  // (maybe we should use workarea instead of sg, otherwise the window ends up below kicker)
  int right = tl->x() + tl->frameGeometry().width();
  int bottom = tl->y() + tl->frameGeometry().height();
  int moveByX = 0;
  int moveByY = 0;
  if ( right > sg.right() )
    moveByX = - right + sg.right(); // always <0
  if ( bottom > sg.bottom() )
    moveByY = - bottom + sg.bottom(); // always <0
  if ( moveByX || moveByY )
    emit ext->moveTopLevelWidget( tl->x() + moveByX , tl->y() + moveByY );
}

Value Window::openWindow(ExecState *exec, const List& args)
{
  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);
  if (!part)
    return Undefined();
  TDEHTMLView *widget = part->view();
  Value v = args[0];
  TQString str;
  if (v.isValid() && !v.isA(UndefinedType))
    str = v.toString(exec).qstring();

  // prepare arguments
  KURL url;
  if (!str.isEmpty())
  {
    TDEHTMLPart* p = ::tqt_cast<TDEHTMLPart *>(Window::retrieveActive(exec)->m_frame->m_part);
    if ( p )
      url = p->htmlDocument().completeURL(str).string();
    if ( !p ||
         !static_cast<DOM::DocumentImpl*>(p->htmlDocument().handle())->isURLAllowed(url.url()) )
      return Undefined();
  }

  TDEHTMLSettings::KJSWindowOpenPolicy policy =
		part->settings()->windowOpenPolicy(part->url().host());
  if ( policy == TDEHTMLSettings::KJSWindowOpenAsk ) {
    emit part->browserExtension()->requestFocus(part);
    TQString caption;
    if (!part->url().host().isEmpty())
      caption = part->url().host() + " - ";
    caption += i18n( "Confirmation: JavaScript Popup" );
    if ( KMessageBox::questionYesNo(widget,
                                    str.isEmpty() ?
                                    i18n( "This site is requesting to open up a new browser "
                                          "window via JavaScript.\n"
                                          "Do you want to allow this?" ) :
                                    i18n( "<qt>This site is requesting to open<p>%1</p>in a new browser window via JavaScript.<br />"
                                          "Do you want to allow this?</qt>").arg(KStringHandler::csqueeze(url.htmlURL(),  100)),
                                    caption, i18n("Allow"), i18n("Do Not Allow") ) == KMessageBox::Yes )
      policy = TDEHTMLSettings::KJSWindowOpenAllow;
  } else if ( policy == TDEHTMLSettings::KJSWindowOpenSmart )
  {
    // window.open disabled unless from a key/mouse event
    if (static_cast<ScriptInterpreter *>(exec->interpreter())->isWindowOpenAllowed())
      policy = TDEHTMLSettings::KJSWindowOpenAllow;
  }

  TQString frameName = args.size() > 1 ? args[1].toString(exec).qstring() : TQString("_blank");

  v = args[2];
  TQString features;
  if (!v.isNull() && v.type() != UndefinedType && v.toString(exec).size() > 0) {
    features = v.toString(exec).qstring();
    // Buggy scripts have ' at beginning and end, cut those
    if (features.startsWith("\'") && features.endsWith("\'"))
      features = features.mid(1, features.length()-2);
  }

  if ( policy != TDEHTMLSettings::KJSWindowOpenAllow ) {
    if ( url.isEmpty() )
      part->setSuppressedPopupIndicator(true, 0);
    else {
      part->setSuppressedPopupIndicator(true, part);
      m_suppressedWindowInfo.append( SuppressedWindowInfo( url, frameName, features ) );
    }
    return Undefined();
  } else {
    return executeOpenWindow(exec, url, frameName, features);
  }
}

Value Window::executeOpenWindow(ExecState *exec, const KURL& url, const TQString& frameName, const TQString& features)
{
    TDEHTMLPart *p = ::tqt_cast<TDEHTMLPart *>(m_frame->m_part);
    TDEHTMLView *widget = p->view();
    KParts::WindowArgs winargs;

    // scan feature argument
    if (!features.isEmpty()) {
      // specifying window params means false defaults
      winargs.menuBarVisible = false;
      winargs.toolBarsVisible = false;
      winargs.statusBarVisible = false;
      winargs.scrollBarsVisible = false;
      TQStringList flist = TQStringList::split(',', features);
      TQStringList::ConstIterator it = flist.begin();
      while (it != flist.end()) {
        TQString s = *it++;
        TQString key, val;
        int pos = s.find('=');
        if (pos >= 0) {
          key = s.left(pos).stripWhiteSpace().lower();
          val = s.mid(pos + 1).stripWhiteSpace().lower();
          TQRect screen = TDEGlobalSettings::desktopGeometry(widget->topLevelWidget());

          if (key == "left" || key == "screenx") {
            winargs.x = (int)val.toFloat() + screen.x();
            if (winargs.x < screen.x() || winargs.x > screen.right())
              winargs.x = screen.x(); // only safe choice until size is determined
          } else if (key == "top" || key == "screeny") {
            winargs.y = (int)val.toFloat() + screen.y();
            if (winargs.y < screen.y() || winargs.y > screen.bottom())
              winargs.y = screen.y(); // only safe choice until size is determined
          } else if (key == "height") {
            winargs.height = (int)val.toFloat() + 2*tqApp->style().pixelMetric( TQStyle::PM_DefaultFrameWidth ) + 2;
            if (winargs.height > screen.height())  // should actually check workspace
              winargs.height = screen.height();
            if (winargs.height < 100)
              winargs.height = 100;
          } else if (key == "width") {
            winargs.width = (int)val.toFloat() + 2*tqApp->style().pixelMetric( TQStyle::PM_DefaultFrameWidth ) + 2;
            if (winargs.width > screen.width())    // should actually check workspace
              winargs.width = screen.width();
            if (winargs.width < 100)
              winargs.width = 100;
          } else {
            goto boolargs;
          }
          continue;
        } else {
          // leaving away the value gives true
          key = s.stripWhiteSpace().lower();
          val = "1";
        }
      boolargs:
        if (key == "menubar")
          winargs.menuBarVisible = (val == "1" || val == "yes");
        else if (key == "toolbar")
          winargs.toolBarsVisible = (val == "1" || val == "yes");
        else if (key == "location")  // ### missing in WindowArgs
          winargs.toolBarsVisible = (val == "1" || val == "yes");
        else if (key == "status" || key == "statusbar")
          winargs.statusBarVisible = (val == "1" || val == "yes");
        else if (key == "scrollbars")
          winargs.scrollBarsVisible = (val == "1" || val == "yes");
        else if (key == "resizable")
          winargs.resizable = (val == "1" || val == "yes");
        else if (key == "fullscreen")
          winargs.fullscreen = (val == "1" || val == "yes");
      }
    }

    KParts::URLArgs uargs;
    uargs.frameName = frameName;

    if ( uargs.frameName.lower() == "_top" )
    {
      while ( p->parentPart() )
        p = p->parentPart();
      Window::retrieveWindow(p)->goURL(exec, url.url(), false /*don't lock history*/);
      return Window::retrieve(p);
    }
    if ( uargs.frameName.lower() == "_parent" )
    {
      if ( p->parentPart() )
        p = p->parentPart();
      Window::retrieveWindow(p)->goURL(exec, url.url(), false /*don't lock history*/);
      return Window::retrieve(p);
    }
    if ( uargs.frameName.lower() == "_self")
    {
      Window::retrieveWindow(p)->goURL(exec, url.url(), false /*don't lock history*/);
      return Window::retrieve(p);
    }
    if ( uargs.frameName.lower() == "replace" )
    {
      Window::retrieveWindow(p)->goURL(exec, url.url(), true /*lock history*/);
      return Window::retrieve(p);
    }
    uargs.serviceType = "text/html";

    // request window (new or existing if framename is set)
    KParts::ReadOnlyPart *newPart = 0L;
    emit p->browserExtension()->createNewWindow(KURL(), uargs,winargs,newPart);
    if (newPart && ::tqt_cast<TDEHTMLPart*>(newPart)) {
      TDEHTMLPart *tdehtmlpart = static_cast<TDEHTMLPart*>(newPart);
      //tqDebug("opener set to %p (this Window's part) in new Window %p  (this Window=%p)",part,win,window);
      tdehtmlpart->setOpener(p);
      tdehtmlpart->setOpenedByJS(true);
      if (tdehtmlpart->document().isNull()) {
        tdehtmlpart->begin();
        tdehtmlpart->write("<HTML><BODY>");
        tdehtmlpart->end();
        if ( p->docImpl() ) {
          //kdDebug(6070) << "Setting domain to " << p->docImpl()->domain().string() << endl;
          tdehtmlpart->docImpl()->setDomain( p->docImpl()->domain());
          tdehtmlpart->docImpl()->setBaseURL( p->docImpl()->baseURL() );
        }
      }
      uargs.serviceType = TQString::null;
      if (uargs.frameName.lower() == "_blank")
        uargs.frameName = TQString::null;
      if (!url.isEmpty())
        emit tdehtmlpart->browserExtension()->openURLRequest(url,uargs);
      return Window::retrieve(tdehtmlpart); // global object
    } else
      return Undefined();
}

void Window::forgetSuppressedWindows()
{
  m_suppressedWindowInfo.clear();
}

void Window::showSuppressedWindows()
{
  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>( m_frame->m_part );
  KJS::Interpreter *interpreter = part->jScript()->interpreter();
  ExecState *exec = interpreter->globalExec();

  TQValueList<SuppressedWindowInfo> suppressedWindowInfo = m_suppressedWindowInfo;
  m_suppressedWindowInfo.clear();
  TQValueList<SuppressedWindowInfo>::Iterator it = suppressedWindowInfo.begin();
  for ( ; it != suppressedWindowInfo.end() ; ++it ) {
    executeOpenWindow(exec, (*it).url, (*it).frameName, (*it).features);
  }
}

Value WindowFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( Window, thisObj );

  // these should work no matter whether the window is already
  // closed or not
  if (id == Window::ValueOf || id == Window::ToString) {
    return String("[object Window]");
  }

  Window *window = static_cast<Window *>(thisObj.imp());
  TQString str, str2;

  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(window->m_frame->m_part);
  if (!part)
    return Undefined();

  TDEHTMLView *widget = part->view();
  Value v = args[0];
  UString s;
  if (v.isValid() && !v.isA(UndefinedType)) {
    s = v.toString(exec);
    str = s.qstring();
  }

  TQString caption;
  if (part && !part->url().host().isEmpty())
    caption = part->url().host() + " - ";
  caption += "JavaScript"; // TODO: i18n
  // functions that work everywhere
  switch(id) {
  case Window::Alert:
    if (!widget->dialogsAllowed())
      return Undefined();
    if ( part && part->xmlDocImpl() )
      part->xmlDocImpl()->updateRendering();
    if ( part )
      emit part->browserExtension()->requestFocus(part);
    KMessageBox::error(widget, TQStyleSheet::convertFromPlainText(str, TQStyleSheetItem::WhiteSpaceNormal), caption);
    return Undefined();
  case Window::Confirm:
    if (!widget->dialogsAllowed())
      return Undefined();
    if ( part && part->xmlDocImpl() )
      part->xmlDocImpl()->updateRendering();
    if ( part )
      emit part->browserExtension()->requestFocus(part);
    return Boolean((KMessageBox::warningYesNo(widget, TQStyleSheet::convertFromPlainText(str), caption,
                                                KStdGuiItem::ok(), KStdGuiItem::cancel()) == KMessageBox::Yes));
  case Window::Prompt:
#ifndef KONQ_EMBEDDED
    if (!widget->dialogsAllowed())
      return Undefined();
    if ( part && part->xmlDocImpl() )
      part->xmlDocImpl()->updateRendering();
    if ( part )
      emit part->browserExtension()->requestFocus(part);
    bool ok;
    if (args.size() >= 2)
      str2 = KInputDialog::getText(caption,
                                   TQStyleSheet::convertFromPlainText(str),
                                   args[1].toString(exec).qstring(), &ok, widget);
    else
      str2 = KInputDialog::getText(caption,
                                   TQStyleSheet::convertFromPlainText(str),
                                   TQString::null, &ok, widget);
    if ( ok )
        return String(str2);
    else
        return Null();
#else
    return Undefined();
#endif
  case Window::GetComputedStyle:  {
       if ( !part || !part->xmlDocImpl() )
         return Undefined();
        DOM::Node arg0 = toNode(args[0]);
        if (arg0.nodeType() != DOM::Node::ELEMENT_NODE)
          return Undefined(); // throw exception?
        else
          return getDOMCSSStyleDeclaration(exec, part->document().defaultView().getComputedStyle(static_cast<DOM::Element>(arg0),
                                                                              args[1].toString(exec).string()));
      }
  case Window::Open:
    return window->openWindow(exec, args);
  case Window::Close: {
    /* From http://developer.netscape.com/docs/manuals/js/client/jsref/window.htm :
       The close method closes only windows opened by JavaScript using the open method.
       If you attempt to close any other window, a confirm is generated, which
       lets the user choose whether the window closes.
       This is a security feature to prevent "mail bombs" containing self.close().
       However, if the window has only one document (the current one) in its
       session history, the close is allowed without any confirm. This is a
       special case for one-off windows that need to open other windows and
       then dispose of themselves.
    */
    bool doClose = false;
    if (!part->openedByJS())
    {
      // To conform to the SPEC, we only ask if the window
      // has more than one entry in the history (NS does that too).
      History history(exec,part);

      if ( history.get( exec, "length" ).toInt32(exec) <= 1 )
      {
        doClose = true;
      }
      else
      {
        // Can we get this dialog with tabs??? Does it close the window or the tab in that case?
        emit part->browserExtension()->requestFocus(part);
        if ( KMessageBox::questionYesNo( window->part()->widget(),
                                         i18n("Close window?"), i18n("Confirmation Required"),
                                         KStdGuiItem::close(), KStdGuiItem::cancel() )
             == KMessageBox::Yes )
          doClose = true;
      }
    }
    else
      doClose = true;

    if (doClose)
    {
      // If this is the current window (the one the interpreter runs in),
      // then schedule a delayed close (so that the script terminates first).
      // But otherwise, close immediately. This fixes w=window.open("","name");w.close();window.open("name");
      if ( Window::retrieveActive(exec) == window ) {
        if (widget) {
          // quit all dialogs of this view
          // this fixes 'setTimeout('self.close()',1000); alert("Hi");' crash
          widget->closeChildDialogs();
        }
        //kdDebug() << "scheduling delayed close"  << endl;
        // We'll close the window at the end of the script execution
        Window* w = const_cast<Window*>(window);
        w->m_delayed.append( Window::DelayedAction( Window::DelayedClose ) );
      } else {
        //kdDebug() << "closing NOW"  << endl;
        (const_cast<Window*>(window))->closeNow();
      }
    }
    return Undefined();
  }
  case Window::Navigate:
    window->goURL(exec, args[0].toString(exec).qstring(), false /*don't lock history*/);
    return Undefined();
  case Window::Focus: {
    TDEHTMLSettings::KJSWindowFocusPolicy policy =
		part->settings()->windowFocusPolicy(part->url().host());
    if(policy == TDEHTMLSettings::KJSWindowFocusAllow && widget) {
      widget->topLevelWidget()->raise();
      KWin::deIconifyWindow( widget->topLevelWidget()->winId() );
      widget->setActiveWindow();
      emit part->browserExtension()->requestFocus(part);
    }
    return Undefined();
  }
  case Window::Blur:
    // TODO
    return Undefined();
  case Window::BToA:
  case Window::AToB: {
      if (!s.is8Bit())
          return Undefined();
       TQByteArray  in, out;
       char *binData = s.ascii();
       in.setRawData( binData, s.size() );
       if (id == Window::AToB)
           KCodecs::base64Decode( in, out );
       else
           KCodecs::base64Encode( in, out );
       in.resetRawData( binData, s.size() );
       UChar *d = new UChar[out.size()];
       for (uint i = 0; i < out.size(); i++)
           d[i].uc = (uchar) out[i];
       UString ret(d, out.size(), false /*no copy*/);
       return String(ret);
  }

  };


  // now unsafe functions..
  if (!window->isSafeScript(exec))
    return Undefined();

  switch (id) {
  case Window::ScrollBy:
    if(args.size() == 2 && widget)
      widget->scrollBy(args[0].toInt32(exec), args[1].toInt32(exec));
    return Undefined();
  case Window::Scroll:
  case Window::ScrollTo:
    if(args.size() == 2 && widget)
      widget->setContentsPos(args[0].toInt32(exec), args[1].toInt32(exec));
    return Undefined();
  case Window::MoveBy: {
    TDEHTMLSettings::KJSWindowMovePolicy policy =
		part->settings()->windowMovePolicy(part->url().host());
    if(policy == TDEHTMLSettings::KJSWindowMoveAllow && args.size() == 2 && widget)
    {
      KParts::BrowserExtension *ext = part->browserExtension();
      if (ext) {
        TQWidget * tl = widget->topLevelWidget();
        TQRect sg = TDEGlobalSettings::desktopGeometry(tl);

        TQPoint dest = tl->pos() + TQPoint( args[0].toInt32(exec), args[1].toInt32(exec) );
        // Security check (the spec talks about UniversalBrowserWrite to disable this check...)
        if ( dest.x() >= sg.x() && dest.y() >= sg.x() &&
             dest.x()+tl->width() <= sg.width()+sg.x() &&
             dest.y()+tl->height() <= sg.height()+sg.y() )
          emit ext->moveTopLevelWidget( dest.x(), dest.y() );
      }
    }
    return Undefined();
  }
  case Window::MoveTo: {
    TDEHTMLSettings::KJSWindowMovePolicy policy =
		part->settings()->windowMovePolicy(part->url().host());
    if(policy == TDEHTMLSettings::KJSWindowMoveAllow && args.size() == 2 && widget)
    {
      KParts::BrowserExtension *ext = part->browserExtension();
      if (ext) {
        TQWidget * tl = widget->topLevelWidget();
        TQRect sg = TDEGlobalSettings::desktopGeometry(tl);

        TQPoint dest( args[0].toInt32(exec)+sg.x(), args[1].toInt32(exec)+sg.y() );
        // Security check (the spec talks about UniversalBrowserWrite to disable this check...)
        if ( dest.x() >= sg.x() && dest.y() >= sg.y() &&
             dest.x()+tl->width() <= sg.width()+sg.x() &&
             dest.y()+tl->height() <= sg.height()+sg.y() )
		emit ext->moveTopLevelWidget( dest.x(), dest.y() );
      }
    }
    return Undefined();
  }
  case Window::ResizeBy: {
    TDEHTMLSettings::KJSWindowResizePolicy policy =
		part->settings()->windowResizePolicy(part->url().host());
    if(policy == TDEHTMLSettings::KJSWindowResizeAllow
    		&& args.size() == 2 && widget)
    {
      TQWidget * tl = widget->topLevelWidget();
      TQRect geom = tl->frameGeometry();
      window->resizeTo( tl,
                        geom.width() + args[0].toInt32(exec),
                        geom.height() + args[1].toInt32(exec) );
    }
    return Undefined();
  }
  case Window::ResizeTo: {
    TDEHTMLSettings::KJSWindowResizePolicy policy =
               part->settings()->windowResizePolicy(part->url().host());
    if(policy == TDEHTMLSettings::KJSWindowResizeAllow
               && args.size() == 2 && widget)
    {
      TQWidget * tl = widget->topLevelWidget();
      window->resizeTo( tl, args[0].toInt32(exec), args[1].toInt32(exec) );
    }
    return Undefined();
  }
  case Window::SetTimeout:
  case Window::SetInterval: {
    bool singleShot;
    int i; // timeout interval
    if (args.size() == 0)
      return Undefined();
    if (args.size() > 1) {
      singleShot = (id == Window::SetTimeout);
      i = args[1].toInt32(exec);
    } else {
      // second parameter is missing. Emulate Mozilla behavior.
      singleShot = true;
      i = 4;
    }
    if (v.isA(StringType)) {
      int r = (const_cast<Window*>(window))->winq->installTimeout(Identifier(s), i, singleShot );
      return Number(r);
    }
    else if (v.isA(ObjectType) && Object::dynamicCast(v).implementsCall()) {
      Object func = Object::dynamicCast(v);
      List funcArgs;
      ListIterator it = args.begin();
      int argno = 0;
      while (it != args.end()) {
	Value arg = it++;
	if (argno++ >= 2)
	    funcArgs.append(arg);
      }
      if (args.size() < 2)
	funcArgs.append(Number(i));
      int r = (const_cast<Window*>(window))->winq->installTimeout(func, funcArgs, i, singleShot );
      return Number(r);
    }
    else
      return Undefined();
  }
  case Window::ClearTimeout:
  case Window::ClearInterval:
    (const_cast<Window*>(window))->winq->clearTimeout(v.toInt32(exec));
    return Undefined();
  case Window::Print:
    if ( widget ) {
      // ### TODO emit onbeforeprint event
      widget->print();
      // ### TODO emit onafterprint event
    }
  case Window::CaptureEvents:
  case Window::ReleaseEvents:
    // Do nothing for now. These are NS-specific legacy calls.
    break;
  case Window::AddEventListener: {
        JSEventListener *listener = Window::retrieveActive(exec)->getJSEventListener(args[1]);
        if (listener) {
	    DOM::DocumentImpl* docimpl = static_cast<DOM::DocumentImpl *>(part->document().handle());
            docimpl->addWindowEventListener(DOM::EventImpl::typeToId(args[0].toString(exec).string()),listener,args[2].toBoolean(exec));
        }
        return Undefined();
    }
  case Window::RemoveEventListener: {
        JSEventListener *listener = Window::retrieveActive(exec)->getJSEventListener(args[1]);
        if (listener) {
	    DOM::DocumentImpl* docimpl = static_cast<DOM::DocumentImpl *>(part->document().handle());
            docimpl->removeWindowEventListener(DOM::EventImpl::typeToId(args[0].toString(exec).string()),listener,args[2].toBoolean(exec));
        }
        return Undefined();
    }

  }
  return Undefined();
}

////////////////////// ScheduledAction ////////////////////////

// KDE 4: Make those parameters const ... &
ScheduledAction::ScheduledAction(Object _func, List _args, DateTimeMS _nextTime, int _interval, bool _singleShot,
				  int _timerId)
{
  //kdDebug(6070) << "ScheduledAction::ScheduledAction(isFunction) " << this << endl;
  func = static_cast<ObjectImp*>(_func.imp());
  args = _args;
  isFunction = true;
  singleShot = _singleShot;
  nextTime = _nextTime;
  interval = _interval;
  executing = false;
  timerId = _timerId;
}

// KDE 4: Make it const TQString &
ScheduledAction::ScheduledAction(TQString _code, DateTimeMS _nextTime, int _interval, bool _singleShot, int _timerId)
{
  //kdDebug(6070) << "ScheduledAction::ScheduledAction(!isFunction) " << this << endl;
  //func = 0;
  //args = 0;
  func = 0;
  code = _code;
  isFunction = false;
  singleShot = _singleShot;
  nextTime = _nextTime;
  interval = _interval;
  executing = false;
  timerId = _timerId;
}

bool ScheduledAction::execute(Window *window)
{
  TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart *>(window->m_frame->m_part);
  if (!part || !part->jScriptEnabled())
    return false;
  ScriptInterpreter *interpreter = static_cast<ScriptInterpreter *>(part->jScript()->interpreter());

  interpreter->setProcessingTimerCallback(true);

  //kdDebug(6070) << "ScheduledAction::execute " << this << endl;
  if (isFunction) {
    if (func->implementsCall()) {
      // #### check this
      Q_ASSERT( part );
      if ( part )
      {
        KJS::Interpreter *interpreter = part->jScript()->interpreter();
        ExecState *exec = interpreter->globalExec();
        Q_ASSERT( window == interpreter->globalObject().imp() );
        Object obj( window );
        func->call(exec,obj,args); // note that call() creates its own execution state for the func call
        if (exec->hadException())
          exec->clearException();

        // Update our document's rendering following the execution of the timeout callback.
        part->document().updateRendering();
      }
    }
  }
  else {
    part->executeScript(DOM::Node(), code);
  }

  interpreter->setProcessingTimerCallback(false);
  return true;
}

void ScheduledAction::mark()
{
  if (func && !func->marked())
    func->mark();
  args.mark();
}

ScheduledAction::~ScheduledAction()
{
  //kdDebug(6070) << "ScheduledAction::~ScheduledAction " << this << endl;
}

////////////////////// WindowQObject ////////////////////////

WindowQObject::WindowQObject(Window *w)
  : parent(w)
{
  //kdDebug(6070) << "WindowQObject::WindowQObject " << this << endl;
  if ( !parent->m_frame )
      kdDebug(6070) << "WARNING: null part in " << k_funcinfo << endl;
  else
      connect( parent->m_frame, TQ_SIGNAL( destroyed() ),
               this, TQ_SLOT( parentDestroyed() ) );
  pausedTime = 0;
  lastTimerId = 0;
  currentlyDispatching = false;
}

WindowQObject::~WindowQObject()
{
  //kdDebug(6070) << "WindowQObject::~WindowQObject " << this << endl;
  parentDestroyed(); // reuse same code
}

void WindowQObject::parentDestroyed()
{
  killTimers();

  TQPtrListIterator<ScheduledAction> it(scheduledActions);
  for (; it.current(); ++it)
    delete it.current();
  scheduledActions.clear();
}

int WindowQObject::installTimeout(const Identifier &handler, int t, bool singleShot)
{
  int id = ++lastTimerId;
  if (t < 10) t = 10;
  DateTimeMS nextTime = DateTimeMS::now().addMSecs(-pausedTime + t);
  
  ScheduledAction *action = new ScheduledAction(handler.qstring(),nextTime,t,singleShot,id);
  scheduledActions.append(action);
  setNextTimer();
  return id;
}

int WindowQObject::installTimeout(const Value &func, List args, int t, bool singleShot)
{
  Object objFunc = Object::dynamicCast( func );
  if (!objFunc.isValid())
    return 0;
  int id = ++lastTimerId;
  if (t < 10) t = 10;
  
  DateTimeMS nextTime = DateTimeMS::now().addMSecs(-pausedTime + t);
  ScheduledAction *action = new ScheduledAction(objFunc,args,nextTime,t,singleShot,id);
  scheduledActions.append(action);
  setNextTimer();
  return id;
}

void WindowQObject::clearTimeout(int timerId)
{
  TQPtrListIterator<ScheduledAction> it(scheduledActions);
  for (; it.current(); ++it) {
    ScheduledAction *action = it.current();
    if (action->timerId == timerId) {
      scheduledActions.removeRef(action);
      if (!action->executing)
	delete action;
      return;
    }
  }
}

bool WindowQObject::hasTimers() const
{
  return scheduledActions.count();
}

void WindowQObject::mark()
{
  TQPtrListIterator<ScheduledAction> it(scheduledActions);
  for (; it.current(); ++it)
    it.current()->mark();
}

void WindowQObject::timerEvent(TQTimerEvent *)
{
  killTimers();

  if (scheduledActions.isEmpty())
    return;

  currentlyDispatching = true;


  DateTimeMS currentActual = DateTimeMS::now();
  DateTimeMS currentAdjusted = currentActual.addMSecs(-pausedTime);

  // Work out which actions are to be executed. We take a separate copy of
  // this list since the main one may be modified during action execution
  TQPtrList<ScheduledAction> toExecute;
  TQPtrListIterator<ScheduledAction> it(scheduledActions);
  for (; it.current(); ++it)
    if (currentAdjusted >= it.current()->nextTime)
      toExecute.append(it.current());

  // ### verify that the window can't be closed (and action deleted) during execution
  it = TQPtrListIterator<ScheduledAction>(toExecute);
  for (; it.current(); ++it) {
    ScheduledAction *action = it.current();
    if (!scheduledActions.containsRef(action)) // removed by clearTimeout()
      continue;

    action->executing = true; // prevent deletion in clearTimeout()

    if (parent->part()) {
      bool ok = action->execute(parent);
      if ( !ok ) // e.g. JS disabled
        scheduledActions.removeRef( action );
    }

    if (action->singleShot) {
      scheduledActions.removeRef(action);
    }

    action->executing = false;

    if (!scheduledActions.containsRef(action))
      delete action;
    else
      action->nextTime = action->nextTime.addMSecs(action->interval);
  }

  pausedTime += currentActual.msecsTo(DateTimeMS::now());

  currentlyDispatching = false;

  // Work out when next event is to occur
  setNextTimer();
}

DateTimeMS DateTimeMS::addMSecs(int s) const
{
  DateTimeMS c = *this;
  c.mTime = mTime.addMSecs(s);
  if (s > 0)
  {
    if (c.mTime < mTime)
      c.mDate = mDate.addDays(1);
  }
  else
  {
    if (c.mTime > mTime)
      c.mDate = mDate.addDays(-1);
  }
  return c;
}

bool DateTimeMS::operator >(const DateTimeMS &other) const
{
  if (mDate > other.mDate)
    return true;

  if (mDate < other.mDate)
    return false;

  return mTime > other.mTime;
}

bool DateTimeMS::operator >=(const DateTimeMS &other) const
{
  if (mDate > other.mDate)
    return true;

  if (mDate < other.mDate)
    return false;

  return mTime >= other.mTime;
}

int DateTimeMS::msecsTo(const DateTimeMS &other) const
{
	int d = mDate.daysTo(other.mDate);
	int ms = mTime.msecsTo(other.mTime);
	return d*24*60*60*1000 + ms;
}


DateTimeMS DateTimeMS::now()
{
  DateTimeMS t;
  TQTime before = TQTime::currentTime();
  t.mDate = TQDate::currentDate();
  t.mTime = TQTime::currentTime();
  if (t.mTime < before)
    t.mDate = TQDate::currentDate(); // prevent race condition in hacky way :)
  return t;
}

void WindowQObject::setNextTimer()
{
  if (currentlyDispatching)
    return; // Will schedule at the end 

  if (scheduledActions.isEmpty())
    return;

  TQPtrListIterator<ScheduledAction> it(scheduledActions);
  DateTimeMS nextTime = it.current()->nextTime;
  for (++it; it.current(); ++it)
    if (nextTime > it.current()->nextTime)
      nextTime = it.current()->nextTime;

  DateTimeMS nextTimeActual = nextTime.addMSecs(pausedTime);
  int nextInterval = DateTimeMS::now().msecsTo(nextTimeActual);
  if (nextInterval < 0)
    nextInterval = 0;
  startTimer(nextInterval);
}

void WindowQObject::timeoutClose()
{
  parent->closeNow();
}

Value FrameArray::get(ExecState *exec, const Identifier &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "FrameArray::get " << p.qstring() << " part=" << (void*)part << endl;
#endif
  if (part.isNull())
    return Undefined();

  TQPtrList<KParts::ReadOnlyPart> frames = part->frames();
  unsigned int len = frames.count();
  if (p == lengthPropertyName)
    return Number(len);
  else if (p== "location") // non-standard property, but works in NS and IE
  {
    Object obj = Object::dynamicCast( Window::retrieve( part ) );
    if ( obj.isValid() )
      return obj.get( exec, "location" );
    return Undefined();
  }

  // check for the name or number
  KParts::ReadOnlyPart *frame = part->findFramePart(p.qstring());
  if (!frame) {
    bool ok;
    unsigned int i = p.toArrayIndex(&ok);
    if (ok && i < len)
      frame = frames.at(i);
  }

  // we are potentially fetching a reference to a another Window object here.
  // i.e. we may be accessing objects from another interpreter instance.
  // Therefore we have to be a bit careful with memory management.
  if (frame) {
    return Window::retrieve(frame);
  }

  // Fun IE quirk: name lookup in there is actually done by document.all 
  // hence, it can find non-frame things (and even let them hide frame ones!)
  // We don't quite do that, but do this as a fallback.
  DOM::DocumentImpl* doc  = static_cast<DOM::DocumentImpl*>(part->document().handle());
  if (doc) {
    DOM::HTMLCollectionImpl docuAll(doc, DOM::HTMLCollectionImpl::DOC_ALL);
    DOM::NodeImpl*     node = docuAll.namedItem(p.string());
    if (node) {
      if (node->id() == ID_FRAME || node->id() == ID_IFRAME) {
        //Return the Window object.
        TDEHTMLPart* part = static_cast<DOM::HTMLFrameElementImpl*>(node)->contentPart();
        if (part)
          return Value(Window::retrieveWindow(part));
        else
          return Undefined();
      } else {
        //Just a regular node..
        return getDOMNode(exec, node);
      }
    }
  } else {
    kdWarning(6070) << "Missing own document in FrameArray::get()" << endl;
  }

  return ObjectImp::get(exec, p);
}

Value FrameArray::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
    //IE supports a subset of the get functionality as call...
    //... basically, when the return is a window, it supports that, otherwise it 
    //errors out. We do a cheap-and-easy emulation of that, and just do the same
    //thing as get does.
    if (args.size() == 1)
        return get(exec, Identifier(args[0].toString(exec)));

    return Undefined();
}


////////////////////// Location Object ////////////////////////

const ClassInfo Location::info = { "Location", 0, &LocationTable, 0 };
/*
@begin LocationTable 11
  hash		Location::Hash		DontDelete
  host		Location::Host		DontDelete
  hostname	Location::Hostname	DontDelete
  href		Location::Href		DontDelete
  pathname	Location::Pathname	DontDelete
  port		Location::Port		DontDelete
  protocol	Location::Protocol	DontDelete
  search	Location::Search	DontDelete
  [[==]]	Location::EqualEqual	DontDelete|ReadOnly
  assign	Location::Assign	DontDelete|Function 1
  toString	Location::ToString	DontDelete|Function 0
  replace	Location::Replace	DontDelete|Function 1
  reload	Location::Reload	DontDelete|Function 0
@end
*/
IMPLEMENT_PROTOFUNC_DOM(LocationFunc)
Location::Location(tdehtml::ChildFrame *f) : m_frame(f)
{
  //kdDebug(6070) << "Location::Location " << this << " m_part=" << (void*)m_part << endl;
}

Location::~Location()
{
  //kdDebug(6070) << "Location::~Location " << this << " m_part=" << (void*)m_part << endl;
}

KParts::ReadOnlyPart *Location::part() const {
  return m_frame ? static_cast<KParts::ReadOnlyPart *>(m_frame->m_part) : 0L;
}

Value Location::get(ExecState *exec, const Identifier &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Location::get " << p.qstring() << " m_part=" << (void*)m_frame->m_part << endl;
#endif

  if (m_frame.isNull() || m_frame->m_part.isNull())
    return Undefined();

  const HashEntry *entry = Lookup::findEntry(&LocationTable, p);

  // properties that work on all Location objects
  if ( entry && entry->value == Replace )
      return lookupOrCreateFunction<LocationFunc>(exec,p,this,entry->value,entry->params,entry->attr);

  // XSS check
  const Window* window = Window::retrieveWindow( m_frame->m_part );
  if ( !window || !window->isSafeScript(exec) )
    return Undefined();

  KURL url = m_frame->m_part->url();
  if (entry)
    switch (entry->value) {
    case Hash:
      return String( url.ref().isNull() ? TQString("") : "#" + url.ref() );
    case Host: {
      UString str = url.host();
      if (url.port())
        str += ":" + TQString::number((int)url.port());
      return String(str);
      // Note: this is the IE spec. The NS spec swaps the two, it says
      // "The hostname property is the concatenation of the host and port properties, separated by a colon."
      // Bleh.
    }
    case Hostname:
      return String( url.host() );
    case Href:
      if (url.isEmpty())
	return String("about:blank");
      else if (!url.hasPath())
        return String( url.prettyURL()+"/" );
      else
        return String( url.prettyURL() );
    case Pathname:
      if (url.isEmpty())
	return String("");
      return String( url.path().isEmpty() ? TQString("/") : url.path() );
    case Port:
      return String( url.port() ? TQString::number((int)url.port()) : TQString::fromLatin1("") );
    case Protocol:
      return String( url.protocol()+":" );
    case Search:
      return String( url.query() );
    case EqualEqual: // [[==]]
      return String(toString(exec));
    case ToString:
      return lookupOrCreateFunction<LocationFunc>(exec,p,this,entry->value,entry->params,entry->attr);
    }
  // Look for overrides
  ValueImp * val = ObjectImp::getDirect(p);
  if (val)
    return Value(val);
  if (entry && (entry->attr & Function))
    return lookupOrCreateFunction<LocationFunc>(exec,p,this,entry->value,entry->params,entry->attr);

  return Undefined();
}

void Location::put(ExecState *exec, const Identifier &p, const Value &v, int attr)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Location::put " << p.qstring() << " m_part=" << (void*)m_frame->m_part << endl;
#endif
  if (m_frame.isNull() || m_frame->m_part.isNull())
    return;

  const Window* window = Window::retrieveWindow( m_frame->m_part );
  if ( !window )
    return;

  KURL url = m_frame->m_part->url();

  const HashEntry *entry = Lookup::findEntry(&LocationTable, p);

  if (entry) {

    // XSS check. Only new hrefs can be set from other sites
    if (entry->value != Href && !window->isSafeScript(exec))
      return;

    TQString str = v.toString(exec).qstring();
    switch (entry->value) {
    case Href: {
      TDEHTMLPart* p =::tqt_cast<TDEHTMLPart*>(Window::retrieveActive(exec)->part());
      if ( p )
        url = p->htmlDocument().completeURL( str ).string();
      else
        url = str;
      break;
    }
    case Hash:
      // when the hash is already the same ignore it
      if (str == url.ref()) return;
      url.setRef(str);
      break;
    case Host: {
      TQString host = str.left(str.find(":"));
      TQString port = str.mid(str.find(":")+1);
      url.setHost(host);
      url.setPort(port.toUInt());
      break;
    }
    case Hostname:
      url.setHost(str);
      break;
    case Pathname:
      url.setPath(str);
      break;
    case Port:
      url.setPort(str.toUInt());
      break;
    case Protocol:
      url.setProtocol(str);
      break;
    case Search:
      url.setQuery(str);
      break;
    }
  } else {
    ObjectImp::put(exec, p, v, attr);
    return;
  }

  Window::retrieveWindow(m_frame->m_part)->goURL(exec, url.url(), false /* don't lock history*/ );
}

Value Location::toPrimitive(ExecState *exec, Type) const
{
  if (m_frame) {
    Window* window = Window::retrieveWindow( m_frame->m_part );
    if ( window && window->isSafeScript(exec) )
      return String(toString(exec));
  }
  return Undefined();
}

UString Location::toString(ExecState *exec) const
{
  if (m_frame) {
    Window* window = Window::retrieveWindow( m_frame->m_part );
    if ( window && window->isSafeScript(exec) )
    {
      KURL url = m_frame->m_part->url();
      if (url.isEmpty())
	return "about:blank";
      else if (!url.hasPath())
        return url.prettyURL()+"/";
      else
        return url.prettyURL();
    }
  }
  return "";
}

Value LocationFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( Location, thisObj );
  Location *location = static_cast<Location *>(thisObj.imp());
  KParts::ReadOnlyPart *part = location->part();

  if (!part) return Undefined();

  Window* window = Window::retrieveWindow(part);

  if ( !window->isSafeScript(exec) && id != Location::Replace)
      return Undefined();

  switch (id) {
  case Location::Assign:
  case Location::Replace:
    Window::retrieveWindow(part)->goURL(exec, args[0].toString(exec).qstring(),
            id == Location::Replace);
    break;
  case Location::Reload: {
    TDEHTMLPart *tdehtmlpart = ::tqt_cast<TDEHTMLPart *>(part);
    if (tdehtmlpart)
      tdehtmlpart->scheduleRedirection(-1, part->url().url(), true/*lock history*/);
    else
      part->openURL(part->url());
    break;
  }
  case Location::ToString:
    return String(location->toString(exec));
  }
  return Undefined();
}

////////////////////// External Object ////////////////////////

const ClassInfo External::info = { "External", 0, 0, 0 };
/*
@begin ExternalTable 4
  addFavorite	External::AddFavorite	DontDelete|Function 1
@end
*/
IMPLEMENT_PROTOFUNC_DOM(ExternalFunc)

Value External::get(ExecState *exec, const Identifier &p) const
{
  return lookupGetFunction<ExternalFunc,ObjectImp>(exec,p,&ExternalTable,this);
}

Value ExternalFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( External, thisObj );
  External *external = static_cast<External *>(thisObj.imp());

  TDEHTMLPart *part = external->part;
  if (!part)
    return Undefined();

  TDEHTMLView *widget = part->view();

  switch (id) {
  case External::AddFavorite:
  {
#ifndef KONQ_EMBEDDED
    if (!widget->dialogsAllowed())
      return Undefined();
    part->xmlDocImpl()->updateRendering();
    if (args.size() != 1 && args.size() != 2)
      return Undefined();

    TQString url = args[0].toString(exec).qstring();
    TQString title;
    if (args.size() == 2)
      title = args[1].toString(exec).qstring();

    // AK - don't do anything yet, for the moment i
    // just wanted the base js handling code in cvs
    return Undefined();

    TQString question;
    if ( title.isEmpty() )
      question = i18n("Do you want a bookmark pointing to the location \"%1\" to be added to your collection?")
                 .arg(url);
    else
      question = i18n("Do you want a bookmark pointing to the location \"%1\" titled \"%2\" to be added to your collection?")
                 .arg(url).arg(title);

    emit part->browserExtension()->requestFocus(part);

    TQString caption;
    if (!part->url().host().isEmpty())
       caption = part->url().host() + " - ";
    caption += i18n("JavaScript Attempted Bookmark Insert");

    if (KMessageBox::warningYesNo(
          widget, question, caption,
          i18n("Insert"), i18n("Disallow")) == KMessageBox::Yes)
    {
      KBookmarkManager *mgr = KBookmarkManager::userBookmarksManager();
      mgr->addBookmarkDialog(url,title);
    }
#else
    return Undefined();
#endif
    break;
  }
  default:
    return Undefined();
  }

  return Undefined();
}

////////////////////// History Object ////////////////////////

const ClassInfo History::info = { "History", 0, 0, 0 };
/*
@begin HistoryTable 4
  length	History::Length		DontDelete|ReadOnly
  back		History::Back		DontDelete|Function 0
  forward	History::Forward	DontDelete|Function 0
  go		History::Go		DontDelete|Function 1
@end
*/
IMPLEMENT_PROTOFUNC_DOM(HistoryFunc)

Value History::get(ExecState *exec, const Identifier &p) const
{
  return lookupGet<HistoryFunc,History,ObjectImp>(exec,p,&HistoryTable,this);
}

Value History::getValueProperty(ExecState *, int token) const
{
  // if previous or next is implemented, make sure its not a major
  // privacy leak (see i.e. http://security.greymagic.com/adv/gm005-op/)
  switch (token) {
  case Length:
  {
    if ( !part )
      return Number( 0 );

    KParts::BrowserExtension *ext = part->browserExtension();
    if ( !ext )
      return Number( 0 );

    KParts::BrowserInterface *iface = ext->browserInterface();
    if ( !iface )
      return Number( 0 );

    TQVariant length = iface->property( "historyLength" );

    if ( length.type() != TQVariant::UInt )
      return Number( 0 );

    return Number( length.toUInt() );
  }
  default:
    kdDebug(6070) << "WARNING: Unhandled token in History::getValueProperty : " << token << endl;
    return Undefined();
  }
}

Value HistoryFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( History, thisObj );
  History *history = static_cast<History *>(thisObj.imp());

  Value v = args[0];
  Number n;
  if(v.isValid())
    n = v.toInteger(exec);

  int steps;
  switch (id) {
  case History::Back:
    steps = -1;
    break;
  case History::Forward:
    steps = 1;
    break;
  case History::Go:
    steps = n.intValue();
    break;
  default:
    return Undefined();
  }

  // Special case for go(0) from a frame -> reload only the frame
  // go(i!=0) from a frame navigates into the history of the frame only,
  // in both IE and NS (but not in Mozilla).... we can't easily do that
  // in Konqueror...
  if (!steps) // add && history->part->parentPart() to get only frames, but doesn't matter
  {
    history->part->openURL( history->part->url() ); /// ## need args.reload=true?
  } else
  {
    // Delay it.
    // Testcase: history.back(); alert("hello");
    Window* window = Window::retrieveWindow( history->part );
    window->delayedGoHistory( steps );
  }
  return Undefined();
}

/////////////////////////////////////////////////////////////////////////////

#ifdef TQ_WS_QWS

const ClassInfo Konqueror::info = { "Konqueror", 0, 0, 0 };

bool Konqueror::hasProperty(ExecState *exec, const Identifier &p) const
{
  if ( p.qstring().startsWith( "goHistory" ) ) return false;

  return true;
}

Value Konqueror::get(ExecState *exec, const Identifier &p) const
{
  if ( p == "goHistory" || part->url().protocol() != "http" || part->url().host() != "localhost" )
    return Undefined();

  KParts::BrowserExtension *ext = part->browserExtension();
  if ( ext ) {
    KParts::BrowserInterface *iface = ext->browserInterface();
    if ( iface ) {
      TQVariant prop = iface->property( p.qstring().latin1() );

      if ( prop.isValid() ) {
        switch( prop.type() ) {
        case TQVariant::Int:
          return Number( prop.toInt() );
        case TQVariant::String:
          return String( prop.toString() );
        default:
          break;
        }
      }
    }
  }

  return Value( new KonquerorFunc(exec, this, p.qstring().latin1() ) );
}

Value KonquerorFunc::tryCall(ExecState *exec, Object &, const List &args)
{
  KParts::BrowserExtension *ext = konqueror->part->browserExtension();

  if (!ext)
    return Undefined();

  KParts::BrowserInterface *iface = ext->browserInterface();

  if ( !iface )
    return Undefined();

  TQCString n = m_name.data();
  n += "()";
  iface->callMethod( n.data(), TQVariant() );

  return Undefined();
}

UString Konqueror::toString(ExecState *) const
{
  return UString("[object Konqueror]");
}

#endif
/////////////////////////////////////////////////////////////////////////////
} //namespace KJS

#include "kjs_window.moc"
