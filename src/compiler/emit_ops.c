#include "emit.h"
#include "list.h"
#include "params.h"
#include "nodes.h"
#include "parser.tab.h"
#include "mempool.h"
#include "transform.h"
#include "parrot/exceptions.h"


static int
contains( const int array[], 
          const int elem, 
          const int size    ) {
    int i;
    for (i=0; i<size; i++) {
        if (array[i] == elem)
            return 1;
    }
    return 0;
}

/**
 * Examples: 
 *      x++
 *      y--
 *      ++z 
 *      delete x
 */
static void 
emit_unop_change_env( Node expr, 
                      int op, 
                      int destReg, 
                      Naming naming, 
                      parrot_data* to     ) {
    int reg = naming.regNo++;
    char* varName;
    varName = expr->u.unop.expr->u.strVal;
    switch (op) {
    case DELETE: /* TODO */
    {   /* @env_0[0] == the scope hash */
        int label = (*naming.labelNo)++;
        emit(to, "    $P%d = @env_%d[0]\n", reg, naming.scopeNo);
        emit(to, "    $I%d = can $P%d, 'deleteProperty_str'\n", reg, reg);
        emit(to, "    unless $I%d goto delete@else_%d\n", reg, label);
        emit(to, "    $I%d = $P%d.'deleteProperty_str'('%s')\n", destReg, reg, varName);
        emit(to, "    goto delete@end_%d\n", label);
        emit(to, "  delete@else_%d:\n", label);
        emit(to, "    delete $P%d['%s']\n", reg, varName);
        emit(to, "    $I%d = exists $P%d['%s']\n", destReg, reg, varName);
        emit(to, "    $I%d = ! $I%d\n", destReg, destReg);
        emit(to, "  delete@end_%d:\n", label);
        
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        emit(to, "    $P%d = $I%d\n", destReg, destReg);
    }
        break;
    case PRE_INC_OP:
        emit_expr(expr->u.unop.expr, reg, naming, to);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    inc $P%d\n", destReg);
        emit(to, "    pjs_store_lex $P%d, @env_%d, '%s'\n", destReg, naming.scopeNo, varName);
        break;
    case PRE_DEC_OP:
        emit_expr(expr->u.unop.expr, reg, naming, to);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    dec $P%d\n", destReg);
        emit(to, "    pjs_store_lex $P%d, @env_%d, '%s'\n", destReg, naming.scopeNo, varName);
        break;
    case POST_INC_OP:
        emit_expr(expr->u.unop.expr, destReg, naming, to);
        emit(to, "    $P%d = new 'PjsNumber'\n", reg);
        emit(to, "    assign $P%d, $P%d\n", reg, destReg);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    inc $P%d\n", reg);
        emit(to, "    pjs_store_lex $P%d, @env_%d, '%s'\n", reg, naming.scopeNo, varName);
        break;
    case POST_DEC_OP:
        emit_expr(expr->u.unop.expr, destReg, naming, to);
        emit(to, "    $P%d = new 'PjsNumber'\n", reg);
        emit(to, "    assign $P%d, $P%d\n", reg, destReg);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    dec $P%d\n", reg);
        emit(to, "    pjs_store_lex $P%d, @env_%d, '%s'\n", reg, naming.scopeNo, varName);
        break;
    }
}

/**
 * Examples: 
 *      obj.x++
 *      obj.y--
 *      ++obj.z 
 *      delete obj.x
 */
static void 
emit_unop_change_prop( Node expr,
                       int op, 
                       int destReg, 
                       Naming naming, 
                       parrot_data* to     ) {
    int contReg = naming.regNo++;
    int reg = naming.regNo++;
    char* propname = expr->u.unop.expr->u.getprop.propName;
    emit_expr(expr->u.unop.expr->u.getprop.base, contReg, naming, to);
    switch (op) {
    case DELETE:
    {
        emit(to, "    $I%d = $P%d.'deleteProperty'('%s')\n", destReg, contReg, propname);
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        emit(to, "    $P%d = $I%d\n", destReg, destReg);
    }
        break;
    case PRE_INC_OP:
        emit(to, "    $P%d = getattribute $P%d, '%s'\n", reg, contReg, propname);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    inc $P%d\n", destReg);
        emit(to, "    setattribute $P%d, '%s', $P%d\n", contReg, propname, destReg);
        break;
    case PRE_DEC_OP:
        emit(to, "    $P%d = getattribute $P%d, '%s'\n", reg, contReg, propname);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    dec $P%d\n", destReg);
        emit(to, "    setattribute $P%d, '%s', $P%d\n", contReg, propname, destReg);
        break;
    case POST_INC_OP:
        emit(to, "    $P%d = getattribute $P%d, '%s'\n", destReg, contReg, propname);
        emit(to, "    $P%d = new 'PjsNumber'\n", reg);
        emit(to, "    assign $P%d, $P%d\n", reg, destReg);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    inc $P%d\n", reg);
        emit(to, "    setattribute $P%d, '%s', $P%d\n", contReg, propname, reg);
        break;
    case POST_DEC_OP:
        emit(to, "    $P%d = getattribute $P%d, '%s'\n", destReg, contReg, propname);
        emit(to, "    $P%d = new 'PjsNumber'\n", reg);
        emit(to, "    assign $P%d, $P%d\n", reg, destReg);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    dec $P%d\n", reg);
        emit(to, "    setattribute $P%d, '%s', $P%d\n", contReg, propname, reg);
        break;
    }
}

