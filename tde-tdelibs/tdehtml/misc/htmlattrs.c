/* ANSI-C code produced by gperf version 3.0.3 */
/* Command-line: gperf -c -a -L ANSI-C -P -G -D -E -C -o -t -k '*' -NfindAttr -Hhash_attr -Wwordlist_attr -Qspool_attr -s 2 htmlattrs.gperf  */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "htmlattrs.gperf"

/* This file is automatically generated from
#htmlattrs.in by makeattrs, do not edit */
#include "htmlattrs.h"
#line 6 "htmlattrs.gperf"
struct attrs {
    int name;
    int id;
};
enum
  {
    TOTAL_KEYWORDS = 152,
    MIN_WORD_LENGTH = 2,
    MAX_WORD_LENGTH = 15,
    MIN_HASH_VALUE = 3,
    MAX_HASH_VALUE = 576
  };

/* maximum key range = 574, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash_attr (const char *str, unsigned int len)
{
  static const unsigned short asso_values[] =
    {
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577,   5,   5, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577,  30,  10,   0,
       10,   5,  10,  15, 165,  40, 140,  80,   0,  25,
       70,   0,   5,   5,   0,   0,  10,  55,  70, 180,
        5,  90,  45, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577, 577, 577, 577,
      577, 577, 577, 577, 577, 577, 577
    };
  int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[14]];
      /*FALLTHROUGH*/
      case 14:
        hval += asso_values[(unsigned char)str[13]];
      /*FALLTHROUGH*/
      case 13:
        hval += asso_values[(unsigned char)str[12]];
      /*FALLTHROUGH*/
      case 12:
        hval += asso_values[(unsigned char)str[11]];
      /*FALLTHROUGH*/
      case 11:
        hval += asso_values[(unsigned char)str[10]];
      /*FALLTHROUGH*/
      case 10:
        hval += asso_values[(unsigned char)str[9]];
      /*FALLTHROUGH*/
      case 9:
        hval += asso_values[(unsigned char)str[8]];
      /*FALLTHROUGH*/
      case 8:
        hval += asso_values[(unsigned char)str[7]];
      /*FALLTHROUGH*/
      case 7:
        hval += asso_values[(unsigned char)str[6]];
      /*FALLTHROUGH*/
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      /*FALLTHROUGH*/
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      /*FALLTHROUGH*/
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      /*FALLTHROUGH*/
      case 2:
        hval += asso_values[(unsigned char)str[1]+1];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

