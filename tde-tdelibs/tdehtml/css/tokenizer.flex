%option case-insensitive
%option noyywrap
%option 8bit

h               [0-9a-fA-F]
nonascii        [\200-\377]
unicode         \\{h}{1,6}[ \t\r\n\f]?
escape          {unicode}|\\[ -~\200-\377]
nmstart         [_a-zA-Z]|{nonascii}|{escape}
nmchar          [_a-zA-Z0-9-]|{nonascii}|{escape}
string1         \"([\t !#$%&(-~]|\\{nl}|\'|{nonascii}|{escape})*\"
string2         \'([\t !#$%&(-~]|\\{nl}|\"|{nonascii}|{escape})*\'

ident           -?{nmstart}{nmchar}*
name            {nmchar}+
num             [0-9]+|[0-9]*"."[0-9]+
intnum          [0-9]+
string          {string1}|{string2}
url             ([!#$%&*-~]|{nonascii}|{escape})*
w               [ \t\r\n\f]*
nl              \n|\r\n|\r|\f
range           \?{1,6}|{h}(\?{0,5}|{h}(\?{0,4}|{h}(\?{0,3}|{h}(\?{0,2}|{h}(\??|{h})))))
nth             (-?[0-9]*n[\+-][0-9]+)|(-?[0-9]*n)

%%

\/\*[^*]*\*+([^/*][^*]*\*+)*\/  /* ignore comments */

[ \t\r\n\f]+            {yyTok = S; return yyTok;}

"<!--"                  {yyTok = SGML_CD; return yyTok;}
"-->"                   {yyTok = SGML_CD; return yyTok;}
"~="                    {yyTok = INCLUDES; return yyTok;}
"|="                    {yyTok = DASHMATCH; return yyTok;}
"^="                    {yyTok = BEGINSWITH; return yyTok;}
"$="                    {yyTok = ENDSWITH; return yyTok;}
"*="                    {yyTok = CONTAINS; return yyTok;}

{string}                {yyTok = STRING; return yyTok;}

{ident}                 {yyTok = IDENT; return yyTok;}

{nth}                   {yyTok = NTH; return yyTok;}

"#"{name}               {yyTok = HASH; return yyTok;}

"@import"               {yyTok = IMPORT_SYM; return yyTok;}
"@page"                 {yyTok = PAGE_SYM; return yyTok;}
"@media"                {yyTok = MEDIA_SYM; return yyTok;}
"@font-face"            {yyTok = FONT_FACE_SYM; return yyTok;}
"@charset"              {yyTok = CHARSET_SYM; return yyTok;}
"@namespace"		{yyTok = NAMESPACE_SYM; return yyTok; }
"@-khtml-rule"    {yyTok = TDEHTML_RULE_SYM; return yyTok; }
"@-khtml-decls"   {yyTok = TDEHTML_DECLS_SYM; return yyTok; }
"@-khtml-value"   {yyTok = TDEHTML_VALUE_SYM; return yyTok; }

"!"{w}"important"         {yyTok = IMPORTANT_SYM; return yyTok;}

{num}em                 {yyTok = EMS; return yyTok;}
{num}__qem              {yyTok = QEMS; return yyTok;} /* quirky ems */
{num}ex                 {yyTok = EXS; return yyTok;}
{num}px                 {yyTok = PXS; return yyTok;}
{num}cm                 {yyTok = CMS; return yyTok;}
{num}mm                 {yyTok = MMS; return yyTok;}
{num}in                 {yyTok = INS; return yyTok;}
{num}pt                 {yyTok = PTS; return yyTok;}
{num}pc                 {yyTok = PCS; return yyTok;}
{num}deg                {yyTok = DEGS; return yyTok;}
{num}rad                {yyTok = RADS; return yyTok;}
{num}grad               {yyTok = GRADS; return yyTok;}
{num}ms                 {yyTok = MSECS; return yyTok;}
{num}s                  {yyTok = SECS; return yyTok;}
{num}Hz                 {yyTok = HERZ; return yyTok;}
{num}kHz                {yyTok = KHERZ; return yyTok;}
{num}{ident}            {yyTok = DIMEN; return yyTok;}
{num}%                  {yyTok = PERCENTAGE; return yyTok;}
{intnum}                {yyTok = INTEGER; return yyTok;}
{num}                   {yyTok = FLOAT; return yyTok;}


"not("                  {yyTok = NOTFUNCTION; return yyTok;}
"url("{w}{string}{w}")" {yyTok = URI; return yyTok;}
"url("{w}{url}{w}")"    {yyTok = URI; return yyTok;}
{ident}"("              {yyTok = FUNCTION; return yyTok;}

U\+{range}              {yyTok = UNICODERANGE; return yyTok;}
U\+{h}{1,6}-{h}{1,6}    {yyTok = UNICODERANGE; return yyTok;}

.                       {yyTok = *yytext; return yyTok;}

%%

