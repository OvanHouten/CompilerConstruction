#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"
#include "ctinfo.h"
#include "myglobals.h"

#include "short_circuit.h"

/*
 * INFO structure
 */
struct INFO {
	int non_empty;
};

/*
 * INFO macros
 */
#define INFO_NON_EMPTY(n)      ((n)->non_empty)

/*
 * INFO functions
 */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));
  
  INFO_NON_EMPTY(result) = 0;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  info = MEMfree( info);

  DBUG_RETURN( info);
}

node *SCBEexprs(node *arg_node, info *arg_info) {
    DBUG_ENTER("SCBEexpr");

	printf("HOI IK BEN NU HIER\n");

    DBUG_RETURN(arg_node);
}

node *SCBEdoShortCircuit(node *syntaxtree) {
    DBUG_ENTER("SCBEdoShortCircuit");

    info *arg_info = MakeInfo();

    TRAVpush(TR_scbe);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
