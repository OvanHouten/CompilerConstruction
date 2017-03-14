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

void registerNewVarDecl(node* arg_node, info* arg_info, char* name) {
    DBUG_PRINT("SA", ("Registering variable [%s]", name));
    if (registerNewDecl(arg_node, "Variable", INFO_CURRENTSCOPE(arg_info)->varDecls, name)) {
        // Only adjust the offset when the registration was successful
        INFO_CURRENTSCOPE(arg_info)->varCount++;
        VARDEF_OFFSET(arg_node) = INFO_CURRENTSCOPE(arg_info)->varCount;
        DBUG_PRINT("SA", ("Registered variable [%s] at offset [%d].", name, VARDEF_OFFSET(arg_node)));
    }
}

node *findVarDecl(info *arg_info, char *name, int *distance) {
    DBUG_PRINT("SA", ("Looking for variable [%s]", name));
    struct SymbolTable *currentScope = INFO_CURRENTSCOPE(arg_info);
    while (currentScope) {
        lut_t* varDecls = currentScope->varDecls;
        node* declaringNode = DEREF_IF_NOT_NULL(LUTsearchInLutS(varDecls, name));
        if (declaringNode) {
            DBUG_PRINT("SA", ("Found [%s] at distace [%d] and offset [%d]", name, *distance, VARDEF_OFFSET(declaringNode)));
            return declaringNode;
        } else {
            (*distance)++;
            currentScope = currentScope->parent;
        }
    }
    DBUG_PRINT("SA", ("[%s] not found", name));
    return NULL;
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

// =============================================
// Traversal code starts here
// =============================================

node *SAprogram(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAprogram");

    startNewScope(arg_info);

    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);

    closeScope(arg_info);

    DBUG_RETURN(arg_node);
}

node *SAdeclarations(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAdeclarations");

    // Just register the name of the function or variable
    if (NODE_TYPE(DECLARATIONS_DECLARATION(arg_node)) == N_fundef) {
        node *funDef = DECLARATIONS_DECLARATION(arg_node);
        registerNewFunDecl(FUNDEF_FUNHEADER(funDef), arg_info, ID_NAME(FUNHEADER_ID(FUNDEF_FUNHEADER(funDef))));
        // TODO check if this is realy needed and useful.
        node *funHeader = FUNDEF_FUNHEADER(funDef);
        node *id = FUNHEADER_ID(FUNDEF_FUNHEADER(funDef));
        ID_DECL(id) = funHeader;
        ID_DISTANCE(id) = 0; // Just to make it explicit that each function is defined in the current scope and hence has an offset of 0.
        ID_OFFSET(id) = FUNHEADER_OFFSET(funHeader);
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

        TRAVopt(FUNDEF_FUNHEADER(arg_node), arg_info);
        TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

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
    if (FUNBODY_LOCALFUNDEFS(arg_node)) {
        TRAVopt(FUNBODY_LOCALFUNDEFS(arg_node), arg_info);
    }
    TRAVopt(FUNBODY_STATEMENTS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAvardef(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAvardef");

    // First go right
    TRAVopt(VARDEF_EXPR(arg_node), arg_info);

    // And now we van register the variable name
    registerNewVarDecl(arg_node, arg_info, ID_NAME(VARDEF_ID(arg_node)));
    // TODO check if this is really needed and useful.
    node *id = VARDEF_ID(arg_node);
    ID_DECL(id) = arg_node;
    ID_DISTANCE(id) = 0;
    ID_OFFSET(id) = VARDEF_OFFSET(arg_node);


    DBUG_RETURN(arg_node);
}

node *SAid(node * arg_node, info * arg_info) {
    DBUG_ENTER("SAid");

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

node *SAfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAfuncall");

    char *name = ID_NAME(FUNCALL_ID(arg_node));
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

node *SAparam(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAparam");

    registerNewVarDecl(arg_node, arg_info, ID_NAME(PARAM_ID(arg_node)));

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

    startNewScope(arg_info);
    TRAVdo(IF_IFBLOCK(arg_node), arg_info);
    closeScope(arg_info);

    startNewScope(arg_info);
    TRAVopt(IF_ELSEBLOCK(arg_node), arg_info);
    closeScope(arg_info);

    DBUG_RETURN(arg_node);
}

node *SAwhile(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAwhile");

    TRAVdo(WHILE_CONDITION(arg_node), arg_info);
    startNewScope(arg_info);
    TRAVopt(WHILE_BLOCK(arg_node), arg_info);
    closeScope(arg_info);

    DBUG_RETURN(arg_node);
}

node *SAdo(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAdo");

    startNewScope(arg_info);
    TRAVopt(DO_BLOCK(arg_node), arg_info);
    closeScope(arg_info);
    TRAVdo(DO_CONDITION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *SAfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAfor");

    startNewScope(arg_info);
    registerNewVarDecl(FOR_VARDEF(arg_node), arg_info, ID_NAME(VARDEF_ID(FOR_VARDEF(arg_node))));
    TRAVdo(VARDEF_EXPR(FOR_VARDEF(arg_node)), arg_info);
    TRAVdo(FOR_FINISH(arg_node), arg_info);
    TRAVopt(FOR_STEP(arg_node), arg_info);
    TRAVopt(FOR_BLOCK(arg_node), arg_info);
    closeScope(arg_info);

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

node *SAlocalfundefs(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAlocalfundefs");

    // Just register the function name
    node * localFunDef = LOCALFUNDEFS_LOCALFUNDEF(arg_node);
    registerNewFunDecl(LOCALFUNDEF_FUNHEADER(localFunDef), arg_info, ID_NAME(FUNHEADER_ID(LOCALFUNDEF_FUNHEADER(localFunDef))));

    // Register other functions
    TRAVopt(LOCALFUNDEFS_NEXT(arg_node), arg_info);

    // Process the function header and the body
    TRAVdo(localFunDef, arg_info);

    DBUG_RETURN(arg_node);
}

node *SAlocalfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("SAlocalfundef");

    startNewScope(arg_info);

    TRAVopt(FUNHEADER_PARAMS(LOCALFUNDEF_FUNHEADER(arg_node)), arg_info);
    TRAVopt(LOCALFUNDEF_FUNBODY(arg_node), arg_info);

    closeScope(arg_info);

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

