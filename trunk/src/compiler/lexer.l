%{
/*
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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
 *
 *  Mehmet Soyturk <Mehmet.Soyturk@UGent.be>
 *
 *  Writen with the help of:
 *    ecmascan.l:     code from links2
 *    java_lexer.l:   Jakob Petsovits <jpetso@gmx.at> (unicode)
 *
 */
%}

%{
#include "params.h"

#define YY_EXTRA_TYPE Params*
#define YY_USER_ACTION yylloc->first_line = yylineno;

/*=====================================*/

#include <stdio.h>
#include "nodes.h"
#include "parser.tab.h"

/*#define DEBUG(txt) fprintf(stderr, "[%d,%d][%s] %s\n", yyextra->prev_line_no+1, yyextra->prev_column_no+1, (txt), yytext) */
#define DEBUG(txt) 

#define COUNT (count(yyscanner, yyextra))

static void count(yyscan_t scanner, Params* params);

%}

 /*=====================================*/

%option reentrant
%option bison-bridge
%option bison-locations
%option noyywrap
%option yylineno

 /*====================================*/
 

 /* UTF-8 sequences, generated with the Unicode.hs script from
  * http://lists.gnu.org/archive/html/help-flex/2005-01/msg00043.html */

 /* \u0024, \u0041-\u005a, \u005f, \u0061-\u007a: one byte in UTF-8 */
Letter1         [A-Za-z_$]
 /* \u00c0-\u00d6, \u00d8-\u00f6, \u00f8-\u00ff */
Letter2         [\xC3]([\x80-\x96]|[\x98-\xB6]|[\xB8-\xBF])
 /* \u0100-\u1fff */
Letter3         [\xC4-\xDF][\x80-\xBF]|([\xE0][\xA0-\xBF]|[\xE1][\x80-\xBF])[\x80-\xBF]
 /* \u3040-\u318f */
Letter4         [\xE3]([\x86][\x80-\x8F]|[\x81-\x85][\x80-\xBF])
 /* \u3300-\u337f */
Letter5         [\xE3][\x8C-\x8D][\x80-\xBF]
 /* \u3400-\u3d2d */
Letter6         [\xE3](\xB4[\x80-\xAD]|[\x90-\xB3][\x80-\xBF])
 /* \u4e00-\u9fff */
Letter7         ([\xE4][\xB8-\xBF]|[\xE5-\xE9][\x80-\xBF])[\x80-\xBF]
 /* \uf900-\ufaff */
Letter8         [\xEF][\xA4-\xAB][\x80-\xBF]




 /* \u0030-\u0039: ISO-LATIN-1 digits */
Digit1          [0-9]
 /* \u0660-\u0669, \u06f0-\u06f9: Arabic-Indic and extended Ar.-Indic digits */
Digit2          [\xD9][\xA0-\xA9]|[\xDB][\xB0-\xB9]
 /* \u0966-\u096f, \u09e6-\u09ef: Devanagari digits */
Digit3          [\xE0]([\xA5]|[\xA7])[\xA6-\xAF]
 /* \u0a66-\u0a6f, \u0ae6-\u0aef */
Digit4          [\xE0]([\xA9]|[\xAB])[\xA6-\xAF]
 /* \u0b66-\u0b6f, \u0be7-\u0bef */
Digit5          [\xE0]([\xAD][\xA6-\xAF]|[\xAF][\xA7-\xAF])
 /* \u0c66-\u0c6f, \u0ce6-\u0cef, \u0d66-\u0d6f */
Digit6          [\xE0]([\xB1]|[\xB3]|[\xB5])[\xA6-\xAF]
 /* \u0e50-\u0e59, \u0ed0-\u0ed9 */
Digit7          [\xE0]([\xB9]|[\xBB])[\x90-\x99]
 /* \u1040-\u1049 */
Digit8          [\xE1][\x81][\x80-\x89]
 /* \uff10-\uff19: Fullwidth digits */
Digit9          [\xEF][\xBC][\x90-\x99]

 /* \u0080-\uffff */
Multibyte1      ([\xC2-\xDF]|[\xE0][\xA0-\xBF]|[\xE1-\xEF][\x80-\xBF])[\x80-\xBF]
 /* \u10000-\u1fffff */
