/* This file is part of the KDE libraries
   Copyright (C) 2003, 2004 Anders Lund <anders@alweb.dk>
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2001,2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

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

//BEGIN INCLUDES
#include "katehighlight.h"
#include "katehighlight.moc"

#include "katetextline.h"
#include "katedocument.h"
#include "katesyntaxdocument.h"
#include "katerenderer.h"
#include "katefactory.h"
#include "kateschema.h"
#include "kateconfig.h"

#include <tdeconfig.h>
#include <tdeglobal.h>
#include <kinstance.h>
#include <kmimetype.h>
#include <tdelocale.h>
#include <kregexp.h>
#include <tdepopupmenu.h>
#include <tdeglobalsettings.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <tdemessagebox.h>
#include <kstaticdeleter.h>
#include <tdeapplication.h>

#include <tqstringlist.h>
#include <tqtextstream.h>
//END

//BEGIN defines
// same as in kmimemagic, no need to feed more data
#define KATE_HL_HOWMANY 1024

// min. x seconds between two dynamic contexts reset
static const int KATE_DYNAMIC_CONTEXTS_RESET_DELAY = 30 * 1000;

// x is a TQString. if x is "true" or "1" this expression returns "true"
#define IS_TRUE(x) x.lower() == TQString("true") || x.toInt() == 1
//END defines

//BEGIN  Prviate HL classes

inline bool kateInsideString (const TQString &str, TQChar ch)
{
  const TQChar *unicode = str.unicode();
  const uint len = str.length();
  for (uint i=0; i < len; i++)
    if (unicode[i] == ch)
      return true;

  return false;
}

class KateHlItem
{
  public:
    KateHlItem(int attribute, int context,signed char regionId, signed char regionId2);
    virtual ~KateHlItem();

  public:
    // caller must keep in mind: LEN > 0 is a must !!!!!!!!!!!!!!!!!!!!!1
    // Now, the function returns the offset detected, or 0 if no match is found.
    // bool linestart isn't needed, this is equivalent to offset == 0.
    virtual int checkHgl(const TQString& text, int offset, int len) = 0;

    virtual bool lineContinue(){return false;}

    virtual TQStringList *capturedTexts() {return 0;}
    virtual KateHlItem *clone(const TQStringList *) {return this;}

    static void dynamicSubstitute(TQString& str, const TQStringList *args);

    TQMemArray<KateHlItem*> subItems;
    int attr;
    int ctx;
    signed char region;
    signed char region2;

    bool lookAhead;

    bool dynamic;
    bool dynamicChild;
    bool firstNonSpace;
    bool onlyConsume;
    int column;

    // start enable flags, nicer than the virtual methodes
    // saves function calls
    bool alwaysStartEnable;
    bool customStartEnable;
};

class KateHlContext
{
  public:
    KateHlContext(const TQString &_hlId, int attribute, int lineEndContext,int _lineBeginContext,
                  bool _fallthrough, int _fallthroughContext, bool _dynamic,bool _noIndentationBasedFolding);
    virtual ~KateHlContext();
    KateHlContext *clone(const TQStringList *args);

    TQValueVector<KateHlItem*> items;
    TQString hlId; ///< A unique highlight identifier. Used to look up correct properties.
    int attr;
    int ctx;
    int lineBeginContext;
    /** @internal anders: possible escape if no rules matches.
       false unless 'fallthrough="1|true"' (insensitive)
       if true, go to ftcxt w/o eating of string.
       ftctx is "fallthroughContext" in xml files, valid values are int or #pop[..]
       see in KateHighlighting::doHighlight */
    bool fallthrough;
    int ftctx; // where to go after no rules matched

    bool dynamic;
    bool dynamicChild;
    bool noIndentationBasedFolding;
};

class KateEmbeddedHlInfo
{
  public:
    KateEmbeddedHlInfo() {loaded=false;context0=-1;}
    KateEmbeddedHlInfo(bool l, int ctx0) {loaded=l;context0=ctx0;}

  public:
    bool loaded;
    int context0;
};

class KateHlIncludeRule
{
  public:
    KateHlIncludeRule(int ctx_=0, uint pos_=0, const TQString &incCtxN_="", bool incAttrib=false)
      : ctx(ctx_)
      , pos( pos_)
      , incCtxN( incCtxN_ )
      , includeAttrib( incAttrib )
    {
      incCtx=-1;
    }
    //KateHlIncludeRule(int ctx_, uint  pos_, bool incAttrib) {ctx=ctx_;pos=pos_;incCtx=-1;incCtxN="";includeAttrib=incAttrib}

  public:
    int ctx;
    uint pos;
    int incCtx;
    TQString incCtxN;
    bool includeAttrib;
};

class KateHlCharDetect : public KateHlItem
{
  public:
    KateHlCharDetect(int attribute, int context,signed char regionId,signed char regionId2, TQChar);

    virtual int checkHgl(const TQString& text, int offset, int len);
    virtual KateHlItem *clone(const TQStringList *args);

  private:
    TQChar sChar;
};

class KateHl2CharDetect : public KateHlItem
{
  public:
    KateHl2CharDetect(int attribute, int context, signed char regionId,signed char regionId2,  TQChar ch1, TQChar ch2);
    KateHl2CharDetect(int attribute, int context,signed char regionId,signed char regionId2,  const TQChar *ch);

    virtual int checkHgl(const TQString& text, int offset, int len);
    virtual KateHlItem *clone(const TQStringList *args);

  private:
    TQChar sChar1;
    TQChar sChar2;
};

class KateHlStringDetect : public KateHlItem
{
  public:
    KateHlStringDetect(int attribute, int context, signed char regionId,signed char regionId2, const TQString &, bool inSensitive=false);

    virtual int checkHgl(const TQString& text, int offset, int len);
    virtual KateHlItem *clone(const TQStringList *args);

  private:
    const TQString str;
    const int strLen;
    const bool _inSensitive;
};

class KateHlRangeDetect : public KateHlItem
{
  public:
    KateHlRangeDetect(int attribute, int context, signed char regionId,signed char regionId2, TQChar ch1, TQChar ch2);

    virtual int checkHgl(const TQString& text, int offset, int len);

  private:
    TQChar sChar1;
    TQChar sChar2;
};

class KateHlKeyword : public KateHlItem
{
  public:
    KateHlKeyword(int attribute, int context,signed char regionId,signed char regionId2, bool insensitive, const TQString& delims);
    virtual ~KateHlKeyword ();

    void addList(const TQStringList &);
    virtual int checkHgl(const TQString& text, int offset, int len);

  private:
    TQMemArray< TQDict<bool>* > dict;
    bool _insensitive;
    const TQString& deliminators;
    int minLen;
    int maxLen;
};

class KateHlInt : public KateHlItem
{
  public:
    KateHlInt(int attribute, int context, signed char regionId,signed char regionId2);

    virtual int checkHgl(const TQString& text, int offset, int len);
};

class KateHlFloat : public KateHlItem
{
  public:
    KateHlFloat(int attribute, int context, signed char regionId,signed char regionId2);
    virtual ~KateHlFloat () {}

    virtual int checkHgl(const TQString& text, int offset, int len);
};

class KateHlCFloat : public KateHlFloat
{
  public:
    KateHlCFloat(int attribute, int context, signed char regionId,signed char regionId2);

    virtual int checkHgl(const TQString& text, int offset, int len);
    int checkIntHgl(const TQString& text, int offset, int len);
};

class KateHlCOct : public KateHlItem
{
  public:
    KateHlCOct(int attribute, int context, signed char regionId,signed char regionId2);

    virtual int checkHgl(const TQString& text, int offset, int len);
};

class KateHlCHex : public KateHlItem
{
  public:
    KateHlCHex(int attribute, int context, signed char regionId,signed char regionId2);

    virtual int checkHgl(const TQString& text, int offset, int len);
};

class KateHlLineContinue : public KateHlItem
{
  public:
    KateHlLineContinue(int attribute, int context, signed char regionId,signed char regionId2);

    virtual bool endEnable(TQChar c) {return c == '\0';}
    virtual int checkHgl(const TQString& text, int offset, int len);
    virtual bool lineContinue(){return true;}
};

class KateHlCStringChar : public KateHlItem
{
  public:
    KateHlCStringChar(int attribute, int context, signed char regionId,signed char regionId2);

    virtual int checkHgl(const TQString& text, int offset, int len);
};

class KateHlCChar : public KateHlItem
{
  public:
    KateHlCChar(int attribute, int context,signed char regionId,signed char regionId2);

    virtual int checkHgl(const TQString& text, int offset, int len);
};

class KateHlAnyChar : public KateHlItem
{
  public:
    KateHlAnyChar(int attribute, int context, signed char regionId,signed char regionId2, const TQString& charList);

    virtual int checkHgl(const TQString& text, int offset, int len);

  private:
    const TQString _charList;
};

class KateHlRegExpr : public KateHlItem
{
  public:
    KateHlRegExpr(int attribute, int context,signed char regionId,signed char regionId2 ,TQString expr, bool insensitive, bool minimal);
    ~KateHlRegExpr() { delete Expr; };

    virtual int checkHgl(const TQString& text, int offset, int len);
    virtual TQStringList *capturedTexts();
    virtual KateHlItem *clone(const TQStringList *args);

  private:
    TQRegExp *Expr;
    bool handlesLinestart;
    TQString _regexp;
    bool _insensitive;
    bool _minimal;
};

class KateHlDetectSpaces : public KateHlItem
{
  public:
    KateHlDetectSpaces (int attribute, int context,signed char regionId,signed char regionId2)
      : KateHlItem(attribute,context,regionId,regionId2) {}

    virtual int checkHgl(const TQString& text, int offset, int len)
    {
      int len2 = offset + len;
      while ((offset < len2) && text[offset].isSpace()) offset++;
      return offset;
    }
};

class KateHlDetectIdentifier : public KateHlItem
{
  public:
    KateHlDetectIdentifier (int attribute, int context,signed char regionId,signed char regionId2)
      : KateHlItem(attribute,context,regionId,regionId2) { alwaysStartEnable = false; }

    virtual int checkHgl(const TQString& text, int offset, int len)
    {
      // first char should be a letter or underscore
      if ( text[offset].isLetter() || text[offset] == TQChar ('_') )
      {
        // memorize length
        int len2 = offset+len;

        // one char seen
        offset++;

        // now loop for all other thingies
        while (
               (offset < len2)
               && (text[offset].isLetterOrNumber() || (text[offset] == TQChar ('_')))
              )
          offset++;

        return offset;
      }

      return 0;
    }
};

//END

//BEGIN STATICS
KateHlManager *KateHlManager::s_self = 0;

static const bool trueBool = true;
static const TQString stdDeliminator = TQString (" \t.():!+,-<=>%&*/;?[]^{|}~\\");
//END

//BEGIN NON MEMBER FUNCTIONS
static KateHlItemData::ItemStyles getDefStyleNum(TQString name)
{
  if (name=="dsNormal") return KateHlItemData::dsNormal;
  else if (name=="dsKeyword") return KateHlItemData::dsKeyword;
  else if (name=="dsDataType") return KateHlItemData::dsDataType;
  else if (name=="dsDecVal") return KateHlItemData::dsDecVal;
  else if (name=="dsBaseN") return KateHlItemData::dsBaseN;
  else if (name=="dsFloat") return KateHlItemData::dsFloat;
  else if (name=="dsChar") return KateHlItemData::dsChar;
  else if (name=="dsString") return KateHlItemData::dsString;
  else if (name=="dsComment") return KateHlItemData::dsComment;
  else if (name=="dsOthers")  return KateHlItemData::dsOthers;
  else if (name=="dsAlert") return KateHlItemData::dsAlert;
  else if (name=="dsFunction") return KateHlItemData::dsFunction;
  else if (name=="dsRegionMarker") return KateHlItemData::dsRegionMarker;
  else if (name=="dsError") return KateHlItemData::dsError;

  return KateHlItemData::dsNormal;
}
//END

//BEGIN KateHlItem
KateHlItem::KateHlItem(int attribute, int context,signed char regionId,signed char regionId2)
  : attr(attribute),
    ctx(context),
    region(regionId),
    region2(regionId2),
    lookAhead(false),
    dynamic(false),
    dynamicChild(false),
    firstNonSpace(false),
    onlyConsume(false),
    column (-1),
    alwaysStartEnable (true),
    customStartEnable (false)
{
}

KateHlItem::~KateHlItem()
{
  //kdDebug(13010)<<"In hlItem::~KateHlItem()"<<endl;
  for (uint i=0; i < subItems.size(); i++)
    delete subItems[i];
}

void KateHlItem::dynamicSubstitute(TQString &str, const TQStringList *args)
{
  uint strLength = str.length();
  if (strLength > 0) {
    for (uint i = 0; i < strLength - 1; ++i) {
      if (str[i] == '%') {
        char c = str[i + 1].latin1();
        if (c == '%') {
          str.replace(i, 1, "");
        }
        else if (c >= '0' && c <= '9') {
          if ((uint)(c - '0') < args->size()) {
            str.replace(i, 2, (*args)[c - '0']);
            i += ((*args)[c - '0']).length() - 1;
          }
          else {
            str.replace(i, 2, "");
            --i;
          }
        }
      }
    }
  }
}
//END

//BEGIN KateHlCharDetect
KateHlCharDetect::KateHlCharDetect(int attribute, int context, signed char regionId,signed char regionId2, TQChar c)
  : KateHlItem(attribute,context,regionId,regionId2)
  , sChar(c)
{
}

int KateHlCharDetect::checkHgl(const TQString& text, int offset, int /*len*/)
{
  if (text[offset] == sChar)
    return offset + 1;

  return 0;
}

