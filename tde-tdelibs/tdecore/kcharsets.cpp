/* This file is part of the KDE libraries
    Copyright (C) 1999 Lars Knoll (knoll@kde.org)

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
#include "kcharsets.h"

#include "kqiodevicegzip_p.h"
#include "kentities.c"

#include <tdeapplication.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <tdeconfig.h>

#include <tqfontinfo.h>
#include <tqstrlist.h>
#include <tqfontdatabase.h>
#include <kdebug.h>

#include <tqtextcodec.h>
#include <tqmap.h>
#include <tqcstring.h>
#include <tqdir.h>
#include <tqregexp.h>

#include <assert.h>

static const char * const language_names[] = {
	I18N_NOOP( "Other" ),
	I18N_NOOP( "Arabic" ),
	I18N_NOOP( "Baltic" ),
	I18N_NOOP( "Central European" ),
	I18N_NOOP( "Chinese Simplified" ),
	I18N_NOOP( "Chinese Traditional" ),
	I18N_NOOP( "Cyrillic" ),
	I18N_NOOP( "Greek" ),
	I18N_NOOP( "Hebrew" ),
	I18N_NOOP( "Japanese" ),
	I18N_NOOP( "Korean" ),
	I18N_NOOP( "Thai" ),
	I18N_NOOP( "Turkish" ),
	I18N_NOOP( "Western European" ),
	I18N_NOOP( "Tamil" ),
	I18N_NOOP( "Unicode" ),
	I18N_NOOP( "Northern Saami" ),
        I18N_NOOP( "Vietnamese" ),
        I18N_NOOP( "South-Eastern Europe" )
};

// This list gives the charsets that can be used to display a file given in a certain encoding.
// The list should be in order of preference
static const char* const charsets_for_encoding[] = {
    "koi8-r",
    "koi8-u",
    "iso 8859-1",
    "iso 8859-2",
    "iso 8859-3",
    "iso 8859-4",
    "iso 8859-5",
    "iso 8859-6",
    "iso 8859-7",
    "iso 8859-8",
    "iso 8859-8-i",
    "iso 8859-9",
    "iso 8859-11",
    "iso 8859-13",
    "iso 8859-14",
    "iso 8859-15",
    "iso 8859-16",
    "utf8",
    "utf16",
    "iso-10646-ucs-2",
    "cp 1250",
    "cp 1251",
    "cp 1252",
    "cp 1253",
    "cp 1254",
    "cp 1255",
    "cp 1256",
    "cp 1257",
    "cp 1258",
    "ibm850",
    "ibm852",
    "ibm866",
    "tis620",
    "eucjp",
    "sjis",
    "jis7",
    "big5",
    "big5-hkscs",
    "gbk",
    "gb18030",
    "gb2312",
    "euckr",
    "tscii",
//    "pt 154",
    "winsami2",
    "cp 874",
    0 }; // extra 0 for end

// 0 other
// 1 Arabic
// 2 Baltic
// 3 Central European
// 4 Chinese Simplified
// 5 Chinese Traditional
// 6 Cyrillic
// 7 Greek
// 8 Hebrew
// 9 Japanese
// 10 Korean
// 11 Thai
// 12 Turkish
// 13 Western European
// 14 Tamil
// 15 Unicode
// 16 Northern Sami
// 17 Vietnamese
// 18 South-Eastern Europe
// ### FIXME KDE4: the name of the encodings should mostly be uppercase
static struct LanguageForEncoding
    {
    const char* index;
    int data;
    } const language_for_encoding[] = {
    { "iso 8859-1", 13 },
    { "iso 8859-15", 13 },
    { "iso 8859-14", 13 },
    { "cp 1252", 13 },
    { "ibm850", 13 },
    { "iso 8859-2", 3 },
    { "iso 8859-3", 3 },
    { "iso 8859-4", 2 },
    { "iso 8859-13", 2 },
    { "iso 8859-16", 18 },
    { "cp 1250", 3 },
    { "cp 1254", 12 },
    { "cp 1257", 2 },
    { "ibm852", 3 },
    { "koi8-r", 6 },
    { "iso 8859-5", 6 },
    { "cp 1251", 6 },
    { "koi8-u", 6 },
//    { "pt 154", 6 },
    { "ibm866", 6 },
    { "big5", 5 },
    { "big5-hkscs", 5 },
    { "gb18030", 4 },
    { "gbk", 4 },
    { "gb2312", 4 },
    { "euckr", 10 },
    { "sjis", 9 },
    { "jis7", 9 },
    { "eucjp", 9 },
    { "iso 8859-7", 7 },
    { "cp 1253", 7 },
    { "iso 8859-6", 1 },
    { "cp 1256", 1 },
    { "iso 8859-8", 8 },
    { "iso 8859-8-i", 8 },
    { "cp 1255", 8 },
    { "iso 8859-9", 12 },
    { "tis620", 11 },
    { "iso 8859-11", 11 },
    { "cp 874", 11 },
    { "cp 1258", 17 },
    { "tscii", 14 },
    { "utf8", 15 },
    { "utf16", 15 },
    { "utf7", 15 }, // ### FIXME: UTF-7 is not in Qt
    { "ucs2", 15 },
    { "iso-10646-ucs-2", 15 },
    { "winsami2", 16},
    { 0, 0 } };

// defines some different names for codecs that are built into Qt.
static struct Builtin
    {
    const char* index;
    const char* data;
    } const builtin[] = {
    { "iso-ir-111", "koi8-r" },
    { "koi8-ru", "koi8-u" }, // ### Qt 3.3 maps it to koi8-r
    { "koi unified", "koi8-r" }, // ### FIXME: Qt 3.3 seems to map this to EUC-KR, so this mapping is too late :-(
    // Using ISO-8859-1 for ASCII is an approximation at write
    { "us-ascii", "iso 8859-1" },
    { "usascii", "iso 8859-1" },
    { "ascii", "iso 8859-1" },
    { "x-utf-8", "utf-8" },
    { "x-utf-7", "utf-7" }, // ### FIXME: UTF-7 is not in Qt 
    { "unicode-1-1-utf-7", "utf-7" }, // ### FIXME: UTF-7 is not in Qt
    { "utf-16", "iso-10646-ucs-2" },
    { "utf16", "iso-10646-ucs-2" },
    { "ucs2", "iso-10646-ucs-2" },
    { "iso10646-1", "iso-10646-ucs-2" },
    { "gb18030.2000-1", "gb18030" },
    { "gb18030.2000-0", "gb18030" },
    { "gbk-0", "gbk" },
    { "gb2312.1980-0", "gbk" },
    { "gb_2312-80", "gbk" },/* this one is not official, but MS is using it :/ */
    { "x-euc-kr", "euckr" },
    { "jisx0201.1976-0", "eucjp" },
    { "jisx0208.1983-0", "eucjp" },
    { "jisx0208.1990-0", "eucjp" },
    { "jisx0208.1997-0", "eucjp" },
    { "jisx0212.1990-0", "eucjp" },
    { "jisx0213.2000-1", "eucjp" },
    { "jisx0213.2000-2", "eucjp" },
    { "windows850", "ibm850" },
    { "windows866", "ibm866" },
    { "windows1251", "cp 1251" },
    { "windows1252", "cp 1252" },
    { "windows1253", "cp 1253" },
    { "windows1254", "cp 1254" },
    { "windows1255", "cp 1255" },
    { "windows1256", "cp 1256" },
    { "windows1257", "cp 1257" },
    { "windows1258", "cp 1258" },
    { "windows-850", "ibm850" },
    { "windows-866", "ibm866" },
    { "x-windows-850", "ibm850" },
    { "x-windows-866", "ibm866" },
    { "x-windows-1250", "cp 1250" },
    { "x-windows-1251", "cp 1251" },
    { "x-windows-1252", "cp 1252" },
    { "x-windows-1253", "cp 1253" },
    { "x-windows-1254", "cp 1254" },
    { "x-windows-1255", "cp 1255" },
    { "x-windows-1256", "cp 1256" },
    { "x-windows-1257", "cp 1257" },
    { "x-windows-1258", "cp 1258" },
    { "cp819", "iso 8859-1" },
    { "cp850", "ibm850" },
    { "cp866", "ibm866" },
    { "cp-819", "iso 8859-1" },
    { "cp-850", "ibm850" },
    { "cp-866", "ibm866" },
    { "cp-1250", "cp 1250" },
    { "cp-1251", "cp 1251" },
    { "cp-1252", "cp 1252" },
    { "cp-1253", "cp 1253" },
    { "cp-1254", "cp 1254" },
    { "cp-1255", "cp 1255" },
    { "cp-1256", "cp 1256" },
    { "cp-1257", "cp 1257" },
    { "cp-1258", "cp 1258" },
    { "cp-10000", "apple roman" },
    { "x-cp-850", "ibm850" },
    { "x-cp-866", "ibm866" },
    { "x-cp-1250", "cp 1250" },
    { "x-cp-1251", "cp 1251" },
    { "x-cp-1252", "cp 1252" },
    { "x-cp-1253", "cp 1253" },
    { "x-cp-1254", "cp 1254" },
    { "x-cp-1255", "cp 1255" },
    { "x-cp-1256", "cp 1256" },
    { "x-cp-1257", "cp 1257" },
    { "x-cp-1258", "cp 1258" },
    { "x-cp-10000", "apple roman" },
    { "ibm819", "iso 8859-1" },
    { "thai-tis620", "iso 8859-11" },
    { "windows-874", "cp 874" },
    { "windows874", "cp 874" },
    { "x-windows-874", "cp 874" },
    { "x-cp-874", "cp 874" },
    { "ibm 874", "cp 874" },
    { "ibm874", "cp 874" }, // Qt4 name
    { "x-ibm874", "cp 874" },
    { "ksc5601.1987-0", "euckr" },
    { "x-winsami2", "winsami2" },
    { "x-mac-roman", "apple roman" },
    { "macintosh", "apple roman" },
    { "mac", "apple roman" },
    { "csiso2022jp", "jis7" }, // See bug #77243
    { "big5-eten", "big5-hkscs" },
    { "cp950", "big5-hkscs" },
    { 0, 0 }};