Multibyte2      ([\xF0][\x90-\xBF]|[\xF1-\xF7][\x80-\xBF])[\x80-\xBF][\x80-\xBF]
 /* \u200000-\u3ffffff */
Multibyte3      ([\xF8][\x88-\xBF]|[\xF9-\xFB][\x80-\xBF])[\x80-\xBF][\x80-\xBF][\x80-\xBF]
 /* \u4000000-\u7fffffff */
Multibyte4      ([\xFC][\x84-\xBF]|[\xFD][\x80-\xBF])[\x80-\xBF][\x80-\xBF][\x80-\xBF]
 /* Any multi-byte Unicode character. Single-byte ones are just . in lex. */
Multibyte       {Multibyte1}|{Multibyte2}|{Multibyte3}|{Multibyte4}


ULetter         {Letter1}|{Letter2}|{Letter3}|{Letter4}|{Letter5}|{Letter6}|{Letter7}|{Letter8}
Digit           [0-9]
NonZeroDigit    [1-9]
OctalDigit      [0-7]
HexDigit        [0-9a-fA-F]
UDigit          {Digit1}|{Digit2}|{Digit3}|{Digit4}|{Digit5}|{Digit6}|{Digit7}|{Digit8}|{Digit9}

DecimalInteger  0|{NonZeroDigit}{Digit}*

ExponentPart    [Ee][+-]?{Digit}+

UnicodeEscapeSequence   "u"{HexDigit}{HexDigit}{HexDigit}{HexDigit}

IDStart         {ULetter}|"$"|"_"|\\{UnicodeEscapeSequence}

  /* TODO UnicodeCombiningMark, UnicodeConnectorPunctuation */
IDPart          {ULetter}|{UDigit}|"$"|"_"|\\{UnicodeEscapeSequence}

RegexFirstChar  [^\n\r\*\\/]|\\[^\n\r]
RegexChar       [^\n\r\\/]|\\[^\n\r]
RegexFlag       {IDPart}

    /*      "/@@PIR <some pir code> END@/"   (replace @ with *)    */
PirCode    "/**PIR"([^E]|E[^N]|EN[^D]|END[^\*]|END\*[^/])*"END*/"
SingleLineComment   "//"[^\n\r]*

    /*      "/@ <anything> @/" bu not starting with "/@@PIR"   (replace @ with *)    */
MultiLineComment    ("/*"([^\*]|\*[^P]|\*P[^I]|\*PI[^R])([^\*]|\*[^/])*"*/")|"/**/"|"/***/"|"/**P*/"|"/**PI*/"


HWS                 [ \t\v\f]|{SingleLineComment}|{MultiLineComment}
HWSNoSingleLineComment [ \t\v\f]|{MultiLineComment}
VWS                 [\n\r]
WS                  {HWS}|{VWS}

%s REGEX DIV

%%


 /*===================================================*/
 
  /*
   I had to do some ugly hacks to make automatic semicolon
   insertion work. [TODO]
 */
 
 
 
    {   /* that part of code is executed before each step. */

        /* I want to start in a REGEX startcondition. */
        if (YY_START == INITIAL) BEGIN(REGEX);
    }