KateHlItem *KateHlCharDetect::clone(const TQStringList *args)
{
  char c = sChar.latin1();

  if (c < '0' || c > '9' || (unsigned)(c - '0') >= args->size())
    return this;

  KateHlCharDetect *ret = new KateHlCharDetect(attr, ctx, region, region2, (*args)[c - '0'][0]);
  ret->dynamicChild = true;
  return ret;
}
//END

//BEGIN KateHl2CharDetect
KateHl2CharDetect::KateHl2CharDetect(int attribute, int context, signed char regionId,signed char regionId2, TQChar ch1, TQChar ch2)
  : KateHlItem(attribute,context,regionId,regionId2)
  , sChar1 (ch1)
  , sChar2 (ch2)
{
}

int KateHl2CharDetect::checkHgl(const TQString& text, int offset, int len)
{
  if ((len >= 2) && text[offset++] == sChar1 && text[offset++] == sChar2)
    return offset;

  return 0;
}

KateHlItem *KateHl2CharDetect::clone(const TQStringList *args)
{
  char c1 = sChar1.latin1();
  char c2 = sChar2.latin1();

  if (c1 < '0' || c1 > '9' || (unsigned)(c1 - '0') >= args->size())
    return this;

  if (c2 < '0' || c2 > '9' || (unsigned)(c2 - '0') >= args->size())
    return this;

  KateHl2CharDetect *ret = new KateHl2CharDetect(attr, ctx, region, region2, (*args)[c1 - '0'][0], (*args)[c2 - '0'][0]);
  ret->dynamicChild = true;
  return ret;
}
//END

//BEGIN KateHlStringDetect
KateHlStringDetect::KateHlStringDetect(int attribute, int context, signed char regionId,signed char regionId2,const TQString &s, bool inSensitive)
  : KateHlItem(attribute, context,regionId,regionId2)
  , str(inSensitive ? s.upper() : s)
  , strLen (str.length())
  , _inSensitive(inSensitive)
{
}

int KateHlStringDetect::checkHgl(const TQString& text, int offset, int len)
{
  if (len < strLen)
    return 0;

  if (_inSensitive)
  {
    for (int i=0; i < strLen; i++)
      if (text[offset++].upper() != str[i])
        return 0;

    return offset;
  }
  else
  {
    for (int i=0; i < strLen; i++)
      if (text[offset++] != str[i])
        return 0;

    return offset;
  }

  return 0;
}

KateHlItem *KateHlStringDetect::clone(const TQStringList *args)
{
  TQString newstr = str;

  dynamicSubstitute(newstr, args);

  if (newstr == str)
    return this;

  KateHlStringDetect *ret = new KateHlStringDetect(attr, ctx, region, region2, newstr, _inSensitive);
  ret->dynamicChild = true;
  return ret;
}
//END

//BEGIN KateHlRangeDetect
KateHlRangeDetect::KateHlRangeDetect(int attribute, int context, signed char regionId,signed char regionId2, TQChar ch1, TQChar ch2)
  : KateHlItem(attribute,context,regionId,regionId2)
  , sChar1 (ch1)
  , sChar2 (ch2)
{
}

int KateHlRangeDetect::checkHgl(const TQString& text, int offset, int len)
{
  if (text[offset] == sChar1)
  {
    do
    {
      offset++;
      len--;
      if (len < 1) return 0;
    }
    while (text[offset] != sChar2);

    return offset + 1;
  }
  return 0;
}
//END

//BEGIN KateHlKeyword
KateHlKeyword::KateHlKeyword (int attribute, int context, signed char regionId,signed char regionId2, bool insensitive, const TQString& delims)
  : KateHlItem(attribute,context,regionId,regionId2)
  , _insensitive(insensitive)
  , deliminators(delims)
  , minLen (0xFFFFFF)
  , maxLen (0)
{
  alwaysStartEnable = false;
  customStartEnable = true;
}

KateHlKeyword::~KateHlKeyword ()
{
  for (uint i=0; i < dict.size(); ++i)
    delete dict[i];
}

void KateHlKeyword::addList(const TQStringList& list)
{
  for(uint i=0; i < list.count(); ++i)
  {
    int len = list[i].length();

    if (minLen > len)
      minLen = len;

    if (maxLen < len)
      maxLen = len;

    if ((uint)len >= dict.size())
    {
      uint oldSize = dict.size();
      dict.resize (len+1);

      for (uint m=oldSize; m < dict.size(); ++m)
        dict[m] = 0;
    }

    if (!dict[len])
      dict[len] = new TQDict<bool> (17, !_insensitive);

    dict[len]->insert(list[i], &trueBool);
  }
}

int KateHlKeyword::checkHgl(const TQString& text, int offset, int len)
{
  int offset2 = offset;
  int wordLen = 0;

  while ((len > wordLen) && !kateInsideString (deliminators, text[offset2]))
  {
    offset2++;
    wordLen++;

    if (wordLen > maxLen) return 0;
  }

  if (wordLen < minLen) return 0;

  if ( dict[wordLen] && dict[wordLen]->find(TQConstString(text.unicode() + offset, wordLen).string()) )
    return offset2;

  return 0;
}
//END

//BEGIN KateHlInt
KateHlInt::KateHlInt(int attribute, int context, signed char regionId,signed char regionId2)
  : KateHlItem(attribute,context,regionId,regionId2)
{
  alwaysStartEnable = false;
}

int KateHlInt::checkHgl(const TQString& text, int offset, int len)
{
  int offset2 = offset;

  while ((len > 0) && text[offset2].isDigit())
  {
    offset2++;
    len--;
  }

  if (offset2 > offset)
  {
    if (len > 0)
    {
      for (uint i=0; i < subItems.size(); i++)
      {
        if ( (offset = subItems[i]->checkHgl(text, offset2, len)) )
          return offset;
      }
    }

    return offset2;
  }

  return 0;
}
//END

//BEGIN KateHlFloat
KateHlFloat::KateHlFloat(int attribute, int context, signed char regionId,signed char regionId2)
  : KateHlItem(attribute,context, regionId,regionId2)
{
  alwaysStartEnable = false;
}

int KateHlFloat::checkHgl(const TQString& text, int offset, int len)
{
  bool b = false;
  bool p = false;

  while ((len > 0) && text[offset].isDigit())
  {
    offset++;
    len--;
    b = true;
  }

  if ((len > 0) && (p = (text[offset] == '.')))
  {
    offset++;
    len--;

    while ((len > 0) && text[offset].isDigit())
    {
      offset++;
      len--;
      b = true;
    }
  }

  if (!b)
    return 0;

  if ((len > 0) && ((text[offset] & 0xdf) == 'E'))
  {
    offset++;
    len--;
  }
  else
  {
    if (!p)
      return 0;
    else
    {
      if (len > 0)
      {
        for (uint i=0; i < subItems.size(); i++)
        {
          int offset2 = subItems[i]->checkHgl(text, offset, len);

          if (offset2)
            return offset2;
        }
      }

      return offset;
    }
  }

  if ((len > 0) && (text[offset] == '-' || text[offset] =='+'))
  {
    offset++;
    len--;
  }

  b = false;

  while ((len > 0) && text[offset].isDigit())
  {
    offset++;
    len--;
    b = true;
  }

  if (b)
  {
    if (len > 0)
    {
      for (uint i=0; i < subItems.size(); i++)
      {
        int offset2 = subItems[i]->checkHgl(text, offset, len);

        if (offset2)
          return offset2;
      }
    }

    return offset;
  }

  return 0;
}
//END

//BEGIN KateHlCOct
KateHlCOct::KateHlCOct(int attribute, int context, signed char regionId,signed char regionId2)
  : KateHlItem(attribute,context,regionId,regionId2)
{
  alwaysStartEnable = false;
}

int KateHlCOct::checkHgl(const TQString& text, int offset, int len)
{
  if (text[offset] == '0')
  {
    offset++;
    len--;

    int offset2 = offset;

    while ((len > 0) && (text.at(offset2) >= TQChar('0') && text.at(offset2) <= TQChar('7')))
    {
      offset2++;
      len--;
    }

    if (offset2 > offset)
    {
      if ((len > 0) && ((text[offset2] & 0xdf) == 'L' || (text[offset] & 0xdf) == 'U' ))
        offset2++;

      return offset2;
    }
  }

  return 0;
}
//END

//BEGIN KateHlCHex
KateHlCHex::KateHlCHex(int attribute, int context,signed char regionId,signed char regionId2)
  : KateHlItem(attribute,context,regionId,regionId2)
{
  alwaysStartEnable = false;
}

int KateHlCHex::checkHgl(const TQString& text, int offset, int len)
{
  if ((len > 1) && (text[offset++] == '0') && ((text[offset++] & 0xdf) == 'X' ))
  {
    len -= 2;

    int offset2 = offset;

    while ((len > 0) && (text[offset2].isDigit() || ((text[offset2] & 0xdf) >= 'A' && (text[offset2] & 0xdf) <= 'F')))
    {
      offset2++;
      len--;
    }

    if (offset2 > offset)
    {
      if ((len > 0) && ((text[offset2] & 0xdf) == 'L' || (text[offset2] & 0xdf) == 'U' ))
        offset2++;

      return offset2;
    }
  }

  return 0;
}
//END

//BEGIN KateHlCFloat
KateHlCFloat::KateHlCFloat(int attribute, int context, signed char regionId,signed char regionId2)
  : KateHlFloat(attribute,context,regionId,regionId2)
{
  alwaysStartEnable = false;
}

int KateHlCFloat::checkIntHgl(const TQString& text, int offset, int len)
{
  int offset2 = offset;

  while ((len > 0) && text[offset].isDigit()) {
    offset2++;
    len--;
  }

  if (offset2 > offset)
     return offset2;

  return 0;
}

int KateHlCFloat::checkHgl(const TQString& text, int offset, int len)
{
  int offset2 = KateHlFloat::checkHgl(text, offset, len);

  if (offset2)
  {
    if ((text[offset2] & 0xdf) == 'F' )
      offset2++;

    return offset2;
  }
  else
  {
    offset2 = checkIntHgl(text, offset, len);

    if (offset2 && ((text[offset2] & 0xdf) == 'F' ))
      return ++offset2;
    else
      return 0;
  }
}
//END

//BEGIN KateHlAnyChar
KateHlAnyChar::KateHlAnyChar(int attribute, int context, signed char regionId,signed char regionId2, const TQString& charList)
  : KateHlItem(attribute, context,regionId,regionId2)
  , _charList(charList)
{
}

int KateHlAnyChar::checkHgl(const TQString& text, int offset, int)
{
  if (kateInsideString (_charList, text[offset]))
    return ++offset;

  return 0;
}
//END

//BEGIN KateHlRegExpr
KateHlRegExpr::KateHlRegExpr( int attribute, int context, signed char regionId,signed char regionId2, TQString regexp, bool insensitive, bool minimal)
  : KateHlItem(attribute, context, regionId,regionId2)
  , handlesLinestart (regexp.startsWith("^"))
  , _regexp(regexp)
  , _insensitive(insensitive)
  , _minimal(minimal)
{
  if (!handlesLinestart)
    regexp.prepend("^");

  Expr = new TQRegExp(regexp, !_insensitive);
  Expr->setMinimal(_minimal);
}

int KateHlRegExpr::checkHgl(const TQString& text, int offset, int /*len*/)
{
  if (offset && handlesLinestart)
    return 0;

  int offset2 = Expr->search( text, offset, TQRegExp::CaretAtOffset );

  if (offset2 == -1) return 0;

  return (offset + Expr->matchedLength());
}

TQStringList *KateHlRegExpr::capturedTexts()
{
  return new TQStringList(Expr->capturedTexts());
}

KateHlItem *KateHlRegExpr::clone(const TQStringList *args)
{
  TQString regexp = _regexp;
  TQStringList escArgs = *args;

  for (TQStringList::Iterator it = escArgs.begin(); it != escArgs.end(); ++it)
  {
    (*it).replace(TQRegExp("(\\W)"), "\\\\1");
  }

  dynamicSubstitute(regexp, &escArgs);

  if (regexp == _regexp)
    return this;

  // kdDebug (13010) << "clone regexp: " << regexp << endl;

  KateHlRegExpr *ret = new KateHlRegExpr(attr, ctx, region, region2, regexp, _insensitive, _minimal);
  ret->dynamicChild = true;
  return ret;
}
//END

//BEGIN KateHlLineContinue
KateHlLineContinue::KateHlLineContinue(int attribute, int context, signed char regionId,signed char regionId2)
  : KateHlItem(attribute,context,regionId,regionId2) {
}

int KateHlLineContinue::checkHgl(const TQString& text, int offset, int len)
{
  if ((len == 1) && (text[offset] == '\\'))
    return ++offset;

  return 0;
}
//END

//BEGIN KateHlCStringChar
KateHlCStringChar::KateHlCStringChar(int attribute, int context,signed char regionId,signed char regionId2)
  : KateHlItem(attribute,context,regionId,regionId2) {
}

// checks for C escaped chars \n and escaped hex/octal chars
static int checkEscapedChar(const TQString& text, int offset, int& len)
{
  int i;
  if (text[offset] == '\\' && len > 1)
  {
    offset++;
    len--;

    switch(text[offset])
    {
      case  'a': // checks for control chars
      case  'b': // we want to fall through
      case  'e':
      case  'f':

      case  'n':
      case  'r':
      case  't':
      case  'v':
      case '\'':
      case '\"':
      case '?' : // added ? ANSI C classifies this as an escaped char
      case '\\':
        offset++;
        len--;
        break;

      case 'x': // if it's like \xff
        offset++; // eat the x
        len--;
        // these for loops can probably be
        // replaced with something else but
        // for right now they work
        // check for hexdigits
        for (i = 0; (len > 0) && (i < 2) && (((static_cast<const char>(text.at(offset)) >= '0') && (static_cast<const char>(text.at(offset)) <= '9')) || ((text[offset] & 0xdf) >= 'A' && (text[offset] & 0xdf) <= 'F')); i++)
        {
          offset++;
          len--;
        }

        if (i == 0)
          return 0; // takes care of case '\x'

        break;

      case '0': case '1': case '2': case '3' :
      case '4': case '5': case '6': case '7' :
        for (i = 0; (len > 0) && (i < 3) && (static_cast<const char>(text.at(offset)) >= '0' && static_cast<const char>(text.at(offset)) <= '7'); i++)
        {
          offset++;
          len--;
        }
        break;

      default:
        return 0;
    }

    return offset;
  }

  return 0;
}

