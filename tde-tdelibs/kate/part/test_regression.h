/**
 * This file is part of the KDE project
 *
 * Copyright (C) 2001,2003 Peter Kelly (pmk@post.com)
 * Copyright 2006 Leo Savernik (l.savernik@aon.at)
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

#ifndef TEST_REGRESSION_H
#define TEST_REGRESSION_H

#include <katejscript.h>
#include <kateview.h>
#include <kurl.h>
#include <tqobject.h>
#include <tqstringlist.h>
#include <kjs/ustring.h>
#include <kjs/object.h>
#include <kjs/interpreter.h>

class KateDocument;
class KateView;
class RegressionTest;
class TQTimer;

namespace KParts {
  class URLArgs;
}

class OutputObject;

/**
 * @internal
 * The backbone of Kate's automatic regression tests.
 */
class TestJScriptEnv : public KateJScript
{
  public:
    TestJScriptEnv(KateDocument *part);
    virtual ~TestJScriptEnv();

    /** returns the global scope */
    KJS::Object global() const { return *m_global; }
    /** returns the script interpreter */
    KJS::Interpreter &interpreter() { return *m_interpreter; }
    /** returns the document scope */
    KJS::Object document() const { return *m_document; }
    /** returns the view scope */
    KJS::Object view() const { return *m_view; }
    /** returns the output object */
    OutputObject *output() const { return m_output; }

  protected:
    OutputObject *m_output;
};

/**
 * @internal
 */
class KateViewObject : public KJS::ObjectImp
{
  public:
    KateViewObject(KJS::ExecState *exec, KateView *v, KJS::ObjectImp *fallback);
    virtual ~KateViewObject();

    virtual const KJS::ClassInfo *classInfo() const;
    virtual KJS::Value get(KJS::ExecState *exec, const KJS::Identifier &propertyName) const;

  private:
    // evil hack I: class layout of katejscript/KateJSView must be duplicated
    // here, structurally as well as functionally
    KateView *view;
    // end evil hack
    KJS::ObjectImp *fallback;
};

/**
 * @internal
 */
class KateViewFunction : public KJS::ObjectImp
{
  public:
    KateViewFunction(KJS::ExecState *exec, KateView *v, int _id, int length);

    bool implementsCall() const;
    KJS::Value call(KJS::ExecState *exec, KJS::Object &thisObj, const KJS::List &args);

    enum { KeyReturn, Type, Backspace, DeleteWordLeft, KeyDelete,
      DeleteWordRight, Transpose, CursorLeft, ShiftCursorLeft, CursorRight,
      ShiftCursorRight, WordLeft, ShiftWordLeft, WordRight, ShiftWordRight,
      Home, ShiftHome, End, ShiftEnd, Up, ShiftUp, Down, ShiftDown, ScrollUp,
      ScrollDown, TopOfView, ShiftTopOfView, BottomOfView, ShiftBottomOfView,
      PageUp, ShiftPageUp, PageDown, ShiftPageDown, Top, ShiftTop, Bottom,
      ShiftBottom, ToMatchingBracket, ShiftToMatchingBracket };
  private:
    KateView *m_view;
    int id;
};

class OutputFunction;

/**
 * Customizing output to result-files. Writing any output into result files
 * inhibits outputting the content of the katepart after script execution, enabling one to check for coordinates and the like.
 * @internal
 */
class OutputObject : public KJS::ObjectImp
{
  public:
    OutputObject(KJS::ExecState *exec, KateDocument *d, KateView *v);
    virtual ~OutputObject();

    virtual KJS::UString className() const;

    void setChangedFlag(bool *flag) { changed = flag; }
    void setOutputString(TQString *s) { outstr = s; }

  private:
    KateDocument *doc;
    KateView *view;
    bool *changed;
    TQString *outstr;

    friend class OutputFunction;
};

/**
 * Customizing output to result-files.
 * @internal
 */
class OutputFunction : public KJS::ObjectImp
{
  public:
    OutputFunction(KJS::ExecState *exec, OutputObject *obj, int _id, int length);

    bool implementsCall() const;
    virtual KJS::Value call(KJS::ExecState *exec, KJS::Object &thisObj, const KJS::List &args);

    enum { Write, Writeln, WriteCursorPosition, WriteCursorPositionln };
  private:
    OutputObject *o;
    int id;
};

/**
 * @internal
 */
class RegressionTest : public TQObject
{
  TQ_OBJECT
public:

    RegressionTest(KateDocument *part, TDEConfig *baseConfig,
                   const TQString &baseDir, const TQString &outputDir,
                   bool _genOutput);
    ~RegressionTest();

    enum OutputType { ResultDocument };
    void testStaticFile(const TQString& filename, const TQStringList &commands);
    enum CheckResult { Failure = 0, Success = 1, Ignored = 2 };
    CheckResult checkOutput(const TQString& againstFilename);
    enum FailureType { NoFailure = 0, AllFailure = 1, ResultFailure = 4, NewFailure = 65536 };
    bool runTests(TQString relPath = TQString::null, bool mustExist = false, int known_failure = NoFailure);
    bool reportResult( bool passed, const TQString & description = TQString::null, bool *newfailure = 0 );
    bool reportResult(CheckResult result, const TQString & description = TQString::null, bool *newfailure = 0 );
    void rereadConfig();
    static void createMissingDirs(const TQString &path);

    void setFailureSnapshotConfig(TDEConfig *cfg, const TQString &snapshotname);
    void setFailureSnapshotSaver(TDEConfig *cfg, const TQString &snapshotname);

    void createLink( const TQString& test, int failures );
    void doFailureReport( const TQString& test, int failures );

    KateDocument *m_part;
    KateView *m_view;
    TDEConfig *m_baseConfig;
    TQString m_baseDir;
    TQString m_outputDir;
    bool m_genOutput;
    TQString m_currentBase;
    TDEConfig *m_failureComp;
    TDEConfig *m_failureSave;

    TQString m_currentOutput;
    TQString m_currentCategory;
    TQString m_currentTest;

    bool m_keepOutput;
    bool m_getOutput;
    bool m_showGui;
    int m_passes_work;
    int m_passes_fail;
    int m_passes_new;
    int m_failures_work;
    int m_failures_fail;
    int m_failures_new;
    int m_errors;
    bool saw_failure;
    bool ignore_errors;
    int m_known_failures;
    bool m_outputCustomised;
    TQString m_outputString;

    static RegressionTest *curr;

private:
    void printDescription(const TQString& description);

    static bool svnIgnored( const TQString &filename );

private:
    /**
     * evaluate script given by \c filename within the context of \c interp.
     * @param ignore if \c true don't evaluate if script does not exist but
     * return true nonetheless.
     * @return true if script was valid, false otherwise
     */
    bool evalJS( KJS::Interpreter &interp, const TQString &filename, bool ignore = false);
    /**
     * concatenate contents of all list files down to but not including the
     * tests directory.
     * @param relPath relative path against tests-directory
     * @param filename file name of the list files
     */
    TQStringList concatListFiles(const TQString &relPath, const TQString &filename);

private slots:
    void slotOpenURL(const KURL &url, const KParts::URLArgs &args);
    void resizeTopLevelWidget( int, int );

};

#endif