"true"         { COUNT; BEGIN(DIV);   DEBUG("TRUE");          return(TRUE_); }
"false"        { COUNT; BEGIN(DIV);   DEBUG("FALSE");         return(FALSE_); }
"null"         { COUNT; BEGIN(DIV);   DEBUG("NULL");          return(NULL_); }
"instanceof"   { COUNT; BEGIN(REGEX); DEBUG("INSTANCEOF");    return(INSTANCEOF); }
"break"        { COUNT; BEGIN(REGEX); DEBUG("BREAK");         return(BREAK); }
"for"          { COUNT; BEGIN(REGEX); DEBUG("FOR");           return(FOR); }
"new"          { COUNT; BEGIN(REGEX); DEBUG("NEW");           return(NEW); }
"var"          { COUNT; BEGIN(REGEX); DEBUG("VAR");           return(VAR); }
"continue"     { COUNT; BEGIN(REGEX); DEBUG("CONTINUE");      return(CONTINUE); }
"function"     { COUNT; BEGIN(REGEX); DEBUG("FUNCTION");      return(FUNCTION); }
"return"       { COUNT; BEGIN(REGEX); DEBUG("RETURN");        return(RETURN); }
"void"         { COUNT; BEGIN(REGEX); DEBUG("VOID");          return(VOID); }
"delete"       { COUNT; BEGIN(REGEX); DEBUG("DELETE");        return(DELETE); }
"if"           { COUNT; BEGIN(REGEX); DEBUG("IF");            return(IF); }
"this"         { COUNT; BEGIN(DIV);   DEBUG("THIS");          return(THIS); }
"while"        { COUNT; BEGIN(REGEX); DEBUG("WHILE");         return(WHILE); }
"else"         { COUNT; BEGIN(REGEX); DEBUG("ELSE");          return(ELSE); }
"in"           { COUNT; BEGIN(REGEX); DEBUG("IN");            return(IN); }
"typeof"       { COUNT; BEGIN(REGEX); DEBUG("TYPEOF");        return(TYPEOF); }
"with"         { COUNT; BEGIN(REGEX); DEBUG("WITH");          return(WITH); }
"case"         { COUNT; BEGIN(REGEX); DEBUG("CASE");          return(CASE); }
"debugger"     { COUNT; BEGIN(REGEX); DEBUG("DEBUGGER");      return(DEBUGGER); }
"export"       { COUNT; BEGIN(REGEX); DEBUG("EXPORT");        return(EXPORT); }
"super"        { COUNT; BEGIN(DIV);   DEBUG("SUPER");         return(SUPER); }
"catch"        { COUNT; BEGIN(REGEX); DEBUG("CATCH");         return(CATCH); }
"default"      { COUNT; BEGIN(REGEX); DEBUG("DEFAULT");       return(DEFAULT); }
"extends"      { COUNT; BEGIN(REGEX); DEBUG("EXTENDS");       return(EXTENDS); }
"switch"       { COUNT; BEGIN(REGEX); DEBUG("SWITCH");        return(SWITCH); }
"class"        { COUNT; BEGIN(REGEX); DEBUG("CLASS");         return(CLASS); }
"do"           { COUNT; BEGIN(REGEX); DEBUG("DO");            return(DO); }
"final"        { COUNT; BEGIN(REGEX); DEBUG("FINAL");         return(FINAL); }
"finally"      { COUNT; BEGIN(REGEX); DEBUG("FINALLY");       return(FINALLY); }
"throw"        { COUNT; BEGIN(REGEX); DEBUG("THROW");         return(THROW); }
"const"        { COUNT; BEGIN(REGEX); DEBUG("CONST");         return(CONST); }
"enum"         { COUNT; BEGIN(REGEX); DEBUG("ENUM");          return(ENUM); }
 /*"import"       { COUNT; BEGIN(REGEX); DEBUG("IMPORT");        return(IMPORT); } */