int KateHlCStringChar::checkHgl(const TQString& text, int offset, int len)
{
  return checkEscapedChar(text, offset, len);
}
//END

//BEGIN KateHlCChar
KateHlCChar::KateHlCChar(int attribute, int context,signed char regionId,signed char regionId2)
  : KateHlItem(attribute,context,regionId,regionId2) {
}

int KateHlCChar::checkHgl(const TQString& text, int offset, int len)
{
  if ((len > 1) && (text[offset] == '\'') && (text[offset+1] != '\''))
  {
    int oldl;
    oldl = len;

    len--;

    int offset2 = checkEscapedChar(text, offset + 1, len);

    if (!offset2)
    {
      if (oldl > 2)
      {
        offset2 = offset + 2;
        len = oldl - 2;
      }
      else
      {
        return 0;
      }
    }

    if ((len > 0) && (text[offset2] == '\''))
      return ++offset2;
  }

  return 0;
}
//END

//BEGIN KateHl2CharDetect
KateHl2CharDetect::KateHl2CharDetect(int attribute, int context, signed char regionId,signed char regionId2, const TQChar *s)
  : KateHlItem(attribute,context,regionId,regionId2) {
  sChar1 = s[0];
  sChar2 = s[1];
  }
//END KateHl2CharDetect

KateHlItemData::KateHlItemData(const TQString  name, int defStyleNum)
  : name(name), defStyleNum(defStyleNum) {
}

KateHlData::KateHlData(const TQString &wildcards, const TQString &mimetypes, const TQString &identifier, int priority)
  : wildcards(wildcards), mimetypes(mimetypes), identifier(identifier), priority(priority)
{
}

//BEGIN KateHlContext
KateHlContext::KateHlContext (const TQString &_hlId, int attribute, int lineEndContext, int _lineBeginContext, bool _fallthrough,
	int _fallthroughContext, bool _dynamic, bool _noIndentationBasedFolding)
{
  hlId = _hlId;
  attr = attribute;
  ctx = lineEndContext;
  lineBeginContext = _lineBeginContext;
  fallthrough = _fallthrough;
  ftctx = _fallthroughContext;
  dynamic = _dynamic;
  dynamicChild = false;
  noIndentationBasedFolding=_noIndentationBasedFolding;
  if (_noIndentationBasedFolding) kdDebug(13010)<<TQString("**********************_noIndentationBasedFolding is TRUE*****************")<<endl;

}

KateHlContext *KateHlContext::clone(const TQStringList *args)
{
  KateHlContext *ret = new KateHlContext(hlId, attr, ctx, lineBeginContext, fallthrough, ftctx, false,noIndentationBasedFolding);

  for (uint n=0; n < items.size(); ++n)
  {
    KateHlItem *item = items[n];
    KateHlItem *i = (item->dynamic ? item->clone(args) : item);
    ret->items.append(i);
  }

  ret->dynamicChild = true;

  return ret;
}

KateHlContext::~KateHlContext()
{
  if (dynamicChild)
  {
    for (uint n=0; n < items.size(); ++n)
    {
      if (items[n]->dynamicChild)
        delete items[n];
    }
  }
}
//END

//BEGIN KateHighlighting
KateHighlighting::KateHighlighting(const KateSyntaxModeListItem *def) : refCount(0)
{
  m_attributeArrays.setAutoDelete (true);

  errorsAndWarnings = "";
  building=false;
  noHl = false;
  m_foldingIndentationSensitive = false;
  folding=false;
  internalIDList.setAutoDelete(true);

  if (def == 0)
  {
    noHl = true;
    iName = "None"; // not translated internal name (for config and more)
    iNameTranslated = i18n("None"); // user visible name
    iSection = "";
    m_priority = 0;
    iHidden = false;
    m_additionalData.insert( "none", new HighlightPropertyBag );
    m_additionalData["none"]->deliminator = stdDeliminator;
    m_additionalData["none"]->wordWrapDeliminator = stdDeliminator;
    m_hlIndex[0] = "none";
  }
  else
  {
    iName = def->name;
    iNameTranslated = def->nameTranslated;
    iSection = def->section;
    iHidden = def->hidden;
    iWildcards = def->extension;
    iMimetypes = def->mimetype;
    identifier = def->identifier;
    iVersion=def->version;
    iAuthor=def->author;
    iLicense=def->license;
    m_priority=def->priority.toInt();
  }

   deliminator = stdDeliminator;
}

KateHighlighting::~KateHighlighting()
{
  // cu contexts
  for (uint i=0; i < m_contexts.size(); ++i)
    delete m_contexts[i];
  m_contexts.clear ();
}

void KateHighlighting::generateContextStack(int *ctxNum, int ctx, TQMemArray<short>* ctxs, int *prevLine)
{
  //kdDebug(13010)<<TQString("Entering generateContextStack with %1").arg(ctx)<<endl;
  while (true)
  {
    if (ctx >= 0)
    {
      (*ctxNum) = ctx;

      ctxs->resize (ctxs->size()+1, TQGArray::SpeedOptim);
      (*ctxs)[ctxs->size()-1]=(*ctxNum);

      return;
    }
    else
    {
      if (ctx == -1)
      {
        (*ctxNum)=( (ctxs->isEmpty() ) ? 0 : (*ctxs)[ctxs->size()-1]);
      }
      else
      {
        int size = ctxs->size() + ctx + 1;

        if (size > 0)
        {
          ctxs->resize (size, TQGArray::SpeedOptim);
          (*ctxNum)=(*ctxs)[size-1];
        }
        else
        {
          ctxs->resize (0, TQGArray::SpeedOptim);
          (*ctxNum)=0;
        }

        ctx = 0;

        if ((*prevLine) >= (int)(ctxs->size()-1))
        {
          *prevLine=ctxs->size()-1;

          if ( ctxs->isEmpty() )
            return;

          KateHlContext *c = contextNum((*ctxs)[ctxs->size()-1]);
          if (c && (c->ctx != -1))
          {
            //kdDebug(13010)<<"PrevLine > size()-1 and ctx!=-1)"<<endl;
            ctx = c->ctx;

            continue;
          }
        }
      }

      return;
    }
  }
}

/**
 * Creates a new dynamic context or reuse an old one if it has already been created.
 */
int KateHighlighting::makeDynamicContext(KateHlContext *model, const TQStringList *args)
{
  TQPair<KateHlContext *, TQString> key(model, args->front());
  short value;

  if (dynamicCtxs.contains(key))
    value = dynamicCtxs[key];
  else
  {
    kdDebug(13010) << "new stuff: " << startctx << endl;

    KateHlContext *newctx = model->clone(args);

    m_contexts.push_back (newctx);

    value = startctx++;
    dynamicCtxs[key] = value;
    KateHlManager::self()->incDynamicCtxs();
  }

  // kdDebug(13010) << "Dynamic context: using context #" << value << " (for model " << model << " with args " << *args << ")" << endl;

  return value;
}

/**
 * Drop all dynamic contexts. Shall be called with extreme care, and shall be immediatly
 * followed by a full HL invalidation.
 */
void KateHighlighting::dropDynamicContexts()
{
  for (uint i=base_startctx; i < m_contexts.size(); ++i)
    delete m_contexts[i];

  m_contexts.resize (base_startctx);

  dynamicCtxs.clear();
  startctx = base_startctx;
}

/**
 * Parse the text and fill in the context array and folding list array
 *
 * @param prevLine The previous line, the context array is picked up from that if present.
 * @param textLine The text line to parse
 * @param foldingList will be filled
 * @param ctxChanged will be set to reflect if the context changed
 */
void KateHighlighting::doHighlight ( KateTextLine *prevLine,
                                     KateTextLine *textLine,
                                     TQMemArray<uint>* foldingList,
                                     bool *ctxChanged )
{
  if (!textLine)
    return;

  if (noHl)
  {
    if (textLine->length() > 0)
      memset (textLine->attributes(), 0, textLine->length());

    return;
  }

  // duplicate the ctx stack, only once !
  TQMemArray<short> ctx;
  ctx.duplicate (prevLine->ctxArray());

  int ctxNum = 0;
  int previousLine = -1;
  KateHlContext *context;

  if (ctx.isEmpty())
  {
    // If the stack is empty, we assume to be in Context 0 (Normal)
    context = contextNum(ctxNum);
  }
  else
  {
    // There does an old context stack exist -> find the context at the line start
    ctxNum = ctx[ctx.size()-1]; //context ID of the last character in the previous line

    //kdDebug(13010) << "\t\tctxNum = " << ctxNum << " contextList[ctxNum] = " << contextList[ctxNum] << endl; // ellis

    //if (lineContinue)   kdDebug(13010)<<TQString("The old context should be %1").arg((int)ctxNum)<<endl;

    if (!(context = contextNum(ctxNum)))
      context = contextNum(0);

    //kdDebug(13010)<<"test1-2-1-text2"<<endl;

    previousLine=ctx.size()-1; //position of the last context ID of th previous line within the stack

    // hl continue set or not ???
    if (prevLine->hlLineContinue())
    {
      prevLine--;
    }
    else
    {
      generateContextStack(&ctxNum, context->ctx, &ctx, &previousLine); //get stack ID to use

      if (!(context = contextNum(ctxNum)))
        context = contextNum(0);
    }

    //kdDebug(13010)<<"test1-2-1-text4"<<endl;

    //if (lineContinue)   kdDebug(13010)<<TQString("The new context is %1").arg((int)ctxNum)<<endl;
  }

  // text, for programming convenience :)
  TQChar lastChar = ' ';
  const TQString& text = textLine->string();
  const int len = textLine->length();

  // calc at which char the first char occurs, set it to length of line if never
  const int firstChar = textLine->firstChar();
  const int startNonSpace = (firstChar == -1) ? len : firstChar;

  // last found item
  KateHlItem *item = 0;

  // loop over the line, offset gives current offset
  int offset = 0;
  while (offset < len)
  {
    bool anItemMatched = false;
    bool standardStartEnableDetermined = false;
    bool customStartEnableDetermined = false;

    uint index = 0;
    for (item = context->items.empty() ? 0 : context->items[0]; item; item = (++index < context->items.size()) ? context->items[index] : 0 )
    {
      // does we only match if we are firstNonSpace?
      if (item->firstNonSpace && (offset > startNonSpace))
        continue;

      // have we a column specified? if yes, only match at this column
      if ((item->column != -1) && (item->column != offset))
        continue;

      if (!item->alwaysStartEnable)
      {
        if (item->customStartEnable)
        {
            if (customStartEnableDetermined || kateInsideString (m_additionalData[context->hlId]->deliminator, lastChar))
            customStartEnableDetermined = true;
          else
            continue;
        }
        else
        {
          if (standardStartEnableDetermined || kateInsideString (stdDeliminator, lastChar))
            standardStartEnableDetermined = true;
          else
            continue;
        }
      }

      int offset2 = item->checkHgl(text, offset, len-offset);

      if (offset2 <= offset)
        continue;
      // BUG 144599: Ignore a context change that would push the same context
      // without eating anything... this would be an infinite loop!
      if ( item->lookAhead && item->ctx == ctxNum )
        continue;

      if (item->region2)
      {
        // kdDebug(13010)<<TQString("Region mark 2 detected: %1").arg(item->region2)<<endl;
        if ( !foldingList->isEmpty() && ((item->region2 < 0) && ((int)((*foldingList)[foldingList->size()-2]) == -item->region2) ) )
        {
          foldingList->resize (foldingList->size()-2, TQGArray::SpeedOptim);
        }
        else
        {
          foldingList->resize (foldingList->size()+2, TQGArray::SpeedOptim);
          (*foldingList)[foldingList->size()-2] = (uint)item->region2;
          if (item->region2<0) //check not really needed yet
            (*foldingList)[foldingList->size()-1] = offset2;
          else
          (*foldingList)[foldingList->size()-1] = offset;
        }

      }

      if (item->region)
      {
        // kdDebug(13010)<<TQString("Region mark detected: %1").arg(item->region)<<endl;

      /* if ( !foldingList->isEmpty() && ((item->region < 0) && (*foldingList)[foldingList->size()-1] == -item->region ) )
        {
          foldingList->resize (foldingList->size()-1, TQGArray::SpeedOptim);
        }
        else*/
        {
          foldingList->resize (foldingList->size()+2, TQGArray::SpeedOptim);
          (*foldingList)[foldingList->size()-2] = item->region;
          if (item->region<0) //check not really needed yet
            (*foldingList)[foldingList->size()-1] = offset2;
          else
            (*foldingList)[foldingList->size()-1] = offset;
        }

      }

      // regenerate context stack if needed
      if (item->ctx != -1)
      {
        generateContextStack (&ctxNum, item->ctx, &ctx, &previousLine);
        context = contextNum(ctxNum);
      }

      // dynamic context: substitute the model with an 'instance'
      if (context->dynamic)
      {
        TQStringList *lst = item->capturedTexts();
        if (lst != 0)
        {
          // Replace the top of the stack and the current context
          int newctx = makeDynamicContext(context, lst);
          if (ctx.size() > 0)
            ctx[ctx.size() - 1] = newctx;
          ctxNum = newctx;
          context = contextNum(ctxNum);
        }
        delete lst;
      }

      // dominik: look ahead w/o changing offset?
      if (!item->lookAhead)
      {
        if (offset2 > len)
          offset2 = len;

        // even set attributes ;)
        memset ( textLine->attributes()+offset
               , item->onlyConsume ? context->attr : item->attr
               , offset2-offset);

        offset = offset2;
        lastChar = text[offset-1];
      }

      anItemMatched = true;
      break;
    }

    // something matched, continue loop
    if (anItemMatched)
      continue;

    // nothing found: set attribute of one char
    // anders: unless this context does not want that!
    if ( context->fallthrough )
    {
    // set context to context->ftctx.
      generateContextStack(&ctxNum, context->ftctx, &ctx, &previousLine);  //regenerate context stack
      context=contextNum(ctxNum);
    //kdDebug(13010)<<"context num after fallthrough at col "<<z<<": "<<ctxNum<<endl;
    // the next is nessecary, as otherwise keyword (or anything using the std delimitor check)
    // immediately after fallthrough fails. Is it bad?
    // jowenn, can you come up with a nicer way to do this?
    /*  if (offset)
        lastChar = text[offset - 1];
      else
        lastChar = '\\';*/
      continue;
    }
    else
    {
      *(textLine->attributes() + offset) = context->attr;
      lastChar = text[offset];
      offset++;
    }
  }

  // has the context stack changed ?
  if (ctx == textLine->ctxArray())
  {
    if (ctxChanged)
      (*ctxChanged) = false;
  }
  else
  {
    if (ctxChanged)
      (*ctxChanged) = true;

    // assign ctx stack !
    textLine->setContext(ctx);
  }

  // write hl continue flag
  textLine->setHlLineContinue (item && item->lineContinue());

  if (m_foldingIndentationSensitive) {
    bool noindent=false;
    for(int i=ctx.size()-1; i>=0; --i) {
      if (contextNum(ctx[i])->noIndentationBasedFolding) {
        noindent=true;
        break;
      }
    }
    textLine->setNoIndentBasedFolding(noindent);
  }
}

