%pure-parser
%locations
%defines
%error-verbose
%parse-param { Params* params }
%lex-param { void* scanner }
%name-prefix="pjs_yy"

%{

#include "params.h"
#include "mempool.h"
#include "str_escaping.h"

#define YYERROR_VERBOSE 1

#define yyparams ((Params*)params)
#define MP (yyparams->mempool)

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "nodes.h"
#include "list.h"


#define scanner params->scanner
#define DBG(txt) fprintf(stderr, "  @@@ %s", (txt))
/*#define DBG */

#define YYDEBUG 1
#define yymatch_len (yyparams->match_len)


static void err_msg(Params* p, const char* s);
static int automatic_test(Params* params, int yychar);

%}

%union NODE {
    double num;
    char* str;
    Node expr;
    PjsList exprList;
    PjsList nameValueList;
    int tokType;
    Node stmt;
    PjsList stmtList;
    NameValue nameValue;
    PjsList params;
    CaseClause caseClause;
    PjsList caseList;
    NodePair pair;
    PjsList nodePairList;
}


%{
    void yyerror (YYLTYPE* locp, Params* params, const char* err);
    extern int pjs_yylex(YYSTYPE* lvalp, YYLTYPE* llocp, void* scanner_);
%}


%type <expr> identifier
%type <expr> primary_expr
%type <expr> member_expr
%type <expr> new_expr
%type <expr> call_expr
%type <expr> left_hand_side_expr
%type <expr> postfix_expr
%type <expr> unary_expr
%type <expr> multiplicative_expr
%type <expr> additive_expr
%type <expr> shift_expr
%type <expr> relational_expr
%type <expr> relational_expr_noin
%type <expr> equality_expr
%type <expr> equality_expr_noin
%type <expr> bitwise_and_expr
%type <expr> bitwise_and_expr_noin
%type <expr> bitwise_xor_expr
%type <expr> bitwise_xor_expr_noin
%type <expr> bitwise_or_expr
%type <expr> bitwise_or_expr_noin
%type <expr> logical_and_expr
%type <expr> logical_and_expr_noin
%type <expr> logical_or_expr
%type <expr> logical_or_expr_noin
%type <expr> conditional_expr
%type <expr> conditional_expr_noin
%type <expr> assignment_expr
%type <expr> assignment_expr_noin
%type <expr> expr
%type <expr> expr_noin
%type <expr> function_expr

%type <expr> initialiser initialiser_noin

%type <exprList> array_literal
%type <exprList> elision
%type <exprList> element_list
%type <exprList> arguments
%type <exprList> argument_list

%type <stmt> statement
%type <stmt> variable_statement
%type <stmt> empty_statement
%type <stmt> expression_statement; 
%type <stmt> if_statement
%type <stmt> iteration_statement
%type <stmt> continue_statement_no_semi
%type <stmt> continue_statement
%type <stmt> break_statement_no_semi
%type <stmt> break_statement
%type <stmt> return_statement_no_semi
%type <stmt> return_statement
%type <stmt> with_statement
%type <stmt> labelled_statement
%type <stmt> throw_statement
%type <stmt> try_statement
%type <stmt> catch finally
%type <stmt> function_declaration
%type <stmt> source_element
%type <stmt> switch_statement
%type <stmt> case_block

%type <stmtList> block_contents
%type <stmtList> statement_list
%type <stmtList> function_body
%type <stmtList> source_elements
%type <stmtList> program
%type <stmtList> default_clause

%type <params> formal_parameter_list

%type <nameValue> variable_declaration
%type <nameValue> variable_declaration_noin
%type <nameValueList> object_literal

/*%type <nameValueList> property_name_and_value_list */
%type <nodePairList> property_name_and_value_list

%type <nameValueList> variable_declaration_list
%type <nameValueList> variable_declaration_list_noin

%type <caseClause> case_clause
%type <caseList> case_clauses

/*%type <str> property_name */
%type <expr> property_name

%type <tokType> assignment_operator



%token <str> NUMERIC_LITERAL STRING_LITERAL IDENTIFIER OCTAL_NUMBER_LITERAL

/*-- ECMAScript Special - bool constants + pseudo stuff */
%token TRUE_ FALSE_ NULL_
%token REGEX_LITERAL
%token INC_OP DEC_OP LEFT_OP RIGHT_OP
%token LE_OP GE_OP EQ_OP NE_OP STRICT_EQ_OP STRICT_NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN

%token UNSIGNED_RIGHT UNSIGNED_RIGHT_ASSIGN


/*-- ECMAScript Keywords */
%token BREAK FOR NEW VAR CONTINUE FUNCTION RETURN VOID
%token DELETE IF THIS WHILE ELSE IN TYPEOF WITH

/*-- ECMAScript Reserved Words */
%token CASE DEBUGGER EXPORT SUPER CATCH DEFAULT EXTENDS SWITCH
%token CLASS DO FINALLY THROW CONST ENUM IMPORT TRY

%token INSTANCEOF FINAL ABSTRACT INT SHORT BOOLEAN INTERFACE
%token STATIC BYTE LONG CHAR NATIVE SYNCHRONIZED FLOAT PACKAGE_
%token THROWS GOTO PRIVATE TRANSIENT IMPLEMENTS
%token PROTECTED VOLATILE DOUBLE PUBLIC

%token RETURN_NL THROW_NL BREAK_NL CONTINUE_NL NL_INC_OP NL_DEC_OP

%token PIR_CODE

%token END LEX_ERROR

 /* The next are only used as enums for
    the AST, not for tokenizing. */
%token PRE_INC_OP PRE_DEC_OP POST_INC_OP POST_DEC_OP


%start program
%%

 /*==========  EXPRESSIONS  =============================*/
 
