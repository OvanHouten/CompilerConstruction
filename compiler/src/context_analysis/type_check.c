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
#include "str.h"
#include "lookup_table.h"

#include "type_check.h"
#include "type_utils.h"

/*
 * INFO structure
 */
struct INFO {
  node *funHeader;
  lut_t *loopVars;
};

/*
 * INFO macros
 */
#define INFO_FUNHEADER(n)     ((n)->funHeader)
#define INFO_LOOPVARS(n)      ((n)->loopVars)

/*
 * INFO functions
 */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));
  INFO_FUNHEADER(result) = NULL;
  INFO_LOOPVARS(result) = LUTgenerateLut();

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  INFO_LOOPVARS(info) = LUTremoveLut(INFO_LOOPVARS(info));
  info = MEMfree( info);

  DBUG_RETURN( info);
}

node *TCassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCassign");

    DBUG_PRINT("TC", ("Assign '%s'", ID_NAME(ASSIGN_LET(arg_node))));

    TRAVdo(ASSIGN_EXPR(arg_node), arg_info);

    type rightResultType = determineType(ASSIGN_EXPR(arg_node));
    type leftResultType = determineType(ASSIGN_LET(arg_node));

    if (leftResultType != rightResultType) {
        CTIerror("The type at the left hand side '%s' and the right hand side '%s' don't match at line %d and column %d.", typeToString(leftResultType), typeToString(rightResultType), NODE_LINE(arg_node), NODE_COL(arg_node));
    }
    // Make sure that the for loop variable is read-only
    void **varName = LUTsearchInLutS(INFO_LOOPVARS(arg_info), ID_NAME(ASSIGN_LET(arg_node)));
    if (varName != NULL && (*varName) != NULL) {
        CTIerror("The loop variable '%s' can't be changed at line %d.", ID_NAME(ASSIGN_LET(arg_node)), NODE_LINE(arg_node));
    }

    DBUG_RETURN(arg_node);
}

node *TCvardef(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCvarded");

    DBUG_PRINT("TC", ("VarDef"));

    TRAVopt(VARDEF_EXPR(arg_node), arg_info);

    if (VARDEF_EXPR(arg_node)) {
        type rightResultType = determineType(VARDEF_EXPR(arg_node));
        type leftResultType = determineType(arg_node);

        if (leftResultType != rightResultType) {
            CTIerror("The type at the left hand side '%s' and the right hand side '%s' don't match at line %d and column %d.", typeToString(leftResultType), typeToString(rightResultType), NODE_LINE(arg_node), NODE_COL(arg_node));
        }
    }

    DBUG_RETURN(arg_node);
}