void KateHighlighting::loadWildcards()
{
  TDEConfig *config = KateHlManager::self()->getTDEConfig();
  config->setGroup("Highlighting " + iName);

  TQString extensionString = config->readEntry("Wildcards", iWildcards);

  if (extensionSource != extensionString) {
    regexpExtensions.clear();
    plainExtensions.clear();

    extensionSource = extensionString;

    static TQRegExp sep("\\s*;\\s*");

    TQStringList l = TQStringList::split( sep, extensionSource );

    static TQRegExp boringExpression("\\*\\.[\\d\\w]+");

    for( TQStringList::Iterator it = l.begin(); it != l.end(); ++it )
      if (boringExpression.exactMatch(*it))
        plainExtensions.append((*it).mid(1));
      else
        regexpExtensions.append(TQRegExp((*it), true, true));
  }
}

TQValueList<TQRegExp>& KateHighlighting::getRegexpExtensions()
{
  return regexpExtensions;
}

TQStringList& KateHighlighting::getPlainExtensions()
{
  return plainExtensions;
}

TQString KateHighlighting::getMimetypes()
{
  TDEConfig *config = KateHlManager::self()->getTDEConfig();
  config->setGroup("Highlighting " + iName);

  return config->readEntry("Mimetypes", iMimetypes);
}

int KateHighlighting::priority()
{
  TDEConfig *config = KateHlManager::self()->getTDEConfig();
  config->setGroup("Highlighting " + iName);

  return config->readNumEntry("Priority", m_priority);
}

KateHlData *KateHighlighting::getData()
{
  TDEConfig *config = KateHlManager::self()->getTDEConfig();
  config->setGroup("Highlighting " + iName);

  KateHlData *hlData = new KateHlData(
  config->readEntry("Wildcards", iWildcards),
  config->readEntry("Mimetypes", iMimetypes),
  config->readEntry("Identifier", identifier),
  config->readNumEntry("Priority", m_priority));

 return hlData;
}

void KateHighlighting::setData(KateHlData *hlData)
{
  TDEConfig *config = KateHlManager::self()->getTDEConfig();
  config->setGroup("Highlighting " + iName);

  config->writeEntry("Wildcards",hlData->wildcards);
  config->writeEntry("Mimetypes",hlData->mimetypes);
  config->writeEntry("Priority",hlData->priority);
}

void KateHighlighting::getKateHlItemDataList (uint schema, KateHlItemDataList &list)
{
  TDEConfig *config = KateHlManager::self()->getTDEConfig();
  config->setGroup("Highlighting " + iName + " - Schema " + KateFactory::self()->schemaManager()->name(schema));

  list.clear();
  createKateHlItemData(list);

  for (KateHlItemData *p = list.first(); p != 0L; p = list.next())
  {
    TQStringList s = config->readListEntry(p->name);

//    kdDebug(13010)<<p->name<<s.count()<<endl;
    if (s.count()>0)
    {

      while(s.count()<9) s<<"";
      p->clear();

      TQString tmp=s[0]; if (!tmp.isEmpty()) p->defStyleNum=tmp.toInt();

      TQRgb col;

      tmp=s[1]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); p->setTextColor(col); }

      tmp=s[2]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); p->setSelectedTextColor(col); }

      tmp=s[3]; if (!tmp.isEmpty()) p->setBold(tmp!="0");

      tmp=s[4]; if (!tmp.isEmpty()) p->setItalic(tmp!="0");

      tmp=s[5]; if (!tmp.isEmpty()) p->setStrikeOut(tmp!="0");

      tmp=s[6]; if (!tmp.isEmpty()) p->setUnderline(tmp!="0");

      tmp=s[7]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); p->setBGColor(col); }

      tmp=s[8]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); p->setSelectedBGColor(col); }

    }
  }
}

/**
 * Saves the KateHlData attribute definitions to the config file.
 *
 * @param schema The id of the schema group to save
 * @param list KateHlItemDataList containing the data to be used
 */
void KateHighlighting::setKateHlItemDataList(uint schema, KateHlItemDataList &list)
{
  TDEConfig *config = KateHlManager::self()->getTDEConfig();
  config->setGroup("Highlighting " + iName + " - Schema "
      + KateFactory::self()->schemaManager()->name(schema));

  TQStringList settings;

  for (KateHlItemData *p = list.first(); p != 0L; p = list.next())
  {
    settings.clear();
    settings<<TQString::number(p->defStyleNum,10);
    settings<<(p->itemSet(KateAttribute::TextColor)?TQString::number(p->textColor().rgb(),16):"");
    settings<<(p->itemSet(KateAttribute::SelectedTextColor)?TQString::number(p->selectedTextColor().rgb(),16):"");
    settings<<(p->itemSet(KateAttribute::Weight)?(p->bold()?"1":"0"):"");
    settings<<(p->itemSet(KateAttribute::Italic)?(p->italic()?"1":"0"):"");
    settings<<(p->itemSet(KateAttribute::StrikeOut)?(p->strikeOut()?"1":"0"):"");
    settings<<(p->itemSet(KateAttribute::Underline)?(p->underline()?"1":"0"):"");
    settings<<(p->itemSet(KateAttribute::BGColor)?TQString::number(p->bgColor().rgb(),16):"");
    settings<<(p->itemSet(KateAttribute::SelectedBGColor)?TQString::number(p->selectedBGColor().rgb(),16):"");
    settings<<"---";
    config->writeEntry(p->name,settings);
  }
}

/**
 * Increase the usage count, and trigger initialization if needed.
 */
void KateHighlighting::use()
{
  if (refCount == 0)
    init();

  refCount++;
}

/**
 * Decrease the usage count, and trigger cleanup if needed.
 */
void KateHighlighting::release()
{
  refCount--;

  if (refCount == 0)
    done();
}

/**
 * Initialize a context for the first time.
 */

void KateHighlighting::init()
{
  if (noHl)
    return;

  // cu contexts
  for (uint i=0; i < m_contexts.size(); ++i)
    delete m_contexts[i];
  m_contexts.clear ();

  makeContextList();
}


/**
 * If the there is no document using the highlighting style free the complete
 * context structure.
 */
void KateHighlighting::done()
{
  if (noHl)
    return;

  // cu contexts
  for (uint i=0; i < m_contexts.size(); ++i)
    delete m_contexts[i];
  m_contexts.clear ();

  internalIDList.clear();
}

/**
 * KateHighlighting - createKateHlItemData
 * This function reads the itemData entries from the config file, which specifies the
 * default attribute styles for matched items/contexts.
 *
 * @param list A reference to the internal list containing the parsed default config
 */
void KateHighlighting::createKateHlItemData(KateHlItemDataList &list)
{
  // If no highlighting is selected we need only one default.
  if (noHl)
  {
    list.append(new KateHlItemData(i18n("Normal Text"), KateHlItemData::dsNormal));
    return;
  }

  // If the internal list isn't already available read the config file
  if (internalIDList.isEmpty())
    makeContextList();

  list=internalIDList;
}

/**
 * Adds the styles of the currently parsed highlight to the itemdata list
 */
void KateHighlighting::addToKateHlItemDataList()
{
  //Tell the syntax document class which file we want to parse and which data group
  KateHlManager::self()->syntax->setIdentifier(buildIdentifier);
  KateSyntaxContextData *data = KateHlManager::self()->syntax->getGroupInfo("highlighting","itemData");

  //begin with the real parsing
  while (KateHlManager::self()->syntax->nextGroup(data))
  {
    // read all attributes
    TQString color = KateHlManager::self()->syntax->groupData(data,TQString("color"));
    TQString selColor = KateHlManager::self()->syntax->groupData(data,TQString("selColor"));
    TQString bold = KateHlManager::self()->syntax->groupData(data,TQString("bold"));
    TQString italic = KateHlManager::self()->syntax->groupData(data,TQString("italic"));
    TQString underline = KateHlManager::self()->syntax->groupData(data,TQString("underline"));
    TQString strikeOut = KateHlManager::self()->syntax->groupData(data,TQString("strikeOut"));
    TQString bgColor = KateHlManager::self()->syntax->groupData(data,TQString("backgroundColor"));
    TQString selBgColor = KateHlManager::self()->syntax->groupData(data,TQString("selBackgroundColor"));

    KateHlItemData* newData = new KateHlItemData(
            buildPrefix+KateHlManager::self()->syntax->groupData(data,TQString("name")).simplifyWhiteSpace(),
            getDefStyleNum(KateHlManager::self()->syntax->groupData(data,TQString("defStyleNum"))));

    /* here the custom style overrides are specified, if needed */
    if (!color.isEmpty()) newData->setTextColor(TQColor(color));
    if (!selColor.isEmpty()) newData->setSelectedTextColor(TQColor(selColor));
    if (!bold.isEmpty()) newData->setBold( IS_TRUE(bold) );
    if (!italic.isEmpty()) newData->setItalic( IS_TRUE(italic) );
    // new attributes for the new rendering view
    if (!underline.isEmpty()) newData->setUnderline( IS_TRUE(underline) );
    if (!strikeOut.isEmpty()) newData->setStrikeOut( IS_TRUE(strikeOut) );
    if (!bgColor.isEmpty()) newData->setBGColor(TQColor(bgColor));
    if (!selBgColor.isEmpty()) newData->setSelectedBGColor(TQColor(selBgColor));

    internalIDList.append(newData);
  }

  //clean up
  if (data)
    KateHlManager::self()->syntax->freeGroupInfo(data);
}

/**
 * KateHighlighting - lookupAttrName
 * This function is  a helper for makeContextList and createKateHlItem. It looks the given
 * attribute name in the itemData list up and returns it's index
 *
 * @param name the attribute name to lookup
 * @param iDl the list containing all available attributes
 *
 * @return The index of the attribute, or 0 if the attribute isn't found
 */
int  KateHighlighting::lookupAttrName(const TQString& name, KateHlItemDataList &iDl)
{
  for (uint i = 0; i < iDl.count(); i++)
    if (iDl.at(i)->name == buildPrefix+name)
      return i;

  kdDebug(13010)<<"Couldn't resolve itemDataName:"<<name<<endl;
  return 0;
}

/**
 * KateHighlighting - createKateHlItem
 * This function is  a helper for makeContextList. It parses the xml file for
 * information.
 *
 * @param data Data about the item read from the xml file
 * @param iDl List of all available itemData entries.
 *            Needed for attribute name->index translation
 * @param RegionList list of code folding region names
 * @param ContextNameList list of context names
 *
 * @return A pointer to the newly created item object
 */