identifier
    : IDENTIFIER
        {{  $$ = newNode(EIdentifier, MP);
            $$->u.strVal = dupl(yylval.str, yymatch_len, MP);
        }}
    ;

primary_expr
    : THIS
        {{  $$ = newNode(EThis, MP); 
        }}
    | identifier
        {{  $$ = $1; 
        }}
    | NULL_
        {{  $$ = newNode(ENull, MP); 
        }}
    | TRUE_
        {{  $$ = newNode(EBoolean, MP);
            $$->u.boolVal = 1;
        }}
    | FALSE_
        {{  $$ = newNode(EBoolean, MP);
            $$->u.boolVal = 0;
        }}
    | NUMERIC_LITERAL
        {{  $$ = newNumberExpr(dupl(yylval.str, yymatch_len, MP), MP);
        }}
    | STRING_LITERAL
        {{  int err = 0;
            $$ = newNode(EString, MP);
            $$->u.strVal = unescape_js_string(yylval.str, yymatch_len, &err);
            if (err) {
                err_msg(params, "Can't parse string literal.");
                YYABORT;
            } else {
                mp_register($$->u.strVal, MP);
            }
        }}
    | array_literal
        {{  $$ = newNode(EArrayLit, MP); 
            $$->u.exprList = $1; 
        }}
    | object_literal
        {{  $$ = newNode(EObjectLit, MP); 
            /*$$->u.nameValueList = $1;  */
            $$->u.nodePairList = $1; 
        }}
    | '(' expr ')'
        {{  $$ = $2; 
        }}
    | REGEX_LITERAL
        {{
            $$ = newRegexExpr(dupl(yylval.str, yymatch_len, MP), MP);
        }}
    | '(' expr error
        {{  err_msg(params, "Unclosed paren.");  YYABORT;  }}
    | '(' error
        {{  err_msg(params, "Expected an expression.");  YYABORT; }}
    ;

array_literal
    : '[' ']'
        {{  $$ = newList(MP);
        }}
    | '[' elision ']'
        {{  $$ = $2;
        }}
    | '[' element_list ']'
        {{  $$ = $2;
        }}
    | '[' element_list ',' ']'
        {{  $$ = $2;
        }}
    | '[' element_list ',' elision ']'
        {{  $$ = appendList($2, $4);
            /*free($4); freed from mempool */
        }}
    ;

element_list
    : assignment_expr
        {{  $$ = newList(MP);
            pushBack($1, $$, MP);
        }}
    | elision assignment_expr
        {{  $$ =$1;
            pushBack($2, $$, MP);
        }}
    | element_list ',' assignment_expr
        {{  $$ = $1;
            pushBack($3, $$, MP);
        }}
    | element_list ',' elision assignment_expr
        {{  $$ = appendList($1, $3);
            pushBack($4, $$, MP);
            /*free($3); freed from mempool */
        }}
    ;

elision
    : ',' 
        {{  $$ = newList(MP);
            pushBack(0, $$, MP);
        }}
    | elision ',' 
        {{  $$ = $1;
            pushBack(0, $$, MP);
        }}
    ;

object_literal
    : '{' '}'
        {{  $$ = newList(MP); }}
    | '{' property_name_and_value_list '}'
        {{  $$ = $2; }}
    ;

property_name_and_value_list
    : identifier ':' assignment_expr
        {{  $$ = newList(MP);
            /*pushBack(newNameValue($1->u.strVal, $3, MP), $$, MP); */
            pushBack(newNodePair($1, $3, MP), $$, MP);
        }}
    | property_name ':' assignment_expr
        {{  $$ = newList(MP);
            pushBack(newNodePair($1, $3, MP), $$, MP);
        }}
    | property_name_and_value_list ',' identifier ':' assignment_expr
        {{  $$ = $1;
            pushBack(newNodePair($3, $5, MP), $$, MP);
        }}
    | property_name_and_value_list ',' property_name ':' assignment_expr
        {{  $$ = $1;
            pushBack(newNodePair($3, $5, MP), $$, MP);
        }}
    ;

property_name
    : STRING_LITERAL
        {{  int err = 0;
            $$ = newNode(EString, MP);
            $$->u.strVal = unescape_js_string(yylval.str, yymatch_len, &err);
            if (err) {
                err_msg(params, "Can't parse string literal.");
                YYABORT;
            } else {
                mp_register($$->u.strVal, MP);
            }
        }}
    | NUMERIC_LITERAL
        {{  $$ = newNumberExpr(dupl(yylval.str, yymatch_len, MP), MP);
        }}
    ;

member_expr
    : primary_expr
        {{  $$ = $1; }}
    | function_expr
        {{  $$ = $1; }}
    | member_expr '[' expr ']'
        {{  $$ = newNode(EIndexedAccess, MP);
            $$->u.getindexed.base = $1;
            $$->u.getindexed.index = $3;
        }}
    | member_expr '.' IDENTIFIER
        {{  char* id = mp_malloc(yymatch_len + 1, MP);
            strcpy(id, yylval.str);
            $$ = newNode(EPropAccess, MP);
            $$->u.getprop.base = $1;
            $$->u.getprop.propName = id;
        }}
    | NEW member_expr arguments
        {{  $$ = newNode(ENewFuncall, MP);
            $$->u.funcall.fun = $2;
            $$->u.funcall.args = $3;
        }}
    ;

new_expr
    : member_expr
        {{  $$ = $1; }}
    | NEW new_expr
        {{  $$ = newNode(ENewFuncall, MP);
            $$->u.funcall.fun = $2;
            $$->u.funcall.args = newList(MP);
        }}
    ;

