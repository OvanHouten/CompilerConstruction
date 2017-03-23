/*
 * type_utils.c
 *
 *  Created on: 16 Mar 2017
 *      Author: nico
 */

#include "dbug.h"
#include "types_nodetype.h"
#include "tree_basic.h"
#include "mytypes.h"

#include "type_utils.h"

char *typeToString(type typeInfo) {
    switch (typeInfo) {
        case TY_int:
            return "int";
        case TY_float:
            return "float";
        case TY_bool:
            return "bool";
        case TY_void:
            return "void";
        case TY_unknown :
            return "unknown";
        default:
            // Just a precaution for future expansion
            return "<<TBD>>";
    }
}

type determineType(node *expr) {
    DBUG_ENTER("determineType");

    type exprType = TY_unknown;
    DBUG_PRINT("UTIL", ("Determining type for [%d]", NODE_TYPE(expr)));
    switch (NODE_TYPE(expr)) {
        case N_funcall :
            exprType = SYMBOLTABLEENTRY_TYPE(FUNCALL_DECL(expr));
            break;
        case N_id :
            exprType = SYMBOLTABLEENTRY_TYPE(ID_DECL(expr));
            break;
        case N_vardef :
            exprType = VARDEF_TYPE(expr);
            break;
        case N_typecast :
            exprType = TYPECAST_TYPE(expr);
            break;
        case N_unop:
            exprType = UNOP_TYPE(expr);
            break;
        case N_binop:
            exprType = BINOP_TYPE(expr);
            break;
        case N_return :
            exprType = RETURN_TYPE(expr);
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
    DBUG_PRINT("UTIL", ("Type [%d]", exprType));

    DBUG_RETURN(exprType);
}
