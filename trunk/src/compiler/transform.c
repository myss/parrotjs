#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "nodes.h"
#include "mempool.h"

#include "transform.h"

typedef Node GetNode_t(void* listElem);

static Node 
castNode(void* x) { 
    return (Node) x;
}
static Node 
getValue(void* nameValue) {
    return ((NameValue) nameValue)->value;
}

static void 
visitList( PjsList nodeList, 
           Node parentFunc, 
           Node parentBlock, 
           GetNode_t getNode, 
           Mempool MP) {
    ListEntry entry;
    
    if (! nodeList)
        return;
    entry = nodeList->start;
    while(entry) {
        Node node = getNode(entry->elem);
        visit(node, parentFunc, parentBlock, MP);
        entry = entry->next;
    }
}

static void 
visitCaseList( PjsList nodeList, 
               Node parentFunc, 
               Node parentBlock, 
               Mempool MP) {
    ListEntry entry = nodeList->start;
    while(entry) {
        CaseClause clause = (CaseClause) entry->elem;
        visit(clause->expr, parentFunc, parentBlock, MP);
        visitList(clause->stmtList, parentFunc, parentBlock, castNode, MP);
        entry = entry->next;
    }
}

static int
containsStr(PjsList strList, char* str) {
    ListEntry entry;
    
    if (! strList)
        return 0;
    entry = strList->start;
    while(entry) {
        char* elem = (char*) entry->elem;
        if (strcmp(str, elem) == 0)
            return 1;
        entry = entry->next;
    }
    return 0;
}

