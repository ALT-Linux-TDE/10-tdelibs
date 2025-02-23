/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001,2003 Peter Kelly (pmk@post.com)
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

#include "kjs_debugwin.h"
#include "kjs_proxy.h"

#ifdef KJS_DEBUGGER

#include <assert.h>
#include <stdlib.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqtextedit.h>
#include <tqlistbox.h>
#include <tqmultilineedit.h>
#include <tqapplication.h>
#include <tqsplitter.h>
#include <tqcombobox.h>
#include <tqbitmap.h>
#include <tqwidgetlist.h>
#include <tqlabel.h>
#include <tqdatastream.h>
#include <tqcstring.h>
#include <tqpainter.h>
#include <tqscrollbar.h>

#include <tdelocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <tdeglobal.h>
#include <tdemessagebox.h>
#include <kguiitem.h>
#include <tdepopupmenu.h>
#include <tdemenubar.h>
#include <tdeaction.h>
#include <tdeactioncollection.h>
#include <tdeglobalsettings.h>
#include <tdeshortcut.h>
#include <tdeconfig.h>
#include <tdeconfigbase.h>
#include <tdeapplication.h>
#include <dcop/dcopclient.h>
#include <kstringhandler.h> 

#include "kjs_dom.h"
#include "kjs_binding.h"
#include "tdehtml_part.h"
#include "tdehtmlview.h"
#include "tdehtml_pagecache.h"
#include "tdehtml_settings.h"
#include "tdehtml_factory.h"
#include "misc/decoder.h"
#include <kjs/ustring.h>
#include <kjs/object.h>
#include <kjs/function.h>
#include <kjs/interpreter.h>

using namespace KJS;
using namespace tdehtml;

SourceDisplay::SourceDisplay(KJSDebugWin *debugWin, TQWidget *parent, const char *name)
  : TQScrollView(parent,name), m_currentLine(-1), m_sourceFile(0), m_debugWin(debugWin),
    m_font(TDEGlobalSettings::fixedFont())
{
  verticalScrollBar()->setLineStep(TQFontMetrics(m_font).height());
  viewport()->setBackgroundMode(TQt::NoBackground);
  m_breakpointIcon = TDEGlobal::iconLoader()->loadIcon("process-stop",TDEIcon::Small);
}

SourceDisplay::~SourceDisplay()
{
  if (m_sourceFile) {
    m_sourceFile->deref();
    m_sourceFile = 0L;
  }
}

void SourceDisplay::setSource(SourceFile *sourceFile)
{
  if ( sourceFile )
      sourceFile->ref();
  if (m_sourceFile)
      m_sourceFile->deref();
  m_sourceFile = sourceFile;
  if ( m_sourceFile )
      m_sourceFile->ref();

  if (!m_sourceFile || !m_debugWin->isVisible()) {
    return;
  }

  TQString code = sourceFile->getCode();
  const TQChar *chars = code.unicode();
  uint len = code.length();
  TQChar newLine('\n');
  TQChar cr('\r');
  TQChar tab('\t');
  TQString tabstr("        ");
  TQString line;
  m_lines.clear();
  int width = 0;
  TQFontMetrics metrics(m_font);

  for (uint pos = 0; pos < len; pos++) {
    TQChar c = chars[pos];
    if (c == cr) {
      if (pos < len-1 && chars[pos+1] == newLine)
	continue;
      else
	c = newLine;
    }
    if (c == newLine) {
      m_lines.append(line);
      int lineWidth = metrics.width(line);
      if (lineWidth > width)
	width = lineWidth;
      line = "";
    }
    else if (c == tab) {
      line += tabstr;
    }
    else {
      line += c;
    }
  }
  if (line.length()) {
    m_lines.append(line);
    int lineWidth = metrics.width(line);
    if (lineWidth > width)
      width = lineWidth;
  }

  int linenoDisplayWidth = metrics.width("888888");
  resizeContents(linenoDisplayWidth+4+width,metrics.height()*m_lines.count());
  update();
  sourceFile->deref();
}

void SourceDisplay::setCurrentLine(int lineno, bool doCenter)
{
  m_currentLine = lineno;

  if (doCenter && m_currentLine >= 0) {
    TQFontMetrics metrics(m_font);
    int height = metrics.height();
    center(0,height*m_currentLine+height/2);
  }

  updateContents();
}

void SourceDisplay::contentsMousePressEvent(TQMouseEvent *e)
{
  TQScrollView::mouseDoubleClickEvent(e);
  TQFontMetrics metrics(m_font);
  int lineno = e->y()/metrics.height();
  emit lineDoubleClicked(lineno+1); // line numbers start from 1
}

void SourceDisplay::showEvent(TQShowEvent *)
{
    setSource(m_sourceFile);
}