"try"          { COUNT; BEGIN(REGEX); DEBUG("TRY");           return(TRY); }
"abstract"     { COUNT; BEGIN(REGEX); DEBUG("ABSTRACT");      return(ABSTRACT); }
"int"          { COUNT; BEGIN(REGEX); DEBUG("INT");           return(INT); }
"short"        { COUNT; BEGIN(REGEX); DEBUG("SHORT");         return(SHORT); }
"boolean"      { COUNT; BEGIN(REGEX); DEBUG("BOOLEAN");       return(BOOLEAN); }
"interface"    { COUNT; BEGIN(REGEX); DEBUG("INTERFACE");     return(INTERFACE); }
"static"       { COUNT; BEGIN(REGEX); DEBUG("STATIC");        return(STATIC); }
"byte"         { COUNT; BEGIN(REGEX); DEBUG("BYTE");          return(BYTE); }
"long"         { COUNT; BEGIN(REGEX); DEBUG("LONG");          return(LONG); }
"char"         { COUNT; BEGIN(REGEX); DEBUG("CHAR");          return(CHAR); }
"native"       { COUNT; BEGIN(REGEX); DEBUG("NATIVE");        return(NATIVE); }
"synchronized" { COUNT; BEGIN(REGEX); DEBUG("SYNCHRONIZED");  return(SYNCHRONIZED); }
"float"        { COUNT; BEGIN(REGEX); DEBUG("FLOAT");         return(FLOAT); }
"package"      { COUNT; BEGIN(REGEX); DEBUG("PACKAGE");       return(PACKAGE); }
"throws"       { COUNT; BEGIN(REGEX); DEBUG("THROWS");        return(THROWS); }
"goto"         { COUNT; BEGIN(REGEX); DEBUG("GOTO");          return(GOTO); }
"private"      { COUNT; BEGIN(REGEX); DEBUG("PRIVATE");       return(PRIVATE); }
"transient"    { COUNT; BEGIN(REGEX); DEBUG("TRANSIENT");     return(TRANSIENT); }
"implements"   { COUNT; BEGIN(REGEX); DEBUG("IMPLEMENTS");    return(IMPLEMENTS); }
"protected"    { COUNT; BEGIN(REGEX); DEBUG("PROTECTED");     return(PROTECTED); }
"volatile"     { COUNT; BEGIN(REGEX); DEBUG("VOLATILE");      return(VOLATILE); }
"double"       { COUNT; BEGIN(REGEX); DEBUG("DOUBLE");        return(DOUBLE); }
"public"       { COUNT; BEGIN(REGEX); DEBUG("PUBLIC");        return(PUBLIC); }

">>>"          { COUNT; BEGIN(REGEX); DEBUG("UNSIGNED_RIGHT");  return(UNSIGNED_RIGHT); }
">>>="         { COUNT; BEGIN(REGEX); DEBUG("UNSIGNED_RIGHT_ASSIGN"); return(UNSIGNED_RIGHT_ASSIGN); }
">>="          { COUNT; BEGIN(REGEX); DEBUG("RIGHT_ASSIGN");  return(RIGHT_ASSIGN); }
"<<="          { COUNT; BEGIN(REGEX); DEBUG("LEFT_ASSIGN");   return(LEFT_ASSIGN); }
"+="           { COUNT; BEGIN(REGEX); DEBUG("ADD_ASSIGN");    return(ADD_ASSIGN); }
"-="           { COUNT; BEGIN(REGEX); DEBUG("SUB_ASSIGN");    return(SUB_ASSIGN); }
"*="           { COUNT; BEGIN(REGEX); DEBUG("MUL_ASSIGN");    return(MUL_ASSIGN); }
"%="           { COUNT; BEGIN(REGEX); DEBUG("MOD_ASSIGN");    return(MOD_ASSIGN); }
"&="           { COUNT; BEGIN(REGEX); DEBUG("AND_ASSIGN");    return(AND_ASSIGN); }
"^="           { COUNT; BEGIN(REGEX); DEBUG("XOR_ASSIGN");    return(XOR_ASSIGN); }
"|="           { COUNT; BEGIN(REGEX); DEBUG("OR_ASSIGN");     return(OR_ASSIGN); }
">>"           { COUNT; BEGIN(REGEX); DEBUG("RIGHT_OP");      return(RIGHT_OP); }
"<<"           { COUNT; BEGIN(REGEX); DEBUG("LEFT_OP");       return(LEFT_OP); }
"++"           { COUNT; BEGIN(REGEX); DEBUG("INC_OP");        return(INC_OP); }
"--"           { COUNT; BEGIN(REGEX); DEBUG("DEC_OP");        return(DEC_OP); }
"&&"           { COUNT; BEGIN(REGEX); DEBUG("AND_OP");        return(AND_OP); }
"||"           { COUNT; BEGIN(REGEX); DEBUG("OR_OP");         return(OR_OP); }
"<="           { COUNT; BEGIN(REGEX); DEBUG("LE_OP");         return(LE_OP); }
">="           { COUNT; BEGIN(REGEX); DEBUG("GE_OP");         return(GE_OP); }
"==="          { COUNT; BEGIN(REGEX); DEBUG("STRICT_EQ_OP");  return(STRICT_EQ_OP); }
"=="           { COUNT; BEGIN(REGEX); DEBUG("EQ_OP");         return(EQ_OP); }
"!=="          { COUNT; BEGIN(REGEX); DEBUG("STRICT_NE_OP");  return(STRICT_NE_OP); }
"!="           { COUNT; BEGIN(REGEX); DEBUG("NE_OP");         return(NE_OP); }

