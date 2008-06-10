#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "emit.h"
#include "list.h"
#include "params.h"
#include "nodes.h"
#include "parser.tab.h"
#include "mempool.h"
#include "transform.h"
#include "str_escaping.h"

#include "../pmc/pjsstructs.h"

#include "parrot/parrot.h"
#include "parrot/misc.h"


#define EXCEPTION_MSG_INDEX 0
#define EXCEPTION_JSOBJ_INDEX 9

/* tailcall seems to have some bugs (causes segfaults) */
#define DO_TAIL_CALL 0


void emit(parrot_data* to, const char *format, ...) {
    va_list args;
    STRING *written;
    va_start(args, format);
    written = Parrot_vsprintf_c(to->interp, format, args);
    /* Double memory usage while emitting code, free written? */
    to->code = string_append(to->interp, to->code, written);
}

char* escapeForLabel(const char* str) {
    int len = strlen(str);
    char* result = (char*) malloc(len + 1);
    int i;
    for (i=0; i<len; i++) {
        char ch = str[i];
        if (ch == '$')
            ch = '@';
        result[i] = ch;
    }
    result[len] = '\0';
    return result;
}


static void emit_newObject(int destReg, Naming naming, parrot_data* to) {
    int protoReg = naming.regNo++;
    emit(to, "    $P%d = pjs_find_lex @env_%d, 'Object'\n", protoReg, naming.scopeNo);
    emit(to, "    $P%d = $P%d['prototype']\n", protoReg, protoReg);
    emit(to, "    $P%d = new 'PjsObject'\n", destReg);
    emit(to, "    $P%d.'setProto'($P%d)\n", destReg, protoReg);
}


void emit_undefined(int destReg, Naming naming, parrot_data* to) {
    /* emit(to, "    $P%d = new 'PjsUndefined' \n", destReg); */
    /*emit(to, "    $P%d = get_hll_global 'undefined'\n", destReg);*/
    emit(to, "    $P%d = @undefined\n", destReg);
}

static char* createName(char* prefix, int n, Mempool mp) {
    char* name = mp_malloc(11 + strlen(prefix), mp);
    assert(n >= 0);
    sprintf(name, "%s_%d", prefix, n);
    return name;
}

static void emit_stmtList(PjsList stmts, int destReg, Naming naming, parrot_data* to) {
    ListEntry entry;
    int emitted = 0;
    
    /* the value of the last statement is stored in destReg */
    for ( entry = stmts->start;
          entry;
          entry = entry->next ) {
        emit_stmt((Node) entry->elem, destReg, naming, to);
        emitted = 1;
    }
    /* if there is no statement, store undefined in destReg */
    if (destReg >= 0 && ! emitted) {
        emit_undefined(destReg, naming, to);
    }
}

static void emitTryFinally(PjsList tryBlock, PjsList finallyBlock, int destReg, Naming naming, parrot_data* to) {
    int labelNo = (*naming.labelNo)++;
    
    if (destReg >= 0) {
        emit_undefined(destReg, naming, to);
    }
    emit(to, "# <try-finally %d>\n", labelNo);
    emit(to, "    pushmark %d\n", labelNo);
    emit(to, "    push_eh FINALLY_HANDLER@%d\n", labelNo);

    emit(to, "    .local int @has_exception_%d\n", labelNo);
    emit(to, "    @has_exception_%d = 0\n", labelNo);
    emit_stmtList(tryBlock, destReg, naming, to);
    emit(to, "    goto FINALLY@%d\n", labelNo);
    
    emit(to, "  FINALLY_HANDLER@%d:\n", labelNo);
    emit(to, "    .local pmc @exc\n");
    emit(to, "    .local string @exc_msg\n");
    emit(to, "    .get_results(@exc, @exc_msg)\n");
    emit(to, "    @has_exception_%d = 1\n", labelNo);
    
    emit(to, "  FINALLY@%d:\n", labelNo);
    emit(to, "    popmark %d\n", labelNo);

    emit_stmtList(finallyBlock, destReg, naming, to);
        
    emit(to, "    unless @has_exception_%d goto END_TRY_FINALLY@%d\n", labelNo, labelNo);
    
    emit(to, "    rethrow @exc\n");
    
    emit(to, "  END_TRY_FINALLY@%d:\n", labelNo);
    emit(to, "# end <try-finally %d>\n", labelNo);
}
static void emitTryCatch(PjsList tryBlock, char *catchVar, PjsList catchBlock, int destReg, Naming naming, parrot_data* to) {
    int labelNo = (*naming.labelNo)++;
    int regNo = naming.regNo++;
    Naming namingCopy;

    if (destReg >= 0) {
        emit_undefined(destReg, naming, to);
    }
    emit(to, "# <try-catch %d>\n", labelNo);
    emit(to, "    push_eh CATCH@%d\n", labelNo);
    emit_stmtList(tryBlock, destReg, naming, to);
    emit(to, "    pop_eh\n", labelNo);
    emit(to, "    goto END_TRY_CATCH@%d\n", labelNo);
    
    namingCopy = naming;
    namingCopy.scopeNo++;
    emit(to, "  CATCH@%d:\n", labelNo);
    emit(to, "    .local pmc @exc\n");
    emit(to, "    .local string @exc_msg\n");
    emit(to, "    .get_results(@exc, @exc_msg)\n");
    
    /* Unwrap our js exception from the parrot exception.
     * If it is not a js exception, unwrap it as a string.
     */
    emit(to, "    $I%d = exists @exc[%d]\n", regNo, EXCEPTION_JSOBJ_INDEX);
    emit(to, "    unless $I%d goto EXC_NOT_JS@%d\n", regNo, labelNo);
    emit(to, "    $P%d = @exc[%d]\n", regNo, EXCEPTION_JSOBJ_INDEX);
    emit(to, "    goto EXC_END@%d\n", labelNo);
    emit(to, "  EXC_NOT_JS@%d:\n", labelNo);
    emit(to, "    $I%d = exists @exc[%d]\n", regNo, EXCEPTION_MSG_INDEX);
    emit(to, "    unless $I%d goto EXC_NO_MSG@%d\n", regNo, labelNo);
    emit(to, "    $P%d = @exc[%d]\n", regNo, EXCEPTION_MSG_INDEX);
    emit(to, "    goto EXC_END@%d\n", labelNo);
    emit(to, "  EXC_NO_MSG@%d:\n", labelNo);
    emit(to, "    $P%d = new 'PjsString'\n", regNo);
    emit(to, "    $P%d = 'Unknown exception.'\n", regNo);
    emit(to, "  EXC_END@%d:\n", labelNo);
    
    /* We need to push a new Hash at the front of the scope chain 
     * to store the caught exception variable.
     */
    emit(to, "    $P%d = new 'Hash'\n", naming.regNo);
    emit(to, "    $P%d['%s'] = $P%d\n", naming.regNo, catchVar, regNo);
    emit(to, "    .local pmc @env_%d\n", namingCopy.scopeNo);
    emit(to, "    @env_%d = pjs_augment_scope_chain_with @env_%d, $P%d\n",
                            namingCopy.scopeNo, naming.scopeNo, naming.regNo);

    emit_stmtList(catchBlock, destReg, namingCopy, to);
    emit(to, "  END_TRY_CATCH@%d:\n", labelNo);
}


