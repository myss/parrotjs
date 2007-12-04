
#include "params.h"
#include "mempool.h"

void resetParams(Params* p) {
    p->scanner = 0;
    p->match_len = 0;
    p->line_no = p->column_no = p->prev_line_no = p->prev_column_no = 0;
    p->is_newline = p->prev_was_newline = 0;
    p->prev_was_r = 0;
    p->program = 0;
    p->mempool = 0;
}