// some different names for the encodings defined in the charmaps files.
// even though the charmap file names are all uppercase, the names are all lowercase here.
static struct Aliases
    {
    const char* index;
    const char* data;
    } const aliases[] = {
    { "cp852", "ibm852" },
    { "cp-852", "ibm852" },
    { "x-cp-852", "ibm852" },
    { "windows852", "ibm852" },
    { "windows-852", "ibm852" },
    { "x-windows-852", "ibm852" },
    { 0, 0 }};

// some last resort hints in case the charmap file couldn't be found. This gives at least a partial conversion
// and helps making things readable.
// the name used as input here is already converted to the more canonical name as defined in the aliases array.
static struct ConversionHints
    {
    const char* index;
    const char* data;
    } const conversion_hints[] = {
    { "cp1250", "iso-8859-2" },
    { "koi8-r", "iso-8859-5" },
    { "koi8-u", "koi8-r" },
    // KDE had always "CP 1251" as best fallback to PT 154. Now that Qt does not offer this encoding anymore, it is our fallback.
    { "pt 154", "cp 1251" },
    { "paratype-154", "cp 1251" },
    { "pt-154", "cp 1251" },
    { 0, 0 }};


// search an array of items index/data, index is const char*, data is T, find first matching index
// and return data, or return 0
template< typename T, typename Data >
static Data kcharsets_array_search( const T* start, const char* entry )
{
    for( const T* pos = start;
         pos->index != 0;
         ++pos )
        if( qstrcmp( pos->index, entry ) == 0 )
            return pos->data;
    return 0;
}