static void emit_func_params(int startReg, int nParams, int currentScope, parrot_data* to) {
    int i;
    for (i = 0; i<nParams; i++) {
        emit(to, ", $P%d", startReg + i);
    }
    emit(to, ")\n");
}
static void 
emit_funcall(Node expr, int destReg, int doReturn, Naming naming, parrot_data* to) {
    int nParams, funcReg;
    int startReg = naming.regNo;
    ListEntry entry;
    for (entry = expr->u.funcall.args->start; entry; entry = entry->next) {
        Node arg = (Node) entry->elem;
        int reg = naming.regNo++;
        emit_expr(arg, reg, naming, to);
    }
    nParams = naming.regNo - startReg;
    if (expr->u.funcall.fun->kind == EPropAccess) {
        int thisReg = naming.regNo++;
        Node base = expr->u.funcall.fun->u.getprop.base;
        char* propname = expr->u.funcall.fun->u.getprop.propName;
        funcReg = naming.regNo++;
        
        emit_expr(base, thisReg, naming, to);
        emit(to, "    $P%d = '__toObject__'($P%d, @env_%d)\n", thisReg, thisReg, naming.scopeNo);
        
        if (doReturn) {
#if DO_TAIL_CALL
            emit(to, "    .return 'pjs_invoke_method'($P%d, '%s', @env_%d", thisReg, thisReg, propname, naming.scopeNo);
            emit_func_params(startReg, nParams, naming.scopeNo, to);
#else
            emit(to, "    $P%d = 'pjs_invoke_method'($P%d, '%s', @env_%d", thisReg, thisReg, propname, naming.scopeNo);
            emit_func_params(startReg, nParams, naming.scopeNo, to);
            emit(to, "    .return ($P%d)\n", thisReg);
#endif /* DO_TAIL_CALL */
        } else {
            emit(to, "    $P%d = 'pjs_invoke_method'($P%d, '%s', @env_%d", destReg, thisReg, propname, naming.scopeNo);
            emit_func_params(startReg, nParams, naming.scopeNo, to);
        }
    } else if (expr->u.funcall.fun->kind == EIndexedAccess) {
        int thisReg = naming.regNo++;
        int indexReg = naming.regNo++;
        Node base = expr->u.funcall.fun->u.getindexed.base;
        Node index = expr->u.funcall.fun->u.getindexed.index;
        funcReg = naming.regNo++;
        
        emit_expr(base, thisReg, naming, to);
        emit(to, "    $P%d = '__toObject__'($P%d, @env_%d)\n", thisReg, thisReg, naming.scopeNo);
        emit_expr(index, indexReg, naming, to);
        emit(to, "    $P%d = pjs_to_primitive $P%d\n", indexReg, indexReg);
        emit(to, "    $P%d = $P%d[$P%d] \n", funcReg, thisReg, indexReg);
        
        if (doReturn) {
#if DO_TAIL_CALL
            emit(to, "    .return 'pjs_call_function'($P%d, $P%d, @env_%d", thisReg, funcReg, naming.scopeNo);
            emit_func_params(startReg, nParams, naming.scopeNo, to);
#else
            emit(to, "    $P%d = 'pjs_call_function'($P%d, $P%d, @env_%d", thisReg, thisReg, funcReg, naming.scopeNo);
            emit_func_params(startReg, nParams, naming.scopeNo, to);
            emit(to, "    .return ($P%d)\n", thisReg);
#endif /* DO_TAIL_CALL */
        } else {
            emit(to, "    $P%d = 'pjs_call_function'($P%d, $P%d, @env_%d", destReg, thisReg, funcReg, naming.scopeNo);
            emit_func_params(startReg, nParams, naming.scopeNo, to);
        }
    } else if (expr->u.funcall.fun->kind == EIdentifier) {
        /* That solves the next problem:
         *
         * x = 'from scope';
         * obj.x = 'from object';
         * obj.func = function() { print(this.x); };
         * with(obj)
         *     func();
         *
         * It should write 'from object', not 'from scope'.
         */
        int thisReg = naming.regNo++;
        funcReg = naming.regNo++;
        emit(to, "    $P%d = @this\n", thisReg);
        emit(to, "    $P%d = pjs_find_lex_and_base @env_%d, '%s', $P%d\n", 
            funcReg, naming.scopeNo, expr->u.funcall.fun->u.strVal, thisReg);

        if (doReturn) {
#if DO_TAIL_CALL
            emit(to, "    .return 'pjs_call_function'($P%d, $P%d, @env_%d", thisReg, funcReg, naming.scopeNo);
            emit_func_params(startReg, nParams, naming.scopeNo, to);
#else
            emit(to, "    $P%d = 'pjs_call_function'($P%d, $P%d, @env_%d", thisReg, thisReg, funcReg, naming.scopeNo);
            emit_func_params(startReg, nParams, naming.scopeNo, to);
            emit(to, "    .return ($P%d)\n", thisReg);
#endif /* DO_TAIL_CALL */
        } else {
            emit(to, "    $P%d = 'pjs_call_function'($P%d, $P%d, @env_%d", destReg, thisReg, funcReg, naming.scopeNo);
            emit_func_params(startReg, nParams, naming.scopeNo, to);
        }
    } else { /* function call from an expression (e.g. (getSomeFunction())() */
        int thisReg = naming.regNo++;
        funcReg = naming.regNo++;
        emit(to, "    $P%d = pjs_get_global @env_%d\n", thisReg, naming.scopeNo);
        emit_expr(expr->u.funcall.fun, funcReg, naming, to);
        if (doReturn) {
#if DO_TAIL_CALL
            emit(to, "    .return 'pjs_call_function'($P%d, $P%d, @env_%d", thisReg, funcReg, naming.scopeNo);
            emit_func_params(startReg, nParams, naming.scopeNo, to);
#else
            emit(to, "    $P%d = 'pjs_call_function'($P%d, $P%d, @env_%d", thisReg, thisReg, funcReg, naming.scopeNo);
            emit_func_params(startReg, nParams, naming.scopeNo, to);
            emit(to, "    .return ($P%d)\n", thisReg);
#endif /* DO_TAIL_CALL */
        } else {
            emit(to, "    $P%d = 'pjs_call_function'($P%d, $P%d, @env_%d", destReg, thisReg, funcReg, naming.scopeNo);
            emit_func_params(startReg, nParams, naming.scopeNo, to);
        }
    }
}