call_expr
    : member_expr arguments
        {{  $$ = newNode(EFuncall, MP);
            $$->u.funcall.fun = $1;
            $$->u.funcall.args = $2;
        }}
    | call_expr arguments
        {{  $$ = newNode(EFuncall, MP);
            $$->u.funcall.fun = $1;
            $$->u.funcall.args = $2;
        }}
    | call_expr '[' expr ']'
        {{  $$ = newNode(EIndexedAccess, MP);
            $$->u.getindexed.base = $1;
            $$->u.getindexed.index = $3;
        }}
    | call_expr '.' IDENTIFIER
        {{  char* id = mp_malloc(yymatch_len + 1, MP);
            strcpy(id, yylval.str);
            $$ = newNode(EPropAccess, MP);
            $$->u.getprop.base = $1;
            $$->u.getprop.propName = id;
        }}
    ;

arguments
    : '(' ')'
        {{  $$ = newList(MP); }}
    | '(' argument_list ')'
        {{  $$ = $2; }}
    |
    '(' error {
        err_msg(params, "Unclosed paren.");
        YYABORT;
    }
    |
     '(' argument_list error {
        err_msg(params, "Unclosed paren.");
        YYABORT;
    }
    ;

argument_list
    : assignment_expr
        {{  $$ = newList(MP);
            pushBack($1, $$, MP);
        }}
    | argument_list ',' assignment_expr
        {{  $$ = $1;
            pushBack($3, $$, MP);
        }}
    ;

left_hand_side_expr
    : new_expr
        {{  $$ = $1; }}
    | call_expr
        {{  $$ = $1; }}
    ;

postfix_expr
    : left_hand_side_expr
        {{  $$ = $1; }}
    | left_hand_side_expr INC_OP
        {{  $$ = newUnop(POST_INC_OP, $1, MP);
            if (! $$) YYABORT;
        }}
    | left_hand_side_expr DEC_OP
        {{  $$ = newUnop(POST_DEC_OP, $1, MP);
            if (! $$) YYABORT;
        }}
    ;

unary_expr
    : postfix_expr
        {{  $$ = $1; }}
    | DELETE unary_expr
        {{  $$ = newUnop(DELETE, $2, MP);
            if (! $$) YYABORT;
        }}
    | VOID unary_expr
        {{  $$ = newUnop(VOID, $2, MP);
            if (! $$) YYABORT;
        }}
    | TYPEOF unary_expr
        {{  $$ = newUnop(TYPEOF, $2, MP);
            if (! $$) YYABORT;
        }}
    | INC_OP unary_expr
        {{  $$ = newUnop(PRE_INC_OP, $2, MP);
            if (! $$) YYABORT;
        }}
    | NL_INC_OP unary_expr
        {{  $$ = newUnop(PRE_INC_OP, $2, MP);
            if (! $$) YYABORT;
        }}
    | DEC_OP unary_expr
        {{  $$ = newUnop(PRE_DEC_OP, $2, MP);
            if (! $$) YYABORT;
        }}
    | NL_DEC_OP unary_expr
        {{  $$ = newUnop(PRE_DEC_OP, $2, MP);
            if (! $$) YYABORT;
        }}
    | '+' unary_expr
        {{  $$ = newUnop('+', $2, MP);
            if (! $$) YYABORT;
        }}
    | '-' unary_expr
        {{  $$ = newUnop('-', $2, MP);
            if (! $$) YYABORT;
        }}
    | '~' unary_expr
        {{  $$ = newUnop('~', $2, MP);
            if (! $$) YYABORT;
        }}
    | '!' unary_expr
        {{  $$ = newUnop('!', $2, MP);
            if (! $$) YYABORT;
        }}
    ;

multiplicative_expr
    : unary_expr
        {{  $$ = $1; }}
    | multiplicative_expr '*' unary_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '*';
        }}
    | multiplicative_expr '/' unary_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '/';
        }}
    | multiplicative_expr '%' unary_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '%';
        }}
    ;

additive_expr
    : multiplicative_expr
        {{  $$ = $1; }}
    | additive_expr '+' multiplicative_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '+';
        }}
    | additive_expr '-' multiplicative_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '-';
        }}
    ;

shift_expr
    : additive_expr
        {{  $$ = $1; }}
    | shift_expr LEFT_OP additive_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = LEFT_OP;
        }}
    | shift_expr RIGHT_OP additive_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = RIGHT_OP;
        }}
    | shift_expr UNSIGNED_RIGHT additive_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = UNSIGNED_RIGHT;
        }}
    ;

relational_expr
    : shift_expr
        {{  $$ = $1; }}
    | relational_expr '<' shift_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '<';
        }}
    | relational_expr '>' shift_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '>';
        }}
    | relational_expr LE_OP shift_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = LE_OP;
        }}
    | relational_expr GE_OP shift_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = GE_OP;
        }}
    | relational_expr IN shift_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = IN;
        }}
    | relational_expr INSTANCEOF shift_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = INSTANCEOF;
        }}
    ;

relational_expr_noin
    : shift_expr
        {{  $$ = $1; }}
    | relational_expr '<' shift_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '<';
        }}
    | relational_expr '>' shift_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '>';
        }}
    | relational_expr LE_OP shift_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = LE_OP;
        }}
    | relational_expr GE_OP shift_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = GE_OP;
        }}
    | relational_expr INSTANCEOF shift_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = INSTANCEOF;
        }}
    ;

equality_expr
    : relational_expr
        {{  $$ = $1; }}
    | equality_expr EQ_OP relational_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = EQ_OP;
        }}
    | equality_expr NE_OP relational_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = NE_OP;
        }}
    | equality_expr STRICT_EQ_OP relational_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = STRICT_EQ_OP;
        }}
    | equality_expr STRICT_NE_OP relational_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = STRICT_NE_OP;
        }}
    ;

equality_expr_noin
    : relational_expr_noin
        {{  $$ = $1; }}
    | equality_expr_noin EQ_OP relational_expr_noin
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = EQ_OP;
        }}
    | equality_expr_noin NE_OP relational_expr_noin
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = NE_OP;
        }}
    | equality_expr_noin STRICT_EQ_OP relational_expr_noin
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = STRICT_EQ_OP;
        }}
    | equality_expr_noin STRICT_NE_OP relational_expr_noin
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = STRICT_NE_OP;
        }}
    ;

