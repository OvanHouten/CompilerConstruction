/*
 * split_vardefs.c
 *
 *  Created on: 14 Mar 2017
 *      Author: nico
 */

#include "split_vardefs.h"

#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"
#include "ctinfo.h"

#include "global_init.h"

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


node *SVfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("SVfundef");

    DBUG_RETURN(arg_node);
}

node *SVlocalfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("SVlocalfundef");

    DBUG_RETURN(arg_node);
}

node *SVvardef(node *arg_node, info *arg_info) {
    DBUG_ENTER("SVvardef");

    DBUG_RETURN(arg_node);
}

node *SVdoSplitVarDefs(node *syntaxtree) {
    DBUG_ENTER("SVSplitVarDefs");

    info *arg_info = MakeInfo();

    TRAVpush(TR_gi);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
