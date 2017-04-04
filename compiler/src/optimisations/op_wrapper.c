/*
 * op_wrapper.c
 *
 *  Created on: 22 Mar 2017
 *      Author: nico
 */

#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"
#include "ctinfo.h"
#include "myglobals.h"
#include "copy.h"
#include "free_node.h"

#include "op_wrapper.h"

struct INFO {
  bool keepOptimizing;
};

#define INFO_KEEPOPTIMIZING(n) ((n)->keepOptimizing)

/*
 * INFO functions
 */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));
  INFO_KEEPOPTIMIZING(result) = FALSE;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  info = MEMfree( info);

  DBUG_RETURN( info);
}

node *OPunop(node *arg_node, info *arg_info) {
    DBUG_ENTER("OPunop");

    // First try the expression, you never know...
    UNOP_EXPR(arg_node) = TRAVdo(UNOP_EXPR(arg_node), arg_info);

    DBUG_PRINT("OP", ("Potential constant reduction optimazation candidate from line %d", NODE_LINE(arg_node)));
    node *optimzedNode = NULL;
    switch (UNOP_OP(arg_node)) {
    case UO_neg :
        if (NODE_TYPE(UNOP_EXPR(arg_node)) == N_intconst) {
            DBUG_PRINT("OP", ("Optimizing int constant."));
            optimzedNode = TBmakeIntconst(TY_int, - INTCONST_VALUE(UNOP_EXPR(arg_node)));
        } else if (NODE_TYPE(UNOP_EXPR(arg_node)) == N_floatconst) {
            DBUG_PRINT("OP", ("Optimizing float constant."));
            optimzedNode = TBmakeFloatconst(TY_float, - FLOATCONST_VALUE(UNOP_EXPR(arg_node)));
        }
        break;
    case UO_not:
        if (NODE_TYPE(UNOP_EXPR(arg_node)) == N_boolconst) {
            DBUG_PRINT("OP", ("Optimizing bool constant."));
            optimzedNode = TBmakeBoolconst(TY_bool, !BOOLCONST_VALUE(UNOP_EXPR(arg_node)));
        }
        break;
    default:
        // We should never ever make it to here...
        DBUG_PRINT("OP", ("Can't optimize."));
        break;
    }

    if (optimzedNode){
        NODE_LINE(optimzedNode) = NODE_LINE(arg_node);
        NODE_COL(optimzedNode) = NODE_COL(arg_node);

        arg_node = MEMfree(arg_node);
        arg_node = optimzedNode;
        DBUG_PRINT("OP", ("Optimization activated."));

        INFO_KEEPOPTIMIZING(arg_info) = TRUE;
    }

    DBUG_RETURN(arg_node);
}