void SourceDisplay::drawContents(TQPainter *p, int clipx, int clipy, int clipw, int cliph)
{
  if (!m_sourceFile) {
    p->fillRect(clipx,clipy,clipw,cliph,palette().active().base());
    return;
  }

  TQFontMetrics metrics(m_font);
  int height = metrics.height();

  int bottom = clipy + cliph;
  int right = clipx + clipw;

  int firstLine = clipy/height-1;
  if (firstLine < 0)
    firstLine = 0;
  int lastLine = bottom/height+2;
  if (lastLine > (int)m_lines.count())
    lastLine = m_lines.count();

  p->setFont(m_font);

  int linenoWidth = metrics.width("888888");

  for (int lineno = firstLine; lineno <= lastLine; lineno++) {
    TQString linenoStr = TQString().sprintf("%d",lineno+1);


    p->fillRect(0,height*lineno,linenoWidth,height,palette().active().mid());

    p->setPen(palette().active().text());
    p->drawText(0,height*lineno,linenoWidth,height,TQt::AlignRight,linenoStr);

    TQColor bgColor;
    TQColor textColor;

    if (lineno == m_currentLine) {
      bgColor = palette().active().highlight();
      textColor = palette().active().highlightedText();
    }
    else if (m_debugWin->haveBreakpoint(m_sourceFile,lineno+1,lineno+1)) {
      bgColor = palette().active().text();
      textColor = palette().active().base();
      p->drawPixmap(2,height*lineno+height/2-m_breakpointIcon.height()/2,m_breakpointIcon);
    }
    else {
      bgColor = palette().active().base();
      textColor = palette().active().text();
    }

    p->fillRect(linenoWidth,height*lineno,right-linenoWidth,height,bgColor);
    p->setPen(textColor);
    p->drawText(linenoWidth+4,height*lineno,contentsWidth()-linenoWidth-4,height,
		TQt::AlignLeft,m_lines[lineno]);
  }

  int remainingTop = height*(lastLine+1);
  p->fillRect(0,remainingTop,linenoWidth,bottom-remainingTop,palette().active().mid());

  p->fillRect(linenoWidth,remainingTop,
	      right-linenoWidth,bottom-remainingTop,palette().active().base());
}

//-------------------------------------------------------------------------

KJSDebugWin * KJSDebugWin::kjs_html_debugger = 0;

TQString SourceFile::getCode()
{
  if (interpreter) {
    TDEHTMLPart *part = ::tqt_cast<TDEHTMLPart*>(static_cast<ScriptInterpreter*>(interpreter)->part());
    if (part && url == part->url().url() && TDEHTMLPageCache::self()->isValid(part->cacheId())) {
      Decoder *decoder = part->createDecoder();
      TQByteArray data;
      TQDataStream stream(data,IO_WriteOnly);
      TDEHTMLPageCache::self()->saveData(part->cacheId(),&stream);
      TQString str;
      if (data.size() == 0)
	str = "";
      else
	str = decoder->decode(data.data(),data.size()) + decoder->flush();
      delete decoder;
      return str;
    }
  }

  return code;
}

//-------------------------------------------------------------------------

SourceFragment::SourceFragment(int sid, int bl, int el, SourceFile *sf)
{
  sourceId = sid;
  baseLine = bl;
  errorLine = el;
  sourceFile = sf;
  sourceFile->ref();
}

SourceFragment::~SourceFragment()
{
  sourceFile->deref();
  sourceFile = 0L;
}

//-------------------------------------------------------------------------

KJSErrorDialog::KJSErrorDialog(TQWidget *parent, const TQString& errorMessage, bool showDebug)
  : KDialogBase(parent,0,true,i18n("JavaScript Error"),
		showDebug ? KDialogBase::Ok|KDialogBase::User1 : KDialogBase::Ok,
		KDialogBase::Ok,false,KGuiItem("&Debug","gear"))
{
  TQWidget *page = new TQWidget(this);
  setMainWidget(page);

  TQLabel *iconLabel = new TQLabel("",page);
  iconLabel->setPixmap(TDEGlobal::iconLoader()->loadIcon("messagebox_critical",
						       TDEIcon::NoGroup,TDEIcon::SizeMedium,
						       TDEIcon::DefaultState,0,true));

  TQWidget *contents = new TQWidget(page);
  TQLabel *label = new TQLabel(errorMessage,contents);
  m_dontShowAgainCb = new TQCheckBox(i18n("&Do not show this message again"),contents);

  TQVBoxLayout *vl = new TQVBoxLayout(contents,0,spacingHint());
  vl->addWidget(label);
  vl->addWidget(m_dontShowAgainCb);

  TQHBoxLayout *topLayout = new TQHBoxLayout(page,0,spacingHint());
  topLayout->addWidget(iconLabel);
  topLayout->addWidget(contents);
  topLayout->addStretch(10);

  m_debugSelected = false;
}

KJSErrorDialog::~KJSErrorDialog()
{
}

void KJSErrorDialog::slotUser1()
{
  m_debugSelected = true;
  close();
}

//-------------------------------------------------------------------------
EvalMultiLineEdit::EvalMultiLineEdit(TQWidget *parent)
    : TQMultiLineEdit(parent) {
}