struct spool_attr_t
  {
    char spool_attr_str0[sizeof("src")];
    char spool_attr_str1[sizeof("onscroll")];
    char spool_attr_str2[sizeof("cols")];
    char spool_attr_str3[sizeof("color")];
    char spool_attr_str4[sizeof("onerror")];
    char spool_attr_str5[sizeof("rel")];
    char spool_attr_str6[sizeof("loop")];
    char spool_attr_str7[sizeof("for")];
    char spool_attr_str8[sizeof("coords")];
    char spool_attr_str9[sizeof("top")];
    char spool_attr_str10[sizeof("code")];
    char spool_attr_str11[sizeof("scope")];
    char spool_attr_str12[sizeof("onreset")];
    char spool_attr_str13[sizeof("onselect")];
    char spool_attr_str14[sizeof("face")];
    char spool_attr_str15[sizeof("label")];
    char spool_attr_str16[sizeof("left")];
    char spool_attr_str17[sizeof("border")];
    char spool_attr_str18[sizeof("text")];
    char spool_attr_str19[sizeof("defer")];
    char spool_attr_str20[sizeof("bordercolor")];
    char spool_attr_str21[sizeof("abbr")];
    char spool_attr_str22[sizeof("pagex")];
    char spool_attr_str23[sizeof("onload")];
    char spool_attr_str24[sizeof("id")];
    char spool_attr_str25[sizeof("selected")];
    char spool_attr_str26[sizeof("prompt")];
    char spool_attr_str27[sizeof("target")];
    char spool_attr_str28[sizeof("onabort")];
    char spool_attr_str29[sizeof("class")];
    char spool_attr_str30[sizeof("declare")];
    char spool_attr_str31[sizeof("data")];
    char spool_attr_str32[sizeof("clear")];
    char spool_attr_str33[sizeof("accept")];
    char spool_attr_str34[sizeof("profile")];
    char spool_attr_str35[sizeof("alt")];
    char spool_attr_str36[sizeof("type")];
    char spool_attr_str37[sizeof("onblur")];
    char spool_attr_str38[sizeof("onfocus")];
    char spool_attr_str39[sizeof("codebase")];
    char spool_attr_str40[sizeof("char")];
    char spool_attr_str41[sizeof("frame")];
    char spool_attr_str42[sizeof("rules")];
    char spool_attr_str43[sizeof("compact")];
    char spool_attr_str44[sizeof("rev")];
    char spool_attr_str45[sizeof("shape")];
    char spool_attr_str46[sizeof("charset")];
    char spool_attr_str47[sizeof("charoff")];
    char spool_attr_str48[sizeof("lang")];
    char spool_attr_str49[sizeof("start")];
    char spool_attr_str50[sizeof("onresize")];
    char spool_attr_str51[sizeof("truespeed")];
    char spool_attr_str52[sizeof("frameborder")];
    char spool_attr_str53[sizeof("span")];
    char spool_attr_str54[sizeof("classid")];
    char spool_attr_str55[sizeof("longdesc")];
    char spool_attr_str56[sizeof("name")];
    char spool_attr_str57[sizeof("ismap")];
    char spool_attr_str58[sizeof("colspan")];
    char spool_attr_str59[sizeof("media")];
    char spool_attr_str60[sizeof("enctype")];
    char spool_attr_str61[sizeof("datetime")];
    char spool_attr_str62[sizeof("vspace")];
    char spool_attr_str63[sizeof("onclick")];
    char spool_attr_str64[sizeof("pagey")];
    char spool_attr_str65[sizeof("usemap")];
    char spool_attr_str66[sizeof("codetype")];
    char spool_attr_str67[sizeof("scrolling")];
    char spool_attr_str68[sizeof("value")];
    char spool_attr_str69[sizeof("checked")];
    char spool_attr_str70[sizeof("onsubmit")];
    char spool_attr_str71[sizeof("ondblclick")];
    char spool_attr_str72[sizeof("dir")];
    char spool_attr_str73[sizeof("onmouseup")];
    char spool_attr_str74[sizeof("style")];
    char spool_attr_str75[sizeof("scrolldelay")];
    char spool_attr_str76[sizeof("cite")];
    char spool_attr_str77[sizeof("onmouseout")];
    char spool_attr_str78[sizeof("object")];
    char spool_attr_str79[sizeof("multiple")];
    char spool_attr_str80[sizeof("axis")];
    char spool_attr_str81[sizeof("action")];
    char spool_attr_str82[sizeof("tabindex")];
    char spool_attr_str83[sizeof("title")];
    char spool_attr_str84[sizeof("onmouseover")];
    char spool_attr_str85[sizeof("autocomplete")];
    char spool_attr_str86[sizeof("onunload")];
    char spool_attr_str87[sizeof("challenge")];
    char spool_attr_str88[sizeof("plain")];
    char spool_attr_str89[sizeof("content")];
    char spool_attr_str90[sizeof("noresize")];
    char spool_attr_str91[sizeof("cellspacing")];
    char spool_attr_str92[sizeof("bgcolor")];
    char spool_attr_str93[sizeof("href")];
    char spool_attr_str94[sizeof("align")];
    char spool_attr_str95[sizeof("nosave")];
    char spool_attr_str96[sizeof("z-index")];
    char spool_attr_str97[sizeof("rows")];
    char spool_attr_str98[sizeof("oversrc")];
    char spool_attr_str99[sizeof("size")];
    char spool_attr_str100[sizeof("onkeypress")];
    char spool_attr_str101[sizeof("onmousemove")];
    char spool_attr_str102[sizeof("version")];
    char spool_attr_str103[sizeof("cellpadding")];
    char spool_attr_str104[sizeof("language")];
    char spool_attr_str105[sizeof("topmargin")];
    char spool_attr_str106[sizeof("valign")];
    char spool_attr_str107[sizeof("scrollamount")];
    char spool_attr_str108[sizeof("disabled")];
    char spool_attr_str109[sizeof("scheme")];
    char spool_attr_str110[sizeof("readonly")];
    char spool_attr_str111[sizeof("wrap")];
    char spool_attr_str112[sizeof("leftmargin")];
    char spool_attr_str113[sizeof("hspace")];
    char spool_attr_str114[sizeof("method")];
    char spool_attr_str115[sizeof("headers")];
    char spool_attr_str116[sizeof("accesskey")];
    char spool_attr_str117[sizeof("onkeyup")];
    char spool_attr_str118[sizeof("summary")];
    char spool_attr_str119[sizeof("html")];
    char spool_attr_str120[sizeof("alink")];
    char spool_attr_str121[sizeof("bgproperties")];
    char spool_attr_str122[sizeof("valuetype")];
    char spool_attr_str123[sizeof("background")];
    char spool_attr_str124[sizeof("nohref")];
    char spool_attr_str125[sizeof("standby")];
    char spool_attr_str126[sizeof("pluginurl")];
    char spool_attr_str127[sizeof("pluginpage")];
    char spool_attr_str128[sizeof("pluginspage")];
    char spool_attr_str129[sizeof("direction")];
    char spool_attr_str130[sizeof("accept-charset")];
    char spool_attr_str131[sizeof("vlink")];
    char spool_attr_str132[sizeof("noshade")];
    char spool_attr_str133[sizeof("onchange")];
    char spool_attr_str134[sizeof("link")];
    char spool_attr_str135[sizeof("contenteditable")];
    char spool_attr_str136[sizeof("nowrap")];
    char spool_attr_str137[sizeof("rowspan")];
    char spool_attr_str138[sizeof("hreflang")];
    char spool_attr_str139[sizeof("maxlength")];
    char spool_attr_str140[sizeof("archive")];
    char spool_attr_str141[sizeof("behavior")];
    char spool_attr_str142[sizeof("onmousedown")];
    char spool_attr_str143[sizeof("hidden")];
    char spool_attr_str144[sizeof("height")];
    char spool_attr_str145[sizeof("http-equiv")];
    char spool_attr_str146[sizeof("onkeydown")];
    char spool_attr_str147[sizeof("visibility")];
    char spool_attr_str148[sizeof("unknown")];
    char spool_attr_str149[sizeof("width")];
    char spool_attr_str150[sizeof("marginheight")];
    char spool_attr_str151[sizeof("marginwidth")];
  };