bitwise_and_expr
    : equality_expr
        {{  $$ = $1; }}
    | bitwise_and_expr '&' equality_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '&';
        }}
    ;

bitwise_and_expr_noin
    : equality_expr_noin
        {{  $$ = $1; }}
    | bitwise_and_expr_noin '&' equality_expr_noin
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '&';
        }}
    ;

bitwise_xor_expr
    : bitwise_and_expr
        {{  $$ = $1; }}
    | bitwise_xor_expr '^' bitwise_and_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '^';
        }}
    ;

bitwise_xor_expr_noin
    : bitwise_and_expr_noin
        {{  $$ = $1; }}
    | bitwise_xor_expr_noin '^' bitwise_and_expr_noin
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '^';
        }}
    ;

bitwise_or_expr
    : bitwise_xor_expr
        {{  $$ = $1; }}
    | bitwise_or_expr '|' bitwise_xor_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '|';
        }}
    ;

bitwise_or_expr_noin
    : bitwise_xor_expr_noin
        {{  $$ = $1; }}
    | bitwise_or_expr_noin '|' bitwise_xor_expr_noin
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = '|';
        }}
    ;

logical_and_expr
    : bitwise_or_expr
        {{  $$ = $1; }}
    | logical_and_expr AND_OP bitwise_or_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = AND_OP;
        }}
    ;

logical_and_expr_noin
    : bitwise_or_expr_noin
        {{  $$ = $1; }}
    | logical_and_expr_noin AND_OP bitwise_or_expr_noin
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = AND_OP;
        }}
    ;

logical_or_expr
    : logical_and_expr
        {{  $$ = $1; }}
    | logical_or_expr OR_OP logical_and_expr
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = OR_OP;
        }}
    ;

logical_or_expr_noin
    : logical_and_expr_noin
        {{  $$ = $1; }}
    | logical_or_expr_noin OR_OP logical_and_expr_noin
        {{  $$ = newNode(EBinop, MP);
            $$->u.binop.left = $1;
            $$->u.binop.right = $3;
            $$->u.binop.op = OR_OP;
        }}
    ;

conditional_expr
    : logical_or_expr
        {{  $$ = $1; }}
    | logical_or_expr '?' assignment_expr ':' assignment_expr
        {{  $$ = newNode(ETernop, MP);
            $$->u.ternop.cond = $1;
            $$->u.ternop.ifTrue = $3;
            $$->u.ternop.ifFalse = $5;
        }}
    ;

conditional_expr_noin
    : logical_or_expr_noin
        {{  $$ = $1; }}
    | logical_or_expr_noin '?' assignment_expr ':' assignment_expr
        {{  $$ = newNode(ETernop, MP);
            $$->u.ternop.cond = $1;
            $$->u.ternop.ifTrue = $3;
            $$->u.ternop.ifFalse = $5;
        }}
    ;

assignment_expr
    : conditional_expr
        {{  $$ = $1; }}
    | left_hand_side_expr assignment_operator assignment_expr
        {{  $$ = newNode(EAssign, MP);
            $$->u.assign.left = $1;
            $$->u.assign.type = $2;
            $$->u.assign.right = $3;
        }}
    ;

assignment_expr_noin
    : conditional_expr_noin
        {{  $$ = $1; }}
    | left_hand_side_expr assignment_operator assignment_expr_noin
        {{  $$ = newNode(EAssign, MP);
            $$->u.assign.left = $1;
            $$->u.assign.type = $2;
            $$->u.assign.right = $3;
        }}
    ;

assignment_operator
    : '='
        {{  $$ = '='; }}
    | MUL_ASSIGN
        {{  $$ = '*'; }}
    | DIV_ASSIGN
        {{  $$ = '/'; }}
    | MOD_ASSIGN
        {{  $$ = '%'; }}
    | ADD_ASSIGN
        {{  $$ = '+'; }}
    | SUB_ASSIGN
        {{  $$ = '-'; }}
    | LEFT_ASSIGN
        {{  $$ = LEFT_OP; }}
    | RIGHT_ASSIGN
        {{  $$ = RIGHT_OP; }}
    | AND_ASSIGN
        {{  $$ = '&'; }}
    | XOR_ASSIGN
        {{  $$ = '^'; }}
    | OR_ASSIGN
        {{  $$ = '|'; }}
    | UNSIGNED_RIGHT_ASSIGN
        {{  $$ = UNSIGNED_RIGHT; }}
    ;

expr
    : assignment_expr
        {{  $$ = $1; 
        }}
    | expr ',' assignment_expr
        {{  $$ = newNode(ECompound, MP);
            $$->u.exprPair.left = $1;
            $$->u.exprPair.right = $3;
        }}
    ;

expr_noin
    : assignment_expr_noin
        {{  $$ = $1;
        }}
    | expr_noin ',' assignment_expr_noin
        {{  $$ = newNode(ECompound, MP);
            $$->u.exprPair.left = $1;
            $$->u.exprPair.right = $3;
        }}
    ;


 /*==========  STATEMENTS  =============================*/