static int isControlStructure(Node stmt) {
    int kind = stmt->kind;
    return  kind == EFor_stm     || 
            kind == EForIn_stm   || 
            kind == EWith_stm    ||
            kind == EDoWhile_stm ||
            kind == ESwitch_stm;
}

void emit_stmt(Node stmt, int destReg, Naming naming, parrot_data* to) {
    if (stmt->stmtLabels && ! isControlStructure(stmt)) {
        stmt->labelNo = (*naming.labelNo)++;
        
        /* never needed (no continue inside a non-loop!) */
        /* emit(to, "  CONTINUE@%d:\n", stmt->labelNo); */
    }
    
    switch(stmt->kind) {
    case EFunDec_stm: 
        break;
    case EBlock_stm:
        emit_stmtList(stmt->u.stmtList, destReg, naming, to);
        break;
    case EVardec_stm:
    {   emit_vardecList_assignment(stmt->u.nameValueList, naming, to);
        if (destReg >= 0)
            emit_undefined(destReg, naming, to);
    }
        break;
    case EIfElse_stm:
    {   int condReg = naming.regNo++;
        int labelNo = (*naming.labelNo)++;
        
        emit(to, "# <if %d>\n", labelNo);
        emit_expr(stmt->u.ifElse.cond, condReg, naming, to);
        
        if (stmt->u.ifElse.ifFalse) {
            /* if with else */
            emit(to, "    unless $P%d goto ELSE@%d\n", condReg, labelNo);
            emit_stmt(stmt->u.ifElse.ifTrue, destReg, naming, to);
            emit(to, "    goto END_IF@%d\n", labelNo);
            emit(to, "  ELSE@%d:\n", labelNo);
            emit_stmt(stmt->u.ifElse.ifFalse, destReg, naming, to);
            emit(to, "  END_IF@%d:\n", labelNo);
        } else {
            /* if without else */
            if (destReg != -1) {
                emit(to, "    unless $P%d goto ELSE@%d\n", condReg, labelNo);
                emit_stmt(stmt->u.ifElse.ifTrue, destReg, naming, to);
                emit(to, "    goto END_IF@%d\n", labelNo);
                emit(to, "  ELSE@%d:\n", labelNo);
                emit_undefined(destReg, naming, to);
                emit(to, "  END_IF@%d:\n", labelNo);
            } else {
                emit(to, "    unless $P%d goto END_IF@%d\n", condReg, labelNo);
                emit_stmt(stmt->u.ifElse.ifTrue, destReg, naming, to);
                emit(to, "  END_IF@%d:\n", labelNo);
            }
        }
    }
        break;
    case EWhile_stm:
    {   int condReg = naming.regNo++;
        int labelNo = (*naming.labelNo)++;
        stmt->labelNo = labelNo;
        if (destReg >= 0) {
            emit_undefined(destReg, naming, to);
        }
        emit(to, "  CONTINUE@%d: # <while>\n", labelNo);
        emit_expr(stmt->u.whileLoop.cond, condReg, naming, to);
        emit(to, "    unless $P%d goto BREAK@%d\n", condReg, labelNo);
        emit_stmt(stmt->u.whileLoop.stmt, destReg, naming, to);
        emit(to, "    goto CONTINUE@%d\n", labelNo);
        emit(to, "  BREAK@%d: # end <while>\n", labelNo);
    }
        break;
    case EDoWhile_stm:
    {   int condReg = naming.regNo++;
        int labelNo = (*naming.labelNo)++;
        stmt->labelNo = labelNo;
        if (destReg >= 0) {
            emit_undefined(destReg, naming, to);
        }
        emit(to, "  CONTINUE@%d: # <do while>\n", labelNo);
        emit_stmt(stmt->u.whileLoop.stmt, destReg, naming, to);
        emit_expr(stmt->u.whileLoop.cond, condReg, naming, to);
        emit(to, "    if $P%d goto CONTINUE@%d\n", condReg, labelNo);
        emit(to, "  BREAK@%d: # end <do while>\n", labelNo);
    }
        break;
    case EFor_stm:
    {   int condReg = naming.regNo++;
        int labelNo = (*naming.labelNo)++;
        int unneededReg = naming.regNo++;
        stmt->labelNo = labelNo;
        
        emit(to, "# <for %d>\n", labelNo);
        
        if (destReg >= 0) {
            emit_undefined(destReg, naming, to);
        }
        if(stmt->u.forLoop.init) {
            if (stmt->u.forLoop.init->kind == EVardec_stm) {
                emit_vardecList_assignment(stmt->u.forLoop.init->u.nameValueList, naming, to);
            } else {
                emit_expr(stmt->u.forLoop.init, unneededReg, naming, to);
            }
        }
        
        if(stmt->u.forLoop.next)
            emit(to, "    goto FIRSTLOOP@%d\n", labelNo);
        
        emit(to, "  CONTINUE@%d: # <for>\n", labelNo);
        
        if(stmt->u.forLoop.next) {
            emit_expr(stmt->u.forLoop.next, unneededReg, naming, to);
            emit(to, "  FIRSTLOOP@%d: # <for>\n", labelNo);
        }
            
        if (stmt->u.forLoop.cond) {
            emit_expr(stmt->u.forLoop.cond, condReg, naming, to);
            emit(to, "    unless $P%d goto BREAK@%d\n", condReg, labelNo);
        }
        emit_stmt(stmt->u.forLoop.stmt, destReg, naming, to);
        
        emit(to, "    goto CONTINUE@%d\n", labelNo);
        emit(to, "  BREAK@%d:\n", labelNo);
        emit(to, "# end <for %d>\n", labelNo);
    }
        break;
    case EForIn_stm:
    {   int collectionReg = naming.regNo++;
        int indexReg = naming.regNo++;
        int labelNo = (*naming.labelNo)++;
        stmt->labelNo = labelNo;
        
        emit(to, "# <for in %d>\n", labelNo);
        
        if (destReg >= 0) {
            emit_undefined(destReg, naming, to);
        }
        if (stmt->u.forInLoop.isVarDec || 
            stmt->u.forInLoop.var->kind == EIdentifier) {
            
            char* varName = NULL;
            if (stmt->u.forInLoop.isVarDec) {
                NameValue nv = stmt->u.forInLoop.newVar;
                varName = nv->name;
                if (nv->value) {
                    emit_expr(nv->value, indexReg, naming, to);
                    emit(to, "    pjs_store_lex $P%d, @env_%d, '%s'\n", 
                        indexReg, naming.scopeNo, varName);
                }
            } else {
                varName = stmt->u.forInLoop.var->u.strVal;
            }
            emit_expr(stmt->u.forInLoop.collection, collectionReg, naming, to);
            emit(to, "    $P%d = '__toObject__'($P%d, @env_%d)\n", collectionReg, collectionReg, naming.scopeNo);
            emit(to, "    $P%d = iter $P%d\n", collectionReg, collectionReg);
            emit(to, "  CONTINUE@%d: # <for in>\n", labelNo);
            emit(to, "    unless $P%d goto BREAK@%d\n", collectionReg, labelNo);
            emit(to, "    $S%d = shift $P%d\n", indexReg, collectionReg);
            emit(to, "    $P%d = new 'PjsString'\n", indexReg);
            emit(to, "    $P%d = $S%d\n", indexReg, indexReg);
            emit(to, "    pjs_store_lex $P%d, @env_%d, '%s'\n", 
                        indexReg, naming.scopeNo, varName);
            emit_stmt(stmt->u.forInLoop.stmt, destReg, naming, to);
            emit(to, "    goto CONTINUE@%d\n", labelNo);
            emit(to, "  BREAK@%d:\n", labelNo);
            emit(to, "# end <for in %d>\n", labelNo);
        } else {
            /* TODO for (x[i] in arr) {} */
            /* for(f() in arr) not supported */
        }
    }
        break;
        
    case EContinue_stm:
        assert( stmt->u.continueBreak.brokenStmt &&
                stmt->u.continueBreak.brokenStmt->labelNo != -1);
        if (destReg >= 0) {
            emit_undefined(destReg, naming, to);
        }
        emit(to, 
                "    goto CONTINUE@%d\n",
                stmt->u.continueBreak.brokenStmt->labelNo);
        break;
        
    case EBreak_stm:
        assert( stmt->u.continueBreak.brokenStmt &&
                stmt->u.continueBreak.brokenStmt->labelNo != -1);
        if (destReg >= 0) {
            emit_undefined(destReg, naming, to);
        }
        emit(to, 
                "    goto BREAK@%d\n",
                stmt->u.continueBreak.brokenStmt->labelNo);
        break;
        
    case EReturn_stm:
    {   int reg;
        if (destReg >= 0)
            reg = destReg;
        else
            reg = naming.regNo++;
            
        if (stmt->u.expr) {
            if (stmt->u.expr->kind == EFuncall) {
                /* emit returning funcall (tail call) */
                emit_funcall(stmt->u.expr, -1, 1, naming, to);
            } else {
                emit_expr(stmt->u.expr, reg, naming, to);
                emit(to, "    .return ($P%d)\n", reg);
            }
            
        } else {
            /*emit_undefined(reg, naming, to);
            emit(to, "    .return ($P%d)\n", reg);*/
            emit(to, "    .return (@undefined)\n");
        }
    }
        break;
    case EWith_stm:
    {   int scopObjReg = naming.regNo++;
    
        emit(to, "# <with> (new env = @env_%d)\n", naming.scopeNo+1);
        if (destReg >= 0) {
            emit_undefined(destReg, naming, to);
        }
        emit_expr(stmt->u.with.scope, scopObjReg, naming, to);
        emit(to, "    $P%d = '__toObject__'($P%d, @env_%d)\n", scopObjReg, scopObjReg, naming.scopeNo);
        emit(to, "    .local pmc @env_%d\n", naming.scopeNo+1);
        emit(to, "    @env_%d = pjs_augment_scope_chain_with @env_%d, $P%d\n",
                                naming.scopeNo+1, naming.scopeNo, scopObjReg);
        naming.scopeNo++;
        emit_stmt(stmt->u.with.stmt, destReg, naming, to);
        emit(to, "# end <with>\n");
    }
        break;
    case EThrow_stm:
    {   int jsExReg = naming.regNo++;
        int parExReg = naming.regNo++;
        emit(to, "# <throw>\n");
        emit_expr(stmt->u.expr, jsExReg, naming, to);
        emit(to, "    $P%d = new 'Exception'\n", parExReg);
        emit(to, "    $P%d[%d] = $P%d\n", parExReg, EXCEPTION_JSOBJ_INDEX, jsExReg);
        emit(to, "    $S%d = $P%d\n", jsExReg, jsExReg);
        emit(to, "    $P%d[%d] = $S%d\n", parExReg, EXCEPTION_MSG_INDEX, jsExReg);
        emit(to, "    throw $P%d\n", parExReg);
        emit(to, "# end <throw>\n");
    }
        break;
    case ETry_stm:
        if (stmt->u.tryCatchFinally.catchBlock && stmt->u.tryCatchFinally.finallyBlock) {
            PjsList finallyBlock = stmt->u.tryCatchFinally.finallyBlock;
            PjsList tryBlock = newList(0);
            stmt->u.tryCatchFinally.finallyBlock = 0;
            pushBack(stmt, tryBlock, 0);
            if (destReg >= 0) {
                emit_undefined(destReg, naming, to);
            }
            emitTryFinally(tryBlock, finallyBlock, -1, naming, to);
            freeList(tryBlock);
        } else if (stmt->u.tryCatchFinally.catchBlock) {
            emitTryCatch( stmt->u.tryCatchFinally.tryBlock, 
                          stmt->u.tryCatchFinally.catchVar,
                          stmt->u.tryCatchFinally.catchBlock, 
                          destReg, naming, to);
        } else {
            assert(stmt->u.tryCatchFinally.finallyBlock);
            emitTryFinally( stmt->u.tryCatchFinally.tryBlock, 
                            stmt->u.tryCatchFinally.finallyBlock, 
                            destReg, naming, to);
        }
        break;
    case ESwitch_stm:
    {   int labelNo = (*naming.labelNo)++;
        int searchedForReg = naming.regNo++;
        int comparedToReg = naming.regNo++;
        int i;
        ListEntry entry;
        stmt->labelNo = labelNo;
        
        emit(to, "# <switch %d>\n", labelNo);
        
        emit_expr(stmt->u.switchStm.expr, searchedForReg, naming, to);
        for (   entry = stmt->u.switchStm.caseList->start, i = 0;
                entry;
                entry = entry->next, i++ ) {
            CaseClause cc = (CaseClause) entry->elem;
            int compareResultReg;
            emit_expr(cc->expr, comparedToReg, naming, to);
            compareResultReg = naming.regNo;
            emit_strict_binop(searchedForReg, comparedToReg, STRICT_EQ_OP,
                    compareResultReg, naming, to);
            emit(to, "    if $P%d goto CASE_%d@%d\n", compareResultReg, i, labelNo);
        }
        if (stmt->u.switchStm.defaultStmtList) {
            emit(to, "    goto CASE_DEFAULT@%d\n", labelNo);
        } else {
            emit(to, "    goto BREAK@%d\n", labelNo);
        }
        
        for (   entry = stmt->u.switchStm.caseList->start, i = 0;
                entry;
                entry = entry->next, i++ ) {
            CaseClause cc = (CaseClause) entry->elem;
            emit(to, "  CASE_%d@%d:\n", i, labelNo);
            if (cc->stmtList)
                emit_stmtList(cc->stmtList, destReg, naming, to);
        }
        if (stmt->u.switchStm.defaultStmtList) {
            emit(to, "  CASE_DEFAULT@%d:\n", labelNo);
            emit_stmtList(stmt->u.switchStm.defaultStmtList, destReg, naming, to);
        }
        emit(to, "  BREAK@%d:\n", labelNo);
        emit(to, "# end <switch %d>\n", labelNo);
    }
        break;
    case EPirCode_stm:
    {   emit(to, "# start PIR\n%s\n# end PIR\n", stmt->u.strVal);
    }
        break;

    case EThis:
    case ENull:
    case EBoolean:
    case EElision:
    case EIdentifier:
    case EString:
    case ENumeric:
    case EArrayLit:
    case EObjectLit:
    case ERegex:
    case EIndexedAccess:
    case EPropAccess:
    case ENewFuncall:
    case EFuncall:
    case EBinop:
    case EUnop:
    case ETernop:
    case EAssign:
    case ECompound:
    case EFunExpr:
    {   if (destReg < 0) {
            int reg = naming.regNo++;
            emit_expr(stmt, reg, naming, to);
        } else {
            emit_expr(stmt, destReg, naming, to);
        }
    }
        break;
    default: 
        emit(to, "# UNKNOWN STATEMENT! \n");
        break;
    }
    /* write end-labels for that stmt */
    if (stmt->stmtLabels && ! isControlStructure(stmt)) {
        emit(to, "  BREAK@%d:\n", stmt->labelNo);
    }
}

