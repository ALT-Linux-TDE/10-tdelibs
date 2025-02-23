/**
 * This file is part of the KDE project
 *
 * Copyright (C) 2001,2003 Peter Kelly (pmk@post.com)
 * Copyright (C) 2003,2004 Stephan Kulow (coolo@kde.org)
 * Copyright (C) 2004 Dirk Mueller ( mueller@kde.org )
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

#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>

#include <tdeapplication.h>
#include <kstandarddirs.h>
#include <tqimage.h>
#include <tqfile.h>
#include "test_regression.h"
#include <unistd.h>
#include <stdio.h>

#include "css/cssstyleselector.h"
#include <dom_string.h>
#include "rendering/render_style.h"
#include "rendering/render_layer.h"
#include "tdehtmldefaults.h"

//We don't use the default fonts, though, but traditional testregression ones
#undef HTML_DEFAULT_VIEW_FONT
#undef HTML_DEFAULT_VIEW_FIXED_FONT
#undef HTML_DEFAULT_VIEW_SERIF_FONT
#undef HTML_DEFAULT_VIEW_SANSSERIF_FONT
#undef HTML_DEFAULT_VIEW_CURSIVE_FONT
#undef HTML_DEFAULT_VIEW_FANTASY_FONT
#define HTML_DEFAULT_VIEW_FONT "helvetica"
#define HTML_DEFAULT_VIEW_FIXED_FONT "courier"
#define HTML_DEFAULT_VIEW_SERIF_FONT "times"
#define HTML_DEFAULT_VIEW_SANSSERIF_FONT "helvetica"
#define HTML_DEFAULT_VIEW_CURSIVE_FONT "helvetica"
#define HTML_DEFAULT_VIEW_FANTASY_FONT "helvetica"


#include <tdeaction.h>
#include <tdecmdlineargs.h>
#include "tdehtml_factory.h"
#include <tdeio/job.h>
#include <tdemainwindow.h>
#include <ksimpleconfig.h>
#include <tdeglobalsettings.h>

#include <tqcolor.h>
#include <tqcursor.h>
#include <tqdir.h>
#include <tqobject.h>
#include <tqpushbutton.h>
#include <tqscrollview.h>
#include <tqstring.h>
#include <tqtextstream.h>
#include <tqvaluelist.h>
#include <tqwidget.h>
#include <tqfileinfo.h>
#include <tqtimer.h>
#include <kstatusbar.h>
#include <tqfileinfo.h>

#include "misc/decoder.h"
#include "dom/dom2_range.h"
#include "dom/dom_exception.h"
#include "dom/html_document.h"
#include "html/htmltokenizer.h"
#include "tdehtml_part.h"
#include "tdehtmlpart_p.h"
#include <tdeparts/browserextension.h>

#include "tdehtmlview.h"
#include "rendering/render_replaced.h"
#include "xml/dom_docimpl.h"
#include "html/html_baseimpl.h"
#include "dom/dom_doc.h"
#include "misc/loader.h"
#include "ecma/kjs_binding.h"
#include "ecma/kjs_dom.h"
#include "ecma/kjs_window.h"
#include "ecma/kjs_binding.h"
#include "ecma/kjs_proxy.h"

using namespace tdehtml;
using namespace DOM;
using namespace KJS;

static bool visual = false;
static pid_t xvfb;

// -------------------------------------------------------------------------

PartMonitor *PartMonitor::sm_highestMonitor = NULL;

PartMonitor::PartMonitor(TDEHTMLPart *_part)
{
    m_part = _part;
    m_completed = false;
    connect(m_part,TQ_SIGNAL(completed()),this,TQ_SLOT(partCompleted()));
    m_timer_waits = 200;
    m_timeout_timer = new TQTimer(this);
}

PartMonitor::~PartMonitor()
{
   if (this == sm_highestMonitor)
	sm_highestMonitor = 0;
}


void PartMonitor::waitForCompletion()
{
    if (!m_completed) {

        if (sm_highestMonitor)
		return;

	sm_highestMonitor = this;

        kapp->enter_loop();

        //connect(m_timeout_timer, TQ_SIGNAL(timeout()), this, TQ_SLOT( timeout() ) );
        //m_timeout_timer->stop();
	//m_timeout_timer->start( visual ? 100 : 2, true );
    }

    TQTimer::singleShot( 0, this, TQ_SLOT( finishTimers() ) );
    kapp->enter_loop();
}

void PartMonitor::timeout()
{
    kapp->exit_loop();
}

void PartMonitor::finishTimers()
{
    KJS::Window *w = KJS::Window::retrieveWindow( m_part );
    --m_timer_waits;
    if ( m_timer_waits && (w && w->winq->hasTimers()) || m_part->inProgress()) {
        // wait a bit
        TQTimer::singleShot( 10, this, TQ_SLOT(finishTimers() ) );
        return;
    }
    kapp->exit_loop();
}

void PartMonitor::partCompleted()
{
    m_completed = true;
    RenderWidget::flushWidgetResizes();
    m_timeout_timer->stop();
    connect(m_timeout_timer, TQ_SIGNAL(timeout()),this, TQ_SLOT( timeout() ) );
    m_timeout_timer->start( visual ? 100 : 2, true );
    disconnect(m_part,TQ_SIGNAL(completed()),this,TQ_SLOT(partCompleted()));
}

static void signal_handler( int )
{
    printf( "timeout\n" );
    abort();
}
// -------------------------------------------------------------------------

RegTestObject::RegTestObject(ExecState *exec, RegressionTest *_regTest)
{
    m_regTest = _regTest;
    putDirect("print",new RegTestFunction(exec,m_regTest,RegTestFunction::Print,1), DontEnum);
    putDirect("reportResult",new RegTestFunction(exec,m_regTest,RegTestFunction::ReportResult,3), DontEnum);
    putDirect("checkOutput",new RegTestFunction(exec,m_regTest,RegTestFunction::CheckOutput,1), DontEnum);
    // add "quit" for compatibility with the mozilla js shell
    putDirect("quit", new RegTestFunction(exec,m_regTest,RegTestFunction::Quit,1), DontEnum );
}

RegTestFunction::RegTestFunction(ExecState* /*exec*/, RegressionTest *_regTest, int _id, int length)
{
    m_regTest = _regTest;
    id = _id;
    putDirect("length",length);
}

bool RegTestFunction::implementsCall() const
{
    return true;
}

Value RegTestFunction::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
    Value result = Undefined();
    if ( m_regTest->ignore_errors )
        return result;

    switch (id) {
	case Print: {
	    UString str = args[0].toString(exec);
            if ( str.qstring().lower().find( "failed!" ) >= 0 )
                m_regTest->saw_failure = true;
            TQString res = str.qstring().replace('\007', "");
            m_regTest->m_currentOutput += res + "\n";
	    break;
	}
	case ReportResult: {
            bool passed = args[0].toBoolean(exec);
            TQString description = args[1].toString(exec).qstring();
            if (args[1].isA(UndefinedType) || args[1].isA(NullType))
                description = TQString::null;
            m_regTest->reportResult(passed,description);
            if ( !passed )
                m_regTest->saw_failure = true;
            break;
        }
	case CheckOutput: {
            DOM::DocumentImpl* docimpl = static_cast<DOM::DocumentImpl*>( m_regTest->m_part->document().handle() );
            if ( docimpl && docimpl->view() && docimpl->renderer() )
            {
                docimpl->updateRendering();
                docimpl->view()->layout();
            }
            TQString filename = args[0].toString(exec).qstring();
            filename = RegressionTest::curr->m_currentCategory+"/"+filename;
            int failures = RegressionTest::NoFailure;
            if ( m_regTest->m_genOutput ) {
                if ( !m_regTest->reportResult( m_regTest->checkOutput(filename+"-dom"),
                                               "Script-generated " + filename + "-dom") )
                    failures |= RegressionTest::DomFailure;
                if ( !m_regTest->reportResult( m_regTest->checkOutput(filename+"-render"),
                                         "Script-generated " + filename + "-render") )
                    failures |= RegressionTest::RenderFailure;
            } else {
                // compare with output file
                if ( !m_regTest->reportResult( m_regTest->checkOutput(filename+"-dom"), "DOM") )
                    failures |= RegressionTest::DomFailure;
                if ( !m_regTest->reportResult( m_regTest->checkOutput(filename+"-render"), "RENDER") )
                    failures |= RegressionTest::RenderFailure;
            }
            RegressionTest::curr->doFailureReport( filename, failures );
            break;
        }
        case Quit:
            m_regTest->reportResult(true,
				    "Called quit" );
            if ( !m_regTest->saw_failure )
                m_regTest->ignore_errors = true;
            break;
    }

    return result;
}

