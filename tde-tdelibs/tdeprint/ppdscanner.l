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

#include <qstringlist.h>
#include <qiodevice.h>
#define YYSTYPE TQStringList
#include "ppdparser.cpp.h"

#define yylval tdeprint_ppdlval

TQIODevice *tdeprint_ppdscanner_device = NULL;
#define YY_INPUT(buf,result,max_size) \
	{ \
		if (tdeprint_ppdscanner_device) \
		{ \
			result = tdeprint_ppdscanner_device->readBlock(buf,max_size); \
			if (result < 0) \
				result = 0; \
		} \
		else \
			result = 0; \
	}
int tdeprint_ppdscanner_lno = 0;

#if 0
#define QDEBUG0(s) tqDebug(s)
#define QDEBUG1(s,a) tqDebug(s,a)
#else
#define QDEBUG0(s)
#define QDEBUG1(s,a)
#endif
%}

%x option value translation_1 translation_2 constr
%option noyywrap

WORD     [^:/\x00-\x20\x7F-\xFF]+
WORD2    [^:/\x00-\x20\x7F-\xFF\*]+
WORD3    [^:/\x00-\x20\x7F-\xFF][^:/\x00-\x1F\x7F-\xFF]*
L        [[:alnum:]]

%%

	/**
	 * Initial state
	 */
\*"JCLOpenUI"        |
\*"OpenUI"{L}*       { QDEBUG0("Open UI"); BEGIN(option); return OPENUI; }
\*"JCLCloseUI"       |
\*"CloseUI"{L}*      { QDEBUG0("Close UI"); BEGIN(value); return CLOSEUI; }
\*"OpenGroup"        { QDEBUG0("Open group"); BEGIN(option); return OPENGROUP; }
\*"CloseGroup"       { QDEBUG0("Close group"); BEGIN(option); return CLOSEGROUP; }
\*"Default"\*{WORD}  { yylval = yytext+9; BEGIN(option); return DEFAULT; }
\*"Default"{WORD}    { yylval = yytext+8; BEGIN(option); return DEFAULT; }
\*"UIConstraints"    |
\*"NonUIConstraints" { BEGIN(constr); return CONSTRAINT; }
\*"PaperDimension"   { BEGIN(option); return PAPERDIM; }
\*"ImageableArea"    { BEGIN(option); return IMGAREA; }
\*"End"              { /* eat up */ }

"*% COMDATA #".*$    { yylval = yytext+12; return FOODATA; }
\*%.*                { QDEBUG0("Comment"); return COMMENT; }
\*{WORD}             { yylval = yytext+1; QDEBUG1("Main keyword: %s",yytext+1); BEGIN(option); return KEYWORD; }

	/**
	 * Option state
	 */
<option>{WORD3}      { yylval = yytext; QDEBUG1("Option: %s",yytext); return OPTION; }
<option>":"          { BEGIN(value); return ':'; }
<option>"\n"         { tdeprint_ppdscanner_lno++; BEGIN(INITIAL); }
<option>"/"          { BEGIN(translation_1); return '/'; }

	/**
	 * Translation state (1: without colon)
	 */
<translation_1>[^\n:]*  { yylval = yytext; QDEBUG1("Translation: %s",yytext); return TRANSLATION; }
<translation_1>":"      { BEGIN(value); return ':'; }
<translation_1>"\n"     { tdeprint_ppdscanner_lno++; BEGIN(INITIAL); }

	/**
	 * Value state
	 */
<value>\"[^\"]*\"    { yylval = yytext; tdeprint_ppdscanner_lno += yylval[0].contains('\n'); QDEBUG1("Quoted value: %s",yytext); return QUOTED; }
<value>{WORD}        { yylval = yytext; QDEBUG1("String part: %s",yytext); return STRINGPART; }
<value>"/"           { BEGIN(translation_2); return '/'; }
<value>"\n"          { tdeprint_ppdscanner_lno++; BEGIN(INITIAL); }
<value>":"           { /* stay in the same state */ return ':'; }

	/**
	 * Translation state (2: with colon)
	 */
<translation_2>[^\n]*   { yylval = yytext; QDEBUG1("Translation: %s",yytext); return TRANSLATION; }
<translation_2>"\n"     { tdeprint_ppdscanner_lno++; BEGIN(INITIAL); }

	/**
	 * Constraint state
	 */
<constr>":"         { return ':'; }
<constr>\*{WORD2}   { yylval = yytext+1; QDEBUG1("Constraint keyword: %s",yytext); return KEYWORD; }
<constr>{WORD2}     { yylval = yytext; QDEBUG1("Constraint option: %s",yytext); return OPTION; }
<constr>"\n"        { tdeprint_ppdscanner_lno++; BEGIN(INITIAL); }

"\n"                { tdeprint_ppdscanner_lno++; /* eat up */ }
<*>.                { /* eat up */ }

%%

void tdeprint_ppdscanner_init(TQIODevice *d)
{
	tdeprint_ppdscanner_device = d;
	tdeprint_ppdscanner_lno = 1;
}

void tdeprint_ppdscanner_terminate( bool deleteIt )
{
	if (deleteIt)
		delete tdeprint_ppdscanner_device;
	tdeprint_ppdscanner_device = NULL;
}

int tdeprint_ppdscanner_numberoflines()
{
	return tdeprint_ppdscanner_lno;
}
