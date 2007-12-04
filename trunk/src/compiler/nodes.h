
#ifndef _NODES_H_
#define _NODES_H_

#include "list.h"
#include "mempool.h"

struct Node_;
typedef struct Node_ *Node;

struct NameValue_;
typedef struct NameValue_ *NameValue;

struct CaseClause_;
typedef struct CaseClause_ *CaseClause;

struct NodePair_;
typedef struct NodePair_ *NodePair;


Node newNode(int kind, Mempool mp);
Node newNumberExpr(char* str, Mempool mp);
Node newRegexExpr(char* str, Mempool mp);

Node newUnop(int op, Node expr, Mempool mp);

NameValue newNameValue(char* name, Node value, Mempool mp);
NodePair newNodePair(Node first, Node second, Mempool mp);

CaseClause newCaseClause(Node expr, PjsList stmtList, Mempool mp);

/* e.g. "\"Hello\\n"\"" -> "Hello\n" */
char* strlit2str(char* str, Mempool mp);
char* str2strlit(char* str, Mempool mp);

char* dupl(char* str, int limit, Mempool mp);

struct Node_ {
    enum { 
        /* EXPRESSIONS */
        EThis, ENull, EBoolean, EElision,
        EIdentifier, EString, ENumeric, EArrayLit, 
        EObjectLit, ERegex, 
        ENewFuncall, EFuncall, EBinop, EUnop,
        ETernop, EAssign, ECompound, EFunExpr,
        EPropAccess, EIndexedAccess, EEval,
        /* STATEMENTS */
        EBlock_stm, EVardec_stm, EIfElse_stm,
        EWhile_stm, EDoWhile_stm, EFor_stm,
        EForIn_stm, EContinue_stm, EBreak_stm,
        EReturn_stm, EWith_stm, EThrow_stm,
        ETry_stm, EFunDec_stm, ESwitch_stm,
        EPirCode_stm
    } kind;
    
    PjsList stmtLabels;
    int labelNo;
    Node outerBlock;
    union {
        /* EXPRESSIONS */
        int boolVal;
        char* strVal;
        char* doubleVal;
        
        PjsList exprList;
        PjsList nameValueList;
        PjsList nodePairList;
        struct {
            Node fun; 
            PjsList args;
        } funcall;
        
        struct {
            Node left; 
            Node right;
        } exprPair;
        
        struct {
            /* base.propname */
            Node base; 
            char* propName;
        } getprop;
        struct {
            /* base[index] */
            Node base; 
            Node index;
        } getindexed;
        
        
        struct {
            Node left;
            Node right; 
            int op;
        } binop;
        
        struct {
            Node expr;
            int op;
        } unop;
        
        struct {
            Node cond; 
            Node ifTrue; 
            Node ifFalse;
        } ternop;

        struct { 
            Node left;
            Node right;
            int type;
        } assign;

        
        /* STATEMENTS */
        
        PjsList stmtList;
        
        struct {
            Node cond; 
            Node ifTrue; 
            Node ifFalse;
        } ifElse;
        
        struct {
            Node cond; 
            Node stmt;
        } whileLoop;
        
        struct {
            Node init; 
            Node cond; 
            Node next; 
            Node stmt;
        } forLoop;
        
        struct {
            Node var; /* expr */
            NameValue newVar;
            
            Node collection; 
            Node stmt; 
            int isVarDec;
        } forInLoop;
        
        struct {
            char* label;
            /* used after transformation */
            Node brokenStmt;
        } continueBreak;
        
        Node expr; /* for EReturn_stm or EThrow_stm or ... */
        
        struct {
            Node scope; 
            Node stmt;
        } with;
        
        struct {
            PjsList tryBlock;
            char* catchVar;
            PjsList catchBlock;
            PjsList finallyBlock;
        } tryCatchFinally;
        
        struct {
            char* name;
            PjsList params; /* PjsList<char*> */
            PjsList stmtList; /* PjsList<Node> */
            
            /* Used after transformation */
            int functionNr;
            PjsList vardecList; /* PjsList<char*> */
            PjsList innerFuncs; /* PjsList<Node[type=EFunExpr|EFunDec_stm]> */
            int nInnerFuncs;
            
            PjsList funDecList;
            int hasTryCatch;
            char* pir_subroutine_name;
        } function;
        
        struct {
            Node expr;
            PjsList caseList; /* PjsList<CaseClause> */
            PjsList defaultStmtList; /* type==stmt */
        } switchStm;
        
        struct {
            char* label;
            Node stmt;
        } labelledStm;
        
    } u;
};

struct NameValue_ {
    char* name;
    Node value;
};

struct CaseClause_ {
    Node expr;
    PjsList stmtList;
};

struct NodePair_ {
    Node first;
    Node second;
};

#endif
