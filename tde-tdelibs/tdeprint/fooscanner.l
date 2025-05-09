%{
/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2003 Michael Goffioul <tdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
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
 **/

#include <qvariant.h>
#include <qiodevice.h>
#define YYSTYPE TQVariant
#include "fooparser.cpp.h"

#define yylval tdeprint_foomatic2lval

#define YY_INPUT( buf, result, max_size ) \
	{ \
		if ( tdeprint_foomatic2scanner_device ) \
		{ \
			result = tdeprint_foomatic2scanner_device->readBlock( buf, max_size ); \
			if ( result < 0 ) \
				result = 0; \
		} \
		else \
			result = 0; \
	}

TQIODevice* tdeprint_foomatic2scanner_device = NULL;
%}

%option noyywrap

D        [[:digit:]]

%%

^"$VAR1->".*$  { /* eat up those assignments */ }
^"$VAR1"       { return VAR; }
^"$postpipe"   { return POSTPIPE; }
"undef"        { return UNDEF; }
#.*            { /* eat up comment parts */ }

"="            { return '='; }
">"            { return '>'; }
","            { return ','; }
"{"            { return '{'; }
"}"            { return '}'; }
"["            { return '['; }
"]"            { return ']'; }
";"            { return ';'; }

'([^']|"\\'")*'          { yylval = QCString(yytext+1, strlen(yytext)-1); return STRING; }
\"[^\"]*\"               { yylval = QCString(yytext+1, strlen(yytext)-1); return QUOTED; }
-?{D}+                   { yylval = yytext; return NUMBER; }
-?{D}+"."{D}+            { yylval = yytext; return NUMBER; }
-?{D}+"."{D}+(e|E)-?{D}+ { yylval = yytext; return NUMBER; }

\n |
.                        { /* eat up anything else */ }

%%

void tdeprint_foomatic2scanner_init( TQIODevice *d )
{
	tdeprint_foomatic2scanner_device = d;
}

void tdeprint_foomatic2scanner_terminate()
{
	tdeprint_foomatic2scanner_device = NULL;
}
