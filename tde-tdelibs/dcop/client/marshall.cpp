/*****************************************************************
Copyright (c) 2000 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#define KDE_QT_ONLY
#include "../../tdecore/kurl.cpp"

bool mkBool( const TQString& s )
{
    if ( s.lower()  == "true" )
	return true;
    if ( s.lower()  == "yes" )
	return true;
    if ( s.lower()  == "on" )
	return true;
    if ( s.toInt() != 0 )
	return true;

    return false;
}

TQPoint mkPoint( const TQString &str )
{
    const char *s = str.latin1();
    char *end;
    while(*s && !isdigit(*s) && *s != '-') s++;
    int x = strtol(s, &end, 10);
    s = (const char *)end;
    while(*s && !isdigit(*s) && *s != '-') s++;
    int y = strtol(s, &end, 10);
    return TQPoint( x, y );
}

TQSize mkSize( const TQString &str )
{
    const char *s = str.latin1();
    char *end;
    while(*s && !isdigit(*s) && *s != '-') s++;
    int w = strtol(s, &end, 10);
    s = (const char *)end;
    while(*s && !isdigit(*s) && *s != '-') s++;
    int h = strtol(s, &end, 10);
    return TQSize( w, h );
}

TQRect mkRect( const TQString &str )
{
    const char *s = str.latin1();
    char *end;
    while(*s && !isdigit(*s) && *s != '-') s++;
    int p1 = strtol(s, &end, 10);
    s = (const char *)end;
    bool legacy = (*s == 'x');
    while(*s && !isdigit(*s) && *s != '-') s++;
    int p2 = strtol(s, &end, 10);
    s = (const char *)end;
    while(*s && !isdigit(*s) && *s != '-') s++;
    int p3 = strtol(s, &end, 10);
    s = (const char *)end;
    while(*s && !isdigit(*s) && *s != '-') s++;
    int p4 = strtol(s, &end, 10);
    if (legacy)
    {
       return TQRect( p3, p4, p1, p2 );
    }
    return TQRect( p1, p2, p3, p4 );
}

TQColor mkColor( const TQString& s )
{
    TQColor c;
    c.setNamedColor(s);
    return c;
}

const char *qStringToC(const TQCString &s)
{
   if (s.isEmpty())
      return "";
   return s.data();
}

TQCString demarshal( TQDataStream &stream, const TQString &type )
{
    TQCString result;

    if ( type == "int" || type == "TQ_INT32" )
    {
        int i;
        stream >> i;
        result.setNum( i );
    } else if ( type == "uint" || type == "TQ_UINT32" || type == "unsigned int" )
    {
        uint i;
        stream >> i;
        result.setNum( i );
    } else if ( type == "long" || type == "long int" )
    {
        long l;
        stream >> l;
        result.setNum( l );
    } else if ( type == "unsigned long" || type == "unsigned long int" )
    {
        unsigned long l;
        stream >> l;
        result.setNum( l );
    } else if ( type == "float" )
    {
        float f;
        stream >> f;
        result.setNum( f, 'f' );
    } else if ( type == "double" )
    {
        double d;
        stream >> d;
        result.setNum( d, 'f' );
    } else if ( type == "TQ_INT64" ) {
        TQ_INT64 i;
        stream >> i;
        result.sprintf( "%lld", i );
    } else if ( type == "TQ_UINT64" ) {
        TQ_UINT64 i;
        stream >> i;
        result.sprintf( "%llu", i );
    } else if ( type == "bool" )
    {
        bool b;
        stream >> b;
        result = b ? "true" : "false";
    } else if ( type == "TQString" )
    {
        TQString s;
        stream >> s;
        result = s.local8Bit();
    } else if ( type == "TQCString" )
    {
        stream >> result;
    } else if ( type == "QCStringList" )
    {
        return demarshal( stream, "TQValueList" "<" "TQCString" ">" );
    } else if ( type == "TQStringList" )
    {
        return demarshal( stream, "TQValueList" "<" "TQString" ">" );
    } else if ( type == "TQStringVariantMap" )
    {
        return demarshal(stream, "TQMap" "<" "TQString" "," "TQVariant" ">");
    } else if ( type == "TQColor" )
    {
        TQColor c;
        stream >> c;
        result = TQString(c.name()).local8Bit();
    } else if ( type == "TQSize" )
    {
        TQSize s;
        stream >> s;
        result.sprintf( "%dx%d", s.width(), s.height() );
    } else if ( type == "TQPixmap" || type == "TQImage" )
    {
        TQImage i;
        stream >> i;
        TQByteArray ba;
        TQBuffer buf( ba );
        buf.open( IO_WriteOnly );
        i.save( &buf, "XPM" );
        result = buf.buffer();
    } else if ( type == "TQPoint" )
    {
        TQPoint p;
        stream >> p;
        result.sprintf( "+%d+%d", p.x(), p.y() );
    } else if ( type == "TQRect" )
    {
        TQRect r;
        stream >> r;
        result.sprintf( "%dx%d+%d+%d", r.width(), r.height(), r.x(), r.y() );
    } else if ( type == "TQVariant" )
    {
        TQ_INT32 type;
        stream >> type;
        return demarshal( stream, TQVariant::typeToName( (TQVariant::Type)type ) );
    } else if ( type == "DCOPRef" )
    {
        DCOPRef r;
        stream >> r;
        result.sprintf( "DCOPRef(%s,%s)", qStringToC(r.app()), qStringToC(r.object()) );
    } else if ( type == "KURL" )
    {
        KURL r;
        stream >> r;
        result = r.url().local8Bit();
    } else if ( type.left( 12 ) == "TQValueList" "<" )
    {
        if ( (uint)type.find( '>', 12 ) != type.length() - 1 )
            return result;

        TQString nestedType = type.mid( 12, type.length() - 13 );

        if ( nestedType.isEmpty() )
            return result;

        TQ_UINT32 count;
        stream >> count;

        TQ_UINT32 i = 0;
        for (; i < count; ++i )
        {
            TQCString arg = demarshal( stream, nestedType );
            result += arg;

            if ( i < count - 1 )
                result += '\n';
        }
    } else if ( type.left( 6 ) == "TQMap" "<" )
    {
        int commaPos = type.find( ',', 6 );

        if ( commaPos == -1 )
            return result;

        if ( (uint)type.find( '>', commaPos ) != type.length() - 1 )
            return result;

        TQString keyType = type.mid( 6, commaPos - 6 );
        TQString valueType = type.mid( commaPos + 1, type.length() - commaPos - 2 );

        TQ_UINT32 count;
        stream >> count;

        TQ_UINT32 i = 0;
        for (; i < count; ++i )
        {
            TQCString key = demarshal( stream, keyType );

            if ( key.isEmpty() )
                continue;

            TQCString value = demarshal( stream, valueType );

            if ( value.isEmpty() )
                continue;

            result += key + "->" + value;

            if ( i < count - 1 )
                result += '\n';
        }
    }
    else
    {
       result.sprintf( "<%s>", type.latin1());
    }

    return result;

}

void marshall( TQDataStream &arg, QCStringList args, uint &i, TQString type )
{
    if( i >= args.count() )
    {
	tqWarning("Not enough arguments (expected %u, got %lu).",  i,  args.count());
	exit(1);
    }
    TQString s = TQString::fromLocal8Bit( args[ i ] );

    if (type == "TQStringList") {
       type = "TQValueList" "<" "TQString" ">";
    }
    if (type == "QCStringList") {
       type = "TQValueList" "<" "TQString" ">";
    }

    if ( type == "int" )
	arg << s.toInt();
    else if ( type == "uint" )
	arg << s.toUInt();
    else if ( type == "unsigned" )
	arg << s.toUInt();
    else if ( type == "unsigned int" )
	arg << s.toUInt();
    else if ( type == "TQ_INT32" )
	arg << s.toInt();
    else if ( type == "TQ_INT64" ) {
	TQVariant qv = TQVariant( s );
	arg << qv.toLongLong();
    }
    else if ( type == "TQ_UINT32" )
	arg << s.toUInt();
    else if ( type == "TQ_UINT64" ) {
	TQVariant qv = TQVariant( s );
	arg << qv.toULongLong();
    }
    else if ( type == "long" )
	arg << s.toLong();
    else if ( type == "long int" )
	arg << s.toLong();
    else if ( type == "unsigned long" )
	arg << s.toULong();
    else if ( type == "unsigned long int" )
	arg << s.toULong();
    else if ( type == "float" )
	arg << s.toFloat();
    else if ( type == "double" )
	arg << s.toDouble();
    else if ( type == "bool" )
	arg << mkBool( s );
    else if ( type == "TQString" )
	arg << s;
    else if ( type == "TQCString" )
	arg << TQCString( args[ i ] );
    else if ( type == "TQColor" )
	arg << mkColor( s );
    else if ( type == "TQPoint" )
	arg << mkPoint( s );
    else if ( type == "TQSize" )
	arg << mkSize( s );
    else if ( type == "TQRect" )
	arg << mkRect( s );
    else if ( type == "KURL" )
	arg << KURL( s );
    else if ( type == "TQVariant" ) {
	int tqPointKeywordLength = strlen("TQPoint");
	int tqSizeKeywordLength = strlen("TQSize");
	int tqRectKeywordLength = strlen("TQRect");
	int tqColorKeywordLength = strlen("TQColor");
	if ( s == "true" || s == "false" ) {
	    arg << TQVariant( mkBool( s ) );
	}
	else if ( s.left( 4 ) == "int(" ) {
	    arg << TQVariant( s.mid(4, s.length()-5).toInt() );
	}
	else if ( s.left( (tqPointKeywordLength+1) ) == "TQPoint" "(" ) {
	    arg << TQVariant( mkPoint( s.mid((tqPointKeywordLength+1), s.length()-(tqPointKeywordLength+2)) ) );
	}
	else if ( s.left( (tqSizeKeywordLength+1) ) == "TQSize" "(" ) {
	    arg << TQVariant( mkSize( s.mid((tqSizeKeywordLength+1), s.length()-(tqSizeKeywordLength+2)) ) );
	}
	else if ( s.left( (tqRectKeywordLength+1) ) == "TQRect" "(" ) {
	    arg << TQVariant( mkRect( s.mid((tqRectKeywordLength+1), s.length()-(tqRectKeywordLength+2)) ) );
	}
	else if ( s.left( (tqColorKeywordLength+1) ) == "TQColor" "(" ) {
	    arg << TQVariant( mkColor( s.mid((tqColorKeywordLength+1), s.length()-(tqColorKeywordLength+2)) ) );
	}
	else {
	    arg << TQVariant( s );
	}
    } else if ( type.startsWith("TQValueList" "<") || type == "KURL::List" ) {
	if ( type == "KURL::List" ) {
            type = "KURL";
        }
        else {
	    int tqValueListKeywordLength = strlen("TQValueList");
	    type = type.mid((tqValueListKeywordLength+1), type.length() - (tqValueListKeywordLength+2));
	}
	TQStringList list;
	TQString delim = s;
	if (delim == "[")
	   delim = "]";
	if (delim == "(")
	   delim = ")";
	i++;
	TQByteArray dummy_data;
	TQDataStream dummy_arg(dummy_data, IO_WriteOnly);

	uint j = i;
	uint count = 0;
	// Parse list to get the count
	while (true) {
	    if( j > args.count() )
	    {
		tqWarning("List end-delimiter '%s' not found.", delim.latin1());
		exit(1);
	    }
	    if( TQString::fromLocal8Bit( args[ j ] ) == delim )
		break;
	    marshall( dummy_arg, args, j, type );
	    count++;
	}
	arg << (TQ_UINT32) count;
	// Parse the list for real
	while (true) {
	    if( i > args.count() )
	    {
		tqWarning("List end-delimiter '%s' not found.", delim.latin1());
		exit(1);
	    }
	    if( TQString::fromLocal8Bit( args[ i ] ) == delim )
		break;
	    marshall( arg, args, i, type );
	}
    } else {
	tqWarning( "cannot handle datatype '%s'", type.latin1() );
	exit(1);
    }
    i++;
}