/**
 * Examples: 
 *      obj[3]++
 *      obj['s']--
 *      ++obj[a] 
 *      delete obj[10]
 */
static void 
emit_unop_change_indexed( Node expr,
                          int op, 
                          int destReg, 
                          Naming naming, 
                          parrot_data* to     ) {
    int contReg = naming.regNo++;
    int propNameReg = naming.regNo++;
    int reg = naming.regNo++;
    emit_expr(expr->u.unop.expr->u.getindexed.base, contReg, naming, to);
    emit_expr(expr->u.unop.expr->u.getindexed.index, propNameReg, naming, to);
    switch (op) {
    case DELETE:
        emit(to, "    $I%d = $P%d.'deleteProperty'($P%d)\n", 
            destReg, contReg, propNameReg);
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        emit(to, "    $P%d = $I%d\n", destReg, destReg);
        break;
    case PRE_INC_OP:
        emit(to, "    $P%d = $P%d[$P%d] \n", reg, contReg, propNameReg);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    inc $P%d\n", destReg);
        emit(to, "    $P%d[$P%d] = $P%d \n", contReg, propNameReg, destReg);
        break;
    case PRE_DEC_OP:
        emit(to, "    $P%d = $P%d[$P%d] \n", reg, contReg, propNameReg);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    dec $P%d\n", destReg);
        emit(to, "    $P%d[$P%d] = $P%d \n", contReg, propNameReg, destReg);
        break;
    case POST_INC_OP:
        emit(to, "    $P%d = $P%d[$P%d] \n", destReg, contReg, propNameReg);
        emit(to, "    $P%d = new 'PjsNumber'\n", reg);
        emit(to, "    assign $P%d, $P%d\n", reg, destReg);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    inc $P%d\n", reg);
        emit(to, "    $P%d[$P%d] = $P%d \n", contReg, propNameReg, reg);
        break;
    case POST_DEC_OP:
        emit(to, "    $P%d = $P%d[$P%d] \n", destReg, contReg, propNameReg);
        emit(to, "    $P%d = new 'PjsNumber'\n", reg);
        emit(to, "    assign $P%d, $P%d\n", reg, destReg);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    dec $P%d\n", reg);
        emit(to, "    $P%d[$P%d] = $P%d \n", contReg, propNameReg, reg);
        break;
    }
}

/**
 * Examples: 
 *      void 10
 *      typeof x
 *      +10
 *      -'20'
 *      ! b
 *      ~n
 */
static void 
emit_unop_no_change( Node expr, 
                     int op, 
                     int destReg, 
                     Naming naming, 
                     parrot_data* to     ) {
    int reg = naming.regNo++;
    switch (op) {
    case VOID:
        emit_expr(expr->u.unop.expr, destReg, naming, to);
        emit_undefined(destReg, naming, to);
        break;
    case TYPEOF:
        emit_expr(expr->u.unop.expr, reg, naming, to);
        emit(to, "    $S%d = $P%d.'getJsType'()\n", destReg, reg);
        emit(to, "    $P%d = new 'PjsString'\n", destReg);
        emit(to, "    $P%d = $S%d\n", destReg, destReg);
        break;
    case '+':
        emit_expr(expr->u.unop.expr, reg, naming, to);
        emit(to, "    $P%d = pjs_to_primitive_hint_number $P%d\n", reg, reg);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        break;
    case '-':
        emit_expr(expr->u.unop.expr, reg, naming, to);
        emit(to, "    $P%d = pjs_to_primitive_hint_number $P%d\n", reg, reg);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    neg $P%d\n", destReg);
        break;
    case '!':
        emit_expr(expr->u.unop.expr, reg, naming, to);
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    not $P%d\n", destReg);
        break;
    case '~':
        emit_expr(expr->u.unop.expr, reg, naming, to);
        emit(to, "    $P%d = pjs_to_primitive_hint_number $P%d\n", reg, reg);
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, reg);
        emit(to, "    bnot $P%d\n", destReg);
        break;
    }
}

