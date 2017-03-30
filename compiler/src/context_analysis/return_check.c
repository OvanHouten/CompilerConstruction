/*
 * return_check.c
 *
 *  Created on: 30 Mar 2017
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

#include "type_utils.h"

#include "return_check.h"

node *RCfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCfundef");

    // Only need to check non-void returning functions which are not imported to have the proper number of return statements
    if (FUNHEADER_RETURNTYPE(FUNDEF_FUNHEADER(arg_node)) != TY_void && !FUNDEF_EXTERN(arg_node)) {
        node *statement = FUNBODY_STATEMENTS(FUNDEF_FUNBODY(arg_node));
        node *returnStatement = NULL;
        while (returnStatement == NULL && statement != NULL) {
            if (NODE_TYPE(STATEMENTS_STATEMENT(statement)) == N_return)
                returnStatement = statement;
            statement = STATEMENTS_NEXT(statement);
        }

        if (returnStatement == NULL) {
            CTIerror("Function '%s' starting at line %d is missing at least one return statement.", FUNHEADER_NAME(FUNDEF_FUNHEADER(arg_node)), NODE_LINE(arg_node));
        } else if (returnStatement != FUNBODY_STATEMENTS(FUNDEF_FUNBODY(arg_node))) {
            node *blabla = STATEMENTS_STATEMENT(returnStatement);
            int bla = NODE_LINE(blabla);
            CTIwarn("Function '%s' contains unreachable code after the return statement at line %d.", FUNHEADER_NAME(FUNDEF_FUNHEADER(arg_node)), bla);
        }
    }

    DBUG_RETURN(arg_node);
}

node *RCdoReturnCheck(node *syntaxtree) {
    DBUG_ENTER("RCdoReturnCheck");

    DBUG_PRINT("RC", ("Starting the return check."));

    TRAVpush(TR_rc);

    TRAVdo(syntaxtree, NULL);

    TRAVpop();

    DBUG_PRINT("RC", ("Finished the return check."));

    DBUG_RETURN(syntaxtree);
}
