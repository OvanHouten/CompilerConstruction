/*
 * return_check.c
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

#include "return_check.h"

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

node *RCfunbody(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCfunbody");

    TRAVopt(FUNBODY_STATEMENTS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RCif(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCif");

    TRAVopt(IF_IFBLOCK(arg_node), arg_info);
    TRAVopt(IF_ELSEBLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RCfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCfor");

    TRAVopt(FOR_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RCwhile(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCwhile");

    TRAVopt(WHILE_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RCdo(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCdo");

    TRAVopt(DO_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RCstatements(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCstatements");

    node *statements = arg_node;
    while (statements) {
        if (NODE_TYPE(statements) == N_return) {
            if (STATEMENTS_NEXT(statements)) {
                CTIerror("The return statement at line [%d] and column [%d] must be the last statement.", NODE_LINE(statements), NODE_COL(statements));
            }
        }
        statements = STATEMENTS_NEXT(statements);
    }

    DBUG_RETURN(arg_node);
}

node *RCdoReturnCheck(node *syntaxtree) {
    DBUG_ENTER("RCdoReturnCheck");

    info *arg_info = MakeInfo();

    TRAVpush(TR_rc);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
