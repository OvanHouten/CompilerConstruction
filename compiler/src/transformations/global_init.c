/*
 * global_init.c
 *
 *  Created on: 14 Mar 2017
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

#include "global_init.h"

node *GIprogram(node *arg_node, info *arg_info) {
    DBUG_ENTER("GIprogram");

    DBUG_RETURN(arg_node);
}

node *GIdoGlobalInit(node *syntaxtree) {
    DBUG_ENTER("GIdoGlobalInit");

    DBUG_RETURN(syntaxtree);
}

