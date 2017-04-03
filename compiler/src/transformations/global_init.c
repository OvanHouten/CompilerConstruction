/*
 * global_init.c
 *
 *  Created on: 14 Mar 2017
 *      Author: nico
 */

#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "copy_node.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"
#include "ctinfo.h"
#include "myglobals.h"
#include "list_utils.h"
#include "scope_utils.h"

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

node *GIprogram(node *arg_node, info *arg_info) {
    DBUG_ENTER("GIprogram");

    DBUG_PRINT("GI", ("Finding global variable definitions."));
    node *funBody = TBmakeFunbody(NULL, NULL, NULL);
    node *initSymbolTable = TBmakeSymboltable(NULL);
    node *declarations = PROGRAM_DECLARATIONS(arg_node);

    // Find all globaldefs (vardefs with an expr)
    while (declarations) {
        if (NODE_TYPE(DECLARATIONS_DECLARATION(declarations)) == N_vardef) {
            node *varDef = DECLARATIONS_DECLARATION(declarations);
            DBUG_PRINT("GI", ("Found [%s]", VARDEF_NAME(varDef)));
            if (VARDEF_EXPR(varDef)) {
                DBUG_PRINT("GI", ("Creating assignment."));
                // Pull out the expression
                node *expr = VARDEF_EXPR(varDef);
                VARDEF_EXPR(varDef) = NULL;
                // Create a new ID node
                node* id = TBmakeId(STRcpy(VARDEF_NAME(varDef)));
                NODE_LINE(id) = NODE_LINE(varDef);
                NODE_COL(id) = NODE_COL(varDef);
                // And create a list of assignment statements
                appendToStatements(funBody, TBmakeStatements(TBmakeAssign(id, expr), NULL));
                // And add it to the symboltable
                // TODO check why we don't use 'registerWithinCurrentScope'
                node *symbolTableEntry = TBmakeSymboltableentry(NULL);
                ID_DECL(id) = symbolTableEntry;
                SYMBOLTABLEENTRY_ENTRYTYPE(symbolTableEntry) = STE_varusage;
                SYMBOLTABLEENTRY_TYPE(symbolTableEntry) = VARDEF_TYPE(varDef);
                SYMBOLTABLEENTRY_NAME(symbolTableEntry) = STRcpy(VARDEF_NAME(varDef));
                SYMBOLTABLEENTRY_DEFNODE(symbolTableEntry) = varDef;

                // By design the global variables will be at distance 1 from the calling '__init' function.
                SYMBOLTABLEENTRY_DISTANCE(symbolTableEntry) = 1;
                SYMBOLTABLEENTRY_OFFSET(symbolTableEntry) = SYMBOLTABLEENTRY_OFFSET(VARDEF_DECL(varDef));
                appendToSymbolTableEntries(initSymbolTable, symbolTableEntry);
            }
        }
        declarations = DECLARATIONS_NEXT(declarations);
    }

    // If we have assignments create the spacial 'init' method.
    if (FUNBODY_STATEMENTS(funBody)) {
        DBUG_PRINT("GI", ("Creating '__init' function for globladefs."));
        node *initMethod = TBmakeFundef( FALSE, TRUE, TBmakeFunheader(TY_void, STRcpy("__init"), NULL), funBody, initSymbolTable);
        // And register it in the ST
        node *initSTE = registerWithinCurrentScope(PROGRAM_SYMBOLTABLE(arg_node), initMethod, "__init", STE_fundef, TY_void);
        FUNDEF_DECL(initMethod) = initSTE;
        SYMBOLTABLEENTRY_DEFNODE(initSTE) = initMethod;
        // And add the new function to the declarations section
        PROGRAM_DECLARATIONS(arg_node) = TBmakeDeclarations(initMethod, PROGRAM_DECLARATIONS(arg_node));
    } else {
        // Cleanup
        initSymbolTable = MEMfree(initSymbolTable);
        funBody = MEMfree(funBody);
    }

    DBUG_RETURN(arg_node);
}

node *GIdoGlobalInit(node *syntaxtree) {
    DBUG_ENTER("GIdoGlobalInit");

    info *arg_info = MakeInfo();

    TRAVpush(TR_gi);

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}

