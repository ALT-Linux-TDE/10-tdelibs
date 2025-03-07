/*
 * tester.h
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 * Copyright (C)  2005  Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TESTER_H
#define TESTER_H

/*! @mainpage KUnitTest - a UnitTest library for KDE
 *
 * @section contents Contents
 * @li @ref background
 * @li @ref usage
 * @li @ref integration
 * @li @ref module
 * @li @ref advanced
 * @li @ref scripts
 *
 * @section background Background
 *
 * KUnitTest is based on the "in reality no one wants to write tests and
 * if it takes a lot of code no one will. So the less code to write the
 * better" design principle.
 *
 * Copyright and credits:
 * @li (C) 2004 Zack Rusin (original author)
 * @li Brad Hards (import into CVS)
 * @li (C) 2005 Jeroen Wijnhout (GUI, library, module)
 *
 * You are responsible for what you do with it though. It
 * is licensed under a BSD license - read the top of each file.
 *
 * All the GUI related stuff is in tdesdk/tdeunittest, the core libraries are in tdelibs/tdeunittest.
 * A simple example modules is in kdelisbs/tdeunittest/samplemodule.{h,cpp}, however more examples
 * can be found in tdesdk/tdeunittest/example.
 *
 * There are roughly two ways to use the KUnitTest library. Either you create dynamically
 * loadable modules and use the tdeunittestmodrunner or tdeunittestguimodrunner programs to run
 * the tests, or you use the tdeunittest/tdeunittestgui library to create your own test runner
 * application.
 *
 * The main parts of the KUnitTest library are:
 * @li runner.{h,cpp} - it is the tester runner, holds all tests and runs
 * them.
 * @li runnergui.{h,cpp} - the GUI wrapper around the runner. The GUI neatly organizes the
 *   test results. With the tdeunittest helper script it can even add the debug output
 *   to the test results. For this you need to have the tdesdk module installed.
 * @li tester.h - which holds the base of a pure test object (Tester).
 * @li module.h - defines macros to create a dynamically loadable module.
 *
 * @section usage Example usage
 *
 * This section describes how to use the library to create your own tests and runner
 * application.
 *
 * Now lets see how you would add a new test to KUnitTest. You do that by
 * writting a Tester derived class which has an "allTests()" method:
 *
 * @code
 * class SampleTest : public Tester
 * {
 * public:
 *    SampleTest();
 *
 *    void allTests();
 * };
 * @endcode
 *
 * Now in the allTests() method we implement our tests, which might look
 * like:
 *
 * @code
 * void SampleTest::allTests()
 * {
 *    CHECK( 3+3, 6 );
 *    CHECK( TQString( "hello%1" ).arg( " world not" ), TQString( "hello world" ) );
 * }
 * @endcode
 *
 * CHECK() is implemented using a template, so you get type safe
 * comparison. All that is needed is that the argument types have an
 * @c operator==() defined.
 *
 * Now that you did that the only other thing to do is to tell the
 * framework to add this test case, by using the TDEUNITTEST_REGISTER_TESTER(x) macro. Just
 * put the following line in the implementation file:
 *
 * @code TDEUNITTEST_REGISTER_TESTER( SampleTest ); @endcode
 *
 * Note the ;, it is necessary.
 *
 * KUnitTest will do the rest. It will tell you which tests failed, how, what was the expected
 * result, what was the result it got, what was the code that failed and so on. For example for
 * the code above it would output:
 *
 * @verbatim
SampleTest - 1 test passed, 1 test failed
    Unexpected failure:
        sampletest.cpp[38]: failed on "TQString( "hello%1" ).arg( " world not" )"
            result = 'hello world not', expected = 'hello world'
@endverbatim
 *
 * If you use the RunnerGUI class then you will be presented with a scrollable list of the
 * test results.
 *
 * @section integration Integration
 *
 * The KUnitTest library is easy to use. Let's say that you have the tests written in the
 * sampletest.h and sampletest.cpp files. Then all you need is a main.cpp file and a Makefile.am.
 * You can copy both from the example file provided with the library. A typical main.cpp file
 * looks like this:
 *
 * @code
 * #include <tdeaboutdata.h>
 * #include <tdeapplication.h>
 * #include <tdecmdlineargs.h>
 * #include <tdecmdlineargs.h>
 * #include <tdelocale.h>
 * #include <tdeunittest/runnergui.h>
 *
 * static const char description[] = I18N_NOOP("SampleTests");
 * static const char version[] = "0.1";
 * static TDECmdLineOptions options[] = { TDECmdLineLastOption };
 *
 * int main( int argc, char** argv )
 * {
 *     TDEAboutData about("SampleTests", I18N_NOOP("SampleTests"), version, description,
 *                     TDEAboutData::License_BSD, "(C) 2005 You!", 0, 0, "mail@provider");
 *
 *     TDECmdLineArgs::init(argc, argv, &about);
 *     TDECmdLineArgs::addCmdLineOptions( options );
 *     TDEApplication app;
 *
 *     KUnitTest::RunnerGUI runner(0);
 *     runner.show();
 *     app.setMainWidget(&runner);
 *
 *     return app.exec();
 * }
 * @endcode
 *
 * The Makefile.am file will look like:
 *
 * @code
 * INCLUDES = -I$(top_srcdir)/src $(all_includes)
 * METASOURCES = AUTO
 * check_PROGRAMS = sampletests
 * sampletests_SOURCES = main.cpp sampletest.cpp
 * sampletests_LDFLAGS = $(KDE_RPATH) $(all_libraries)
 * sampletests_LDADD = -ltdeunittest
 * noinst_HEADERS = sampletest.h
 *
 * check:
 *    tdeunittest $(top_builddir)/src/sampletests SampleTests
 * @endcode
 *
 * Most of this Makefile.am will be self-explanatory. After running
 * "make check" the binary "sampletests" will be built. The reason for
 * adding the extra make target "check" is that you probably do not want
 * to rebuild the test suite everytime you run make.
 *
 * You can run the binary on its own, but you get more functionality if you use
 * the tdeunittest helper script. The Makefile.am is set up in such
 * a way that this helper script is automatically run after you do a
 * "make check". This scripts take two arguments, the first is the path
 * to the binary to run. The second the application name, in this case SampleTests.
 * This name is important since it is used to let the script communicate with the application
 * via DCOP. The helper scripts relies on the Perl DCOP bindings, so these need to be installed.
 *
 * @section module Creating test modules
 *
 * If you think that writing your own test runner if too much work then you can also
 * use the tdeunittestermodrunner application or the kunitguimodrunner script to run
 * the tests for you. You do have to put your tests in a dynamically loadable module though.
 * Fortunately KUnitTest comes with a few macros to help you do this.
 *
 * First the good news, you don't have to change the header file sampletest.h. However, we
 * will rename it to samplemodule.h, so we remember we are making a module. The
 * implementation file should be rename to samplemodule.cpp. This file requires some
 * modifications. First we need to include the module.h header:
 *
 * @code
 * #include <tdeunittest/module.h>
 * @endcode
 *
 * This header file is needed because it defines some macro you'll need. In fact this is
 * how you use them:
 *
 * @code
 * TDEUNITTEST_MODULE( tdeunittest_samplemodule, "Tests for sample module" );
 * TDEUNITTEST_MODULE_REGISTER_TESTER( SimpleSampleTester );
 * TDEUNITTEST_MODULE_REGISTER_TESTER( SomeSampleTester );
 * @endcode
 *
 * The first macro, TDEUNITTEST_MODULE(), makes sure that the module can be loaded and that
 * the test classes are created. The first argument "tdeunittest_samplemodule" is the library
 * name, in this case the library we're creating a tdeunittest_samplemodule.la module. The
 * second argument is name which will appear in the test runner for this test suite.
 *
 * The tester class are now added by the TDEUNITTEST_MODULE_REGISTER_TESTER() macro, not the
 * TDEUNITTEST_REGISTER_TESTER(). The only difference between the two is that you have to
 * pass the module class name to this macro.
 *
 * The Makefile.am is also a bit different, but not much:
 *
 * @code
 * INCLUDES = -I$(top_srcdir)/include $(all_includes)
 * METASOURCES = AUTO
 * check_LTLIBRARIES = tdeunittest_samplemodule.la
 * tdeunittest_samplemodule_la_SOURCES = samplemodule.cpp
 * tdeunittest_samplemodule_la_LIBADD = $(LIB_TDEUNITTEST)
 * tdeunittest_samplemodule_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)
 * @endcode
 *
 * The $(KDE_CHECK_PLUGIN) macro is there to make sure a dynamically loadable
 * module is created.
 *
 * After you have built the module you open a Konsole and cd into the build folder. Running
 * the tests in the module is now as easy as:
 *
 * @code
 * $ make check && tdeunittestmodrunner
 * @endcode
 *
 * The tdeunittestmodrunner application loads all tdeunittest_*.la modules in the current
 * directory. The exit code of this console application is the number of unexpected failures.
 *
 * If you want the GUI, you should use the tdeunittestmod script:
 *
 * @code
 * $ make check && tdeunittestmod
 * @endcode
 *
 * This script starts tdeunittestguimodrunner application and a helper script to take
 * care of dealing with debug output.
 *
 * @section advanced Advanced usage
 *
 * Normally you just want to use CHECK(). If you are developing some more
 * tests, and they are run (or not) based on some external dependency,
 * you may need to skip some tests. In this case, rather than doing
 * nothing (or worse, writing a test step that aborts the test run), you
 * might want to use SKIP() to record that. Note that this is just a
 * logging / reporting tool, so you just pass in a string:
 *
 * @code
 *     SKIP( "Test skipped because of lack of foo support." );
 * @endcode
 *
 * Similarly, you may have a test step that you know will fail, but you
 * don't want to delete the test step (because it is showing a bug), but
 * equally you can't fix it right now (eg it would break binary
 * compatibility, or would violate a string freeze). In that case, it
 * might help to use XFAIL(), for "expected failure". The test will still
 * be run, and recorded as a failure (assuming it does fail), but will
 * also be recorded separately. Usage might be as follows:
 *
 * @code
 *     XFAIL( 2+1, 4 );
 * @endcode
 *
 * You can mix CHECK(), SKIP() and XFAIL() within a single Tester derived
 * class.
 *
 *
 * @section exceptions Exceptions
 *
 * KUnitTest comes with simple support for testing whether an exception, such as a function call,
 * throws an exception or not. Simply, for the usual macros there corresponding ones for
 * exception testing: CHECK_EXCEPTION(), XFAIL_EXCEPTION(), and SKIP_EXCEPTION(). They all take two
 * arguments: the expression that will catch the exception, and the expression that is supposed
 * to throw the exception.
 *
 * For example:
 *
 * @code
 * CHECK_EXCEPTION(EvilToothFairyException *, myFunction("I forgot to brush my teeth!"));
 * @endcode
 *
 * @note The exception is not de-allocated in anyway.
 *
 * The macros does not allow introspection of the exceptions, such as testing a supplied
 * identifier code on the exception object or similar; this requires manual coding, such
 * as custom macros.
 *
 * @section scripts Scripts
 *
 * The library comes with several helper scripts:
 *
 * @li tdeunittest [app] [dcopobject] : Runs the application app and redirects all debug output to the dcopobject.
 * @li tdeunittestmod --folder [folder] --query [query] : Loads and runs all modules in the folder matching the query. Use a GUI.
 * @li tdeunittest_debughelper [dcopobject] : A PERL script that is able to redirect debug output to a RunnerGUI instance.
 *
 * These scripts are part of the tdesdk/tdeunittest module.
 */

