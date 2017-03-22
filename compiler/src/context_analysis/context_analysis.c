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
#include "lookup_table.h"
#include "ctinfo.h"
#include "math.h"
#include "type_utils.h"

#include "context_analysis.h"

/*
 * INFO structure
 */
struct INFO {
  node* curScope;
};

/*
 * INFO macros
 */
#define INFO_CURSCOPE(n)      ((n)->curScope)

// The Lookup table (lut) returns a pointer to the original pointer that was provided while inserting
// the new value. By using this macro we can get the original pointer back without any hassle.
#define DEREF_IF_NOT_NULL(n) (n == NULL ? n : *n)

/*
 * INFO functions
 */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));
  result->curScope = NULL;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  info = MEMfree( info);

  DBUG_RETURN( info);
}

// =============================================
// Scope handling
// =============================================

node* findDefWithinScope(info* arg_info, char* name, ste_type type) {
    DBUG_ENTER("findDefWithinScope");
    node* varDefSTE = SYMBOLTABLE_SYMBOLTABLEENTRY(INFO_CURSCOPE(arg_info));
    while (varDefSTE) {
        if (SYMBOLTABLEENTRY_ENTRYTYPE(varDefSTE) == type && STReq(name, SYMBOLTABLEENTRY_NAME(varDefSTE))) {
            break;
        }
        varDefSTE = SYMBOLTABLEENTRY_NEXT(varDefSTE);
    }
    DBUG_RETURN(varDefSTE);
}

node* findInAnyScope(info* arg_info, char *name, int* distance, ste_type type) {
    DBUG_ENTER("findInAnyScope");

    DBUG_PRINT("SA", ("Trying to locate variable [%s] in the symbol table.", name));
    // Used for traversing to outer ST/scopes
    node* lookupST = INFO_CURSCOPE(arg_info);
    node* varDefSTE = SYMBOLTABLE_SYMBOLTABLEENTRY(lookupST);
    while (varDefSTE || lookupST) {
        if (varDefSTE) {
            if (SYMBOLTABLEENTRY_ENTRYTYPE(varDefSTE) == type && STReq(name, SYMBOLTABLEENTRY_NAME(varDefSTE))) {
                DBUG_PRINT("SA", ("Found [%s] at distance [%d] offset [%d].", name, *distance, SYMBOLTABLEENTRY_OFFSET(varDefSTE)));
                break;
            }
            DBUG_PRINT("SA", ("Trying next entry."));
            varDefSTE = SYMBOLTABLEENTRY_NEXT(varDefSTE);
        } else {
            DBUG_PRINT("SA", ("Trying outer scope for [%s].", name));
            (*distance)++;
            lookupST = SYMBOLTABLE_PARENT(lookupST);
            if (lookupST) {
                varDefSTE = SYMBOLTABLE_SYMBOLTABLEENTRY(lookupST);
            }
        }
    }
    DBUG_RETURN(varDefSTE);
}

bool isUniqueInSymbolTable(node *symbolTableEntry, char *name, ste_type type) {
    while (symbolTableEntry) {
        if (SYMBOLTABLEENTRY_ENTRYTYPE(symbolTableEntry) == type && STReq(name, SYMBOLTABLEENTRY_NAME(symbolTableEntry))) {
            return FALSE;
        }
        symbolTableEntry = SYMBOLTABLEENTRY_NEXT(symbolTableEntry);
    }
    return TRUE;
}

char *createUniqueNameForSymbolTable(node *symbolTable, char *name, ste_type type) {
    DBUG_ENTER("createUniqueNameForSymbolTable");
    int duplicates = 0;
    char *newName = NULL;
    do {
        int numberOfDigits = duplicates == 0 ? 1 : 1 + log10(duplicates);
        // 3 is the number of character in our pattern + room for the trailing zero
        newName = MEMmalloc(3 + STRlen(name) + numberOfDigits);
        sprintf(newName, "_%s_%d", name, duplicates);
        if (!isUniqueInSymbolTable(SYMBOLTABLE_SYMBOLTABLEENTRY(symbolTable), newName, type)) {
            duplicates++;
            newName = MEMfree(newName);
        }
    } while (newName == NULL);
    DBUG_RETURN(newName);
}