void emit_expr(Node expr, int destReg, Naming naming, parrot_data* to) {
    if (! expr) {
        emit(to, "## EXPRESSION WAS NULL!\n");
    }
    switch(expr->kind) {
    case EIdentifier:
        emit(to, "    $P%d = pjs_find_lex @env_%d, '%s'\n", destReg, naming.scopeNo, expr->u.strVal);
        return;
    case EThis:
        emit(to, "    $P%d = @this\n", destReg);
        return;
    case ENull:
        emit(to, "    $P%d = pjs_find_lex @env_%d, 'null'\n", destReg, naming.scopeNo);
        return;
    case EBoolean:
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        emit(to, "    $P%d = %d\n", destReg, expr->u.boolVal ? 1 : 0);
        return;
    case EString:
    {   char* escaped = escape_pir_string(expr->u.strVal);
        emit(to, "    $P%d = new 'PjsString'\n", destReg);
        emit(to, "    $P%d = utf8:unicode:\"%s\"\n", destReg, escaped);
        free(escaped);
    }
        return;
    case ENumeric:
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    $P%d = '%s'\n", destReg, (char*) expr->u.doubleVal);
        return;
    case EArrayLit: 
    {
        int elemReg = naming.regNo++;
        int indexReg = naming.regNo++;
        
        ListEntry entry;
        int elemNo;
        
        emit_newObject(destReg, naming, to);
        emit(to, "    $P%d = pjs_find_lex @env_%d, 'Array'\n", 
            destReg, naming.scopeNo);
        emit(to, "    $P%d = 'pjs_invoke_new'($P%d, @env_%d)\n", 
            destReg, destReg, naming.scopeNo);
        
        emit(to, "    $P%d = new 'PjsNumber'\n", indexReg);
        for (   entry = expr->u.exprList->start, elemNo = 0; 
                entry; 
                entry = entry->next, elemNo++) {
            Node elem = (Node) entry->elem;
            if (elem)
                emit_expr(elem, elemReg, naming, to);
            else
                emit_undefined(elemReg, naming, to);
            emit(to, "    $P%d = %d\n", indexReg, elemNo);
            emit(to, "    $P%d[$P%d] = $P%d\n", destReg, indexReg, elemReg);
        }
    }
        return;
    case EObjectLit:
    {   int elemReg = naming.regNo++;
        ListEntry entry;
        emit_newObject(destReg, naming, to);
        
        for (entry = expr->u.nodePairList->start; entry; entry = entry->next) {
            NodePair pair = (NodePair) entry->elem;
            emit_expr(pair->second, elemReg, naming, to);
            if (pair->first->kind == EIdentifier) {
                emit(to, "    $P%d['%s'] = $P%d \n", 
                            destReg, pair->first->u.strVal, elemReg);
            } else if (pair->first->kind == EString) {
                char* escaped = escape_pir_string(pair->first->u.strVal);
                emit(to, "    $P%d['%s'] = $P%d \n", 
                            destReg, escaped, elemReg);
                free(escaped);
            } else {
                int indexReg = naming.regNo++;
                assert(pair->first->kind == ENumeric);
                emit_expr(pair->first, indexReg, naming, to);
                emit(to, "    $P%d[$P%d] = $P%d \n", destReg, indexReg, elemReg);
            }
        }
    }
        return;
    case EFuncall:
        emit_funcall(expr, destReg, 0, naming, to);
        return;
    case ENewFuncall:
    {   int nParams, i, funcReg;
        int startReg = naming.regNo;
        ListEntry entry;
        for (entry = expr->u.funcall.args->start; entry; entry = entry->next) {
            Node arg = (Node) entry->elem;
            int reg = naming.regNo++;
            emit_expr(arg, reg, naming, to);
        }
        nParams = naming.regNo - startReg;
        funcReg = naming.regNo++;
        emit_expr(expr->u.funcall.fun, funcReg, naming, to);
        emit(to, "    $P%d = 'pjs_invoke_new'($P%d, @env_%d", destReg, funcReg, naming.scopeNo);
        for (i = 0; i<nParams; i++) {
            emit(to, ", $P%d", startReg + i);
        }
        emit(to, ")\n");
    }
        return;
    case ERegex:
        /* TODO */
        emit(to, "# TODO: regex: %s\n", expr->u.strVal);
        emit(to, "    $P%d = new 'Exception'\n", destReg);
        emit(to, "    $S%d = 'Regular expressions not implemented (regex=%s)'\n", destReg, expr->u.strVal);
        emit(to, "    $P%d[%d] = $S%d\n", destReg, EXCEPTION_MSG_INDEX, destReg);
        emit(to, "    throw $P%d\n", destReg);
        return;
    case EPropAccess:
    {   int contReg = naming.regNo++;
        Node base = expr->u.getprop.base;
        char* propname = expr->u.getprop.propName;
        emit_expr(base, contReg, naming, to);
        emit(to, "    $P%d = '__toObject__'($P%d, @env_%d)\n", contReg, contReg, naming.scopeNo);
        emit(to, "    $P%d = getattribute $P%d, '%s'\n", destReg, contReg, propname);
        return;
    }
    case EIndexedAccess:
    {   int contReg = naming.regNo++;
        int propNameReg = naming.regNo++;
        Node base = expr->u.getindexed.base;
        Node index = expr->u.getindexed.index;
        emit_expr(base, contReg, naming, to);
        emit(to, "    $P%d = '__toObject__'($P%d, @env_%d)\n", contReg, 
            contReg, naming.scopeNo);
        emit_expr(index, propNameReg, naming, to);
        emit(to, "    $P%d = pjs_to_primitive $P%d\n", propNameReg, propNameReg);
        emit(to, "    $P%d = $P%d[$P%d] \n", destReg, contReg, propNameReg);
        return;
    }
    case EBinop:
        emit_binop(expr->u.binop.left, expr->u.binop.right, expr->u.binop.op, 
            destReg, naming, to);
        return;
    case EUnop:
        emit_unop(expr, expr->u.unop.op, destReg, naming, to);
        return;
    case ETernop:
    {   int labelNo = (*naming.labelNo)++;
        emit_expr(expr->u.ternop.cond, destReg, naming, to);
        emit(to, "    unless $P%d goto TERNOP_ELSE@%d\n", destReg, labelNo);
        emit_expr(expr->u.ternop.ifTrue, destReg, naming, to);
        emit(to, "    goto TERNOP_END@%d\n", labelNo);
        emit(to, " TERNOP_ELSE@%d:\n", labelNo);
        emit_expr(expr->u.ternop.ifFalse, destReg, naming, to);
        emit(to, " TERNOP_END@%d:\n", labelNo);
    }
        return;
    case EAssign:
        emit_assignment(expr, destReg, naming, to);
        return;
    case ECompound:
        emit_expr(expr->u.exprPair.left, destReg, naming, to);
        emit_expr(expr->u.exprPair.right, destReg, naming, to);
        return;
    case EFunExpr:
    {   if (! expr->u.function.name) {
            emit_jsfunc_creation(expr, destReg, naming, to);
        } else {
            int hashReg = naming.regNo++;
            Naming newNaming = naming;
            newNaming.scopeNo++;
            emit(to, "    .local pmc @env_%d\n", newNaming.scopeNo);
            emit(to, "    $P%d = new 'Hash'\n", hashReg);
            emit(to, "    @env_%d = pjs_augment_scope_chain_with @env_%d, $P%d\n", 
                newNaming.scopeNo, naming.scopeNo, hashReg);
            emit_jsfunc_creation(expr, destReg, newNaming, to);
            emit(to, "    $P%d['%s'] = $P%d\n", 
                                hashReg, expr->u.function.name, destReg);
        }
    }
        return;
    default: 
        /* TODO real_exception() */
        return;
    }
}
void emit_jsfunc_creation(Node func, int destReg, Naming naming, parrot_data* to) {

    emit(to, "    .const .Sub %s = '%s'\n", func->u.function.pir_subroutine_name, 
                                            func->u.function.pir_subroutine_name);
    
    emit(to, "    $P%d = 'create_function'(%s, @env_%d, %d)\n", 
                                    destReg,
                                    func->u.function.pir_subroutine_name, 
                                    naming.scopeNo, 
                                    pjslist_length(func->u.function.params));
}