class KCharsetsPrivate
{
public:
    KCharsetsPrivate(KCharsets* _kc)
        : codecForNameDict(43, false) // case insensitive
    {
        db = 0;
        kc = _kc;
    }
    ~KCharsetsPrivate()
    {
        delete db;
    }
    TQFontDatabase *db;
    TQAsciiDict<TQTextCodec> codecForNameDict;
    KCharsets* kc;
};

// --------------------------------------------------------------------------

KCharsets::KCharsets()
{
    d = new KCharsetsPrivate(this);
}

KCharsets::~KCharsets()
{
    delete d;
}

TQChar KCharsets::fromEntity(const TQString &str)
{
    TQChar res = TQChar::null;

    int pos = 0;
    if(str[pos] == (TQChar)'&') pos++;

    // Check for '&#000' or '&#x0000' sequence
    if (str[pos] == (TQChar)'#' && str.length()-pos > 1) {
        bool ok;
        pos++;
        if (str[pos] == (TQChar)'x' || str[pos] == (TQChar)'X') {
            pos++;
            // '&#x0000', hexadeciaml character reference
            TQString tmp(str.unicode()+pos, str.length()-pos);
            res = tmp.toInt(&ok, 16);
        } else {
            //  '&#0000', decimal character reference
            TQString tmp(str.unicode()+pos, str.length()-pos);
            res = tmp.toInt(&ok, 10);
        }
        return res;
    }

    const entity *e = kde_findEntity(str.ascii(), str.length());

    if(!e)
    {
        //kdDebug( 0 ) << "unknown entity " << str <<", len = " << str.length() << endl;
        return TQChar::null;
    }
    //kdDebug() << "got entity " << str << " = " << e->code << endl;

    return TQChar(e->code);
}