statement
    : variable_statement
        {{  $$ = $1; }}
    | empty_statement
        {{  $$ = $1; }}
    | '{' '}' 
        {{  $$ = newNode(EBlock_stm, MP);
            $$->u.stmtList = newList(MP); 
        }}
    | '{' statement_list '}'
        {{  $$ = newNode(EBlock_stm, MP);
            $$->u.stmtList = $2;
        }}
    | expression_statement
        {{  $$ = $1; }}
    | if_statement
        {{  $$ = $1; }}
    | iteration_statement
        {{  $$ = $1; }}
    | continue_statement
        {{  $$ = $1; }}
    | break_statement
        {{  $$ = $1; }}
    | return_statement
        {{  $$ = $1; }}
    | with_statement
        {{  $$ = $1; }}
    | labelled_statement
        {{  $$ = $1; }}
    | switch_statement
        {{  $$ = $1; }}
    | throw_statement
        {{  $$ = $1; }}
    | try_statement
        {{  $$ = $1; }}
    | PIR_CODE
        {{  char* start = yylval.str;
            char* end = start + yymatch_len;
            while (start < end && *start != '\n')
                start++;
            start++;
            while (end > start && *end != '\n')
                end--;
            
            $$ = newNode(EPirCode_stm, MP);
            if (start < end) {
                $$->u.strVal = dupl(start, end-start, MP);
            } else {
                $$->u.strVal = dupl("", 0, MP);
            }
        }}
    ;


block_contents
    : /* empty */
        {{  $$ = newList(MP); }}
    | statement_list
        {{  $$ = $1; }}
    ;

statement_list
    : statement
        {{  $$ = newList(MP);
            pushBack($1, $$, MP);
        }}
    | statement_list statement
        {{  $$ = $1;
            pushBack($2, $$, MP);
        }}
    ;

variable_statement
    : VAR variable_declaration_list ';'
        {{  $$ = newNode(EVardec_stm, MP);
            $$->u.nameValueList = $2;
        }}
    | VAR variable_declaration_list error
        {{  if(! automatic_test(yyparams, yychar)) YYABORT;
            $$ = newNode(EVardec_stm, MP);
            $$->u.nameValueList = $2;
        }}
    ;

variable_declaration_list
    : variable_declaration
        {{  $$ = newList(MP);
            pushBack($1, $$, MP);
        }}
    | variable_declaration_list ',' variable_declaration
        {{  pushBack($3, $1, MP);
        }}
    ;

variable_declaration_list_noin
    : variable_declaration_noin
        {{  $$ = newList(MP);
            pushBack($1, $$, MP);
        }}
    | variable_declaration_list_noin ',' variable_declaration_noin
        {{  pushBack($3, $1, MP);
        }}
    ;

variable_declaration
    : identifier
        {{  $$ = newNameValue($1->u.strVal, 0, MP);
            /*free($1); freed from mempool */
        }}
    | identifier initialiser
        {{  $$ = newNameValue($1->u.strVal, $2, MP);
            /*free($1); freed from mempool */
        }}
    ;

variable_declaration_noin
    : identifier
        {{  $$ = newNameValue($1->u.strVal, 0, MP);
            /*free($1); freed from mempool */
        }}
    | identifier initialiser_noin
        {{  $$ = newNameValue($1->u.strVal, $2, MP);
            /*free($1); freed from mempool */
        }}
    ;

initialiser
    : '=' assignment_expr
        {{ $$ = $2; }}
    ;

initialiser_noin
    : '=' assignment_expr_noin
        {{ $$ = $2; }}
    ;

empty_statement
    : ';'
        {{  $$ = newNode(EBlock_stm, MP);
            $$->u.stmtList = newList(MP);
        }}
    ;

 /* lookahead?*/
expression_statement
    : expr ';'
        {{ $$ = $1; }}
    | expr error
        {{  if(! automatic_test(yyparams, yychar)) YYABORT;
            $$ = $1;
        }}
    ;

if_statement
    : IF '(' expr ')' statement
        {{  $$ = newNode(EIfElse_stm, MP);
            $$->u.ifElse.cond = $3;
            $$->u.ifElse.ifTrue = $5;
            $$->u.ifElse.ifFalse = 0;
        }}
    | IF '(' expr ')' statement ELSE statement
        {{  $$ = newNode(EIfElse_stm, MP);
            $$->u.ifElse.cond = $3;
            $$->u.ifElse.ifTrue = $5;
            $$->u.ifElse.ifFalse = $7;
        }}
    ;

