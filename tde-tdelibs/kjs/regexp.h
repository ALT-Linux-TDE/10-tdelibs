/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef _KJS_REGEXP_H_
#define _KJS_REGEXP_H_

#include <sys/types.h>

#include "config.h"

#ifdef HAVE_PCRE2POSIX
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
typedef PCRE2_UCHAR8 buftype_t; 
#else  // POSIX regex - not so good...
extern "C" { // bug with some libc5 distributions
#include <regex.h>
typedef char buftype_t; 
}
#endif

#include "ustring.h"

namespace KJS {

  class RegExp {
  public:
    enum { None = 0, Global = 1, IgnoreCase = 2, Multiline = 4 };
    RegExp(const UString &p, int f = None);
    ~RegExp();
    int flags() const { return flgs; }
    UString pattern() const { return pat; }
    bool isValid() const { return valid; }
    UString match(const UString &s, int i, int *pos = 0, int **ovector = 0);
    // test is unused. The JS spec says that RegExp.test should use
    // RegExp.exec, so it has to store $1 etc.
    // bool test(const UString &s, int i = -1);
    unsigned int subPatterns() const { return nrSubPatterns; }
    
    //These methods should be called around the match of the same string..
    void prepareMatch(const UString &s);
    void doneMatch();
  private:
    const UString pat;
    int flgs : 8;
    bool m_notEmpty;
    bool valid;
    
    // Cached encoding info...
    buftype_t *buffer;
    int*  originalPos;
    int   bufferSize;

    void prepareUtf8  (const UString& s);
    void prepareASCII (const UString& s);
#ifndef NDEBUG
    UString originalS; // the original string, used for sanity-checking
#endif

#ifndef HAVE_PCRE2POSIX
    regex_t preg;
#else
    pcre2_code *pcregex;
    pcre2_match_data *match_data;
    
    enum UTF8SupportState {
      Unknown,
      Supported,
      Unsupported
    };
    static UTF8SupportState utf8Support;
#endif
    uint32_t nrSubPatterns;

    RegExp();
  };

} // namespace

#endif
