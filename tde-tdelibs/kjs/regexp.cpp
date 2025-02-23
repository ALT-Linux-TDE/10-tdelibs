/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003,2004 Apple Computer, Inc.
 *  Copyright (C) 2006      Maksim Orlovich (maksim@kde.org)
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

#include "regexp.h"

#include "lexer.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace KJS;

RegExp::UTF8SupportState RegExp::utf8Support = RegExp::Unknown;

RegExp::RegExp(const UString &p, int f)
  : pat(p), flgs(f), m_notEmpty(false), valid(true), buffer(0), originalPos(0)
{
  // Determine whether libpcre has unicode support if need be..
  if (utf8Support == Unknown) {
    uint32_t supported;
    pcre2_config(PCRE2_CONFIG_COMPILED_WIDTHS, (void*)&supported);
    utf8Support = (supported & 0x0001) ? Supported : Unsupported;
  }

  nrSubPatterns = 0; // determined in match() with POSIX regex.

  // JS regexps can contain Unicode escape sequences (\uxxxx) which
  // are rather uncommon elsewhere. As our regexp libs don't understand
  // them we do the unescaping ourselves internally.
  // Also make sure to expand out any nulls as pcre_compile 
  // expects null termination..
  UString intern;
  const char* const nil = "\\x00";
  if (p.find('\\') >= 0 || p.find(KJS::UChar('\0')) >= 0) {
    bool escape = false;
    for (int i = 0; i < p.size(); ++i) {
      UChar c = p[i];
      if (escape) {
        escape = false;
        // we only care about \u
        if (c == 'u') {
    // standard unicode escape sequence looks like \uxxxx but
    // other browsers also accept less then 4 hex digits
    unsigned short u = 0;
    int j = 0;
    for (j = 0; j < 4; ++j) {
      if (i + 1 < p.size() && Lexer::isHexDigit(p[i + 1].unicode())) {
        u = (u << 4) + Lexer::convertHex(p[i + 1].unicode());
        ++i;
      } else {
        // sequence incomplete. restore index.
        // TODO: cleaner way to propagate warning
        fprintf(stderr, "KJS: saw %d digit \\u sequence.\n", j);
        i -= j;
        break;
      }
    }
    if (j < 4) {
      // sequence was incomplete. treat \u as u which IE always
      // and FF sometimes does.
      intern.append(UString('u'));
    } else {
            c = UChar(u);
            switch (u) {
            case 0:
        // Make sure to encode 0, to avoid terminating the string
        intern += UString(nil);
        break;
            case '^':
            case '$':
            case '\\':
            case '.':
            case '*':
            case '+':
            case '?':
            case '(': case ')':
            case '{': case '}':
            case '[': case ']':
            case '|':
        // escape pattern characters have to remain escaped
        intern.append(UString('\\'));
        // intentional fallthrough
            default:
        intern += UString(&c, 1);
        break;
      }
          }
          continue;
        }
        intern += UString('\\');
        intern += UString(&c, 1);
      } else {
        if (c == '\\')
          escape = true;
        else if (c == '\0')
          intern += UString(nil);
        else
          intern += UString(&c, 1);
      }
    }
  } else {
    intern = p;
  }

#ifdef HAVE_PCRE2POSIX
  uint32_t pcre2flags = 0;
  int errorCode;
  PCRE2_SIZE errorOffset;

  if (flgs & IgnoreCase)
    pcre2flags |= PCRE2_CASELESS;

  if (flgs & Multiline)
    pcre2flags |= PCRE2_MULTILINE;

  if (utf8Support == Supported)
    pcre2flags |= (PCRE2_UTF | PCRE2_NO_UTF_CHECK);

  // Fill our buffer with an encoded version, whether utf-8, or, 
  // if PCRE is incapable, truncated.
  prepareMatch(intern);

  pcregex = pcre2_compile(buffer, PCRE2_ZERO_TERMINATED, pcre2flags,
       &errorCode, &errorOffset, NULL);
  doneMatch(); // Cleanup buffers
  if (!pcregex) {
#ifndef NDEBUG
    PCRE2_UCHAR errorMsg[256];
    pcre2_get_error_message(errorCode, errorMsg, sizeof(errorMsg));
    fprintf(stderr, "KJS: pcre_compile() failed with '%s'\n", errorMsg);
#endif
    match_data = nullptr;
    valid = false;
    return;
  }

  // Get number of subpatterns that will be returned
  int rc = pcre2_pattern_info(pcregex, PCRE2_INFO_CAPTURECOUNT, &nrSubPatterns);
  if (rc != 0)
  {
    nrSubPatterns = 0; // fallback. We always need the first pair of offsets.
  }

  match_data = pcre2_match_data_create_from_pattern(pcregex, NULL);
#else

  int regflags = 0;
#ifdef REG_EXTENDED
  regflags |= REG_EXTENDED;
#endif
#ifdef REG_ICASE
  if ( f & IgnoreCase )
    regflags |= REG_ICASE;
#endif

  //NOTE: Multiline is not feasible with POSIX regex.
  //if ( f & Multiline )
  //    ;
  // Note: the Global flag is already handled by RegExpProtoFunc::execute

  int errorCode = regcomp(&preg, intern.ascii(), regflags);
  if (errorCode != 0) {
#ifndef NDEBUG
    char errorMessage[80];
    regerror(errorCode, &preg, errorMessage, sizeof errorMessage);
    fprintf(stderr, "KJS: regcomp failed with '%s'\n", errorMessage);
#endif
    valid = false;
  }
#endif
}