static void emit_fundecList(PjsList stmts, Naming naming, parrot_data* to) {
    ListEntry entry;
    
    if (! (stmts && stmts-> start))
        return;
    for ( entry = stmts->start;
          entry;
          entry = entry->next ) {
        Node func = (Node) entry->elem;
        int funcReg = naming.regNo++;
        emit(to, "    pjs_new_lex_with_flags @env_%d, '%s', %d\n", 
            naming.scopeNo, func->u.function.name,
            PJS_HASH_ENTRY__DONT_DELETE);
        emit_jsfunc_creation(func, funcReg, naming, to);
        emit(to, "    pjs_store_lex $P%d, @env_%d, '%s'\n", funcReg, naming.scopeNo, func->u.function.name);
    }
}

static void emit_params(PjsList params, Naming naming, parrot_data* to) {
    ListEntry entry;
    
    emit(to, "    .param pmc @this\n");
    emit(to, "    .param pmc @env_0\n");
    emit(to, "    .param pmc @dyn_env\n");
    for (entry = params->start; entry; entry = entry->next) {
        char* escapedParam = escapeForLabel((char*) entry->elem);
        emit(to, "    .param pmc par@%s :optional\n", escapedParam);
        emit(to, "    .param int has@%s :opt_flag\n", escapedParam);
        free(escapedParam);
    }
    emit(to, "    .param pmc @rest :slurpy\n\n");
    
    /* store 'this' in the environment */
    /* flag don't delete and don't enum */
    emit(to, "    pjs_new_lex_with_flags @env_%d, 'this', %d\n", 
        naming.scopeNo,
        PJS_HASH_ENTRY__DONT_ENUM | PJS_HASH_ENTRY__DONT_DELETE);
    emit(to, "    pjs_store_lex @this, @env_%d, 'this'\n", naming.scopeNo);
    emit(to, "\n");

    /* Give the not passed in parameters the value of undefined */
    emit(to, "    .local pmc @undefined\n");
    emit(to, "    @undefined = new 'PjsUndefined'\n\n");
    for (entry = params->start; entry; entry = entry->next) {
        char* escapedParam = escapeForLabel((char*) entry->elem);
        emit(to, "    if has@%s goto @default_val_%d\n", escapedParam, *naming.labelNo);
        emit(to, "    par@%s = @undefined\n", escapedParam);
        emit(to, "  @default_val_%d:\n", *naming.labelNo);
        ++(*naming.labelNo);
        free(escapedParam);
    }
    emit(to, "\n");
    
    /* Store the parameters in the environment */
    for (entry = params->start; entry; entry = entry->next) {
        char* param = (char*) entry->elem;
        char* escapedParam = escapeForLabel(param);
        emit(to, "    pjs_new_lex_with_flags @env_%d, '%s', %d\n", 
            naming.scopeNo, param,
            PJS_HASH_ENTRY__DONT_DELETE);
        emit(to, "    pjs_store_lex par@%s, @env_%d, '%s'\n", escapedParam, naming.scopeNo, param);
        free(escapedParam);
    }
    emit(to, "\n");
}

