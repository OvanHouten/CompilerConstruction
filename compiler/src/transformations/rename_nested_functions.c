/*
 * rename_nested_functions.c
 *
 *  Created on: 28 Mar 2017
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

#include "rename_nested_functions.h"

/*
 * INFO structure
 */
struct INFO {
    char *outerFunctionName;
};

/*
 * INFO macros
 */
#define INFO_OUTERFUNCTIONNAME(n) ((n)->outerFunctionName)

/*
 * INFO functions
 */
static info *MakeInfo(void) {
    info* result;

    DBUG_ENTER("MakeInfo");

    result = (info*)MEMmalloc(sizeof(info));

    INFO_OUTERFUNCTIONNAME(result) = NULL;

    DBUG_RETURN(result);
}

static info* FreeInfo(info* info) {
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node *RNFfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("RNFfundef");

    DBUG_PRINT("RNF", ("Processing function '%s' from line %d", FUNHEADER_NAME(FUNDEF_FUNHEADER(arg_node)), NODE_LINE(arg_node)));
    char *currentOuterName = INFO_OUTERFUNCTIONNAME(arg_info);
    if (currentOuterName) {
        DBUG_PRINT("RNF", ("Pre-pending '%s' for functions inside '%s'", currentOuterName, SYMBOLTABLEENTRY_NAME(FUNDEF_DECL(arg_node))));
        char *functionName = SYMBOLTABLEENTRY_NAME(FUNDEF_DECL(arg_node));
        SYMBOLTABLEENTRY_NAME(FUNDEF_DECL(arg_node)) = STRcatn(3, currentOuterName, "_", functionName);
        DBUG_PRINT("RNF", ("Renamed to '%s'\t\t\t%p", SYMBOLTABLEENTRY_NAME(FUNDEF_DECL(arg_node)), FUNDEF_DECL(arg_node)));
    }
    // Skip the __init function
    if (FUNDEF_DECL(arg_node)) {
        INFO_OUTERFUNCTIONNAME(arg_info) = SYMBOLTABLEENTRY_NAME(FUNDEF_DECL(arg_node));
    } else {
        CTIwarn("Possible duplicate functions will fail inside '%s'.", FUNHEADER_NAME(FUNDEF_FUNHEADER(arg_node)));
    }

    TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

    DBUG_PRINT("RNF", ("Renaming is done for '%s'", SYMBOLTABLEENTRY_NAME(FUNDEF_DECL(arg_node))));
    INFO_OUTERFUNCTIONNAME(arg_info) = currentOuterName;

    DBUG_RETURN(arg_node);
}

node* RNFdoRenameNestedFunctions(node* syntaxtree) {
    DBUG_ENTER("RNFdoRenameNestedFunctions");

    info *arg_info = MakeInfo();

    TRAVpush(TR_rnf);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
