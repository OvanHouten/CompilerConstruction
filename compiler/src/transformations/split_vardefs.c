/*
 * split_vardefs.c
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
#include "myglobals.h"
#include "copy.h"

#include "list_utils.h"
#include "split_vardefs.h"

/*
 * INFO structure
 */
struct INFO {
  node *funbody;
  node *varInits;
};

/*
 * INFO macros
 */
#define INFO_FUNBODY(n)  ((n)->funbody)
#define INFO_VARINITS(n)  ((n)->varInits)

/*
 * INFO functions
 */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));
  INFO_FUNBODY(result) = NULL;
  INFO_VARINITS(result) = NULL;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  info = MEMfree( info);

  DBUG_RETURN( info);
}

node *SVfunbody(node *arg_node, info *arg_info) {
    DBUG_ENTER("SVfunbody");

    node *previousFunBody = INFO_FUNBODY(arg_info);
    INFO_FUNBODY(arg_info) = arg_node;

    // Start with a clean slate
    INFO_VARINITS(arg_info) = NULL;
    FUNBODY_VARDECS(arg_node) = TRAVopt(FUNBODY_VARDECS(arg_node), arg_info);

    arg_node = appendToStatements(arg_node, INFO_VARINITS(arg_info));
    INFO_VARINITS(arg_info) = NULL;

    FUNBODY_STATEMENTS(arg_node) = TRAVopt(FUNBODY_STATEMENTS(arg_node), arg_info);

    INFO_FUNBODY(arg_info) = previousFunBody;

    DBUG_RETURN(arg_node);
}

node *SVvardecs(node *arg_node, info *arg_info) {
    DBUG_ENTER("SVvardecs");

    DBUG_PRINT("SV", ("Next..."));
    TRAVopt(VARDECS_NEXT(arg_node), arg_info);

    DBUG_PRINT("SV", ("Checking [%s]", VARDEF_NAME(VARDECS_VARDEC(arg_node))));
    node *varDef = VARDECS_VARDEC(arg_node);
    if (VARDEF_EXPR(varDef)) {
        DBUG_PRINT("SV", ("Splitting [%s] from line [%d]", VARDEF_NAME(varDef), NODE_LINE(arg_node)));
        // Remove the expression from the vardef
        node *expr = VARDEF_EXPR(varDef);
        VARDEF_EXPR(varDef) = NULL;
        node* id = TBmakeId(STRcpy(VARDEF_NAME(varDef)), NULL); // TODO array
        ID_DECL(id) = VARDEF_DECL(varDef);
        NODE_LINE(id) = NODE_LINE(varDef);
        NODE_COL(id) = NODE_COL(varDef);

        // Create new assign statement
        node *assignment = TBmakeAssign(id, expr);

        INFO_VARINITS(arg_info) = TBmakeStatements( assignment, INFO_VARINITS(arg_info));
    } else {
        DBUG_PRINT("SV", ("Nothing to split on line %d.", NODE_LINE(varDef)));
    }

    DBUG_RETURN(arg_node);
}

node *SVstatements(node *arg_node, info *arg_info) {
    DBUG_ENTER("SVstatements");

    if (NODE_TYPE(STATEMENTS_STATEMENT(arg_node)) == N_for) {
        DBUG_PRINT("SV", ("Processing a for statement."));
        node *forNode = STATEMENTS_STATEMENT(arg_node);
        node *varDef = FOR_VARDEF(forNode);

        DBUG_PRINT("SV", ("Splitting [%s] from line [%d]", VARDEF_NAME(varDef), NODE_LINE(forNode)));
        // Remove the expression from the vardef
        node *expr = VARDEF_EXPR(varDef);
        VARDEF_EXPR(varDef)  = NULL;
        FUNBODY_VARDECS(INFO_FUNBODY(arg_info)) = TBmakeVardecs( varDef, FUNBODY_VARDECS(INFO_FUNBODY(arg_info)));

        // Create a copy for the var-loop variable
        node *id = TBmakeId(STRcpy(VARDEF_NAME(varDef)), NULL); //TODO array
        ID_DECL(id) = VARDEF_DECL(varDef);
        NODE_LINE(id) = NODE_LINE(varDef);
        NODE_COL(id) = NODE_COL(varDef);

        // Put assignment (the expression to the new ID copy)statement in front of the for-loop.
        node *assignment = TBmakeAssign(id, expr);
        STATEMENTS_NEXT(arg_node) = TBmakeStatements(assignment, STATEMENTS_NEXT(arg_node));

        TRAVopt(FOR_BLOCK(forNode), arg_info);
    } else {
        TRAVdo(STATEMENTS_STATEMENT(arg_node), arg_info);
    }
    STATEMENTS_NEXT(arg_node) = TRAVopt(STATEMENTS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SVdoSplitVarDefs(node *syntaxtree) {
    DBUG_ENTER("SVSplitVarDefs");

    info *arg_info = MakeInfo();

    TRAVpush(TR_sv);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