node *OPbinop(node *arg_node, info *arg_info) {
    DBUG_ENTER("OPbinop");

    // First try left and right, who knows...
    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    node *folded = NULL;
    if (NODE_TYPE(BINOP_LEFT(arg_node)) == N_intconst && NODE_TYPE(BINOP_RIGHT(arg_node)) == N_intconst) {
        DBUG_PRINT("OP", ("Potential int binop for folding."));
        switch (BINOP_OP(arg_node)) {
            case BO_add:
                folded = TBmakeIntconst(TY_int, INTCONST_VALUE(BINOP_LEFT(arg_node)) + INTCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_sub:
                folded = TBmakeIntconst(TY_int, INTCONST_VALUE(BINOP_LEFT(arg_node)) - INTCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_mul:
                folded = TBmakeIntconst(TY_int, INTCONST_VALUE(BINOP_LEFT(arg_node)) * INTCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_div:
                if (INTCONST_VALUE(BINOP_RIGHT(arg_node)) != 0) {
                    folded = TBmakeIntconst(TY_int, INTCONST_VALUE(BINOP_LEFT(arg_node)) / INTCONST_VALUE(BINOP_RIGHT(arg_node)));
                }
                break;
            case BO_mod:
                folded = TBmakeIntconst(TY_int, INTCONST_VALUE(BINOP_LEFT(arg_node)) % INTCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_lt:
                folded = TBmakeBoolconst(TY_bool, INTCONST_VALUE(BINOP_LEFT(arg_node)) < INTCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_le:
                folded = TBmakeBoolconst(TY_bool, INTCONST_VALUE(BINOP_LEFT(arg_node)) <= INTCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_eq:
                folded = TBmakeBoolconst(TY_bool, INTCONST_VALUE(BINOP_LEFT(arg_node)) == INTCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_ne:
                folded = TBmakeBoolconst(TY_bool, INTCONST_VALUE(BINOP_LEFT(arg_node)) != INTCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_ge:
                folded = TBmakeBoolconst(TY_bool, INTCONST_VALUE(BINOP_LEFT(arg_node)) >= INTCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_gt:
                folded = TBmakeBoolconst(TY_bool, INTCONST_VALUE(BINOP_LEFT(arg_node)) > INTCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            default:
                DBUG_PRINT("OP", ("Unable to fold int binop %d.", BINOP_OP(arg_node)));
        }
    } else if (NODE_TYPE(BINOP_LEFT(arg_node)) == N_floatconst && NODE_TYPE(BINOP_RIGHT(arg_node)) == N_floatconst) {
        DBUG_PRINT("OP", ("Potential float binop for folding."));
        switch (BINOP_OP(arg_node)) {
            case BO_add:
                folded = TBmakeFloatconst(TY_float, FLOATCONST_VALUE(BINOP_LEFT(arg_node)) + FLOATCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_sub:
                folded = TBmakeFloatconst(TY_float, FLOATCONST_VALUE(BINOP_LEFT(arg_node)) - FLOATCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_mul:
                folded = TBmakeFloatconst(TY_float, FLOATCONST_VALUE(BINOP_LEFT(arg_node)) * FLOATCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_div:
                if (FLOATCONST_VALUE(BINOP_RIGHT(arg_node)) != 0.0) {
                    folded = TBmakeFloatconst(TY_float, FLOATCONST_VALUE(BINOP_LEFT(arg_node)) / FLOATCONST_VALUE(BINOP_RIGHT(arg_node)));
                }
                break;
            case BO_lt:
                folded = TBmakeBoolconst(TY_bool, FLOATCONST_VALUE(BINOP_LEFT(arg_node)) < FLOATCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_le:
                folded = TBmakeBoolconst(TY_bool, FLOATCONST_VALUE(BINOP_LEFT(arg_node)) <= FLOATCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_eq:
                folded = TBmakeBoolconst(TY_bool, FLOATCONST_VALUE(BINOP_LEFT(arg_node)) == FLOATCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_ne:
                folded = TBmakeBoolconst(TY_bool, FLOATCONST_VALUE(BINOP_LEFT(arg_node)) != FLOATCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_ge:
                folded = TBmakeBoolconst(TY_bool, FLOATCONST_VALUE(BINOP_LEFT(arg_node)) >= FLOATCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_gt:
                folded = TBmakeBoolconst(TY_bool, FLOATCONST_VALUE(BINOP_LEFT(arg_node)) > FLOATCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            default:
                DBUG_PRINT("OP", ("Unable to fold float binop %d.", BINOP_OP(arg_node)));
        }
    } else if (NODE_TYPE(BINOP_LEFT(arg_node)) == N_boolconst && NODE_TYPE(BINOP_RIGHT(arg_node)) == N_boolconst) {
        DBUG_PRINT("OP", ("Potential bool binop for folding."));
        switch (BINOP_OP(arg_node)) {
            case BO_mul:
            case BO_and:
                folded = TBmakeBoolconst(TY_bool, BOOLCONST_VALUE(BINOP_LEFT(arg_node)) & BOOLCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_add:
            case BO_or:
                folded = TBmakeBoolconst(TY_bool, BOOLCONST_VALUE(BINOP_LEFT(arg_node)) | BOOLCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_eq:
                folded = TBmakeBoolconst(TY_bool, BOOLCONST_VALUE(BINOP_LEFT(arg_node)) == BOOLCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            case BO_ne:
                folded = TBmakeBoolconst(TY_bool, BOOLCONST_VALUE(BINOP_LEFT(arg_node)) != BOOLCONST_VALUE(BINOP_RIGHT(arg_node)));
                break;
            default:
                DBUG_PRINT("OP", ("Unable to fold bool binop %d.", BINOP_OP(arg_node)));
        }
    }
    if (folded) {
        NODE_LINE(folded) = NODE_LINE(arg_node);
        NODE_COL(folded) = NODE_COL(arg_node);
        arg_node = FREEbinop(arg_node, arg_info);
        arg_node = folded;
        INFO_KEEPOPTIMIZING(arg_info) = TRUE;
        DBUG_PRINT("OP", ("Folded :-)"));
    }

    DBUG_RETURN(arg_node);
}

bool areSameVars(node *left, node *right) {
    return ID_STE(left) == ID_STE(right);
}
node *OPassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("OPassign");

    // Lets try to optimize the expression first
    ASSIGN_EXPR(arg_node) = TRAVdo(ASSIGN_EXPR(arg_node), arg_info);

    node *compoundOp = NULL;
    // Detecting 'var = var + const' and 'var = var - const' kind of operations
    if (NODE_TYPE(ASSIGN_EXPR(arg_node)) == N_binop && NODE_TYPE(BINOP_LEFT(ASSIGN_EXPR(arg_node))) == N_id && NODE_TYPE(BINOP_RIGHT(ASSIGN_EXPR(arg_node))) == N_intconst) {
        if (areSameVars(ASSIGN_LET(arg_node), BINOP_LEFT(ASSIGN_EXPR(arg_node)))) {
            DBUG_PRINT("OP", ("Found a potential INC/DEC optimization candidate."));
            if (BINOP_OP(ASSIGN_EXPR(arg_node)) == BO_add) {
                compoundOp = TBmakeCompop(CO_inc, COPYdoCopy(BINOP_LEFT(ASSIGN_EXPR(arg_node))), COPYdoCopy(BINOP_RIGHT(ASSIGN_EXPR(arg_node))));
            } else if (BINOP_OP(ASSIGN_EXPR(arg_node)) == BO_sub) {
                compoundOp = TBmakeCompop(CO_dec, COPYdoCopy(BINOP_LEFT(ASSIGN_EXPR(arg_node))), COPYdoCopy(BINOP_RIGHT(ASSIGN_EXPR(arg_node))));
            }
        }
    }
    if (compoundOp) {
        DBUG_PRINT("OP", ("Optimized a 'var = var +/- const' statement."));
        NODE_LINE(compoundOp) = NODE_LINE(arg_node);
        NODE_COL(compoundOp) = NODE_COL(arg_node);
        arg_node = FREEassign(arg_node, arg_info);
        arg_node = compoundOp;
        INFO_KEEPOPTIMIZING(arg_info) = TRUE;
    }

    DBUG_RETURN(arg_node);
}

node *OPif(node *arg_node, info *arg_info) {
    DBUG_ENTER("OPif");

    IF_CONDITION(arg_node) = TRAVdo(IF_CONDITION(arg_node), arg_info);
    IF_IFBLOCK(arg_node) = TRAVopt(IF_IFBLOCK(arg_node), arg_info);
    IF_ELSEBLOCK(arg_node) = TRAVopt(IF_ELSEBLOCK(arg_node), arg_info);

    if (NODE_TYPE(IF_CONDITION(arg_node)) == N_boolconst) {
        DBUG_PRINT("OP", ("Reducing a if-else statement."));

        node *remainingBlock = NULL;
        if (BOOLCONST_VALUE(IF_CONDITION(arg_node))) {
            remainingBlock = IF_IFBLOCK(arg_node);
            IF_IFBLOCK(arg_node) = NULL;
        } else {
            remainingBlock = IF_ELSEBLOCK(arg_node);
            IF_ELSEBLOCK(arg_node) = NULL;
        }

        if (remainingBlock) {
            DBUG_PRINT("OP", ("Leaving the active block in place."));
            remainingBlock = TRAVopt(remainingBlock, arg_info);
        } else {
            DBUG_PRINT("OP", ("Nothing left :-)"));
            // This NOP statement will replace the removed if-else statement
            remainingBlock = TBmakeNop();
        }

        arg_node = FREEif(arg_node, arg_info);
        // Just return the code block, the OPstatements function will insert
        // the statements into the surrounding code
        arg_node = remainingBlock;

        INFO_KEEPOPTIMIZING(arg_info) = TRUE;
    }

    DBUG_RETURN(arg_node);
}

node *OPternop(node *arg_node, info *arg_info) {
    DBUG_ENTER("OPternop");

    TERNOP_CONDITION(arg_node) = TRAVdo(TERNOP_CONDITION(arg_node), arg_info);
    TERNOP_THEN(arg_node) = TRAVdo(TERNOP_THEN(arg_node), arg_info);
    TERNOP_ELSE(arg_node) = TRAVdo(TERNOP_ELSE(arg_node), arg_info);

    if (NODE_TYPE(TERNOP_CONDITION(arg_node)) == N_boolconst) {
        DBUG_PRINT("OP", ("Optimizing a ternary operation."));
        node *remaining = COPYdoCopy(BOOLCONST_VALUE(TERNOP_CONDITION(arg_node)) ? TERNOP_THEN(arg_node) : TERNOP_ELSE(arg_node));
        arg_node = FREEternop(arg_node, arg_info);
        arg_node = remaining;

        INFO_KEEPOPTIMIZING(arg_info) = TRUE;
    }

    DBUG_RETURN(arg_node);
}

node *OPdoOptimisations(node *syntaxtree) {
    DBUG_ENTER("OPdoOptimisations");

    info *arg_info = MakeInfo();

    int maxOptimizationLoops = 10;
    do {
        INFO_KEEPOPTIMIZING(arg_info) = FALSE;
        TRAVpush(TR_op);

        syntaxtree = TRAVdo(syntaxtree, arg_info);

        TRAVpop();
        DBUG_PRINT("OP", ("Optimized code: %s", INFO_KEEPOPTIMIZING(arg_info) ? "TRUE" : "FALSE"));
    } while (INFO_KEEPOPTIMIZING(arg_info) && maxOptimizationLoops-- > 0);

    arg_info = FreeInfo(arg_info);
    DBUG_RETURN(syntaxtree);
}

