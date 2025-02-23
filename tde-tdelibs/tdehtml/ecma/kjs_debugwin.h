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

#ifndef _KJS_DEBUGGER_H_
#define _KJS_DEBUGGER_H_

#include <tqglobal.h>

#define KJS_DEBUGGER

#ifdef KJS_DEBUGGER

#include <tqwidget.h>
#include <tqmultilineedit.h>
#include <tqpixmap.h>
#include <tqptrlist.h>
#include <tqptrstack.h>
#include <tqcheckbox.h>
#include <kdialogbase.h>
#include <tdemainwindow.h>
#include <tqscrollview.h>

#include <kjs/debugger.h>

#include "dom/dom_misc.h"

class TQListBox;
class TQComboBox;
class TDEActionCollection;
class TDEAction;

namespace KJS {
  class FunctionImp;
  class List;
  class Interpreter;
  class KJSDebugWin;

  class SourceFile : public DOM::DomShared
  {
   public:
    SourceFile(TQString u, TQString c, Interpreter *interp) 
	: url(u), code(c), interpreter(interp) {}
    TQString getCode();
    TQString url;
    TQString code;
    Interpreter *interpreter;
  };

  /**
   * @internal
   *
   * When kjs parses some code, it generates a source code fragment (or just "source").
   * This is referenced by its source id in future calls to functions such as atLine()
   * and callEvent(). We keep a record of all source fragments parsed in order to display
   * then to the user.
   *
   * For .js files, the source fragment will be the entire file. For js code included
   * in html files, however, there may be multiple source fragments within the one file
   * (e.g. multiple SCRIPT tags or onclick="..." attributes)
   *
   * In the case where a single file has multiple source fragments, the source objects
   * for these fragments will all point to the same SourceFile for their code.
   */
  class SourceFragment
  {
  public:
    SourceFragment(int sid, int bl, int el, SourceFile *sf);
    ~SourceFragment();

    int sourceId;
    int baseLine;
    int errorLine;
    SourceFile *sourceFile;
  private:
    SourceFragment(const SourceFragment& other);
    SourceFragment& operator = (const SourceFragment& other);
  };

  class KJSErrorDialog : public KDialogBase {
    TQ_OBJECT
  public:
    KJSErrorDialog(TQWidget *parent, const TQString& errorMessage, bool showDebug);
    virtual ~KJSErrorDialog();

    bool debugSelected() const { return m_debugSelected; }
    bool dontShowAgain() const { return m_dontShowAgainCb->isChecked(); }

  protected slots:
    virtual void slotUser1();

  private:
    TQCheckBox *m_dontShowAgainCb;
    bool m_debugSelected;
  };

  class EvalMultiLineEdit : public TQMultiLineEdit {
    TQ_OBJECT
  public:
      EvalMultiLineEdit(TQWidget *parent);
      const TQString & code() const { return m_code; }
  protected:
      void keyPressEvent(TQKeyEvent * e);
  private:
      TQString m_code;
  };

  class SourceDisplay : public TQScrollView {
    TQ_OBJECT
  public:
    SourceDisplay(KJSDebugWin *debugWin, TQWidget *parent, const char *name = 0);
    ~SourceDisplay();

    void setSource(SourceFile *sourceFile);
    void setCurrentLine(int lineno, bool doCenter = true);

  signals:
    void lineDoubleClicked(int lineno);

  protected:
    virtual void contentsMousePressEvent(TQMouseEvent *e);
    virtual void showEvent(TQShowEvent *);
    virtual void drawContents(TQPainter *p, int clipx, int clipy, int clipw, int cliph);

    TQString m_source;
    int m_currentLine;
    SourceFile *m_sourceFile;
    TQStringList m_lines;

    KJSDebugWin *m_debugWin;
    TQFont m_font;
    TQPixmap m_breakpointIcon;
  };

  /**
   * @internal
   *
   * KJSDebugWin represents the debugger window that is visible to the user. It contains
   * a stack frame list, a code viewer and a source fragment selector, plus buttons
   * to control execution including next, step and continue.
   *
   * There is only one debug window per program. This can be obtained by calling #instance
   */
  class KJSDebugWin : public TDEMainWindow, public Debugger, public TDEInstance
  {
    TQ_OBJECT
    friend class SourceDisplay;
  public:
    KJSDebugWin(TQWidget *parent=0, const char *name=0);
    virtual ~KJSDebugWin();

