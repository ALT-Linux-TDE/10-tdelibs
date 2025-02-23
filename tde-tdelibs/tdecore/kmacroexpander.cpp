/*
    This file is part of the KDE libraries

    Copyright (c) 2002-2003 Oswald Buddenhagen <ossi@kde.org>
    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

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

#include <kmacroexpander.h>

#include <tqvaluestack.h>
#include <tqregexp.h>

KMacroExpanderBase::KMacroExpanderBase( TQChar c )
{
    escapechar = c;
}

KMacroExpanderBase::~KMacroExpanderBase()
{
}

void
KMacroExpanderBase::setEscapeChar( TQChar c )
{
    escapechar = c;
}

TQChar
KMacroExpanderBase::escapeChar() const
{
    return escapechar;
}

void KMacroExpanderBase::expandMacros( TQString &str )
{
    uint pos;
    int len;
    TQChar ec( escapechar );
    TQStringList rst;
    TQString rsts;

    for (pos = 0; pos < str.length(); ) {
        if (ec != (TQChar)0) {
            if (str.unicode()[pos] != ec)
                goto nohit;
            if (!(len = expandEscapedMacro( str, pos, rst )))
                goto nohit;
        } else {
            if (!(len = expandPlainMacro( str, pos, rst )))
                goto nohit;
        }
            if (len < 0) {
                pos -= len;
                continue;
            }
            rsts = rst.join( " " );
            rst.clear();
            str.replace( pos, len, rsts );
            pos += rsts.length();
            continue;
      nohit:
        pos++;
    }
}


namespace KMacroExpander {

    /** @intern Quoting state of the expander code. Not available publicly. */
    enum Quoting { noquote, singlequote, doublequote, dollarquote, 
                   paren, subst, group, math };
    typedef struct {
        Quoting current;
        bool dquote;
    } State;
    typedef struct {
        TQString str;
        uint pos;
    } Save;

}

using namespace KMacroExpander;

