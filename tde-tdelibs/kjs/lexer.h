/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef _KJSLEXER_H_
#define _KJSLEXER_H_

#include "ustring.h"


namespace KJS {

  class Identifier;

  class RegExp;

  class Lexer {
  public:
    Lexer();
    ~Lexer();
    static Lexer *curr();

    void setCode(const UChar *c, unsigned int len);
    int lex();

    int lineNo() const { return yylineno + 1; }

    bool prevTerminator() const { return terminator; }

    enum State { Start,
                 IdentifierOrKeyword,
                 Identifier,
                 InIdentifierOrKeyword,
                 InIdentifier,
                 InIdentifierUnicodeEscapeStart,
                 InIdentifierUnicodeEscape,
                 InSingleLineComment,
                 InMultiLineComment,
                 InNum,
                 InNum0,
                 InHex,
                 InOctal,
                 InDecimal,
                 InExponentIndicator,
                 InExponent,
                 Hex,
                 Octal,
                 Number,
                 String,
                 Eof,
                 InString,
                 InEscapeSequence,
                 InHexEscape,
                 InUnicodeEscape,
                 Other,
                 Bad };

    bool scanRegExp();
    UString pattern, flags;
    bool hadError() const { return foundBad; }

    static bool isWhiteSpace(unsigned short c);
    static bool isIdentLetter(unsigned short c);
    static bool isDecimalDigit(unsigned short c);
    static bool isHexDigit(unsigned short c);
    static bool isOctalDigit(unsigned short c);

  private:
    int yylineno;
    bool done;
    char *buffer8;
    UChar *buffer16;
    unsigned int size8, size16;
    unsigned int pos8, pos16;
    bool terminator;
    bool restrKeyword;
    // encountered delimiter like "'" and "}" on last run
    bool delimited;
    bool skipLF;
    bool skipCR;
    bool convertNextIdentifier;
    int stackToken;
    int lastToken;
    bool foundBad;

    State state;
    void setDone(State s);
    unsigned int pos;
    void shift(unsigned int p);
    void nextLine();
    int lookupKeyword(const char *);

    int matchPunctuator(unsigned short c1, unsigned short c2,
                        unsigned short c3, unsigned short c4);
    unsigned short singleEscape(unsigned short c) const;
    unsigned short convertOctal(unsigned short c1, unsigned short c2,
                                unsigned short c3) const;
  public:
    static unsigned char convertHex(unsigned short c1);
    static unsigned char convertHex(unsigned short c1, unsigned short c2);
    static UChar convertUnicode(unsigned short c1, unsigned short c2,
                                unsigned short c3, unsigned short c4);

#ifdef KJS_DEBUG_MEM
    /**
     * Clear statically allocated resources
     */
    static void globalClear();
#endif

    void doneParsing();

  private:

    void record8(unsigned short c);
    void record16(int c);
    void record16(UChar c);

    KJS::Identifier *makeIdentifier(UChar *buffer, unsigned int pos);
    UString *makeUString(UChar *buffer, unsigned int pos);

    const UChar *code;
    unsigned int length;
    int yycolumn;
#ifndef KJS_PURE_ECMA
    int bol;     // begin of line
#endif

    // current and following unicode characters (int to allow for -1 for end-of-file marker)
    int current, next1, next2, next3;

    UString **strings;
    unsigned int numStrings;
    unsigned int stringsCapacity;

    KJS::Identifier **identifiers;
    unsigned int numIdentifiers;
    unsigned int identifiersCapacity;

    // for future extensions
    class LexerPrivate;
    LexerPrivate *priv;
  };

} // namespace

#endif