TQChar KCharsets::fromEntity(const TQString &str, int &len)
{
    // entities are never longer than 8 chars... we start from
    // that length and work backwards...
    len = 8;
    while(len > 0)
    {
        TQString tmp = str.left(len);
        TQChar res = fromEntity(tmp);
        if( res != (TQChar)TQChar::null ) return res;
        len--;
    }
    return TQChar::null;
}


TQString KCharsets::toEntity(const TQChar &ch)
{
    TQString ent;
    ent.sprintf("&#0x%x;", ch.unicode());
    return ent;
}

TQString KCharsets::resolveEntities( const TQString &input )
{
    TQString text = input;
    const TQChar *p = text.unicode();
    const TQChar *end = p + text.length();
    const TQChar *ampersand = 0;
    bool scanForSemicolon = false;

    for ( ; p < end; ++p ) {
        const TQChar ch = *p;

        if ( ch == (TQChar)'&' ) {
            ampersand = p;
            scanForSemicolon = true;
            continue;
        }

        if ( ch != (TQChar)';' || scanForSemicolon == false )
            continue;

        assert( ampersand );

        scanForSemicolon = false;

        const TQChar *entityBegin = ampersand + 1;

        const uint entityLength = p - entityBegin;
        if ( entityLength == 0 )
            continue;

        const TQChar entityValue = KCharsets::fromEntity( TQConstString( entityBegin, entityLength ).string() );
        if ( entityValue.isNull() )
            continue;

        const uint ampersandPos = ampersand - text.unicode();

        text[ (int)ampersandPos ] = entityValue;
        text.remove( ampersandPos + 1, entityLength + 1 );
        p = text.unicode() + ampersandPos;
        end = text.unicode() + text.length();
        ampersand = 0;
    }

    return text;
}

TQStringList KCharsets::availableEncodingNames()
{
    TQStringList available;
    for ( const char* const* pos = charsets_for_encoding; *pos; ++pos ) {
        //kdDebug(0) << *charsets << " available" << endl;
        available.append( TQString::fromLatin1( *pos ));
    }
    return available;
}

TQString KCharsets::languageForEncoding( const TQString &encoding )
{
    int lang = kcharsets_array_search< LanguageForEncoding, int >
        ( language_for_encoding, encoding.latin1());
    return i18n( language_names[lang] );
}

TQString KCharsets::encodingForName( const TQString &descriptiveName )
{
    const int left = descriptiveName.findRev( '(' );
    
    if (left<0) // No parenthesis, so assume it is a normal encoding name
	return descriptiveName.stripWhiteSpace();
    
    TQString name(descriptiveName.mid(left+1));
    
    const int right = name.findRev( ')' );
    
    if (right<0) 
        return name;

    return name.left(right).stripWhiteSpace();
}

TQString KCharsets::descriptiveNameForEncoding( const TQString &encodingName )
{
    for ( const LanguageForEncoding* pos = language_for_encoding; pos->index; ++pos ) {
        if ( encodingName == TQString::fromLatin1( pos->index )) {
            const TQString description = i18n( language_names[ pos->data ] );
            return i18n("Descriptive Encoding Name", "%1 ( %2 )"). arg ( description ). arg( encodingName );
        }
    }
    return TQString::null;
}

TQStringList KCharsets::descriptiveEncodingNames()
{
    // As we are sorting, we can directly read the array language_for_encoding
    TQStringList encodings;
    for ( const LanguageForEncoding* pos = language_for_encoding; pos->index; ++pos ) {
        const TQString name = TQString::fromLatin1( pos->index );
        const TQString description = i18n( language_names[ pos->data ] );
        encodings.append( i18n("Descriptive Encoding Name", "%1 ( %2 )"). arg ( description ). arg( name ) );
    }
    encodings.sort();
    return encodings;
}

TQTextCodec *KCharsets::codecForName(const TQString &n) const
{
    bool b;
    return codecForName( n, b );
}