void visit(Node node, Node parentFunc, Node parentBlock, Mempool MP) {
    if (! node)
        return;

    node->outerBlock = parentBlock;
    
    switch(node->kind) {
    case EArrayLit: 
        visitList(node->u.exprList, parentFunc, parentBlock, castNode, MP);
        break;
    case EObjectLit: 
        visitList(node->u.nameValueList, parentFunc, parentBlock, getValue, MP);
        break;
    case EPropAccess: 
        visit(node->u.getprop.base, parentFunc, parentBlock, MP);
        break;
    case EIndexedAccess: 
        visit(node->u.getindexed.base, parentFunc, parentBlock, MP);
        visit(node->u.getindexed.index, parentFunc, parentBlock, MP);
        break;
    case ENewFuncall: 
    case EFuncall: 
        visit(node->u.funcall.fun, parentFunc, parentBlock, MP);
        visitList(node->u.funcall.args, parentFunc, parentBlock, castNode, MP);
        break;
    case EBinop: 
        visit(node->u.binop.left, parentFunc, parentBlock, MP);
        visit(node->u.binop.right, parentFunc, parentBlock, MP);
        break;
    case EUnop:
        visit(node->u.unop.expr, parentFunc, parentBlock, MP);
        break;
    case ETernop: 
        visit(node->u.ternop.cond, parentFunc, parentBlock, MP);
        visit(node->u.ternop.ifTrue, parentFunc, parentBlock, MP);
        visit(node->u.ternop.ifFalse, parentFunc, parentBlock, MP);
        break;
    case EAssign: 
        visit(node->u.assign.left, parentFunc, parentBlock, MP);
        visit(node->u.assign.right, parentFunc, parentBlock, MP);
        break;
    case ECompound: 
        visit(node->u.exprPair.left, parentFunc, parentBlock, MP);
        visit(node->u.exprPair.right, parentFunc, parentBlock, MP);
        break;
    case EFunExpr:
        if (parentFunc) {
            node->u.function.functionNr = parentFunc->u.function.nInnerFuncs++;
            pushBack(node, parentFunc->u.function.innerFuncs, MP);
        } else {
            node->u.function.functionNr = 0;
        }
        node->u.function.innerFuncs = newList(MP);
        node->u.function.vardecList = newList(MP);
        node->u.function.funDecList = newList(MP);
        visitList(node->u.function.stmtList, node, node, castNode, MP);
        break;
    case EFunDec_stm: 
        if (parentFunc) {
            node->u.function.functionNr = parentFunc->u.function.nInnerFuncs++; 
            pushBack(node, parentFunc->u.function.innerFuncs, MP);
            pushBack(node, parentFunc->u.function.funDecList, MP);
        } else {
            node->u.function.functionNr = 0;
        }
        node->u.function.innerFuncs = newList(MP);
        node->u.function.vardecList = newList(MP);
        node->u.function.funDecList = newList(MP);
        visitList(node->u.function.stmtList, node, node, castNode, MP);
        break;
    case EBlock_stm: 
        visitList(node->u.stmtList, parentFunc, node, castNode, MP);
        break;
    case EVardec_stm: {
        ListEntry entry = node->u.nameValueList->start;
        if (! parentFunc->u.function.vardecList) {
            parentFunc->u.function.vardecList = newList(MP);
        }
        while(entry) {
            NameValue nv = (NameValue) entry->elem;
            pushBack(nv->name, parentFunc->u.function.vardecList, MP);
            if (nv->value)
                visit(nv->value, parentFunc, parentBlock, MP);
            entry = entry->next;
        }
    }
        break;
    case EIfElse_stm:
        visit(node->u.ifElse.cond, parentFunc, parentBlock, MP);
        visit(node->u.ifElse.ifTrue, parentFunc, node, MP);
        visit(node->u.ifElse.ifFalse, parentFunc, node, MP);
        break;
    case EWhile_stm: 
    case EDoWhile_stm: 
        visit(node->u.whileLoop.cond, parentFunc, parentBlock, MP);
        visit(node->u.whileLoop.stmt, parentFunc, node, MP);
        break;
    case EFor_stm:
        visit(node->u.forLoop.init, parentFunc, parentBlock, MP);
        visit(node->u.forLoop.cond, parentFunc, parentBlock, MP);
        visit(node->u.forLoop.next, parentFunc, parentBlock, MP);
        visit(node->u.forLoop.stmt, parentFunc, node, MP);
        break;
    case EForIn_stm: 
        if (node->u.forInLoop.isVarDec) {
            if (! parentFunc->u.function.vardecList) {
                parentFunc->u.function.vardecList = newList(MP);
            }
            pushBack(node->u.forInLoop.newVar->name, parentFunc->u.function.vardecList, MP);
        } else {
            visit(node->u.forInLoop.var, parentFunc, parentBlock, MP);
        }
        visit(node->u.forInLoop.collection, parentFunc, parentBlock, MP);
        visit(node->u.forInLoop.stmt, parentFunc, node, MP);
        break;
    case EReturn_stm: 
        visit(node->u.expr, parentFunc, parentBlock, MP);
        break;
    case ESwitch_stm:
        visit(node->u.switchStm.expr, parentFunc, parentBlock, MP);
        visitCaseList(node->u.switchStm.caseList, parentFunc, node, MP);
        visitList(node->u.switchStm.defaultStmtList, parentFunc, node, castNode, MP);
        break;
    case EContinue_stm:
        if (! node->u.continueBreak.label) {
            Node contBlock = parentBlock;
            while(  contBlock && 
                    contBlock->kind != EFunExpr &&
                    contBlock->kind != EFunDec_stm &&
                    contBlock->kind != EFor_stm &&
                    contBlock->kind != EForIn_stm &&
                    contBlock->kind != EWhile_stm) {
                contBlock = contBlock->outerBlock;
            }
            if (    ! contBlock || 
                    contBlock->kind == EFunExpr ||
                    contBlock->kind == EFunDec_stm) {
                /* TODO how to handle error? */
                fprintf(stderr, "continue not inside a loop\n");
                exit(1);
            }
            assert( contBlock->kind == EFor_stm ||
                    contBlock->kind == EForIn_stm ||
                    contBlock->kind == EWhile_stm);
            node->u.continueBreak.brokenStmt = contBlock;
        } else {
            char* label = node->u.continueBreak.label;
            Node contBlock = parentBlock;
            while(  contBlock && 
                    contBlock->kind != EFunExpr &&
                    contBlock->kind != EFunDec_stm &&
                    ! containsStr(contBlock->stmtLabels, label)) {
                contBlock = contBlock->outerBlock;
            }
            if ( ! contBlock ||
                 contBlock->kind == EFunExpr ||
                 contBlock->kind == EFunDec_stm ) {
                /* TODO how to handle error? */
                fprintf(stderr, "Invalid label: %s\n", label);
                exit(1);
            } else if ( contBlock->kind != EFor_stm &&
                        contBlock->kind != EForIn_stm &&
                        contBlock->kind != EWhile_stm   ) {
                /* TODO how to handle error? */
                fprintf(stderr, "continue not inside a loop\n");
                exit(1);
            } else {
                node->u.continueBreak.brokenStmt = contBlock;
                
                /* don't allow duplicate labels */
                contBlock = contBlock->outerBlock;
                while(  contBlock && 
                        contBlock->kind != EFunExpr &&
                        contBlock->kind != EFunDec_stm &&
                        ! containsStr(contBlock->stmtLabels, label)) {
                    contBlock = contBlock->outerBlock;
                }
                if ( contBlock && 
                     contBlock->kind != EFunExpr && 
                     contBlock->kind != EFunDec_stm ) {
                    /* TODO how to handle error? */
                    fprintf(stderr, "Duplicate label: %s\n", label);
                    exit(1);
                }
            }
        }
        break;
    case EBreak_stm:
        if (! node->u.continueBreak.label) {
            Node contBlock = parentBlock;
            while(  contBlock && 
                    contBlock->kind != EFunExpr &&
                    contBlock->kind != EFunDec_stm &&
                    contBlock->kind != EFor_stm &&
                    contBlock->kind != EForIn_stm &&
                    contBlock->kind != EWhile_stm &&
                    contBlock->kind != EDoWhile_stm &&
                    contBlock->kind != ESwitch_stm) {
                contBlock = contBlock->outerBlock;
            }
            if (    ! contBlock || 
                    contBlock->kind == EFunExpr ||
                    contBlock->kind == EFunDec_stm) {
                /* TODO how to handle error? */
                fprintf(stderr, "break not inside a loop\n");
                exit(1);
            }
            node->u.continueBreak.brokenStmt = contBlock;
        } else {
            char* label = node->u.continueBreak.label;
            Node contBlock = parentBlock;
            while(  contBlock && 
                    contBlock->kind != EFunExpr &&
                    contBlock->kind != EFunDec_stm &&
                    ! containsStr(contBlock->stmtLabels, label)) {
                contBlock = contBlock->outerBlock;
            }
            if ( ! contBlock ||
                 contBlock->kind == EFunExpr ||
                 contBlock->kind == EFunDec_stm ) {
                /* TODO how to handle error? */
                fprintf(stderr, "Invalid label: %s\n", label);
                exit(1);
            } else {
                node->u.continueBreak.brokenStmt = contBlock;
                
                /* don't allow duplicate labels */
                contBlock = contBlock->outerBlock;
                while(  contBlock && 
                        contBlock->kind != EFunExpr &&
                        contBlock->kind != EFunDec_stm &&
                        ! containsStr(contBlock->stmtLabels, label)) {
                    contBlock = contBlock->outerBlock;
                }
                if ( contBlock && 
                     contBlock->kind != EFunExpr && 
                     contBlock->kind != EFunDec_stm ) {
                    /* TODO how to handle error? */
                    fprintf(stderr, "Duplicate label: %s\n", label);
                    exit(1);
                }
            }
        }
        break;
    case EWith_stm: 
        visit(node->u.with.scope, parentFunc, parentBlock, MP);
        visit(node->u.with.stmt, parentFunc, node, MP);
        break;
        
    case EThrow_stm:
        visit(node->u.expr, parentFunc, parentBlock, MP);
        break;
        
    case ETry_stm:
        if(parentFunc) 
            parentFunc->u.function.hasTryCatch = 1;
        visitList(node->u.tryCatchFinally.tryBlock, parentFunc, node, castNode, MP);
        visitList(node->u.tryCatchFinally.catchBlock, parentFunc, node, castNode, MP);
        visitList(node->u.tryCatchFinally.finallyBlock, parentFunc, node, castNode, MP);
        break;
        
    case EThis: 
    case ENull: 
    case EBoolean: 
    case EElision:
    case EIdentifier: 
    case EString: 
    case ENumeric: 
    case ERegex: 
    case EPirCode_stm:
        break;
    }
}