void EvalMultiLineEdit::keyPressEvent(TQKeyEvent * e)
{
    if (e->key() == TQt::Key_Return) {
        if (hasSelectedText()) {
            m_code = selectedText();
        } else {
            int para, index;
            getCursorPosition(&para, &index);
            m_code = text(para);
        }
        end();
    }
    TQMultiLineEdit::keyPressEvent(e);
}
//-------------------------------------------------------------------------
KJSDebugWin::KJSDebugWin(TQWidget *parent, const char *name)
  : TDEMainWindow(parent, name, (WFlags)WType_TopLevel), TDEInstance("kjs_debugger")
{
  m_breakpoints = 0;
  m_breakpointCount = 0;

  m_curSourceFile = 0;
  m_mode = Continue;
  m_nextSourceUrl = "";
  m_nextSourceBaseLine = 1;
  m_execs = 0;
  m_execsCount = 0;
  m_execsAlloc = 0;
  m_steppingDepth = 0;

  m_stopIcon = TDEGlobal::iconLoader()->loadIcon("process-stop",TDEIcon::Small);
  m_emptyIcon = TQPixmap(m_stopIcon.width(),m_stopIcon.height());
  TQBitmap emptyMask(m_stopIcon.width(),m_stopIcon.height(),true);
  m_emptyIcon.setMask(emptyMask);

  setCaption(i18n("JavaScript Debugger"));

  TQWidget *mainWidget = new TQWidget(this);
  setCentralWidget(mainWidget);

  TQVBoxLayout *vl = new TQVBoxLayout(mainWidget,5);

  // frame list & code
  TQSplitter *hsplitter = new TQSplitter(TQt::Vertical,mainWidget);
  TQSplitter *vsplitter = new TQSplitter(hsplitter);
  TQFont font(TDEGlobalSettings::fixedFont());

  TQWidget *contextContainer = new TQWidget(vsplitter);

  TQLabel *contextLabel = new TQLabel(i18n("Call stack"),contextContainer);
  TQWidget *contextListContainer = new TQWidget(contextContainer);
  m_contextList = new TQListBox(contextListContainer);
  m_contextList->setMinimumSize(100,200);
  connect(m_contextList,TQ_SIGNAL(highlighted(int)),this,TQ_SLOT(slotShowFrame(int)));

  TQHBoxLayout *clistLayout = new TQHBoxLayout(contextListContainer);
  clistLayout->addWidget(m_contextList);
  clistLayout->addSpacing(KDialog::spacingHint());

  TQVBoxLayout *contextLayout = new TQVBoxLayout(contextContainer);
  contextLayout->addWidget(contextLabel);
  contextLayout->addSpacing(KDialog::spacingHint());
  contextLayout->addWidget(contextListContainer);

  // source selection & display
  TQWidget *sourceSelDisplay = new TQWidget(vsplitter);
  TQVBoxLayout *ssdvl = new TQVBoxLayout(sourceSelDisplay);

  m_sourceSel = new TQComboBox(toolBar());
  connect(m_sourceSel,TQ_SIGNAL(activated(int)),this,TQ_SLOT(slotSourceSelected(int)));

  m_sourceDisplay = new SourceDisplay(this,sourceSelDisplay);
  ssdvl->addWidget(m_sourceDisplay);
  connect(m_sourceDisplay,TQ_SIGNAL(lineDoubleClicked(int)),TQ_SLOT(slotToggleBreakpoint(int)));

  TQValueList<int> vsplitSizes;
  vsplitSizes.insert(vsplitSizes.end(),120);
  vsplitSizes.insert(vsplitSizes.end(),480);
  vsplitter->setSizes(vsplitSizes);

  // evaluate

  TQWidget *evalContainer = new TQWidget(hsplitter);

  TQLabel *evalLabel = new TQLabel(i18n("JavaScript console"),evalContainer);
  m_evalEdit = new EvalMultiLineEdit(evalContainer);
  m_evalEdit->setWordWrap(TQMultiLineEdit::NoWrap);
  m_evalEdit->setFont(font);
  connect(m_evalEdit,TQ_SIGNAL(returnPressed()),TQ_SLOT(slotEval()));
  m_evalDepth = 0;

  TQVBoxLayout *evalLayout = new TQVBoxLayout(evalContainer);
  evalLayout->addSpacing(KDialog::spacingHint());
  evalLayout->addWidget(evalLabel);
  evalLayout->addSpacing(KDialog::spacingHint());
  evalLayout->addWidget(m_evalEdit);

  TQValueList<int> hsplitSizes;
  hsplitSizes.insert(hsplitSizes.end(),400);
  hsplitSizes.insert(hsplitSizes.end(),200);
  hsplitter->setSizes(hsplitSizes);

  vl->addWidget(hsplitter);

  // actions
  TDEPopupMenu *debugMenu = new TDEPopupMenu(this);
  menuBar()->insertItem("&Debug",debugMenu);

  m_actionCollection = new TDEActionCollection(this);
  m_actionCollection->setInstance(this);

  // Venkman use F12, KDevelop F10
  TDEShortcut scNext = TDEShortcut(KKeySequence(KKey(TQt::Key_F12)));
  scNext.append(KKeySequence(KKey(TQt::Key_F10)));
  m_nextAction       = new TDEAction(i18n("Next breakpoint","&Next"),"dbgnext",scNext,this,TQ_SLOT(slotNext()),
				   m_actionCollection,"next");
  m_stepAction       = new TDEAction(i18n("&Step"),"dbgstep",TDEShortcut(TQt::Key_F11),this,TQ_SLOT(slotStep()),
				   m_actionCollection,"step");
  // Venkman use F5, Kdevelop F9
  TDEShortcut scCont = TDEShortcut(KKeySequence(KKey(TQt::Key_F5)));
  scCont.append(KKeySequence(KKey(TQt::Key_F9)));
  m_continueAction   = new TDEAction(i18n("&Continue"),"dbgrun",scCont,this,TQ_SLOT(slotContinue()),
				   m_actionCollection,"cont");
  m_stopAction       = new TDEAction(i18n("St&op"),"process-stop",TDEShortcut(TQt::Key_F4),this,TQ_SLOT(slotStop()),
				   m_actionCollection,"stop");
  m_breakAction      = new TDEAction(i18n("&Break at Next Statement"),"dbgrunto",TDEShortcut(TQt::Key_F8),this,TQ_SLOT(slotBreakNext()),
				   m_actionCollection,"breaknext");


  m_nextAction->setToolTip(i18n("Next breakpoint","Next"));
  m_stepAction->setToolTip(i18n("Step"));
  m_continueAction->setToolTip(i18n("Continue"));
  m_stopAction->setToolTip(i18n("Stop"));
  m_breakAction->setToolTip("Break at next Statement");

  m_nextAction->setEnabled(false);
  m_stepAction->setEnabled(false);
  m_continueAction->setEnabled(false);
  m_stopAction->setEnabled(false);
  m_breakAction->setEnabled(true);

  m_nextAction->plug(debugMenu);
  m_stepAction->plug(debugMenu);
  m_continueAction->plug(debugMenu);
//   m_stopAction->plug(debugMenu); ### disabled until DebuggerImp::stop() works reliably
  m_breakAction->plug(debugMenu);

  m_nextAction->plug(toolBar());
  m_stepAction->plug(toolBar());
  m_continueAction->plug(toolBar());
//   m_stopAction->plug(toolBar()); ###
  m_breakAction->plug(toolBar());

  toolBar()->insertWidget(1,300,m_sourceSel);
  toolBar()->setItemAutoSized(1);

  updateContextList();
  setMinimumSize(300,200);
  resize(600,450);

}

