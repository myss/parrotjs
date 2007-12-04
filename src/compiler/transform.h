
#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include "nodes.h"
#include "mempool.h"

/*
 * After visiting each node, the next should be accomplished:
 *  - each function contains a list of innerfunctions
 *  - each loop block contains a pointer to the parent loop block
 *  - each loop block contains a list of labels, labels are removed from the tree
 *  - variable declarations are placed at the start of the function
 *  - FuncDec_stmt's are placed at the start of the function
 */
void visit(Node node, Node parentFunc, Node parentBlock, Mempool MP);

#endif
