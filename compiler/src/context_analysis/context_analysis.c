/*
 * context_analysis.c
 *
 *  Created on: 3 Mar 2017
 *      Author: nico
 */

#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "context_analysis.h"
#include "lookup_table.h"
#include "ctinfo.h"

typedef enum { RegisterOnly, ProcessOnly, RegisterAndProcess } phase_phase;

struct SymbolTable {
    struct SymbolTable *parent;
    lut_t *varDecls;
    lut_t *funDecls;
    int funCount; // Needed for determining the offset within the ST
    int varCount; // Needed for determining the offset within the ST
};

/*
 * INFO structure
 */
struct INFO {
  struct SymbolTable *currentScope;
  phase_phase processPhase;
};

/*
 * INFO macros
 */
#define INFO_CURRENTSCOPE(n)  ((n)->currentScope)

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
  result->processPhase = RegisterAndProcess;

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
    st->varDecls = LUTgenerateLut();
    st->funDecls = LUTgenerateLut();
    st->funCount = 0;
    st->varCount = 0;
    return st;
}

struct SymbolTable *freeSymbolTable(struct SymbolTable *symbolTable) {
    symbolTable->parent = NULL;
    LUTremoveContentLut(symbolTable->varDecls);
    LUTremoveContentLut(symbolTable->funDecls);
    symbolTable->varDecls = LUTremoveLut(symbolTable->varDecls);
    symbolTable->funDecls = LUTremoveLut(symbolTable->funDecls);
    symbolTable = MEMfree(symbolTable);
    return symbolTable;
}

struct SymbolTable* startNewScope(info *arg_info) {
    DBUG_PRINT("CA", ("Starting new scope"));
    struct SymbolTable *newScope = makeNewSymbolTable();
    // Create link to parent
    newScope->parent = INFO_CURRENTSCOPE(arg_info);
    INFO_CURRENTSCOPE(arg_info) = newScope;
    return newScope;
}

void* printVarDecls(void* lut_item) {
//  	node* cur_item = (node*)lut_item;
//  	printf("%s\n", ID_NAME(cur_item));
	
	return lut_item;
}

void* printFunDecls(void* lut_item) {
//  	node* cur_item = (node*)lut_item;
//  	printf("%s\n", ID_NAME(cur_item));
	
	return lut_item;
}

void printScope(info* arg_info) {
	DBUG_PRINT("CA", ("Printing current scope"));
	
	struct SymbolTable *currentScope = INFO_CURRENTSCOPE(arg_info);
	lut_t* varDecls = currentScope->varDecls;
	lut_t* funDecls = currentScope->funDecls;
		
	if (funDecls) {
		funDecls = LUTmapLutS(varDecls, printFunDecls);
		printf("Total functions = %d\n", currentScope->funCount); // Temp check
	}
	if (varDecls) {
		varDecls = LUTmapLutS(varDecls, printVarDecls);
		printf("Total variables = %d\n", currentScope->varCount); // Temp check
	}
}

void closeScope(info *arg_info) {
    DBUG_PRINT("CA", ("Closing scope"));
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
    DBUG_PRINT("CA", ("Registering function [%s]", name));
    if (registerNewDecl(arg_node, "Function", INFO_CURRENTSCOPE(arg_info)->funDecls, name)) {
        // Only adjust the offset when the registration was successful
        FUNHEADER_OFFSET(arg_node) = INFO_CURRENTSCOPE(arg_info)->funCount++;
        DBUG_PRINT("CA", ("Registered function [%s]", name));
    }
}

void registerNewVarDecl(node* arg_node, info* arg_info, char* name) {
    DBUG_PRINT("CA", ("Registering variable [%s]", name));
    if (registerNewDecl(arg_node, "Variable", INFO_CURRENTSCOPE(arg_info)->varDecls, name)) {
        // Only adjust the offset when the registration was successful
        VARDEF_OFFSET(arg_node) = INFO_CURRENTSCOPE(arg_info)->varCount++;
        DBUG_PRINT("CA", ("Registered variable [%s]", name));
    }
}

node *findVarDecl(info *arg_info, char *name, int *distance) {
    DBUG_PRINT("CA", ("Looking for variable [%s]", name));
    struct SymbolTable *currentScope = INFO_CURRENTSCOPE(arg_info);
    while (currentScope) {
        lut_t* varDecls = currentScope->varDecls;
        node* declaringNode = DEREF_IF_NOT_NULL(LUTsearchInLutS(varDecls, name));
        if (declaringNode) {
            DBUG_PRINT("CA", ("Found [%s] it at [%p]", name, declaringNode));
            return declaringNode;
        } else {
            (*distance)++;
            currentScope = currentScope->parent;
        }
    }
    DBUG_PRINT("CA", ("[%s] not found", name));
    return NULL;
}