KJSDebugWin::~KJSDebugWin()
{
  free(m_breakpoints);
  free(m_execs);
}

KJSDebugWin *KJSDebugWin::createInstance()
{
  assert(!kjs_html_debugger);
  kjs_html_debugger = new KJSDebugWin();
  return kjs_html_debugger;
}

void KJSDebugWin::destroyInstance()
{
  assert(kjs_html_debugger);
  kjs_html_debugger->hide();
  delete kjs_html_debugger;
}

void KJSDebugWin::slotNext()
{
  m_mode = Next;
  leaveSession();
}

void KJSDebugWin::slotStep()
{
  m_mode = Step;
  leaveSession();
}

void KJSDebugWin::slotContinue()
{
  m_mode = Continue;
  leaveSession();
}

void KJSDebugWin::slotStop()
{
  m_mode = Stop;
  while (!m_execStates.isEmpty())
    leaveSession();
}

void KJSDebugWin::slotBreakNext()
{
  m_mode = Step;
}

void KJSDebugWin::slotToggleBreakpoint(int lineno)
{
  if (m_sourceSel->currentItem() < 0)
    return;

  SourceFile *sourceFile = m_sourceSelFiles.at(m_sourceSel->currentItem());

  // Find the source fragment containing the selected line (if any)
  int sourceId = -1;
  int highestBaseLine = -1;
  TQMap<int,SourceFragment*>::Iterator it;

  for (it = m_sourceFragments.begin(); it != m_sourceFragments.end(); ++it) {
    SourceFragment *sourceFragment = it.data();
    if (sourceFragment &&
	sourceFragment->sourceFile == sourceFile &&
	sourceFragment->baseLine <= lineno &&
	sourceFragment->baseLine > highestBaseLine) {

	sourceId = sourceFragment->sourceId;
	highestBaseLine = sourceFragment->baseLine;
    }
  }

  if (sourceId < 0)
    return;

  // Update the source code display with the appropriate icon
  int fragmentLineno = lineno-highestBaseLine+1;
  if (!setBreakpoint(sourceId,fragmentLineno)) // was already set
    deleteBreakpoint(sourceId,fragmentLineno);

  m_sourceDisplay->updateContents();
}

void KJSDebugWin::slotShowFrame(int frameno)
{
  if (frameno < 0 || frameno >= m_execsCount)
    return;

  Context ctx = m_execs[frameno]->context();
  setSourceLine(ctx.sourceId(),ctx.curStmtFirstLine());
}