// -------------------------------------------------------------------------

TDEHTMLPartObject::TDEHTMLPartObject(ExecState *exec, TDEHTMLPart *_part)
{
    m_part = _part;
    putDirect("openPage", new TDEHTMLPartFunction(exec,m_part,TDEHTMLPartFunction::OpenPage,1), DontEnum);
    putDirect("openPageAsUrl", new TDEHTMLPartFunction(exec,m_part,TDEHTMLPartFunction::OpenPageAsUrl,1), DontEnum);
    putDirect("begin",     new TDEHTMLPartFunction(exec,m_part,TDEHTMLPartFunction::Begin,1), DontEnum);
    putDirect("write",    new TDEHTMLPartFunction(exec,m_part,TDEHTMLPartFunction::Write,1), DontEnum);
    putDirect("end",    new TDEHTMLPartFunction(exec,m_part,TDEHTMLPartFunction::End,0), DontEnum);
    putDirect("executeScript", new TDEHTMLPartFunction(exec,m_part,TDEHTMLPartFunction::ExecuteScript,0), DontEnum);
    putDirect("processEvents", new TDEHTMLPartFunction(exec,m_part,TDEHTMLPartFunction::ProcessEvents,0), DontEnum);
}

Value TDEHTMLPartObject::get(ExecState *exec, const Identifier &propertyName) const
{
    if (propertyName == "document")
        return getDOMNode(exec,m_part->document());
    else if (propertyName == "window")
        return Object(KJS::Window::retrieveWindow(m_part));
    else
        return ObjectImp::get(exec,propertyName);
}

TDEHTMLPartFunction::TDEHTMLPartFunction(ExecState */*exec*/, TDEHTMLPart *_part, int _id, int length)
{
    m_part = _part;
    id = _id;
    putDirect("length",length);
}

bool TDEHTMLPartFunction::implementsCall() const
{
    return true;
}

Value TDEHTMLPartFunction::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
    Value result = Undefined();

    switch (id) {
        case OpenPage: {
	    if (args[0].type() == NullType || args[0].type() == NullType) {
		exec->setException(Error::create(exec, GeneralError,"No filename specified"));
		return Undefined();
	    }

            TQString filename = args[0].toString(exec).qstring();
            TQString fullFilename = TQFileInfo(RegressionTest::curr->m_currentBase+"/"+filename).absFilePath();
            KURL url;
            url.setProtocol("file");
            url.setPath(fullFilename);
            PartMonitor pm(m_part);
            m_part->openURL(url);
            pm.waitForCompletion();
	    kapp->processEvents(60000);
            break;
        }
	case OpenPageAsUrl: {
	    if (args[0].type() == NullType || args[0].type() == UndefinedType) {
		exec->setException(Error::create(exec, GeneralError,"No filename specified"));
		return Undefined();
	    }
	    if (args[1].type() == NullType || args[1].type() == UndefinedType) {
		exec->setException(Error::create(exec, GeneralError,"No url specified"));
		return Undefined();
	    }

            TQString filename = args[0].toString(exec).qstring();
            TQString url = args[1].toString(exec).qstring();
            TQFile file(RegressionTest::curr->m_currentBase+"/"+filename);
	    if (!file.open(IO_ReadOnly)) {
		exec->setException(Error::create(exec, GeneralError,
						 TQString("Error reading " + filename).latin1()));
	    }
	    else {
		TQByteArray fileData;
		TQDataStream stream(fileData,IO_WriteOnly);
		char buf[1024];
		int bytesread;
		while (!file.atEnd()) {
		    bytesread = file.readBlock(buf,1024);
		    stream.writeRawBytes(buf,bytesread);
		}
		file.close();
		TQString contents(fileData);
		PartMonitor pm(m_part);
		m_part->begin(KURL( url ));
		m_part->write(contents);
		m_part->end();
		pm.waitForCompletion();
	    }
	    kapp->processEvents(60000);
	    break;
	}
	case Begin: {
            TQString url = args[0].toString(exec).qstring();
            m_part->begin(KURL( url ));
            break;
        }
        case Write: {
            TQString str = args[0].toString(exec).qstring();
            m_part->write(str);
            break;
        }
        case End: {
            m_part->end();
	    kapp->processEvents(60000);
            break;
        }
	case ExecuteScript: {
	    TQString code = args[0].toString(exec).qstring();
	    Completion comp;
	    KJSProxy *proxy = m_part->jScript();
	    proxy->evaluate("",0,code,0,&comp);
	    if (comp.complType() == Throw)
		exec->setException(comp.value());
	    kapp->processEvents(60000);
	    break;
	}
	case ProcessEvents: {
	    kapp->processEvents(60000);
	    break;
	}
    }

    return result;
}

// -------------------------------------------------------------------------

static TDECmdLineOptions options[] =
{
    { "b", 0, 0 },
    { "base <base_dir>", "Directory containing tests, basedir and output directories.", 0},
    { "d", 0, 0 },
    { "debug", "Do not supress debug output", 0},
    { "g", 0, 0 } ,
    { "genoutput", "Regenerate baseline (instead of checking)", 0 } ,
    { "s", 0, 0 } ,
    { "show", "Show the window while running tests", 0 } ,
    { "t", 0, 0 } ,
    { "test <filename>", "Only run a single test. Multiple options allowed.", 0 } ,
    { "js",  "Only run .js tests", 0 },
    { "html", "Only run .html tests", 0},
    { "noxvfb", "Do not use Xvfb", 0},
    { "o", 0, 0 },
    { "output <directory>", "Put output in <directory> instead of <base_dir>/output", 0 } ,
    { "+[base_dir]", "Directory containing tests,basedir and output directories. Only regarded if -b is not specified.", 0 } ,
    { "+[testcases]", "Relative path to testcase, or directory of testcases to be run (equivalent to -t).", 0 } ,
    TDECmdLineLastOption
};

static bool existsDir(TQCString dir)
{
    struct stat st;

    return (!stat(dir.data(), &st) && S_ISDIR(st.st_mode));
}