bool KMacroExpanderBase::expandMacrosShellQuote( TQString &str, uint &pos )
{
    int len;
    uint pos2;
    TQChar ec( escapechar );
    State state = { noquote, false };
    TQValueStack<State> sstack;
    TQValueStack<Save> ostack;
    TQStringList rst;
    TQString rsts;

    while (pos < str.length()) {
        TQChar cc( str.unicode()[pos] );
        if (ec != (TQChar)0) {
            if (cc != ec)
                goto nohit;
            if (!(len = expandEscapedMacro( str, pos, rst )))
                goto nohit;
        } else {
            if (!(len = expandPlainMacro( str, pos, rst )))
                goto nohit;
        }
            if (len < 0) {
                pos -= len;
                continue;
            }
            if (state.dquote) {
                rsts = rst.join( " " );
                rsts.replace( TQRegExp("([$`\"\\\\])"), "\\\\1" );
            } else if (state.current == dollarquote) {
                rsts = rst.join( " " );
                rsts.replace( TQRegExp("(['\\\\])"), "\\\\1" );
            } else if (state.current == singlequote) {
                rsts = rst.join( " " );
                rsts.replace( '\'', "'\\''");
            } else {
                if (rst.isEmpty()) {
                    str.remove( pos, len );
                    continue;
                } else {
                    rsts = "'";
#if 0 // this could pay off if join() would be cleverer and the strings were long
                    for (TQStringList::Iterator it = rst.begin(); it != rst.end(); ++it)
                        (*it).replace( '\'', "'\\''" );
                    rsts += rst.join( "' '" );
#else
                    for (TQStringList::ConstIterator it = rst.begin(); it != rst.end(); ++it) {
                        if (it != rst.begin())
                            rsts += "' '";
                        TQString trsts( *it );
                        trsts.replace( '\'', "'\\''" );
                        rsts += trsts;
                    }
#endif
                    rsts += "'";
                }
            }
            rst.clear();
            str.replace( pos, len, rsts );
            pos += rsts.length();
            continue;
      nohit:
        if (state.current == singlequote) {
            if (cc == (TQChar)'\'')
                state = sstack.pop();
        } else if (cc == (TQChar)'\\') {
            // always swallow the char -> prevent anomalies due to expansion
            pos += 2;
            continue;
        } else if (state.current == dollarquote) {
            if (cc == (TQChar)'\'')
                state = sstack.pop();
        } else if (cc == (TQChar)'$') {
            cc = str[++pos];
            if (cc == (TQChar)'(') {
                sstack.push( state );
                if (str[pos + 1] == (TQChar)'(') {
                    Save sav = { str, pos + 2 };
                    ostack.push( sav );
                    state.current = math;
                    pos += 2;
                    continue;
                } else {
                    state.current = paren;
                    state.dquote = false;
                }
            } else if (cc == (TQChar)'{') {
                sstack.push( state );
                state.current = subst;
            } else if (!state.dquote) {
                if (cc == (TQChar)'\'') {
                    sstack.push( state );
                    state.current = dollarquote;
                } else if (cc == (TQChar)'"') {
                    sstack.push( state );
                    state.current = doublequote;
                    state.dquote = true;
                }
            }
            // always swallow the char -> prevent anomalies due to expansion
        } else if (cc == (TQChar)'`') {
            str.replace( pos, 1, "$( " ); // add space -> avoid creating $((
            pos2 = pos += 3;
            for (;;) {
                if (pos2 >= str.length()) {
                    pos = pos2;
                    return false;
                }
                cc = str.unicode()[pos2];
                if (cc == (TQChar)'`')
                    break;
                if (cc == (TQChar)'\\') {
                    cc = str[++pos2];
                    if (cc == (TQChar)'$' || cc == (TQChar)'`' || cc == (TQChar)'\\' ||
                        (cc == (TQChar)'"' && state.dquote))
                    {
                        str.remove( pos2 - 1, 1 );
                        continue;
                    }
                }
                pos2++;
            }
            str[pos2] = ')';
            sstack.push( state );
            state.current = paren;
            state.dquote = false;
            continue;
        } else if (state.current == doublequote) {
            if (cc == (TQChar)'"')
                state = sstack.pop();
        } else if (cc == (TQChar)'\'') {
            if (!state.dquote) {
                sstack.push( state );
                state.current = singlequote;
            }
        } else if (cc == (TQChar)'"') {
            if (!state.dquote) {
                sstack.push( state );
                state.current = doublequote;
                state.dquote = true;
            }
        } else if (state.current == subst) {
            if (cc == (TQChar)'}')
                state = sstack.pop();
        } else if (cc == (TQChar)')') {
            if (state.current == math) {
                if (str[pos + 1] == (TQChar)')') {
                    state = sstack.pop();
                    pos += 2;
                } else {
                    // false hit: the $(( was a $( ( in fact
                    // ash does not care, but bash does
                    pos = ostack.top().pos;
                    str = ostack.top().str;
                    ostack.pop();
                    state.current = paren;
                    state.dquote = false;
                    sstack.push( state );
                }
                continue;
            } else if (state.current == paren)
                state = sstack.pop();
            else
                break;
        } else if (cc == (TQChar)'}') {
            if (state.current == KMacroExpander::group)
                state = sstack.pop();
            else
                break;
        } else if (cc == (TQChar)'(') {
            sstack.push( state );
            state.current = paren;
        } else if (cc == (TQChar)'{') {
            sstack.push( state );
            state.current = KMacroExpander::group;
        }
        pos++;
    }
    return sstack.empty();
}

bool KMacroExpanderBase::expandMacrosShellQuote( TQString &str )
{
  uint pos = 0;
  return expandMacrosShellQuote( str, pos ) && pos == str.length();
}

int KMacroExpanderBase::expandPlainMacro( const TQString &, uint, TQStringList & )
{ tqFatal( "KMacroExpanderBase::expandPlainMacro called!" ); return 0; }

int KMacroExpanderBase::expandEscapedMacro( const TQString &, uint, TQStringList & )
{ tqFatal( "KMacroExpanderBase::expandEscapedMacro called!" ); return 0; }


//////////////////////////////////////////////////

