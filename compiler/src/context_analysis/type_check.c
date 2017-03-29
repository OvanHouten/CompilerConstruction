/*
 * type_check.c
 *
 *  Created on: 15 Mar 2017
 *      Author: nico
 */


#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "traverse.h"
#include "memory.h"
#include "dbug.h"
#include "ctinfo.h"
#include "type_utils.h"

#include "type_check.h"
#include "type_utils.h"

/*
 * INFO structure
 */
struct INFO {
  node *funHeader;
  bool containsReturn;
};

/*
 * INFO macros
 */
#define INFO_FUNHEADER(n)  ((n)->funHeader)
#define INFO_CONTAINSRETURN(n)  ((n)->containsReturn)

/*
 * INFO functions
 */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));
  INFO_FUNHEADER(result) = NULL;
  INFO_CONTAINSRETURN(result) = FALSE;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  info = MEMfree( info);

  DBUG_RETURN( info);
}

node *TCassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCassign");

    DBUG_PRINT("TC", ("Assign %s >>", ID_NAME(ASSIGN_LET(arg_node))));

    TRAVdo(ASSIGN_EXPR(arg_node), arg_info);

    type rightResultType = determineType(ASSIGN_EXPR(arg_node));
    type leftResultType = determineType(ASSIGN_LET(arg_node));

    if (leftResultType != rightResultType) {
        CTIerror("The type at the left hand side [%d] and the right hand side [%d] don't match at line [%d] and column [%d].", leftResultType, rightResultType, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    DBUG_PRINT("TC", ("Assign <<"));
    DBUG_RETURN(arg_node);
}

node *TCvardef(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCvarded");

    DBUG_PRINT("TC", ("VarDef >>"));

    TRAVopt(VARDEF_EXPR(arg_node), arg_info);

    if (VARDEF_EXPR(arg_node)) {
        type rightResultType = determineType(VARDEF_EXPR(arg_node));
        type leftResultType = determineType(arg_node);

        if (leftResultType != rightResultType) {
            CTIerror("The type at the left hand side [%d] and the right hand side [%d] don't match at line [%d] and column [%d].", leftResultType, rightResultType, NODE_LINE(arg_node), NODE_COL(arg_node));
        }
    }

    DBUG_PRINT("TC", ("VarDef <<"));
    DBUG_RETURN(arg_node);
}

node *TCtypecast(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCtypecast");

    DBUG_PRINT("TC", ("Typecast >>"));

    TRAVdo(TYPECAST_EXPR(arg_node), arg_info);

    type exprType = determineType(TYPECAST_EXPR(arg_node));
    switch (exprType) {
        case TY_int:
        case TY_float:
        case TY_bool:
            break;
        default :
            CTIerror("The type of the expression [%d] can not be casted into [%d] at line [%d] and column [%d].", exprType, TYPECAST_TYPE(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    DBUG_PRINT("TC", ("Typecast <<"));
    DBUG_RETURN(arg_node);
}

node *TCunop(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCunop");

    DBUG_PRINT("TC", ("UnOp >>"));

    TRAVdo(UNOP_EXPR(arg_node), arg_info);

    type exprType = determineType(UNOP_EXPR(arg_node));
    switch (exprType) {
        case TY_int:
        case TY_float:
            switch (UNOP_OP(arg_node)) {
                case UO_neg :
                    break;
                default:
                    CTIerror("Invalid unary operator [%d] for expression type [%d] at line [%d] and column [%d].", UNOP_OP(arg_node), exprType, NODE_LINE(arg_node), NODE_COL(arg_node));
            }
            break;
        case TY_bool:
            switch (UNOP_OP(arg_node)) {
                case UO_not :
                    break;
                default:
                    CTIerror("Invalid unary operator [%d] for expression type [%d] at line [%d] and column [%d].", UNOP_OP(arg_node), exprType, NODE_LINE(arg_node), NODE_COL(arg_node));
            }
            break;
            break;
        case TY_void:
            break;
        default :
            CTIerror("Untyped expression for binary operator [%d] at line [%d] and column [%d].", BINOP_OP(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    }
    UNOP_TYPE(arg_node) = exprType;

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
        switch (leftResultType) {
            case TY_int :
            case TY_float :
                switch (BINOP_OP(arg_node)) {
                    case BO_add :
                    case BO_sub :
                    case BO_mul :
                    case BO_div :
                        BINOP_TYPE(arg_node) = leftResultType;
                        break;
                    case BO_lt :
                    case BO_le :
                    case BO_eq :
                    case BO_ne :
                    case BO_ge :
                    case BO_gt :
                        BINOP_TYPE(arg_node) = TY_bool;
                        break;
                    case BO_mod :
                        if (leftResultType == TY_int) {
                            BINOP_TYPE(arg_node) = leftResultType;
                        } else {
                            CTIerror("Invalid binary operator [%d] at line [%d] and column [%d].", BINOP_OP(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
                        }
                        break;
                    default:
                        CTIerror("Invalid binary operator [%d] at line [%d] and column [%d].", BINOP_OP(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
                }
                break;
            case TY_bool :
                switch (BINOP_OP(arg_node)) {
                    case BO_and :
                    case BO_or :
                    case BO_add :
                    case BO_mul :
                    case BO_eq :
                    case BO_ne :
                        BINOP_TYPE(arg_node) = TY_bool;
                        break;
                    default:
                        CTIerror("Invalid binary operator [%d] at line [%d] and column [%d].", BINOP_OP(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
                }
                break;
            case TY_void :
                CTIerror("A binary operator [%d] at line [%d] and column [%d] returns a void!.", BINOP_OP(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
                break;
            default :
                CTIerror("Untyped expression for binary operator [%d] at line [%d] and column [%d].", BINOP_OP(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
        }
    }

    DBUG_PRINT("TC", ("BinOp <<"));
    DBUG_RETURN(arg_node);
}

node *TCfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCfundef");

    DBUG_PRINT("TC", ("Fundef >>"));
    node *outerFunHeader = INFO_FUNHEADER(arg_info);
    INFO_FUNHEADER(arg_info) = FUNDEF_FUNHEADER(arg_node);

    TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

    INFO_FUNHEADER(arg_info) = outerFunHeader;
    DBUG_PRINT("TC", ("Fundef <<"));

    DBUG_RETURN(arg_node);
}

node *TCfunbody(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCfunbody");

    TRAVopt(FUNBODY_VARDECS(arg_node), arg_info);
    TRAVopt(FUNBODY_LOCALFUNDEFS(arg_node), arg_info);

    bool outerContainsReturn = INFO_CONTAINSRETURN(arg_info);
    INFO_CONTAINSRETURN(arg_info) = FALSE;

    TRAVopt(FUNBODY_STATEMENTS(arg_node), arg_info);
    DBUG_PRINT("TC", ("..."));
    if (FUNHEADER_RETURNTYPE(INFO_FUNHEADER(arg_info)) != TY_void && !INFO_CONTAINSRETURN(arg_info)) {
        CTIerror("The function '%s' at line [%d] must have a return statements that returns [%d].", FUNHEADER_NAME(INFO_FUNHEADER(arg_info)), NODE_LINE(arg_node), FUNHEADER_RETURNTYPE(INFO_FUNHEADER(arg_info)));
    }

    INFO_CONTAINSRETURN(arg_info) = outerContainsReturn;

    DBUG_RETURN(arg_node);
}

node *TCfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCfuncall");

    DBUG_PRINT("TC", ("Funcall >> [%d]", NODE_LINE(arg_node)));
    TRAVopt(FUNCALL_EXPRS(arg_node), arg_info);

    if (FUNCALL_EXPRS(arg_node)) {
        node *params = FUNHEADER_PARAMS(FUNDEF_FUNHEADER(SYMBOLTABLEENTRY_DECL(FUNCALL_DECL(arg_node))));
        node *exprs = FUNCALL_EXPRS(arg_node);
        while (exprs) {
            type expectedType = determineType(PARAMS_PARAM(params));
            type actualType = determineType(EXPRS_EXPR(exprs));
            if (actualType != expectedType) {
                CTIerror("The actual type [%d] and the expected parameter type [%d] don't match at line [%d] and column [%d].", actualType, expectedType, NODE_LINE(arg_node), NODE_COL(arg_node));
            }
            params = PARAMS_NEXT(params);
            exprs = EXPRS_NEXT(exprs);
        }
    }

    DBUG_PRINT("TC", ("Funcall << [%d]", NODE_LINE(arg_node)));
    DBUG_RETURN(arg_node);
}

node *TCreturn(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCreturn");

    DBUG_PRINT("TC", ("Return >>"));
    if (RETURN_EXPR(arg_node)) {
        TRAVopt(RETURN_EXPR(arg_node), arg_info);

        type returnType = determineType(RETURN_EXPR(arg_node));
        if (returnType != FUNHEADER_RETURNTYPE(INFO_FUNHEADER(arg_info))) {
            CTIerror("The type of the expression [%d] and the return type [%d] for the function don't match at line [%d] and column [%d].", returnType, FUNHEADER_RETURNTYPE(INFO_FUNHEADER(arg_info)), NODE_LINE(arg_node), NODE_COL(arg_node));
        }
        INFO_CONTAINSRETURN(arg_info) = TRUE;
    } else {
        if (FUNHEADER_RETURNTYPE(INFO_FUNHEADER(arg_info)) != TY_void) {
            CTIerror("A void returning function can not return anything, at line [%d] and column [%d].", NODE_LINE(arg_node), NODE_COL(arg_node));
        }
    }

    DBUG_PRINT("TC", ("Return <<"));
    DBUG_RETURN(arg_node);
}

node *TCdo(node *arg_node, info *arg_info) {
    DBUG_ENTER("");

    TRAVdo(DO_CONDITION(arg_node), arg_info);
    if (determineType(DO_CONDITION(arg_node)) != TY_bool) {
        CTIerror("The expression for a do-loop must be evaluate to boolean and not to [%d] at line [%d] and column [%d].", determineType(DO_CONDITION(arg_node)), NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    TRAVopt(DO_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCwhile(node *arg_node, info *arg_info) {
    DBUG_ENTER("");

    TRAVopt(WHILE_BLOCK(arg_node), arg_info);

    TRAVdo(WHILE_CONDITION(arg_node), arg_info);
    if (determineType(WHILE_CONDITION(arg_node)) != TY_bool) {
        CTIerror("The expression for a while-loop must be evaluate to boolean and not to [%d] at line [%d] and column [%d].", determineType(WHILE_CONDITION(arg_node)), NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    DBUG_RETURN(arg_node);
}

node *TCfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("");

    TRAVdo(FOR_VARDEF(arg_node), arg_info);
    TRAVdo(FOR_FINISH(arg_node), arg_info);
    if (determineType(FOR_FINISH(arg_node)) != TY_int) {
        CTIerror("The finish expression for a for-loop must be evaluate to 'int' and not to [%d] at line [%d] and column [%d].", determineType(FOR_FINISH(arg_node)), NODE_LINE(arg_node), NODE_COL(arg_node));
    }
    TRAVopt(FOR_STEP(arg_node), arg_info);
    if (determineType(FOR_STEP(arg_node)) != TY_int) {
        CTIerror("The step expression for a for-loop must be evaluate to 'int' and not to [%d] at line [%d] and column [%d].", determineType(FOR_STEP(arg_node)), NODE_LINE(arg_node), NODE_COL(arg_node));
    }
    TRAVopt(FOR_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCdoTypeCheck(node *syntaxtree) {
    DBUG_ENTER("RCdoReturnCheck");

    DBUG_PRINT("TC", ("Starting the type check."));

    info *arg_info = MakeInfo();

    TRAVpush(TR_tc);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_PRINT("TC", ("Finished the type check."));

    DBUG_RETURN(syntaxtree);
}
