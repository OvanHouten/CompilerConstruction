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
#include "context_analysis.h"
#include "lookup_table.h"
#include "ctinfo.h"

struct SymbolTable {
    struct SymbolTable *parent;
    lut_t *funDecls;
    int funCount; // Needed for determining the offset within the ST
};

/*
 * INFO structure
 */
struct INFO {
  struct SymbolTable *currentScope;
  node* curScope;
  node* prevScope;
};

/*
 * INFO macros
 */
#define INFO_CURRENTSCOPE(n)  ((n)->currentScope)
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
  result->currentScope = NULL;
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

struct SymbolTable *makeNewSymbolTable() {
    struct SymbolTable *st = MEMmalloc(sizeof(struct SymbolTable));
    st->parent = NULL;
    st->funDecls = LUTgenerateLut();
    st->funCount = 0;
    return st;
}

struct SymbolTable *freeSymbolTable(struct SymbolTable *symbolTable) {
    symbolTable->parent = NULL;
    LUTremoveContentLut(symbolTable->funDecls);
    symbolTable->funDecls = LUTremoveLut(symbolTable->funDecls);
    symbolTable = MEMfree(symbolTable);
    return symbolTable;
}

struct SymbolTable* startNewScope(info *arg_info) {
    DBUG_PRINT("SA", ("Starting new scope"));
    struct SymbolTable *newScope = makeNewSymbolTable();
    // Create link to parent
    newScope->parent = INFO_CURRENTSCOPE(arg_info);
    INFO_CURRENTSCOPE(arg_info) = newScope;
    return newScope;
}

void closeScope(info *arg_info) {
    DBUG_PRINT("SA", ("Closing scope"));
    struct SymbolTable *scopeToBeFreed = INFO_CURRENTSCOPE(arg_info);
    INFO_CURRENTSCOPE(arg_info) = INFO_CURRENTSCOPE(arg_info)->parent;
    scopeToBeFreed = freeSymbolTable(scopeToBeFreed);
}

bool registerNewDecl(node *arg_node, char *typeName, lut_t* decls, char *name) {
    node *declaringNode = DEREF_IF_NOT_NULL(LUTsearchInLutS(decls, name));
    if (declaringNode) {
        CTIerror(
                "%s [%s] at line %d, column %d has already been declared at line %d, column %d.", typeName,
                name, NODE_LINE(arg_node), NODE_COL(arg_node), NODE_LINE(declaringNode), NODE_COL(declaringNode));
        return FALSE;
    } else {
        LUTinsertIntoLutS(decls, name, arg_node);
        return TRUE;
    }
}

void registerNewFunDecl(node* arg_node, info* arg_info, char* name) {
    DBUG_PRINT("SA", ("Registering function [%s]", name));
    if (registerNewDecl(arg_node, "Function", INFO_CURRENTSCOPE(arg_info)->funDecls, name)) {
        // Only adjust the offset when the registration was successful
        FUNHEADER_OFFSET(arg_node) = INFO_CURRENTSCOPE(arg_info)->funCount++;
        DBUG_PRINT("SA", ("Registered function [%s] at offset [%d].", name, FUNHEADER_OFFSET(arg_node)));
    }
}

node *findFunDecl(info *arg_info, char *name, int *distance) {
    DBUG_PRINT("SA", ("Looking for function [%s]", name));
    struct SymbolTable *currentScope = INFO_CURRENTSCOPE(arg_info);
    while (currentScope) {
        lut_t* funDecls = currentScope->funDecls;
        node* declaringNode = DEREF_IF_NOT_NULL(LUTsearchInLutS(funDecls, name));
        if (declaringNode) {
            DBUG_PRINT("SA", ("Found [%s] at distace [%d] and offset [%d]", name, *distance, FUNHEADER_OFFSET(declaringNode)));
            return declaringNode;
        } else {
            (*distance)++;
            currentScope = currentScope->parent;
        }
    }
    DBUG_PRINT("SA", ("[%s] not found", name));
    return NULL;
}

node* findVarDefWithinScope(info* arg_info, char* name) {
    DBUG_ENTER("findVarDefWithinScope");
    node* varDefSTE = SYMBOLTABLE_SYMBOLTABLEENTRY(INFO_CURSCOPE(arg_info));
    while (varDefSTE) {
        if (STReq(name, SYMBOLTABLEENTRY_NAME(varDefSTE))) {
            break;
        }
        varDefSTE = SYMBOLTABLEENTRY_NEXT(varDefSTE);
    }
    DBUG_RETURN(varDefSTE);
}