KateHlItem *KateHighlighting::createKateHlItem(KateSyntaxContextData *data,
                                               KateHlItemDataList &iDl,
                                               TQStringList *RegionList,
                                               TQStringList *ContextNameList)
{
  // No highlighting -> exit
  if (noHl)
    return 0;

  // get the (tagname) itemd type
  TQString dataname=KateHlManager::self()->syntax->groupItemData(data,TQString(""));

  // code folding region handling:
  TQString beginRegionStr=KateHlManager::self()->syntax->groupItemData(data,TQString("beginRegion"));
  TQString endRegionStr=KateHlManager::self()->syntax->groupItemData(data,TQString("endRegion"));

  signed char regionId=0;
  signed char regionId2=0;

  if (!beginRegionStr.isEmpty())
  {
    regionId = RegionList->findIndex(beginRegionStr);

    if (regionId==-1) // if the region name doesn't already exist, add it to the list
    {
      (*RegionList)<<beginRegionStr;
      regionId = RegionList->findIndex(beginRegionStr);
    }

    regionId++;

    kdDebug(13010) << "########### BEG REG: "  << beginRegionStr << " NUM: " << regionId << endl;
  }

  if (!endRegionStr.isEmpty())
  {
    regionId2 = RegionList->findIndex(endRegionStr);

    if (regionId2==-1) // if the region name doesn't already exist, add it to the list
    {
      (*RegionList)<<endRegionStr;
      regionId2 = RegionList->findIndex(endRegionStr);
    }

    regionId2 = -regionId2 - 1;

    kdDebug(13010) << "########### END REG: "  << endRegionStr << " NUM: " << regionId2 << endl;
  }

  int attr = 0;
  TQString tmpAttr=KateHlManager::self()->syntax->groupItemData(data,TQString("attribute")).simplifyWhiteSpace();
  bool onlyConsume = tmpAttr.isEmpty();

  // only relevant for non consumer
  if (!onlyConsume)
  {
    if (TQString("%1").arg(tmpAttr.toInt())==tmpAttr)
    {
      errorsAndWarnings+=i18n(
          "<B>%1</B>: Deprecated syntax. Attribute (%2) not addressed by symbolic name<BR>").
      arg(buildIdentifier).arg(tmpAttr);
      attr=tmpAttr.toInt();
    }
    else
      attr=lookupAttrName(tmpAttr,iDl);
  }

  // Info about context switch
  int context = -1;
  TQString unresolvedContext;
  TQString tmpcontext=KateHlManager::self()->syntax->groupItemData(data,TQString("context"));
  if (!tmpcontext.isEmpty())
    context=getIdFromString(ContextNameList, tmpcontext,unresolvedContext);

  // Get the char parameter (eg DetectChar)
  char chr;
  if (! KateHlManager::self()->syntax->groupItemData(data,TQString("char")).isEmpty())
    chr= (KateHlManager::self()->syntax->groupItemData(data,TQString("char")).latin1())[0];
  else
    chr=0;

  // Get the String parameter (eg. StringDetect)
  TQString stringdata=KateHlManager::self()->syntax->groupItemData(data,TQString("String"));

  // Get a second char parameter (char1) (eg Detect2Chars)
  char chr1;
  if (! KateHlManager::self()->syntax->groupItemData(data,TQString("char1")).isEmpty())
    chr1= (KateHlManager::self()->syntax->groupItemData(data,TQString("char1")).latin1())[0];
  else
    chr1=0;

  // Will be removed eventually. Atm used for StringDetect, keyword and RegExp
  const TQString & insensitive_str = KateHlManager::self()->syntax->groupItemData(data,TQString("insensitive"));
  bool insensitive = IS_TRUE( insensitive_str );

  // for regexp only
  bool minimal = IS_TRUE( KateHlManager::self()->syntax->groupItemData(data,TQString("minimal")) );

  // dominik: look ahead and do not change offset. so we can change contexts w/o changing offset1.
  bool lookAhead = IS_TRUE( KateHlManager::self()->syntax->groupItemData(data,TQString("lookAhead")) );

  bool dynamic= IS_TRUE(KateHlManager::self()->syntax->groupItemData(data,TQString("dynamic")) );

  bool firstNonSpace = IS_TRUE(KateHlManager::self()->syntax->groupItemData(data,TQString("firstNonSpace")) );

  int column = -1;
  TQString colStr = KateHlManager::self()->syntax->groupItemData(data,TQString("column"));
  if (!colStr.isEmpty())
    column = colStr.toInt();

  //Create the item corresponding to it's type and set it's parameters
  KateHlItem *tmpItem;

  if (dataname=="keyword")
  {
    bool keywordInsensitive = insensitive_str.isEmpty() ? !casesensitive : insensitive;
    KateHlKeyword *keyword=new KateHlKeyword(attr,context,regionId,regionId2,keywordInsensitive,
                                             m_additionalData[ buildIdentifier ]->deliminator);

    //Get the entries for the keyword lookup list
    keyword->addList(KateHlManager::self()->syntax->finddata("highlighting",stringdata));
    tmpItem=keyword;
  }
  else if (dataname=="Float") tmpItem= (new KateHlFloat(attr,context,regionId,regionId2));
  else if (dataname=="Int") tmpItem=(new KateHlInt(attr,context,regionId,regionId2));
  else if (dataname=="DetectChar") tmpItem=(new KateHlCharDetect(attr,context,regionId,regionId2,chr));
  else if (dataname=="Detect2Chars") tmpItem=(new KateHl2CharDetect(attr,context,regionId,regionId2,chr,chr1));
  else if (dataname=="RangeDetect") tmpItem=(new KateHlRangeDetect(attr,context,regionId,regionId2, chr, chr1));
  else if (dataname=="LineContinue") tmpItem=(new KateHlLineContinue(attr,context,regionId,regionId2));
  else if (dataname=="StringDetect") tmpItem=(new KateHlStringDetect(attr,context,regionId,regionId2,stringdata,insensitive));
  else if (dataname=="AnyChar") tmpItem=(new KateHlAnyChar(attr,context,regionId,regionId2,stringdata));
  else if (dataname=="RegExpr") tmpItem=(new KateHlRegExpr(attr,context,regionId,regionId2,stringdata, insensitive, minimal));
  else if (dataname=="HlCChar") tmpItem= ( new KateHlCChar(attr,context,regionId,regionId2));
  else if (dataname=="HlCHex") tmpItem= (new KateHlCHex(attr,context,regionId,regionId2));
  else if (dataname=="HlCOct") tmpItem= (new KateHlCOct(attr,context,regionId,regionId2));
  else if (dataname=="HlCFloat") tmpItem= (new KateHlCFloat(attr,context,regionId,regionId2));
  else if (dataname=="HlCStringChar") tmpItem= (new KateHlCStringChar(attr,context,regionId,regionId2));
  else if (dataname=="DetectSpaces") tmpItem= (new KateHlDetectSpaces(attr,context,regionId,regionId2));
  else if (dataname=="DetectIdentifier") tmpItem= (new KateHlDetectIdentifier(attr,context,regionId,regionId2));
  else
  {
    // oops, unknown type. Perhaps a spelling error in the xml file
    return 0;
  }

  // set lookAhead & dynamic properties
  tmpItem->lookAhead = lookAhead;
  tmpItem->dynamic = dynamic;
  tmpItem->firstNonSpace = firstNonSpace;
  tmpItem->column = column;
  tmpItem->onlyConsume = onlyConsume;

  if (!unresolvedContext.isEmpty())
  {
    unresolvedContextReferences.insert(&(tmpItem->ctx),unresolvedContext);
  }

  return tmpItem;
}

TQString KateHighlighting::hlKeyForAttrib( int i ) const
{
  // find entry. This is faster than TQMap::find. m_hlIndex always has an entry
  // for key '0' (it is "none"), so the result is always valid.
  int k = 0;
  TQMap<int,TQString>::const_iterator it = m_hlIndex.constEnd();
  while ( it != m_hlIndex.constBegin() )
  {
    --it;
    k = it.key();
    if ( i >= k )
      break;
  }
  return it.data();
}

bool KateHighlighting::isInWord( TQChar c, int attrib ) const
{
  return m_additionalData[ hlKeyForAttrib( attrib ) ]->deliminator.find(c) < 0
      && !c.isSpace() && c != '"' && c != '\'';
}

bool KateHighlighting::canBreakAt( TQChar c, int attrib ) const
{
  static const TQString& sq = TDEGlobal::staticQString("\"'");
  return (m_additionalData[ hlKeyForAttrib( attrib ) ]->wordWrapDeliminator.find(c) != -1) && (sq.find(c) == -1);
}

signed char KateHighlighting::commentRegion(int attr) const {
  TQString commentRegion=m_additionalData[ hlKeyForAttrib( attr ) ]->multiLineRegion;
  return (commentRegion.isEmpty()?0:(commentRegion.toShort()));
}

bool KateHighlighting::canComment( int startAttrib, int endAttrib ) const
{
  TQString k = hlKeyForAttrib( startAttrib );
  return ( k == hlKeyForAttrib( endAttrib ) &&
      ( ( !m_additionalData[k]->multiLineCommentStart.isEmpty() && !m_additionalData[k]->multiLineCommentEnd.isEmpty() ) ||
       ! m_additionalData[k]->singleLineCommentMarker.isEmpty() ) );
}

TQString KateHighlighting::getCommentStart( int attrib ) const
{
  return m_additionalData[ hlKeyForAttrib( attrib) ]->multiLineCommentStart;
}

TQString KateHighlighting::getCommentEnd( int attrib ) const
{
  return m_additionalData[ hlKeyForAttrib( attrib ) ]->multiLineCommentEnd;
}

TQString KateHighlighting::getCommentSingleLineStart( int attrib ) const
{
  return m_additionalData[ hlKeyForAttrib( attrib) ]->singleLineCommentMarker;
}

KateHighlighting::CSLPos KateHighlighting::getCommentSingleLinePosition( int attrib ) const
{
  return m_additionalData[ hlKeyForAttrib( attrib) ]->singleLineCommentPosition;
}


/**
 * Helper for makeContextList. It parses the xml file for
 * information, how single or multi line comments are marked
 */
void KateHighlighting::readCommentConfig()
{
  KateHlManager::self()->syntax->setIdentifier(buildIdentifier);
  KateSyntaxContextData *data=KateHlManager::self()->syntax->getGroupInfo("general","comment");

  TQString cmlStart="", cmlEnd="", cmlRegion="", cslStart="";
  CSLPos cslPosition=CSLPosColumn0;

  if (data)
  {
    while  (KateHlManager::self()->syntax->nextGroup(data))
    {
      if (KateHlManager::self()->syntax->groupData(data,"name")=="singleLine")
      {
        cslStart=KateHlManager::self()->syntax->groupData(data,"start");
        TQString cslpos=KateHlManager::self()->syntax->groupData(data,"position");
        if (cslpos=="afterwhitespace")
          cslPosition=CSLPosAfterWhitespace;
        else
          cslPosition=CSLPosColumn0;
      }
      else if (KateHlManager::self()->syntax->groupData(data,"name")=="multiLine")
      {
        cmlStart=KateHlManager::self()->syntax->groupData(data,"start");
        cmlEnd=KateHlManager::self()->syntax->groupData(data,"end");
        cmlRegion=KateHlManager::self()->syntax->groupData(data,"region");
      }
    }

    KateHlManager::self()->syntax->freeGroupInfo(data);
  }

  m_additionalData[buildIdentifier]->singleLineCommentMarker = cslStart;
  m_additionalData[buildIdentifier]->singleLineCommentPosition = cslPosition;
  m_additionalData[buildIdentifier]->multiLineCommentStart = cmlStart;
  m_additionalData[buildIdentifier]->multiLineCommentEnd = cmlEnd;
  m_additionalData[buildIdentifier]->multiLineRegion = cmlRegion;
}

/**
 * Helper for makeContextList. It parses the xml file for information,
 * if keywords should be treated case(in)sensitive and creates the keyword
 * delimiter list. Which is the default list, without any given weak deliminiators
 */
void KateHighlighting::readGlobalKeywordConfig()
{
  deliminator = stdDeliminator;
  // Tell the syntax document class which file we want to parse
  kdDebug(13010)<<"readGlobalKeywordConfig:BEGIN"<<endl;

  KateHlManager::self()->syntax->setIdentifier(buildIdentifier);
  KateSyntaxContextData *data = KateHlManager::self()->syntax->getConfig("general","keywords");

  if (data)
  {
    kdDebug(13010)<<"Found global keyword config"<<endl;

    if ( IS_TRUE( KateHlManager::self()->syntax->groupItemData(data,TQString("casesensitive")) ) )
      casesensitive=true;
    else
      casesensitive=false;

    //get the weak deliminators
    weakDeliminator=(KateHlManager::self()->syntax->groupItemData(data,TQString("weakDeliminator")));

    kdDebug(13010)<<"weak delimiters are: "<<weakDeliminator<<endl;

    // remove any weakDelimitars (if any) from the default list and store this list.
    for (uint s=0; s < weakDeliminator.length(); s++)
    {
      int f = deliminator.find (weakDeliminator[s]);

      if (f > -1)
        deliminator.remove (f, 1);
    }

    TQString addDelim = (KateHlManager::self()->syntax->groupItemData(data,TQString("additionalDeliminator")));

    if (!addDelim.isEmpty())
      deliminator=deliminator+addDelim;

    KateHlManager::self()->syntax->freeGroupInfo(data);
  }
  else
  {
    //Default values
    casesensitive=true;
    weakDeliminator=TQString("");
  }

  kdDebug(13010)<<"readGlobalKeywordConfig:END"<<endl;

  kdDebug(13010)<<"delimiterCharacters are: "<<deliminator<<endl;

  m_additionalData[buildIdentifier]->deliminator = deliminator;
}

/**
 * Helper for makeContextList. It parses the xml file for any wordwrap
 * deliminators, characters * at which line can be broken. In case no keyword
 * tag is found in the xml file, the wordwrap deliminators list defaults to the
 * standard denominators. In case a keyword tag is defined, but no
 * wordWrapDeliminator attribute is specified, the deliminator list as computed
 * in readGlobalKeywordConfig is used.
 *
 * @return the computed delimiter string.
 */
void KateHighlighting::readWordWrapConfig()
{
  // Tell the syntax document class which file we want to parse
  kdDebug(13010)<<"readWordWrapConfig:BEGIN"<<endl;

  KateHlManager::self()->syntax->setIdentifier(buildIdentifier);
  KateSyntaxContextData *data = KateHlManager::self()->syntax->getConfig("general","keywords");

  TQString wordWrapDeliminator = stdDeliminator;
  if (data)
  {
    kdDebug(13010)<<"Found global keyword config"<<endl;

    wordWrapDeliminator = (KateHlManager::self()->syntax->groupItemData(data,TQString("wordWrapDeliminator")));
    //when no wordWrapDeliminator is defined use the deliminator list
    if ( wordWrapDeliminator.length() == 0 ) wordWrapDeliminator = deliminator;

    kdDebug(13010) << "word wrap deliminators are " << wordWrapDeliminator << endl;

    KateHlManager::self()->syntax->freeGroupInfo(data);
  }

  kdDebug(13010)<<"readWordWrapConfig:END"<<endl;

  m_additionalData[buildIdentifier]->wordWrapDeliminator = wordWrapDeliminator;
}

