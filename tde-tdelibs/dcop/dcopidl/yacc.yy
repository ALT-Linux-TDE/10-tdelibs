%{
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
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <config.h>

// Workaround for a bison issue:
// bison.simple concludes from _GNU_SOURCE that stpcpy is available,
// while GNU string.h only exposes it if __USE_GNU is set.
#ifdef _GNU_SOURCE
#define __USE_GNU 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <tqstring.h>

#define AMP_ENTITY "&amp;"
#define YYERROR_VERBOSE

extern int yylex();

// extern TQString idl_lexFile;
extern int idl_line_no;
extern int function_mode;

static int dcop_area = 0;
static int dcop_signal_area = 0;

static TQString in_namespace( "" );

void dcopidlInitFlex( const char *_code );

void yyerror( const char *s )
{
	tqDebug( "In line %i : %s", idl_line_no, s );
        exit(1);
	//   theParser->parse_error( idl_lexFile, s, idl_line_no );
}

%}


%union
{
  long   _int;
  TQString        *_str;
  unsigned short          _char;
  double _float;
}

%nonassoc T_UNIMPORTANT
%token <_char> T_CHARACTER_LITERAL
%token <_float> T_DOUBLE_LITERAL
%right <_str> T_IDENTIFIER
%token <_int> T_INTEGER_LITERAL
%token <_str> T_STRING_LITERAL
%token <_str> T_INCLUDE
%token T_CLASS
%token T_STRUCT
%token T_LEFT_CURLY_BRACKET
%token T_LEFT_PARANTHESIS
%token T_RIGHT_CURLY_BRACKET
%token T_RIGHT_PARANTHESIS
%token T_COLON
%token T_SEMICOLON
%token T_PUBLIC
%token T_PROTECTED
%token T_TRIPE_DOT
%token T_PRIVATE
%token T_VIRTUAL
%token T_CONST
%token T_INLINE
%token T_FRIEND
%token T_RETURN
%token T_SIGNAL
%token T_SLOT
%token T_TYPEDEF
%token T_PLUS
%token T_MINUS
%token T_COMMA
%right T_ASTERISK
%token T_TILDE
%token T_LESS
%token T_GREATER
%token T_AMPERSAND
%token T_EXTERN
%token T_EXTERN_C
%token T_ACCESS
%token T_ENUM
%token T_NAMESPACE
%token T_USING
%token T_UNKNOWN
%token T_TRIPLE_DOT
%token T_TRUE
%token T_FALSE
%token T_STATIC
%token T_MUTABLE
%token T_EQUAL
%token T_SCOPE
%token T_NULL
%token T_INT
%token T_ARRAY_OPEN
%token T_ARRAY_CLOSE
%token T_CHAR
%token T_DCOP
%token T_DCOP_AREA
%token T_DCOP_SIGNAL_AREA
%token T_SIGNED
%token T_UNSIGNED
%token T_LONG
%token T_SHORT
%token T_FUNOPERATOR
%token T_MISCOPERATOR
%token T_SHIFT

%type <_str> body
%type <_str> class_header
%type <_str> super_classes
%type <_str> super_class
%type <_str> super_class_name
%type <_str> typedef
%type <_str> function
%type <_str> function_header
%type <_str> param
%type <_str> type
%type <_str> type_name
%type <_str> templ_type
%type <_str> templ_type_list
%type <_str> type_list
%type <_str> params
%type <_str> int_type
%type <_int> const_qualifier
%type <_int> virtual_qualifier
%type <_str> Identifier
%type <_int> dcoptag

%%

/*1*/
main
	: includes declaration main
	  {
	  }
	| /* empty */
	;
	
includes
	: includes T_INCLUDE
          {
		printf("<INCLUDE>%s</INCLUDE>\n", $2->latin1() );
	  }
	| T_EXTERN_C T_LEFT_CURLY_BRACKET main T_RIGHT_CURLY_BRACKET
	  {
	  }
        | /* empty */
          {
          }
        ;

dcoptag
	: T_DCOP { $$ = 1; }
	| /* empty */ { $$ = 0; }
	;