static const struct spool_attr_t spool_attr_contents =
  {
    "src",
    "onscroll",
    "cols",
    "color",
    "onerror",
    "rel",
    "loop",
    "for",
    "coords",
    "top",
    "code",
    "scope",
    "onreset",
    "onselect",
    "face",
    "label",
    "left",
    "border",
    "text",
    "defer",
    "bordercolor",
    "abbr",
    "pagex",
    "onload",
    "id",
    "selected",
    "prompt",
    "target",
    "onabort",
    "class",
    "declare",
    "data",
    "clear",
    "accept",
    "profile",
    "alt",
    "type",
    "onblur",
    "onfocus",
    "codebase",
    "char",
    "frame",
    "rules",
    "compact",
    "rev",
    "shape",
    "charset",
    "charoff",
    "lang",
    "start",
    "onresize",
    "truespeed",
    "frameborder",
    "span",
    "classid",
    "longdesc",
    "name",
    "ismap",
    "colspan",
    "media",
    "enctype",
    "datetime",
    "vspace",
    "onclick",
    "pagey",
    "usemap",
    "codetype",
    "scrolling",
    "value",
    "checked",
    "onsubmit",
    "ondblclick",
    "dir",
    "onmouseup",
    "style",
    "scrolldelay",
    "cite",
    "onmouseout",
    "object",
    "multiple",
    "axis",
    "action",
    "tabindex",
    "title",
    "onmouseover",
    "autocomplete",
    "onunload",
    "challenge",
    "plain",
    "content",
    "noresize",
    "cellspacing",
    "bgcolor",
    "href",
    "align",
    "nosave",
    "z-index",
    "rows",
    "oversrc",
    "size",
    "onkeypress",
    "onmousemove",
    "version",
    "cellpadding",
    "language",
    "topmargin",
    "valign",
    "scrollamount",
    "disabled",
    "scheme",
    "readonly",
    "wrap",
    "leftmargin",
    "hspace",
    "method",
    "headers",
    "accesskey",
    "onkeyup",
    "summary",
    "html",
    "alink",
    "bgproperties",
    "valuetype",
    "background",
    "nohref",
    "standby",
    "pluginurl",
    "pluginpage",
    "pluginspage",
    "direction",
    "accept-charset",
    "vlink",
    "noshade",
    "onchange",
    "link",
    "contenteditable",
    "nowrap",
    "rowspan",
    "hreflang",
    "maxlength",
    "archive",
    "behavior",
    "onmousedown",
    "hidden",
    "height",
    "http-equiv",
    "onkeydown",
    "visibility",
    "unknown",
    "width",
    "marginheight",
    "marginwidth"
  };