<DIV>"/="           { COUNT; BEGIN(REGEX); DEBUG("DIV_ASSIGN");    return(DIV_ASSIGN); }

";"            { COUNT; BEGIN(REGEX); DEBUG(";"); return(';'); }
"{"            { COUNT; BEGIN(REGEX); DEBUG("{"); return('{'); }
"}"            { COUNT; BEGIN(REGEX); DEBUG("}"); return('}'); }
","            { COUNT; BEGIN(REGEX); DEBUG(","); return(','); }
":"            { COUNT; BEGIN(REGEX); DEBUG(":"); return(':'); }
"="            { COUNT; BEGIN(REGEX); DEBUG("="); return('='); }
"("            { COUNT; BEGIN(REGEX); DEBUG("("); return('('); }
")"            { COUNT; BEGIN(DIV);   DEBUG(")"); return(')'); }
"["            { COUNT; BEGIN(REGEX); DEBUG("["); return('['); }
"]"            { COUNT; BEGIN(DIV);   DEBUG("]"); return(']'); }
"."            { COUNT; BEGIN(REGEX); DEBUG("."); return('.'); }
"&"            { COUNT; BEGIN(REGEX); DEBUG("&"); return('&'); }
"!"            { COUNT; BEGIN(REGEX); DEBUG("!"); return('!'); }
"~"            { COUNT; BEGIN(REGEX); DEBUG("~"); return('~'); }
"-"            { COUNT; BEGIN(REGEX); DEBUG("-"); return('-'); }
"+"            { COUNT; BEGIN(REGEX); DEBUG("+"); return('+'); }
"*"            { COUNT; BEGIN(REGEX); DEBUG("*"); return('*'); }
"%"            { COUNT; BEGIN(REGEX); DEBUG("%"); return('%'); }
"<"            { COUNT; BEGIN(REGEX); DEBUG("<"); return('<'); }
">"            { COUNT; BEGIN(REGEX); DEBUG(">"); return('>'); }
"^"            { COUNT; BEGIN(REGEX); DEBUG("^"); return('^'); }
"|"            { COUNT; BEGIN(REGEX); DEBUG("|"); return('|'); }
"?"            { COUNT; BEGIN(REGEX); DEBUG("?"); return('?'); }

<DIV>"/"       { COUNT; BEGIN(REGEX); DEBUG("/"); return('/'); }

{IDStart}{IDPart}*  {
        COUNT;
        BEGIN(DIV); 
        DEBUG("IDENTIFIER");
        yylval->str = yytext;
        yyextra->match_len = yyleng;
        return(IDENTIFIER);
    }

0[xX]{HexDigit}+  {
        COUNT;
        BEGIN(DIV); 
        DEBUG("NUMERIC_LITERAL");
        yylval->str = yytext;
        yyextra->match_len = yyleng;
        return(NUMERIC_LITERAL);
    }

{DecimalInteger}"."{Digit}*{ExponentPart}?  {
        COUNT;
        BEGIN(DIV); 
        DEBUG("NUMERIC_LITERAL");
        yylval->str = yytext;
        yyextra->match_len = yyleng;
        return(NUMERIC_LITERAL);
    }

"."{Digit}+{ExponentPart}? {
        COUNT;
        BEGIN(DIV); 
        DEBUG("NUMERIC_LITERAL");
        yylval->str = yytext;
        yyextra->match_len = yyleng;
        return(NUMERIC_LITERAL);
    }

{DecimalInteger}{ExponentPart}? {
        COUNT;
        BEGIN(DIV); 
        DEBUG("NUMERIC_LITERAL");
        yylval->str = yytext;
        yyextra->match_len = yyleng;
        return(NUMERIC_LITERAL);
    }

 /* octals not allowed */
0{OctalDigit}+  { return OCTAL_NUMBER_LITERAL; }
    