declaration
	: T_CLASS Identifier class_header dcoptag body T_SEMICOLON
	  {
	 	if ($4)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n%s%s</CLASS>\n", ( in_namespace + *$2 ).latin1(), $3->latin1(), $5->latin1() );
		// default C++ visibility specifier is 'private'
		dcop_area = 0;
		dcop_signal_area = 0;

	  }
	| T_CLASS T_IDENTIFIER Identifier class_header dcoptag body T_SEMICOLON
	  {
	 	if ($5)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n    <LINK_SCOPE>%s</LINK_SCOPE>\n%s%s</CLASS>\n", ( in_namespace + *$3 ).latin1(),$2->latin1(),  $4->latin1(), $6->latin1() );
		// default C++ visibility specifier is 'private'
		dcop_area = 0;
		dcop_signal_area = 0;

	  }
	| T_CLASS Identifier T_SEMICOLON
	  {
	  }
	| T_STRUCT Identifier T_SEMICOLON
	  {
	  }
	| T_STRUCT Identifier class_header body T_SEMICOLON
	  {
	  }
	| T_NAMESPACE T_IDENTIFIER T_LEFT_CURLY_BRACKET
                  {
                      in_namespace += *$2; in_namespace += "::";
                  } 
            main T_RIGHT_CURLY_BRACKET opt_semicolon
                  {
                      int pos = in_namespace.findRev( "::", -3 );
                      if( pos >= 0 )
                          in_namespace = in_namespace.left( pos + 2 );
                      else
                          in_namespace = "";
                  }
        | T_USING T_NAMESPACE T_IDENTIFIER T_SEMICOLON
          {
          }
        | T_USING T_IDENTIFIER T_SCOPE T_IDENTIFIER T_SEMICOLON
          {
          }
	| T_EXTERN T_SEMICOLON
	  {
	  }
	| T_TYPEDEF type Identifier T_SEMICOLON
	  {
	  }
	| T_TYPEDEF T_STRUCT T_LEFT_CURLY_BRACKET member_list T_RIGHT_CURLY_BRACKET Identifier T_SEMICOLON
	  {
	  }
	| T_TYPEDEF T_STRUCT Identifier T_LEFT_CURLY_BRACKET member_list T_RIGHT_CURLY_BRACKET Identifier T_SEMICOLON
	  {
	  }
	| T_INLINE function
	  {
	  }
	| function
	  {
	  }
	| member
	  {
	  }
	| enum
	  {
	  }
	;

member_list
	: member member_list
	| /* empty */
	;

bool_value: T_TRUE | T_FALSE;

nodcop_area: T_PRIVATE | T_PROTECTED | T_PUBLIC ;

sigslot: T_SIGNAL | T_SLOT | ;

nodcop_area_begin
	: nodcop_area sigslot T_COLON
        {
	  dcop_area = 0;
	  dcop_signal_area = 0;
	}
	| sigslot T_COLON
	{
	  dcop_area = 0;
	  dcop_signal_area = 0;
	}
	;

dcop_area_begin
	: T_DCOP_AREA T_COLON
	{
	  dcop_area = 1;
	  dcop_signal_area = 0;
	}
	;

dcop_signal_area_begin
	: T_DCOP_SIGNAL_AREA T_COLON
	{
	  /*
	  A dcop signals area needs all dcop area capabilities,
	  e.g. parsing of function parameters.
	  */
	  dcop_area = 1;
	  dcop_signal_area = 1;
	}
	;
	
Identifier
	: T_IDENTIFIER %prec T_UNIMPORTANT {
	  $$ = $1;
	}
	| T_IDENTIFIER T_SCOPE Identifier {
	   TQString* tmp = new TQString( "%1::%2" );
           *tmp = tmp->arg(*($1)).arg(*($3));
           $$ = tmp;
	}
	;
		
super_class_name
	: Identifier
	  {
		TQString* tmp = new TQString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *($1) );
		$$ = tmp;
	  }
	| Identifier T_LESS type_list T_GREATER
	  {
		TQString* tmp = new TQString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *($1) + "&lt;" + *($3) + "&gt;" );
		$$ = tmp;
	  }
	;

super_class
	: virtual_qualifier T_PUBLIC super_class_name
	  {
		$$ = $3;
	  }
	| super_class_name
	  {
		$$ = $1;
	  }
	;

super_classes
	: super_class T_LEFT_CURLY_BRACKET
	  {
		$$ = $1;
	  }
	| super_class T_COMMA super_classes
	  {
		/* $$ = $1; */
		$$ = new TQString( *($1) + *($3) );
	  }
	;

class_header
	: T_COLON super_classes
	  {
		$$ = $2;
	  }
	| T_LEFT_CURLY_BRACKET
	  {
		$$ = new TQString( "" );
	  }
	;

opt_semicolon
        : /* empty */
          {
          }
        | T_SEMICOLON
	  ;