node *registerVarDefWithinScope(node* arg_node, info* arg_info, char* name) {
    DBUG_ENTER("registerVarDefWithinScope");
    // Add the vardef to the ST
    node* varDefSTE = TBmakeSymboltableentry(SYMBOLTABLE_SYMBOLTABLEENTRY(INFO_CURSCOPE(arg_info)));
    SYMBOLTABLEENTRY_NAME(varDefSTE) = STRcpy(name);
    SYMBOLTABLEENTRY_TYPE(varDefSTE) = TY_unknown;
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

    startNewScope(arg_info);
	// Start new scope, change curscope, prevscope stays NULL;
	INFO_CURSCOPE(arg_info) = PROGRAM_SYMBOLTABLE(arg_node);
	
    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);
    
    closeScope(arg_info);
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
        registerNewFunDecl(FUNDEF_FUNHEADER(funDef), arg_info, FUNHEADER_NAME(FUNDEF_FUNHEADER(funDef)));
    }

    // Continue to register function and variable names
    TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);

    // Now process the body of the function or the expression of the variable
    TRAVdo(DECLARATIONS_DECLARATION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAfundef");
	
    if (FUNDEF_FUNBODY(arg_node)) {
        startNewScope(arg_info);
			
		// 	Start new scope, change curscope and prevscope;
		node* previousScope = INFO_CURSCOPE(arg_info);
		FUNDEF_SYMBOLTABLE(arg_node) = TBmakeSymboltable(NULL);
		SYMBOLTABLE_PARENT(FUNDEF_SYMBOLTABLE(arg_node)) = INFO_CURSCOPE(arg_info);
		INFO_CURSCOPE(arg_info) = FUNDEF_SYMBOLTABLE(arg_node);
			
        TRAVopt(FUNDEF_FUNHEADER(arg_node), arg_info);
        TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);
		
		FUNDEF_SYMBOLTABLE(arg_node) = INFO_CURSCOPE(arg_info);
		INFO_CURSCOPE(arg_info) = previousScope;
        closeScope(arg_info);
    }
	
    DBUG_RETURN(arg_node);
}

