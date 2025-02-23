/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kanq <taj@kde.org>

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
#ifndef _TDEGLOBAL_H
#define _TDEGLOBAL_H

#include "tdelibs_export.h"
#include <kinstance.h> // KDE4: class TDEInstance is enough here

class KCharsets;
class TDEConfig;
class TDESharedConfig;
class TDEIconLoader;
#ifdef __TDE_HAVE_TDEHWLIB
class TDEHardwareDevices;
class TDEGlobalNetworkManager;
#endif
class TDELocale;
class TDEStandardDirs;
class KStaticDeleterBase;
class KStaticDeleterList;
class KStringDict;
class TQString;

/**
 * Access to the KDE global objects.
 * TDEGlobal provides you with pointers of many central
 * objects that exist only once in the process. It is also
 * responsible for managing instances of KStaticDeleterBase.
 *
 * @see KStaticDeleterBase
 * @author Sirtaj Singh Kang (taj@kde.org)
 */
class TDECORE_EXPORT TDEGlobal
{
public:

    /**
     * Returns the global instance.  There is always at least
     * one instance of a component in one application (in most
     * cases the application itself).
     * @return the global instance
     */
    static TDEInstance            *instance();

    /**
     *  Returns the application standard dirs object.
     * @return the global standard dir object
     */
    static TDEStandardDirs	*dirs();

    /**
     *  Returns the general config object.
     * @return the global configuration object.
     */
    static TDEConfig		*config();

    /**
     *  Returns the general config object.
     * @return the global configuration object.
     */
    static TDESharedConfig        *sharedConfig();

    /**
     *  Returns an iconloader object.
     * @return the global iconloader object
     */
    static TDEIconLoader	        *iconLoader();

#ifdef __TDE_HAVE_TDEHWLIB
    /**
     *  Returns a TDEHardwareDevices object.
     * @return the global hardware devices object
     */
    static TDEHardwareDevices	*hardwareDevices();

    /**
     *  Returns a TDEGlobalNetworkManager object.
     * @return the global network manager object
     */
    static TDEGlobalNetworkManager   *networkManager();
#endif

    /**
     * Returns the global locale object.
     * @return the global locale object
     */
    static TDELocale              *locale();

    /**
     * The global charset manager.
     * @return the global charset manager
     */
    static KCharsets	        *charsets();

    /**
     * Creates a static TQString.
     *
     * To be used inside functions(!) like:
     * \code
     * static const TQString &myString = TDEGlobal::staticQString("myText");
     * \endcode
     *
     * !!! Do _NOT_ use: !!!
     * \code
     * static TQString myString = TDEGlobal::staticQString("myText");
     * \endcode
     * This creates a static object (instead of a static reference)
     * and as you know static objects are EVIL.
     * @param str the string to create
     * @return the static string
     */
    static const TQString        &staticQString(const char *str);

    /**
     * Creates a static TQString.
     *
     * To be used inside functions(!) like:
     * \code
     * static const TQString &myString = TDEGlobal::staticQString(i18n("My Text"));
     * \endcode
     *
     * !!! Do _NOT_ use: !!!
     * \code
     * static TQString myString = TDEGlobal::staticQString(i18n("myText"));
     * \endcode
     * This creates a static object (instead of a static reference)
     * and as you know static objects are EVIL.
     * @param str the string to create
     * @return the static string
     */
    static const TQString        &staticQString(const TQString &str);

    /**
     * Registers a static deleter.
     * @param d the static deleter to register
     * @see KStaticDeleterBase
     * @see KStaticDeleter
     */
    static void registerStaticDeleter(KStaticDeleterBase *d);

    /**
     * Unregisters a static deleter.
     * @param d the static deleter to unregister
     * @see KStaticDeleterBase
     * @see KStaticDeleter
     */
    static void unregisterStaticDeleter(KStaticDeleterBase *d);

    /**
     * Calls KStaticDeleterBase::destructObject() on all
     * registered static deleters and unregisters them all.
     * @see KStaticDeleterBase
     * @see KStaticDeleter
     */
    static void deleteStaticDeleters();

    //private:
    static  KStringDict         *_stringDict;
    static  TDEInstance           *_instance;
    static  TDELocale             *_locale;
    static  KCharsets	        *_charsets;
    static  KStaticDeleterList  *_staticDeleters;

    /**
     * The instance currently active (useful in a multi-instance
     * application, such as a KParts application).
     * Don't use this - it's mainly for TDEAboutDialog and KBugReport.
     * @internal
     */
    static void setActiveInstance(TDEInstance *d);
    static TDEInstance *activeInstance() { return _activeInstance; }

    static  TDEInstance           *_activeInstance;
};

/**
 * \relates TDEGlobal
 * A typesafe function to find the minimum of the two arguments.
 */
#define KMIN(a,b)	kMin(a,b)
/**
 * \relates TDEGlobal
 * A typesafe function to find the maximum of the two arguments.
 */
#define KMAX(a,b)	kMax(a,b)
/**
 * \relates TDEGlobal
 * A typesafe function to determine the absolute value of the argument.
 */
#define KABS(a)	kAbs(a)
/**
 * \relates TDEGlobal
 * A typesafe function that returns x if it's between low and high values.
 * low if x is smaller than then low and high if x is bigger than high.
 */
#define KCLAMP(x,low,high) kClamp(x,low,high)

// XXX KDE4: Make kMin, kMax and kClamp return "T" instead of "const T &"!
template<class T>
inline const T& kMin (const T& a, const T& b) { return a < b ? a : b; }

template<class T>
inline const T& kMax (const T& a, const T& b) { return b < a ? a : b; }

template<class T>
inline T kAbs (const T& a) { return a < 0 ? -a : a; }

template<class T>
inline const T& kClamp( const T& x, const T& low, const T& high )
{
    if ( x < low )       return low;
    else if ( high < x ) return high;
    else                 return x;
}

/**
 * Locale-independent tqstricmp. Use this for comparing ascii keywords
 * in a case-insensitive way.
 * tqstricmp fails with e.g. the Turkish locale where 'I'.lower() != 'i'
 * @since 3.4
 */
TDECORE_EXPORT int kasciistricmp( const char *str1, const char *str2 );

/**
  Locale-independent function to convert ASCII strings to lower case ASCII
  strings. This means that it affects @em only the ASCII characters A-Z.

  @param str  pointer to the string which should be converted to lower case
  @return     pointer to the converted string (same as @a str)
*/
TDECORE_EXPORT char* kasciitolower( char *str );

/**
  Locale-independent function to convert ASCII strings to upper case ASCII
  strings. This means that it affects @em only the ASCII characters a-z.

  @param str  pointer to the string which should be converted to upper case
  @return     pointer to the converted string (same as @a str)
*/
TDECORE_EXPORT char* kasciitoupper( char *str );
 

/**
 * \mainpage The KDE Core Functionality Library
 *
 * All KDE programs use this library to provide basic functionality such
 * as the configuration system, IPC, internationalization and locale
 * support, site-independent access to the filesystem and a large number
 * of other (but no less important) things.
 *
 * All KDE applications should link to the tdecore library. Also, using a
 * TDEApplication derived class instead of TQApplication is almost
 * mandatory if you expect your application to behave nicely within the
 * KDE environment.
 */

#endif // _TDEGLOBAL_H

