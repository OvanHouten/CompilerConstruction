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
#include "free_node.h"
#include "mytypes.h"
#include "copy.h"

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
	
	// FIXME The Start, Finish and Step must be executed only once! So we must create a couple of variables and
	// use them
	// Depending on the sign of the step value the condition must be different
	// step > 0 ? loop-var < finish : loop-var > finish
	node* positiveStepCondition = TBmakeBinop(BO_lt, COPYdoCopy(id), COPYdoCopy(FOR_FINISH(arg_node)));
	BINOP_TYPE(positiveStepCondition) = TY_bool;

	node* negativeStepCondition = TBmakeBinop(BO_gt, COPYdoCopy(id), COPYdoCopy(FOR_FINISH(arg_node)));
    BINOP_TYPE(negativeStepCondition) = TY_bool;

    // Construct the ternary operator
    node *stepSelection = TBmakeBinop(BO_ge, COPYdoCopy(FOR_STEP(arg_node)), TBmakeIntconst(TY_int, 0));
    BINOP_TYPE(stepSelection) = TY_bool;

    // Construct the ternary while condition
    node *whileCondition = TBmakeTernop(stepSelection, positiveStepCondition, negativeStepCondition);
    TERNOP_TYPE(whileCondition) = TY_bool;
	
	// Create Increment statement node for the end of the codeblock
    node* addInstruction = TBmakeBinop(BO_add, COPYdoCopy(id), FOR_STEP(arg_node));
    BINOP_TYPE(addInstruction) = TY_int;
    node* nextStep = TBmakeAssign(COPYid(id, arg_info), addInstruction);
	
	// Add increment statement at the end of the codeblock
	node* whileLoop = TBmakeWhile(whileCondition, TBmakeStatements(nextStep, FOR_BLOCK(arg_node)));
	
	// Free old For node
	FOR_BLOCK(arg_node) = NULL;
	FOR_STEP(arg_node) = NULL;
	FOR_FINISH(arg_node) = NULL;
	FREEfor(arg_node, arg_info);
	
	TRAVdo(WHILE_BLOCK(whileLoop), arg_info);
	
    DBUG_RETURN(whileLoop);
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