body
	: T_RIGHT_CURLY_BRACKET
	  {
		$$ = new TQString( "" );
	  }
	| typedef body
	  {
		$$ = new TQString( *($1) + *($2) );
	  }
	| T_INLINE function body
	  {
		$$ = new TQString( *($2) + *($3) );
	  }
	| function body
	  {
		$$ = new TQString( *($1) + *($2) );
	  }
	| dcop_signal_area_begin body
	  {
		$$ = $2;
	  }
	| enum body
	  {
		$$ = $2;
	  }
	| dcop_area_begin body
	  {
		$$ = $2;
	  }
	| nodcop_area_begin body
	  {	
	        $$ = $2;
	  }
	| member body
	  {
 	        $$ = $2;
	  }
	| T_FRIEND T_CLASS Identifier T_SEMICOLON body
	  {
		$$ = $5;
	  }
	| T_FRIEND Identifier T_SEMICOLON body
	  {
		$$ = $4;
	  }
	| T_FRIEND function_header T_SEMICOLON body
	  {
		$$ = $4;
	  }
	| T_CLASS Identifier T_SEMICOLON body
	  {
		$$ = $4;
	  }
        | T_CLASS Identifier class_header body T_SEMICOLON body
          {
                $$ = $6;
          }
	| T_STRUCT Identifier T_SEMICOLON body
	  {
		$$ = $4;
	  }
        | T_STRUCT Identifier class_header body T_SEMICOLON body
          {
                $$ = $6;
          }
        | T_USING T_IDENTIFIER T_SCOPE T_IDENTIFIER T_SEMICOLON body
          {
                $$ = $6;
          }
	;

enum
	: T_ENUM T_IDENTIFIER T_LEFT_CURLY_BRACKET enum_list T_RIGHT_CURLY_BRACKET T_IDENTIFIER T_SEMICOLON
	| T_ENUM T_IDENTIFIER T_LEFT_CURLY_BRACKET enum_list T_RIGHT_CURLY_BRACKET T_SEMICOLON
	| T_ENUM T_LEFT_CURLY_BRACKET enum_list T_RIGHT_CURLY_BRACKET T_IDENTIFIER T_SEMICOLON
	| T_ENUM T_LEFT_CURLY_BRACKET enum_list T_RIGHT_CURLY_BRACKET T_SEMICOLON
	;

enum_list
	: enum_item T_COMMA enum_list
	| enum_item
	;

enum_item
	: T_IDENTIFIER T_EQUAL int_expression {}
	| T_IDENTIFIER {}
	;

number
	: T_CHARACTER_LITERAL {}
	| T_INTEGER_LITERAL {}
	| T_MINUS T_INTEGER_LITERAL {}
	| T_NULL {}
	| Identifier {}
	;
	
int_expression
	: number {}
	| number T_PLUS number {}
	| number T_SHIFT number {}
	;

typedef
	: T_TYPEDEF Identifier T_LESS type_list T_GREATER Identifier T_SEMICOLON
	  {
		if (dcop_area) {
 		  TQString* tmp = new TQString("<TYPEDEF name=\"%1\" template=\"%2\"><PARAM %3</TYPEDEF>\n");
		  *tmp = tmp->arg( *($6) ).arg( *($2) ).arg( *($4) );
		  $$ = tmp;
		} else {
		  $$ = new TQString("");
		}
	  }
	| T_TYPEDEF Identifier T_LESS type_list T_GREATER T_SCOPE T_IDENTIFIER Identifier T_SEMICOLON
	  {
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  }
	;

const_qualifier
	: /* empty */
	  {
		$$ = 0;
	  }
	| T_CONST
	  {
		$$ = 1;
	  }
	;

int_type
	: T_SIGNED { $$ = new TQString("signed int"); }
	| T_SIGNED T_INT { $$ = new TQString("signed int"); }
	| T_UNSIGNED { $$ = new TQString("unsigned int"); }
	| T_UNSIGNED T_INT { $$ = new TQString("unsigned int"); }
	| T_SIGNED T_SHORT { $$ = new TQString("signed short int"); }
	| T_SIGNED T_SHORT T_INT { $$ = new TQString("signed short int"); }
	| T_SIGNED T_LONG { $$ = new TQString("signed long int"); }
	| T_SIGNED T_LONG T_INT { $$ = new TQString("signed long int"); }
	| T_UNSIGNED T_SHORT { $$ = new TQString("unsigned short int"); }
	| T_UNSIGNED T_SHORT T_INT { $$ = new TQString("unsigned short int"); }
	| T_UNSIGNED T_LONG { $$ = new TQString("unsigned long int"); }
	| T_UNSIGNED T_LONG T_INT { $$ = new TQString("unsigned long int"); }
	| T_INT { $$ = new TQString("int"); }
	| T_LONG { $$ = new TQString("long int"); }
	| T_LONG T_INT { $$ = new TQString("long int"); }
	| T_SHORT { $$ = new TQString("short int"); }
	| T_SHORT T_INT { $$ = new TQString("short int"); }
	| T_CHAR { $$ = new TQString("char"); }
	| T_SIGNED T_CHAR { $$ = new TQString("signed char"); }
	| T_UNSIGNED T_CHAR { $$ = new TQString("unsigned char"); }
	;