int main(int argc, char *argv[])
{
    // forget about any settings
    passwd* pw = getpwuid( getuid() );
    if (!pw) {
        fprintf(stderr, "dang, I don't even know who I am.\n");
        exit(1);
    }

    TQString kh("/var/tmp/%1_non_existant");
    kh = kh.arg( pw->pw_name );
    setenv( "TDEHOME", kh.latin1(), 1 );
    setenv( "LC_ALL", "C", 1 );
    setenv( "LANG", "C", 1 );

    signal( SIGALRM, signal_handler );

    // workaround various Qt crashes by always enforcing a TrueColor visual
    TQApplication::setColorSpec( TQApplication::ManyColor );

    TDECmdLineArgs::init(argc, argv, "testregression", "TestRegression",
                       "Regression tester for tdehtml", "1.0");
    TDECmdLineArgs::addCmdLineOptions(options);

    TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs( );

    TQCString baseDir = args->getOption("base");

    if ( args->count() < 1 && baseDir.isEmpty() ) {
	TDECmdLineArgs::usage();
	::exit( 1 );
    }

    int testcase_index = 0;
    if (baseDir.isEmpty()) baseDir = args->arg(testcase_index++);

    TQFileInfo bdInfo(baseDir);
    baseDir = TQFile::encodeName(bdInfo.absFilePath());

    const char *subdirs[] = {"tests", "baseline", "output", "resources"};
    for ( int i = 0; i < 3; i++ ) {
        TQFileInfo sourceDir(TQFile::encodeName( baseDir ) + "/" + subdirs[i]);
        if ( !sourceDir.exists() || !sourceDir.isDir() ) {
            fprintf(stderr,"ERROR: Source directory \"%s/%s\": no such directory.\n", (const char *)baseDir, subdirs[i]);
            exit(1);
        }
    }

    if (args->isSet("xvfb"))
    {
        TQString xvfbPath = TDEStandardDirs::findExe("Xvfb");
        if ( xvfbPath.isEmpty() ) {
            fprintf( stderr, "[test_regression] ERROR: We need Xvfb to be installed for reliable results\n" );
            exit( 1 );
        }
        
        TQCString xvfbPath8 = TQFile::encodeName(xvfbPath);
        TQStringList fpaths;
        fpaths.append(baseDir+"/resources");

        const char* const fontdirs[] = { "75dpi", "misc", "Type1" };
        const char* const fontpaths[] =  {"/usr/share/fonts/", "/usr/X11/lib/X11/fonts/",
            "/usr/lib/X11/fonts/", "/usr/share/fonts/X11/" };

        for (size_t fp=0; fp < sizeof(fontpaths)/sizeof(*fontpaths); ++fp)
            for (size_t fd=0; fd < sizeof(fontdirs)/sizeof(*fontdirs); ++fd)
                if (existsDir(TQCString(fontpaths[fp])+TQCString(fontdirs[fd])))
                    if (strcmp(fontdirs[fd] , "Type1"))
                        fpaths.append(TQCString(fontpaths[fp])+TQCString(fontdirs[fd])+":unscaled");
                    else
                        fpaths.append(TQCString(fontpaths[fp])+TQCString(fontdirs[fd]));

        xvfb = fork();
        if ( !xvfb ) {
            TQCString buffer = fpaths.join(",").latin1();
            execl( xvfbPath8.data(), xvfbPath8.data(), "-once", "-dpi", "100", "-screen", "0",
                    "1024x768x16", "-ac", "-fp", buffer.data(), ":47", (char*)NULL );
        }

        setenv( "DISPLAY", ":47", 1 );
    }

    TDEApplication a;
    a.disableAutoDcopRegistration();
    a.setStyle( "windows" );
    KSimpleConfig sc1( "cryptodefaults" );
    sc1.setGroup( "Warnings" );
    sc1.writeEntry( "OnUnencrypted",  false );
    a.config()->setGroup( "Notification Messages" );
    a.config()->writeEntry( "kjscupguard_alarmhandler", true );
    a.config()->setGroup("HTML Settings");
    a.config()->writeEntry("ReportJSErrors", false);
    TDEConfig cfg( "tdehtmlrc" );
    cfg.setGroup("HTML Settings");
    cfg.writeEntry( "StandardFont", HTML_DEFAULT_VIEW_SANSSERIF_FONT );
    cfg.writeEntry( "FixedFont", HTML_DEFAULT_VIEW_FIXED_FONT );
    cfg.writeEntry( "SerifFont", HTML_DEFAULT_VIEW_SERIF_FONT );
    cfg.writeEntry( "SansSerifFont", HTML_DEFAULT_VIEW_SANSSERIF_FONT );
    cfg.writeEntry( "CursiveFont", HTML_DEFAULT_VIEW_CURSIVE_FONT );
    cfg.writeEntry( "FantasyFont", HTML_DEFAULT_VIEW_FANTASY_FONT );
    cfg.writeEntry( "MinimumFontSize", HTML_DEFAULT_MIN_FONT_SIZE );
    cfg.writeEntry( "MediumFontSize", 10 );
    cfg.writeEntry( "Fonts", TQStringList() );
    cfg.writeEntry( "DefaultEncoding", "" );
    cfg.setGroup("Java/JavaScript Settings");
    cfg.writeEntry( "WindowOpenPolicy", TDEHTMLSettings::KJSWindowOpenAllow);

    cfg.sync();

    int rv = 1;

    if ( !args->isSet( "debug" ) ) {
        KSimpleConfig dc( "kdebugrc" );
        static int areas[] = { 1000, 6000, 6005, 6010, 6020, 6030,
                               6031, 6035, 6036, 6040, 6041, 6045,
                               6050, 6060, 6061, 7000, 7006, 170,
                               171, 7101, 7002, 7019, 7027, 7014,
                               7011, 6070, 6080, 6090, 0};
        for ( int i = 0; areas[i]; ++i ) {
            dc.setGroup( TQString::number( areas[i] ) );
            dc.writeEntry( "InfoOutput", 4 );
        }
        dc.sync();

        kdClearDebugConfig();
    }

    // create widgets
    TDEHTMLFactory *fac = new TDEHTMLFactory();
    TDEMainWindow *toplevel = new TDEMainWindow();
    TDEHTMLPart *part = new TDEHTMLPart( toplevel, 0, toplevel, 0, TDEHTMLPart::BrowserViewGUI );

    toplevel->setCentralWidget( part->widget() );
    part->setJScriptEnabled(true);

    part->executeScript(DOM::Node(), ""); // force the part to create an interpreter
//    part->setJavaEnabled(true);

    if (args->isSet("show"))
	visual = true;

    a.setTopWidget(part->widget());
    a.setMainWidget( toplevel );
    if ( visual )
        toplevel->show();

    // we're not interested
    toplevel->statusBar()->hide();

    if (!getenv("TDE_DEBUG")) {
        // set ulimits
        rlimit vmem_limit = { 256*1024*1024, RLIM_INFINITY };	// 256Mb Memory should suffice
        setrlimit(RLIMIT_AS, &vmem_limit);
        rlimit stack_limit = { 8*1024*1024, RLIM_INFINITY };	// 8Mb Memory should suffice
        setrlimit(RLIMIT_STACK, &stack_limit);
    }

    // run the tests
    RegressionTest *regressionTest = new RegressionTest(part,
                                                        baseDir,
                                                        args->getOption("output"),
                                                        args->isSet("genoutput"),
                                                        !args->isSet( "html" ),
                                                        !args->isSet( "js" ));
    TQObject::connect(part->browserExtension(), TQ_SIGNAL(openURLRequest(const KURL &, const KParts::URLArgs &)),
		     regressionTest, TQ_SLOT(slotOpenURL(const KURL&, const KParts::URLArgs &)));
    TQObject::connect(part->browserExtension(), TQ_SIGNAL(resizeTopLevelWidget( int, int )),
		     regressionTest, TQ_SLOT(resizeTopLevelWidget( int, int )));

    bool result = false;
    QCStringList tests = args->getOptionList("test");
    // merge testcases specified on command line
    for (; testcase_index < args->count(); testcase_index++)
        tests << args->arg(testcase_index);
    if (tests.count() > 0)
        for (TQValueListConstIterator<TQCString> it = tests.begin(); it != tests.end(); ++it) {
	    result = regressionTest->runTests(*it,true);
            if (!result) break;
        }
    else
	result = regressionTest->runTests();

    if (result) {
	if (args->isSet("genoutput")) {
	    printf("\nOutput generation completed.\n");
	}
	else {
	    printf("\nTests completed.\n");
            printf("Total:    %d\n",
                   regressionTest->m_passes_work+
                   regressionTest->m_passes_fail+
                   regressionTest->m_failures_work+
                   regressionTest->m_failures_fail+
                   regressionTest->m_errors);
	    printf("Passes:   %d",regressionTest->m_passes_work);
            if ( regressionTest->m_passes_fail )
                printf( " (%d unexpected passes)\n", regressionTest->m_passes_fail );
            else
                printf( "\n" );
	    printf("Failures: %d",regressionTest->m_failures_work);
            if ( regressionTest->m_failures_fail )
                printf( " (%d expected failures)\n", regressionTest->m_failures_fail );
            else
                printf( "\n" );
            if ( regressionTest->m_errors )
                printf("Errors:   %d\n",regressionTest->m_errors);

            TQFile list( regressionTest->m_outputDir + "/links.html" );
            list.open( IO_WriteOnly|IO_Append );
            TQString link, cl;
            link = TQString( "<hr>%1 failures. (%2 expected failures)" )
                   .arg(regressionTest->m_failures_work )
                   .arg( regressionTest->m_failures_fail );
            list.writeBlock( link.latin1(), link.length() );
            list.close();
	}
    }

    // Only return a 0 exit code if all tests were successful
    if (regressionTest->m_failures_work == 0 && regressionTest->m_errors == 0)
	rv = 0;

    // cleanup
    delete regressionTest;
    delete part;
    delete toplevel;
    delete fac;

    tdehtml::Cache::clear();
    tdehtml::CSSStyleSelector::clear();
    tdehtml::RenderStyle::cleanup();

    kill( xvfb, SIGINT );

    return rv;
}