iteration_statement
    : WHILE '(' expr ')' statement
        {{  $$ = newNode(EWhile_stm, MP);
            $$->u.whileLoop.cond = $3;
            $$->u.whileLoop.stmt = $5;
        }}
    | DO statement WHILE '(' expr ')' ';'
        {{  $$ = newNode(EDoWhile_stm, MP);
            $$->u.whileLoop.cond = $5;
            $$->u.whileLoop.stmt = $2;
        }}
    | FOR '(' ';' ';' ')' statement
        {{  $$ = newNode(EFor_stm, MP);
            $$->u.forLoop.init = 0;
            $$->u.forLoop.cond = 0;
            $$->u.forLoop.next = 0;
            $$->u.forLoop.stmt = $6;
        }}
    | FOR '(' ';' ';' expr ')' statement
        {{  $$ = newNode(EFor_stm, MP);
            $$->u.forLoop.init = 0;
            $$->u.forLoop.cond = 0;
            $$->u.forLoop.next = $5;
            $$->u.forLoop.stmt = $7;
        }}
    | FOR '(' ';' expr ';' ')' statement
        {{  $$ = newNode(EFor_stm, MP);
            $$->u.forLoop.init = 0;
            $$->u.forLoop.cond = $4;
            $$->u.forLoop.next = 0;
            $$->u.forLoop.stmt = $7;
        }}
    | FOR '(' ';' expr ';' expr ')' statement
        {{  $$ = newNode(EFor_stm, MP);
            $$->u.forLoop.init = 0;
            $$->u.forLoop.cond = $4;
            $$->u.forLoop.next = $6;
            $$->u.forLoop.stmt = $8;
        }}
    | FOR '(' expr_noin ';' ';' ')' statement
        {{  $$ = newNode(EFor_stm, MP);
            $$->u.forLoop.init = $3;
            $$->u.forLoop.cond = 0;
            $$->u.forLoop.next = 0;
            $$->u.forLoop.stmt = $7;
        }}
    | FOR '(' expr_noin ';' ';' expr ')' statement
        {{  $$ = newNode(EFor_stm, MP);
            $$->u.forLoop.init = $3;
            $$->u.forLoop.cond = 0;
            $$->u.forLoop.next = $6;
            $$->u.forLoop.stmt = $8;
        }}
    | FOR '(' expr_noin ';' expr ';' ')' statement
        {{  $$ = newNode(EFor_stm, MP);
            $$->u.forLoop.init = $3;
            $$->u.forLoop.cond = $5;
            $$->u.forLoop.next = 0;
            $$->u.forLoop.stmt = $8;
        }}
    | FOR '(' expr_noin ';' expr ';' expr ')' statement
        {{  $$ = newNode(EFor_stm, MP);
            $$->u.forLoop.init = $3;
            $$->u.forLoop.cond = $5;
            $$->u.forLoop.next = $7;
            $$->u.forLoop.stmt = $9;
        }}
    | FOR '(' VAR variable_declaration_list_noin ';'  ';'  ')' statement
        {{  Node varDec = newNode(EVardec_stm, MP);
            varDec->u.nameValueList = $4;
            $$ = newNode(EFor_stm, MP);
            $$->u.forLoop.init = varDec;
            $$->u.forLoop.cond = 0;
            $$->u.forLoop.next = 0;
            $$->u.forLoop.stmt = $8;
        }}
    | FOR '(' VAR variable_declaration_list_noin ';'  ';' expr ')' statement
        {{  Node varDec = newNode(EVardec_stm, MP);
            varDec->u.nameValueList = $4;
            $$ = newNode(EFor_stm, MP);
            $$->u.forLoop.init = varDec;
            $$->u.forLoop.cond = 0;
            $$->u.forLoop.next = $7;
            $$->u.forLoop.stmt = $9;
        }}
    | FOR '(' VAR variable_declaration_list_noin ';' expr ';'  ')' statement
        {{  Node varDec = newNode(EVardec_stm, MP);
            varDec->u.nameValueList = $4;
            $$ = newNode(EFor_stm, MP);
            $$->u.forLoop.init = varDec;
            $$->u.forLoop.cond = $6;
            $$->u.forLoop.next = 0;
            $$->u.forLoop.stmt = $9;
        }}
    | FOR '(' VAR variable_declaration_list_noin ';' expr ';' expr ')' statement
        {{  Node varDec = newNode(EVardec_stm, MP);
            varDec->u.nameValueList = $4;
            $$ = newNode(EFor_stm, MP);
            $$->u.forLoop.init = varDec;
            $$->u.forLoop.cond = $6;
            $$->u.forLoop.next = $8;
            $$->u.forLoop.stmt = $10;
        }}
    | FOR '(' left_hand_side_expr IN expr ')' statement
        {{  $$ = newNode(EForIn_stm, MP);
            $$->u.forInLoop.var = $3;
            $$->u.forInLoop.collection = $5;
            $$->u.forInLoop.stmt = $7;
            $$->u.forInLoop.isVarDec = 0;
        }}
    /*| FOR '(' VAR left_hand_side_expr IN expr ')' statement
        {{  $$ = newNode(EForIn_stm, MP);
            $$->u.forInLoop.var = $4;
            $$->u.forInLoop.collection = $6;
            $$->u.forInLoop.stmt = $8;
            $$->u.forInLoop.isVarDec = 1;
        }}*/
      | FOR '(' VAR variable_declaration_noin IN expr ')' statement
        {{  $$ = newNode(EForIn_stm, MP);
            $$->u.forInLoop.var = 0;
            $$->u.forInLoop.newVar = $4;
            $$->u.forInLoop.collection = $6;
            $$->u.forInLoop.stmt = $8;
            $$->u.forInLoop.isVarDec = 1;
        }}
    ;


continue_statement_no_semi
    :
    CONTINUE identifier
        {{  $$ = newNode(EContinue_stm, MP);
            $$->u.continueBreak.label = $2->u.strVal;
            $$->u.continueBreak.brokenStmt = 0;
            /*free($2); freed from mempool */
        }}
    | CONTINUE
        {{  $$ = newNode(EContinue_stm, MP);
            $$->u.continueBreak.label = 0;
            $$->u.continueBreak.brokenStmt = 0;
        }}
    | CONTINUE_NL
        {{  $$ = newNode(EContinue_stm, MP);
            $$->u.continueBreak.label = 0;
            $$->u.continueBreak.brokenStmt = 0;
        }}
    ;

continue_statement
    : continue_statement_no_semi ';'
        {{  $$ = $1; }}
    | continue_statement_no_semi error
        {{  if(! automatic_test(yyparams, yychar)) YYABORT;
            $$ = $1;
        }}
    ;

break_statement_no_semi
    : BREAK identifier 
        {{  $$ = newNode(EBreak_stm, MP);
            $$->u.continueBreak.label = $2->u.strVal;
            $$->u.continueBreak.brokenStmt = 0;
            /*free($2); freed from mempool */
        }}
    | BREAK
        {{  $$ = newNode(EBreak_stm, MP);
            $$->u.continueBreak.label = 0;
            $$->u.continueBreak.brokenStmt = 0;
        }}
    | BREAK_NL
        {{  $$ = newNode(EBreak_stm, MP);
            $$->u.continueBreak.label = 0;
            $$->u.continueBreak.brokenStmt = 0;
        }}
    ;

break_statement
    : break_statement_no_semi ';'
        {{  $$ = $1; }}
    | break_statement_no_semi error
        {{  if(! automatic_test(yyparams, yychar)) YYABORT;
            $$ = $1;
        }}
    ;

