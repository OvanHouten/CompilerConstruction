#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"
#include "ctinfo.h"
#include "myglobals.h"
#include "mytypes.h"

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
#define INFO_NON_EMPTY(n) ((n)->non_empty)

/*
 * INFO functions
 */
static info *MakeInfo(void) {
	info *result;

	DBUG_ENTER( "MakeInfo");

	result = (info *)MEMmalloc(sizeof(info));

	INFO_NON_EMPTY(result) = 0;

	DBUG_RETURN( result);
}

static info *FreeInfo( info *info) {
	DBUG_ENTER ("FreeInfo");

	info = MEMfree( info);

	DBUG_RETURN( info);
}

node *SCBEbinop(node *arg_node, info *arg_info) {
    DBUG_ENTER("SCBEbinop");
	
	if(BINOP_OP(arg_node) == BO_and) {
		node* new_node = TBmakeTernop(TRAVdo(BINOP_LEFT(arg_node), arg_info), TRAVdo(BINOP_RIGHT(arg_node), arg_info), TBmakeBoolconst(TY_bool, FALSE));
		DBUG_RETURN(new_node);
	}
	else if(BINOP_OP(arg_node) == BO_or) {
		node* new_node = TBmakeTernop(TRAVdo(BINOP_LEFT(arg_node), arg_info), TBmakeBoolconst(TY_bool, TRUE), TRAVdo(BINOP_RIGHT(arg_node), arg_info));
		DBUG_RETURN(new_node);
	}
	
    DBUG_RETURN(arg_node);
}

node *SCBEunop(node *arg_node, info *arg_info) {
    DBUG_ENTER("SCBEunop");
	
	// MAYBE TERNARY OPERATOR AS WELL
	
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
