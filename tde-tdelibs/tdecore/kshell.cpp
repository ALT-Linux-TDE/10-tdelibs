/*
    This file is part of the KDE libraries

    Copyright (c) 2003 Oswald Buddenhagen <ossi@kde.org>

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

#include <kshell.h>

#include <tqfile.h>

#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

static int fromHex( TQChar c )
{
    if (c >= (TQChar)'0' && c <= (TQChar)'9')
        return c - (TQChar)'0';
    else if (c >= (TQChar)'A' && c <= (TQChar)'F')
        return c - (TQChar)'A' + 10;
    else if (c >= (TQChar)'a' && c <= (TQChar)'f')
        return c - (TQChar)'a' + 10;
    return -1;
}

inline static bool isQuoteMeta( uint c )
{
#if 0 // it's not worth it, especially after seeing gcc's asm output ...
    static const uchar iqm[] = {
        0x00, 0x00, 0x00, 0x00, 0x94, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00
    }; // \'"$
    
    return (c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7)));
#else
    return c == (int)'\\' || c == (int)'\'' || c == (int)'"' || c == (int)'$';
#endif
}

inline static bool isMeta( uint c )
{
    static const uchar iqm[] = {
        0x00, 0x00, 0x00, 0x00, 0xdc, 0x07, 0x00, 0xd8,
        0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x38
    }; // \'"$`<>|;&(){}*?#
    
    return (c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7)));
}

TQStringList KShell::splitArgs( const TQString &args, int flags, int *err )
{
    TQStringList ret;
    bool firstword = flags & AbortOnMeta;

    for (uint pos = 0; ; ) {
        TQChar c;
        do {
            if (pos >= args.length())
                goto okret;
            c = args.unicode()[pos++];
        } while (c.isSpace());
        TQString cret;
        if ((flags & TildeExpand) && c == (TQChar)'~') {
            uint opos = pos;
            for (; ; pos++) {
                if (pos >= args.length())
                    break;
                c = args.unicode()[pos];
                if (c == (TQChar)'/' || c.isSpace())
                    break;
                if (isQuoteMeta( c )) {
                    pos = opos;
                    c = (TQChar)'~';
                    goto notilde;
                }
                if ((flags & AbortOnMeta) && isMeta( c ))
                    goto metaerr;
            }
            TQString ccret = homeDir( TQConstString( args.unicode() + opos, pos - opos ).string() );
            if (ccret.isEmpty()) {
                pos = opos;
                c = (TQChar)'~';
                goto notilde;
            }
            if (pos >= args.length()) {
                ret += ccret;
                goto okret;
            }
            pos++;
            if (c.isSpace()) {
                ret += ccret;
                firstword = false;
                continue;
            }
            cret = ccret;
        }
        // before the notilde label, as a tilde does not match anyway
        if (firstword) {
            if (c == (TQChar)'_' || (c >= (TQChar)'A' && c <= (TQChar)'Z') || (c >= (TQChar)'a' && c <= (TQChar)'z')) {
                uint pos2 = pos;
                TQChar cc;
                do
                  cc = args[pos2++];
                while (cc == (TQChar)'_' || (cc >= (TQChar)'A' && cc <= (TQChar)'Z') ||
                       (cc >= (TQChar)'a' && cc <= (TQChar)'z') || (cc >= (TQChar)'0' && cc <= (TQChar)'9'));
                if (cc == (TQChar)'=')
                    goto metaerr;
            }
        }
      notilde:
        do {
            if (c == (TQChar)'\'') {
                uint spos = pos;
                do {
                    if (pos >= args.length())
                        goto quoteerr;
                    c = args.unicode()[pos++];
                } while (c != (TQChar)'\'');
                cret += TQConstString( args.unicode() + spos, pos - spos - 1 ).string();
            } else if (c == (TQChar)'"') {
                for (;;) {
                    if (pos >= args.length())
                        goto quoteerr;
                    c = args.unicode()[pos++];
                    if (c == (TQChar)'"')
                        break;
                    if (c == (TQChar)'\\') {
                        if (pos >= args.length())
                            goto quoteerr;
                        c = args.unicode()[pos++];
                        if (c != (TQChar)'"' && c != (TQChar)'\\' &&
                            !((flags & AbortOnMeta) && (c == (TQChar)'$' || c == (TQChar)'`')))
                            cret += (TQChar)'\\';
                    } else if ((flags & AbortOnMeta) && (c == (TQChar)'$' || c == (TQChar)'`'))
                        goto metaerr;
                    cret += c;
                }
            } else if (c == (TQChar)'$' && args[pos] == (TQChar)'\'') {
                pos++;
                for (;;) {
                    if (pos >= args.length())
                        goto quoteerr;
                    c = args.unicode()[pos++];
                    if (c == (TQChar)'\'')
                        break;
                    if (c == (TQChar)'\\') {
                        if (pos >= args.length())
                            goto quoteerr;
                        c = args.unicode()[pos++];
                        switch (c) {
                        case 'a': cret += (TQChar)'\a'; break;
                        case 'b': cret += (TQChar)'\b'; break;
                        case 'e': cret += (TQChar)'\033'; break;
                        case 'f': cret += (TQChar)'\f'; break;
                        case 'n': cret += (TQChar)'\n'; break;
                        case 'r': cret += (TQChar)'\r'; break;
                        case 't': cret += (TQChar)'\t'; break;
                        case '\\': cret += (TQChar)'\\'; break;
                        case '\'': cret += (TQChar)'\''; break;
                        case 'c': cret += args[pos++] & 31; break;
                        case 'x':
                          {
                            int hv = fromHex( args[pos] );
                            if (hv < 0) {
                                cret += "\\x";
                            } else {
                                int hhv = fromHex( args[++pos] );
                                if (hhv > 0) {
                                    hv = hv * 16 + hhv;
                                    pos++;
                                }
                                cret += TQChar( hv );
                            }
                            break;
                          }
                        default:
                            if (c >= (TQChar)'0' && c <= (TQChar)'7') {
                                int hv = c - '0';
                                for (int i = 0; i < 2; i++) {
                                    c = args[pos];
                                    if (c < (TQChar)'0' || c > (TQChar)'7')
                                        break;
                                    hv = hv * 8 + (c - '0');
                                    pos++;
                                }
                                cret += TQChar( hv );
                            } else {
                                cret += '\\';
                                cret += c;
                            }
                            break;
                        }
                    } else
                        cret += c;
                }
            } else {
                if (c == (TQChar)'\\') {
                    if (pos >= args.length())
                        goto quoteerr;
                    c = args.unicode()[pos++];
                    if (!c.isSpace() &&
                        !((flags & AbortOnMeta) ? isMeta( c ) : isQuoteMeta( c )))
                        cret += '\\';
                } else if ((flags & AbortOnMeta) && isMeta( c ))
                    goto metaerr;
                cret += c;
            }
            if (pos >= args.length())
                break;
            c = args.unicode()[pos++];
        } while (!c.isSpace());
        ret += cret;
        firstword = false;
    }

  okret:
    if (err)
        *err = NoError;
    return ret;

  quoteerr:
   if (err)
       *err = BadQuoting;
   return TQStringList();

  metaerr:
   if (err)
       *err = FoundMeta;
   return TQStringList();
}

inline static bool isSpecial( uint c )
{
    static const uchar iqm[] = {
        0xff, 0xff, 0xff, 0xff, 0xdd, 0x07, 0x00, 0xd8,
        0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x38
    }; // 0-32 \'"$`<>|;&(){}*?#
    
    return (c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7)));
}

TQString KShell::joinArgs( const TQStringList &args )
{
    TQChar q( '\'' );
    TQString ret;
    for (TQStringList::ConstIterator it = args.begin(); it != args.end(); ++it) {
        if (!ret.isEmpty())
            ret += ' ';
        if (!(*it).length())
            ret.append( q ).append( q );
        else {
            for (uint i = 0; i < (*it).length(); i++)
                if (isSpecial((*it).unicode()[i])) {
                    TQString tmp(*it);
                    tmp.replace( q, "'\\''" );
                    ret += q;
                    tmp += q;
                    ret += tmp;
                    goto ex;
                }
            ret += *it;
          ex: ;
        }
    }
    return ret;
}

TQString KShell::joinArgs( const char * const *args, int nargs )
{
    if (!args)
        return TQString::null; // well, TQString::empty, in fact. qt sucks ;)
    TQChar q( '\'' );
    TQString ret;
    for (const char * const *argp = args; nargs && *argp; argp++, nargs--) {
        if (!ret.isEmpty())
            ret += ' ';
        if (!**argp)
            ret.append( q ).append( q );
        else {
            TQString tmp( TQFile::decodeName( *argp ) );
            for (uint i = 0; i < tmp.length(); i++)
                if (isSpecial(tmp.unicode()[i])) {
                    tmp.replace( q, "'\\''" );
                    ret += q;
                    tmp += q;
                    ret += tmp;
                    goto ex;
                }
            ret += tmp;
          ex: ;
       }
    }
    return ret;
}

TQString KShell::joinArgsDQ( const TQStringList &args )
{
    TQChar q( '\'' ), sp( ' ' ), bs( '\\' );
    TQString ret;
    for (TQStringList::ConstIterator it = args.begin(); it != args.end(); ++it) {
        if (!ret.isEmpty())
            ret += sp;
        if (!(*it).length())
            ret.append( q ).append( q );
        else {
            for (uint i = 0; i < (*it).length(); i++)
                if (isSpecial((*it).unicode()[i])) {
                    ret.append( '$' ).append( q );
                    for (uint pos = 0; pos < (*it).length(); pos++) {
                        int c = (*it).unicode()[pos];
                        if (c < 32) {
                            ret += bs;
                            switch (c) {
                            case '\a': ret += 'a'; break;
                            case '\b': ret += 'b'; break;
                            case '\033': ret += 'e'; break;
                            case '\f': ret += 'f'; break;
                            case '\n': ret += 'n'; break;
                            case '\r': ret += 'r'; break;
                            case '\t': ret += 't'; break;
                            case '\034': ret += 'c'; ret += '|'; break;
                            default: ret += 'c'; ret += c + '@'; break;
                            }
                        } else {
                            if (c == '\'' || c == '\\')
                                ret += bs;
                            ret += c;
                        }
                    }
                    ret.append( q );
                    goto ex;
                }
            ret += *it;
          ex: ;
        }
    }
    return ret;
}

TQString KShell::tildeExpand( const TQString &fname )
{
    if (fname[0] == (TQChar)'~') {
        int pos = fname.find( '/' );
        if (pos < 0)
            return homeDir( TQConstString( fname.unicode() + 1, fname.length() - 1 ).string() );
        TQString ret = homeDir( TQConstString( fname.unicode() + 1, pos - 1 ).string() );
        if (!ret.isNull())
            ret += TQConstString( fname.unicode() + pos, fname.length() - pos ).string();
        return ret;
    }
    return fname;
}

TQString KShell::homeDir( const TQString &user )
{
    if (user.isEmpty())
        return TQFile::decodeName( getenv( "HOME" ) );
    struct passwd *pw = getpwnam( TQFile::encodeName( user ).data() );
    if (!pw)
        return TQString::null;
    return TQFile::decodeName( pw->pw_dir );
}