void KJSDebugWin::slotSourceSelected(int sourceSelIndex)
{
  // A source file has been selected from the drop-down list - display the file
  if (sourceSelIndex < 0 || sourceSelIndex >= (int)m_sourceSel->count())
    return;
  SourceFile *sourceFile = m_sourceSelFiles.at(sourceSelIndex);
  displaySourceFile(sourceFile,true);

  // If the currently selected context is in the current source file, then hilight
  // the line it's on.
  if (m_contextList->currentItem() >= 0) {
    Context ctx = m_execs[m_contextList->currentItem()]->context();
    if (m_sourceFragments[ctx.sourceId()]->sourceFile == m_sourceSelFiles.at(sourceSelIndex))
      setSourceLine(ctx.sourceId(),ctx.curStmtFirstLine());
  }
}

void KJSDebugWin::slotEval()
{
  // Work out which execution state to use. If we're currently in a debugging session,
  // use the current context - otherwise, use the global execution state from the interpreter
  // corresponding to the currently displayed source file.
  ExecState *exec;
  Object thisobj;
  if (m_execStates.isEmpty()) {
    if (m_sourceSel->currentItem() < 0)
      return;
    SourceFile *sourceFile = m_sourceSelFiles.at(m_sourceSel->currentItem());
    if (!sourceFile->interpreter)
      return;
    exec = sourceFile->interpreter->globalExec();
    thisobj = exec->interpreter()->globalObject();
  }
  else {
    exec = m_execStates.top();
    thisobj = exec->context().thisValue();
  }

  // Evaluate the js code from m_evalEdit
  UString code(m_evalEdit->code());
  TQString msg;

  KJSCPUGuard guard;
  guard.start();

  Interpreter *interp = exec->interpreter();

  Object obj = Object::dynamicCast(interp->globalObject().get(exec, "eval"));
  List args;
  args.append(String(code));

  m_evalDepth++;
  Value retval = obj.call(exec, thisobj, args);
  m_evalDepth--;
  guard.stop();

  // Print the return value or exception message to the console
  if (exec->hadException()) {
    Value exc = exec->exception();
    exec->clearException();
    msg = "Exception: " + exc.toString(interp->globalExec()).qstring();
  }
  else {
    msg = retval.toString(interp->globalExec()).qstring();
  }

  m_evalEdit->insert(msg+"\n");
  updateContextList();
}

void KJSDebugWin::closeEvent(TQCloseEvent *e)
{
  while (!m_execStates.isEmpty()) // ### not sure if this will work
    leaveSession();
  return TQWidget::closeEvent(e);
}

bool KJSDebugWin::eventFilter(TQObject *o, TQEvent *e)
{
  switch (e->type()) {
  case TQEvent::MouseButtonPress:
  case TQEvent::MouseButtonRelease:
  case TQEvent::MouseButtonDblClick:
  case TQEvent::MouseMove:
  case TQEvent::KeyPress:
  case TQEvent::KeyRelease:
  case TQEvent::Destroy:
  case TQEvent::Close:
  case TQEvent::Quit:
    while (o->parent())
      o = o->parent();
    if (o == this)
      return TQWidget::eventFilter(o,e);
    else
      return true;
    break;
  default:
    return TQWidget::eventFilter(o,e);
  }
}

void KJSDebugWin::disableOtherWindows()
{
  TQWidgetList *widgets = TQApplication::allWidgets();
  TQWidgetListIt it(*widgets);
  for (; it.current(); ++it)
    it.current()->installEventFilter(this);
}

void KJSDebugWin::enableOtherWindows()
{
  TQWidgetList *widgets = TQApplication::allWidgets();
  TQWidgetListIt it(*widgets);
  for (; it.current(); ++it)
    it.current()->removeEventFilter(this);
}

bool KJSDebugWin::sourceParsed(KJS::ExecState *exec, int sourceId,
                               const KJS::UString &source, int errorLine)
{
  // Work out which source file this fragment is in
  SourceFile *sourceFile = 0;
  if (!m_nextSourceUrl.isEmpty())
    sourceFile = getSourceFile(exec->interpreter(),m_nextSourceUrl);

  int index;
  if (!sourceFile) {
    index = m_sourceSel->count();
    if (!m_nextSourceUrl.isEmpty()) {

      TQString code = source.qstring();
      KParts::ReadOnlyPart *part = static_cast<ScriptInterpreter*>(exec->interpreter())->part();
      if (m_nextSourceUrl == part->url().url()) {
	// Only store the code here if it's not from the part's html page... in that
	// case we can get it from TDEHTMLPageCache
	code = TQString::null;
      }

      sourceFile = new SourceFile(m_nextSourceUrl,code,exec->interpreter());
      setSourceFile(exec->interpreter(),m_nextSourceUrl,sourceFile);
      m_sourceSelFiles.append(sourceFile);
      m_sourceSel->insertItem(m_nextSourceUrl);
    }
    else {
      // Sourced passed from somewhere else (possibly an eval call)... we don't know the url,
      // but we still know the interpreter
      sourceFile = new SourceFile("(unknown)",source.qstring(),exec->interpreter());
      m_sourceSelFiles.append(sourceFile);
      m_sourceSel->insertItem(TQString::number(index) += "-???");
    }
  }
  else {
    // Ensure that each source file to be displayed is associated with
    // an appropriate interpreter
    if (!sourceFile->interpreter)
      sourceFile->interpreter = exec->interpreter();
    for (index = 0; index < m_sourceSel->count(); index++) {
      if (m_sourceSelFiles.at(index) == sourceFile)
	break;
    }
    assert(index < m_sourceSel->count());
  }

  SourceFragment *sf = new SourceFragment(sourceId,m_nextSourceBaseLine,errorLine,sourceFile);
  m_sourceFragments[sourceId] = sf;

  if (m_sourceSel->currentItem() < 0)
    m_sourceSel->setCurrentItem(index);

  if (m_sourceSel->currentItem() == index) {
    displaySourceFile(sourceFile,true);
  }

  m_nextSourceBaseLine = 1;
  m_nextSourceUrl = "";

  return (m_mode != Stop);
}