// -------------------------------------------------------------------------

RegressionTest *RegressionTest::curr = 0;

RegressionTest::RegressionTest(TDEHTMLPart *part, const TQString &baseDir, const TQString &outputDir,
			       bool _genOutput, bool runJS, bool runHTML)
  : TQObject(part)
{
    m_part = part;
    m_baseDir = baseDir;
    m_baseDir = m_baseDir.replace( "//", "/" );
    if ( m_baseDir.endsWith( "/" ) )
        m_baseDir = m_baseDir.left( m_baseDir.length() - 1 );
    if (outputDir.isEmpty())
        m_outputDir = m_baseDir + "/output";
    else {
        createMissingDirs(outputDir + "/");
        m_outputDir = outputDir;
    }
    m_genOutput = _genOutput;
    m_runJS = runJS;
    m_runHTML =  runHTML;
    m_passes_work = m_passes_fail = 0;
    m_failures_work = m_failures_fail = 0;
    m_errors = 0;

    ::unlink( TQFile::encodeName( m_outputDir + "/links.html" ) );
    TQFile f( m_outputDir + "/empty.html" );
    TQString s;
    f.open( IO_WriteOnly | IO_Truncate );
    s = "<html><body>Follow the white rabbit";
    f.writeBlock( s.latin1(), s.length() );
    f.close();
    f.setName( m_outputDir + "/index.html" );
    f.open( IO_WriteOnly | IO_Truncate );
    s = "<html><frameset cols=150,*><frame src=links.html><frame name=content src=empty.html>";
    f.writeBlock( s.latin1(), s.length() );
    f.close();

    m_paintBuffer = 0;

    curr = this;
}

#include <tqobjectlist.h>

static TQStringList readListFile( const TQString &filename )
{
    // Read ignore file for this directory
    TQString ignoreFilename = filename;
    TQFileInfo ignoreInfo(ignoreFilename);
    TQStringList ignoreFiles;
    if (ignoreInfo.exists()) {
        TQFile ignoreFile(ignoreFilename);
        if (!ignoreFile.open(IO_ReadOnly)) {
            fprintf(stderr,"Can't open %s\n",ignoreFilename.latin1());
            exit(1);
        }
        TQTextStream ignoreStream(&ignoreFile);
        TQString line;
        while (!(line = ignoreStream.readLine()).isNull())
            ignoreFiles.append(line);
        ignoreFile.close();
    }
    return ignoreFiles;
}

RegressionTest::~RegressionTest()
{
    delete m_paintBuffer;
}

bool RegressionTest::runTests(TQString relPath, bool mustExist, int known_failure)
{
    m_currentOutput = TQString::null;

    if (!TQFile(m_baseDir + "/tests/"+relPath).exists()) {
	fprintf(stderr,"%s: No such file or directory\n",relPath.latin1());
	return false;
    }

    TQString fullPath = m_baseDir + "/tests/"+relPath;
    TQFileInfo info(fullPath);

    if (!info.exists() && mustExist) {
	fprintf(stderr,"%s: No such file or directory\n",relPath.latin1());
	return false;
    }

    if (!info.isReadable() && mustExist) {
	fprintf(stderr,"%s: Access denied\n",relPath.latin1());
	return false;
    }

    if (info.isDir()) {
        TQStringList ignoreFiles = readListFile(  m_baseDir + "/tests/"+relPath+"/ignore" );
        TQStringList failureFiles = readListFile(  m_baseDir + "/tests/"+relPath+"/KNOWN_FAILURES" );

	// Run each test in this directory, recusively
	TQDir sourceDir(m_baseDir + "/tests/"+relPath);
	for (uint fileno = 0; fileno < sourceDir.count(); fileno++) {
	    TQString filename = sourceDir[fileno];
	    TQString relFilename = relPath.isEmpty() ? filename : relPath+"/"+filename;

	    if (filename == "." || filename == ".." ||  ignoreFiles.contains(filename) )
                continue;
            int failure_type = NoFailure;
            if ( failureFiles.contains( filename ) )
                failure_type |= AllFailure;
            if ( failureFiles.contains ( filename + "-render" ) )
                failure_type |= RenderFailure;
            if ( failureFiles.contains ( filename + "-dump.png" ) )
                failure_type |= PaintFailure;
            if ( failureFiles.contains ( filename + "-dom" ) )
                failure_type |= DomFailure;
            runTests(relFilename, false, failure_type );
	}
    }
    else if (info.isFile()) {

        alarm( 400 );

        tdehtml::Cache::init();

	TQString relativeDir = TQFileInfo(relPath).dirPath();
	TQString filename = info.fileName();
	m_currentBase = m_baseDir + "/tests/"+relativeDir;
	m_currentCategory = relativeDir;
	m_currentTest = filename;
        m_known_failures = known_failure;
	if ( filename.endsWith(".html") || filename.endsWith( ".htm" ) || filename.endsWith( ".xhtml" ) || filename.endsWith( ".xml" ) ) {
            if ( relPath.startsWith( "domts/" ) && !m_runJS )
                return true;
	    if ( relPath.startsWith( "ecma/" ) && !m_runJS )
	        return true;
            if ( m_runHTML )
                testStaticFile(relPath);
	}
	else if (filename.endsWith(".js")) {
            if ( m_runJS )
                testJSFile(relPath);
	}
	else if (mustExist) {
	    fprintf(stderr,"%s: Not a valid test file (must be .htm(l) or .js)\n",relPath.latin1());
	    return false;
	}
    } else if (mustExist) {
        fprintf(stderr,"%s: Not a regular file\n",relPath.latin1());
        return false;
    }

    return true;
}