#define spool_attr ((const char *) &spool_attr_contents)

static const struct attrs wordlist_attr[] =
  {
#line 157 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str0, ATTR_SRC},
#line 151 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str1, ATTR_ONSCROLL},
#line 33 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str2, ATTR_COLS},
#line 32 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str3, ATTR_COLOR},
#line 72 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str4, ATTR_ONERROR},
#line 82 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str5, ATTR_REL},
#line 59 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str6, ATTR_LOOP},
#line 127 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str7, ATTR_FOR},
#line 37 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str8, ATTR_COORDS},
#line 100 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str9, ATTR_TOP},
#line 122 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str10, ATTR_CODE},
#line 87 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str11, ATTR_SCOPE},
#line 149 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str12, ATTR_ONRESET},
#line 150 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str13, ATTR_ONSELECT},
#line 44 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str14, ATTR_FACE},
#line 131 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str15, ATTR_LABEL},
#line 56 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str16, ATTR_LEFT},
#line 21 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str17, ATTR_BORDER},
#line 99 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str18, ATTR_TEXT},
#line 39 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str19, ATTR_DEFER},
#line 22 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str20, ATTR_BORDERCOLOR},
#line 114 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str21, ATTR_ABBR},
#line 75 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str22, ATTR_PAGEX},
#line 143 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str23, ATTR_ONLOAD},
#line 130 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str24, ATTR_ID},
#line 91 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str25, ATTR_SELECTED},
#line 155 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str26, ATTR_PROMPT},
#line 98 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str27, ATTR_TARGET},
#line 71 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str28, ATTR_ONABORT},
#line 120 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str29, ATTR_CLASS},
#line 38 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str30, ATTR_DECLARE},
#line 125 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str31, ATTR_DATA},
#line 30 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str32, ATTR_CLEAR},
#line 12 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str33, ATTR_ACCEPT},
#line 154 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str34, ATTR_PROFILE},
#line 116 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str35, ATTR_ALT},
#line 103 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str36, ATTR_TYPE},
#line 135 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str37, ATTR_ONBLUR},
#line 139 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str38, ATTR_ONFOCUS},
#line 123 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str39, ATTR_CODEBASE},
#line 25 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str40, ATTR_CHAR},
#line 45 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str41, ATTR_FRAME},
#line 86 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str42, ATTR_RULES},
#line 35 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str43, ATTR_COMPACT},
#line 83 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str44, ATTR_REV},
#line 92 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str45, ATTR_SHAPE},
#line 28 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str46, ATTR_CHARSET},
#line 27 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str47, ATTR_CHAROFF},
#line 54 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str48, ATTR_LANG},
#line 95 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str49, ATTR_START},
#line 73 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str50, ATTR_ONRESIZE},
#line 102 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str51, ATTR_TRUESPEED},
#line 46 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str52, ATTR_FRAMEBORDER},
#line 94 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str53, ATTR_SPAN},
#line 121 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str54, ATTR_CLASSID},
#line 132 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str55, ATTR_LONGDESC},
#line 133 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str56, ATTR_NAME},
#line 53 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str57, ATTR_ISMAP},
#line 34 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str58, ATTR_COLSPAN},
#line 63 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str59, ATTR_MEDIA},
#line 43 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str60, ATTR_ENCTYPE},
#line 126 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str61, ATTR_DATETIME},
#line 110 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str62, ATTR_VSPACE},
#line 137 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str63, ATTR_ONCLICK},
#line 76 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str64, ATTR_PAGEY},
#line 161 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str65, ATTR_USEMAP},
#line 31 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str66, ATTR_CODETYPE},
#line 90 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str67, ATTR_SCROLLING},
#line 162 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str68, ATTR_VALUE},
#line 29 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str69, ATTR_CHECKED},
#line 152 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str70, ATTR_ONSUBMIT},
#line 138 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str71, ATTR_ONDBLCLICK},
#line 40 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str72, ATTR_DIR},
#line 148 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str73, ATTR_ONMOUSEUP},
#line 96 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str74, ATTR_STYLE},
#line 89 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str75, ATTR_SCROLLDELAY},
#line 119 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str76, ATTR_CITE},
#line 146 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str77, ATTR_ONMOUSEOUT},
#line 134 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str78, ATTR_OBJECT},
#line 65 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str79, ATTR_MULTIPLE},
#line 17 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str80, ATTR_AXIS},
#line 115 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str81, ATTR_ACTION},
#line 97 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str82, ATTR_TABINDEX},
#line 160 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str83, ATTR_TITLE},
#line 147 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str84, ATTR_ONMOUSEOVER},
#line 16 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str85, ATTR_AUTOCOMPLETE},
#line 153 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str86, ATTR_ONUNLOAD},
#line 26 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str87, ATTR_CHALLENGE},
#line 77 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str88, ATTR_PLAIN},
#line 124 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str89, ATTR_CONTENT},
#line 67 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str90, ATTR_NORESIZE},
#line 24 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str91, ATTR_CELLSPACING},
#line 19 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str92, ATTR_BGCOLOR},
#line 129 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str93, ATTR_HREF},
#line 14 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str94, ATTR_ALIGN},
#line 68 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str95, ATTR_NOSAVE},
#line 113 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str96, ATTR_Z_INDEX},
#line 84 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str97, ATTR_ROWS},
#line 74 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str98, ATTR_OVERSRC},
#line 93 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str99, ATTR_SIZE},
#line 141 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str100, ATTR_ONKEYPRESS},
#line 145 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str101, ATTR_ONMOUSEMOVE},
#line 107 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str102, ATTR_VERSION},
#line 23 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str103, ATTR_CELLPADDING},
#line 55 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str104, ATTR_LANGUAGE},
#line 101 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str105, ATTR_TOPMARGIN},
#line 105 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str106, ATTR_VALIGN},
#line 88 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str107, ATTR_SCROLLAMOUNT},
#line 42 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str108, ATTR_DISABLED},
#line 156 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str109, ATTR_SCHEME},
#line 81 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str110, ATTR_READONLY},
#line 112 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str111, ATTR_WRAP},
#line 57 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str112, ATTR_LEFTMARGIN},
#line 50 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str113, ATTR_HSPACE},
#line 64 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str114, ATTR_METHOD},
#line 128 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str115, ATTR_HEADERS},
#line 13 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str116, ATTR_ACCESSKEY},
#line 142 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str117, ATTR_ONKEYUP},
#line 159 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str118, ATTR_SUMMARY},
#line 51 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str119, ATTR_HTML},
#line 15 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str120, ATTR_ALINK},
#line 20 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str121, ATTR_BGPROPERTIES},
#line 106 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str122, ATTR_VALUETYPE},
#line 118 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str123, ATTR_BACKGROUND},
#line 66 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str124, ATTR_NOHREF},
#line 158 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str125, ATTR_STANDBY},
#line 80 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str126, ATTR_PLUGINURL},
#line 78 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str127, ATTR_PLUGINPAGE},
#line 79 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str128, ATTR_PLUGINSPAGE},
#line 41 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str129, ATTR_DIRECTION},
#line 11 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str130, ATTR_ACCEPT_CHARSET},
#line 109 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str131, ATTR_VLINK},
#line 69 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str132, ATTR_NOSHADE},
#line 136 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str133, ATTR_ONCHANGE},
#line 58 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str134, ATTR_LINK},
#line 36 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str135, ATTR_CONTENTEDITABLE},
#line 70 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str136, ATTR_NOWRAP},
#line 85 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str137, ATTR_ROWSPAN},
#line 49 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str138, ATTR_HREFLANG},
#line 62 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str139, ATTR_MAXLENGTH},
#line 117 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str140, ATTR_ARCHIVE},
#line 18 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str141, ATTR_BEHAVIOR},
#line 144 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str142, ATTR_ONMOUSEDOWN},
#line 48 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str143, ATTR_HIDDEN},
#line 47 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str144, ATTR_HEIGHT},
#line 52 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str145, ATTR_HTTP_EQUIV},
#line 140 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str146, ATTR_ONKEYDOWN},
#line 108 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str147, ATTR_VISIBILITY},
#line 104 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str148, ATTR_UNKNOWN},
#line 111 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str149, ATTR_WIDTH},
#line 60 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str150, ATTR_MARGINHEIGHT},
#line 61 "htmlattrs.gperf"
    {(int)(long)&((struct spool_attr_t *)0)->spool_attr_str151, ATTR_MARGINWIDTH}
  };

