/*
 * type_check.c
 *
 *  Created on: 15 Mar 2017
 *      Author: nico
 */

#include "type_check.h"

#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "traverse.h"
#include "memory.h"
#include "dbug.h"
#include "ctinfo.h"
#include "type_utils.h"


/*
 * INFO structure
 */
struct INFO {
  int dummy;
};

/*
 * INFO macros
 */
#define INFO_DUMMY(n)  ((n)->dummy)

/*
 * INFO functions
 */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));
  INFO_DUMMY(result) = 0;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  info = MEMfree( info);

  DBUG_RETURN( info);
}

type determineType(node *expr) {
    DBUG_ENTER("determineType");

    type exprType = TY_unknown;
    DBUG_PRINT("TC", ("Determining type for [%d]", NODE_TYPE(expr)));
    switch (NODE_TYPE(expr)) {
        case N_funcall :
            exprType = SYMBOLTABLEENTRY_TYPE(FUNCALL_DECL(expr));
            break;
        case N_id :
            exprType = SYMBOLTABLEENTRY_TYPE(ID_DECL(expr));
            break;
        case N_unop:
            exprType = UNOP_TYPE(expr);
            break;
        case N_binop:
            exprType = BINOP_TYPE(expr);
            break;
        case N_intconst :
            exprType = TY_int;
            break;
        case N_floatconst :
            exprType = TY_float;
            break;
        case N_boolconst :
            exprType = TY_bool;
            break;
        default :
            DBUG_PRINT("TC", ("Unhandled epxression with type [%d]", NODE_TYPE(expr)));
            break;
    }
    DBUG_PRINT("TC", ("Type [%d]", exprType));

    DBUG_RETURN(exprType);
}

node *TCassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCassign");

    DBUG_PRINT("TC", ("Assign >>"));

    TRAVdo(ASSIGN_EXPR(arg_node), arg_info);

    type rightResultType = determineType(ASSIGN_EXPR(arg_node));
    type leftResultType = determineType(ASSIGN_LET(arg_node));

    if (leftResultType != rightResultType) {
        CTIerror("The type at the left hand side [%d] and the right hand side [%d] don't match at line [%d] and column [%d].", leftResultType, rightResultType, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    DBUG_PRINT("TC", ("Assign <<"));
    DBUG_RETURN(arg_node);
}

node *TCunop(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCunop");

    DBUG_PRINT("TC", ("UnOp >>"));

    TRAVdo(UNOP_RIGHT(arg_node), arg_info);

    DBUG_PRINT("TC", ("UnOp <<"));
    DBUG_RETURN(arg_node);
}

node *TCbinop(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCbinop");

    DBUG_PRINT("TC", ("BinOp >>"));

    TRAVdo(BINOP_LEFT(arg_node), arg_info);
    TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    type leftResultType = determineType(BINOP_LEFT(arg_node));
    type rightResultType = determineType(BINOP_RIGHT(arg_node));

    if (leftResultType != rightResultType) {
        CTIerror("The type at the left hand side [%d] and the right hand side [%d] don't match at line [%d] and column [%d].", leftResultType, rightResultType, NODE_LINE(arg_node), NODE_COL(arg_node));
    } else {
        switch (BINOP_OP(arg_node)) {
            case BO_add :
            case BO_sub :
            case BO_mul :
            case BO_div :
                BINOP_TYPE(arg_node) = leftResultType;
                break;
            default:
                CTIerror("Unhandled binary operator [%d] at line [%d] and column [%d].", BINOP_OP(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
                break;
        }
    }

    DBUG_PRINT("TC", ("BinOp <<"));
    DBUG_RETURN(arg_node);
}

node *TCfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCfuncall");

    DBUG_PRINT("TC", ("Funcall >>"));
    TRAVopt(FUNCALL_PARAMS(arg_node), arg_info);

    FUNCALL_TYPE(arg_node) = FUNHEADER_RETURNTYPE(SYMBOLTABLEENTRY_DECL(FUNCALL_DECL(arg_node)));

    DBUG_PRINT("TC", ("Funcall <<"));
    DBUG_RETURN(arg_node);
}

node *TCdoTypeCheck(node *syntaxtree) {
    DBUG_ENTER("RCdoReturnCheck");

    info *arg_info = MakeInfo();

    TRAVpush(TR_tc);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