bool KJSDebugWin::sourceUnused(KJS::ExecState *exec, int sourceId)
{
  // Verify that there aren't any contexts on the stack using the given sourceId
  // This should never be the case because this function is only called when
  // the interpreter has deleted all Node objects for the source.
  for (int e = 0; e < m_execsCount; e++)
    assert(m_execs[e]->context().sourceId() != sourceId);

  // Now remove the fragment (and the SourceFile, if it was the last fragment in that file)
  SourceFragment *fragment = m_sourceFragments[sourceId];
  if (fragment) {
    m_sourceFragments.erase(sourceId);

    SourceFile *sourceFile = fragment->sourceFile;
    if (sourceFile->hasOneRef()) {
      for (int i = 0; i < m_sourceSel->count(); i++) {
	if (m_sourceSelFiles.at(i) == sourceFile) {
	  m_sourceSel->removeItem(i);
	  m_sourceSelFiles.remove(i);
	  break;
	}
      }
      removeSourceFile(exec->interpreter(),sourceFile->url);
    }
    delete fragment;
  }

  return (m_mode != Stop);
}

bool KJSDebugWin::exception(ExecState *exec, const Value &value, bool inTryCatch)
{
  assert(value.isValid());

  // Ignore exceptions that will be caught by the script
  if (inTryCatch)
    return true;

  KParts::ReadOnlyPart *part = static_cast<ScriptInterpreter*>(exec->interpreter())->part();
  TDEHTMLPart *tdehtmlpart = ::tqt_cast<TDEHTMLPart*>(part);
  if (tdehtmlpart && !tdehtmlpart->settings()->isJavaScriptErrorReportingEnabled())
    return true;

  TQWidget *dlgParent = (m_evalDepth == 0) ? (TQWidget*)part->widget() : (TQWidget*)this;

  TQString exceptionMsg = value.toString(exec).qstring();

  // Syntax errors are a special case. For these we want to display the url & lineno,
  // which isn't included in the exception messeage. So we work it out from the values
  // passed to sourceParsed()
  Object valueObj = Object::dynamicCast(value);
  Object syntaxError = exec->interpreter()->builtinSyntaxError();
  if (valueObj.isValid() && valueObj.get(exec,"constructor").imp() == syntaxError.imp()) {
    Value sidValue = valueObj.get(exec,"sid");
    if (sidValue.isA(NumberType)) { // sid is not set for Function() constructor
      int sourceId = (int)sidValue.toNumber(exec);
      assert(m_sourceFragments[sourceId]);
      exceptionMsg = i18n("Parse error at %1 line %2")
		     .arg(m_sourceFragments[sourceId]->sourceFile->url)
		     .arg(m_sourceFragments[sourceId]->baseLine+m_sourceFragments[sourceId]->errorLine-1);
    }
  }

  bool dontShowAgain = false;
  if (m_execsCount == 0) {
    // An exception occurred and we're not currently executing any code... this can
    // happen in some cases e.g. a parse error, or native code accessing funcitons like
    // Object::put()
    TQString msg = i18n("An error occurred while attempting to run a script on this page.\n\n%1")
		  .arg(exceptionMsg);
    KJSErrorDialog dlg(dlgParent,msg,false);
    dlg.exec();
    dontShowAgain = dlg.dontShowAgain();
  }
  else {
    Context ctx = m_execs[m_execsCount-1]->context();
    SourceFragment *sourceFragment = m_sourceFragments[ctx.sourceId()];
    TQString msg = i18n("An error occurred while attempting to run a script on this page.\n\n%1 line %2:\n%3")
		  .arg(KStringHandler::rsqueeze( sourceFragment->sourceFile->url,80),
		  TQString::number( sourceFragment->baseLine+ctx.curStmtFirstLine()-1),
		  exceptionMsg);

    KJSErrorDialog dlg(dlgParent,msg,true);
    dlg.exec();
    dontShowAgain = dlg.dontShowAgain();

    if (dlg.debugSelected()) {
      m_mode = Next;
      m_steppingDepth = m_execsCount-1;
      enterSession(exec);
    }
  }

  if (dontShowAgain) {
    TDEConfig *config = kapp->config();
    TDEConfigGroupSaver saver(config,TQString::fromLatin1("Java/JavaScript Settings"));
    config->writeEntry("ReportJavaScriptErrors",TQVariant(false));
    config->sync();
    TQByteArray data;
    kapp->dcopClient()->send( "konqueror*", "KonquerorIface", "reparseConfiguration()", data );
  }

  return (m_mode != Stop);
}

