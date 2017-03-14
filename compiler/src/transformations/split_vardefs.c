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

    TRAVopt(FUNBODY_VARDECS(arg_node), arg_info);

    if (FUNBODY_STATEMENTS(arg_node)) {
        node *statements = FUNBODY_STATEMENTS(arg_node);
        while (STATEMENTS_NEXT(statements)) {
            statements = STATEMENTS_NEXT(statements);
        }
        STATEMENTS_NEXT(statements) = INFO_VARINITS(arg_info);
    } else {
        FUNBODY_STATEMENTS(arg_node) = INFO_VARINITS(arg_info);
    }
    INFO_VARINITS(arg_info) = NULL;

    INFO_FUNBODY(arg_info) = previousFunBody;

    TRAVopt(FUNBODY_LOCALFUNDEFS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SVvardecs(node *arg_node, info *arg_info) {
    DBUG_ENTER("SVvardecs");

    TRAVopt(VARDECS_NEXT(arg_node), arg_info);

    node *varDef = VARDECS_VARDEC(arg_node);
    if (VARDEF_EXPR(varDef)) {
        DBUG_PRINT("SV", ("Splitting [%s] from line [%d]", ID_NAME(VARDEF_ID(varDef)), NODE_LINE(arg_node)));
        // Remove the expression from the vardef
        node *expr = VARDEF_EXPR(varDef);
        VARDEF_EXPR(varDef)  = NULL;
        // Create new assign statement
        node *id = TBmakeId(STRcpy(ID_NAME(VARDEF_ID(varDef))));
        ID_DECL(id) = varDef;
        node *assignment = TBmakeAssign(id, expr);
        INFO_VARINITS(arg_info) = TBmakeStatements( assignment, INFO_VARINITS(arg_info));
    }

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