void 
emit_unop( Node expr, 
           int op, 
           int destReg, 
           Naming naming, 
           parrot_data* to   ) {
    static const int noChangeOperators[] = {
        VOID,  TYPEOF, '+', '-', '!', '~'
    };
    if (contains(noChangeOperators, op, sizeof(noChangeOperators)/sizeof(int))) {
        emit_unop_no_change(expr, op, destReg, naming, to);
    } else if (expr->u.unop.expr->kind == EPropAccess) {
        emit_unop_change_prop(expr, op, destReg, naming, to);
    } else if (expr->u.unop.expr->kind == EIndexedAccess) {
        emit_unop_change_indexed(expr, op, destReg, naming, to);
    } else if(expr->u.unop.expr->kind == EIdentifier) {
        emit_unop_change_env(expr, op, destReg, naming, to);
    } else {
        Parrot_ex_throw_from_c_args(to->interp, NULL, EXCEPTION_SYNTAX_ERROR, 
                "Unexpected operand for binary operation.");
    }
}

/**
 * Binary operators that evaluate both operands.
 * These are every binary operators except && and ||
 */
void
emit_strict_binop( int leftReg, 
                   int rightReg, 
                   int op, 
                   int destReg,
                   Naming naming,
                   parrot_data* to   ) {
    switch (op) {
    case '+':
        emit(to, "    $P%d = new 'PjsUndefined'\n", destReg);
        emit(to, "    $P%d = $P%d + $P%d \n", destReg, leftReg, rightReg);
        break;
    case '-':
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    $P%d = $P%d - $P%d \n", destReg, leftReg, rightReg);
        break;
    case '*':
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    $P%d = $P%d * $P%d \n", destReg, leftReg, rightReg);
        break;
    case '/':
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    $P%d = $P%d / $P%d \n", destReg, leftReg, rightReg);
        break;
    case '%':
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    $P%d = cmod $P%d, $P%d \n", destReg, leftReg, rightReg);
        break;
    case LEFT_OP:
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    $P%d = $P%d << $P%d \n", destReg, leftReg, rightReg);
        break;
    case RIGHT_OP:
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    $P%d = $P%d >> $P%d \n", destReg, leftReg, rightReg);
        break;
    case UNSIGNED_RIGHT:
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    $P%d = $P%d >>> $P%d \n", destReg, leftReg, rightReg);
        break;

    case '<':
        emit(to, "    $P%d = pjs_to_primitive_hint_number $P%d\n", leftReg, leftReg);
        emit(to, "    $P%d = pjs_to_primitive_hint_number $P%d\n", rightReg, rightReg);
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        
        emit(to, "    $I%d = $P%d < $P%d\n", destReg, leftReg, rightReg);
        emit(to, "    $P%d = $I%d \n", destReg, destReg);
        break;
    case '>':
        emit(to, "    $P%d = pjs_to_primitive_hint_number $P%d\n", leftReg, leftReg);
        emit(to, "    $P%d = pjs_to_primitive_hint_number $P%d\n", rightReg, rightReg);
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        
        emit(to, "    $I%d = $P%d > $P%d\n", destReg, leftReg, rightReg);
        emit(to, "    $P%d = $I%d \n", destReg, destReg);
        break;
    case LE_OP:
        emit(to, "    $P%d = pjs_to_primitive_hint_number $P%d\n", leftReg, leftReg);
        emit(to, "    $P%d = pjs_to_primitive_hint_number $P%d\n", rightReg, rightReg);
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        
        emit(to, "    $I%d = '__pjs_lte__'($P%d, $P%d)\n", destReg, leftReg, rightReg);
        emit(to, "    $P%d = $I%d \n", destReg, destReg);
        break;
    case GE_OP:
        emit(to, "    $P%d = pjs_to_primitive_hint_number $P%d\n", leftReg, leftReg);
        emit(to, "    $P%d = pjs_to_primitive_hint_number $P%d\n", rightReg, rightReg);
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        
        emit(to, "    $I%d = '__pjs_gte__'($P%d, $P%d)\n", destReg, leftReg, rightReg);
        emit(to, "    $P%d = $I%d \n", destReg, destReg);
        break;
    case IN:
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        emit(to, "    $I%d = '__pjs_in__'($P%d, $P%d)\n", destReg, leftReg, rightReg);
        emit(to, "    $P%d = $I%d \n", destReg, destReg);
        break;
    case INSTANCEOF:
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        emit(to, "    $I%d = '__pjs_instanceof__'(@env_%d, $P%d, $P%d)\n", destReg, naming.scopeNo, leftReg, rightReg);
        emit(to, "    $P%d = $I%d \n", destReg, destReg);
        break;
    case EQ_OP:
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        emit(to, "    $I%d = '__pjs_eq__'($P%d, $P%d)\n", destReg, leftReg, rightReg);
        emit(to, "    $P%d = $I%d \n", destReg, destReg);
        break;
    case NE_OP:
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        emit(to, "    $I%d = '__pjs_eq__'($P%d, $P%d)\n", destReg, leftReg, rightReg);
        emit(to, "    $I%d = ! $I%d\n", destReg, destReg);
        emit(to, "    $P%d = $I%d \n", destReg, destReg);
        break;
    case STRICT_EQ_OP:
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        emit(to, "    $I%d = iseq $P%d, $P%d \n", destReg, leftReg, rightReg);
        emit(to, "    $P%d = $I%d \n", destReg, destReg);
        break;
    case STRICT_NE_OP:
        emit(to, "    $P%d = new 'PjsBoolean'\n", destReg);
        emit(to, "    $I%d = iseq $P%d, $P%d \n", destReg, leftReg, rightReg);
        emit(to, "    $I%d = ! $I%d\n", destReg, destReg);
        emit(to, "    $P%d = $I%d \n", destReg, destReg);
        break;
        
    case '&':
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, leftReg);
        emit(to, "    $P%d &= $P%d \n", destReg, rightReg);
        break;
    case '|':
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, leftReg);
        emit(to, "    $P%d |= $P%d \n", destReg, rightReg);
        break;
    case '^':
        emit(to, "    $P%d = new 'PjsNumber'\n", destReg);
        emit(to, "    assign $P%d, $P%d\n", destReg, leftReg);
        emit(to, "    $P%d ~= $P%d\n", destReg, rightReg);
        break;
    default:
        emit(to, "    # UNKNOWN OPERATOR\n");
        fprintf(stderr, "UNKNOWN OPERATOR\n");
        break;
    }
}

