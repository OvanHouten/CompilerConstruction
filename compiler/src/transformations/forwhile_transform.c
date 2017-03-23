#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"
#include "ctinfo.h"
#include "myglobals.h"
#include "copy_node.h"
#include "mytypes.h"

#include "forwhile_transform.h"

/*
 * INFO structure
 */
struct INFO {
	int non_empty;
};

/*
 * INFO macros
 */
#define INFO_NON_EMPTY(n) ((n)->non_empty)

/*
 * INFO functions
 */
static info *MakeInfo(void) {
	info* result;

	DBUG_ENTER("MakeInfo");

	result = (info*)MEMmalloc(sizeof(info));

	INFO_NON_EMPTY(result) = 0;

	DBUG_RETURN(result);
}

static info* FreeInfo(info* info) {
	DBUG_ENTER("FreeInfo");

	info = MEMfree(info);

	DBUG_RETURN(info);
}

node* FWTfor(node* arg_node, info* arg_info) {
    DBUG_ENTER("FWTfor");
		
	// Create Id node for condition
	node* id = TBmakeId(VARDEF_NAME(FOR_VARDEF(arg_node)));
	ID_DECL(id) = VARDEF_DECL(FOR_VARDEF(arg_node));
	ID_TYPE(id) = VARDEF_TYPE(FOR_VARDEF(arg_node));
	
	// Create Binop node for condition	
	node* condition_node = TBmakeBinop(BO_lt, id, FOR_FINISH(arg_node));
	
	// Create Increment statement node for the end of the codeblock
	node* incr_node = TBmakeAssign(COPYid(id, arg_info), TBmakeBinop(BO_add, COPYid(id, arg_info), FOR_STEP(arg_node)));
	
	// Add increment statement at the end of the codeblock
	node* while_node = TBmakeWhile(condition_node, TBmakeStatements(incr_node, FOR_BLOCK(arg_node)));
	FOR_BLOCK(arg_node) = NULL;
	
	TRAVdo(WHILE_BLOCK(while_node), arg_info);
	
    DBUG_RETURN(while_node);
}

node* FWTdoForWhileTransform(node* syntaxtree) {
    DBUG_ENTER("FWTdoForWhileTransform");

    info *arg_info = MakeInfo();

    TRAVpush(TR_fwt);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}