asterisks
	: T_ASTERISK asterisks
	| T_ASTERISK
	;

params
	: /* empty */
	  {
		$$ = new TQString( "" );
	  }
	| param
	| params T_COMMA param
	  {
		$$ = new TQString( *($1) + *($3) );
	  }
	;

/* Lowlevel definition of a type - doesn't include const, * nor & */
type_name
	: int_type
	| Identifier { $$ = $1; }
	| T_STRUCT Identifier { $$ = $2; }
	| T_CLASS Identifier { $$ = $2; }
	| Identifier T_LESS templ_type_list T_GREATER {
		TQString *tmp = new TQString("%1&lt;%2&gt;");
		*tmp = tmp->arg(*($1));
		*tmp = tmp->arg(*($3));
		$$ = tmp;
	 }
	| Identifier T_LESS templ_type_list T_GREATER T_SCOPE Identifier{
		TQString *tmp = new TQString("%1&lt;%2&gt;::%3");
		*tmp = tmp->arg(*($1));
		*tmp = tmp->arg(*($3));
		*tmp = tmp->arg(*($6));
		$$ = tmp;
	 }
	;

/* List of types inside a template def like QMap< blah, blah > */
templ_type_list
	: templ_type T_COMMA templ_type_list
	  {
	    $$ = new TQString(*($1) + "," + *($3));
	  }
	| templ_type
  	  {
 	    $$ = $1;
	  }
	;

/* One type inside a template. No '&' or const here. */
templ_type
	: type_name asterisks
  	  {
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  }
	| type_name
  	  {
 	    $$ = $1;
	  }
	;

/* The hi-level, complete definition of a type, which also generates
   the <TYPE> tag for it */
type
	: T_CONST type_name asterisks
  	  {
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  }
	| T_CONST type_name T_AMPERSAND {
	     if (dcop_area) {
	  	TQString* tmp = new TQString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *($2) );
		$$ = tmp;
	     }
	  }
	| T_CONST type_name %prec T_UNIMPORTANT {
		TQString* tmp = new TQString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *($2) );
		$$ = tmp;
	}
    | type_name T_CONST %prec T_UNIMPORTANT {
        TQString* tmp = new TQString("<TYPE>%1</TYPE>");
        *tmp = tmp->arg( *($1) );
        $$ = tmp;
    }
    | type_name T_CONST T_AMPERSAND { 
        if (dcop_area) { 
           TQString* tmp = new TQString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>"); 
           *tmp = tmp->arg( *($1) ); 
           $$ = tmp; 
        } 
    } 
	| type_name T_AMPERSAND {
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  }

	| type_name %prec T_UNIMPORTANT {
		TQString* tmp = new TQString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *($1) );
		$$ = tmp;
	}
	| type_name asterisks
  	  {
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  }
	;

type_list
	: type T_COMMA type_list
	  {
	    $$ = new TQString(*($1) + "," + *($3));
	  }
	| type
  	  {
 	    $$ = $1;
	  }
	;
	
param
	: type Identifier default
	  {
		if (dcop_area) {
		   TQString* tmp = new TQString("\n        <ARG>%1<NAME>%2</NAME></ARG>");
  		   *tmp = tmp->arg( *($1) );
  		   *tmp = tmp->arg( *($2) );
		   $$ = tmp;		
		} else $$ = new TQString();
	  }
	| type default
	  {
		if (dcop_area) {
		   TQString* tmp = new TQString("\n        <ARG>%1</ARG>");
  		   *tmp = tmp->arg( *($1) );
		   $$ = tmp;		
		} else $$ = new TQString();
	  }
	| T_TRIPLE_DOT
	  {
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area.");
		$$ = new TQString("");
	  }
	;

default
	: /* empty */
	  {
	  }
	| T_EQUAL value
	  {
	  }
	| T_EQUAL T_LEFT_PARANTHESIS type T_RIGHT_PARANTHESIS value /* cast */
	  {
	  }
	;

