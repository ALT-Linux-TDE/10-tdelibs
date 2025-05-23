/*
 * tdeunittest.h
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
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

/*!
 * @file runner.h
 * Defines a set of macros and classes for running unit tests
 */

#ifndef TDEUNITTEST_RUNNER_H
#define TDEUNITTEST_RUNNER_H

#include <iostream>
using namespace std;

#include <tqobject.h>
#include <tqasciidict.h>
#include <tqstring.h>

#include <tdelibs_export.h>

#include "tester.h"

class TQSocketNotifier;

namespace KUnitTest
{
    /*! @def TDEUNITTEST_SUITE(suite)
     * 
     * This macro must be used if you are not making a test-module. The macro
     * defines the name of the test suite.
     */
    #define TDEUNITTEST_SUITE(suite)\
    static const TQString s_tdeunittest_suite  = suite;

    /*! @def TDEUNITTEST_REGISTER_TESTER( tester )
     * @brief Automatic registration of Tester classes.
     *
     * This macro can be used to register the Tester into the global registry. Use
     * this macro in the implementation file of your Tester class. If you keep the
     * Tester classes in a shared or convenience library then you should not use this
     * macro as this macro relies on the static initialization of a TesterAutoregister class.
     * You can always use the static Runner::registerTester(const char *name, Tester *test) method.
    */
    #define TDEUNITTEST_REGISTER_TESTER( tester )\
    static TesterAutoregister tester##Autoregister( TQString(s_tdeunittest_suite + TQString("::") + TQString::fromLocal8Bit(#tester)).local8Bit() , new tester ())

    #define TDEUNITTEST_REGISTER_NAMEDTESTER( name, tester )\
    static TesterAutoregister tester##Autoregister( TQString(s_tdeunittest_suite + TQString("::") + TQString::fromLocal8Bit(name)).local8Bit() , new tester ())

    /*! The type of the registry. */
    typedef TQAsciiDict<Tester> RegistryType;

    /*! A type that can be used to iterate through the registry. */
    typedef TQAsciiDictIterator<Tester> RegistryIteratorType;
    
    /*! The Runner class holds a list of registered Tester classes and is able
     * to run those test cases. The Runner class follows the singleton design
     * pattern, which means that you can only have one Runner instance. This
     * instance can be retrieved using the Runner::self() method.
     *
     * The registry is an object of type RegistryType, it is able to map the name
     * of a test to a pointer to a Tester object. The registry is also a singleton
     * and can be accessed via Runner::registry(). Since there is only one registry,
     * which can be accessed at all times, test cases can be added without having to
     * worry if a Runner instance is present or not. This allows for a design in which
     * the KUnitTest library can be kept separate from the test case sources. Test cases
     * (classes inheriting from Tester) can be added using the static 
     * registerTester(const char *name, Tester *test) method. Allthough most users
     * will want to use the TDEUNITTEST_REGISTER_TESTER macro.
     *
     * @see TDEUNITTEST_REGISTER_TESTER
     */
    class TDEUNITTEST_EXPORT Runner : public TQObject
    {
        TQ_OBJECT
    
    public:
        /*! Registers a test case. A registry will be automatically created if necessary.
         * @param name The name of the test case.
         * @param test A pointer to a Tester object.
         */
        static void registerTester(const char *name, Tester *test);

        /*! @returns The registry holding all the Tester objects.
         */
        RegistryType &registry();

        /*! @returns The global Runner instance. If necessary an instance will be created.
         */
        static Runner *self();

        /*! @returns The number of registered test cases.
         */
        int numberOfTestCases();

        /*! Load all modules found in the folder. 
         * @param folder The folder where to look for modules.
         * @param query A regular expression. Only modules which match the query will be run.
         */
        static void loadModules(const TQString &folder, const TQString &query);

        /*! The runner can spit out special debug messages needed by the Perl script: tdeunittest_debughelper.
         * This script can attach the debug output of each suite to the results in the KUnitTest GUI.
         * Not very useful for console minded developers, so this static method can be used to disable
         * those debug messages.
         * @param enabled If true the debug messages are enabled (default), otherwise they are disabled.
         */
        static void setDebugCapturingEnabled(bool enabled);
            
    private:
        RegistryType         m_registry;
        static Runner       *s_self;
        static bool          s_debugCapturingEnabled;
    
    protected:
        Runner();
    
    public:
        /*! @returns The number of finished tests. */
        int numberOfTests() const;

        /*! @returns The number of passed tests. */
        int numberOfPassedTests() const;

        /*! @returns The number of failed tests, this includes the number of expected failures. */
        int numberOfFailedTests() const;

        /*! @returns The number of failed tests which were expected. */
        int numberOfExpectedFailures() const;

        /*! @returns The number of skipped tests. */
        int numberOfSkippedTests() const;

    public slots:
        /*! Call this slot to run all the registered tests.
         * @returns The number of finished tests.
         */
        int runTests();

        /*! Call this slot to run a single test.
         * @param name The name of the test case. This name has to correspond to the name
         * that was used to register the test. If the TDEUNITTEST_REGISTER_TESTER macro was
         * used to register the test case then this name is the class name.
         */
        void runTest(const char *name);

        /*! Call this slot to run tests with names starting with prefix.
         * @param prefix Only run tests starting with the string prefix.
         */
        void runMatchingTests(const TQString &prefix);

        /*! Reset the Runner in order to prepare it to run one or more tests again.
         */
        void reset();

    signals:
        /*! Emitted after a test is finished.
         * @param name The name of the test.
         * @param test A pointer to the Tester object.
         */
        void finished(const char *name, Tester *test);
        void invoke();
    
    private:
        void registerTests();
    
    private:
        int globalSteps;
        int globalTests;
        int globalPasses;
        int globalFails;
        int globalXFails;
        int globalXPasses;
        int globalSkipped;
    };
    
    /*! The TesterAutoregister is a helper class to allow the automatic registration
     * of Tester classes.
     */
    class TesterAutoregister
    {
    public:
        /*! @param name A unique name that identifies the Tester class.
         * @param test A pointer to a Tester object.
         */
        TesterAutoregister(const char *name, Tester *test) 
        { 
            if ( test->name() == 0L ) test->setName(name);
            Runner::registerTester(name, test); 
        }
    };

}

#endif
