/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KLIBLOADER_H
#define KLIBLOADER_H

#include <tqobject.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqasciidict.h>
#include <tqptrlist.h>
#include <tdeglobal.h>

#include <stdlib.h> // For backwards compatibility

class TDEInstance;
class TQTimer;
class KLibrary;
class KLibFactory;
class KLibFactoryPrivate;
class KLibLoaderPrivate;
class KLibraryPrivate;

# define K_EXPORT_COMPONENT_FACTORY( libname, factory ) \
    extern "C" { TDE_EXPORT void *init_##libname() { return new factory; } }

/**
 * @short Represents a dynamically loaded library.
 *
 * KLibrary allows you to look up symbols of the shared library.
 * Use KLibLoader to create a new instance of KLibrary.
 *
 * @see KLibLoader
 * @author Torben Weis <weis@kde.org>
 */
class TDECORE_EXPORT KLibrary : public TQObject
{
    friend class KLibLoader;
    friend class TQAsciiDict<KLibrary>;

    TQ_OBJECT
public:
    /**
     * Don't create KLibrary objects on your own. Instead use KLibLoader.
     */
    KLibrary( const TQString& libname, const TQString& filename, void * handle );

    /**
     * Returns the name of the library.
     * @return The name of the library like "libkspread".
     */
    TQString name() const;

    /**
     * Returns the file name of the library.
     * @return The filename of the library, for example "/opt/kde2&/lib/libkspread.la"
     */
    TQString fileName() const;

    /**
     * Returns the factory of the library.
     * @return The factory of the library if there is any, otherwise 0
     */
    KLibFactory* factory();

    /**
     * Looks up a symbol from the library. This is a very low level
     * function that you usually don't want to use. Usually you should
     * check using hasSymbol() whether the symbol actually exists,
     * otherwise a warning will be printed.
     * @param name the name of the symbol to look up
     * @return the address of the symbol, or 0 if it does not exist
     * @see hasSymbol
     */
    void* symbol( const char* name ) const;

    /**
     * Looks up a symbol from the library. This is a very low level
     * function that you usually don't want to use.
     * Unlike symbol(), this method doesn't warn if the symbol doesn't exist,
     * so if the symbol might or might not exist, better use hasSymbol() before symbol().
     * @param name the name of the symbol to check
     * @return true if the symbol exists
     * @since 3.1
     */
    bool hasSymbol( const char* name ) const;

    /**
     * Unloads the library.
     * This typically results in the deletion of this object. You should
     * not reference its pointer after calling this function.
     */
    void unload() const;

private slots:
    void slotObjectCreated( TQObject *obj );
    void slotObjectDestroyed();
    void slotTimeout();

private:
    /**
     * @internal
     * Don't destruct KLibrary objects yourself. Instead use unload() instead.
     */
    ~KLibrary();

    TQString m_libname;
    TQString m_filename;
    KLibFactory* m_factory;
    void * m_handle;
    TQPtrList<TQObject> m_objs;
    TQTimer *m_timer;
    KLibraryPrivate *d;
};

class KLibWrapPrivate;

/**
 * The KLibLoader allows you to load libraries dynamically at runtime.
 * Dependent libraries are loaded automatically.
 *
 * KLibLoader follows the singleton pattern. You can not create multiple
 * instances. Use self() to get a pointer to the loader.
 *
 * @see KLibrary
 * @author Torben Weis <weis@kde.org>
 */
class TDECORE_EXPORT KLibLoader : public TQObject
{
    friend class KLibrary;

    TQ_OBJECT
public:
    /**
     * You should NEVER destruct an instance of KLibLoader
     * until you know what you are doing. This will release
     * the loaded libraries.
     */
    ~KLibLoader();

    /**
     * Loads and initializes a library. Loading a library multiple times is
     * handled gracefully.
     *
     * This is a convenience function that returns the factory immediately
     * @param libname  This is the library name without extension. Usually that is something like
     *                 "libkspread". The function will then search for a file named
     *                 "libkspread.la" in the KDE library paths.
     *                 The *.la files are created by libtool and contain
     *                 important information especially about the libraries dependencies
     *                 on other shared libs. Loading a "libfoo.so" could not solve the
     *                 dependencies problem.
     *
     *                 You can, however, give a library name ending in ".so"
     *                 (or whatever is used on your platform), and the library
     *                 will be loaded without resolving dependencies. Use with caution.
     * @return the KLibFactory, or 0 if the library does not exist or it does
     *         not have a factory
     * @see library
     */
    KLibFactory* factory( const char* libname );

    /**
     * Loads and initializes a library. Loading a library multiple times is
     * handled gracefully.
     *
     * @param libname  This is the library name without extension. Usually that is something like
     *                 "libkspread". The function will then search for a file named
     *                 "libkspread.la" in the KDE library paths.
     *                 The *.la files are created by libtool and contain
     *                 important information especially about the libraries dependencies
     *                 on other shared libs. Loading a "libfoo.so" could not solve the
     *                 dependencies problem.
     *
     *                 You can, however, give a library name ending in ".so"
     *                 (or whatever is used on your platform), and the library
     *                 will be loaded without resolving dependencies. Use with caution.
     * @return KLibrary is invalid (0) when the library couldn't be dlopened. in such
     * a case you can retrieve the error message by calling KLibLoader::lastErrorMessage()
     *
     * @see factory
     */
    virtual KLibrary* library( const char* libname );

    /**
     * Loads and initializes a library. Loading a library multiple times is
     * handled gracefully.   The library is loaded such that the symbols are
     * globally accessible so libraries with dependencies can be loaded
     * sequentially.
     *
     * @param name     This is the library name without extension. Usually that is something like
     *                 "libkspread". The function will then search for a file named
     *                 "libkspread.la" in the KDE library paths.
     *                 The *.la files are created by libtool and contain
     *                 important information especially about the libraries dependencies
     *                 on other shared libs. Loading a "libfoo.so" could not solve the
     *                 dependencies problem.
     *
     *                 You can, however, give a library name ending in ".so"
     *                 (or whatever is used on your platform), and the library
     *                 will be loaded without resolving dependencies. Use with caution.
     * @return KLibrariy is invalid (0) when the library couldn't be dlopened. in such
     * a case you can retrieve the error message by calling KLibLoader::lastErrorMessage()
     *
     * @see factory
     */
    KLibrary* globalLibrary( const char *name );

    /**
     * Returns an error message that can be useful to debug the problem.
     * Returns TQString::null if the last call to library() was successful.
     * You can call this function more than once. The error message is only
     * reset by a new call to library().
     * @return the last error message, or TQString::null if there was no error
     */
    TQString lastErrorMessage() const;

    /**
     * Unloads the library with the given name.
     * @param libname  This is the library name without extension. Usually that is something like
     *                 "libkspread". The function will then search for a file named
     *                 "libkspread.la" in the KDE library paths.
     *                 The *.la files are created by libtool and contain
     *                 important information especially about the libraries dependencies
     *                 on other shared libs. Loading a "libfoo.so" could not solve the
     *                 dependencies problem.
     *
     *                 You can, however, give a library name ending in ".so"
     *                 (or whatever is used on your platform), and the library
     *                 will be loaded without resolving dependencies. Use with caution.
     */
    virtual void unloadLibrary( const char *libname );

    /**
     * Returns a pointer to the factory. Use this function to get an instance
     * of KLibLoader.
     * @return a pointer to the loader. If no loader exists until now
     *         then one is created.
     */
    static KLibLoader* self();

    /**
     * @internal
     * Internal Method, called by the TDEApplication destructor.
     * Do not call it.
     * This is what makes it possible to rely on ~KLibFactory
     * being called in all cases, whether the library is unloaded
     * while the application is running or when exiting.
     */
    static void cleanUp();

    /**
     * Helper method which looks for a library in the standard paths
     * ("module" and "lib" resources).
     * Made public for code that doesn't use KLibLoader itself, but still
     * wants to open modules.
     * @param name of the library. If it is not a path, the function searches in
     *             the "module" and "lib" resources. If there is no extension,
     *             ".la" will be appended.
     * @param instance a TDEInstance used to get the standard paths
     */
    static TQString findLibrary( const char * name, const TDEInstance * instance = TDEGlobal::instance() );

protected:
    KLibLoader( TQObject* parent = 0, const char* name = 0 );

private slots:
    void slotLibraryDestroyed();
private:
    void close_pending( KLibWrapPrivate * );
    TQAsciiDict<KLibWrapPrivate> m_libs;

    static KLibLoader* s_self;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    KLibLoaderPrivate *d;
};

/**
 * If you develop a library that is to be loaded dynamically at runtime, then
 * you should return a pointer to your factory. The K_EXPORT_COMPONENT_FACTORY
 * macro is provided for this purpose:
 * \code
 *   K_EXPORT_COMPONENT_FACTORY( libkspread, KSpreadFactory )
 * \endcode
 *
 * The first macro argument is the name of your library, the second specifies the name
 * of your factory.
 *
 * NOTE: you probably want to use KGenericFactory<PluginClassName>
 * instead of writing your own factory.
 *
 * In the constructor of your factory you should create an instance of TDEInstance
 * like this:
 * \code
 *     s_global = new TDEInstance( "kspread" );
 * \endcode
 * This TDEInstance is comparable to TDEGlobal used by normal applications.
 * It allows you to find resource files (images, XML, sound etc.) belonging
 * to the library.
 *
 * If you want to load a library, use KLibLoader. You can query KLibLoader
 * directly for a pointer to the libraries factory by using the KLibLoader::factory()
 * function.
 *
 * The KLibFactory is used to create the components, the library has to offer.
 * The factory of KSpread for example will create instances of KSpreadDoc,
 * while the Konqueror factory will create KonqView widgets.
 * All objects created by the factory must be derived from TQObject, since TQObject
 * offers type safe casting.
 *
 * KLibFactory is an abstract class. Reimplement the
 * createObject() method to give it functionality.
 *
 * @author Torben Weis <weis@kde.org>
 */
class TDECORE_EXPORT KLibFactory : public TQObject
{
    TQ_OBJECT
public:
    /**
     * Create a new factory.
     * @param parent the parent of the TQObject, 0 for no parent
     * @param name the name of the TQObject, 0 for no name
     */
    KLibFactory( TQObject* parent = 0, const char* name = 0 );
    virtual ~KLibFactory();

    /**
     * Creates a new object. The returned object has to be derived from
     * the requested classname.
     *
     * It is valid behavior to create different kinds of objects
     * depending on the requested @p classname. For example a koffice
     * library may usually return a pointer to KoDocument.  But
     * if asked for a "TQWidget", it could create a wrapper widget,
     * that encapsulates the Koffice specific features.
     *
     * create() automatically emits a signal objectCreated to tell
     * the library about its newly created object.  This is very
     * important for reference counting, and allows unloading the
     * library automatically once all its objects have been destroyed.
     *
     * @param parent the parent of the TQObject, 0 for no parent
     * @param name the name of the TQObject, 0 for no name
     * @param classname the name of the class
     * @param args a list of arguments
     */

     TQObject* create( TQObject* parent = 0, const char* name = 0, const char* classname = "TQObject", const TQStringList &args = TQStringList() );

signals:
    /**
     * Emitted in #create
     * @param obj the new object
     */
    void objectCreated( TQObject *obj );


protected:

    /**
     * Creates a new object. The returned object has to be derived from
     * the requested classname.
     *
     * It is valid behavior to create different kinds of objects
     * depending on the requested @p className. For example a koffice
     * library may usually return a pointer to KoDocument.  But
     * if asked for a "TQWidget", it could create a wrapper widget,
     * that encapsulates the Koffice specific features.
     *
     * This function is called by #create()
     * @param parent the parent of the TQObject, 0 for no parent
     * @param name the name of the TQObject, 0 for no name
     * @param className the name of the class
     * @param args a list of arguments
     */
    virtual TQObject* createObject( TQObject* parent = 0, const char* name = 0,
                                   const char* className = "TQObject",
                                   const TQStringList &args = TQStringList() ) = 0;


protected:
    virtual void virtual_hook( int id, void* data );
private:
    KLibFactoryPrivate *d;
};

#endif
