/****************************************************************************

 Copyright (C) 2005 Lubos Lunak        <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

#undef TQT_NO_CAST_ASCII

// See description in tdestartupconfig.cpp .

#include <tqfile.h>
#include <tqtextstream.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <tdeconfig.h>
#include <kdebug.h>

TQString get_entry( TQString* ll )
    {
    TQString& l = *ll;
    l = l.stripWhiteSpace();
    if( l.isEmpty())
        return TQString::null;
    TQString ret;
    if( l[ 0 ] == '\'' )
        {
        unsigned int pos = 1;
        while( pos < l.length() && l[ pos ] != '\'' )
            ret += l[ pos++ ];
        if( pos >= l.length())
            {
            *ll = TQString::null;
            return TQString::null;
            }
        *ll = l.mid( pos + 1 );
        return ret;
        }
    unsigned int pos = 0;
    while( pos < l.length() && l[ pos ] != ' ' )
        ret += l[ pos++ ];
    *ll = l.mid( pos );
    return ret;
    }

int main()
    {
    TDEInstance inst( "tdedostartupconfig" );
    kdDebug() << "Running tdedostartupconfig." << endl;
    TQString keysname = locateLocal( "config", "startupconfigkeys" );
    TQFile keys( keysname );
    if( !keys.open( IO_ReadOnly ))
        return 3;
    TQFile f1( locateLocal( "config", "startupconfig" ));
    if( !f1.open( IO_WriteOnly ))
        return 4;
    TQFile f2( locateLocal( "config", "startupconfigfiles" ));
    if( !f2.open( IO_WriteOnly ))
        return 5;
    TQTextStream startupconfig( &f1 );
    TQTextStream startupconfigfiles( &f2 );
    startupconfig << "#! /bin/sh\n";
    for(;;)
        {
        TQString line;
        if( keys.readLine( line, 1024 ) < 0 )
            break;
        line = line.stripWhiteSpace();
        if( line.isEmpty())
            break;
        TQString tmp = line;
        TQString file, group, key, def;
        file = get_entry( &tmp );
        group = get_entry( &tmp );
        key = get_entry( &tmp );
        def = get_entry( &tmp );
        if( file.isEmpty() || group.isEmpty())
            return 6;
        if( group.left( 1 ) == "[" && group.right( 1 ) == "]" )
            { // whole config group
            TDEConfig cfg( file );
            group = group.mid( 1, group.length() - 2 );
            TQMap< TQString, TQString > entries = cfg.entryMap( group );
            startupconfig << "# " << line << "\n";
            for( TQMap< TQString, TQString >::ConstIterator it = entries.begin();
                 it != entries.end();
                 ++it )
                {
                TQString key = it.key();
                TQString value = *it;
                startupconfig << TQString(file.replace( ' ', '_' )).lower()
                    << "_" << TQString(group.replace( ' ', '_' )).lower()
                    << "_" << TQString(key.replace( ' ', '_' )).lower()
                    << "=\"" << value.replace( "\"", "\\\"" ) << "\"\n";
                }
            }
        else
            { // a single key
            if( key.isEmpty())
                return 7;
            TDEConfig cfg( file );
            cfg.setGroup( group );
            TQString value = cfg.readEntry( key, def );
            startupconfig << "# " << line << "\n";
            startupconfig << TQString(file.replace( ' ', '_' )).lower()
                << "_" << TQString(group.replace( ' ', '_' )).lower()
                << "_" <<TQString( key.replace( ' ', '_' )).lower()
                << "=\"" << value.replace( "\"", "\\\"" ) << "\"\n";
            }
        startupconfigfiles << line << endl;
        // use even currently non-existing paths in $TDEDIRS
        TQStringList dirs = TQStringList::split( KPATH_SEPARATOR, TDEGlobal::dirs()->kfsstnd_prefixes());
        for( TQStringList::ConstIterator it = dirs.begin();
             it != dirs.end();
             ++it )
            {
            TQString cfg = *it + "share/config/" + file;
            if( TDEStandardDirs::exists( cfg ))
                startupconfigfiles << cfg << "\n";
            else
                startupconfigfiles << "!" << cfg << "\n";
            }
        startupconfigfiles << "*\n";
        }
    return 0;
    }