void 
emit_binop( Node left, 
            Node right, 
            int op, 
            int destReg, 
            Naming naming, 
            parrot_data* to    ) {
    int leftReg = naming.regNo++;
    int rightReg = naming.regNo++;
    if (op == AND_OP) {
        int label = (*naming.labelNo)++;
        emit_expr(left, leftReg, naming, to);
        emit(to, "    $P%d = new 'PjsUndefined'\n", destReg);
        emit(to, "    $P%d = $P%d\n", destReg, leftReg);
        emit(to, "    unless $P%d goto and@%d\n", destReg, label);
        emit_expr(right, rightReg, naming, to);
        emit(to, "    $P%d = new 'PjsUndefined'\n", destReg);
        emit(to, "    $P%d = $P%d\n", destReg, rightReg);
        emit(to, "  and@%d:\n", label);
    } else if (op == OR_OP) {
        int label = (*naming.labelNo)++;
        emit_expr(left, leftReg, naming, to);
        emit(to, "    $P%d = new 'PjsUndefined'\n", destReg);
        emit(to, "    $P%d = $P%d\n", destReg, leftReg);
        emit(to, "    if $P%d goto or@%d\n", destReg, label);
        emit_expr(right, rightReg, naming, to);
        emit(to, "    $P%d = new 'PjsUndefined'\n", destReg);
        emit(to, "    $P%d = $P%d\n", destReg, rightReg);
        emit(to, "  or@%d:\n", label);
    } else {
        emit_expr(left, leftReg, naming, to);
        emit_expr(right, rightReg, naming, to);
        emit_strict_binop(leftReg, rightReg, op, destReg, naming, to);
    }
}

/**
 * Examples:
 *      x = 10
 *      y += 'abc'
 *      ...
 */
