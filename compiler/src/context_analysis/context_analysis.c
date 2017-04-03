/*
 * context_analysis.c
 *
 *  Created on: 3 Mar 2017
 *      Author: nico
 */

#include "str.h"
#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "ctinfo.h"
#include "type_utils.h"
#include "globals.h"
#include "scope_utils.h"
#include "string.h"

#include "context_analysis.h"

/*
 * INFO structure
 */
struct INFO {
	node* curScope;
	int externalFuns;
	int externalVars;
};

/*
 * INFO macros
 */
#define INFO_CURSCOPE(n)      ((n)->curScope)
#define INFO_EXTERNALFUNS(n)  ((n)->externalFuns)
#define INFO_EXTERNALVARS(n)  ((n)->externalVars)

/*
 * INFO functions
 */
static info *MakeInfo(void) {
	info *result;

	DBUG_ENTER( "MakeInfo");

	result = (info *)MEMmalloc(sizeof(info));
	INFO_CURSCOPE(result) = NULL;
	INFO_EXTERNALFUNS(result) = 0;
	INFO_EXTERNALVARS(result) = 0;

	DBUG_RETURN( result);
}

static info *FreeInfo( info *info) {
	DBUG_ENTER ("FreeInfo");

	info = MEMfree( info);

	DBUG_RETURN( info);
}

// =============================================
// Traversal code starts here
// =============================================

node *SAprogram(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAprogram");

	// Start new scope, change curscope, prevscope stays NULL;
	INFO_CURSCOPE(arg_info) = PROGRAM_SYMBOLTABLE(arg_node);
	
    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);
    
    PROGRAM_SYMBOLTABLE(arg_node) = INFO_CURSCOPE(arg_info);

    DBUG_RETURN(arg_node);
}

node *SAdeclarations(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAdeclarations");

    // Just register the name of the function
    if (NODE_TYPE(DECLARATIONS_DECLARATION(arg_node)) == N_fundef) {
        node *funDef = DECLARATIONS_DECLARATION(arg_node);
        node *funHeader = FUNDEF_FUNHEADER(funDef);
        char *name = FUNHEADER_NAME(funHeader);
        DBUG_PRINT("SA", ("Registering function [%s].", name));

        // Make sure it does not exist within the current scope
        node* funDefSTE = findWithinScope(INFO_CURSCOPE(arg_info), name, STE_fundef);
        if(funDefSTE) {        	
            CTIerror("Function [%s] at line %d, column %d has already been declared at line %d, column %d.", name, NODE_LINE(arg_node), NODE_COL(arg_node), NODE_LINE(funDefSTE), NODE_COL(funDefSTE));
        } else {
            funDefSTE = registerWithinCurrentScope(INFO_CURSCOPE(arg_info), funDef, name, STE_fundef, FUNHEADER_RETURNTYPE(funHeader));
            if (FUNDEF_EXTERN(funDef)) {
                SYMBOLTABLEENTRY_OFFSET(funDefSTE) = INFO_EXTERNALFUNS(arg_info)++;
                SYMBOLTABLEENTRY_LOCATION(funDefSTE) = LOC_extern;
            } else {
                SYMBOLTABLEENTRY_LOCATION(funDefSTE) = LOC_global;
            }

        }
        // Make sure we have a reference at hand to the STE
        FUNDEF_DECL(funDef) = funDefSTE;
        SYMBOLTABLEENTRY_DECL(funDefSTE) = funDef;
        DBUG_PRINT("SA", ("Registered function [%s] at offset [%d].", name, SYMBOLTABLEENTRY_OFFSET(funDefSTE)));
    }

    // Continue to register
    TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);

    // Now process the body of the function or the whole vardef
    TRAVdo(DECLARATIONS_DECLARATION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAfundef");
	
    DBUG_PRINT("SA", ("Processing a function definition."));
    if (FUNDEF_FUNBODY(arg_node)) {
			
        DBUG_PRINT("SA", ("Starting a new scope."));
        DBUG_PRINT("SA", ("1: %s", FUNHEADER_NAME(FUNDEF_FUNHEADER(arg_node))));
		// 	Start new scope
		node *previousScope = INFO_CURSCOPE(arg_info);
		node *newScope = TBmakeSymboltable(NULL);
		SYMBOLTABLE_PARENT(newScope) = previousScope;
		FUNDEF_SYMBOLTABLE(arg_node) = newScope;
		INFO_CURSCOPE(arg_info) = newScope;
		
		// Register the parameters
        TRAVdo(FUNDEF_FUNHEADER(arg_node), arg_info);
        // And process the body
        TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);
		
        DBUG_PRINT("SA", ("Closing the scope."));

        // Return to previous scope
		INFO_CURSCOPE(arg_info) = previousScope;
    }
	DBUG_PRINT("SA", ("Function definition is processed."));
	
    DBUG_RETURN(arg_node);
}

