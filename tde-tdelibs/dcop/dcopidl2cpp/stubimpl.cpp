/*****************************************************************
Copyright (c) 1999 Torben Weis <weis@kde.org>
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
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/
#include <tqdom.h>
#include <tqfile.h>
#include <tqtextstream.h>
#include <tqstring.h>
#include <tqstringlist.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "main.h"
#include "type.h"

static bool isIntType( const TQString& t )
{
  return ((t == "int")
       || (t == "signed int")
       || (t == "unsigned int")
       || (t == "uint")
       || (t == "unsigned")
       || (t == "signed short int")
       || (t == "signed short")
       || (t == "short int")
       || (t == "short")
       || (t == "unsigned short int")
       || (t == "unsigned short")
       || (t == "ushort")
       || (t == "long int")
       || (t == "signed long int")
       || (t == "long")
       || (t == "signed long")
       || (t == "unsigned long int")
       || (t == "unsigned long")
       || (t == "ulong")
       || (t == "char")
       || (t == "signed char")
       || (t == "unsigned char"));
}

/*
 * Writes the stub implementation
 */
void generateStubImpl( const TQString& idl, const TQString& header, const TQString& /*headerBase*/, const TQString& filename, TQDomElement de )
{
    TQFile impl( filename );
    if ( !impl.open( IO_WriteOnly ) )
	tqFatal("Could not write to %s", filename.latin1() );

    TQTextStream str( &impl );

    str << "/****************************************************************************" << endl;
    str << "**" << endl;
    str << "** DCOP Stub Implementation created by dcopidl2cpp from " << idl << endl;
    str << "**" << endl;
    str << "** WARNING! All changes made in this file will be lost!" << endl;
    str << "**" << endl;
    str << "*****************************************************************************/" << endl;
    str << endl;

    str << "#include \"" << header << "\"" << endl;
    str << "#include <dcopclient.h>" << endl << endl;
    str << "#include <kdatastream.h>" << endl;

    TQDomElement e = de.firstChild().toElement();
    for( ; !e.isNull(); e = e.nextSibling().toElement() ) {
	if ( e.tagName() != "CLASS" )
	    continue;
	TQDomElement n = e.firstChild().toElement();
	Q_ASSERT( n.tagName() == "NAME" );
	TQString classNameBase = n.firstChild().toText().data();
	TQString className_stub = classNameBase + "_stub";
    
	TQString classNameFull = className_stub; // class name with possible namespaces prepended
					   // namespaces will be removed from className now
	int namespace_count = 0;
	TQString namespace_tmp = className_stub;
	str << endl;
	for(;;) {
	    int pos = namespace_tmp.find( "::" );
	    if( pos < 0 ) {
		className_stub = namespace_tmp;
		break;
	    }
	    str << "namespace " << namespace_tmp.left( pos ) << " {" << endl;
	    ++namespace_count;
	    namespace_tmp = namespace_tmp.mid( pos + 2 );
	}

	str << endl;

	// Write constructors
	str << className_stub << "::" << className_stub << "( const TQCString& app, const TQCString& obj )" << endl;
	str << "  : ";

	// Always explicitly call DCOPStub constructor, because it's virtual base class.           
	// Calling other ones doesn't matter, as they don't do anything important.
	str << "DCOPStub( app, obj )" << endl;

	str << "{" << endl;
	str << "}" << endl << endl;

	str << className_stub << "::" << className_stub << "( DCOPClient* client, const TQCString& app, const TQCString& obj )" << endl;
	str << "  : ";
    
	str << "DCOPStub( client, app, obj )" << endl;

	str << "{" << endl;
	str << "}" << endl << endl;

	str << className_stub << "::" << className_stub << "( const DCOPRef& ref )" << endl;
	str << "  : ";
    
	str << "DCOPStub( ref )" << endl;

	str << "{" << endl;
	str << "}" << endl << endl;

	// Write marshalling code
	TQDomElement s = e.firstChild().toElement();
	for( ; !s.isNull(); s = s.nextSibling().toElement() ) {
	    if (s.tagName() != "FUNC")
		continue;
	    TQDomElement r = s.firstChild().toElement();
	    Q_ASSERT( r.tagName() == "TYPE" );
	    TQString result = r.firstChild().toText().data();
	    bool async = result == "ASYNC";
	    if ( async) {
		result = "void";
		str << result << " ";
	    } else
		result = writeType( str, r );

	    r = r.nextSibling().toElement();
	    Q_ASSERT ( r.tagName() == "NAME" );
	    TQString funcName = r.firstChild().toText().data();
	    str << className_stub << "::" << funcName << "(";

	    TQStringList args;
	    TQStringList argtypes;
	    bool first = true;
	    r = r.nextSibling().toElement();
	    for( ; !r.isNull(); r = r.nextSibling().toElement() ) {
		if ( !first )
		    str << ", ";
		else
		    str << " ";
		first = false;
		Q_ASSERT( r.tagName() == "ARG" );
		TQDomElement a = r.firstChild().toElement();
		TQString type = writeType( str, a );
		argtypes.append( type );
		args.append( TQString("arg" ) + TQString::number( args.count() ) ) ;
		str << args.last();
	    }
	    if ( !first )
		str << " ";
	    str << ")";

	    //const methods in a stub can't compile, they need to call setStatus()
	    //if ( s.hasAttribute("qual") ) 
       //   str << " " << s.attribute("qual");
	    str << endl;
	
	    str << "{" << endl ;

	
	    funcName += "(";
	    first = true;
	    for( TQStringList::Iterator it = argtypes.begin(); it != argtypes.end(); ++it ){
		if ( !first )
		    funcName += ",";
		first = false;
		funcName += *it;
	    }
	    funcName += ")";
	
	    if ( async ) {

		str << "    if ( !dcopClient()  ) {"<< endl;
		str << "\tsetStatus( CallFailed );" << endl;
		str << "\treturn;" << endl;
		str << "    }" << endl;
	
		str << "    TQByteArray data;" << endl;
		if ( !args.isEmpty() ) {
		    str << "    TQDataStream arg( data, IO_WriteOnly );" << endl;
		    for( TQStringList::Iterator args_count = args.begin(); args_count != args.end(); ++args_count ){
			str << "    arg << " << *args_count << ";" << endl;
		    }
		}

		str << "    dcopClient()->send( app(), obj(), \"" << funcName << "\", data );" << endl;
		str << "    setStatus( CallSucceeded );" << endl;

	    } else {

		if ( result != "void" ) {
		    str << "    " << result << " result";
		    if (isIntType( result ))
			str << " = 0";
		    else if (result == "float" || result == "double")
			str << " = 0.0";
		    else if ( result == "bool" )
			str << " = false";

		    str << ";" << endl;
		}

		str << "    if ( !dcopClient()  ) {"<< endl;
		str << "\tsetStatus( CallFailed );" << endl;
		if ( result != "void" )
		    str << "\treturn result;" << endl;
		else
		    str << "\treturn;" << endl;
		str << "    }" << endl;

		str << "    TQByteArray data, replyData;" << endl;
		str << "    TQCString replyType;" << endl;
	
		if ( !args.isEmpty() ) {
		    str << "    TQDataStream arg( data, IO_WriteOnly );" << endl;
		    for( TQStringList::Iterator args_count = args.begin(); args_count != args.end(); ++args_count ){
			str << "    arg << " << *args_count << ";" << endl;
		    }
		}
		str << "    if ( dcopClient()->call( app(), obj(), \"" << funcName << "\",";
		str << " data, replyType, replyData ) ) {" << endl;
		if ( result != "void" ) {
		    str << "\tif ( replyType == \"" << result << "\" ) {" << endl;
		    str << "\t    TQDataStream _reply_stream( replyData, IO_ReadOnly );"  << endl;
		    str << "\t    _reply_stream >> result;" << endl;
		    str << "\t    setStatus( CallSucceeded );" << endl;
		    str << "\t} else {" << endl;
		    str << "\t    callFailed();" << endl;
		    str << "\t}" << endl;
		} else {
		    str << "\tsetStatus( CallSucceeded );" << endl;
		}
		str << "    } else { " << endl;
		str << "\tcallFailed();" << endl;
		str << "    }" << endl;
		if ( result != "void" )
		    str << "    return result;" << endl;
	    }
	    str << "}" << endl << endl;
	}

	for(; namespace_count > 0; --namespace_count )
	    str << "} // namespace" << endl;
	str << endl;
    }
    impl.close();
}

// :set expandtab!<RETURN>:set ts=8<RETURN>:set sts=4<RETURN>:set sw=4<RETURN>