node *findFunDecl(info *arg_info, char *name, int *distance) {
    DBUG_PRINT("CA", ("Looking for function [%s]", name));
    struct SymbolTable *currentScope = INFO_CURRENTSCOPE(arg_info);
    while (currentScope) {
        lut_t* funDecls = currentScope->funDecls;
        node* declaringNode = DEREF_IF_NOT_NULL(LUTsearchInLutS(funDecls, name));
        if (declaringNode) {
            DBUG_PRINT("CA", ("Found [%s] it at [%p]", name, declaringNode));
            return declaringNode;
        } else {
            (*distance)++;
            currentScope = currentScope->parent;
        }
    }
    DBUG_PRINT("CA", ("[%s] not found", name));
    return NULL;
}

// =============================================
// Traversal code starts here
// =============================================

node *CAprogram(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAprogram");

    startNewScope(arg_info);

    // Only register functions at this stage
    arg_info->processPhase = RegisterOnly;
    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);
    // No do it again and process the function bodies
    arg_info->processPhase = ProcessOnly;
    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);
	printScope(arg_info);
    closeScope(arg_info);

    DBUG_RETURN(arg_node);
}

node *CAdeclarations(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAdeclarations");

    TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);
    TRAVdo(DECLARATIONS_DECLARATION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAfundef");

    if (arg_info->processPhase == RegisterOnly) {
        registerNewFunDecl(FUNDEF_FUNHEADER(arg_node), arg_info, ID_NAME(FUNHEADER_ID(FUNDEF_FUNHEADER(arg_node))));
    } else {
        if (FUNDEF_FUNBODY(arg_node)) {
            arg_info->processPhase = RegisterAndProcess;
            startNewScope(arg_info);

            TRAVopt(FUNDEF_FUNHEADER(arg_node), arg_info);
            TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

            closeScope(arg_info);
            arg_info->processPhase = ProcessOnly;
        }
    }

    DBUG_RETURN(arg_node);
}