/*!
 * @file tester.h
 * Defines macros for unit testing as well as some test classes.
 */

#include <iostream>
using namespace std;

#include <tqobject.h>
#include <tqstringlist.h>
#include <tqasciidict.h>

#include <tdelibs_export.h>

/*! @def CHECK(x,y)
 * Use this macro to perform an equality check. For example
 *
 * @code CHECK( numberOfErrors(), 0 ); @endcode
 */
#define CHECK( x, y ) check( __FILE__, __LINE__, #x, x, y, false )

/// for source-compat with qttestlib: use COMPARE(x,y) if you plan to port to qttestlib later.
#define COMPARE CHECK

/// for source-compat with qttestlib: use VERIFY(x) if you plan to port to qttestlib later.
#define VERIFY( x ) CHECK( x, true )

/*! @def XFAIL(x,y)
 * Use this macro to perform a check you expect to fail. For example
 *
 * @code XFAIL( numberOfErrors(), 1 ); @endcode
 *
 * If the test fails, it will be counted as such, however it will
 * also be registered separately.
 */
#define XFAIL( x, y ) check( __FILE__, __LINE__, #x, x, y, true )

/*! @def SKIP(x)
 * Use this macro to indicate that a test is skipped.
 *
 * @code SKIP("Test skipped because of lack of foo support."); @endcode
 */