template<class KT,class VT>
class KMacroMapExpander : public KMacroExpanderBase {

public:
    KMacroMapExpander( const TQMap<KT,VT> &map, TQChar c = '%' ) :
        KMacroExpanderBase( c ), macromap( map ) {}

protected:
    virtual int expandPlainMacro( const TQString &str, uint pos, TQStringList &ret );
    virtual int expandEscapedMacro( const TQString &str, uint pos, TQStringList &ret );

private:
    TQMap<KT,VT> macromap;
};

static TQStringList &operator+=( TQStringList &s, const TQString &n) { s << n; return s; }

////////

static bool
isIdentifier( uint c )
{
    return c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

////////

template<class VT>
class KMacroMapExpander<TQChar,VT> : public KMacroExpanderBase {

public:
    KMacroMapExpander( const TQMap<TQChar,VT> &map, TQChar c = '%' ) :
        KMacroExpanderBase( c ), macromap( map ) {}

protected:
    virtual int expandPlainMacro( const TQString &str, uint pos, TQStringList &ret );
    virtual int expandEscapedMacro( const TQString &str, uint pos, TQStringList &ret );

private:
    TQMap<TQChar,VT> macromap;
};

template<class VT>
int
KMacroMapExpander<TQChar,VT>::expandPlainMacro( const TQString &str, uint pos, TQStringList &ret )
{
    TQMapConstIterator<TQChar,VT> it = macromap.find(str[pos]);
    if (it != macromap.end()) {
       ret += it.data();
       return 1;
    }
    return 0;
}

template<class VT>
int
KMacroMapExpander<TQChar,VT>::expandEscapedMacro( const TQString &str, uint pos, TQStringList &ret )
{
    if (str[pos + 1] == escapeChar()) {
        ret += TQString( escapeChar() );
        return 2;
    }
    TQMapConstIterator<TQChar,VT> it = macromap.find(str[pos+1]);
    if (it != macromap.end()) {
       ret += it.data();
       return 2;
    }

    return 0;
}

template<class VT>
class KMacroMapExpander<TQString,VT> : public KMacroExpanderBase {

public:
    KMacroMapExpander( const TQMap<TQString,VT> &map, TQChar c = '%' ) :
        KMacroExpanderBase( c ), macromap( map ) {}

protected:
    virtual int expandPlainMacro( const TQString &str, uint pos, TQStringList &ret );
    virtual int expandEscapedMacro( const TQString &str, uint pos, TQStringList &ret );

private:
    TQMap<TQString,VT> macromap;
};

template<class VT>
int
KMacroMapExpander<TQString,VT>::expandPlainMacro( const TQString &str, uint pos, TQStringList &ret )
{
    if (isIdentifier( str[pos - 1].unicode() ))
        return 0;
    uint sl;
    for (sl = 0; isIdentifier( str[pos + sl].unicode() ); sl++);
    if (!sl)
        return 0;
    TQMapConstIterator<TQString,VT> it =
        macromap.find( TQConstString( str.unicode() + pos, sl ).string() );
    if (it != macromap.end()) {
        ret += it.data();
        return sl;
    }
    return 0;
}

template<class VT>
int
KMacroMapExpander<TQString,VT>::expandEscapedMacro( const TQString &str, uint pos, TQStringList &ret )
{
    if (str[pos + 1] == escapeChar()) {
        ret += TQString( escapeChar() );
        return 2;
    }
    uint sl, rsl, rpos;
    if (str[pos + 1] == (TQChar)'{') {
        rpos = pos + 2;
        for (sl = 0; str[rpos + sl] != (TQChar)'}'; sl++)
            if (rpos + sl >= str.length())
                return 0;
        rsl = sl + 3;
    } else {
        rpos = pos + 1;
        for (sl = 0; isIdentifier( str[rpos + sl].unicode() ); sl++);
        rsl = sl + 1;
    }
    if (!sl)
        return 0;
    TQMapConstIterator<TQString,VT> it =
        macromap.find( TQConstString( str.unicode() + rpos, sl ).string() );
    if (it != macromap.end()) {
        ret += it.data();
        return rsl;
    }
    return 0;
}

////////////

int
KCharMacroExpander::expandPlainMacro( const TQString &str, uint pos, TQStringList &ret )
{
    if (expandMacro( str[pos], ret ))
        return 1;
    return 0;
}

int
KCharMacroExpander::expandEscapedMacro( const TQString &str, uint pos, TQStringList &ret )
{
    if (str[pos + 1] == escapeChar()) {
        ret += TQString( escapeChar() );
        return 2;
    }
    if (expandMacro( str[pos+1], ret ))
        return 2;
    return 0;
}

int
KWordMacroExpander::expandPlainMacro( const TQString &str, uint pos, TQStringList &ret )
{
    if (isIdentifier( str[pos - 1].unicode() ))
        return 0;
    uint sl;
    for (sl = 0; isIdentifier( str[pos + sl].unicode() ); sl++);
    if (!sl)
        return 0;
    if (expandMacro( TQConstString( str.unicode() + pos, sl ).string(), ret ))
        return sl;
    return 0;
}

int
KWordMacroExpander::expandEscapedMacro( const TQString &str, uint pos, TQStringList &ret )
{
    if (str[pos + 1] == escapeChar()) {
        ret += TQString( escapeChar() );
        return 2;
    }
    uint sl, rsl, rpos;
    if (str[pos + 1] == (TQChar)'{') {
        rpos = pos + 2;
        for (sl = 0; str[rpos + sl] != (TQChar)'}'; sl++)
            if (rpos + sl >= str.length())
                return 0;
        rsl = sl + 3;
    } else {
        rpos = pos + 1;
        for (sl = 0; isIdentifier( str[rpos + sl].unicode() ); sl++);
        rsl = sl + 1;
    }
    if (!sl)
        return 0;
    if (expandMacro( TQConstString( str.unicode() + rpos, sl ).string(), ret ))
        return rsl;
    return 0;
}

////////////

template<class KT,class VT>
inline TQString
TexpandMacros( const TQString &ostr, const TQMap<KT,VT> &map, TQChar c )
{
    TQString str( ostr );
    KMacroMapExpander<KT,VT> kmx( map, c );
    kmx.expandMacros( str );
    return str;
}

template<class KT,class VT>
inline TQString
TexpandMacrosShellQuote( const TQString &ostr, const TQMap<KT,VT> &map, TQChar c )
{
    TQString str( ostr );
    KMacroMapExpander<KT,VT> kmx( map, c );
    if (!kmx.expandMacrosShellQuote( str ))
        return TQString();
    return str;
}

// public API
namespace KMacroExpander {

  TQString expandMacros( const TQString &ostr, const TQMap<TQChar,TQString> &map, TQChar c ) { return TexpandMacros( ostr, map, c ); }
  TQString expandMacrosShellQuote( const TQString &ostr, const TQMap<TQChar,TQString> &map, TQChar c ) { return TexpandMacrosShellQuote( ostr, map, c ); }
  TQString expandMacros( const TQString &ostr, const TQMap<TQString,TQString> &map, TQChar c ) { return TexpandMacros( ostr, map, c ); }
  TQString expandMacrosShellQuote( const TQString &ostr, const TQMap<TQString,TQString> &map, TQChar c ) { return TexpandMacrosShellQuote( ostr, map, c ); }
  TQString expandMacros( const TQString &ostr, const TQMap<TQChar,TQStringList> &map, TQChar c ) { return TexpandMacros( ostr, map, c ); }
  TQString expandMacrosShellQuote( const TQString &ostr, const TQMap<TQChar,TQStringList> &map, TQChar c ) { return TexpandMacrosShellQuote( ostr, map, c ); }
  TQString expandMacros( const TQString &ostr, const TQMap<TQString,TQStringList> &map, TQChar c ) { return TexpandMacros( ostr, map, c ); }
  TQString expandMacrosShellQuote( const TQString &ostr, const TQMap<TQString,TQStringList> &map, TQChar c ) { return TexpandMacrosShellQuote( ostr, map, c ); }

} // namespace
