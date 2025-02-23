/* This file is part of the KDE libraries
   Copyright (C) 1999 Ian Zepp (icszepp@islc.net)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kstringhandler.h"
#include "tdeglobal.h"

static void parsePythonRange( const TQCString &range, uint &start, uint &end )
{
    const int colon = range.find( ':' );
    if ( colon == -1 ) {
        start = range.toUInt();
        end = start;
    } else if ( colon == int( range.length() - 1 ) ) {
        start = range.left( colon ).toUInt();
    } else if ( colon == 0 ) {
        end = range.mid( 1 ).toUInt();
    } else {
        start = range.left( colon ).toInt();
        end = range.mid( colon + 1 ).toInt();
    }
}

TQString KStringHandler::word( const TQString &text , uint pos )
{
    return text.section( ' ', pos, pos );
}

TQString KStringHandler::word( const TQString &text , const char *range )
{
    // Format in: START:END
    // Note index starts a 0 (zero)
    //
    // 0:        first word to end
    // 1:3        second to fourth words
    TQStringList list = TQStringList::split( " ", text , true );
    TQString tmp = "";
    TQString r = range;

    if ( text.isEmpty() )
        return tmp;

    uint pos = 0, cnt = list.count();
    parsePythonRange( range, pos, cnt );

    //
    // Extract words
    //
    int wordsToExtract = cnt-pos+1;
    TQStringList::Iterator it = list.at( pos);

    while ( (it != list.end()) && (wordsToExtract-- > 0))
    {
       tmp += *it;
       tmp += " ";
       it++;
    }

    return tmp.stripWhiteSpace();
}

//
// Insertion and removal routines
//
TQString KStringHandler::insword( const TQString &text , const TQString &word , uint pos )
{
    if ( text.isEmpty() )
        return word;

    if ( word.isEmpty() )
        return text;

    // Split words and add into list
    TQStringList list = TQStringList::split( " ", text, true );

    if ( pos >= list.count() )
        list.append( word );
    else
        list.insert( list.at(pos) , word );

    // Rejoin
    return list.join( " " );
}

TQString KStringHandler::setword( const TQString &text , const TQString &word , uint pos )
{
    if ( text.isEmpty() )
        return word;

    if ( word.isEmpty() )
        return text;

    // Split words and add into list
    TQStringList list = TQStringList::split( " ", text, true );

    if ( pos >= list.count() )
        list.append( word );
    else
    {
        list.insert( list.remove( list.at(pos) ) , word );
    }

    // Rejoin
    return list.join( " " );
}

TQString KStringHandler::remrange( const TQString &text , const char *range )
{
    // Format in: START:END
    // Note index starts a 0 (zero)
    //
    // 0:        first word to end
    // 1:3        second to fourth words
    TQStringList list = TQStringList::split( " ", text , true );
    TQString tmp = "";
    TQString r = range;

    if ( text.isEmpty() )
        return tmp;

    uint pos = 0, cnt = list.count();
    parsePythonRange( range, pos, cnt );

    //
    // Remove that range of words
    //
    int wordsToDelete = cnt-pos+1;
    TQStringList::Iterator it = list.at( pos);

    while ( (it != list.end()) && (wordsToDelete-- > 0))
       it = list.remove( it );

    return list.join( " " );
}

TQString KStringHandler::remword( const TQString &text , uint pos )
{
    TQString tmp = "";

    if ( text.isEmpty() )
        return tmp;

    // Split words and add into list
    TQStringList list = TQStringList::split( " ", text, true );

    if ( pos < list.count() )
        list.remove( list.at( pos ) );

    // Rejoin
    return list.join( " " );
}

TQString KStringHandler::remword( const TQString &text , const TQString &word )
{
    TQString tmp = "";

    if ( text.isEmpty() )
        return tmp;

    if ( word.isEmpty() )
        return text;

    // Split words and add into list
    TQStringList list = TQStringList::split( " ", text, true );

    TQStringList::Iterator it = list.find(word);

    if (it != list.end())
       list.remove( it );

    // Rejoin
    return list.join( " " );
}

//
// Capitalization routines
//
TQString KStringHandler::capwords( const TQString &text )
{
    if ( text.isEmpty() ) {
        return text;
    }

    const TQString strippedText = text.stripWhiteSpace();
    const TQStringList words = capwords( TQStringList::split( ' ', strippedText ) );

    TQString result = text;
    result.replace( strippedText, words.join( " " ) );
    return result;
}

TQStringList KStringHandler::capwords( const TQStringList &list )
{
    TQStringList tmp = list;
    for ( TQStringList::Iterator it = tmp.begin(); it != tmp.end(); ++it ) {
        *it = ( *it )[ 0 ].upper() + ( *it ).mid( 1 );
    }
    return tmp;
}

//
// Reverse routines
//
TQString KStringHandler::reverse( const TQString &text )
{
    TQString tmp;

    if ( text.isEmpty() )
        return tmp;

    TQStringList list;
    list = TQStringList::split( " ", text, true );
    list = reverse( list );

    return list.join( " " );
}

TQStringList KStringHandler::reverse( const TQStringList &list )
{
    TQStringList tmp;

    if ( list.count() == 0 )
        return tmp;

    for ( TQStringList::ConstIterator it= list.begin();
          it != list.end();
          it++)
        tmp.prepend( *it );

    return tmp;
}

//
// Left, Right, Center justification
//
TQString KStringHandler::ljust( const TQString &text , uint width )
{
    return text.stripWhiteSpace().leftJustify( width );
}

TQString KStringHandler::rjust( const TQString &text , uint width )
{
    return text.stripWhiteSpace().rightJustify( width );
}

TQString KStringHandler::center( const TQString &text , uint width )
{
    const TQString s = text.stripWhiteSpace();
    const unsigned int length = s.length();
    if ( width <= length ) {
        return s;
     }

    TQString result;
    result.fill( ' ', ( width - length ) / 2 );
    result += s;

    return result.leftJustify( width );
}

TQString KStringHandler::lsqueeze( const TQString & str, uint maxlen )
{
  if (str.length() > maxlen) {
    int part = maxlen-3;
    return TQString("..." + str.right(part));
  }
  else return str;
}

TQString KStringHandler::csqueeze( const TQString & str, uint maxlen )
{
  if (str.length() > maxlen && maxlen > 3) {
    int part = (maxlen-3)/2;
    return TQString(str.left(part) + "..." + str.right(part));
  }
  else return str;
}

TQString KStringHandler::rsqueeze( const TQString & str, uint maxlen )
{
  if (str.length() > maxlen) {
    int part = maxlen-3;
    return TQString(str.left(part) + "...");
  }
  else return str;
}

TQString KStringHandler::lEmSqueeze(const TQString &name, const TQFontMetrics& fontMetrics, uint maxlen)
{
  return lPixelSqueeze(name, fontMetrics, fontMetrics.maxWidth() * maxlen);
}

TQString KStringHandler::lPixelSqueeze(const TQString& name, const TQFontMetrics& fontMetrics, uint maxPixels)
{
  uint nameWidth = fontMetrics.width(name);

  if (maxPixels < nameWidth)
  {
    TQString tmp = name;
    const uint em = fontMetrics.maxWidth();
    maxPixels -= fontMetrics.width("...");

    while (maxPixels < nameWidth && !tmp.isEmpty())
    {
      int delta = (nameWidth - maxPixels) / em;
      delta = kClamp(delta, 1, delta); // no max

      tmp.remove(0, delta);
      nameWidth = fontMetrics.width(tmp);
    }

    return ("..." + tmp);
  }

  return name;
}

TQString KStringHandler::cEmSqueeze(const TQString& name, const TQFontMetrics& fontMetrics, uint maxlen)
{
  return cPixelSqueeze(name, fontMetrics, fontMetrics.maxWidth() * maxlen);
}

TQString KStringHandler::cPixelSqueeze(const TQString& s, const TQFontMetrics& fm, uint width)
{
  if ( s.isEmpty() || uint( fm.width( s ) ) <= width ) {
    return s;
  }

  const unsigned int length = s.length();
  if ( length == 2 ) {
    return s;
  }

  const int maxWidth = width - fm.width( '.' ) * 3;
  if ( maxWidth <= 0 ) {
    return "...";
  }

  unsigned int leftIdx = 0, rightIdx = length;
  unsigned int leftWidth = fm.charWidth( s, leftIdx++ );
  unsigned int rightWidth = fm.charWidth( s, --rightIdx );
  while ( leftWidth + rightWidth < uint( maxWidth ) ) {
    while ( leftWidth <= rightWidth && leftWidth + rightWidth < uint( maxWidth ) ) {
      leftWidth += fm.charWidth( s, leftIdx++ );
    }
    while ( rightWidth <= leftWidth && leftWidth + rightWidth < uint( maxWidth ) ) {
      rightWidth += fm.charWidth( s, --rightIdx );
    }
  }

  if ( leftWidth > rightWidth ) {
    --leftIdx;
  } else {
    ++rightIdx;
  }

  rightIdx = length - rightIdx;
  if ( leftIdx == 0 && rightIdx == 1 || leftIdx == 1 && rightIdx == 0 ) {
    return "...";
  }

  return s.left( leftIdx ) + "..." + s.right( rightIdx );
}

TQString KStringHandler::rEmSqueeze(const TQString& name, const TQFontMetrics& fontMetrics, uint maxlen)
{
  return rPixelSqueeze(name, fontMetrics, fontMetrics.maxWidth() * maxlen);
}

TQString KStringHandler::rPixelSqueeze(const TQString& name, const TQFontMetrics& fontMetrics, uint maxPixels)
{
  uint nameWidth = fontMetrics.width(name);

  if (maxPixels < nameWidth)
  {
    TQString tmp = name;
    const uint em = fontMetrics.maxWidth();
    maxPixels -= fontMetrics.width("...");

    while (maxPixels < nameWidth && !tmp.isEmpty())
    {
      int length = tmp.length();
      int delta = em ? (nameWidth - maxPixels) / em : length;
      delta = kClamp(delta, 1, length) ;

      tmp.remove(length - delta, delta);
      nameWidth = fontMetrics.width(tmp);
    }

    return (tmp + "...");
  }

  return name;
}

///// File name patterns (like *.txt)

bool KStringHandler::matchFileName( const TQString& filename, const TQString& pattern  )
{
   int len = filename.length();
   int pattern_len = pattern.length();

   if (!pattern_len)
      return false;

   // Patterns like "Makefile*"
   if ( pattern[ pattern_len - 1 ] == (TQChar)'*' && len + 1 >= pattern_len ) {
      if ( pattern[ 0 ] == (TQChar)'*' )
      {
         return filename.find(pattern.mid(1, pattern_len - 2)) != -1;
      }

      const TQChar *c1 = pattern.unicode();
      const TQChar *c2 = filename.unicode();
      int cnt = 1;
      while ( cnt < pattern_len && *c1++ == *c2++ )
         ++cnt;
      return cnt == pattern_len;
   }

   // Patterns like "*~", "*.extension"
   if ( pattern[ 0 ] == (TQChar)'*' && len + 1 >= pattern_len )
   {
     const TQChar *c1 = pattern.unicode() + pattern_len - 1;
     const TQChar *c2 = filename.unicode() + len - 1;
     int cnt = 1;
     while ( cnt < pattern_len && *c1-- == *c2-- )
        ++cnt;
     return cnt == pattern_len;
  }

   // Patterns like "Makefile"
   return ( filename == pattern );
}

  TQStringList
KStringHandler::perlSplit(const TQString & sep, const TQString & s, uint max)
{
  bool ignoreMax = 0 == max;

  TQStringList l;

  int searchStart = 0;

  int tokenStart = s.find(sep, searchStart);

  while (-1 != tokenStart && (ignoreMax || l.count() < max - 1))
  {
    if (!s.mid(searchStart, tokenStart - searchStart).isEmpty())
      l << s.mid(searchStart, tokenStart - searchStart);

    searchStart = tokenStart + sep.length();
    tokenStart = s.find(sep, searchStart);
  }

  if (!s.mid(searchStart, s.length() - searchStart).isEmpty())
    l << s.mid(searchStart, s.length() - searchStart);

  return l;
}

  TQStringList
KStringHandler::perlSplit(const TQChar & sep, const TQString & s, uint max)
{
  bool ignoreMax = 0 == max;

  TQStringList l;

  int searchStart = 0;

  int tokenStart = s.find(sep, searchStart);

  while (-1 != tokenStart && (ignoreMax || l.count() < max - 1))
  {
    if (!s.mid(searchStart, tokenStart - searchStart).isEmpty())
      l << s.mid(searchStart, tokenStart - searchStart);

    searchStart = tokenStart + 1;
    tokenStart = s.find(sep, searchStart);
  }

  if (!s.mid(searchStart, s.length() - searchStart).isEmpty())
    l << s.mid(searchStart, s.length() - searchStart);

  return l;
}

  TQStringList
KStringHandler::perlSplit(const TQRegExp & sep, const TQString & s, uint max)
{
  bool ignoreMax = 0 == max;

  TQStringList l;

  int searchStart = 0;
  int tokenStart = sep.search(s, searchStart);
  int len = sep.matchedLength();

  while (-1 != tokenStart && (ignoreMax || l.count() < max - 1))
  {
    if (!s.mid(searchStart, tokenStart - searchStart).isEmpty())
      l << s.mid(searchStart, tokenStart - searchStart);

    searchStart = tokenStart + len;
    tokenStart = sep.search(s, searchStart);
    len = sep.matchedLength();
  }

  if (!s.mid(searchStart, s.length() - searchStart).isEmpty())
    l << s.mid(searchStart, s.length() - searchStart);

  return l;
}

TQString
KStringHandler::tagURLs( const TQString& text )
{
    /*static*/ TQRegExp urlEx("(www\\.(?!\\.)|(fish|(f|ht)tp(|s))://)[\\d\\w\\./,:_~\\?=&;#@\\-\\+\\%\\$]+[\\d\\w/]");

    TQString richText( text );
    int urlPos = 0, urlLen;
    while ((urlPos = urlEx.search(richText, urlPos)) >= 0)
    {
        urlLen = urlEx.matchedLength();
        TQString href = richText.mid( urlPos, urlLen );
        // Qt doesn't support (?<=pattern) so we do it here
        if((urlPos > 0) && richText[urlPos-1].isLetterOrNumber()){
            urlPos++;
            continue;
        }
        // Don't use TQString::arg since %01, %20, etc could be in the string
        TQString anchor = "<a href=\"" + href + "\">" + href + "</a>";
        richText.replace( urlPos, urlLen, anchor );


        urlPos += anchor.length();
    }
    return richText;
}