RegExp::~RegExp()
{
  doneMatch(); // Be 100% sure buffers are freed
#ifdef HAVE_PCRE2POSIX
  if (match_data)
  {
    pcre2_match_data_free(match_data);
  }
  if (pcregex)
  {
    pcre2_code_free(pcregex);
  }
#else
  /* TODO: is this really okay after an error ? */
  regfree(&preg);
#endif
}

void RegExp::prepareUtf8(const UString& s)
{
  // Allocate a buffer big enough to hold all the characters plus \0
  const int length = s.size();
  buffer = new buftype_t[length * 3 + 1];

  // Also create buffer for positions. We need one extra character in there,
  // even past the \0 since the non-empty handling may jump one past the end
  originalPos = new int[length * 3 + 2];

  // Convert to runs of 8-bit characters, and generate indeces
  // Note that we do NOT combine surrogate pairs here, as 
  // regexps operate on them as separate characters
  buftype_t *p = buffer;
  int  *posOut = originalPos;
  const UChar *d = s.data();
  for (int i = 0; i != length; ++i) {
    unsigned short c = d[i].unicode();

    int sequenceLen;
    if (c < 0x80) {
      *p++ = (buftype_t)c;
      sequenceLen = 1;
    } else if (c < 0x800) {
      *p++ = (buftype_t)((c >> 6) | 0xC0); // C0 is the 2-byte flag for UTF-8
      *p++ = (buftype_t)((c | 0x80) & 0xBF); // next 6 bits, with high bit set
      sequenceLen = 2;
    } else {
      *p++ = (buftype_t)((c >> 12) | 0xE0); // E0 is the 3-byte flag for UTF-8
      *p++ = (buftype_t)(((c >> 6) | 0x80) & 0xBF); // next 6 bits, with high bit set
      *p++ = (buftype_t)((c | 0x80) & 0xBF); // next 6 bits, with high bit set
      sequenceLen = 3;
    }

    while (sequenceLen > 0) {
      *posOut = i;
      ++posOut;
      --sequenceLen;
    }
  }

  bufferSize = p - buffer;

  *p++ = '\0';

  // Record positions for \0, and the fictional character after that.
  *posOut     = length;
  *(posOut+1) = length+1;
}

void RegExp::prepareASCII (const UString& s)
{
  originalPos = 0;

  // Best-effort attempt to get something done
  // when we don't have utf 8 available -- use 
  // truncated version, and pray for the best 
  CString truncated = s.cstring();
  buffer = new buftype_t[truncated.size() + 1];
  memcpy(buffer, truncated.c_str(), truncated.size());
  buffer[truncated.size()] = '\0'; // For _compile use
  bufferSize = truncated.size();
}

void RegExp::prepareMatch(const UString &s)
{
  delete[] originalPos; // Just to be sure..
  delete[] buffer;
  if (utf8Support == Supported)
    prepareUtf8(s);
  else
    prepareASCII(s);

#ifndef NDEBUG
  originalS = s;
#endif
}

void RegExp::doneMatch() 
{
  delete[] originalPos; originalPos = 0;
  delete[] buffer;      buffer      = 0;
}