void RegressionTest::getPartDOMOutput( TQTextStream &outputStream, TDEHTMLPart* part, uint indent )
{
    Node node = part->document();
    while (!node.isNull()) {
	// process

	for (uint i = 0; i < indent; i++)
	    outputStream << "  ";
	outputStream << node.nodeName().string();

	switch (node.nodeType()) {
	    case Node::ELEMENT_NODE: {
		// Sort strings to ensure consistent output
		TQStringList attrNames;
		NamedNodeMap attrs = node.attributes();
		for (uint a = 0; a < attrs.length(); a++)
		    attrNames.append(attrs.item(a).nodeName().string());
		attrNames.sort();

		TQStringList::iterator it;
		Element elem(node);
		for (it = attrNames.begin(); it != attrNames.end(); ++it) {
		    TQString name = *it;
		    TQString value = elem.getAttribute(*it).string();
		    outputStream << " " << name << "=\"" << value << "\"";
		}
		if ( node.handle()->id() == ID_FRAME ) {
			outputStream << endl;
			TQString frameName = static_cast<DOM::HTMLFrameElementImpl *>( node.handle() )->name.string();
			TDEHTMLPart* frame = part->findFrame( frameName );
			Q_ASSERT( frame );
			if ( frame )
			    getPartDOMOutput( outputStream, frame, indent );
		}
		break;
	    }
	    case Node::ATTRIBUTE_NODE:
		// Should not be present in tree
		assert(false);
		break;
            case Node::TEXT_NODE:
		outputStream << " \"" << Text(node).data().string() << "\"";
		break;
            case Node::CDATA_SECTION_NODE:
		outputStream << " \"" << CDATASection(node).data().string() << "\"";
		break;
            case Node::ENTITY_REFERENCE_NODE:
		break;
            case Node::ENTITY_NODE:
		break;
            case Node::PROCESSING_INSTRUCTION_NODE:
		break;
            case Node::COMMENT_NODE:
		outputStream << " \"" << Comment(node).data().string() << "\"";
		break;
            case Node::DOCUMENT_NODE:
		break;
            case Node::DOCUMENT_TYPE_NODE:
		break;
            case Node::DOCUMENT_FRAGMENT_NODE:
		// Should not be present in tree
		assert(false);
		break;
            case Node::NOTATION_NODE:
		break;
            default:
		assert(false);
		break;
	}

	outputStream << endl;

	if (!node.firstChild().isNull()) {
	    node = node.firstChild();
	    indent++;
	}
	else if (!node.nextSibling().isNull()) {
	    node = node.nextSibling();
	}
	else {
	    while (!node.isNull() && node.nextSibling().isNull()) {
		node = node.parentNode();
		indent--;
	    }
	    if (!node.isNull())
		node = node.nextSibling();
	}
    }
}

void RegressionTest::dumpRenderTree( TQTextStream &outputStream, TDEHTMLPart* part )
{
    DOM::DocumentImpl* doc = static_cast<DocumentImpl*>( part->document().handle() );
    if ( !doc || !doc->renderer() )
        return;
    doc->renderer()->layer()->dump( outputStream );

    // Dump frames if any
    // Get list of names instead of frames() to sort the list alphabetically
    TQStringList names = part->frameNames();
    names.sort();
    for ( TQStringList::iterator it = names.begin(); it != names.end(); ++it ) {
        outputStream << "FRAME: " << (*it) << "\n";
	TDEHTMLPart* frame = part->findFrame( (*it) );
	Q_ASSERT( frame );
	if ( frame )
            dumpRenderTree( outputStream, frame );
    }
}

TQString RegressionTest::getPartOutput( OutputType type)
{
    // dump out the contents of the rendering & DOM trees
    TQString dump;
    TQTextStream outputStream(dump,IO_WriteOnly);

    if ( type == RenderTree ) {
        dumpRenderTree( outputStream, m_part );
    } else {
        assert( type == DOMTree );
        getPartDOMOutput( outputStream, m_part, 0 );
    }

    dump.replace( m_baseDir + "/tests", TQString::fromLatin1( "REGRESSION_SRCDIR" ) );
    return dump;
}

TQImage RegressionTest::renderToImage()
{
    int ew = m_part->view()->contentsWidth();
    int eh = m_part->view()->contentsHeight();

    if (ew * eh > 4000 * 4000) // don't DoS us
        return TQImage();

    TQImage img( ew, eh, 32 );
    img.fill( 0xff0000 );
    if (!m_paintBuffer )
        m_paintBuffer = new TQPixmap( 512, 128, -1, TQPixmap::MemoryOptim );

    for ( int py = 0; py < eh; py += 128 ) {
        for ( int px = 0; px < ew; px += 512 ) {
            TQPainter* tp = new TQPainter;
            tp->begin( m_paintBuffer );
            tp->translate( -px, -py );
            tp->fillRect(px, py, 512, 128, TQt::magenta);
            m_part->document().handle()->renderer()->layer()->paint( tp, TQRect( px, py, 512, 128 ) );
            tp->end();
            delete tp;

            // now fill the chunk into our image
            TQImage chunk = m_paintBuffer->convertToImage();
            assert( chunk.depth() == 32 );
            for ( int y = 0; y < 128 && py + y < eh; ++y )
                memcpy( img.scanLine( py+y ) + px*4, chunk.scanLine( y ), kMin( 512, ew-px )*4 );
        }
    }

    assert( img.depth() == 32 );
    return img;
}

bool RegressionTest::imageEqual( const TQImage &lhsi, const TQImage &rhsi )
{
    if ( lhsi.width() != rhsi.width() || lhsi.height() != rhsi.height() ) {
        kdDebug() << "dimensions different " << lhsi.size() << " " << rhsi.size() << endl;
        return false;
    }
    int w = lhsi.width();
    int h = lhsi.height();
    int bytes = lhsi.bytesPerLine();

    for ( int y = 0; y < h; ++y )
    {
        TQRgb* ls = ( TQRgb* ) lhsi.scanLine( y );
        TQRgb* rs = ( TQRgb* ) rhsi.scanLine( y );
        if ( memcmp( ls, rs, bytes ) ) {
            for ( int x = 0; x < w; ++x ) {
                TQRgb l = ls[x];
                TQRgb r = rs[x];
                if ( ( abs( tqRed( l ) - tqRed(r ) ) < 20 ) &&
                     ( abs( tqGreen( l ) - tqGreen(r ) ) < 20 ) &&
                     ( abs( tqBlue( l ) - tqBlue(r ) ) < 20 ) )
                    continue;
                 kdDebug() << "pixel (" << x << ", " << y << ") is different " << TQColor(  lhsi.pixel (  x, y ) ) << " " << TQColor(  rhsi.pixel (  x, y ) ) << endl;
                return false;
            }
        }
    }

    return true;
}

void RegressionTest::createLink( const TQString& test, int failures )
{
    createMissingDirs( m_outputDir + "/" + test + "-compare.html" );

    TQFile list( m_outputDir + "/links.html" );
    list.open( IO_WriteOnly|IO_Append );
    TQString link;
    link = TQString( "<a href=\"%1\" target=\"content\" title=\"%2\">" )
           .arg( test + "-compare.html" )
           .arg( test );
    link += m_currentTest;
    link += "</a> [";
    if ( failures & DomFailure )
        link += "D";
    if ( failures & RenderFailure )
        link += "R";
    if ( failures & PaintFailure )
        link += "P";
    link += "]<br>\n";
    list.writeBlock( link.latin1(), link.length() );
    list.close();
}

