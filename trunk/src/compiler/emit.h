
#ifndef _PRETTY_PRINT_H
#define _PRETTY_PRINT_H

#include <stdio.h>
#include "list.h"
#include "nodes.h"
#include "mempool.h"
#include "parrot/parrot.h"

typedef struct parrot_data {
    Interp *interp;
    STRING *code;
    Mempool mempool;
} parrot_data;

void emit(parrot_data* to, const char* format, ...);



struct Naming {
    int regNo;
    int scopeNo;
    int *labelNo;
};
typedef struct Naming Naming;

/* Emits code that evaluates expr, and stores the result
 * in $P<destReg>. destReg should be non-negative.
 */
void emit_expr(Node expr, int destReg, Naming naming, parrot_data* to);

 /* Emits code that executes stmt, and stores the result
 * in $P<destReg> if destReg is not negative.
 */
void emit_stmt(Node stmt, int destReg, Naming naming, parrot_data* to);

void emit_func(Node func, int doEval, parrot_data* to);
void emit_all_funcs(Node root, int doEvalMain, parrot_data* to);

void emit_jsfunc_creation(Node func, int destReg, Naming naming, parrot_data* to);

void emit_undefined(int destReg, Naming naming, parrot_data* to);
void emit_vardecList_declaration(PjsList list, Naming naming, parrot_data* to);
void emit_vardecList_assignment(PjsList nameValueList, Naming naming, parrot_data* to);

int js2pir(char* str, int isMain, parrot_data* to);

void emit_strict_binop (
    int leftReg, int rightReg, 
    int op, int destReg,
    Naming naming, parrot_data* to
);
                       
void emit_binop(Node left, Node right, int op, int destReg, Naming naming, parrot_data* to);
void emit_unop(Node expr, int op, int destReg, Naming naming, parrot_data* to);
void emit_assignment(Node expr, int destReg, Naming naming, parrot_data* to);
#endif