bool KJSDebugWin::atStatement(KJS::ExecState *exec)
{
  assert(m_execsCount > 0);
  assert(m_execs[m_execsCount-1] == exec);
  checkBreak(exec);
  return (m_mode != Stop);
}

bool KJSDebugWin::enterContext(ExecState *exec)
{
  if (m_execsCount >= m_execsAlloc) {
    m_execsAlloc += 10;
    m_execs = (ExecState**)realloc(m_execs,m_execsAlloc*sizeof(ExecState*));
  }
  m_execs[m_execsCount++] = exec;

  if (m_mode == Step)
    m_steppingDepth = m_execsCount-1;

  checkBreak(exec);
  return (m_mode != Stop);
}

bool KJSDebugWin::exitContext(ExecState *exec, const Completion &/*completion*/)
{
  assert(m_execsCount > 0);
  assert(m_execs[m_execsCount-1] == exec);

  checkBreak(exec);

  m_execsCount--;
  if (m_steppingDepth > m_execsCount-1)
    m_steppingDepth = m_execsCount-1;
  if (m_execsCount == 0)
    updateContextList();

  return (m_mode != Stop);
}

void KJSDebugWin::displaySourceFile(SourceFile *sourceFile, bool forceRefresh)
{
  if (m_curSourceFile == sourceFile && !forceRefresh)
    return;
  sourceFile->ref();
  m_sourceDisplay->setSource(sourceFile);
  if (m_curSourceFile)
     m_curSourceFile->deref();
  m_curSourceFile = sourceFile;
}

void KJSDebugWin::setSourceLine(int sourceId, int lineno)
{
  SourceFragment *source = m_sourceFragments[sourceId];
  if (!source)
    return;

  SourceFile *sourceFile = source->sourceFile;
  if (m_curSourceFile != source->sourceFile) {
      for (int i = 0; i < m_sourceSel->count(); i++)
	if (m_sourceSelFiles.at(i) == sourceFile)
	  m_sourceSel->setCurrentItem(i);
      displaySourceFile(sourceFile,false);
  }
  m_sourceDisplay->setCurrentLine(source->baseLine+lineno-2);
}

void KJSDebugWin::setNextSourceInfo(TQString url, int baseLine)
{
  m_nextSourceUrl = url;
  m_nextSourceBaseLine = baseLine;
}

void KJSDebugWin::sourceChanged(Interpreter *interpreter, TQString url)
{
  SourceFile *sourceFile = getSourceFile(interpreter,url);
  if (sourceFile && m_curSourceFile == sourceFile)
    displaySourceFile(sourceFile,true);
}

void KJSDebugWin::clearInterpreter(Interpreter *interpreter)
{
  TQMap<int,SourceFragment*>::Iterator it;

  for (it = m_sourceFragments.begin(); it != m_sourceFragments.end(); ++it)
    if (it.data() && it.data()->sourceFile->interpreter == interpreter)
      it.data()->sourceFile->interpreter = 0;
}

SourceFile *KJSDebugWin::getSourceFile(Interpreter *interpreter, TQString url)
{
  TQString key = TQString("%1|%2").arg((long)interpreter).arg(url);
  return m_sourceFiles[key];
}

void KJSDebugWin::setSourceFile(Interpreter *interpreter, TQString url, SourceFile *sourceFile)
{
  TQString key = TQString("%1|%2").arg((long)interpreter).arg(url);
  sourceFile->ref();
  if (SourceFile* oldFile = m_sourceFiles[key])
    oldFile->deref();
  m_sourceFiles[key] = sourceFile;
}

void KJSDebugWin::removeSourceFile(Interpreter *interpreter, TQString url)
{
  TQString key = TQString("%1|%2").arg((long)interpreter).arg(url);
  if (SourceFile* oldFile = m_sourceFiles[key])
    oldFile->deref();
  m_sourceFiles.remove(key);
}

void KJSDebugWin::checkBreak(ExecState *exec)
{
  if (m_breakpointCount > 0) {
    Context ctx = m_execs[m_execsCount-1]->context();
    if (haveBreakpoint(ctx.sourceId(),ctx.curStmtFirstLine(),ctx.curStmtLastLine())) {
      m_mode = Next;
      m_steppingDepth = m_execsCount-1;
    }
  }

  if ((m_mode == Step || m_mode == Next) && m_steppingDepth == m_execsCount-1)
    enterSession(exec);
}

