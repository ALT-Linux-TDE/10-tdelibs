/**
 * This file is part of the KDE project
 *
 * Copyright (C) 2001,2003 Peter Kelly (pmk@post.com)
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

#include <tdehtml_part.h>
#include <kurl.h>
#include <tqobject.h>
#include <kjs/ustring.h>
#include <kjs/object.h>
#include <kjs/interpreter.h>

class RegressionTest;
class TQTimer;

/**
 * @internal
 */
class PartMonitor : public TQObject
{
  TQ_OBJECT
public:
    PartMonitor(TDEHTMLPart *_part);
    ~PartMonitor();
    void waitForCompletion();
    static PartMonitor* sm_highestMonitor;
    bool m_completed;
    TDEHTMLPart *m_part;
    int m_timer_waits;
    TQTimer *m_timeout_timer;
public slots:
    void partCompleted();
    void timeout();
    void finishTimers();
};

/**
 * @internal
 */
class RegTestObject : public KJS::ObjectImp
{
public:
    RegTestObject(KJS::ExecState *exec, RegressionTest *_regTest);

private:
    RegressionTest *m_regTest;
};

/**
 * @internal
 */
class RegTestFunction : public KJS::ObjectImp
{
public:
    RegTestFunction(KJS::ExecState *exec, RegressionTest *_regTest, int _id, int length);

    bool implementsCall() const;
    KJS::Value call(KJS::ExecState *exec, KJS::Object &thisObj, const KJS::List &args);

    enum { Print, ReportResult, CheckOutput, Quit };

private:
    RegressionTest *m_regTest;
    int id;
};

/**
 * @internal
 */
class TDEHTMLPartObject : public KJS::ObjectImp
{
public:
    TDEHTMLPartObject(KJS::ExecState *exec, TDEHTMLPart *_part);

    virtual KJS::Value get(KJS::ExecState *exec, const KJS::Identifier &propertyName) const;

private:
    TDEHTMLPart *m_part;
};

/**
 * @internal
 */
class TDEHTMLPartFunction : public KJS::ObjectImp
{
public:
    TDEHTMLPartFunction(KJS::ExecState *exec, TDEHTMLPart *_part, int _id, int length);

    bool implementsCall() const;
    KJS::Value call(KJS::ExecState *exec, KJS::Object &thisObj, const KJS::List &args);

    enum { OpenPage, OpenPageAsUrl, Begin, Write, End, ExecuteScript, ProcessEvents };
private:
    TDEHTMLPart *m_part;
    int id;
};

namespace KJS {
class ScriptInterpreter;
}

/**
 * @internal
 */
class RegressionTest : public TQObject
{
  TQ_OBJECT
public:

    RegressionTest(TDEHTMLPart *part, const TQString &baseDir, const TQString &outputDir,
		   bool _genOutput, bool runJS, bool runHTML);
    ~RegressionTest();

    enum OutputType { DOMTree, RenderTree };
    TQString getPartOutput( OutputType type );
    void getPartDOMOutput( TQTextStream &outputStream, TDEHTMLPart* part, uint indent );
    void dumpRenderTree( TQTextStream &outputStream, TDEHTMLPart* part );
    void testStaticFile(const TQString& filename);
    void testJSFile(const TQString& filename);
    enum CheckResult { Failure = 0, Success = 1, Ignored = 2 };
    CheckResult checkOutput(const TQString& againstFilename);
    CheckResult checkPaintdump( const TQString& againstFilename);
    enum FailureType { NoFailure = 0, AllFailure = 1, RenderFailure = 2, DomFailure = 4, PaintFailure = 8, JSFailure = 16};
    bool runTests(TQString relPath = TQString::null, bool mustExist = false, int known_failure = NoFailure);
    bool reportResult( bool passed, const TQString & description = TQString::null );
    bool reportResult(CheckResult result, const TQString & description = TQString::null );
    void createMissingDirs(const TQString &path);

    TQImage renderToImage();
    bool imageEqual( const TQImage &lhs, const TQImage &rhs );
    void createLink( const TQString& test, int failures );
    void doJavascriptReport( const TQString &test );
    void doFailureReport( const TQString& test, int failures );

    TDEHTMLPart *m_part;
    TQString m_baseDir;
    TQString m_outputDir;
    bool m_genOutput;
    TQString m_currentBase;

    TQString m_currentOutput;
    TQString m_currentCategory;
    TQString m_currentTest;
    TQPixmap* m_paintBuffer;

    bool m_getOutput;
    bool m_runJS;
    bool m_runHTML;
    int m_passes_work;
    int m_passes_fail;
    int m_failures_work;
    int m_failures_fail;
    int m_errors;
    bool saw_failure;
    bool ignore_errors;
    int m_known_failures;

    static RegressionTest *curr;

private:
    void printDescription(const TQString& description);

    static bool svnIgnored( const TQString &filename );

private:
    void evalJS( KJS::ScriptInterpreter &interp, const TQString &filename, bool report ); // used by testJS

private slots:
    void slotOpenURL(const KURL &url, const KParts::URLArgs &args);
    void resizeTopLevelWidget( int, int );

};

#endif