node *registerWithinCurrentScope(node* arg_node, info* arg_info, char* name, ste_type entryType, type returnType) {
    DBUG_ENTER("registerWithinCurrentScope");
    DBUG_PRINT("SA", ("Creating a new Symbol Table Entry"));
    // Add the vardef to the ST
    node* varDefSTE = TBmakeSymboltableentry(SYMBOLTABLE_SYMBOLTABLEENTRY(INFO_CURSCOPE(arg_info)));
    SYMBOLTABLEENTRY_ENTRYTYPE(varDefSTE) = entryType;
    SYMBOLTABLEENTRY_NAME(varDefSTE) = STRcpy(name);
    SYMBOLTABLEENTRY_TYPE(varDefSTE) = returnType;
    NODE_LINE(varDefSTE) = NODE_LINE(arg_node);
    NODE_COL(varDefSTE) = NODE_COL(arg_node);

    if (SYMBOLTABLE_SYMBOLTABLEENTRY(INFO_CURSCOPE(arg_info))) {
        SYMBOLTABLEENTRY_OFFSET(varDefSTE) = SYMBOLTABLEENTRY_OFFSET(SYMBOLTABLE_SYMBOLTABLEENTRY(INFO_CURSCOPE(arg_info))) + 1;
    }
    SYMBOLTABLE_SYMBOLTABLEENTRY(INFO_CURSCOPE(arg_info)) = varDefSTE;

    DBUG_RETURN(varDefSTE);
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

node *SAsymboltable(node *arg_node, info *arg_info) {
	DBUG_ENTER("SASymbolTable");
		
	DBUG_RETURN(arg_node);
}

node *SAdeclarations(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAdeclarations");

    // Just register the name of the function or variable
    if (NODE_TYPE(DECLARATIONS_DECLARATION(arg_node)) == N_fundef) {
        node *funDef = DECLARATIONS_DECLARATION(arg_node);
        node *funHeader = FUNDEF_FUNHEADER(funDef);
        char *name = FUNHEADER_NAME(funHeader);
        DBUG_PRINT("SA", ("Registering function [%s].", name));

        // Make sure it does not exist within the current scope
        node* funDefSTE = findDefWithinScope(arg_info, name, STE_fundef);
        if(funDefSTE) {
            CTIerror("Function [%s] at line %d, column %d has already been declared at line %d, column %d.",
                    name, NODE_LINE(arg_node), NODE_COL(arg_node), NODE_LINE(funDefSTE), NODE_COL(funDefSTE));
        } else {
            funDefSTE = registerWithinCurrentScope(funHeader, arg_info, name, STE_fundef, FUNHEADER_RETURNTYPE(funHeader));
        }
        // Make sure we have a reference at hand to the STE
        FUNHEADER_DECL(funHeader) = funDefSTE;
        SYMBOLTABLEENTRY_DECL(funDefSTE) = funHeader;
        DBUG_PRINT("SA", ("Registering function [%s] at offset [%d].", name, SYMBOLTABLEENTRY_OFFSET(funDefSTE)));
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
		// 	Start new scope
		node* previousScope = INFO_CURSCOPE(arg_info);
		node *currentScope = TBmakeSymboltable(NULL);
		SYMBOLTABLE_PARENT(currentScope) = previousScope;
		FUNDEF_SYMBOLTABLE(arg_node) = currentScope;
		INFO_CURSCOPE(arg_info) = currentScope;
			
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

node *SAfunheader(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAfunheader");

    DBUG_PRINT("SA", ("Registering the parameters."));
    TRAVopt(FUNHEADER_PARAMS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAfunbody(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAfunbody");
		
    DBUG_PRINT("SA", ("Processing the VarDecs"));
    TRAVopt(FUNBODY_VARDECS(arg_node), arg_info);
    DBUG_PRINT("SA", ("Processing the LocalFunDefs."));
    TRAVopt(FUNBODY_LOCALFUNDEFS(arg_node), arg_info);
    DBUG_PRINT("SA", ("Processing the Statements."));
    TRAVopt(FUNBODY_STATEMENTS(arg_node), arg_info);
    DBUG_PRINT("SA", ("Function has been processed."));

    DBUG_RETURN(arg_node);
}

node *SAvardef(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAvardef");

    DBUG_PRINT("SA", ("Registering variable [%s].", VARDEF_NAME(arg_node)));
    // First go right
    TRAVopt(VARDEF_EXPR(arg_node), arg_info);

    // Make sure it does not exist within the current scope
    char *name = VARDEF_NAME(arg_node);
    node* varDefSTE = findDefWithinScope(arg_info, name, STE_vardef);
    if(varDefSTE && SYMBOLTABLEENTRY_DISTANCE(varDefSTE) == 0) {
            CTIerror("Variable [%s] at line %d, column %d has already been declared at line %d, column %d.",
                    name, NODE_LINE(arg_node), NODE_COL(arg_node), NODE_LINE(varDefSTE), NODE_COL(varDefSTE));
	} else {
        varDefSTE = registerWithinCurrentScope(arg_node, arg_info, name, STE_vardef, VARDEF_TYPE(arg_node));
	}
    // Make sure we have a reference at hand to the STE
    VARDEF_DECL(arg_node) = varDefSTE;
    // And register a reference to the declaration node
    SYMBOLTABLEENTRY_DECL(varDefSTE) = arg_node;

    DBUG_PRINT("SA", ("Registering variable [%s] at offset [%d].", VARDEF_NAME(arg_node), SYMBOLTABLEENTRY_OFFSET(varDefSTE)));

    DBUG_RETURN(arg_node);
}

node *SAid(node * arg_node, info * arg_info) {
    DBUG_ENTER("SAid");

    // Used for traversing to outer ST/scopes
    int distance = 0;
    node* varDefSTE = findInAnyScope(arg_info, ID_NAME(arg_node), &distance, STE_vardef);

    if(varDefSTE == NULL) {
        CTIerror("Variable [%s] which is used at line %d, column %d is not declared.", ID_NAME(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    } else {
        if(distance > 0) {
            DBUG_PRINT("SA", ("Defined in outer scope, creating a local STE."));
            // Defined in a outer scope, create new STE in current scope
            node* localSTE = registerWithinCurrentScope(arg_node, arg_info, ID_NAME(arg_node), STE_vardef, SYMBOLTABLEENTRY_TYPE(varDefSTE));
            // And link to the original declaration
            SYMBOLTABLEENTRY_DECL(localSTE) = SYMBOLTABLEENTRY_DECL(varDefSTE);
            // Set the correct distance and offset
            SYMBOLTABLEENTRY_OFFSET(localSTE) = SYMBOLTABLEENTRY_OFFSET(varDefSTE);
            SYMBOLTABLEENTRY_DISTANCE(localSTE) = distance;

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
    node *funDefSTE = findInAnyScope(arg_info, name, &distance, STE_fundef);
    if (funDefSTE) {
        DBUG_PRINT("SA", ("It is a known function"));
        if (distance > 0) {
            DBUG_PRINT("SA", ("Function defined in outer scope registering at local ST."));
            // Defined in a outer scope, create new STE in current scope
            node* localSTE = registerWithinCurrentScope(arg_node, arg_info, name, STE_fundef, SYMBOLTABLEENTRY_TYPE(funDefSTE));
            SYMBOLTABLEENTRY_DECL(localSTE) = SYMBOLTABLEENTRY_DECL(funDefSTE);
            // Set the correct distance and offset
            SYMBOLTABLEENTRY_OFFSET(localSTE) = SYMBOLTABLEENTRY_OFFSET(funDefSTE);
            SYMBOLTABLEENTRY_DISTANCE(localSTE) = distance;

            funDefSTE = localSTE;
        }
        FUNCALL_DECL(arg_node) = funDefSTE;

        TRAVopt(FUNCALL_PARAMS(arg_node), arg_info);

        DBUG_PRINT("SA", ("Performing param-count check..."));
        int exprCount = 0;
        node *exprs = FUNCALL_PARAMS(arg_node);
        while (exprs) {
            exprCount++;
            exprs = EXPRS_NEXT(exprs);
        }
        DBUG_PRINT("SA", ("Parameters counted."));
        node *funHeader = SYMBOLTABLEENTRY_DECL(funDefSTE);
        int paramCount = 0;
        node *params = FUNHEADER_PARAMS(funHeader);
        while (params) {
            paramCount++;
            params = PARAMS_NEXT(params);
        }
        DBUG_PRINT("SA", ("The function as [%d] params and there are [%d] expressions.", paramCount, exprCount));
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

node *SAtypecast(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAtypecast");

    TRAVdo(TYPECAST_EXPR(arg_node), arg_info);

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

node *SAif(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAif");

    TRAVdo(IF_CONDITION(arg_node), arg_info);

    TRAVdo(IF_IFBLOCK(arg_node), arg_info);
    TRAVopt(IF_ELSEBLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAwhile(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAwhile");

    TRAVdo(WHILE_CONDITION(arg_node), arg_info);
    TRAVopt(WHILE_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAdo(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAdo");

    TRAVopt(DO_BLOCK(arg_node), arg_info);
    TRAVdo(DO_CONDITION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAfor");

    DBUG_PRINT("SA", ("Processing the start, stop and step expressions."));
    TRAVdo(VARDEF_EXPR(FOR_VARDEF(arg_node)), arg_info);
    TRAVdo(FOR_FINISH(arg_node), arg_info);
    TRAVopt(FOR_STEP(arg_node), arg_info);

    // Only go through the trouble if it is really useful
    if (FOR_BLOCK(arg_node)) {
        DBUG_PRINT("SA", ("Looking for existing name."));
        char *name = VARDEF_NAME(FOR_VARDEF(arg_node));
        node *existingVarDef = findDefWithinScope(arg_info, name, STE_vardef);

        char *originalName = NULL;
        if (existingVarDef) {
            DBUG_PRINT("SA", ("Hiding the existing name in the symboltable for now."));
            // Remember the name and remove it (by giving it a different name) from the ST
            originalName = SYMBOLTABLEENTRY_NAME(existingVarDef);
            SYMBOLTABLEENTRY_NAME(existingVarDef) = "";
        }

        // Register the variable, now all occurrences of our vardef name will get a STE entry to us
        node *forVarEntry = registerWithinCurrentScope(FOR_VARDEF(arg_node), arg_info, name, STE_vardef, TY_int);
        VARDEF_DECL(FOR_VARDEF(arg_node)) = forVarEntry;
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

node *SAreturn(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAreturn");

    TRAVopt(RETURN_EXPR(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAexprs(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAexprs");

    TRAVopt(EXPRS_NEXT(arg_node), arg_info);
    TRAVdo(EXPRS_EXPR(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAbinop(node *arg_node, info *arg_info) {
   DBUG_ENTER("SAbinop");

   TRAVdo(BINOP_LEFT(arg_node), arg_info);
   TRAVdo(BINOP_RIGHT(arg_node), arg_info);

   DBUG_RETURN(arg_node);
}

node *SAunop(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAunop");

    TRAVdo(UNOP_RIGHT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAintconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAintconst");

    DBUG_RETURN(arg_node);
}

node *SAfloatconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAfloatconst");

    DBUG_RETURN(arg_node);
}

node *SAboolconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAboolconst");

    DBUG_RETURN(arg_node);
}

node *SAsymboltableentry(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAsymboltableentry");
	
    DBUG_RETURN(arg_node);
}

node *SAerror(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAerror");

    DBUG_RETURN(arg_node);
}

node *SAlocalfundefs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAlocalfundefs");

    DBUG_RETURN(arg_node);
}

node *SAarrayassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAarrayassign");

    DBUG_RETURN(arg_node);
}

node *SAarray(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAarray");

    DBUG_RETURN(arg_node);
}

node *SAids(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAids");

    DBUG_RETURN(arg_node);
}

node *SAarrexprs(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAarrexprs");

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