return_statement_no_semi
    : RETURN expr
        {{  $$ = newNode(EReturn_stm, MP);
            $$->u.expr = $2;
        }}
    | RETURN
        {{  $$ = newNode(EReturn_stm, MP);
            $$->u.expr = 0;
        }}
    | RETURN_NL
        {{  $$ = newNode(EReturn_stm, MP);
            $$->u.expr = 0;
        }}
    ;

return_statement
    : return_statement_no_semi ';'
        {{  $$ = $1; }}
    | return_statement_no_semi error
        {{  if(! automatic_test(yyparams, yychar)) YYABORT;
            $$ = $1;
        }}
    ;

with_statement
    : WITH '(' expr ')' statement
        {{  $$ = newNode(EWith_stm, MP);
            $$->u.with.scope = $3;
            $$->u.with.stmt = $5;
        }}
    ;

 /* TODO */
switch_statement
    : SWITCH '(' expr ')' case_block
        {{  $$ = $5;
            $$->u.switchStm.expr = $3;
        }}
    ;

 /* TODO */
case_block
    : '{' '}'
        {{  $$ = newNode(ESwitch_stm, MP);
            $$->u.switchStm.defaultStmtList = 0;
            $$->u.switchStm.caseList = newList(MP);
        }}
    | '{' case_clauses '}'
        {{  $$ = newNode(ESwitch_stm, MP);
            $$->u.switchStm.defaultStmtList = 0;
            $$->u.switchStm.caseList = $2;
        }}
    | '{' default_clause '}'
        {{  $$ = newNode(ESwitch_stm, MP);
            $$->u.switchStm.defaultStmtList = $2;
            $$->u.switchStm.caseList = newList(MP);
        }}
    | '{' default_clause case_clauses '}'
        {{  $$ = newNode(ESwitch_stm, MP);
            $$->u.switchStm.defaultStmtList = $2;
            $$->u.switchStm.caseList = $3;
        }}
    | '{' case_clauses default_clause '}'
        {{  $$ = newNode(ESwitch_stm, MP);
            $$->u.switchStm.defaultStmtList = $3;
            $$->u.switchStm.caseList = $2;
        }}
    | '{' case_clauses default_clause case_clauses '}'
        {{  $$ = newNode(ESwitch_stm, MP);
            $$->u.switchStm.defaultStmtList = $3;
            $$->u.switchStm.caseList = appendList($2, $4);
            /*free($4); freed from mempool */
        }}
    ;

case_clauses
    : case_clause
        {{  $$ = newList(MP);
            pushBack($1, $$, MP);
        }}
    | case_clauses case_clause
        {{  $$ = $1;
            pushBack($2, $$, MP);
        }}
    ;

case_clause
    : CASE expr ':'
        {{  $$ = newCaseClause($2, 0, MP); }}
    | CASE expr ':' statement_list
        {{  $$ = newCaseClause($2, $4, MP); }}
    ;

default_clause
    : DEFAULT ':'
        {{  $$ = 0; }}
    | DEFAULT ':' statement_list
        {{  $$ = $3;
        }}
    ;

labelled_statement
    : identifier ':' statement
        {{  $$ = $3;
            if (! $$->stmtLabels) 
                $$->stmtLabels = newList(MP);
            pushFront($1->u.strVal, $$->stmtLabels, MP);
        }}
    ;

 /* no line terminator here ?? */
throw_statement
    :
    THROW expr ';'
        {{  $$ = newNode(EThrow_stm, MP);
            $$->u.expr = $2;
        }}
    | THROW expr error
        {{  if(! automatic_test(yyparams, yychar)) YYABORT;
            $$ = newNode(EThrow_stm, MP);
            $$->u.expr = $2;
        }}
    ;

try_statement
    : TRY '{' block_contents '}' catch
        {{  $$ = $5;
            $$->u.tryCatchFinally.tryBlock = $3;
            $$->u.tryCatchFinally.finallyBlock = 0;
        }}
    | TRY '{' block_contents '}' finally
        {{  $$ = $5;
            $$->u.tryCatchFinally.tryBlock = $3;
            $$->u.tryCatchFinally.catchBlock = 0;
            $$->u.tryCatchFinally.catchVar = 0;
        }}
    | TRY '{' block_contents '}' catch finally
        {{  $$ = $5;
            $$->u.tryCatchFinally.tryBlock = $3;
            $$->u.tryCatchFinally.finallyBlock =
                            $6->u.tryCatchFinally.finallyBlock;
            /*free($6); freed from mempool */
        }}
    ;

catch
    : CATCH '(' identifier ')' '{' block_contents '}'
        {{  $$ = newNode(ETry_stm, MP);
            $$->u.tryCatchFinally.catchVar = $3->u.strVal;
            /*free($3); freed from mempool */
            $$->u.tryCatchFinally.catchBlock = $6;
        }}
    ;

finally
    : FINALLY '{' block_contents '}'
        {{  $$ = newNode(ETry_stm, MP);
            $$->u.tryCatchFinally.finallyBlock = $3;
        }}
    ;


function_declaration
    : FUNCTION identifier '(' formal_parameter_list ')' function_body
        {{  char* name = $2->u.strVal;
            /*free($2); freed from mempool */
            $$ = newNode(EFunDec_stm, MP);
            $$->u.function.name = name;
            $$->u.function.params = $4;
            $$->u.function.stmtList = $6;
            
            $$->u.function.functionNr = 0;
            $$->u.function.vardecList = newList(MP);
            $$->u.function.innerFuncs = newList(MP);
            $$->u.function.nInnerFuncs = 0;
            $$->u.function.funDecList = newList(MP);
            $$->u.function.hasTryCatch = 0;
        }}
    | FUNCTION identifier '(' ')' function_body
        {{  char* name = $2->u.strVal;
            /*free($2); freed from mempool */
            $$ = newNode(EFunDec_stm, MP);
            $$->u.function.name = name;
            $$->u.function.params = newList(MP);
            $$->u.function.stmtList = $5;
            
            $$->u.function.functionNr = 0;
            $$->u.function.vardecList = newList(MP);
            $$->u.function.innerFuncs = newList(MP);
            $$->u.function.nInnerFuncs = 0;
            $$->u.function.funDecList = newList(MP);
            $$->u.function.hasTryCatch = 0;
        }}
    ;

