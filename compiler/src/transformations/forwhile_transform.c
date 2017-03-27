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

#include "scope_utils.h"

#include "forwhile_transform.h"

/*
 * INFO structure
 */
struct INFO {
	node *funDef;
	node *currentStatement;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->funDef)
#define INFO_CURRENTSTATEMENT(n) ((n)->currentStatement)

/*
 * INFO functions
 */
static info *MakeInfo(void) {
	info* result;

	DBUG_ENTER("MakeInfo");

	result = (info*)MEMmalloc(sizeof(info));

	INFO_FUNDEF(result) = NULL;
    INFO_CURRENTSTATEMENT(result) = NULL;

	DBUG_RETURN(result);
}

static info* FreeInfo(info* info) {
	DBUG_ENTER("FreeInfo");

	info = MEMfree(info);

	DBUG_RETURN(info);
}

node *FWTfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("FWTfundef");

    // Make the current fundef node available for inside the body otherwise
    // we can't insert declaraions for the variables.
    node *previousFunDef = INFO_FUNDEF(arg_info);
    INFO_FUNDEF(arg_info) = arg_node;

    TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

    INFO_FUNDEF(arg_info) = previousFunDef;

    DBUG_RETURN(arg_node);
}

node *FWTstatements(node *arg_node, info *arg_info) {
    DBUG_ENTER("FWTstatements");

    // Make the current statements node available for the expression otherwise
    // we can't insert statements for the variables.
    INFO_CURRENTSTATEMENT(arg_info) = arg_node;
    TRAVdo(STATEMENTS_STATEMENT(arg_node), arg_info);
    TRAVopt(STATEMENTS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node* FWTfor(node* arg_node, info* arg_info) {
    DBUG_ENTER("FWTfor");

    node *funBody = FUNDEF_FUNBODY(INFO_FUNDEF(arg_info));
    node *symbolTable = FUNDEF_SYMBOLTABLE(INFO_FUNDEF(arg_info));

    // Get the name of the loop variable and create a variable for it (needed later)
    node *loopVar = TBmakeId(STRcpy(SYMBOLTABLEENTRY_NAME(VARDEF_DECL(FOR_VARDEF(arg_node)))));
    ID_TYPE(loopVar) = TY_int;
    ID_DECL(loopVar) = VARDEF_DECL(FOR_VARDEF(arg_node));
    NODE_LINE(loopVar) = NODE_LINE(FOR_VARDEF(arg_node));
    NODE_COL(loopVar) = NODE_COL(FOR_VARDEF(arg_node));

    // We are only allowed to determine the step and finish values once, so we need some
    // unique variable names to store their values.
    node *stepVar = TBmakeId(STRcat(ID_NAME(loopVar), "_step"));
    ID_TYPE(stepVar) = TY_int;
    NODE_LINE(stepVar) = NODE_LINE(FOR_STEP(arg_node));
    NODE_COL(stepVar) = NODE_COL(FOR_STEP(arg_node));

    node *finishVar = TBmakeId(STRcat(ID_NAME(loopVar), "_finish"));
    ID_TYPE(finishVar) = TY_int;
    NODE_LINE(finishVar) = NODE_LINE(FOR_FINISH(arg_node));
    NODE_COL(finishVar) = NODE_COL(FOR_FINISH(arg_node));

    // Now we have the names we need to declare variables
    node* stepVarDef = TBmakeVardef(FALSE, FALSE, STRcpy(ID_NAME(stepVar)), TY_int, NULL, NULL, NULL);
    node* finishVarDef = TBmakeVardef(FALSE, FALSE, STRcpy(ID_NAME(finishVar)), TY_int, NULL, NULL, NULL);

    // And store them in the SymbolTable and
    VARDEF_DECL(stepVarDef) = registerWithinCurrentScope(symbolTable, stepVarDef, ID_NAME(stepVar), STE_vardef, TY_int);
    SYMBOLTABLEENTRY_OFFSET(VARDEF_DECL(stepVarDef)) = SYMBOLTABLE_VARIABLES(symbolTable)++;
    SYMBOLTABLEENTRY_DECL(VARDEF_DECL(stepVarDef)) = stepVarDef;
    // Link from the variable to the VarDef
    ID_DECL(stepVar) = VARDEF_DECL(stepVarDef);

    VARDEF_DECL(finishVarDef) = registerWithinCurrentScope(symbolTable, finishVarDef, ID_NAME(finishVar), STE_vardef, TY_int);
    SYMBOLTABLEENTRY_OFFSET(VARDEF_DECL(finishVarDef)) = SYMBOLTABLE_VARIABLES(symbolTable)++;
    SYMBOLTABLEENTRY_DECL(VARDEF_DECL(finishVarDef)) = finishVarDef;
    // Link from the variable to the VarDef
    ID_DECL(finishVar) = VARDEF_DECL(finishVarDef);

    // Add them to the declarations list
    FUNBODY_VARDECS(funBody) = TBmakeVardecs(stepVarDef, FUNBODY_VARDECS(funBody));
    FUNBODY_VARDECS(funBody) = TBmakeVardecs(finishVarDef, FUNBODY_VARDECS(funBody));

    // Add assignment statements in front of the while loop
    node *stepVarAssign = TBmakeAssign(COPYdoCopy(stepVar), FOR_STEP(arg_node));
    node *finishVarAssign = TBmakeAssign(COPYdoCopy(finishVar), FOR_FINISH(arg_node));

    STATEMENTS_NEXT(INFO_CURRENTSTATEMENT(arg_info)) = TBmakeStatements(stepVarAssign, STATEMENTS_NEXT(INFO_CURRENTSTATEMENT(arg_info)));
    STATEMENTS_NEXT(INFO_CURRENTSTATEMENT(arg_info)) = TBmakeStatements(finishVarAssign, STATEMENTS_NEXT(INFO_CURRENTSTATEMENT(arg_info)));

  	// Depending on the sign of the step value the condition must be different
	// step > 0 ? loop-var < finish : loop-var > finish
	node* positiveStepCondition = TBmakeBinop(BO_lt, COPYdoCopy(loopVar), COPYdoCopy(finishVar));
	BINOP_TYPE(positiveStepCondition) = TY_bool;

    node* negativeStepCondition = TBmakeBinop(BO_gt, COPYdoCopy(loopVar), COPYdoCopy(finishVar));
    BINOP_TYPE(negativeStepCondition) = TY_bool;

    // Construct the ternary operator
    node *stepSelection = TBmakeBinop(BO_gt, COPYdoCopy(stepVar), TBmakeIntconst(TY_int, 0));
    BINOP_TYPE(stepSelection) = TY_bool;

    // Construct the ternary while condition
    node *whileCondition = TBmakeTernop(stepSelection, positiveStepCondition, negativeStepCondition);
    TERNOP_TYPE(whileCondition) = TY_bool;

	// Create Increment statement node for the end of the codeblock
    node* addInstruction = TBmakeBinop(BO_add, COPYdoCopy(loopVar), COPYdoCopy(stepVar));
    BINOP_TYPE(addInstruction) = TY_int;
    node* nextStep = TBmakeAssign(COPYdoCopy(loopVar), addInstruction);

	// Add increment statement at the end of the codeblock
	node* whileLoop = TBmakeWhile(whileCondition, TBmakeStatements(nextStep, FOR_BLOCK(arg_node)));

	// Free old For node
	FOR_VARDEF(arg_node) = NULL;
    FOR_STEP(arg_node) = NULL;
    FOR_FINISH(arg_node) = NULL;
	FOR_BLOCK(arg_node) = NULL;
	FREEfor(arg_node, arg_info);

	STATEMENTS_STATEMENT(INFO_CURRENTSTATEMENT(arg_info)) = whileLoop;
    arg_node = whileLoop;
	
	TRAVdo(WHILE_BLOCK(arg_node), arg_info);
	
    DBUG_RETURN(arg_node);
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
