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

    DBUG_PRINT("OP", ("Potential optimazation candidate from line %d", NODE_LINE(arg_node)));

    node *optimzedNode = NULL;
    switch (UNOP_OP(arg_node)) {
    case UO_neg :
        if (NODE_TYPE(UNOP_EXPR(arg_node)) == N_intconst) {
            DBUG_PRINT("OP", ("Optimizing int constant."));
            optimzedNode = TBmakeIntconst(TY_int, - INTCONST_VALUE(UNOP_EXPR(arg_node)));
        } else if (NODE_TYPE(UNOP_EXPR(arg_node)) == N_floatconst) {
            DBUG_PRINT("OP", ("Optimizing float constant."));
            optimzedNode = TBmakeIntconst(TY_float, - FLOATCONST_VALUE(UNOP_EXPR(arg_node)));
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