node *SAvardef(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAvardef");
	
    DBUG_PRINT("SA", ("Registering variable [%s].", VARDEF_NAME(arg_node)));
    
    // First we process the expression, if any
    TRAVopt(VARDEF_EXPR(arg_node), arg_info);

    // Make sure it does not exist within the current scope
    char *name = VARDEF_NAME(arg_node);
    node* varDefSTE = findWithinScope(INFO_CURSCOPE(arg_info), name, STE_vardef);
    if(varDefSTE) {
        if (SYMBOLTABLEENTRY_DISTANCE(varDefSTE) == 0) {
            CTIerror("Variable [%s] at line %d, column %d has already been declared at line %d, column %d.",
                    name, NODE_LINE(arg_node), NODE_COL(arg_node), NODE_LINE(varDefSTE), NODE_COL(varDefSTE));
        } else {
            // Declared at outer scope, register at local scope.
            varDefSTE = registerWithinCurrentScope(INFO_CURSCOPE(arg_info), arg_node, name, STE_varusage, VARDEF_TYPE(arg_node));
        }
	} else {
	    // Never seen this variable, register it
        varDefSTE = registerWithinCurrentScope(INFO_CURSCOPE(arg_info), arg_node, name, STE_vardef, VARDEF_TYPE(arg_node));
        if (VARDEF_EXTERN(arg_node)) {
            SYMBOLTABLEENTRY_OFFSET(varDefSTE) = INFO_EXTERNALVARS(arg_info)++;
            SYMBOLTABLEENTRY_LOCATION(varDefSTE) = LOC_extern;
        } else {
            SYMBOLTABLEENTRY_OFFSET(varDefSTE) = SYMBOLTABLE_VARIABLES(INFO_CURSCOPE(arg_info))++;
            if (SYMBOLTABLE_PARENT(INFO_CURSCOPE(arg_info)) == NULL) {
                SYMBOLTABLEENTRY_LOCATION(varDefSTE) = LOC_global;
            }
        }
        // And register a reference to the declaration node
        SYMBOLTABLEENTRY_DECL(varDefSTE) = arg_node;
	}
    // Make sure we have a reference at hand to the STE
    VARDEF_DECL(arg_node) = varDefSTE;

    DBUG_PRINT("SA", ("Registered variable [%s] at offset [%d].", VARDEF_NAME(arg_node), SYMBOLTABLEENTRY_OFFSET(varDefSTE)));

    DBUG_RETURN(arg_node);
}

