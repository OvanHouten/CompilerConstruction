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
    lut_t *vardecls;
    lut_t *fundecls;
};

struct SymbolTableEntry {
    node *decl; // Reference back to the declaring node
};

/*
 * INFO structure
 */
struct INFO {
  struct SymbolTable *currentScope;
  bool registerOnly;
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
  result->registerOnly = FALSE;

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
    st->vardecls = LUTgenerateLut();
    st->fundecls = LUTgenerateLut();
    return st;
}

struct SymbolTable* startNewScope(info *arg_info) {
    DBUG_PRINT("CA", ("Starting new scope"));
    struct SymbolTable *newScope = makeNewSymbolTable();
    newScope->parent = INFO_CURRENTSCOPE(arg_info);
    INFO_CURRENTSCOPE(arg_info) = newScope;
    return newScope;
}

void closeScope(info *arg_info) {
    DBUG_PRINT("CA", ("Closing scope"));
    INFO_CURRENTSCOPE(arg_info) = INFO_CURRENTSCOPE(arg_info)->parent;
}

struct SymbolTableEntry *registerNewDecl(node *arg_node, char *typeName, lut_t* decls, char *name) {
    struct SymbolTableEntry *symbolTableEntry = DEREF_IF_NOT_NULL(LUTsearchInLutS(decls, name));
    if (symbolTableEntry) {
        CTIerror(
                "%s [%s] at line %d, column %d has already been declared at line %d, column %d.", typeName,
                name, arg_node->lineno, arg_node->colno, symbolTableEntry->decl->lineno, symbolTableEntry->decl->colno);
    } else {
        symbolTableEntry = MEMmalloc(sizeof(struct SymbolTableEntry));
        symbolTableEntry->decl = arg_node;
        LUTinsertIntoLutS(decls, name, symbolTableEntry);
    }
    DBUG_PRINT("CA", ("Registered at [%p]", symbolTableEntry));
    return symbolTableEntry;
}

struct SymbolTableEntry * registerNewFunDecl(node* arg_node, info* arg_info, char* name) {
    DBUG_PRINT("CA", ("Registering function [%s]", name));
    return registerNewDecl(arg_node, "Function", INFO_CURRENTSCOPE(arg_info)->fundecls, name);
}

struct SymbolTableEntry * registerNewVarDecl(node* arg_node, info* arg_info, char* name) {
    DBUG_PRINT("CA", ("Registering variable [%s]", name));
    return registerNewDecl(arg_node, "Variable", INFO_CURRENTSCOPE(arg_info)->vardecls, name);
}

struct SymbolTableEntry *findVarDecl(info *arg_info, char *name) {
    DBUG_PRINT("CA", ("Looking for variable [%s]", name));
    struct SymbolTable *currentScope = INFO_CURRENTSCOPE(arg_info);
    while (currentScope) {
        lut_t* varDecls = currentScope->vardecls;
        struct SymbolTableEntry* symbolTableEntry = DEREF_IF_NOT_NULL(LUTsearchInLutS(varDecls, name));
        if (symbolTableEntry) {
            DBUG_PRINT("CA", ("Found [%s] it at [%p]", name, symbolTableEntry));
            return symbolTableEntry;
        } else {
            currentScope = currentScope->parent;
        }
    }
    DBUG_PRINT("CA", ("[%s] not found", name));
    return NULL;
}

struct SymbolTableEntry *findFunDecl(info *arg_info, char *name) {
    DBUG_PRINT("CA", ("Looking for function [%s]", name));
    struct SymbolTable *currentScope = INFO_CURRENTSCOPE(arg_info);
    while (currentScope) {
        lut_t* funDecls = currentScope->fundecls;
        struct SymbolTableEntry* symbolTableEntry = DEREF_IF_NOT_NULL(LUTsearchInLutS(funDecls, name));
        if (symbolTableEntry) {
            DBUG_PRINT("CA", ("Found [%s] it at [%p]", name, symbolTableEntry));
            return symbolTableEntry;
        } else {
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

    INFO_CURRENTSCOPE(arg_info) = makeNewSymbolTable();

    // Only register functions at this stage
    arg_info->registerOnly = TRUE;
    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);
    // No do it again and process the function bodies
    arg_info->registerOnly = FALSE;
    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);