#define SKIP( x ) skip( __FILE__, __LINE__, TQString::fromLatin1(#x))

/*!
 * A macro testing that @p expression throws an exception that is catched
 * with @p exceptionCatch. Use it to test that an expression, such as a function call,
 * throws a certain exception.
 * 
 * @note this macro assumes it's used in a function which is a sub-class of the Tester class.
 */
#define CHECK_EXCEPTION(exceptionCatch, expression) \
    try \
    { \
        expression; \
    } \
    catch(exceptionCatch) \
    { \
        setExceptionRaised(true); \
    } \
    if(exceptionRaised()) \
    { \
        success(TQString(__FILE__) + "[" + TQString::number(__LINE__) + "]: passed " + #expression); \
    } \
    else \
    { \
        failure(TQString(__FILE__) + "[" + TQString::number(__LINE__) + TQString("]: failed to throw " \
                "an exception on: ") + #expression); \
    } \
    setExceptionRaised(false);

/*!
 * This macro is similar to XFAIL, but is for exceptions instead. Flags @p expression
 * as being expected to fail to throw an exception that @p exceptionCatch is supposed to catch.
 */
#define XFAIL_EXCEPTION(exceptionCatch, expression) \
    try \
    { \
        expression; \
    } \
    catch(exceptionCatch) \
    { \
        setExceptionRaised(true); \
    } \
    if(exceptionRaised()) \
    { \
        unexpectedSuccess(TQString(__FILE__) + "[" + TQString::number(__LINE__) + "]: unexpectedly threw an exception and passed: " + #expression); \
    }\
    else \
    { \
        expectedFailure(TQString(__FILE__) + "[" + TQString::number(__LINE__) + TQString("]: failed to throw an exception on: ") + #expression); \
    } \
    setExceptionRaised(false);

/*!
 * This macro is similar to SKIP, but is for exceptions instead. Skip testing @p expression
 * and the @p exceptionCatch which is supposed to catch the exception, and register the test
 * as being skipped.
 */
#define SKIP_EXCEPTION(exceptionCatch, expression) \
	skip( __FILE__, __LINE__, TQString("Exception catch: ")\
			.arg(TQString(#exceptionCatch)).arg(TQString(" Test expression: ")).arg(TQString(#expression)))

/**
 * Namespace for Unit testing classes
 */
namespace KUnitTest
{
    /*! A simple class that encapsulates a test result. A Tester class usually
     * has a single TestResults instance associated with it, however the SlotTester
     * class can have more TestResults instances (one for each test slot in fact).
     */
    class TDEUNITTEST_EXPORT TestResults
    {
        friend class Tester;

    public:
        TestResults() : m_tests( 0 ) {}

        virtual ~TestResults() {}

        /*! Clears the test results and debug info. Normally you do not need to call this.
         */
        virtual void clear()
        {
            m_errorList.clear();
            m_xfailList.clear();
            m_xpassList.clear();
            m_skipList.clear();
            m_successList.clear();
            m_debug = "";
            m_tests = 0;
        }

        /*! Add some debug info that can be view later. Normally you do not need to call this.
         * @param debug The debug info.
         */
        virtual void addDebugInfo(const TQString &debug)
        {
            m_debug += debug;
        }

        /*! @returns The debug info that was added to this Tester object.
         */
        TQString debugInfo() const { return m_debug; }

        /*! @returns The number of finished tests. */
        int testsFinished() const { return m_tests; }

        /*! @returns The number of failed tests. */
        int errors() const { return m_errorList.count(); }

        /*! @returns The number of expected failures. */
        int xfails() const { return m_xfailList.count(); }

        /*! @returns The number of unexpected successes. */
        int xpasses() const { return m_xpassList.count(); }

        /*! @returns The number of skipped tests. */
        int skipped() const { return m_skipList.count(); }

        /*! @returns The number of passed tests. */
        int passed() const { return m_successList.count(); }

        /*! @returns Details about the failed tests. */
        TQStringList errorList() const { return m_errorList; }

        /*! @returns Details about tests that failed expectedly. */
        TQStringList xfailList() const { return m_xfailList; }

        /*! @returns Details about tests that succeeded unexpectedly. */
        TQStringList xpassList() const { return m_xpassList; }

        /*! @returns Details about which tests were skipped. */
        TQStringList skipList() const { return m_skipList; }

        /*! @returns Details about the succeeded tests. */
        TQStringList successList() const { return m_successList; }

    private:
        TQStringList m_errorList;
        TQStringList m_xfailList;
        TQStringList m_xpassList;
        TQStringList m_skipList;
        TQStringList m_successList;
        TQString     m_debug;
        int         m_tests;
    };

    typedef TQAsciiDict<TestResults> TestResultsListType;

    /*! A type that can be used to iterate through the registry. */
    typedef TQAsciiDictIterator<TestResults> TestResultsListIteratorType;

    /*! The abstract Tester class forms the base class for all test cases. Users must
     * implement the void Tester::allTests() method. This method contains the actual test.
     *
     * Use the CHECK(x,y), XFAIL(x,y) and SKIP(x) macros in the allTests() method
     * to perform the tests.
     *
     * @see CHECK, XFAIL, SKIP
     */
    class TDEUNITTEST_EXPORT Tester : public TQObject
    {
    public:
        Tester(const char *name = 0L)
        : TQObject(0L, name), m_results(new TestResults()), m_exceptionState(false)
        {}

        virtual ~Tester() { delete m_results; }

    public:
        /*! Implement this method with the tests and checks you want to perform.
         */
        virtual void allTests() = 0;

    public:
        /*! @return The TestResults instance.
         */
        virtual TestResults *results() { return m_results; }

    protected:
        /*! This is called when the SKIP(x) macro is used.
         * @param file A C-string containing the name of the file where the skipped tests resides. Typically the __FILE__ macro is used to retrieve the filename.
         * @param line The linenumber in the file @p file. Use the __LINE__ macro for this.
         * @param msg The message that identifies the skipped test.
         */
        void skip( const char *file, int line, TQString msg )
        {
            TQString skipEntry;
            TQTextStream ts( &skipEntry, IO_WriteOnly );
            ts << file << "["<< line <<"]: " << msg;
            skipTest( skipEntry );
        }

        /*! This is called when the CHECK or XFAIL macro is used.
         * @param file A C-string containing the name of the file where the skipped tests resides. Typically the __FILE__ macro is used to retrieve the filename.
         * @param line The linenumber in the file @p file. Use the __LINE__ macro for this.
         * @param str The message that identifies the skipped test.
         * @param result The result of the test.
         * @param expectedResult The expected result.
         * @param expectedFail Indicates whether or not a failure is expected.
         */
        template<typename T>
        void check( const char *file, int line, const char *str,
                    const T  &result, const T &expectedResult,
                    bool expectedFail )
        {
            cout << "check: " << file << "["<< line <<"]" << endl;

            if ( result != expectedResult )
            {
                TQString error;
                TQTextStream ts( &error, IO_WriteOnly );
                ts << file << "["<< line <<"]: failed on \"" <<  str
                   <<"\" result = '" << result << "' expected = '" << expectedResult << "'";

                if ( expectedFail )
                    expectedFailure( error );
                else
                    failure( error );

            }
            else
            {
                // then the test passed, but we want to record it if
                // we were expecting a failure
                if (expectedFail)
                {
                    TQString err;
                    TQTextStream ts( &err, IO_WriteOnly );
                    ts << file << "["<< line <<"]: "
                       <<" unexpectedly passed on \""
                       <<  str <<"\"";
                    unexpectedSuccess( err );
                }
                else
                {
                    TQString succ;
                    TQTextStream ts( &succ, IO_WriteOnly );
                    ts << file << "["<< line <<"]: "
                       <<" passed \""
                       <<  str <<"\"";
                    success( succ );
                }
            }

            ++m_results->m_tests;
        }

    /*!
     * This function can be used to flag succeeding tests, when 
     * doing customized tests while not using the check function.
     *
     * @param message the message describing what failed. Should be informative, 
     * such as mentioning the expression that failed and where, the file and file number.
     */
    void success(const TQString &message) { m_results->m_successList.append(message); }

    /*!
     * This function can be used to flag failing tests, when 
     * doing customized tests while not using the check function.
     *
     * @param message the message describing what failed. Should be informative, 
     * such as mentioning the expression that failed and where, the file name and file number.
     */
    void failure(const TQString &message) { m_results->m_errorList.append(message); }

    /*!
     * This function can be used to flag expected failures, when 
     * doing customized tests while not using the check function.
     *
     * @param message the message describing what failed. Should be informative, 
     * such as mentioning the expression that failed and where, the file name and file number.
     */
    void expectedFailure(const TQString &message) { m_results->m_xfailList.append(message); }

    /*!
     * This function can be used to flag unexpected successes, when 
     * doing customized tests while not using the check function.
     *
     * @param message the message describing what failed. Should be informative, 
     * such as mentioning the expression that failed and where, the file name and file number.
     */
    void unexpectedSuccess(const TQString &message) { m_results->m_xpassList.append(message); }

    /*!
     * This function can be used to flag a test as skipped, when 
     * doing customized tests while not using the check function.
     *
     * @param message the message describing what failed. Should be informative, 
     * such as mentioning the expression that failed and where, the file name and file number.
     */
    void skipTest(const TQString &message) { m_results->m_skipList.append(message); }

    /*!
     * exceptionRaised and exceptionState are book-keeping functions for testing for
     * exceptions. setExceptionRaised sets an internal boolean to true.
     *
     * @see exceptionRaised
     * @param state the new 
     */
    void setExceptionRaised(bool state) { m_exceptionState = state; }

    /*!
     * Returns what the currently tested exception state.
     *
     * @see setExceptionRaised
     */
    bool exceptionRaised() const
    {
	return m_exceptionState;
    }

    protected:
        TestResults *m_results;

    private:

	bool m_exceptionState;
    };

    /*! The SlotTester class is a special Tester class, one that will
     * execute all slots that start with the string "test". The method
     * void allTests() is implemented and should not be overriden.
     */
    class TDEUNITTEST_EXPORT SlotTester : public Tester
    {
        TQ_OBJECT

    public:
        SlotTester(const char *name = 0L);

        void allTests();

        TestResults *results(const char *sl);

        TestResultsListType &resultsList() { return m_resultsList; }

    signals:
        void invoke();

    private:
        void invokeMember(const TQString &str);

        TestResultsListType  m_resultsList;
        TestResults         *m_total;
    };
}

TDEUNITTEST_EXPORT TQTextStream& operator<<( TQTextStream& str, const TQRect& r );

TDEUNITTEST_EXPORT TQTextStream& operator<<( TQTextStream& str, const TQPoint& r );

TDEUNITTEST_EXPORT TQTextStream& operator<<( TQTextStream& str, const TQSize& r );

#endif