node *SAid(node * arg_node, info * arg_info) {
    DBUG_ENTER("SAid");

    // Used for traversing to outer ST/scopes
    int distance = 0;
    node* varDefSTE = findInAnyScope(INFO_CURSCOPE(arg_info), ID_NAME(arg_node), &distance, STE_vardef);

    if(varDefSTE == NULL) {
        CTIerror("Variable [%s] which is used at line %d, column %d is not declared.", ID_NAME(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    } else {
        if(distance > 0) {
            DBUG_PRINT("SA", ("Defined in outer scope, creating a local STE."));
            // Defined in a outer scope, create new STE in current scope
            node* localSTE = registerWithinCurrentScope(INFO_CURSCOPE(arg_info), arg_node, ID_NAME(arg_node), STE_varusage, SYMBOLTABLEENTRY_TYPE(varDefSTE));
            // And link to the original declaration
            SYMBOLTABLEENTRY_DECL(localSTE) = SYMBOLTABLEENTRY_DECL(varDefSTE);
            // Set the correct distance and offset
            SYMBOLTABLEENTRY_OFFSET(localSTE) = SYMBOLTABLEENTRY_OFFSET(varDefSTE);
            SYMBOLTABLEENTRY_DISTANCE(localSTE) = distance;
            SYMBOLTABLEENTRY_LOCATION(localSTE) = SYMBOLTABLEENTRY_LOCATION(varDefSTE);

            varDefSTE = localSTE;
        }
        // Make sure we can reference the STE
        ID_DECL(arg_node) = varDefSTE;
    }

    DBUG_RETURN(arg_node);
}

node *SAfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAfuncall");

    DBUG_PRINT("SA", ("Processing a FunCall"));
    char *name = FUNCALL_NAME(arg_node);
    DBUG_PRINT("SA", ("Trying to find the declaration of function [%s].", name));
    int distance = 0;
    node *funDefSTE = findInAnyScope(INFO_CURSCOPE(arg_info), name, &distance, STE_fundef);
    if (funDefSTE) {
        DBUG_PRINT("SA", ("It is a known function"));
        if (distance > 0) {
            DBUG_PRINT("SA", ("Function defined in outer scope registering at local ST."));
            // Defined in a outer scope, create new STE in current scope
            node* localSTE = registerWithinCurrentScope(INFO_CURSCOPE(arg_info), arg_node, name, STE_fundef, SYMBOLTABLEENTRY_TYPE(funDefSTE));
            SYMBOLTABLEENTRY_DECL(localSTE) = SYMBOLTABLEENTRY_DECL(funDefSTE);
            // Set the correct distance and offset
            SYMBOLTABLEENTRY_OFFSET(localSTE) = SYMBOLTABLEENTRY_OFFSET(funDefSTE);
            SYMBOLTABLEENTRY_DISTANCE(localSTE) = distance;

            funDefSTE = localSTE;
        }
        FUNCALL_DECL(arg_node) = funDefSTE;

        TRAVopt(FUNCALL_EXPRS(arg_node), arg_info);

        DBUG_PRINT("SA", ("Performing param and expression count check..."));
        int exprCount = 0;
        node *exprs = FUNCALL_EXPRS(arg_node);
        while (exprs) {
            exprCount++;
            exprs = EXPRS_NEXT(exprs);
        }
        DBUG_PRINT("SA", ("Expressions counted, counting parameters."));
        node *funHeader = FUNDEF_FUNHEADER(SYMBOLTABLEENTRY_DECL(funDefSTE));
        int paramCount = 0;
        node *params = FUNHEADER_PARAMS(funHeader);
        while (params) {
            paramCount++;
            params = PARAMS_NEXT(params);
        }
        DBUG_PRINT("SA", ("The function has [%d] params and there are [%d] expressions.", paramCount, exprCount));
        if (paramCount != exprCount) {
            CTIerror("The number of parameters [%d] as used at line [%d] and column [%d] do not match the number of parameters [%d] to the function [%s] as defined at line [%d].", exprCount, NODE_LINE(arg_node), NODE_COL(arg_node), paramCount, name, NODE_LINE(funHeader));
        }
    } else {
        CTIerror("Function [%s] at line %d, column %d has not yet been declared.", name, NODE_LINE(arg_node), NODE_COL(arg_node));
    }
    DBUG_PRINT("SA", ("FunCall is processed."));

    DBUG_RETURN(arg_node);
}

node *SAparams(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAparams");

    TRAVopt(PARAMS_NEXT(arg_node), arg_info);
    TRAVdo(PARAMS_PARAM(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAvardecs(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAvardecs");

    TRAVopt(VARDECS_NEXT(arg_node), arg_info);
    TRAVdo(VARDECS_VARDEC(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAassign");

    DBUG_PRINT("SA", ("Processing the RH-side"));
    TRAVopt(ASSIGN_EXPR(arg_node), arg_info);
    DBUG_PRINT("SA", ("Processing the LH-side"));
    TRAVdo(ASSIGN_LET(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAfor");

    DBUG_PRINT("SA", ("Processing the start, stop and step expressions."));
    node *varDef = FOR_VARDEF(arg_node);
    TRAVdo(VARDEF_EXPR(varDef), arg_info);
    TRAVdo(FOR_FINISH(arg_node), arg_info);
    TRAVopt(FOR_STEP(arg_node), arg_info);

    // Only go through the trouble if it is really useful
    if (FOR_BLOCK(arg_node)) {
        DBUG_PRINT("SA", ("Looking for existing name."));
        char *name = VARDEF_NAME(varDef);
        node *existingVarDef = findWithinScope(INFO_CURSCOPE(arg_info), name, STE_vardef);

        char *originalName = NULL;
        if (existingVarDef) {
            DBUG_PRINT("SA", ("Hiding the existing name in the symboltable for now."));
            // Remember the name and remove it (by giving it a different name) from the ST
            originalName = SYMBOLTABLEENTRY_NAME(existingVarDef);
            SYMBOLTABLEENTRY_NAME(existingVarDef) = "";
        }

        // Register the variable, now all occurrences of our vardef name will get a STE entry to us
        node *forVarEntry = registerWithinCurrentScope(INFO_CURSCOPE(arg_info), varDef, name, STE_vardef, TY_int);
        VARDEF_DECL(varDef) = forVarEntry;
        SYMBOLTABLEENTRY_DECL(forVarEntry) = varDef;
        SYMBOLTABLEENTRY_OFFSET(forVarEntry) = SYMBOLTABLE_VARIABLES(INFO_CURSCOPE(arg_info))++;
        // Process the block
        DBUG_PRINT("SA", ("Processing the block."));
        FOR_BLOCK(arg_node) = TRAVdo(FOR_BLOCK(arg_node), arg_info);

        // And now replace our name with the generated one and restore the original
        if (existingVarDef) {
            DBUG_PRINT("SA", ("Restoring the original name and generating a unique name."));
            SYMBOLTABLEENTRY_NAME(existingVarDef) = originalName;
            // If the name exists within the current scope generate a new unique name.
            SYMBOLTABLEENTRY_NAME(forVarEntry) = createUniqueNameForSymbolTable(INFO_CURSCOPE(arg_info), name, STE_vardef);
        }
    }

    DBUG_RETURN(arg_node);
}

node *SAexprs(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAexprs");

    TRAVopt(EXPRS_NEXT(arg_node), arg_info);
    TRAVdo(EXPRS_EXPR(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAstatements(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAstatements");

    DBUG_PRINT("SA", ("Going to next statement"));
    TRAVopt(STATEMENTS_NEXT(arg_node), arg_info);
    TRAVdo(STATEMENTS_STATEMENT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAdoScopeAnalysis( node *syntaxtree) {
    DBUG_ENTER("SAdoScopeAnslysis");

    info *arg_info = MakeInfo();

    TRAVpush(TR_sa);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