void KateHighlighting::readIndentationConfig()
{
  m_indentation = "";

  KateHlManager::self()->syntax->setIdentifier(buildIdentifier);
  KateSyntaxContextData *data = KateHlManager::self()->syntax->getConfig("general","indentation");

  if (data)
  {
    m_indentation = (KateHlManager::self()->syntax->groupItemData(data,TQString("mode")));

    KateHlManager::self()->syntax->freeGroupInfo(data);
  }
}

void KateHighlighting::readFoldingConfig()
{
  // Tell the syntax document class which file we want to parse
  kdDebug(13010)<<"readfoldignConfig:BEGIN"<<endl;

  KateHlManager::self()->syntax->setIdentifier(buildIdentifier);
  KateSyntaxContextData *data = KateHlManager::self()->syntax->getConfig("general","folding");

  if (data)
  {
    kdDebug(13010)<<"Found global keyword config"<<endl;

    if ( IS_TRUE( KateHlManager::self()->syntax->groupItemData(data,TQString("indentationsensitive")) ) )
      m_foldingIndentationSensitive=true;
    else
      m_foldingIndentationSensitive=false;

    KateHlManager::self()->syntax->freeGroupInfo(data);
  }
  else
  {
    //Default values
    m_foldingIndentationSensitive = false;
  }

  kdDebug(13010)<<"readfoldingConfig:END"<<endl;

  kdDebug(13010)<<"############################ use indent for fold are: "<<m_foldingIndentationSensitive<<endl;
}

void  KateHighlighting::createContextNameList(TQStringList *ContextNameList,int ctx0)
{
  kdDebug(13010)<<"creatingContextNameList:BEGIN"<<endl;

  if (ctx0 == 0)
      ContextNameList->clear();

  KateHlManager::self()->syntax->setIdentifier(buildIdentifier);

  KateSyntaxContextData *data=KateHlManager::self()->syntax->getGroupInfo("highlighting","context");

  int id=ctx0;

  if (data)
  {
     while (KateHlManager::self()->syntax->nextGroup(data))
     {
          TQString tmpAttr=KateHlManager::self()->syntax->groupData(data,TQString("name")).simplifyWhiteSpace();
    if (tmpAttr.isEmpty())
    {
     tmpAttr=TQString("!KATE_INTERNAL_DUMMY! %1").arg(id);
     errorsAndWarnings +=i18n("<B>%1</B>: Deprecated syntax. Context %2 has no symbolic name<BR>").arg(buildIdentifier).arg(id-ctx0);
    }
          else tmpAttr=buildPrefix+tmpAttr;
    (*ContextNameList)<<tmpAttr;
          id++;
     }
     KateHlManager::self()->syntax->freeGroupInfo(data);
  }
  kdDebug(13010)<<"creatingContextNameList:END"<<endl;

}

int KateHighlighting::getIdFromString(TQStringList *ContextNameList, TQString tmpLineEndContext, /*NO CONST*/ TQString &unres)
{
  unres="";
  int context;
  if ((tmpLineEndContext=="#stay") || (tmpLineEndContext.simplifyWhiteSpace().isEmpty()))
    context=-1;

  else if (tmpLineEndContext.startsWith("#pop"))
  {
    context=-1;
    for(;tmpLineEndContext.startsWith("#pop");context--)
    {
      tmpLineEndContext.remove(0,4);
      kdDebug(13010)<<"#pop found"<<endl;
    }
  }

  else if ( tmpLineEndContext.contains("##"))
  {
    int o = tmpLineEndContext.find("##");
    // FIXME at least with 'foo##bar'-style contexts the rules are picked up
    // but the default attribute is not
    TQString tmp=tmpLineEndContext.mid(o+2);
    if (!embeddedHls.contains(tmp))  embeddedHls.insert(tmp,KateEmbeddedHlInfo());
    unres=tmp+':'+tmpLineEndContext.left(o);
    context=0;
  }

  else
  {
    context=ContextNameList->findIndex(buildPrefix+tmpLineEndContext);
    if (context==-1)
    {
      context=tmpLineEndContext.toInt();
      errorsAndWarnings+=i18n(
        "<B>%1</B>:Deprecated syntax. Context %2 not addressed by a symbolic name"
        ).arg(buildIdentifier).arg(tmpLineEndContext);
    }
//#warning restructure this the name list storage.
//    context=context+buildContext0Offset;
  }
  return context;
}

/**
 * The most important initialization function for each highlighting. It's called
 * each time a document gets a highlighting style assigned. parses the xml file
 * and creates a corresponding internal structure
 */
void KateHighlighting::makeContextList()
{
  if (noHl)  // if this a highlighting for "normal texts" only, tere is no need for a context list creation
    return;

  embeddedHls.clear();
  unresolvedContextReferences.clear();
  RegionList.clear();
  ContextNameList.clear();

  // prepare list creation. To reuse as much code as possible handle this
  // highlighting the same way as embedded onces
  embeddedHls.insert(iName,KateEmbeddedHlInfo());

  bool something_changed;
  // the context "0" id is 0 for this hl, all embedded context "0"s have offsets
  startctx=base_startctx=0;
  // inform everybody that we are building the highlighting contexts and itemlists
  building=true;

  do
  {
    kdDebug(13010)<<"**************** Outer loop in make ContextList"<<endl;
    kdDebug(13010)<<"**************** Hl List count:"<<embeddedHls.count()<<endl;
    something_changed=false; //assume all "embedded" hls have already been loaded
    for (KateEmbeddedHlInfos::const_iterator it=embeddedHls.begin(); it!=embeddedHls.end();++it)
    {
      if (!it.data().loaded)  // we found one, we still have to load
      {
        kdDebug(13010)<<"**************** Inner loop in make ContextList"<<endl;
        TQString identifierToUse;
        kdDebug(13010)<<"Trying to open highlighting definition file: "<< it.key()<<endl;
        if (iName==it.key()) // the own identifier is known
          identifierToUse=identifier;
        else                 // all others have to be looked up
          identifierToUse=KateHlManager::self()->identifierForName(it.key());

        kdDebug(13010)<<"Location is:"<< identifierToUse<<endl;

        buildPrefix=it.key()+':';  // attribute names get prefixed by the names
                                   // of the highlighting definitions they belong to

        if (identifierToUse.isEmpty())
        {
          kdDebug(13010)<<"OHOH, unknown highlighting description referenced"<<endl;
          kdDebug(13010)<<"Highlighting for ("<<it.key()<<") can not be loaded"<<endl;
        }
        else
        {
          // Only do this if we have a non-empty identifier
          kdDebug(13010)<<"setting ("<<it.key()<<") to loaded"<<endl;
  
          //mark hl as loaded
          it=embeddedHls.insert(it.key(),KateEmbeddedHlInfo(true,startctx));
          //set class member for context 0 offset, so we don't need to pass it around
          buildContext0Offset=startctx;
          //parse one hl definition file
          startctx=addToContextList(identifierToUse,startctx);
  
          if (noHl) return;  // an error occurred
  
          base_startctx = startctx;
          something_changed=true; // something has been loaded
        }
      }
    }
  } while (something_changed);  // as long as there has been another file parsed
                  // repeat everything, there could be newly added embedded hls.

  // at this point all needed highlighing (sub)definitions are loaded. It's time
  // to resolve cross file  references (if there are any)
  kdDebug(13010)<<"Unresolved contexts, which need attention: "<<unresolvedContextReferences.count()<<endl;

  //optimize this a littlebit
  for (KateHlUnresolvedCtxRefs::iterator unresIt=unresolvedContextReferences.begin();
    unresIt!=unresolvedContextReferences.end();++unresIt)
  {
    TQString incCtx = unresIt.data();
    kdDebug(13010)<<"Context "<<incCtx<<" is unresolved"<<endl;
    // only resolve '##Name' contexts here; handleKateHlIncludeRules() can figure
    // out 'Name##Name'-style inclusions, but we screw it up
    if (incCtx.endsWith(":")) {
      kdDebug(13010)<<"Looking up context0 for ruleset "<<incCtx<<endl;
      incCtx = incCtx.left(incCtx.length()-1);
      //try to find the context0 id for a given unresolvedReference
      KateEmbeddedHlInfos::const_iterator hlIt=embeddedHls.find(incCtx);
      if (hlIt!=embeddedHls.end())
        *(unresIt.key())=hlIt.data().context0;
    }
  }

  // eventually handle KateHlIncludeRules items, if they exist.
  // This has to be done after the cross file references, because it is allowed
  // to include the context0 from a different definition, than the one the rule
  // belongs to
  handleKateHlIncludeRules();

  embeddedHls.clear(); //save some memory.
  unresolvedContextReferences.clear(); //save some memory
  RegionList.clear();  // I think you get the idea ;)
  ContextNameList.clear();


  // if there have been errors show them
  if (!errorsAndWarnings.isEmpty())
  KMessageBox::detailedSorry(0L,i18n(
        "There were warning(s) and/or error(s) while parsing the syntax "
        "highlighting configuration."),
        errorsAndWarnings, i18n("Kate Syntax Highlighting Parser"));

  // we have finished
  building=false;
}

void KateHighlighting::handleKateHlIncludeRules()
{
  // if there are noe include rules to take care of, just return
  kdDebug(13010)<<"KateHlIncludeRules, which need attention: " <<includeRules.count()<<endl;
  if (includeRules.isEmpty()) return;

  buildPrefix="";
  TQString dummy;

  // By now the context0 references are resolved, now more or less only inner
  // file references are resolved. If we decide that arbitrary inclusion is
  // needed, this doesn't need to be changed, only the addToContextList
  // method.

  //resolove context names
  for (KateHlIncludeRules::iterator it=includeRules.begin();it!=includeRules.end();)
  {
    if ((*it)->incCtx==-1) // context unresolved ?
    {

      if ((*it)->incCtxN.isEmpty())
      {
        // no context name given, and no valid context id set, so this item is
        // going to be removed
        KateHlIncludeRules::iterator it1=it;
        ++it1;
        delete (*it);
        includeRules.remove(it);
        it=it1;
      }
      else
      {
        // resolve name to id
        (*it)->incCtx=getIdFromString(&ContextNameList,(*it)->incCtxN,dummy);
        kdDebug(13010)<<"Resolved "<<(*it)->incCtxN<< " to "<<(*it)->incCtx<<" for include rule"<<endl;
        // It would be good to look here somehow, if the result is valid
      }
    }
    else ++it; //nothing to do, already resolved (by the cross defintion reference resolver)
  }

  // now that all KateHlIncludeRule items should be valid and completely resolved,
  // do the real inclusion of the rules.
  // recursiveness is needed, because context 0 could include context 1, which
  // itself includes context 2 and so on.
  //  In that case we have to handle context 2 first, then 1, 0
  //TODO: catch circular references: eg 0->1->2->3->1
  while (!includeRules.isEmpty())
    handleKateHlIncludeRulesRecursive(includeRules.begin(),&includeRules);
}

void KateHighlighting::handleKateHlIncludeRulesRecursive(KateHlIncludeRules::iterator it, KateHlIncludeRules *list)
{
  if (it==list->end()) return;  //invalid iterator, shouldn't happen, but better have a rule prepared ;)

  KateHlIncludeRules::iterator it1=it;
  int ctx=(*it1)->ctx;

  // find the last entry for the given context in the KateHlIncludeRules list
  // this is need if one context includes more than one. This saves us from
  // updating all insert positions:
  // eg: context 0:
  // pos 3 - include context 2
  // pos 5 - include context 3
  // During the building of the includeRules list the items are inserted in
  // ascending order, now we need it descending to make our life easier.
  while ((it!=list->end()) && ((*it)->ctx==ctx))
  {
    it1=it;
    ++it;
  }

  // iterate over each include rule for the context the function has been called for.
  while ((it1!=list->end()) && ((*it1)->ctx==ctx))
  {
    int ctx1=(*it1)->incCtx;

    //let's see, if the the included context includes other contexts
    for (KateHlIncludeRules::iterator it2=list->begin();it2!=list->end();++it2)
    {
      if ((*it2)->ctx==ctx1)
      {
        //yes it does, so first handle that include rules, since we want to
        // include those subincludes too
        handleKateHlIncludeRulesRecursive(it2,list);
        break;
      }
    }

    // if the context we want to include had sub includes, they are already inserted there.
    KateHlContext *dest=m_contexts[ctx];
    KateHlContext *src=m_contexts[ctx1];
//     kdDebug(3010)<<"linking included rules from "<<ctx<<" to "<<ctx1<<endl;

    // If so desired, change the dest attribute to the one of the src.
    // Required to make commenting work, if text matched by the included context
    // is a different highlight than the host context.
    if ( (*it1)->includeAttrib )
      dest->attr = src->attr;

    // insert the included context's rules starting at position p
    int p=(*it1)->pos;

    // remember some stuff
    int oldLen = dest->items.size();
    uint itemsToInsert = src->items.size();

    // resize target
    dest->items.resize (oldLen + itemsToInsert);

    // move old elements
    for (int i=oldLen-1; i >= p; --i)
      dest->items[i+itemsToInsert] = dest->items[i];

    // insert new stuff
    for (uint i=0; i < itemsToInsert; ++i  )
      dest->items[p+i] = src->items[i];

    it=it1; //backup the iterator
    --it1;  //move to the next entry, which has to be take care of
    delete (*it); //free the already handled data structure
    list->remove(it); // remove it from the list
  }
}

/**
 * Add one highlight to the contextlist.
 *
 * @return the number of contexts after this is added.
 */