void KJSDebugWin::enterSession(ExecState *exec)
{
  // This "enters" a new debugging session, i.e. enables usage of the debugging window
  // It re-enters the qt event loop here, allowing execution of other parts of the
  // program to continue while the script is stopped. We have to be a bit careful here,
  // i.e. make sure the user can't quit the app, and disable other event handlers which
  // could interfere with the debugging session.
  if (!isVisible())
    show();

  m_mode = Continue;

  if (m_execStates.isEmpty()) {
    disableOtherWindows();
    m_nextAction->setEnabled(true);
    m_stepAction->setEnabled(true);
    m_continueAction->setEnabled(true);
    m_stopAction->setEnabled(true);
    m_breakAction->setEnabled(false);
  }
  m_execStates.push(exec);

  updateContextList();

  tqApp->enter_loop(); // won't return until leaveSession() is called
}

void KJSDebugWin::leaveSession()
{
  // Disables debugging for this window and returns to execute the rest of the script
  // (or aborts execution, if the user pressed stop). When this returns, the program
  // will exit the qt event loop, i.e. return to whatever processing was being done
  // before the debugger was stopped.
  assert(!m_execStates.isEmpty());

  m_execStates.pop();

  if (m_execStates.isEmpty()) {
    m_nextAction->setEnabled(false);
    m_stepAction->setEnabled(false);
    m_continueAction->setEnabled(false);
    m_stopAction->setEnabled(false);
    m_breakAction->setEnabled(true);
    m_sourceDisplay->setCurrentLine(-1);
    enableOtherWindows();
  }

  tqApp->exit_loop();
}

void KJSDebugWin::updateContextList()
{
  disconnect(m_contextList,TQ_SIGNAL(highlighted(int)),this,TQ_SLOT(slotShowFrame(int)));

  m_contextList->clear();
  for (int i = 0; i < m_execsCount; i++)
    m_contextList->insertItem(contextStr(m_execs[i]->context()));

  if (m_execsCount > 0) {
    m_contextList->setSelected(m_execsCount-1, true);
    Context ctx = m_execs[m_execsCount-1]->context();
    setSourceLine(ctx.sourceId(),ctx.curStmtFirstLine());
  }

  connect(m_contextList,TQ_SIGNAL(highlighted(int)),this,TQ_SLOT(slotShowFrame(int)));
}

TQString KJSDebugWin::contextStr(const Context &ctx)
{
  TQString str = "";
  SourceFragment *sourceFragment = m_sourceFragments[ctx.sourceId()];
  TQString url = sourceFragment->sourceFile->url;
  int fileLineno = sourceFragment->baseLine+ctx.curStmtFirstLine()-1;

  switch (ctx.codeType()) {
  case GlobalCode:
    str = TQString("Global code at %1:%2").arg(url).arg(fileLineno);
    break;
  case EvalCode:
    str = TQString("Eval code at %1:%2").arg(url).arg(fileLineno);
    break;
  case FunctionCode:
    if (!ctx.functionName().isNull())
      str = TQString("%1() at %2:%3").arg(ctx.functionName().qstring()).arg(url).arg(fileLineno);
    else
      str = TQString("Anonymous function at %1:%2").arg(url).arg(fileLineno);
    break;
  }

  return str;
}

bool KJSDebugWin::setBreakpoint(int sourceId, int lineno)
{
  if (haveBreakpoint(sourceId,lineno,lineno))
    return false;

  m_breakpointCount++;
  m_breakpoints = static_cast<Breakpoint*>(realloc(m_breakpoints,
						   m_breakpointCount*sizeof(Breakpoint)));
  m_breakpoints[m_breakpointCount-1].sourceId = sourceId;
  m_breakpoints[m_breakpointCount-1].lineno = lineno;

  return true;
}

bool KJSDebugWin::deleteBreakpoint(int sourceId, int lineno)
{
  for (int i = 0; i < m_breakpointCount; i++) {
    if (m_breakpoints[i].sourceId == sourceId && m_breakpoints[i].lineno == lineno) {

      memmove(m_breakpoints+i,m_breakpoints+i+1,(m_breakpointCount-i-1)*sizeof(Breakpoint));
      m_breakpointCount--;
      m_breakpoints = static_cast<Breakpoint*>(realloc(m_breakpoints,
						       m_breakpointCount*sizeof(Breakpoint)));
      return true;
    }
  }

  return false;
}

bool KJSDebugWin::haveBreakpoint(SourceFile *sourceFile, int line0, int line1)
{
  for (int i = 0; i < m_breakpointCount; i++) {
    int sourceId = m_breakpoints[i].sourceId;
    int lineno = m_breakpoints[i].lineno;
    if (m_sourceFragments.contains(sourceId) &&
        m_sourceFragments[sourceId]->sourceFile == sourceFile) {
      int absLineno = m_sourceFragments[sourceId]->baseLine+lineno-1;
      if (absLineno >= line0 && absLineno <= line1)
	return true;
    }
  }

  return false;
}

#include "kjs_debugwin.moc"

#endif // KJS_DEBUGGER