void emit_vardecList_declaration(PjsList list, Naming naming, parrot_data* to) {
    ListEntry entry;
    for (entry = list->start; entry; entry = entry->next) {
        char* name = (char*) entry->elem;
        emit(to, "    pjs_new_lex_with_flags @env_%d, '%s', %d\n",
            naming.scopeNo, name,
            PJS_HASH_ENTRY__DONT_DELETE);
    }
}
void emit_vardecList_assignment(PjsList nameValueList, Naming naming, parrot_data* to) {
    ListEntry entry;
    int reg = naming.regNo++;
    for (entry = nameValueList->start; entry; entry = entry->next) {
        NameValue nv = (NameValue) entry->elem;
        if (nv->value) {
            emit_expr(nv->value, reg, naming, to);
            emit(to, "    pjs_store_lex $P%d, @env_%d, '%s'\n", reg, naming.scopeNo, nv->name);
        }
    }
    
}


void emit_func(Node func, int doEval, parrot_data* to) {
    int labelNo = 0;
    int returnReg;
    char* func_name = func->u.function.pir_subroutine_name;
    Naming naming;
    naming.regNo = 0;
    naming.scopeNo = 0;
    naming.labelNo = &labelNo;
    returnReg = naming.regNo++;
    
    if (func->u.function.name) {
        emit(to, "## function %s\n", func->u.function.name);
    }
    emit(to, ".sub %s :anon\n", func_name);
    
    emit_params(func->u.function.params, naming, to);
    emit_fundecList(func->u.function.funDecList, naming, to);
    if (func->u.function.vardecList) {
        emit_vardecList_declaration(func->u.function.vardecList, naming, to);
    }

    if (doEval) {
        emit_stmtList( func->u.function.stmtList, 
                       returnReg, 
                       naming, to);
        emit(to, "    .return ($P%d)\n", returnReg);
    } else { /* return undefined */
        emit_stmtList( func->u.function.stmtList, 
                       -1, 
                       naming, to);
        emit(to, "\n");
        /*emit_undefined(returnReg, naming, to);
        emit(to, "    .return ($P%d)\n", returnReg);*/
        emit(to, "    .return (@undefined)\n", returnReg);
    }
    
    emit(to, ".end\n\n\n");
}