void RegressionTest::doJavascriptReport( const TQString &test )
{
    TQFile compare( m_outputDir + "/" + test + "-compare.html" );
    if ( !compare.open( IO_WriteOnly|IO_Truncate ) )
        kdDebug() << "failed to open " << m_outputDir + "/" + test + "-compare.html" << endl;
    TQString cl;
    cl = TQString( "<html><head><title>%1</title>" ).arg( test );
    cl += "<body><tt>";
    TQString text = "\n" + m_currentOutput;
    text.replace( '<', "&lt;" );
    text.replace( '>', "&gt;" );
    text.replace( TQRegExp( "\nFAILED" ), "\n<span style='color: red'>FAILED</span>" );
    text.replace( TQRegExp( "\nFAIL" ), "\n<span style='color: red'>FAIL</span>" );
    text.replace( TQRegExp( "\nPASSED" ), "\n<span style='color: green'>PASSED</span>" );
    text.replace( TQRegExp( "\nPASS" ), "\n<span style='color: green'>PASS</span>" );
    if ( text.at( 0 ) == '\n' )
        text = text.mid( 1, text.length() );
    text.replace( '\n', "<br>\n" );
    cl += text;
    cl += "</tt></body></html>";
    compare.writeBlock( cl.latin1(), cl.length() );
    compare.close();
}

/** returns the path in a way that is relatively reachable from base.
 * @param base base directory (must not include trailing slash)
 * @param path directory/file to be relatively reached by base
 * @return path with all elements replaced by .. and concerning path elements
 *	to be relatively reachable from base.
 */
static TQString makeRelativePath(const TQString &base, const TQString &path)
{
    TQString absBase = TQFileInfo(base).absFilePath();
    TQString absPath = TQFileInfo(path).absFilePath();
//     kdDebug() << "absPath: \"" << absPath << "\"" << endl;
//     kdDebug() << "absBase: \"" << absBase << "\"" << endl;

    // walk up to common ancestor directory
    int pos = 0;
    do {
        pos++;
        int newpos = absBase.find('/', pos);
        if (newpos == -1) newpos = absBase.length();
        TQConstString cmpPathComp(absPath.unicode() + pos, newpos - pos);
        TQConstString cmpBaseComp(absBase.unicode() + pos, newpos - pos);
//         kdDebug() << "cmpPathComp: \"" << cmpPathComp.string() << "\"" << endl;
//         kdDebug() << "cmpBaseComp: \"" << cmpBaseComp.string() << "\"" << endl;
//         kdDebug() << "pos: " << pos << " newpos: " << newpos << endl;
        if (cmpPathComp.string() != cmpBaseComp.string()) { pos--; break; }
        pos = newpos;
    } while (pos < (int)absBase.length() && pos < (int)absPath.length());
    int basepos = pos < (int)absBase.length() ? pos + 1 : pos;
    int pathpos = pos < (int)absPath.length() ? pos + 1 : pos;

//     kdDebug() << "basepos " << basepos << " pathpos " << pathpos << endl;

    TQString rel;
    {
        TQConstString relBase(absBase.unicode() + basepos, absBase.length() - basepos);
        TQConstString relPath(absPath.unicode() + pathpos, absPath.length() - pathpos);
        // generate as many .. as there are path elements in relBase
        if (relBase.string().length() > 0) {
            for (int i = relBase.string().contains('/'); i > 0; --i)
                rel += "../";
            rel += "..";
            if (relPath.string().length() > 0) rel += "/";
        }
        rel += relPath.string();
    }
    return rel;
}

void RegressionTest::doFailureReport( const TQString& test, int failures )
{
    if ( failures == NoFailure ) {
        ::unlink( TQFile::encodeName( m_outputDir + "/" + test + "-compare.html" ) );
        return;
    }

    createLink( test, failures );

    if ( failures & JSFailure ) {
        doJavascriptReport( test );
        return; // no support for both kind
    }

    TQFile compare( m_outputDir + "/" + test + "-compare.html" );

    TQString testFile = TQFileInfo(test).fileName();

    TQString renderDiff;
    TQString domDiff;

    TQString relOutputDir = makeRelativePath(m_baseDir, m_outputDir);

    // are blocking reads possible with TDEProcess?
    char pwd[PATH_MAX];
    (void) getcwd( pwd, PATH_MAX );
    chdir( TQFile::encodeName( m_baseDir ) );

    if ( failures & RenderFailure ) {
        renderDiff += "<pre>";
        FILE *pipe = popen( TQString::fromLatin1( "diff -u baseline/%1-render %3/%2-render" )
                            .arg ( test, test, relOutputDir ).latin1(), "r" );
        TQTextIStream *is = new TQTextIStream( pipe );
        for ( int line = 0; line < 100 && !is->eof(); ++line ) {
            TQString line = is->readLine();
            line = line.replace( '<', "&lt;" );
            line = line.replace( '>', "&gt;" );
            renderDiff += line + "\n";
        }
        delete is;
        pclose( pipe );
        renderDiff += "</pre>";
    }

    if ( failures & DomFailure ) {
        domDiff += "<pre>";
        FILE *pipe = popen( TQString::fromLatin1( "diff -u baseline/%1-dom %3/%2-dom" )
                            .arg ( test, test, relOutputDir ).latin1(), "r" );
        TQTextIStream *is = new TQTextIStream( pipe );
        for ( int line = 0; line < 100 && !is->eof(); ++line ) {
            TQString line = is->readLine();
            line = line.replace( '<', "&lt;" );
            line = line.replace( '>', "&gt;" );
            domDiff += line  + "\n";
        }
        delete is;
        pclose( pipe );
        domDiff += "</pre>";
    }

    chdir( pwd );

    // create a relative path so that it works via web as well. ugly
    TQString relpath = makeRelativePath(m_outputDir + "/"
        + TQFileInfo(test).dirPath(), m_baseDir);

    compare.open( IO_WriteOnly|IO_Truncate );
    TQString cl;
    cl = TQString( "<html><head><title>%1</title>" ).arg( test );
    cl += TQString( "<script>\n"
                  "var pics = new Array();\n"
                  "pics[0]=new Image();\n"
                  "pics[0].src = '%1';\n"
                  "pics[1]=new Image();\n"
                  "pics[1].src = '%2';\n"
                  "var doflicker = 1;\n"
                  "var t = 1;\n"
                  "var lastb=0;\n" )
          .arg( relpath+"/baseline/"+test+"-dump.png" )
          .arg( testFile+"-dump.png" );
    cl += TQString( "function toggleVisible(visible) {\n"
                  "     document.getElementById('render').style.visibility= visible == 'render' ? 'visible' : 'hidden';\n"
                  "     document.getElementById('image').style.visibility= visible == 'image' ? 'visible' : 'hidden';\n"
                  "     document.getElementById('dom').style.visibility= visible == 'dom' ? 'visible' : 'hidden';\n"
                  "}\n"
                  "function show() { document.getElementById('image').src = pics[t].src; "
                  "document.getElementById('image').style.borderColor = t && !doflicker ? 'red' : 'gray';\n"
                  "toggleVisible('image');\n"
                   "}" );
    cl += TQString ( "function runSlideShow(){\n"
                  "   document.getElementById('image').src = pics[t].src;\n"
                  "   if (doflicker)\n"
                  "       t = 1 - t;\n"
                  "   setTimeout('runSlideShow()', 200);\n"
                  "}\n"
                  "function m(b) { if (b == lastb) return; document.getElementById('b'+b).className='buttondown';\n"
                  "                var e = document.getElementById('b'+lastb);\n"
                  "                 if(e) e.className='button';\n"
                  "                 lastb = b;\n"
                  "}\n"
                  "function showRender() { doflicker=0;toggleVisible('render')\n"
                  "}\n"
                  "function showDom() { doflicker=0;toggleVisible('dom')\n"
                  "}\n"
                   "</script>\n");

    cl += TQString ("<style>\n"
                   ".buttondown { cursor: pointer; padding: 0px 20px; color: white; background-color: blue; border: inset blue 2px;}\n"
                   ".button { cursor: pointer; padding: 0px 20px; color: black; background-color: white; border: outset blue 2px;}\n"
                   ".diff { position: absolute; left: 10px; top: 100px; visibility: hidden; border: 1px black solid; background-color: white; color: black; /* width: 800; height: 600; overflow: scroll; */ }\n"
                   "</style>\n" );

    if ( failures & PaintFailure )
        cl += TQString( "<body onload=\"m(1); show(); runSlideShow();\"" );
    else if ( failures & RenderFailure )
        cl += TQString( "<body onload=\"m(4); toggleVisible('render');\"" );
    else
        cl += TQString( "<body onload=\"m(5); toggleVisible('dom');\"" );
    cl += TQString(" text=black bgcolor=gray>\n<h1>%3</h1>\n" ).arg( test );
    if ( failures & PaintFailure )
        cl += TQString ( "<span id='b1' class='buttondown' onclick=\"doflicker=1;show();m(1)\">FLICKER</span>&nbsp;\n"
                        "<span id='b2' class='button' onclick=\"doflicker=0;t=0;show();m(2)\">BASE</span>&nbsp;\n"
                        "<span id='b3' class='button' onclick=\"doflicker=0;t=1;show();m(3)\">OUT</span>&nbsp;\n" );
    if ( renderDiff.length() )
        cl += "<span id='b4' class='button' onclick='showRender();m(4)'>R-DIFF</span>&nbsp;\n";
    if ( domDiff.length() )
        cl += "<span id='b5' class='button' onclick='showDom();m(5);'>D-DIFF</span>&nbsp;\n";
    // The test file always exists - except for checkOutput called from *.js files
    if ( TQFile::exists( m_baseDir + "/tests/"+ test ) )
        cl += TQString( "<a class=button href=\"%1\">HTML</a>&nbsp;" )
              .arg( relpath+"/tests/"+test );

    cl += TQString( "<hr>"
                   "<img style='border: solid 5px gray' src=\"%1\" id='image'>" )
          .arg( relpath+"/baseline/"+test+"-dump.png" );

    cl += "<div id='render' class='diff'>" + renderDiff + "</div>";
    cl += "<div id='dom' class='diff'>" + domDiff + "</div>";

    cl += "</body></html>";
    compare.writeBlock( cl.latin1(), cl.length() );
    compare.close();
}

