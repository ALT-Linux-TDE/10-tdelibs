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
/*
* tdeglobal.cpp -- Implementation of class TDEGlobal.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sat May  1 02:08:43 EST 1999
*/

#include <tqglobal.h>
#include <tqdict.h>
#include <tqptrlist.h>
#include "tdeglobal.h"

#include <tdeapplication.h>
#include <tdeaboutdata.h>
#include <kdebug.h>
#include <tdeconfig.h>
#include <tdelocale.h>
#include <kcharsets.h>
#include <kiconloader.h>
#ifdef __TDE_HAVE_TDEHWLIB
#include <tdehardwaredevices.h>
#include <tdenetworkconnections.h>
#endif
#include <kstandarddirs.h>
#include <kinstance.h>
#include "kstaticdeleter.h"

#include <tqfont.h>

#ifndef NDEBUG
#define MYASSERT(x) if (!x) \
   tqFatal("Fatal error: you need to have a TDEInstance object before\n" \
         "you do anything that requires it! Examples of this are config\n" \
         "objects, standard directories or translations.");
#else
#define MYASSERT(x) /* nope */
#endif

static void kglobal_init();

TDEStandardDirs *TDEGlobal::dirs()
{
    MYASSERT(_instance);

    return _instance->dirs();
}

TDEConfig	*TDEGlobal::config()
{
    MYASSERT(_instance);

    return _instance->config();
}

TDESharedConfig *TDEGlobal::sharedConfig()
{
    MYASSERT(_instance);

    return _instance->sharedConfig();
}

TDEIconLoader *TDEGlobal::iconLoader()
{
    MYASSERT(_instance);

    return _instance->iconLoader();
}

#ifdef __TDE_HAVE_TDEHWLIB
TDEHardwareDevices *TDEGlobal::hardwareDevices()
{
    MYASSERT(_instance);

    return _instance->hardwareDevices();
}

TDEGlobalNetworkManager *TDEGlobal::networkManager()
{
    MYASSERT(_instance);

    return _instance->networkManager();
}
#endif

TDEInstance *TDEGlobal::instance()
{
    MYASSERT(_instance);
    return _instance;
}

TDELocale	*TDEGlobal::locale()
{
    if( _locale == 0 ) {
	if (!_instance)
	   return 0;
        kglobal_init();

        // will set _locale if it works - otherwise 0 is returned
        TDELocale::initInstance();
        if( _instance->aboutData())
            _instance->aboutData()->translateInternalProgramName();
    }

    return _locale;
}

KCharsets *TDEGlobal::charsets()
{
    if( _charsets == 0 ) {
        _charsets =new KCharsets();
        kglobal_init();
    }

    return _charsets;
}

void TDEGlobal::setActiveInstance(TDEInstance *i)
{
    _activeInstance = i;
    if (i && _locale)
	_locale->setActiveCatalogue(TQString::fromUtf8(i->instanceName()));
}

/**
 * Create a static TQString
 *
 * To be used inside functions(!) like:
 * static const TQString &myString = TDEGlobal::staticQString("myText");
 */
const TQString &
TDEGlobal::staticQString(const char *str)
{
   return staticQString(TQString::fromLatin1(str));
}

class KStringDict : public TQDict<TQString>
{
public:
   KStringDict() : TQDict<TQString>(139) { }
};

/**
 * Create a static TQString
 *
 * To be used inside functions(!) like:
 * static const TQString &myString = TDEGlobal::staticQString(i18n("My Text"));
 */
const TQString &
TDEGlobal::staticQString(const TQString &str)
{
    if (!_stringDict) {
      _stringDict = new KStringDict;
      _stringDict->setAutoDelete( true );
      kglobal_init();
    }
   TQString *result = _stringDict->find(str);
   if (!result)
   {
      result = new TQString(str);
      _stringDict->insert(str, result);
   }
   return *result;
}

class KStaticDeleterList: public TQPtrList<KStaticDeleterBase>
{
public:
   KStaticDeleterList() { }
};

void
TDEGlobal::registerStaticDeleter(KStaticDeleterBase *obj)
{
   if (!_staticDeleters)
      kglobal_init();
   if (_staticDeleters->find(obj) == -1)
      _staticDeleters->append(obj);
}

void
TDEGlobal::unregisterStaticDeleter(KStaticDeleterBase *obj)
{
   if (_staticDeleters)
      _staticDeleters->removeRef(obj);
}

void
TDEGlobal::deleteStaticDeleters()
{
    if (!TDEGlobal::_staticDeleters)
        return;

    for(;_staticDeleters->count();)
    {
        _staticDeleters->take(0)->destructObject();
    }

    delete TDEGlobal::_staticDeleters;
    TDEGlobal::_staticDeleters = 0;
}

// The Variables

KStringDict     *TDEGlobal::_stringDict   = 0;
TDEInstance       *TDEGlobal::_instance     = 0;
TDEInstance       *TDEGlobal::_activeInstance = 0;
TDELocale         *TDEGlobal::_locale	= 0;
KCharsets       *TDEGlobal::_charsets	= 0;
KStaticDeleterList *TDEGlobal::_staticDeleters = 0;

#ifdef WIN32
#include <windows.h>
static void kglobal_freeAll();
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID impLoad )
{
    if (reason == DLL_PROCESS_DETACH)
        kglobal_freeAll();
    return TRUE;
}
#else
__attribute__((destructor))
#endif
static void kglobal_freeAll()
{
    delete TDEGlobal::_locale;
    TDEGlobal::_locale = 0;
    delete TDEGlobal::_charsets;
    TDEGlobal::_charsets = 0;
    delete TDEGlobal::_stringDict;
    TDEGlobal::_stringDict = 0;
    TDEGlobal::deleteStaticDeleters();
    // so that we don't hold a reference and see memory leaks :/
    TDEGlobal::setActiveInstance(0);
}

static void kglobal_init()
{
    if (TDEGlobal::_staticDeleters)
        return;

    TDEGlobal::_staticDeleters = new KStaticDeleterList;
}

int kasciistricmp( const char *str1, const char *str2 )
{
    const unsigned char *s1 = (const unsigned char *)str1;
    const unsigned char *s2 = (const unsigned char *)str2;
    int res;
    unsigned char c1, c2;

    if ( !s1 || !s2 )
        return s1 ? 1 : (s2 ? -1 : 0);
    if ( !*s1 || !*s2 )
        return *s1 ? 1 : (*s2 ? -1 : 0);
    for (;*s1; ++s1, ++s2) {
        c1 = *s1; c2 = *s2;
        if (c1 >= 'A' && c1 <= 'Z')
            c1 += 'a' - 'A';
        if (c2 >= 'A' && c2 <= 'Z')
            c2 += 'a' - 'A';

        if ((res = c1 - c2))
            break;
    }
    return *s1 ? res : (*s2 ? -1 : 0);
}

char* kasciitolower( char *s )
{
  if ( !s )
    return 0;
  for ( unsigned char *p = (unsigned char *) s; *p; ++p )
    *p = ( *p >= 'A' && *p <= 'Z' ) ? (*p - 'A' + 'a') : *p;
  return s;
}

char* kasciitoupper( char *s )
{
  if ( !s )
    return 0;
  for ( unsigned char *p = (unsigned char *) s; *p; ++p )
    *p = ( *p >= 'a' && *p <= 'z' ) ? (*p - 'a' + 'A') : *p;
  return s;
}