node *CAfunheader(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAfunheader");

    TRAVopt(FUNHEADER_PARAMS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAfunbody(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAfunbody");

    arg_info->processPhase = RegisterOnly;
    TRAVopt(FUNBODY_VARDECS(arg_node), arg_info);
    TRAVopt(FUNBODY_LOCALFUNDEFS(arg_node), arg_info);

    arg_info->processPhase = ProcessOnly;
    TRAVopt(FUNBODY_LOCALFUNDEFS(arg_node), arg_info);
    TRAVopt(FUNBODY_STATEMENTS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAvardef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAglobaldef");

    if (arg_info->processPhase == RegisterOnly || arg_info->processPhase == RegisterAndProcess) {
        registerNewVarDecl(arg_node, arg_info, ID_NAME(VARDEF_ID(arg_node)));
    }
    if (arg_info->processPhase == RegisterAndProcess || arg_info->processPhase == ProcessOnly) {
        TRAVopt(VARDEF_EXPR(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node *CAid(node * arg_node, info * arg_info) {
    DBUG_ENTER("CAid");

    char *name = ID_NAME(arg_node);
    int distance = 0;
    node *varDef = findVarDecl(arg_info, name, &distance);
    if (varDef) {
        ID_DECL(arg_node) = varDef;
        ID_DISTANCE(arg_node) = distance;
        ID_OFFSET(arg_node) = VARDEF_OFFSET(varDef);
    } else {
        CTIerror("Variable [%s] which is used at line %d, column %d is not declared.", name, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    DBUG_RETURN(arg_node);
}

node *CAfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAfuncall");

    char *name = ID_NAME(FUNCALL_ID(arg_node));
    int distance = 0;
    node *funDef = findFunDecl(arg_info, name, &distance);
    if (funDef) {
        FUNCALL_DECL(arg_node) = funDef;
        FUNCALL_DISTANCE(arg_node) = distance;
        FUNCALL_OFFSET(arg_node) = FUNHEADER_OFFSET(funDef);

        TRAVopt(FUNCALL_PARAMS(arg_node), arg_info);
    } else {
        CTIerror("Function [%s] at line %d, column %d has not yet been declared.", name, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    DBUG_RETURN(arg_node);
}

node *CAint(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAint");

    DBUG_RETURN(arg_node);
}

node *CAfloat(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAfloat");

    DBUG_RETURN(arg_node);
}

node *CAbool(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAbool");

    DBUG_RETURN(arg_node);
}

node *CAparams(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAparams");

    TRAVopt(PARAMS_NEXT(arg_node), arg_info);
    TRAVdo(PARAMS_PARAM(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAparam(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAparam");

    DBUG_PRINT("CA", ("Param enter"));
    registerNewVarDecl(arg_node, arg_info, ID_NAME(PARAM_ID(arg_node)));
    DBUG_PRINT("CA", ("Param exit"));

    DBUG_RETURN(arg_node);
}

node *CAvardecs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAvardecs");

    TRAVopt(VARDECS_NEXT(arg_node), arg_info);
    TRAVdo(VARDECS_VARDEC(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAtypecast(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAtypecast");

    TRAVdo(TYPECAST_EXPR(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAassign");

    TRAVopt(ASSIGN_EXPR(arg_node), arg_info);
    TRAVdo(ASSIGN_LET(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAif(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAif");

    TRAVdo(IF_CONDITION(arg_node), arg_info);

    startNewScope(arg_info);
    TRAVdo(IF_IFBLOCK(arg_node), arg_info);
    closeScope(arg_info);

    startNewScope(arg_info);
    TRAVopt(IF_ELSEBLOCK(arg_node), arg_info);
    closeScope(arg_info);

    DBUG_RETURN(arg_node);
}

node *CAwhile(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAwhile");

    startNewScope(arg_info);
    TRAVdo(WHILE_CONDITION(arg_node), arg_info);
    TRAVopt(WHILE_BLOCK(arg_node), arg_info);
    closeScope(arg_info);

    DBUG_RETURN(arg_node);
}

node *CAdo(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAdo");

    startNewScope(arg_info);
    TRAVopt(DO_BLOCK(arg_node), arg_info);
    closeScope(arg_info);
    TRAVdo(DO_CONDITION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAfor");

    startNewScope(arg_info);
    registerNewVarDecl(FOR_VARDEF(arg_node), arg_info, ID_NAME(VARDEF_ID(FOR_VARDEF(arg_node))));
    TRAVdo(VARDEF_EXPR(FOR_VARDEF(arg_node)), arg_info);
    TRAVdo(FOR_FINISH(arg_node), arg_info);
    TRAVopt(FOR_STEP(arg_node), arg_info);
    TRAVopt(FOR_BLOCK(arg_node), arg_info);
    closeScope(arg_info);

    DBUG_RETURN(arg_node);
}

node *CAreturn(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAreturn");

    TRAVopt(RETURN_EXPR(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAexprs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAexprs");

    TRAVopt(EXPRS_NEXT(arg_node), arg_info);
    TRAVdo(EXPRS_EXPR(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAarithop(node *arg_node, info *arg_info) {
   DBUG_ENTER("CAarithop");

   TRAVdo(ARITHOP_LEFT(arg_node), arg_info);
   TRAVdo(ARITHOP_RIGHT(arg_node), arg_info);

   DBUG_RETURN(arg_node);
}

node *CArelop(node *arg_node, info *arg_info) {
    DBUG_ENTER("CArelop");

    TRAVdo(RELOP_LEFT(arg_node), arg_info);
    TRAVdo(RELOP_RIGHT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAlogicop(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAlogicop");

    TRAVdo(LOGICOP_LEFT(arg_node), arg_info);
    TRAVdo(LOGICOP_RIGHT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAunop(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAunop");

    TRAVdo(UNOP_RIGHT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAvoid(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAvoid");

    DBUG_RETURN(arg_node);
}

node *CAintconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAintconst");

    DBUG_RETURN(arg_node);
}

node *CAfloatconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAfloatconst");

    DBUG_RETURN(arg_node);
}

node *CAboolconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAboolconst");

    DBUG_RETURN(arg_node);
}

node *CAsymboltableentry(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAsymboltableentry");

    DBUG_RETURN(arg_node);
}

node *CAerror(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAerror");

    DBUG_RETURN(arg_node);
}

node *CAlocalfundefs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAlocalfundefs");

    TRAVopt(LOCALFUNDEFS_NEXT(arg_node), arg_info);
    TRAVdo(LOCALFUNDEFS_LOCALFUNDEF(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAlocalfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAlocalfundef");

    if (arg_info->processPhase == RegisterOnly) {
        registerNewFunDecl(LOCALFUNDEF_FUNHEADER(arg_node), arg_info, ID_NAME(FUNHEADER_ID(LOCALFUNDEF_FUNHEADER(arg_node))));
    } else {
        arg_info->processPhase = RegisterAndProcess;
        startNewScope(arg_info);

        TRAVopt(FUNHEADER_PARAMS(LOCALFUNDEF_FUNHEADER(arg_node)), arg_info);
        TRAVopt(LOCALFUNDEF_FUNBODY(arg_node), arg_info);

        closeScope(arg_info);
        arg_info->processPhase = ProcessOnly;
    }

    DBUG_RETURN(arg_node);
}

node *CAarrayassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAarrayassign");

    DBUG_RETURN(arg_node);
}

node *CAarray(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAarray");

    DBUG_RETURN(arg_node);
}

node *CAids(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAids");

    DBUG_RETURN(arg_node);
}

node *CAarrexprs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAarrexprs");

    DBUG_RETURN(arg_node);
}

node *CAstatements(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAstatements");

    TRAVopt(STATEMENTS_NEXT(arg_node), arg_info);
    TRAVdo(STATEMENTS_STATEMENT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAdoScopeAnalysis( node *syntaxtree) {
    DBUG_ENTER("CAdoScopeAnslysis");

    info *arg_info = MakeInfo();

    TRAVpush(TR_ca);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}