node *SAfunheader(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAfunheader");

    TRAVopt(FUNHEADER_PARAMS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAfunbody(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAfunbody");
		
    TRAVopt(FUNBODY_VARDECS(arg_node), arg_info);
    TRAVopt(FUNBODY_LOCALFUNDEFS(arg_node), arg_info);
    TRAVopt(FUNBODY_STATEMENTS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAvardef(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAvardef");

    // First go right
    TRAVopt(VARDEF_EXPR(arg_node), arg_info);

    // Make it does not exist within the current scope
    char *name = VARDEF_NAME(arg_node);
    node* varDefSTE = findVarDefWithinScope(arg_info, name);
    if(varDefSTE) {
        CTIerror("Variable [%s] at line %d, column %d has already been declared at line %d, column %d.",
                name, NODE_LINE(arg_node), NODE_COL(arg_node), NODE_LINE(varDefSTE), NODE_COL(varDefSTE));
	} else {
	    varDefSTE = registerVarDefWithinScope(arg_node, arg_info, name);
	}
    // Make sure we have a reference at hand to the STE
    VARDEF_DECL(arg_node) = varDefSTE;
	
    DBUG_RETURN(arg_node);
}

node* findVarDefInAnyScope(char *name, info* arg_info, int* distance) {
    // Used for traversing to outer ST/scopes
    node* lookupST = INFO_CURSCOPE(arg_info);
    node* varDefSTE = SYMBOLTABLE_SYMBOLTABLEENTRY(lookupST);
    while (varDefSTE) {
        if (STReq(name, SYMBOLTABLEENTRY_NAME(varDefSTE))) {
            break;
        }
        // Try next entry
        if (SYMBOLTABLEENTRY_NEXT(varDefSTE)) {
            varDefSTE = SYMBOLTABLEENTRY_NEXT(varDefSTE);
        } else {
            // Try next ST
            lookupST = SYMBOLTABLE_PARENT(lookupST);
            varDefSTE = SYMBOLTABLE_SYMBOLTABLEENTRY(lookupST);
            (*distance)++;
        }
    }
    return varDefSTE;
}

node *SAid(node * arg_node, info * arg_info) {
    DBUG_ENTER("SAid");

    int distance = 0;
    // Used for traversing to outer ST/scopes
    node* varDefSTE = findVarDefInAnyScope(ID_NAME(arg_node), arg_info, &distance);

    if(varDefSTE == NULL) {
        CTIerror("Variable [%s] which is used at line %d, column %d is not declared.", ID_NAME(arg_node), NODE_LINE(arg_node), NODE_COL(arg_node));
    } else {
        if(distance > 0) {
            // Defined in a outer scope, create new STE in current scope
            node* localSTE = registerVarDefWithinScope(arg_node, arg_info, ID_NAME(arg_node));
            // Set the correct distance and offset
            SYMBOLTABLEENTRY_OFFSET(localSTE) = SYMBOLTABLEENTRY_OFFSET(varDefSTE);
            SYMBOLTABLEENTRY_DISTANCE(localSTE) = distance;

            varDefSTE = localSTE;
        }
        // Make sure we can referene the out STE
        ID_DECL(arg_node) = varDefSTE;
    }

    DBUG_RETURN(arg_node);
}

node *SAfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAfuncall");

    char *name = FUNCALL_NAME(arg_node);
    int distance = 0;
    node *funDef = findFunDecl(arg_info, name, &distance);
    if (funDef) {
        FUNCALL_DECL(arg_node) = funDef;
        FUNCALL_DISTANCE(arg_node) = distance;
        FUNCALL_OFFSET(arg_node) = FUNHEADER_OFFSET(funDef);

        TRAVopt(FUNCALL_PARAMS(arg_node), arg_info);

        DBUG_PRINT("SA", ("Performing param-count check..."));
        int exprCount = 0;
        node *exprs = FUNCALL_PARAMS(arg_node);
        while (exprs) {
            exprCount++;
            exprs = EXPRS_NEXT(exprs);
        }
        int paramCount = 0;
        node *params = FUNHEADER_PARAMS(funDef);
        while (params) {
            paramCount++;
            params = PARAMS_NEXT(params);
        }
        DBUG_PRINT("SA", ("The function as [%d] params and there are [%d] expressions.", paramCount, exprCount));
        if (paramCount != exprCount) {
            CTIerror("The number of parameters [%d] as used at line [%d] and column [%d] do not match the number of parameters [%d] to the function %s as defined at line [%d].", exprCount, NODE_LINE(arg_node), NODE_COL(arg_node), paramCount, name, NODE_LINE(funDef));
        }
    } else {
        CTIerror("Function [%s] at line %d, column %d has not yet been declared.", name, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    DBUG_RETURN(arg_node);
}

node *SAint(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAint");

    DBUG_RETURN(arg_node);
}

node *SAfloat(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAfloat");

    DBUG_RETURN(arg_node);
}

node *SAbool(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAbool");

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

    TRAVopt(ASSIGN_EXPR(arg_node), arg_info);
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

    TRAVdo(VARDEF_EXPR(FOR_VARDEF(arg_node)), arg_info);
    TRAVdo(FOR_FINISH(arg_node), arg_info);
    TRAVopt(FOR_STEP(arg_node), arg_info);
    // Register vardef
    TRAVopt(FOR_BLOCK(arg_node), arg_info);

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

node *SAarithop(node *arg_node, info *arg_info) {
   DBUG_ENTER("SAarithop");

   TRAVdo(ARITHOP_LEFT(arg_node), arg_info);
   TRAVdo(ARITHOP_RIGHT(arg_node), arg_info);

   DBUG_RETURN(arg_node);
}

node *SArelop(node *arg_node, info *arg_info) {
    DBUG_ENTER("SArelop");

    TRAVdo(RELOP_LEFT(arg_node), arg_info);
    TRAVdo(RELOP_RIGHT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAlogicop(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAlogicop");

    TRAVdo(LOGICOP_LEFT(arg_node), arg_info);
    TRAVdo(LOGICOP_RIGHT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAunop(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAunop");

    TRAVdo(UNOP_RIGHT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAvoid(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAvoid");

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

node *SAlocalfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAlocalfundef");

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

