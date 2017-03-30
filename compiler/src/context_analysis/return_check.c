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

/*
 * INFO structure
 */
struct INFO {
    int numberOfCodePaths;
    int numberOfReturns;
};

/*
 * INFO macros
 */
#define INFO_NUMBEROFCODEPATHS(n)    ((n)->numberOfCodePaths)
#define INFO_NUMBEROFRETURNS(n)       ((n)->numberOfReturns)

/*
 * INFO functions
 */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));
  INFO_NUMBEROFCODEPATHS(result) = 0;
  INFO_NUMBEROFRETURNS(result) = 0;

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

    // Only need to check non-void returning functions which are not imported to have the proper number of return statements
    if (FUNHEADER_RETURNTYPE(FUNDEF_FUNHEADER(arg_node)) != TY_void && !FUNDEF_EXTERN(arg_node)) {
        INFO_NUMBEROFCODEPATHS(arg_info)++;
        TRAVopt(FUNBODY_STATEMENTS(FUNDEF_FUNBODY(arg_node)), arg_info);

        if (INFO_NUMBEROFRETURNS(arg_info) == 0) {
            CTIerror("Function '%s' starting at line %d is missing at least one return statement.", FUNHEADER_NAME(FUNDEF_FUNHEADER(arg_node)), NODE_LINE(arg_node));
        } else if (INFO_NUMBEROFRETURNS(arg_info) != INFO_NUMBEROFCODEPATHS(arg_info)) {
            CTIwarn("Function '%s' starting at line %d has %d return statement(s) but %d are expected.", FUNHEADER_NAME(FUNDEF_FUNHEADER(arg_node)), NODE_LINE(arg_node), INFO_NUMBEROFRETURNS(arg_info), INFO_NUMBEROFCODEPATHS(arg_info));
        }
    }

    DBUG_RETURN(arg_node);
}

node *RCreturn(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCreturn");

    DBUG_PRINT("RC", ("Found a return statement at line %d", NODE_LINE(arg_node)));
    INFO_NUMBEROFRETURNS(arg_info)++;

    DBUG_RETURN(arg_node);
}

node *RCif(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCif");

    INFO_NUMBEROFCODEPATHS(arg_info)++;
    TRAVopt(IF_IFBLOCK(arg_node), arg_info);
    if (IF_ELSEBLOCK(arg_node)) {
        INFO_NUMBEROFCODEPATHS(arg_info)++;
        TRAVopt(IF_ELSEBLOCK(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node *RCdo(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCdo");

    INFO_NUMBEROFCODEPATHS(arg_info)++;
    TRAVopt(DO_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RCwhile(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCwhile");

    INFO_NUMBEROFCODEPATHS(arg_info)++;
    TRAVopt(WHILE_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RCfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCfor");

    INFO_NUMBEROFCODEPATHS(arg_info)++;
    TRAVopt(FOR_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RCdoReturnCheck(node *syntaxtree) {
    DBUG_ENTER("RCdoReturnCheck");

    DBUG_PRINT("RC", ("Starting the type check."));

    info *arg_info = MakeInfo();

    TRAVpush(TR_rc);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_PRINT("RC", ("Finished the type check."));

    DBUG_RETURN(syntaxtree);
}