    static KJSDebugWin *createInstance();
    static void destroyInstance();
    static KJSDebugWin *debugWindow() { return kjs_html_debugger; }

    enum Mode { Disabled = 0, // No break on any statements
		Next     = 1, // Will break on next statement in current context
		Step     = 2, // Will break on next statement in current or deeper context
		Continue = 3, // Will continue until next breakpoint
		Stop     = 4  // The script will stop execution completely,
			      // as soon as possible
    };

    void setSourceLine(int sourceId, int lineno);
    void setNextSourceInfo(TQString url, int baseLine);
    void sourceChanged(Interpreter *interpreter, TQString url);
    bool inSession() const { return !m_execStates.isEmpty(); }
    void setMode(Mode m) { m_mode = m; }
    void clearInterpreter(Interpreter *interpreter);
    ExecState *getExecState() const { return m_execStates.top(); }

    // functions overridden from KJS:Debugger
    bool sourceParsed(ExecState *exec, int sourceId,
		      const UString &source, int errorLine);
    bool sourceUnused(ExecState * exec, int sourceId);
    bool exception(ExecState *exec, const Value &value, bool inTryCatch);
    bool atStatement(ExecState *exec);
    bool enterContext(ExecState *exec);
    bool exitContext(ExecState *exec, const Completion &completion);

  public slots:
    void slotNext();
    void slotStep();
    void slotContinue();
    void slotStop();
    void slotBreakNext();
    void slotToggleBreakpoint(int lineno);
    void slotShowFrame(int frameno);
    void slotSourceSelected(int sourceSelIndex);
    void slotEval();

  protected:

    void closeEvent(TQCloseEvent *e);
    bool eventFilter(TQObject *obj, TQEvent *evt);
    void disableOtherWindows();
    void enableOtherWindows();

  private:

    SourceFile *getSourceFile(Interpreter *interpreter, TQString url);
    void setSourceFile(Interpreter *interpreter, TQString url, SourceFile *sourceFile);
    void removeSourceFile(Interpreter *interpreter, TQString url);

    void checkBreak(ExecState *exec);
    void enterSession(ExecState *exec);
    void leaveSession();
    void displaySourceFile(SourceFile *sourceFile, bool forceRefresh);
    void updateContextList();

    TQString contextStr(const Context &ctx);

    struct Breakpoint {
      int sourceId;
      int lineno;
    };
    Breakpoint *m_breakpoints;
    int m_breakpointCount;
    bool setBreakpoint(int sourceId, int lineno);
    bool deleteBreakpoint(int sourceId, int lineno);
    bool haveBreakpoint(SourceFile *sourceFile, int line0, int line1);
    bool haveBreakpoint(int sourceId, int line0, int line1) const {
      for (int i = 0; i < m_breakpointCount; i++) {
	if (m_breakpoints[i].sourceId == sourceId &&
	    m_breakpoints[i].lineno >= line0 &&
	    m_breakpoints[i].lineno <= line1)
	  return true;
      }
      return false;
    }

    SourceFile *m_curSourceFile;
    Mode m_mode;
    TQString m_nextSourceUrl;
    int m_nextSourceBaseLine;
    TQPtrStack<ExecState> m_execStates;
    ExecState **m_execs;
    int m_execsCount;
    int m_execsAlloc;
    int m_steppingDepth;

    TQMap<TQString,SourceFile*> m_sourceFiles; /* maps url->SourceFile */
    TQMap<int,SourceFragment*> m_sourceFragments; /* maps SourceId->SourceFragment */
    TQPtrList<SourceFile> m_sourceSelFiles; /* maps combobox index->SourceFile */

    TDEActionCollection *m_actionCollection;
    TQPixmap m_stopIcon;
    TQPixmap m_emptyIcon;
    SourceDisplay *m_sourceDisplay;
    TQListBox *m_contextList;

    TDEAction *m_stepAction;
    TDEAction *m_nextAction;
    TDEAction *m_continueAction;
    TDEAction *m_stopAction;
    TDEAction *m_breakAction;

    TQComboBox *m_sourceSel;
    EvalMultiLineEdit *m_evalEdit;
    int m_evalDepth;

    static KJSDebugWin *kjs_html_debugger;
  };

} // namespace

#endif // KJS_DEBUGGER

#endif // _KJS_DEBUGGER_H_
