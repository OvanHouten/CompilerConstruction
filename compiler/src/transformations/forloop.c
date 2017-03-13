/*
 * forloop.c
 *
 *  Created on: 13 Mar 2017
 *      Author: nico
 */

#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
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
        node *lastVarDec = FUNBODY_VARDECS(funBody);
        node *loopVar = FOR_VARDEF(arg_node);
        FUNBODY_VARDECS(funBody) = TBmakeVardecs(loopVar, lastVarDec);
        FOR_VARDEF(arg_node) = NULL;
        if (lastVarDec) {
            VARDEF_OFFSET(loopVar) = VARDEF_OFFSET(VARDECS_VARDEC(lastVarDec)) + 1;
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