void emit_all_funcs(Node root, int doEvalMain, parrot_data* to) {
    ListEntry func;
    emit_func(root, doEvalMain, to);
    for (func = root->u.function.innerFuncs->start; func; func = func->next) {
        emit_all_funcs((Node) func->elem, 0, to);
    }
}

extern int pjs_yylex_init(void**);
extern int pjs_yylex_destroy(void*);
extern void pjs_yy_scan_string(char* str, void* scanner);
extern void pjs_yyset_extra(Params* p, void* scanner);
extern int pjs_yyparse(Params* p);

static void generate_subroutine_names(Node root, char* outer_func_name, parrot_data* to) {
    ListEntry func;
    root->u.function.pir_subroutine_name = outer_func_name;
    
    for (func = root->u.function.innerFuncs->start; func; func = func->next) {
        Node f = (Node) func->elem;
        char* func_name = createName(outer_func_name, f->u.function.functionNr, to->mempool);
        f->u.function.pir_subroutine_name = func_name;        
        generate_subroutine_names(f, func_name, to);
    }
}

static void emitMain(Node root, parrot_data* to) {
    int nextReg;
    
    emit(to, ".HLL 'Pjs', 'pjs_group'\n");
    emit(to, ".loadlib 'pjs_group_ops'\n");
    
    emit(to, ".sub @main :main :load :anon\n");

    emit(to, "    load_bytecode 'languages/pjs/lib/jscore.pbc'\n\n");
    
    emit(to, "    .local pmc global_env, global_obj\n");
    emit(to, "    global_env = get_hll_global 'global_env'\n");
    emit(to, "    global_obj = pjs_get_scope_object global_env\n\n");
    
    emit(to, "    .const .Sub @func_0 = '@func_0'\n");
    
    nextReg = 1;
    generate_subroutine_names(root, "@func_0", to);
    emit(to, "    @func_0(global_obj, global_env, global_env)\n");

    emit(to, ".end\n\n\n## MAIN\n");
}