TQString KStringHandler::obscure( const TQString &str )
{
  TQString result;
  const TQChar *unicode = str.unicode();
  for ( uint i = 0; i < str.length(); ++i )
    result += ( unicode[ i ].unicode() < 0x21 ) ? unicode[ i ] :
        TQChar( 0x1001F - unicode[ i ].unicode() );

  return result;
}

bool KStringHandler::isUtf8(const char *buf)
{
  int i, n;
  unsigned char c;
  bool gotone = false;
  
  if (!buf)
    return true; // whatever, just don't crash

#define F 0   /* character never appears in text */
#define T 1   /* character appears in plain ASCII text */
#define I 2   /* character appears in ISO-8859 text */
#define X 3   /* character appears in non-ISO extended ASCII (Mac, IBM PC) */

  static const unsigned char text_chars[256] = {
        /*                  BEL BS HT LF    FF CR    */
        F, F, F, F, F, F, F, T, T, T, T, F, T, T, F, F,  /* 0x0X */
        /*                              ESC          */
        F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  /* 0x1X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x2X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x3X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x4X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x5X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x6X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F,  /* 0x7X */
        /*            NEL                            */
        X, X, X, X, X, T, X, X, X, X, X, X, X, X, X, X,  /* 0x8X */
        X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,  /* 0x9X */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xaX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xbX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xcX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xdX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xeX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I   /* 0xfX */
  };

  /* *ulen = 0; */
  for (i = 0; (c = buf[i]); i++) {
    if ((c & 0x80) == 0) {        /* 0xxxxxxx is plain ASCII */
      /*
       * Even if the whole file is valid UTF-8 sequences,
       * still reject it if it uses weird control characters.
       */

      if (text_chars[c] != T)
        return false;

    } else if ((c & 0x40) == 0) { /* 10xxxxxx never 1st byte */
      return false;
    } else {                           /* 11xxxxxx begins UTF-8 */
      int following;

    if ((c & 0x20) == 0) {             /* 110xxxxx */
      following = 1;
    } else if ((c & 0x10) == 0) {      /* 1110xxxx */
      following = 2;
    } else if ((c & 0x08) == 0) {      /* 11110xxx */
      following = 3;
    } else if ((c & 0x04) == 0) {      /* 111110xx */
      following = 4;
    } else if ((c & 0x02) == 0) {      /* 1111110x */
      following = 5;
    } else
      return false;

      for (n = 0; n < following; n++) {
        i++;
        if (!(c = buf[i]))
          goto done;

        if ((c & 0x80) == 0 || (c & 0x40))
          return false;
      }
      gotone = true;
    }
  }
done:
  return gotone;   /* don't claim it's UTF-8 if it's all 7-bit */
}

#undef F
#undef T
#undef I
#undef X

TQString KStringHandler::from8Bit( const char *str )
{
  if (!str)
    return TQString::null;
  if (!*str) {
    static const TQString &emptyString = TDEGlobal::staticQString("");
    return emptyString;
  }
  return KStringHandler::isUtf8( str ) ?
             TQString::fromUtf8( str ) : 
             TQString::fromLocal8Bit( str );
}
