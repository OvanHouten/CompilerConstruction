/*
 * type_utils.c
 *
 *  Created on: 16 Mar 2017
 *      Author: nico
 */

#include "types_nodetype.h"
#include "mytypes.h"

#include "type_utils.h"

type nodeTypeToType(nodetype typeInfo) {
    switch (typeInfo) {
        case N_int:
            return TY_int;
        case N_float:
            return TY_float;
        case N_bool:
            return TY_bool;
        case N_void :
            return TY_void;
        default:
            return TY_unknown;
    }
}

char *nodeTypeToString(nodetype typeInfo) {
    switch (typeInfo) {
        case N_int:
            return "int";
        case N_float:
            return "float";
        case N_bool:
            return "bool";
        case N_void :
            return "void";
        default:
            return "<<TBD nt>>";
    }
}

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
            return "<<TBD t>>";
    }
}
