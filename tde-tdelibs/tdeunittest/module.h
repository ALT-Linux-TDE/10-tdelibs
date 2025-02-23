/*
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

/*!
 * @file module.h
 * Provides macros to ease building unit tests as shared libraries
 */

#ifndef TDEUNITTEST_MODULE_H
#define TDEUNITTEST_MODULE_H

#include <tqstring.h>

#include <klibloader.h>
#include <tdeunittest/runner.h>

namespace KUnitTest
{
    /*! @def TDEUNITTEST_MODULE(library,suite)
    * Use this macro if you are creating a KUnitTest module named library. 
    * This macro creates a module-class named a factory class. The module
    * will appear under the name suite in the test runner.
    * There is no need in calling the K_EXPORT_COMPONENT_FACTORY macro,
    * this is taken care of automatically.
    * 
    * @code TDEUNITTEST_MODULE(tdeunittest_samplemodule,"TestSuite") @endcode
    */
    #define TDEUNITTEST_MODULE(library,suite)                                                 \
    static const TQString s_tdeunittest_suite  = TQString::fromLatin1(suite);                   \
    class library##Module : public TQObject                                                  \
    {                                                                                       \
    public:                                                                                 \
        library##Module()                                                                   \
        {                                                                                   \
            KUnitTest::RegistryIteratorType it(s_registry);                                 \
            for( ; it.current(); ++it )                                                     \
                KUnitTest::Runner::registerTester(it.currentKey(), it.current());           \
        }                                                                                   \
                                                                                            \
        static KUnitTest::RegistryType s_registry;                                          \
    };                                                                                      \
                                                                                            \
    KUnitTest::RegistryType library##Module::s_registry;                                    \
                                                                                            \
    void tdeunittest_registerModuleTester(const char *name, KUnitTest::Tester *test)          \
    {                                                                                       \
        library##Module::s_registry.insert(name, test);                                     \
    }                                                                                       \
                                                                                            \
    class module##Factory : public KLibFactory                                              \
    {                                                                                       \
    public:                                                                                 \
        TQObject *createObject (TQObject *, const char *, const char *, const TQStringList &)  \
        {                                                                                   \
            return new library##Module();                                                   \
        };                                                                                  \
    };                                                                                      \
                                                                                            \
    K_EXPORT_COMPONENT_FACTORY( library, module##Factory )

    /*! @def TDEUNITTEST_MODULE_REGISTER_TESTER(tester)
    * Use this macro to add a tester class to your module. The name of the tester will
    * be identical to the class name.
    *
    * @code TDEUNITTEST_MODULE_REGISTER_TESTER(SimpleSampleTester) @endcode
    */
    #define TDEUNITTEST_MODULE_REGISTER_TESTER( tester)                                           \
    static class tester##ModuleAutoregister                                                     \
    {                                                                                           \
    public:                                                                                     \
        tester##ModuleAutoregister()                                                            \
        {                                                                                       \
            KUnitTest::Tester *test = new tester();                                             \
            TQString name = s_tdeunittest_suite + TQString::fromLatin1("::") + TQString::fromLocal8Bit(#tester); \
            test->setName(name.local8Bit());                                                    \
            tdeunittest_registerModuleTester(name.local8Bit(), test );                            \
        }                                                                                       \
    } tester##ModuleAutoregisterInstance;

    /*! @def TDEUNITTEST_MODULE_REGISTER_NAMEDTESTER(name,tester)
    * Use this macro to add a tester class, with specified name, to your module..
    *
    * @code TDEUNITTEST_MODULE_REGISTER_TESTER("SubSuite::PrettyName",SimpleSampleTester) @endcode
    */
    #define TDEUNITTEST_MODULE_REGISTER_NAMEDTESTER( name , tester)                             \
    static class tester##ModuleAutoregister                                                   \
    {                                                                                         \
    public:                                                                                   \
        tester##ModuleAutoregister()                                                          \
        {                                                                                     \
            TQString fullName = s_tdeunittest_suite + TQString("::") + TQString::fromLocal8Bit(name); \
            KUnitTest::Tester *test = new tester(fullName.local8Bit());                       \
            tdeunittest_registerModuleTester(fullName.local8Bit(), test);                       \
        }                                                                                     \
    } tester##ModuleAutoregisterInstance;
}

#endif