void RegressionTest::testStaticFile(const TQString & filename)
{
    tqApp->mainWidget()->resize( 800, 600); // restore size

    // Set arguments
    KParts::URLArgs args;
    if (filename.endsWith(".html") || filename.endsWith(".htm")) args.serviceType = "text/html";
    else if (filename.endsWith(".xhtml")) args.serviceType = "application/xhtml+xml";
    else if (filename.endsWith(".xml")) args.serviceType = "text/xml";
    m_part->browserExtension()->setURLArgs(args);
    // load page
    KURL url;
    url.setProtocol("file");
    url.setPath(TQFileInfo(m_baseDir + "/tests/"+filename).absFilePath());
    PartMonitor pm(m_part);
    m_part->openURL(url);
    pm.waitForCompletion();
    m_part->closeURL();

    if ( filename.startsWith( "domts/" ) ) {
        TQString functionname;

        KJS::Completion comp = m_part->jScriptInterpreter()->evaluate("exposeTestFunctionNames();");
        /*
         *  Error handling
         */
        KJS::ExecState *exec = m_part->jScriptInterpreter()->globalExec();
        if ( comp.complType() == ReturnValue || comp.complType() == Normal )
        {
            if (comp.value().isValid() && comp.value().isA(ObjectType) &&
               (Object::dynamicCast(comp.value()).className() == "Array" ) )
            {
                Object argArrayObj = Object::dynamicCast(comp.value());
                unsigned int length = argArrayObj.
                                      get(exec,lengthPropertyName).
                                      toUInt32(exec);
                if ( length == 1 )
                    functionname = argArrayObj.get(exec, 0).toString(exec).qstring();
            }
        }
        if ( functionname.isNull() ) {
            kdDebug() << "DOM " << filename << " doesn't expose 1 function name - ignoring" << endl;
            return;
        }

        KJS::Completion comp2 = m_part->jScriptInterpreter()->evaluate("setUpPage(); " + functionname + "();" );
        bool success = ( comp2.complType() == ReturnValue || comp2.complType() == Normal );
        TQString description = "DOMTS";
        if ( comp2.complType() == Throw ) {
            KJS::Value val = comp2.value();
            KJS::Object obj = Object::dynamicCast(val);
            if ( obj.isValid() && obj.hasProperty( exec, "jsUnitMessage" ) )
                description = obj.get( exec, "jsUnitMessage" ).toString( exec ).qstring();
            else
                description = comp2.value().toString( exec ).qstring();
        }
        reportResult( success,  description );

        if (!success && !m_known_failures)
            doFailureReport( filename, JSFailure );
        return;
    }

    int back_known_failures = m_known_failures;

    if ( m_genOutput ) {
        if ( m_known_failures & DomFailure)
            m_known_failures = AllFailure;
        reportResult( checkOutput(filename+"-dom"), "DOM" );
        if ( m_known_failures & RenderFailure )
            m_known_failures = AllFailure;
        reportResult( checkOutput(filename+"-render"), "RENDER" );
        if ( m_known_failures & PaintFailure )
            m_known_failures = AllFailure;
        renderToImage().save(m_baseDir + "/baseline/" + filename + "-dump.png","PNG", 60);
        printf("Generated %s\n", TQString( m_baseDir + "/baseline/" + filename + "-dump.png" ).latin1() );
        reportResult( true, "PAINT" );
    } else {
        int failures = NoFailure;

        // compare with output file
        if ( m_known_failures & DomFailure)
            m_known_failures = AllFailure;
        if ( !reportResult( checkOutput(filename+"-dom"), "DOM" ) )
            failures |= DomFailure;

        if ( m_known_failures & RenderFailure )
            m_known_failures = AllFailure;
        if ( !reportResult( checkOutput(filename+"-render"), "RENDER" ) )
            failures |= RenderFailure;

        if ( m_known_failures & PaintFailure )
            m_known_failures = AllFailure;
        if (!reportResult( checkPaintdump(filename), "PAINT") )
            failures |= PaintFailure;

        doFailureReport(filename, failures );
    }

    m_known_failures = back_known_failures;
}

void RegressionTest::evalJS( ScriptInterpreter &interp, const TQString &filename, bool report_result )
{
    TQString fullSourceName = filename;
    TQFile sourceFile(fullSourceName);

    if (!sourceFile.open(IO_ReadOnly)) {
        fprintf(stderr,"Error reading file %s\n",fullSourceName.latin1());
        exit(1);
    }

    TQTextStream stream ( &sourceFile );
    stream.setEncoding( TQTextStream::UnicodeUTF8 );
    TQString code = stream.read();
    sourceFile.close();

    saw_failure = false;
    ignore_errors = false;
    Completion c = interp.evaluate(UString( code ) );

    if ( report_result && !ignore_errors) {
        bool expected_failure = filename.endsWith( "-n.js" );
        if (c.complType() == Throw) {
            TQString errmsg = c.value().toString(interp.globalExec()).qstring();
            if ( !expected_failure ) {
                printf( "ERROR: %s (%s)\n",filename.latin1(), errmsg.latin1());
                m_errors++;
            } else {
                reportResult( true, TQString( "Expected Failure: %1" ).arg( errmsg ) );
            }
        } else if ( saw_failure ) {
            if ( !expected_failure )
                doFailureReport( m_currentCategory + "/" + m_currentTest, JSFailure );
            reportResult( expected_failure, "saw 'failed!'" );
        } else {
            reportResult( !expected_failure, "passed" );
        }
    }
}

class GlobalImp : public ObjectImp {
public:
  virtual UString className() const { return "global"; }
};