/* Literal or calculated value, used for an initialization */
value
	: T_STRING_LITERAL
          {
          }
        | int_expression
          {
          }
        | T_DOUBLE_LITERAL
          {
          }
        | bool_value
          {
          }
        | Identifier T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS
          {
          }
        ;

virtual_qualifier
	: /* empty */ { $$ = 0; }
	| T_VIRTUAL { $$ = 1; }
	;

operator
	: T_MISCOPERATOR | T_SHIFT | T_GREATER | T_LESS | T_EQUAL
	;

function_header
	: type Identifier T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS const_qualifier
	  {
	     if (dcop_area || dcop_signal_area) {
		TQString* tmp = 0;
                tmp = new TQString(
                        "    <%4>\n"
                        "        %2\n"
                        "        <NAME>%1</NAME>"
                        "%3\n"
                        "     </%5>\n");
		*tmp = tmp->arg( *($2) );
		*tmp = tmp->arg( *($1) );
                *tmp = tmp->arg( *($4) );
                
                TQString tagname = (dcop_signal_area) ? "SIGNAL" : "FUNC";
                TQString attr = ($6) ? " qual=\"const\"" : "";
                *tmp = tmp->arg( TQString("%1%2").arg(tagname).arg(attr) );
                *tmp = tmp->arg( TQString("%1").arg(tagname) );
		$$ = tmp;
   	     } else
	        $$ = new TQString("");
	  }
	| type T_FUNOPERATOR operator T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS const_qualifier
	  {
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     $$ = new TQString("");
	  }
	;

/* In an inline constructor:
   List of values used as initialization for member var, or as params to parent constructor */
values
	: value {}
	| value T_COMMA values {}
	| /* empty */ {}
	;
	
/* One initialization done by an inline constructor */
init_item
	: T_IDENTIFIER T_LEFT_PARANTHESIS values T_RIGHT_PARANTHESIS {}
	;

/* List of initializations done by an inline constructor */
init_list
	: init_item {}
	| init_item T_COMMA init_list {}
	;

function
	: function_header function_body
 	  {
	        $$ = $1;
	  }
	| T_VIRTUAL function_header T_EQUAL T_NULL function_body
	  {
		$$ = $2;
	  }
	| T_VIRTUAL function_header function_body
	  {
		$$ = $2;
	  }
	| Identifier T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS function_body
	  {
	      /* The constructor */
	      assert(!dcop_area);
              $$ = new TQString("");
	  }
	| Identifier T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS T_COLON init_list function_body
	  {
	      /* The constructor */
	      assert(!dcop_area);
              $$ = new TQString("");
	  }
	| virtual_qualifier T_TILDE Identifier T_LEFT_PARANTHESIS T_RIGHT_PARANTHESIS function_body
	  {
	      /* The destructor */
  	      assert(!dcop_area);
              $$ = new TQString("");
	  }
	| T_STATIC function_header function_body
	  {
              if (dcop_area) {
                 if (dcop_signal_area)
                     yyerror("DCOP signals cannot be static");
                 else
                     yyerror("DCOP functions cannot be static");
              } else {
                 $$ = new TQString();
              }  
	  }      
	;

function_begin : T_LEFT_CURLY_BRACKET
	{
		function_mode = 1;
	}
	;
	
function_body
	: T_SEMICOLON
	| function_begin function_lines T_RIGHT_CURLY_BRACKET
	| function_begin function_lines T_RIGHT_CURLY_BRACKET T_SEMICOLON
	;

function_lines
	: function_line function_lines {}
	| /* empty */ {}
	;

function_line
	: T_SEMICOLON /* dummy */
	;

Identifier_list_rest
	: T_COMMA Identifier_list
	| /* empty */
	;

Identifier_list_entry : T_IDENTIFIER {}
                      | T_IDENTIFIER T_EQUAL value {}
                      | asterisks T_IDENTIFIER {}
	;

Identifier_list : Identifier_list_entry Identifier_list_rest {}
	;

member
	: type Identifier_list T_SEMICOLON {}
	| type Identifier T_COLON T_INTEGER_LITERAL T_SEMICOLON {}
	| T_STATIC type T_IDENTIFIER default T_SEMICOLON {}
	| T_MUTABLE type T_IDENTIFIER default T_SEMICOLON {}
	| type T_IDENTIFIER T_ARRAY_OPEN int_expression T_ARRAY_CLOSE T_SEMICOLON {}
	| T_STATIC type T_IDENTIFIER T_ARRAY_OPEN int_expression T_ARRAY_CLOSE T_SEMICOLON {}
	;

%%

void dcopidlParse( const char *_code )
{
    dcopidlInitFlex( _code );
    yyparse();
}