static const short lookup[] =
  {
     -1,  -1,  -1,   0,  -1,  -1,  -1,  -1,   1,   2,
      3,  -1,   4,   5,   6,  -1,  -1,  -1,   7,  -1,
     -1,   8,  -1,   9,  10,  11,  -1,  12,  13,  14,
     15,  -1,  -1,  -1,  16,  -1,  17,  -1,  -1,  18,
     19,  20,  -1,  -1,  21,  22,  23,  24,  25,  -1,
     -1,  26,  -1,  -1,  -1,  -1,  27,  28,  -1,  -1,
     29,  -1,  30,  -1,  31,  32,  33,  34,  35,  36,
     -1,  37,  38,  39,  40,  41,  -1,  -1,  -1,  -1,
     42,  -1,  43,  44,  -1,  45,  -1,  -1,  -1,  -1,
     -1,  -1,  46,  -1,  -1,  -1,  -1,  47,  -1,  48,
     49,  -1,  -1,  50,  51,  -1,  52,  -1,  -1,  53,
     -1,  -1,  54,  55,  56,  57,  -1,  58,  -1,  -1,
     59,  -1,  60,  61,  -1,  -1,  62,  63,  -1,  -1,
     64,  -1,  -1,  -1,  -1,  -1,  65,  -1,  66,  -1,
     -1,  -1,  -1,  -1,  67,  68,  -1,  69,  70,  -1,
     71,  -1,  -1,  72,  73,  74,  75,  -1,  -1,  76,
     77,  78,  -1,  79,  80,  -1,  81,  -1,  82,  -1,
     83,  84,  85,  86,  87,  88,  -1,  89,  90,  -1,
     -1,  91,  92,  -1,  93,  94,  95,  96,  -1,  97,
     -1,  -1,  98,  -1,  99, 100, 101, 102,  -1,  -1,
     -1, 103,  -1,  -1,  -1,  -1,  -1,  -1, 104, 105,
     -1, 106, 107, 108,  -1,  -1, 109,  -1, 110, 111,
    112, 113,  -1,  -1,  -1,  -1, 114, 115,  -1, 116,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1, 117,  -1,  -1,  -1,  -1, 118,  -1, 119,
    120,  -1,  -1,  -1,  -1,  -1,  -1, 121,  -1, 122,
    123, 124,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1, 125,  -1, 126, 127, 128,  -1,  -1,  -1,
     -1,  -1,  -1,  -1, 129,  -1,  -1,  -1,  -1, 130,
    131,  -1, 132, 133, 134, 135, 136, 137,  -1,  -1,
     -1,  -1,  -1, 138,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1, 139,  -1,  -1, 140,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1, 141,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1, 142,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1, 143,  -1,  -1,  -1,
     -1, 144,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1, 145,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1, 146,  -1,  -1,  -1,  -1,  -1,
    147,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1, 148,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    149,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1, 150,  -1,  -1,  -1, 151
  };

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
const struct attrs *
findAttr (const char *str, unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      int key = hash_attr (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          int index = lookup[key];

          if (index >= 0)
            {
              const char *s = wordlist_attr[index].name + spool_attr;

              if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
                return &wordlist_attr[index];
            }
        }
    }
  return 0;
}
#line 163 "htmlattrs.gperf"



