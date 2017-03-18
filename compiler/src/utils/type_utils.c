/*
 * type_utils.c
 *
 *  Created on: 16 Mar 2017
 *      Author: nico
 */

#include "types_nodetype.h"
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
            return "<<TBD t>>";
    }
}