int KateHighlighting::addToContextList(const TQString &ident, int ctx0)
{
  kdDebug(13010)<<"=== Adding hl with ident '"<<ident<<"'"<<endl;

  buildIdentifier=ident;
  KateSyntaxContextData *data, *datasub;
  KateHlItem *c;

  TQString dummy;

  // Let the syntax document class know, which file we'd like to parse
  if (!KateHlManager::self()->syntax->setIdentifier(ident))
  {
    noHl=true;
    KMessageBox::information(0L,i18n(
        "Since there has been an error parsing the highlighting description, "
        "this highlighting will be disabled"));
    return 0;
  }

  // only read for the own stuff
  if (identifier == ident)
  {
    readIndentationConfig ();
  }

  RegionList<<"!KateInternal_TopLevel!";

  m_hlIndex[internalIDList.count()] = ident;
  m_additionalData.insert( ident, new HighlightPropertyBag );

  // fill out the propertybag
  readCommentConfig();
  readGlobalKeywordConfig();
  readWordWrapConfig();

  readFoldingConfig ();

  TQString ctxName;

  // This list is needed for the translation of the attribute parameter,
  // if the itemData name is given instead of the index
  addToKateHlItemDataList();
  KateHlItemDataList iDl = internalIDList;

  createContextNameList(&ContextNameList,ctx0);


  kdDebug(13010)<<"Parsing Context structure"<<endl;
  //start the real work
  data=KateHlManager::self()->syntax->getGroupInfo("highlighting","context");
  uint i=buildContext0Offset;
  if (data)
  {
    while (KateHlManager::self()->syntax->nextGroup(data))
    {
      kdDebug(13010)<<"Found a context in file, building structure now"<<endl;
      //BEGIN - Translation of the attribute parameter
      TQString tmpAttr=KateHlManager::self()->syntax->groupData(data,TQString("attribute")).simplifyWhiteSpace();
      int attr;
      if (TQString("%1").arg(tmpAttr.toInt())==tmpAttr)
        attr=tmpAttr.toInt();
      else
        attr=lookupAttrName(tmpAttr,iDl);
      //END - Translation of the attribute parameter

      ctxName=buildPrefix+KateHlManager::self()->syntax->groupData(data,TQString("lineEndContext")).simplifyWhiteSpace();

      TQString tmpLineEndContext=KateHlManager::self()->syntax->groupData(data,TQString("lineEndContext")).simplifyWhiteSpace();
      int context;

      context=getIdFromString(&ContextNameList, tmpLineEndContext,dummy);

      TQString tmpNIBF = KateHlManager::self()->syntax->groupData(data, TQString("noIndentationBasedFolding") );
      bool noIndentationBasedFolding=IS_TRUE(tmpNIBF);

      //BEGIN get fallthrough props
      bool ft = false;
      int ftc = 0; // fallthrough context
      if ( i > 0 )  // fallthrough is not smart in context 0
      {
        TQString tmpFt = KateHlManager::self()->syntax->groupData(data, TQString("fallthrough") );
        if ( IS_TRUE(tmpFt) )
          ft = true;
        if ( ft )
        {
          TQString tmpFtc = KateHlManager::self()->syntax->groupData( data, TQString("fallthroughContext") );

          ftc=getIdFromString(&ContextNameList, tmpFtc,dummy);
          if (ftc == -1) ftc =0;

          kdDebug(13010)<<"Setting fall through context (context "<<i<<"): "<<ftc<<endl;
        }
      }
      //END falltrhough props

      bool dynamic = false;
      TQString tmpDynamic = KateHlManager::self()->syntax->groupData(data, TQString("dynamic") );
      if ( tmpDynamic.lower() == "true" ||  tmpDynamic.toInt() == 1 )
        dynamic = true;

      KateHlContext *ctxNew = new KateHlContext (
        ident,
        attr,
        context,
        (KateHlManager::self()->syntax->groupData(data,TQString("lineBeginContext"))).isEmpty()?-1:
        (KateHlManager::self()->syntax->groupData(data,TQString("lineBeginContext"))).toInt(),
        ft, ftc, dynamic,noIndentationBasedFolding);

      m_contexts.push_back (ctxNew);

      kdDebug(13010) << "INDEX: " << i << " LENGTH " << m_contexts.size()-1 << endl;

      //Let's create all items for the context
      while (KateHlManager::self()->syntax->nextItem(data))
      {
//    kdDebug(13010)<< "In make Contextlist: Item:"<<endl;

      // KateHlIncludeRules : add a pointer to each item in that context
        // TODO add a attrib includeAttrib
      TQString tag = KateHlManager::self()->syntax->groupItemData(data,TQString(""));
      if ( tag == "IncludeRules" ) //if the new item is an Include rule, we have to take special care
      {
        TQString incCtx = KateHlManager::self()->syntax->groupItemData( data, TQString("context"));
        TQString incAttrib = KateHlManager::self()->syntax->groupItemData( data, TQString("includeAttrib"));
        bool includeAttrib = IS_TRUE( incAttrib );
        // only context refernces of type Name, ##Name, and Subname##Name are allowed
        if (incCtx.startsWith("##") || (!incCtx.startsWith("#")))
        {
          int incCtxi = incCtx.find("##");
          //#stay, #pop is not interesting here
          if (incCtxi >= 0)
          {
            TQString incSet = incCtx.mid(incCtxi + 2);
            TQString incCtxN = incSet + ":" + incCtx.left(incCtxi);

            //a cross highlighting reference
            kdDebug(13010)<<"Cross highlight reference <IncludeRules>, context "<<incCtxN<<endl;
            KateHlIncludeRule *ir=new KateHlIncludeRule(i,m_contexts[i]->items.count(),incCtxN,includeAttrib);

            //use the same way to determine cross hl file references as other items do
            if (!embeddedHls.contains(incSet))
              embeddedHls.insert(incSet,KateEmbeddedHlInfo());
            else
              kdDebug(13010)<<"Skipping embeddedHls.insert for "<<incCtxN<<endl;

            unresolvedContextReferences.insert(&(ir->incCtx), incCtxN);

            includeRules.append(ir);
          }
          else
          {
            // a local reference -> just initialize the include rule structure
            incCtx=buildPrefix+incCtx.simplifyWhiteSpace();
            includeRules.append(new KateHlIncludeRule(i,m_contexts[i]->items.count(),incCtx, includeAttrib));
          }
        }

        continue;
      }
      // TODO -- can we remove the block below??
#if 0
                TQString tag = KateHlManager::self()->syntax->groupKateHlItemData(data,TQString(""));
                if ( tag == "IncludeRules" ) {
                  // attrib context: the index (jowenn, i think using names here
                  // would be a cool feat, goes for mentioning the context in
                  // any item. a map or dict?)
                  int ctxId = getIdFromString(&ContextNameList,
                                               KateHlManager::self()->syntax->groupKateHlItemData( data, TQString("context")),dummy); // the index is *required*
                  if ( ctxId > -1) { // we can even reuse rules of 0 if we want to:)
                    kdDebug(13010)<<"makeContextList["<<i<<"]: including all items of context "<<ctxId<<endl;
                    if ( ctxId < (int) i ) { // must be defined
                      for ( c = m_contexts[ctxId]->items.first(); c; c = m_contexts[ctxId]->items.next() )
                        m_contexts[i]->items.append(c);
                    }
                    else
                      kdDebug(13010)<<"Context "<<ctxId<<"not defined. You can not include the rules of an undefined context"<<endl;
                  }
                  continue; // while nextItem
                }
#endif
      c=createKateHlItem(data,iDl,&RegionList,&ContextNameList);
      if (c)
      {
        m_contexts[i]->items.append(c);

        // Not supported completely atm and only one level. Subitems.(all have
        // to be matched to at once)
        datasub=KateHlManager::self()->syntax->getSubItems(data);
        bool tmpbool;
        if ((tmpbool = KateHlManager::self()->syntax->nextItem(datasub)))
        {
          for (;tmpbool;tmpbool=KateHlManager::self()->syntax->nextItem(datasub))
          {
            c->subItems.resize (c->subItems.size()+1);
            c->subItems[c->subItems.size()-1] = createKateHlItem(datasub,iDl,&RegionList,&ContextNameList);
          }                             }
          KateHlManager::self()->syntax->freeGroupInfo(datasub);
                              // end of sublevel
        }
      }
      i++;
    }
  }

  KateHlManager::self()->syntax->freeGroupInfo(data);

  if (RegionList.count()!=1)
    folding=true;

  folding = folding || m_foldingIndentationSensitive;

  //BEGIN Resolve multiline region if possible
  if (!m_additionalData[ ident ]->multiLineRegion.isEmpty()) {
    long commentregionid=RegionList.findIndex( m_additionalData[ ident ]->multiLineRegion );
    if (-1==commentregionid) {
      errorsAndWarnings+=i18n(
          "<B>%1</B>: Specified multiline comment region (%2) could not be resolved<BR>"
                             ).arg(buildIdentifier).arg( m_additionalData[ ident ]->multiLineRegion );
      m_additionalData[ ident ]->multiLineRegion = TQString();
      kdDebug(13010)<<"ERROR comment region attribute could not be resolved"<<endl;

    } else {
      m_additionalData[ ident ]->multiLineRegion=TQString::number(commentregionid+1);
      kdDebug(13010)<<"comment region resolved to:"<<m_additionalData[ ident ]->multiLineRegion<<endl;
    }
  }
  //END Resolve multiline region if possible
  return i;
}

void KateHighlighting::clearAttributeArrays ()
{
  for ( TQIntDictIterator< TQMemArray<KateAttribute> > it( m_attributeArrays ); it.current(); ++it )
  {
    // k, schema correct, let create the data
    KateAttributeList defaultStyleList;
    defaultStyleList.setAutoDelete(true);
    KateHlManager::self()->getDefaults(it.currentKey(), defaultStyleList);

    KateHlItemDataList itemDataList;
    getKateHlItemDataList(it.currentKey(), itemDataList);

    uint nAttribs = itemDataList.count();
    TQMemArray<KateAttribute> *array = it.current();
    array->resize (nAttribs);

    for (uint z = 0; z < nAttribs; z++)
    {
      KateHlItemData *itemData = itemDataList.at(z);
      KateAttribute n = *defaultStyleList.at(itemData->defStyleNum);

      if (itemData && itemData->isSomethingSet())
        n += *itemData;

      array->at(z) = n;
    }
  }
}

TQMemArray<KateAttribute> *KateHighlighting::attributes (uint schema)
{
  TQMemArray<KateAttribute> *array;

  // found it, allready floating around
  if ((array = m_attributeArrays[schema]))
    return array;

  // ohh, not found, check if valid schema number
  if (!KateFactory::self()->schemaManager()->validSchema(schema))
  {
    // uhh, not valid :/, stick with normal default schema, it's always there !
    return attributes (0);
  }

  // k, schema correct, let create the data
  KateAttributeList defaultStyleList;
  defaultStyleList.setAutoDelete(true);
  KateHlManager::self()->getDefaults(schema, defaultStyleList);

  KateHlItemDataList itemDataList;
  getKateHlItemDataList(schema, itemDataList);

  uint nAttribs = itemDataList.count();
  array = new TQMemArray<KateAttribute> (nAttribs);

  for (uint z = 0; z < nAttribs; z++)
  {
    KateHlItemData *itemData = itemDataList.at(z);
    KateAttribute n = *defaultStyleList.at(itemData->defStyleNum);

    if (itemData && itemData->isSomethingSet())
      n += *itemData;

    array->at(z) = n;
  }

  m_attributeArrays.insert(schema, array);

  return array;
}

void KateHighlighting::getKateHlItemDataListCopy (uint schema, KateHlItemDataList &outlist)
{
  KateHlItemDataList itemDataList;
  getKateHlItemDataList(schema, itemDataList);

  outlist.clear ();
  outlist.setAutoDelete (true);
  for (uint z=0; z < itemDataList.count(); z++)
    outlist.append (new KateHlItemData (*itemDataList.at(z)));
}

//END

//BEGIN KateHlManager
KateHlManager::KateHlManager()
  : TQObject()
  , m_config ("katesyntaxhighlightingrc", false, false)
  , commonSuffixes (TQStringList::split(";", ".orig;.new;~;.bak;.BAK"))
  , syntax (new KateSyntaxDocument())
  , dynamicCtxsCount(0)
  , forceNoDCReset(false)
{
  hlList.setAutoDelete(true);
  hlDict.setAutoDelete(false);

  KateSyntaxModeList modeList = syntax->modeList();
  for (uint i=0; i < modeList.count(); i++)
  {
    KateHighlighting *hl = new KateHighlighting(modeList[i]);

    uint insert = 0;
    for (; insert <= hlList.count(); insert++)
    {
      if (insert == hlList.count())
        break;

      if ( TQString(hlList.at(insert)->section() + hlList.at(insert)->nameTranslated()).lower()
            > TQString(hl->section() + hl->nameTranslated()).lower() )
        break;
    }

    hlList.insert (insert, hl);
    hlDict.insert (hl->name(), hl);
  }

  // Normal HL
  KateHighlighting *hl = new KateHighlighting(0);
  hlList.prepend (hl);
  hlDict.insert (hl->name(), hl);

  lastCtxsReset.start();
}

KateHlManager::~KateHlManager()
{
  delete syntax;
}

static KStaticDeleter<KateHlManager> sdHlMan;

KateHlManager *KateHlManager::self()
{
  if ( !s_self )
    sdHlMan.setObject(s_self, new KateHlManager ());

  return s_self;
}

KateHighlighting *KateHlManager::getHl(int n)
{
  if (n < 0 || n >= (int) hlList.count())
    n = 0;

  return hlList.at(n);
}

int KateHlManager::nameFind(const TQString &name)
{
  int z (hlList.count() - 1);
  for (; z > 0; z--)
    if (hlList.at(z)->name() == name)
      return z;

  return z;
}