static const unsigned short attrList[] = {
    65535,
    145,
    33,
    116,
    94,
    120,
    85,
    80,
    141,
    92,
    121,
    17,
    20,
    103,
    91,
    40,
    87,
    47,
    46,
    69,
    32,
    66,
    3,
    2,
    58,
    43,
    135,
    8,
    30,
    19,
    72,
    129,
    108,
    60,
    14,
    41,
    52,
    144,
    143,
    138,
    113,
    119,
    145,
    57,
    48,
    104,
    16,
    112,
    134,
    6,
    150,
    151,
    139,
    59,
    114,
    79,
    124,
    90,
    95,
    132,
    136,
    28,
    4,
    50,
    98,
    22,
    64,
    88,
    127,
    128,
    126,
    110,
    5,
    44,
    97,
    137,
    42,
    11,
    107,
    75,
    67,
    25,
    45,
    99,
    53,
    49,
    74,
    82,
    27,
    18,
    9,
    105,
    51,
    36,
    148,
    106,
    122,
    102,
    147,
    131,
    62,
    149,
    111,
    145,
    21,
    81,
    35,
    140,
    123,
    76,
    29,
    54,
    10,
    39,
    89,
    31,
    61,
    7,
    115,
    93,
    24,
    15,
    55,
    56,
    78,
    37,
    133,
    63,
    71,
    38,
    146,
    100,
    117,
    23,
    142,
    101,
    77,
    84,
    73,
    12,
    13,
    1,
    70,
    86,
    34,
    26,
    109,
    0,
    125,
    118,
    83,
    65,
    68,
    65535
};

const char* TDE_NO_EXPORT getAttrName(unsigned short id)
{
    if (!id || id > TOTAL_KEYWORDS) return "";
    return spool_attr + wordlist_attr[attrList[id]].name;
}
