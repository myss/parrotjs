#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "list.h"
#include "nodes.h"
#include "mempool.h"

Node newNode(int kind, Mempool mp) {
    Node node = mp_malloc(sizeof(struct Node_), mp);
    node->kind = kind;
    node->stmtLabels = 0;
    node->labelNo = -1;
    return node;
}
Node newNumberExpr(char* str, Mempool mp) {
    Node expr = newNode(ENumeric, mp);
    assert(str && str[0]);
    expr->u.doubleVal = str;
    return expr;
}
Node newRegexExpr(char* str, Mempool mp) {
    Node expr = newNode(ERegex, mp);
    expr->u.strVal = str;
    return expr;
}

Node newUnop(int op, Node expr, Mempool mp) {
    Node node = newNode(EUnop, mp);
    node->u.unop.expr = expr;
    node->u.unop.op = op;
    return node;
}


NameValue newNameValue(char* name, Node value, Mempool mp) {
    NameValue prop = (NameValue) mp_malloc(sizeof(struct NameValue_), mp);
    prop->name = name;
    prop->value = value;
    return prop;
}

NodePair newNodePair(Node first, Node second, Mempool mp) {
    NodePair pair = (NodePair) mp_malloc(sizeof(struct NodePair_), mp);
    pair->first = first;
    pair->second = second;
    return pair;
}

CaseClause newCaseClause(Node expr, PjsList stmtList, Mempool mp) {
    CaseClause cl = (CaseClause) mp_malloc(sizeof(struct CaseClause_), mp);
    cl->expr = expr;
    cl->stmtList = stmtList;
    return cl;
}

/* TODO not implemented as it should be  */
char* strlit2str(char* str, Mempool mp) {
    int len;
    char* s2;
    
    assert(str);
    len = strlen(str);
    assert( len >= 2 && str[0]==str[len-1] && 
            (str[0]=='"' || str[0]=='\'')   );
    s2 = mp_malloc((len-1) * sizeof(char), mp);
    strncpy(s2, str+1, len-2);
    s2[len-2] = '\0';
    return s2;
}
/* TODO not implemented as it should be */
char* str2strlit(char* str, Mempool mp) {
    int len;
    char* s2;
    assert(str);
    len = strlen(str);
    s2 = mp_malloc((len+3) * sizeof(char), mp);
    return s2;
}
char* dupl(char* str, int limit, Mempool mp) {
    char* s = mp_malloc((limit+1) * sizeof(char), mp);
    strncpy(s, str, limit);
    s[limit] = '\0';
    return s;
}

