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

#include "forloop.h"

/*
 * INFO structure
 */
struct INFO {
  struct node *currentScope;
};

/*
 * INFO macros
 */
#define FOR_CURRENTSCOPE(n)  ((n)->currentScope)

/*
 * INFO functions
 */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));
  result->currentScope = NULL;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  info->currentScope = NULL;
  info = MEMfree( info);

  DBUG_RETURN( info);
}
node *FLfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("FLfor");

    DBUG_PRINT("FL", ("For"));

    DBUG_RETURN(arg_node);
}

node *FLfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("FLfundef");

    DBUG_PRINT("FL", ("FunDef"));

    DBUG_RETURN(arg_node);
}

node *FLlocalfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("FLlocalfundef");

    DBUG_PRINT("FL", ("LocalFunDef"));

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