static void emitProgramGetter(Node root, parrot_data* to) {
    int nextReg;
    
    emit(to, ".HLL 'Pjs', 'pjs_group'\n");
    emit(to, ".loadlib 'pjs_group_ops'\n");
    
    emit(to, ".sub @main :main :anon\n");
    
    emit(to, "    .const .Sub @func_0 = '@func_0'\n");
    
    nextReg = 1;
    generate_subroutine_names(root, "@func_0", to);
    emit(to, "    .return (@func_0)\n");
    emit(to, ".end\n\n\n## MAIN\n");
}

int js2pir(char* str, int isMain, parrot_data* to) {
    void* scanner;
    Params* p;
    int ret;
    
    pjs_yylex_init(&scanner);
    pjs_yy_scan_string(str, scanner);
    
    p = (Params*) malloc(sizeof(Params));
    resetParams(p);
    p->scanner = scanner;
    p->mempool = mp_newpool(1);
    to->mempool = p->mempool;
    
    pjs_yyset_extra(p, scanner);
    ret = pjs_yyparse(p);
    if (ret != 0) {
        real_exception(to->interp, NULL, E_SyntaxError, 
                "%s", p->err_msg);
    } else {
        visit(p->program, 0, 0, p->mempool);
        if (isMain) {
            emitMain(p->program, to);
        } else {
            emitProgramGetter(p->program, to);
        }
        emit_all_funcs(p->program, ! isMain, to);
    }
    mp_freepool(p->mempool);
    pjs_yylex_destroy(scanner);
    free(p);
    return 0;
}