    INFO_CURRENTSCOPE(arg_info) = INFO_CURRENTSCOPE(arg_info)->parent;

    DBUG_RETURN(arg_node);
}

node *CAdeclarations(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAdeclarations");

    TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);
    TRAVdo(DECLARATIONS_DECLARATION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAfundec(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAfundec");

    if (arg_info->registerOnly) {
        registerNewFunDecl(arg_node, arg_info, ID_NAME(FUNHEADER_ID(FUNDEC_FUNHEADER(arg_node))));
    }

    DBUG_RETURN(arg_node);
}

node *CAfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAfundef");

    if (arg_info->registerOnly) {
        registerNewFunDecl(arg_node, arg_info, ID_NAME(FUNHEADER_ID(FUNDEF_FUNHEADER(arg_node))));
    } else {
        startNewScope(arg_info);

        TRAVopt(FUNDEF_FUNHEADER(arg_node), arg_info);
        TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

        closeScope(arg_info);
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

    TRAVopt(FUNBODY_VARDECS(arg_node), arg_info);
    TRAVopt(FUNBODY_LOCALFUNDEFS(arg_node), arg_info);
    TRAVopt(FUNBODY_STATEMENTS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAglobaldec(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAglobaldec");

    if (arg_info->registerOnly) {
        registerNewVarDecl(arg_node, arg_info, ID_NAME(GLOBALDEC_ID(arg_node)));
    }

    DBUG_RETURN(arg_node);
}

node *CAglobaldef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAglobaldef");

    if (arg_info->registerOnly) {
        registerNewVarDecl(arg_node, arg_info, ID_NAME(GLOBALDEF_ID(arg_node)));
    } else {
        TRAVopt(GLOBALDEF_EXPR(arg_node), arg_info);
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

    registerNewVarDecl(arg_node, arg_info, ID_NAME(PARAM_ID(arg_node)));

    DBUG_RETURN(arg_node);
}

node *CAvardecs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAvardecs");

    TRAVopt(VARDECS_NEXT(arg_node), arg_info);
    TRAVdo(VARDECS_VARDEC(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAvardec(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAvardec");

    TRAVopt(VARDEC_EXPR(arg_node), arg_info);
    registerNewVarDecl(arg_node, arg_info, ID_NAME(VARDEC_ID(arg_node)));

    DBUG_RETURN(arg_node);
}

node *CAfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAfuncall");

    char *name = ID_NAME(FUNCALL_ID(arg_node));
    struct SymbolTableEntry *funDecl = findFunDecl(arg_info, name);
    if (funDecl) {
        TRAVopt(FUNCALL_PARAMS(arg_node), arg_info);
    } else {
        CTIerror("Function [%s] at line %d, column %d has not yet been declared.", name, arg_node->lineno, arg_node->colno);
    }

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
    registerNewVarDecl(arg_node, arg_info, ID_NAME(FOR_ID(arg_node)));
    TRAVdo(FOR_START(arg_node), arg_info);
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

node *CAlocalfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAlocalfundef");

    DBUG_RETURN(arg_node);
}

node *CAlocalfundefs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAlocalfundefs");

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

node *CAid(node * arg_node, info * arg_info) {
    DBUG_ENTER("CAid");

    char *name = ID_NAME(arg_node);
    struct SymbolTableEntry *varDecl = findVarDecl(arg_info, name);
    if (varDecl) {
        ID_DECL(arg_node) = (node *)varDecl;
    } else {
        CTIerror("Variable [%s] which is used at line %d, column %d is not declared.", name, arg_node->lineno, arg_node->colno);
    }

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
