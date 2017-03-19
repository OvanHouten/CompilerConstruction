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
#include "type_utils.h"

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

node *RCfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCfundef");

    if (!FUNDEF_EXTERN(arg_node)) {
        type returnType = FUNHEADER_RETURNTYPE(FUNDEF_FUNHEADER(arg_node));
        node *returnStatement = NULL;
        node *funBody = FUNDEF_FUNBODY(arg_node);
        if (funBody) {
            if (FUNBODY_STATEMENTS(funBody)) {
                returnStatement = STATEMENTS_STATEMENT(FUNBODY_STATEMENTS(funBody));
            }
        }
        if (returnStatement) {
            type returningType = TY_unknown;
            if (NODE_TYPE(returnStatement) == N_return) {
                if (RETURN_EXPR(returnStatement)) {
                    returningType = TY_unknown; // FIXME RETURN_EXPR(returnStatement);
                } else {
                    returningType = TY_void;
                }
            }
            if (returnType != returningType) {
                CTIerror("The function [%s] at line [%d] must end with a '%s' returning statement.", FUNHEADER_NAME(FUNDEF_FUNHEADER(arg_node)), NODE_LINE(arg_node), typeToString(returningType));
            }
        } else {
            // Looks like an empty function
            if (returnType != TY_void) {
                CTIerror("The function [%s] at line [%d] must end with a '%s' returning statement.", FUNHEADER_NAME(FUNDEF_FUNHEADER(arg_node)), NODE_LINE(arg_node), typeToString(returnType));
            }
        }
    }

    DBUG_RETURN(arg_node);
}

node *RClocalfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("RClocalfundef");

    TRAVopt(FUNBODY_STATEMENTS(arg_node), arg_info);

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