UString RegExp::match(const UString &s, int i, int *pos, int **ovector)
{
#ifndef NDEBUG
  assert(s.data() == originalS.data()); // Make sure prepareMatch got called right..
#endif
  assert(valid);

  if (i < 0)
    i = 0;
  if (ovector)
    *ovector = 0L;
  int dummyPos;
  if (!pos)
    pos = &dummyPos;
  *pos = -1;
  if (i > s.size() || s.isNull())
    return UString::null;

#ifdef HAVE_PCRE2POSIX
  if (!pcregex || !match_data)
    return UString::null;
  if (!ovector)
    return UString::null;

  int startPos;
  int nextPos;
  if (utf8Support == Supported)
  {
    startPos = i;
    while (originalPos[startPos] < i)
      ++startPos;

    nextPos = startPos;
    if (i < s.size()) {
      while (originalPos[nextPos] < (i + 1))
        ++nextPos;
    }
  }
  else
  {
    startPos = i;
    nextPos  = i + (i < s.size() ? 1 : 0);
  }

  uint32_t baseFlags = (utf8Support == Supported ? PCRE2_NO_UTF_CHECK : 0);
  if (m_notEmpty)
  {
    baseFlags |= PCRE2_NOTEMPTY | PCRE2_ANCHORED;
  }
  int numMatches = pcre2_match(pcregex, buffer, PCRE2_ZERO_TERMINATED, startPos, baseFlags, match_data, NULL);
  if (numMatches <= 0)
  {
    // Failed to match.
    if (numMatches == PCRE2_ERROR_NOMATCH && (flgs & Global) && m_notEmpty && startPos < nextPos)
    {
      // We set m_notEmpty ourselves, to look for a non-empty match
      // So we don't stop here, we want to try again at i+1.
#ifdef KJS_VERBOSE
      fprintf(stderr, "No match after m_notEmpty. +1 and keep going.\n");
#endif
      m_notEmpty = 0;
      baseFlags = (utf8Support == Supported ? PCRE2_NO_UTF_CHECK : 0);
      numMatches = pcre2_match(pcregex, buffer, PCRE2_ZERO_TERMINATED, nextPos, baseFlags, match_data, NULL);
      if (numMatches <= 0)
        return UString::null;
    }
    else
      return UString::null;
  }

  PCRE2_SIZE *pcre2_ovector = pcre2_get_ovector_pointer(match_data);
  if (!pcre2_ovector)
    return UString::null;

  uint32_t pcre2_ovecCount = pcre2_get_ovector_count(match_data);
  *ovector = new int[pcre2_ovecCount * 2];
  if (originalPos)
  {
    for (size_t c = 0; c < 2 * pcre2_ovecCount; ++c)
    {
      (*ovector)[c] = (pcre2_ovector[c] != -1) ? originalPos[pcre2_ovector[c]] : -1;
    }
  }
  else
  {
    for (size_t c = 0; c < 2 * pcre2_ovecCount; ++c)
    {
      (*ovector)[c] = pcre2_ovector[c];
    }
  }
#else
  const uint maxMatch = 10;
  regmatch_t rmatch[maxMatch];

  char *str = strdup(s.ascii()); // TODO: why ???
  if (regexec(&preg, str + i, maxMatch, rmatch, 0)) {
    free(str);
    return UString::null;
  }
  free(str);

  if (!ovector) {
    *pos = rmatch[0].rm_so + i;
    return s.substr(rmatch[0].rm_so + i, rmatch[0].rm_eo - rmatch[0].rm_so);
  }

  // map rmatch array to ovector used in PCRE case
  nrSubPatterns = 0;
  for (uint j = 0; j < maxMatch && rmatch[j].rm_so >= 0 ; j++) {
    nrSubPatterns++;
    // if the nonEmpty flag is set, return a failed match if any of the
    // subMatches happens to be an empty string.
    if (m_notEmpty && rmatch[j].rm_so == rmatch[j].rm_eo) 
      return UString::null;
  }
  // Allow an ovector slot to return the (failed) match result.
  if (nrSubPatterns == 0) nrSubPatterns = 1;
  
  int ovecsize = (nrSubPatterns)*3; // see above
  *ovector = new int[ovecsize];
  for (uint j = 0; j < nrSubPatterns; j++) {
      (*ovector)[2*j] = rmatch[j].rm_so + i;
      (*ovector)[2*j+1] = rmatch[j].rm_eo + i;
  }
#endif

  *pos = (*ovector)[0];
  if ( *pos == (*ovector)[1] && (flgs & Global) )
  {
    // empty match, next try will be with m_notEmpty=true
    m_notEmpty=true;
  }
  return s.substr((*ovector)[0], (*ovector)[1] - (*ovector)[0]);
}