TQTextCodec *KCharsets::codecForName(const TQString &n, bool &ok) const
{
    ok = true;

    TQTextCodec* codec = 0;
    // dict lookup is case insensitive anyway
    if((codec = d->codecForNameDict[n.isEmpty() ? "->locale<-" : n.latin1()]))
        return codec; // cache hit, return

    if (n.isEmpty()) {
        codec = TDEGlobal::locale()->codecForEncoding();
        d->codecForNameDict.replace("->locale<-", codec);
        return codec;
    }

    TQCString name = n.lower().latin1();
    TQCString key = name;
    if (name.right(8) == "_charset")
       name.truncate(name.length()-8);

    if (name.isEmpty()) {
      ok = false;
      return TQTextCodec::codecForName("iso8859-1");
    }

    codec = TQTextCodec::codecForName(name);

    if(codec) {
        d->codecForNameDict.replace(key, codec);
        return codec;
    }

    // these codecs are built into Qt, but the name given for the codec is different,
    // so TQTextCodec did not recognize it.
    TQCString cname = kcharsets_array_search< Builtin, const char* >( builtin, name.data());

    if(!cname.isEmpty())
        codec = TQTextCodec::codecForName(cname);

    if(codec)
    {
        d->codecForNameDict.replace(key, codec);
        return codec;
    }

    TQString dir;
    {
    TDEConfigGroupSaver cfgsav( TDEGlobal::config(), "i18n" );
    dir = TDEGlobal::config()->readPathEntry("i18ndir", TQString::fromLatin1("/usr/share/i18n/charmaps"));
    }

    // these are codecs not included in Qt. They can be build up if the corresponding charmap
    // is available in the charmap directory.
    cname = kcharsets_array_search< Aliases, const char* >( aliases, name.data());

    if(cname.isEmpty())
        cname = name;
    cname = cname.upper();

    const TQString basicName = TQString::fromLatin1(cname);
    kdDebug() << k_funcinfo << endl << " Trying to find " << cname << " in " << dir << endl;
    
    TQString charMapFileName;
    bool gzipped = false; 
    TQDir qdir(dir);
    if (!qdir.exists()) {
        // The directory for the charmaps does not even exist... (That is common!)
    }
    else if (qdir.exists(basicName, false)) {
        charMapFileName = basicName;
    }
    else if (qdir.exists(basicName+".gz", false)) {
        charMapFileName = basicName + ".gz";
        gzipped = true;
    }
    else {
        // Check if we are asking a code page
        // If yes, then check "CP99999" and "IBM99999"
        // First we need to find the number of the codepage
        TQRegExp regexp("^(X-)?(CP|IBM)(-| )?(0-9)+");
        if ( regexp.search(basicName) != -1) {
            const TQString num = regexp.cap(4);
            if (num.isEmpty()) {
                // No number, not a code page (or something went wrong)
            }
            else if (qdir.exists("IBM"+num)) {
                charMapFileName = "IBM"+num;
            }
            else if (qdir.exists("IBM"+num+".gz")) {
                charMapFileName = "IBM"+num+".gz";
                gzipped = true;
            }
            else if (qdir.exists("CP"+num)) {
                charMapFileName = "CP"+num;
            }
            else if (qdir.exists("CP"+num+".gz")) {
                charMapFileName = "CP"+num+".gz";
                gzipped = true;
            }
        }
    }
    
    if (gzipped && !charMapFileName.isEmpty()) {
        KQIODeviceGZip gzip(dir + "/" + charMapFileName);
        if (gzip.open(IO_ReadOnly)) {
            kdDebug() << "Loading gzipped charset..." << endl;
            codec = TQTextCodec::loadCharmap(&gzip);
            gzip.close();
        }
        else
            kdWarning() << "Could not open gzipped charset!" << endl;
    }
    else if (!charMapFileName.isEmpty()) {
        codec = TQTextCodec::loadCharmapFile(dir + "/" + charMapFileName);
    }

    if(codec) {
        d->codecForNameDict.replace(key, codec);
        return codec;
    }

    // this also failed, the last resort is now to take some compatibility charmap

    cname = kcharsets_array_search< ConversionHints, const char* >( conversion_hints, (const char*)name.data() );

    if(!cname.isEmpty())
        codec = TQTextCodec::codecForName(cname);

    if(codec) {
        d->codecForNameDict.replace(key, codec);
        return codec;
    }

    // could not assign a codec, let's return Latin1
    ok = false;
    return TQTextCodec::codecForName("iso8859-1");
}
