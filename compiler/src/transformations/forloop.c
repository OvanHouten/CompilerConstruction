/*
 * forloop.c
 *
 *  Created on: 13 Mar 2017
 *      Author: nico
 */

#include <math.h>

#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"
#include "ctinfo.h"

#include "forloop.h"

/*
 * INFO structure
 */
struct INFO {
  node *fundef;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n)  ((n)->fundef)

/*
 * INFO functions
 */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));
  INFO_FUNDEF(result) = NULL;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  INFO_FUNDEF(info) = NULL;
  info = MEMfree( info);

  DBUG_RETURN( info);
}

bool isUnique(node *varDecs, char *name) {
    while (varDecs) {
        if (STReq(name, ID_NAME(VARDEF_ID(VARDECS_VARDEC(varDecs))))) {
            return FALSE;
        }
        varDecs = VARDECS_NEXT(varDecs);
    }
    return TRUE;
}

char *createUniqueName(node *varDecs, char *name) {
    int duplicates = 0;
    char *newName = NULL;
    do {
        int numberOfDigits = duplicates == 0 ? 1 : 1 + log10(duplicates);
        // 8 is the number of character in out pattern + room for the trailing zero
        newName = MEMmalloc(8 + STRlen(name) + numberOfDigits);
        sprintf(newName, "__for_%s_%d", name, duplicates);
        if (!isUnique(varDecs, newName)) {
            duplicates++;
            newName = MEMfree(newName);
        }
    } while (newName == NULL);
    MEMfree(name);
    return newName;
}
node *FLfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("FLfor");

    DBUG_PRINT("FL", ("For"));
    DBUG_ASSERT(INFO_FUNDEF(arg_info) != NULL, "A for loop can't exist outside a function!");

    node *funBody = NULL;
    if (NODE_TYPE(INFO_FUNDEF(arg_info)) == N_fundef) {
        DBUG_PRINT("FL", ("For loop inside function."));
        funBody = FUNDEF_FUNBODY(INFO_FUNDEF(arg_info));
    } else if (NODE_TYPE(INFO_FUNDEF(arg_info)) == N_localfundef) {
        DBUG_PRINT("FL", ("For loop inside localfunction."));
        funBody = LOCALFUNDEF_FUNBODY(INFO_FUNDEF(arg_info));
    }

    if (funBody) {
        node *varDecs = FUNBODY_VARDECS(funBody);
        node *loopVar = FOR_VARDEF(arg_node);
        ID_NAME(VARDEF_ID(loopVar)) = createUniqueName(varDecs, ID_NAME(VARDEF_ID(loopVar)));
        FUNBODY_VARDECS(funBody) = TBmakeVardecs(loopVar, varDecs);
        FOR_VARDEF(arg_node) = NULL;
        if (varDecs) {
            VARDEF_OFFSET(loopVar) = VARDEF_OFFSET(VARDECS_VARDEC(varDecs)) + 1;
            ID_OFFSET(VARDEF_ID(loopVar)) = VARDEF_OFFSET(loopVar);
        }
        TRAVopt(FOR_BLOCK(arg_node), arg_info);
    } else {
        CTIerror("A for-loop without a surrounding (local)function!");
    }

    DBUG_RETURN(arg_node);
}

node *FLid(node *arg_node, info *arg_info) {
    DBUG_ENTER("FLid");

    // Make sure the ID belongs to a vardef and not a fundef
    if (ID_DECL(arg_node) != NULL && NODE_TYPE(ID_DECL(arg_node)) == N_vardef) {
        // If the offset changed it must belong to a ID that referenced the loop variable
        if (ID_OFFSET(arg_node) != VARDEF_OFFSET(ID_DECL(arg_node))) {
            ID_OFFSET(arg_node) = VARDEF_OFFSET(ID_DECL(arg_node));
            // TODO use the SymbolTable AST nodes for determining the real distance
            // The distance is incorrect if the for-loop is currently within any other block.
            ID_DISTANCE(arg_node)++;
        }
    }
    DBUG_RETURN(arg_node);
}

node *FLfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("FLfundef");

    node *previousScope = INFO_FUNDEF(arg_info);
    INFO_FUNDEF(arg_info) = arg_node;

    TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

    INFO_FUNDEF(arg_info) = previousScope;

    DBUG_RETURN(arg_node);
}

node *FLlocalfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("FLlocalfundef");

    node *previousScope = INFO_FUNDEF(arg_info);
    INFO_FUNDEF(arg_info) = arg_node;

    TRAVopt(LOCALFUNDEF_FUNBODY(arg_node), arg_info);

    INFO_FUNDEF(arg_info) = previousScope;

    DBUG_RETURN(arg_node);
}

node *FLdoForLoop(node *syntaxtree) {
    DBUG_ENTER("FLdoForloop");

    info *arg_info = MakeInfo();

    TRAVpush(TR_fl);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}