<REGEX>"/"{RegexFirstChar}{RegexChar}*"/"{RegexFlag}*  {
        COUNT;
        BEGIN(DIV); 
        DEBUG("REGEX_LITERAL_");
        yylval->str = yytext;
        yyextra->match_len = yyleng;
        return(REGEX_LITERAL);
    }


\"(\\.|[^\\"\r\n])*\"   {
        COUNT;
        BEGIN(DIV); 
        DEBUG("STRING_LITERAL");
        yylval->str = yytext;
        yyextra->match_len = yyleng;
        return(STRING_LITERAL);
    }

\'(\\.|[^\\'\r\n])*\'  {
        COUNT;
        BEGIN(DIV); 
        DEBUG("STRING_LITERAL");
        yylval->str = yytext;
        yyextra->match_len = yyleng;
        return(STRING_LITERAL);
    }


"return"{HWS}*{VWS} {
        COUNT;
        yyextra->is_newline = 
                yyextra->prev_was_newline = 1;
        BEGIN(REGEX); 
        DEBUG("RETURN_NL");
        return(RETURN_NL);
    }

"throw"{HWS}*{VWS}  {
        COUNT;
        yyextra->is_newline = 
                yyextra->prev_was_newline = 1;
        BEGIN(REGEX); 
        DEBUG("THROW_NL");
        return(THROW_NL);
    }

"break"{HWS}*{VWS}  {
        COUNT;
        yyextra->is_newline = 
                yyextra->prev_was_newline = 1;
        BEGIN(REGEX); 
        DEBUG("BREAK_NL");
        return(BREAK_NL);
    }

"continue"{HWS}*{VWS}  {
        COUNT;
        yyextra->is_newline = 
                yyextra->prev_was_newline = 1;
        BEGIN(REGEX); 
        DEBUG("CONTINUE_NL");
        return(CONTINUE_NL);
    }

{VWS}{HWSNoSingleLineComment}*"++"  {
        COUNT;
        yyextra->is_newline = 
                yyextra->prev_was_newline = 1;
        BEGIN(REGEX); 
        DEBUG("NL_INC_OP");
        return(NL_INC_OP);
    }

{VWS}{HWSNoSingleLineComment}*"--"  {
        COUNT;
        yyextra->is_newline = 
                yyextra->prev_was_newline = 1;
        BEGIN(REGEX); 
        DEBUG("NL_DEC_OP");
        return(NL_DEC_OP);
   }

{PirCode}       { 
        COUNT; DEBUG("PIR CODE"); 
        yylval->str = yytext;
        yyextra->match_len = yyleng;
        return PIR_CODE; 
    }

{MultiLineComment}  { COUNT; DEBUG("MULTI LINE COMMENT"); /* skip */ }
{SingleLineComment} { COUNT; DEBUG("SINGLE LINE COMMENT"); /* skip */ }

[ \t\v\f]           { 
        COUNT; 
        yyextra->is_newline = yyextra->prev_was_newline;
        /* skip */ 
    }
    
[\n\r]              { 
        COUNT; 
        yyextra->is_newline = 1; 
        /* skip */ 
    }

.   {   COUNT;
        fprintf(stderr, "Lexical error.\n");
        return(LEX_ERROR);
    }

%%

static void count(yyscan_t scanner, Params* params) {
    int i, line, col;
    char* text;
    char ch;

    params->prev_was_newline = params->is_newline;
    params->is_newline = 0;

    /* CALCULATE line_no, column_no */
    params->prev_line_no = params->line_no;
    params->prev_column_no = params->column_no;

    text = yyget_text(scanner);
    
    line = params->line_no;
    col = params->column_no;

    for (i = 0; (ch = text[i]) != '\0'; i++) {
        if (ch == '\r') {
            line++;
            col = 0;
            params->prev_was_r = 1;
        } else if (ch == '\n') {
            if (params->prev_was_r == 0) {
                line++;
                col = 0;
            }
            params->prev_was_r = 0;
        } else {
            col++;
            params->prev_was_r = 0;
        }
    }
    params->line_no = line;
    params->column_no = col;
}
