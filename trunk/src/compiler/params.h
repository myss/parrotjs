#ifndef _PARAMS_H_
#define _PARAMS_H_

#include "mempool.h"
#include "nodes.h"

typedef struct Params Params;

struct Params {
    void* scanner;
    
    int match_len;
    int line_no, column_no, prev_line_no, prev_column_no;
    int is_newline, prev_was_newline;
    int prev_was_r;
    Node program; /* nodetype = EFunDec_stm */
    Mempool mempool;
    char err_msg[512];
};

void resetParams(Params* p);

#endif