void RegressionTest::testJSFile(const TQString & filename )
{
    tqApp->mainWidget()->resize( 800, 600); // restore size

    // create interpreter
    // note: this is different from the interpreter used by the part,
    // it contains regression test-specific objects & functions
    Object global(new GlobalImp());
    tdehtml::ChildFrame frame;
    frame.m_part = m_part;
    ScriptInterpreter interp(global,&frame);
    ExecState *exec = interp.globalExec();

    global.put(exec, "part", Object(new TDEHTMLPartObject(exec,m_part)));
    global.put(exec, "regtest", Object(new RegTestObject(exec,this)));
    global.put(exec, "debug", Object(new RegTestFunction(exec,this,RegTestFunction::Print,1) ) );
    global.put(exec, "print", Object(new RegTestFunction(exec,this,RegTestFunction::Print,1) ) );

    TQStringList dirs = TQStringList::split( '/', filename );
    // NOTE: the basename is of little interest here, but the last basedir change
    // isn't taken in account
    TQString basedir =  m_baseDir + "/tests/";
    for ( TQStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it )
    {
        if ( ! ::access( TQFile::encodeName( basedir + "shell.js" ), R_OK ) )
            evalJS( interp, basedir + "shell.js", false );
        basedir += *it + "/";
    }
    evalJS( interp, m_baseDir + "/tests/"+ filename, true );
}

RegressionTest::CheckResult RegressionTest::checkPaintdump(const TQString &filename)
{
    TQString againstFilename( filename + "-dump.png" );
    TQString absFilename = TQFileInfo(m_baseDir + "/baseline/" + againstFilename).absFilePath();
    if ( svnIgnored( absFilename ) ) {
        m_known_failures = NoFailure;
        return Ignored;
    }
    CheckResult result = Failure;

    TQImage baseline;
    baseline.load( absFilename, "PNG");
    TQImage output = renderToImage();
    if ( !imageEqual( baseline, output ) ) {
        TQString outputFilename = m_outputDir + "/" + againstFilename;
        createMissingDirs(outputFilename );

        bool kf = false;
        if ( m_known_failures & AllFailure )
            kf = true;
        else if ( m_known_failures & PaintFailure )
            kf = true;
        if ( kf )
            outputFilename += "-KF";

        output.save(outputFilename, "PNG", 60);
    }
    else {
        ::unlink( TQFile::encodeName( m_outputDir + "/" + againstFilename ) );
        result = Success;
    }
    return result;
}

RegressionTest::CheckResult RegressionTest::checkOutput(const TQString &againstFilename)
{
    TQString absFilename = TQFileInfo(m_baseDir + "/baseline/" + againstFilename).absFilePath();
    if ( svnIgnored( absFilename ) ) {
        m_known_failures = NoFailure;
        return Ignored;
    }

    bool domOut = againstFilename.endsWith( "-dom" );
    TQString data = getPartOutput( domOut ? DOMTree : RenderTree );
    data.remove( char( 13 ) );

    CheckResult result = Success;

    // compare result to existing file
    TQString outputFilename = TQFileInfo(m_outputDir + "/" + againstFilename).absFilePath();
    bool kf = false;
    if ( m_known_failures & AllFailure )
        kf = true;
    else if ( domOut && ( m_known_failures & DomFailure ) )
        kf = true;
    else if ( !domOut && ( m_known_failures & RenderFailure ) )
        kf = true;
    if ( kf )
        outputFilename += "-KF";

    if ( m_genOutput )
        outputFilename = absFilename;

    TQFile file(absFilename);
    if (file.open(IO_ReadOnly)) {
        TQTextStream stream ( &file );
        stream.setEncoding( TQTextStream::UnicodeUTF8 );

        TQString fileData = stream.read();

        result = ( fileData == data ) ? Success : Failure;
        if ( !m_genOutput && result == Success ) {
            ::unlink( TQFile::encodeName( outputFilename ) );
            return Success;
        }
    }

    // generate result file
    createMissingDirs( outputFilename );
    TQFile file2(outputFilename);
    if (!file2.open(IO_WriteOnly)) {
        fprintf(stderr,"Error writing to file %s\n",outputFilename.latin1());
        exit(1);
    }

    TQTextStream stream2(&file2);
    stream2.setEncoding( TQTextStream::UnicodeUTF8 );
    stream2 << data;
    if ( m_genOutput )
        printf("Generated %s\n", outputFilename.latin1());

    return result;
}

bool RegressionTest::reportResult(CheckResult result, const TQString & description)
{
    if ( result == Ignored ) {
        //printf("IGNORED: ");
        //printDescription( description );
        return true; // no error
    } else
        return reportResult( result == Success, description );
}

bool RegressionTest::reportResult(bool passed, const TQString & description)
{
    if (m_genOutput)
	return true;

   if (passed) {
        if ( m_known_failures & AllFailure ) {
            printf("PASS (unexpected!): ");
            m_passes_fail++;
        } else {
            printf("PASS: ");
            m_passes_work++;
        }
    }
    else {
        if ( m_known_failures & AllFailure ) {
            printf("FAIL (known): ");
            m_failures_fail++;
            passed = true; // we knew about it
        } else {
            printf("FAIL: ");
            m_failures_work++;
        }
    }

    printDescription( description );
    return passed;
}

void RegressionTest::printDescription(const TQString& description)
{
    if (!m_currentCategory.isEmpty())
	printf("%s/", m_currentCategory.latin1());

    printf("%s", m_currentTest.latin1());

    if (!description.isEmpty()) {
        TQString desc = description;
        desc.replace( '\n', ' ' );
	printf(" [%s]", desc.latin1());
    }

    printf("\n");
    fflush(stdout);
}

void RegressionTest::createMissingDirs(const TQString & filename)
{
    TQFileInfo dif(filename);
    TQFileInfo dirInfo( dif.dirPath() );
    if (dirInfo.exists())
	return;

    TQStringList pathComponents;
    TQFileInfo parentDir = dirInfo;
    pathComponents.prepend(parentDir.absFilePath());
    while (!parentDir.exists()) {
	TQString parentPath = parentDir.absFilePath();
	int slashPos = parentPath.findRev('/');
	if (slashPos < 0)
	    break;
	parentPath = parentPath.left(slashPos);
	pathComponents.prepend(parentPath);
	parentDir = TQFileInfo(parentPath);
    }
    for (uint pathno = 1; pathno < pathComponents.count(); pathno++) {
	if (!TQFileInfo(pathComponents[pathno]).exists() &&
	    !TQDir(pathComponents[pathno-1]).mkdir(pathComponents[pathno])) {
	    fprintf(stderr,"Error creating directory %s\n",pathComponents[pathno].latin1());
	    exit(1);
	}
    }
}

void RegressionTest::slotOpenURL(const KURL &url, const KParts::URLArgs &args)
{
    m_part->browserExtension()->setURLArgs( args );

    PartMonitor pm(m_part);
    m_part->openURL(url);
    pm.waitForCompletion();
}

bool RegressionTest::svnIgnored( const TQString &filename )
{
    TQFileInfo fi( filename );
    TQString ignoreFilename = fi.dirPath() + "/svnignore";
    TQFile ignoreFile(ignoreFilename);
    if (!ignoreFile.open(IO_ReadOnly))
        return false;

    TQTextStream ignoreStream(&ignoreFile);
    TQString line;
    while (!(line = ignoreStream.readLine()).isNull()) {
        if ( line == fi.fileName() )
            return true;
    }
    ignoreFile.close();
    return false;
}

void RegressionTest::resizeTopLevelWidget( int w, int h )
{
    tqApp->mainWidget()->resize( w, h );
    // Since we're not visible, this doesn't have an immediate effect, TQWidget posts the event
    TQApplication::sendPostedEvents( 0, TQEvent::Resize );
}

#include "test_regression.moc"