int KateHlManager::detectHighlighting (KateDocument *doc)
{
  int hl = wildcardFind( doc->url().filename() );
  if ( hl < 0 )
    hl = mimeFind ( doc );

  return hl;
}

int KateHlManager::wildcardFind(const TQString &fileName)
{
  int result = -1;
  if ((result = realWildcardFind(fileName)) != -1)
    return result;

  int length = fileName.length();
  TQString backupSuffix = KateDocumentConfig::global()->backupSuffix();
  if (fileName.endsWith(backupSuffix)) {
    if ((result = realWildcardFind(fileName.left(length - backupSuffix.length()))) != -1)
      return result;
  }

  for (TQStringList::Iterator it = commonSuffixes.begin(); it != commonSuffixes.end(); ++it) {
    if (*it != backupSuffix && fileName.endsWith(*it)) {
      if ((result = realWildcardFind(fileName.left(length - (*it).length()))) != -1)
        return result;
    }
  }

  return -1;
}

int KateHlManager::realWildcardFind(const TQString &fileName)
{
  static TQRegExp sep("\\s*;\\s*");

  TQPtrList<KateHighlighting> highlights;

  for (KateHighlighting *highlight = hlList.first(); highlight != 0L; highlight = hlList.next()) {
    highlight->loadWildcards();

    for (TQStringList::Iterator it = highlight->getPlainExtensions().begin(); it != highlight->getPlainExtensions().end(); ++it)
      if (fileName.endsWith((*it)))
        highlights.append(highlight);

    for (int i = 0; i < (int)highlight->getRegexpExtensions().count(); i++) {
      TQRegExp re = highlight->getRegexpExtensions()[i];
      if (re.exactMatch(fileName))
        highlights.append(highlight);
    }
  }

  if ( !highlights.isEmpty() )
  {
    int pri = -1;
    int hl = -1;

    for (KateHighlighting *highlight = highlights.first(); highlight != 0L; highlight = highlights.next())
    {
      if (highlight->priority() > pri)
      {
        pri = highlight->priority();
        hl = hlList.findRef (highlight);
      }
    }
    return hl;
  }

  return -1;
}

int KateHlManager::mimeFind( KateDocument *doc )
{
  static TQRegExp sep("\\s*;\\s*");

  KMimeType::Ptr mt = doc->mimeTypeForContent();

  TQPtrList<KateHighlighting> highlights;

  for (KateHighlighting *highlight = hlList.first(); highlight != 0L; highlight = hlList.next())
  {
    TQStringList l = TQStringList::split( sep, highlight->getMimetypes() );

    for( TQStringList::Iterator it = l.begin(); it != l.end(); ++it )
    {
      if ( *it == mt->name() ) // faster than a regexp i guess?
        highlights.append (highlight);
    }
  }

  if ( !highlights.isEmpty() )
  {
    int pri = -1;
    int hl = -1;

    for (KateHighlighting *highlight = highlights.first(); highlight != 0L; highlight = highlights.next())
    {
      if (highlight->priority() > pri)
      {
        pri = highlight->priority();
        hl = hlList.findRef (highlight);
      }
    }

    return hl;
  }

  return -1;
}

uint KateHlManager::defaultStyles()
{
  return 14;
}

TQString KateHlManager::defaultStyleName(int n, bool translateNames)
{
  static TQStringList names;
  static TQStringList translatedNames;

  if (names.isEmpty())
  {
    names << "Normal";
    names << "Keyword";
    names << "Data Type";
    names << "Decimal/Value";
    names << "Base-N Integer";
    names << "Floating Point";
    names << "Character";
    names << "String";
    names << "Comment";
    names << "Others";
    names << "Alert";
    names << "Function";
    // this next one is for denoting the beginning/end of a user defined folding region
    names << "Region Marker";
    // this one is for marking invalid input
    names << "Error";

    translatedNames << i18n("Normal");
    translatedNames << i18n("Keyword");
    translatedNames << i18n("Data Type");
    translatedNames << i18n("Decimal/Value");
    translatedNames << i18n("Base-N Integer");
    translatedNames << i18n("Floating Point");
    translatedNames << i18n("Character");
    translatedNames << i18n("String");
    translatedNames << i18n("Comment");
    translatedNames << i18n("Others");
    translatedNames << i18n("Alert");
    translatedNames << i18n("Function");
    // this next one is for denoting the beginning/end of a user defined folding region
    translatedNames << i18n("Region Marker");
    // this one is for marking invalid input
    translatedNames << i18n("Error");
  }

  return translateNames ? translatedNames[n] : names[n];
}

void KateHlManager::getDefaults(uint schema, KateAttributeList &list)
{
  list.setAutoDelete(true);

  KateAttribute* normal = new KateAttribute();
  normal->setTextColor(TQt::black);
  normal->setSelectedTextColor(TQt::white);
  list.append(normal);

  KateAttribute* keyword = new KateAttribute();
  keyword->setTextColor(TQt::black);
  keyword->setSelectedTextColor(TQt::white);
  keyword->setBold(true);
  list.append(keyword);

  KateAttribute* dataType = new KateAttribute();
  dataType->setTextColor(TQt::darkRed);
  dataType->setSelectedTextColor(TQt::white);
  list.append(dataType);

  KateAttribute* decimal = new KateAttribute();
  decimal->setTextColor(TQt::blue);
  decimal->setSelectedTextColor(TQt::cyan);
  list.append(decimal);

  KateAttribute* basen = new KateAttribute();
  basen->setTextColor(TQt::darkCyan);
  basen->setSelectedTextColor(TQt::cyan);
  list.append(basen);

  KateAttribute* floatAttribute = new KateAttribute();
  floatAttribute->setTextColor(TQt::darkMagenta);
  floatAttribute->setSelectedTextColor(TQt::cyan);
  list.append(floatAttribute);

  KateAttribute* charAttribute = new KateAttribute();
  charAttribute->setTextColor(TQt::magenta);
  charAttribute->setSelectedTextColor(TQt::magenta);
  list.append(charAttribute);

  KateAttribute* string = new KateAttribute();
  string->setTextColor(TQColor("#D00"));
  string->setSelectedTextColor(TQt::red);
  list.append(string);

  KateAttribute* comment = new KateAttribute();
  comment->setTextColor(TQt::darkGray);
  comment->setSelectedTextColor(TQt::gray);
  comment->setItalic(true);
  list.append(comment);

  KateAttribute* others = new KateAttribute();
  others->setTextColor(TQt::darkGreen);
  others->setSelectedTextColor(TQt::green);
  list.append(others);

  KateAttribute* alert = new KateAttribute();
  alert->setTextColor(TQt::black);
  alert->setSelectedTextColor( TQColor("#FCC") );
  alert->setBold(true);
  alert->setBGColor( TQColor("#FCC") );
  list.append(alert);

  KateAttribute* functionAttribute = new KateAttribute();
  functionAttribute->setTextColor(TQt::darkBlue);
  functionAttribute->setSelectedTextColor(TQt::white);
  list.append(functionAttribute);

  KateAttribute* regionmarker = new KateAttribute();
  regionmarker->setTextColor(TQt::white);
  regionmarker->setBGColor(TQt::gray);
  regionmarker->setSelectedTextColor(TQt::gray);
  list.append(regionmarker);

  KateAttribute* error = new KateAttribute();
  error->setTextColor(TQt::red);
  error->setUnderline(true);
  error->setSelectedTextColor(TQt::red);
  list.append(error);

  TDEConfig *config = KateHlManager::self()->self()->getTDEConfig();
  config->setGroup("Default Item Styles - Schema " + KateFactory::self()->schemaManager()->name(schema));

  for (uint z = 0; z < defaultStyles(); z++)
  {
    KateAttribute *i = list.at(z);
    TQStringList s = config->readListEntry(defaultStyleName(z));
    if (!s.isEmpty())
    {
      while( s.count()<8)
        s << "";

      TQString tmp;
      TQRgb col;

      tmp=s[0]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); i->setTextColor(col); }

      tmp=s[1]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); i->setSelectedTextColor(col); }

      tmp=s[2]; if (!tmp.isEmpty()) i->setBold(tmp!="0");

      tmp=s[3]; if (!tmp.isEmpty()) i->setItalic(tmp!="0");

      tmp=s[4]; if (!tmp.isEmpty()) i->setStrikeOut(tmp!="0");

      tmp=s[5]; if (!tmp.isEmpty()) i->setUnderline(tmp!="0");

      tmp=s[6]; if (!tmp.isEmpty()) {
        if ( tmp != "-" )
        {
          col=tmp.toUInt(0,16);
          i->setBGColor(col);
        }
        else
          i->clearAttribute(KateAttribute::BGColor);
      }
      tmp=s[7]; if (!tmp.isEmpty()) {
        if ( tmp != "-" )
        {
          col=tmp.toUInt(0,16);
          i->setSelectedBGColor(col);
        }
        else
          i->clearAttribute(KateAttribute::SelectedBGColor);
      }
    }
  }
}

void KateHlManager::setDefaults(uint schema, KateAttributeList &list)
{
  TDEConfig *config =  KateHlManager::self()->self()->getTDEConfig();
  config->setGroup("Default Item Styles - Schema " + KateFactory::self()->schemaManager()->name(schema));

  for (uint z = 0; z < defaultStyles(); z++)
  {
    TQStringList settings;
    KateAttribute *i = list.at(z);

    settings<<(i->itemSet(KateAttribute::TextColor)?TQString::number(i->textColor().rgb(),16):"");
    settings<<(i->itemSet(KateAttribute::SelectedTextColor)?TQString::number(i->selectedTextColor().rgb(),16):"");
    settings<<(i->itemSet(KateAttribute::Weight)?(i->bold()?"1":"0"):"");
    settings<<(i->itemSet(KateAttribute::Italic)?(i->italic()?"1":"0"):"");
    settings<<(i->itemSet(KateAttribute::StrikeOut)?(i->strikeOut()?"1":"0"):"");
    settings<<(i->itemSet(KateAttribute::Underline)?(i->underline()?"1":"0"):"");
    settings<<(i->itemSet(KateAttribute::BGColor)?TQString::number(i->bgColor().rgb(),16):"-");
    settings<<(i->itemSet(KateAttribute::SelectedBGColor)?TQString::number(i->selectedBGColor().rgb(),16):"-");
    settings<<"---";

    config->writeEntry(defaultStyleName(z),settings);
  }

  emit changed();
}

int KateHlManager::highlights()
{
  return (int) hlList.count();
}

TQString KateHlManager::hlName(int n)
{
  return hlList.at(n)->name();
}

TQString KateHlManager::hlNameTranslated(int n)
{
  return hlList.at(n)->nameTranslated();
}

TQString KateHlManager::hlSection(int n)
{
  return hlList.at(n)->section();
}

bool KateHlManager::hlHidden(int n)
{
  return hlList.at(n)->hidden();
}

TQString KateHlManager::identifierForName(const TQString& name)
{
  KateHighlighting *hl = 0;

  if ((hl = hlDict[name]))
    return hl->getIdentifier ();

  return TQString();
}

bool KateHlManager::resetDynamicCtxs()
{
  if (forceNoDCReset)
    return false;

  if (lastCtxsReset.elapsed() < KATE_DYNAMIC_CONTEXTS_RESET_DELAY)
    return false;

  KateHighlighting *hl;
  for (hl = hlList.first(); hl; hl = hlList.next())
    hl->dropDynamicContexts();

  dynamicCtxsCount = 0;
  lastCtxsReset.start();

  return true;
}
//END

//BEGIN KateHighlightAction
void KateViewHighlightAction::init()
{
  m_doc = 0;
  subMenus.setAutoDelete( true );

  connect(popupMenu(),TQ_SIGNAL(aboutToShow()),this,TQ_SLOT(slotAboutToShow()));
}

void KateViewHighlightAction::updateMenu (Kate::Document *doc)
{
  m_doc = doc;
}

void KateViewHighlightAction::slotAboutToShow()
{
  Kate::Document *doc=m_doc;
  int count = KateHlManager::self()->highlights();

  for (int z=0; z<count; z++)
  {
    TQString hlName = KateHlManager::self()->hlNameTranslated (z);
    TQString hlSection = KateHlManager::self()->hlSection (z);

    if (!KateHlManager::self()->hlHidden(z))
    {
      if ( !hlSection.isEmpty() && (names.contains(hlName) < 1) )
      {
        if (subMenusName.contains(hlSection) < 1)
        {
          subMenusName << hlSection;
          TQPopupMenu *menu = new TQPopupMenu ();
          subMenus.append(menu);
          popupMenu()->insertItem ( '&' + hlSection, menu);
        }

        int m = subMenusName.findIndex (hlSection);
        names << hlName;
        subMenus.at(m)->insertItem ( '&' + hlName, this, TQ_SLOT(setHl(int)), 0,  z);
      }
      else if (names.contains(hlName) < 1)
      {
        names << hlName;
        popupMenu()->insertItem ( '&' + hlName, this, TQ_SLOT(setHl(int)), 0,  z);
      }
    }
  }

  if (!doc) return;

  for (uint i=0;i<subMenus.count();i++)
  {
    for (uint i2=0;i2<subMenus.at(i)->count();i2++)
    {
      subMenus.at(i)->setItemChecked(subMenus.at(i)->idAt(i2),false);
    }
  }
  popupMenu()->setItemChecked (0, false);

  int i = subMenusName.findIndex (KateHlManager::self()->hlSection(doc->hlMode()));
  if (i >= 0 && subMenus.at(i))
    subMenus.at(i)->setItemChecked (doc->hlMode(), true);
  else
    popupMenu()->setItemChecked (0, true);
}

void KateViewHighlightAction::setHl (int mode)
{
  Kate::Document *doc=m_doc;

  if (doc)
    doc->setHlMode((uint)mode);
}
//END KateViewHighlightAction