function_expr
    : FUNCTION identifier '(' formal_parameter_list ')' function_body
        {{  char* name = $2->u.strVal;
            /*free($2); freed from mempool */
            $$ = newNode(EFunExpr, MP);
            $$->u.function.name = name;
            $$->u.function.params = $4;
            $$->u.function.stmtList = $6;
            
            $$->u.function.functionNr = 0;
            $$->u.function.vardecList = newList(MP);
            $$->u.function.innerFuncs = newList(MP);
            $$->u.function.nInnerFuncs = 0;
            $$->u.function.funDecList = newList(MP);
            $$->u.function.hasTryCatch = 0;
        }}
    | FUNCTION identifier '(' ')' function_body
        {{  char* name = $2->u.strVal;
            /*free($2); freed from mempool */
            $$ = newNode(EFunExpr, MP);
            $$->u.function.name = name;
            $$->u.function.params = newList(MP);
            $$->u.function.stmtList = $5;
            
            $$->u.function.functionNr = 0;
            $$->u.function.vardecList = newList(MP);
            $$->u.function.innerFuncs = newList(MP);
            $$->u.function.nInnerFuncs = 0;
            $$->u.function.funDecList = newList(MP);
            $$->u.function.hasTryCatch = 0;
        }}
    | FUNCTION '(' formal_parameter_list ')' function_body
        {{  $$ = newNode(EFunExpr, MP);
            $$->u.function.name = 0;
            $$->u.function.params = $3;
            $$->u.function.stmtList = $5;
            
            $$->u.function.functionNr = 0;
            $$->u.function.vardecList = newList(MP);
            $$->u.function.innerFuncs = newList(MP);
            $$->u.function.nInnerFuncs = 0;
            $$->u.function.funDecList = newList(MP);
            $$->u.function.hasTryCatch = 0;
        }}
    | FUNCTION '(' ')' function_body
        {{  $$ = newNode(EFunExpr, MP);
            $$->u.function.name = 0;
            $$->u.function.params = newList(MP);
            $$->u.function.stmtList = $4;
            
            $$->u.function.functionNr = 0;
            $$->u.function.vardecList = newList(MP);
            $$->u.function.innerFuncs = newList(MP);
            $$->u.function.nInnerFuncs = 0;
            $$->u.function.funDecList = newList(MP);
            $$->u.function.hasTryCatch = 0;
        }}
    ;

formal_parameter_list
    : identifier
        {{  char* name = $1->u.strVal;
            /*free($1); freed from mempool */
            $$ = newList(MP);
            pushBack(name, $$, MP);
        }}
    | formal_parameter_list ',' identifier
        {{  char* name = $3->u.strVal;
            /*free($3); freed from mempool */
            $$ = $1;
            pushBack(name, $$, MP);
        }}
    ;

function_body
    : '{' source_elements '}'
        {{  $$ = $2; }}
    | '{' '}'
        {{  $$ = newList(MP); }}
    ;

program
    : source_elements
        {{  $$ = $1;
            yyparams->program = newNode(EFunExpr, MP);
            yyparams->program->u.function.name = 0;
            yyparams->program->u.function.params = newList(MP);
            yyparams->program->u.function.stmtList = $$;
            
            yyparams->program->u.function.functionNr = 0;
            yyparams->program->u.function.vardecList = newList(MP);
            yyparams->program->u.function.innerFuncs = newList(MP);
            yyparams->program->u.function.nInnerFuncs = 0;
            yyparams->program->u.function.funDecList = newList(MP);
            yyparams->program->u.function.hasTryCatch = 0;
        }}
    | /*empty*/
        {{  $$ = newList(MP);
            yyparams->program = newNode(EFunExpr, MP);
            yyparams->program->u.function.name = 0;
            yyparams->program->u.function.params = newList(MP);
            yyparams->program->u.function.stmtList = $$;
            
            yyparams->program->u.function.functionNr = 0;
            yyparams->program->u.function.vardecList = newList(MP);
            yyparams->program->u.function.innerFuncs = newList(MP);
            yyparams->program->u.function.nInnerFuncs = 0;
            yyparams->program->u.function.funDecList = newList(MP);
            yyparams->program->u.function.hasTryCatch = 0;
        }}
    ;

source_elements
    : source_element
        {{  $$ = newList(MP);
            pushBack($1, $$, MP);
        }}
    | source_elements source_element
        {{  $$ = $1;
            pushBack($2, $$, MP);
        }}
    ;

source_element
    : statement
        {{  $$ = $1; }}
    | function_declaration
        {{ $$ = $1; }}
    ;

%%


static void err_msg(Params* params, const char *msg) {
    snprintf(params->err_msg, sizeof(params->err_msg)-1, "[line %d, column %d] %s", 
        params->line_no + 1, params->column_no, msg);
}

/* may we automatically insert a semicolon? */
static int automatic_test(Params* params, int yychar) {
    if (yychar == LEX_ERROR) {
        return 0;
    } else if (yychar == '}' || yychar == 0) {
        return 1;
    } else if (params->prev_was_newline) {
        return 1;
    } else {
        return 0;
    }
}

void yyerror (YYLTYPE* locp, Params* params, const char* msg) {
    snprintf(params->err_msg, sizeof(params->err_msg)-1, "[line %d, column %d] %s", 
        params->line_no + 1, params->column_no, msg);
}