static void 
emit_assignment_change_env( Node expr, 
                            int destReg, 
                            Naming naming, 
                            parrot_data* to     ) {
    char* varName = expr->u.assign.left->u.strVal;
    if (expr->u.assign.type == '=') {
        emit_expr(expr->u.assign.right, destReg, naming, to);
        emit(to, "    pjs_store_lex $P%d, @env_%d, '%s'\n", destReg, naming.scopeNo, varName);
    } else {
        int leftReg = naming.regNo++;
        int rightReg = naming.regNo++;
        emit(to, "    $P%d = pjs_find_lex @env_%d, '%s'\n", leftReg, naming.scopeNo, varName);
        emit_expr(expr->u.assign.right, rightReg, naming, to);
        emit_strict_binop(leftReg, rightReg, expr->u.assign.type, destReg, naming, to);
        emit(to, "    pjs_store_lex $P%d, @env_%d, '%s'\n", destReg, naming.scopeNo, varName);
    }
}

/**
 * Examples:
 *      obj.x = 10
 *      obj.y += 'abc'
 *      ...
 */
static void 
emit_assignment_change_prop( Node expr, 
                             int destReg, 
                             Naming naming, 
                             parrot_data* to     ) {

    int contReg = naming.regNo++;
    char* propname = expr->u.assign.left->u.getprop.propName;
    emit_expr(expr->u.assign.left->u.getprop.base, contReg, naming, to);
    
    if (expr->u.assign.type == '=') {
        emit_expr(expr->u.assign.right, destReg, naming, to);
        emit(to, "    $P%d = '__toObject__'($P%d, @env_%d)\n", contReg, contReg, naming.scopeNo);
        emit(to, "    setattribute $P%d, '%s', $P%d\n", contReg, propname, destReg);
    } else {
        int leftReg = naming.regNo++;
        int rightReg = naming.regNo++;
        emit(to, "    $P%d = '__toObject__'($P%d, @env_%d)\n", contReg, contReg, naming.scopeNo);
        emit(to, "    $P%d = getattribute $P%d, '%s'\n", leftReg, contReg, propname);
        emit_expr(expr->u.assign.right, rightReg, naming, to);
        emit_strict_binop(leftReg, rightReg, expr->u.assign.type, destReg, naming, to);
        emit(to, "    setattribute $P%d, '%s', $P%d\n", contReg, propname, destReg);
    }
}

/**
 * Examples:
 *      obj[x] = 10
 *      obj[3] += 'abc'
 *      ...
 */
static void 
emit_assignment_change_indexed( Node expr, 
                                int destReg, 
                                Naming naming, 
                                parrot_data* to     ) {

    int indexReg = naming.regNo++;
    int contReg = naming.regNo++;
    emit_expr(expr->u.assign.left->u.getindexed.base, contReg, naming, to);
    emit_expr(expr->u.assign.left->u.getindexed.index, indexReg, naming, to);
    
    if (expr->u.assign.type == '=') {
        emit_expr(expr->u.assign.right, destReg, naming, to);
        emit(to, "    $P%d = '__toObject__'($P%d, @env_%d)\n", contReg, contReg, naming.scopeNo);
        emit(to, "    $P%d = pjs_to_primitive $P%d\n", indexReg, indexReg);
        emit(to, "    $P%d[$P%d] = $P%d\n", contReg, indexReg, destReg);
    } else {
        int leftReg = naming.regNo++;
        int rightReg = naming.regNo++;
        emit(to, "    $P%d = '__toObject__'($P%d, @env_%d)\n", contReg, contReg, naming.scopeNo);
        emit(to, "    $P%d = $P%d[$P%d]\n", leftReg, contReg, indexReg);
        emit_expr(expr->u.assign.right, rightReg, naming, to);
        emit_strict_binop(leftReg, rightReg, expr->u.assign.type, destReg, naming, to);
        emit(to, "    $P%d[$P%d] = $P%d\n", contReg, indexReg, destReg);
    }
}

void 
emit_assignment( Node expr, 
                 int destReg, 
                 Naming naming, 
                 parrot_data* to    ) {
    if (expr->u.assign.left->kind == EPropAccess) {
        emit_assignment_change_prop(expr, destReg, naming, to);
    } else if (expr->u.assign.left->kind == EIndexedAccess) {
        emit_assignment_change_indexed(expr, destReg, naming, to);
    } else if (expr->u.assign.left->kind == EIdentifier) {
        emit_assignment_change_env(expr, destReg, naming, to);
    } else {
        Parrot_ex_throw_from_c_args(to->interp, NULL, EXCEPTION_SYNTAX_ERROR, 
            "Unexpected lefthandside in assignment.");
    }
}