node *TCtypecast(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCtypecast");

    DBUG_PRINT("TC", ("Typecast"));

    TRAVdo(TYPECAST_EXPR(arg_node), arg_info);

    type exprType = determineType(TYPECAST_EXPR(arg_node));
    switch (exprType) {
        case TY_int:
        case TY_float:
        case TY_bool:
            break;
        default :
            CTIerror("The type of the expression '%s' can not be casted into '%s' at line %d and column %d.", typeToString(exprType), typeToString(TYPECAST_TYPE(arg_node)), NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    DBUG_RETURN(arg_node);
}

node *TCunop(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCunop");

    DBUG_PRINT("TC", ("UnOp"));

    TRAVdo(UNOP_EXPR(arg_node), arg_info);

    type exprType = determineType(UNOP_EXPR(arg_node));
    switch (exprType) {
        case TY_int:
        case TY_float:
            switch (UNOP_OP(arg_node)) {
                case UO_neg :
                    break;
                default:
                    CTIerror("Invalid unary operator %d for expression type %d at line %d and column %d.", UNOP_OP(arg_node), exprType, NODE_LINE(arg_node), NODE_COL(arg_node));
            }
            break;
        case TY_bool:
            switch (UNOP_OP(arg_node)) {
                case UO_not :
                    break;
                default:
                    CTIerror("Invalid unary operator %d for expression type %d at line %d and column %d.", UNOP_OP(arg_node), exprType, NODE_LINE(arg_node), NODE_COL(arg_node));
            }
            break;
            break;
        case TY_void:
            break;
        default :
            CTIerror("Untyped expression for binary operator %d at line %d and column %d.", BINOP_OP(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    }
    UNOP_TYPE(arg_node) = exprType;

    DBUG_RETURN(arg_node);
}

node *TCbinop(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCbinop");

    DBUG_PRINT("TC", ("BinOp"));

    TRAVdo(BINOP_LEFT(arg_node), arg_info);
    TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    type leftResultType = determineType(BINOP_LEFT(arg_node));
    type rightResultType = determineType(BINOP_RIGHT(arg_node));

    if (leftResultType != rightResultType) {
        CTIerror("The type at the left hand side '%s' and the right hand side '%s' don't match at line %d and column %d.", typeToString(leftResultType), typeToString(rightResultType), NODE_LINE(arg_node), NODE_COL(arg_node));
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
                            CTIerror("Invalid binary operator %d at line %d and column %d.", BINOP_OP(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
                        }
                        break;
                    default:
                        CTIerror("Invalid binary operator %d at line %d and column %d.", BINOP_OP(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
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
                        CTIerror("Invalid binary operator %d at line %d and column %d.", BINOP_OP(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
                }
                break;
            case TY_void :
                CTIerror("A binary operator %d at line %d and column %d returns a void!.", BINOP_OP(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
                break;
            default :
                CTIerror("Untyped expression for binary operator %d at line %d and column %d.", BINOP_OP(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
        }
    }

    DBUG_RETURN(arg_node);
}

node *TCfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCfundef");

    DBUG_PRINT("TC", ("Fundef"));
    node *outerFunHeader = INFO_FUNHEADER(arg_info);
    INFO_FUNHEADER(arg_info) = FUNDEF_FUNHEADER(arg_node);

    node *param = FUNHEADER_PARAMS(FUNDEF_FUNHEADER(arg_node));
    while (param) {
        if (VARDEF_TYPE(PARAMS_PARAM(param)) == TY_void) {
            CTIerror("Parameter can't be of type 'void' at line %d and column %d.", NODE_LINE(param), NODE_COL(param));
        }
        param = PARAMS_NEXT(param);
    }

    TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

    INFO_FUNHEADER(arg_info) = outerFunHeader;

    DBUG_RETURN(arg_node);
}

node *TCfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCfuncall");

    DBUG_PRINT("TC", ("Funcall"));
    TRAVopt(FUNCALL_EXPRS(arg_node), arg_info);

    if (FUNCALL_EXPRS(arg_node)) {
        node *params = FUNHEADER_PARAMS(FUNDEF_FUNHEADER(SYMBOLTABLEENTRY_DEFNODE(FUNCALL_STE(arg_node))));
        node *exprs = FUNCALL_EXPRS(arg_node);
        while (exprs) {
            type expectedType = determineType(PARAMS_PARAM(params));
            type actualType = determineType(EXPRS_EXPR(exprs));
            if (actualType != expectedType) {
                CTIerror("The actual type '%s' and the expected parameter type '%s don't match at line %d and column %d.", typeToString(actualType), typeToString(expectedType), NODE_LINE(arg_node), NODE_COL(arg_node));
            }
            params = PARAMS_NEXT(params);
            exprs = EXPRS_NEXT(exprs);
        }
    }

    DBUG_RETURN(arg_node);
}

node *TCreturn(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCreturn");

    DBUG_PRINT("TC", ("Return"));
    if (RETURN_EXPR(arg_node)) {
        TRAVopt(RETURN_EXPR(arg_node), arg_info);

        type returnType = determineType(RETURN_EXPR(arg_node));
        if (returnType != FUNHEADER_RETURNTYPE(INFO_FUNHEADER(arg_info))) {
            CTIerror("The type of the expression '%s' and the return type '%s' for the function don't match at line %d and column %d.", typeToString(returnType), typeToString(FUNHEADER_RETURNTYPE(INFO_FUNHEADER(arg_info))), NODE_LINE(arg_node), NODE_COL(arg_node));
        }
    } else {
        if (FUNHEADER_RETURNTYPE(INFO_FUNHEADER(arg_info)) != TY_void) {
            CTIerror("A void returning function can not return anything, at line %d and column %d.", NODE_LINE(arg_node), NODE_COL(arg_node));
        }
    }

    DBUG_RETURN(arg_node);
}

node *TCdo(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCdo");

    TRAVdo(DO_CONDITION(arg_node), arg_info);
    if (determineType(DO_CONDITION(arg_node)) != TY_bool) {
        CTIerror("The expression for a do-loop must be evaluate to boolean and not to '%s' at line %d and column %d.", typeToString(determineType(DO_CONDITION(arg_node))), NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    TRAVopt(DO_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCwhile(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCwhile");

    TRAVopt(WHILE_BLOCK(arg_node), arg_info);

    TRAVdo(WHILE_CONDITION(arg_node), arg_info);
    if (determineType(WHILE_CONDITION(arg_node)) != TY_bool) {
        CTIerror("The expression for a while-loop must be evaluate to boolean and not to '%s' at line %d and column %d.", typeToString(determineType(WHILE_CONDITION(arg_node))), NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    DBUG_RETURN(arg_node);
}

node *TCfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCfor");

    TRAVdo(FOR_VARDEF(arg_node), arg_info);
    TRAVdo(FOR_FINISH(arg_node), arg_info);
    if (determineType(FOR_FINISH(arg_node)) != TY_int) {
        CTIerror("The finish expression for a for-loop must be evaluate to 'int' and not to '%s' at line %d and column %d.", typeToString(determineType(FOR_FINISH(arg_node))), NODE_LINE(arg_node), NODE_COL(arg_node));
    }
    TRAVopt(FOR_STEP(arg_node), arg_info);
    if (determineType(FOR_STEP(arg_node)) != TY_int) {
        CTIerror("The step expression for a for-loop must be evaluate to 'int' and not to '%s' at line %d and column %d.", typeToString(determineType(FOR_STEP(arg_node))), NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    LUTinsertIntoLutS(INFO_LOOPVARS(arg_info), VARDEF_NAME(FOR_VARDEF(arg_node)), VARDEF_NAME(FOR_VARDEF(arg_node)));

    TRAVopt(FOR_BLOCK(arg_node), arg_info);

    void *dummy = NULL;
    LUTupdateLutS(INFO_LOOPVARS(arg_info), VARDEF_NAME(FOR_VARDEF(arg_node)), NULL, &dummy);

    DBUG_RETURN(arg_node);
}

node *TCif(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCif");

    IF_CONDITION(arg_node) = TRAVdo(IF_CONDITION(arg_node), arg_info);
    if (determineType(IF_CONDITION(arg_node)) != TY_bool) {
        CTIerror("The condition for the if-statement must evaluate to 'bool' and not to '%s' on line %d and column %d.", typeToString(determineType(IF_CONDITION(arg_node))), NODE_LINE(arg_node), NODE_COL(arg_node));
    }
    IF_IFBLOCK(arg_node) = TRAVopt(IF_IFBLOCK(arg_node), arg_info);
    IF_ELSEBLOCK(arg_node) = TRAVopt(IF_ELSEBLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCdoTypeCheck(node *syntaxtree) {
    DBUG_ENTER("TCdoTypeCheck");

    DBUG_PRINT("TC", ("Starting the type check."));

    info *arg_info = MakeInfo();

    TRAVpush(TR_tc);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_PRINT("TC", ("Finished the type check."));

    DBUG_RETURN(syntaxtree);
}
