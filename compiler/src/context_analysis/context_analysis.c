/*
 * context_checks.c
 *
 *  Created on: 3 Mar 2017
 *      Author: nico
 */

#include "types.h"
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

struct SymbolTable *makeNewSymbolTable() {
    struct SymbolTable *st = MEMmalloc(sizeof(struct SymbolTable));
    st->parent = NULL;
    st->vardecls = LUTgenerateLut();
    st->fundecls = LUTgenerateLut();
    return st;
}

node *CAprogram(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAprogram");

    INFO_CURRENTSCOPE(arg_info) = makeNewSymbolTable();
    PROGRAM_SYMBOLTABLE(arg_node) = (node *)INFO_CURRENTSCOPE(arg_info);

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

void registerNewFunDecl(node* arg_node, info* arg_info, char* name) {
    lut_t* varDecls = INFO_CURRENTSCOPE(arg_info)->fundecls;
    struct SymbolTableEntry* symbolTableEntry = DEREF_IF_NOT_NULL(LUTsearchInLutS(varDecls, name));
    if (symbolTableEntry) {
        CTIerror(
                "Function [%s] at line %d, column %d has already been declared at line %d, column %d.",
                name, arg_node->lineno, arg_node->colno, symbolTableEntry->decl->lineno, symbolTableEntry->decl->colno);
    } else {
        symbolTableEntry = MEMmalloc(sizeof(struct SymbolTableEntry));
        symbolTableEntry->decl = arg_node;
        LUTinsertIntoLutS(varDecls, name, symbolTableEntry);
    }
}


node *CAfundec(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAfundec");

    registerNewFunDecl(arg_node, arg_info, ID_NAME(FUNHEADER_ID(FUNDEC_FUNHEADER(arg_node))));

    DBUG_RETURN(arg_node);
}

struct SymbolTable* startNewScope(info *arg_info) {
    struct SymbolTable *newScope = makeNewSymbolTable();
    newScope->parent = INFO_CURRENTSCOPE(arg_info);
    INFO_CURRENTSCOPE(arg_info) = newScope;
    return newScope;
}

void closeScope(info *arg_info) {
    INFO_CURRENTSCOPE(arg_info) = INFO_CURRENTSCOPE(arg_info)->parent;
}

node *CAfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAfundef");

    registerNewFunDecl(arg_node, arg_info, ID_NAME(FUNHEADER_ID(FUNDEF_FUNHEADER(arg_node))));
    startNewScope(arg_info);

    TRAVopt(FUNDEF_FUNHEADER(arg_node), arg_info);
    TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

    closeScope(arg_info);

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

void registerNewVarDecl(node* arg_node, info* arg_info, char* name) {
    lut_t* varDecls = INFO_CURRENTSCOPE(arg_info)->vardecls;
    struct SymbolTableEntry* symbolTableEntry = DEREF_IF_NOT_NULL(LUTsearchInLutS(varDecls, name));
    if (symbolTableEntry) {
        CTIerror(
                "Variable [%s] at line %d, column %d has already been declared at line %d, column %d.",
                name, arg_node->lineno, arg_node->colno, symbolTableEntry->decl->lineno, symbolTableEntry->decl->colno);
    } else {
        symbolTableEntry = MEMmalloc(sizeof(struct SymbolTableEntry));
        symbolTableEntry->decl = arg_node;
        LUTinsertIntoLutS(varDecls, name, symbolTableEntry);
    }
}

struct SymbolTableEntry *findVarDecl(info *arg_info, char *name) {
    struct SymbolTable *currentScope = INFO_CURRENTSCOPE(arg_info);
    while (currentScope) {
        lut_t* varDecls = currentScope->vardecls;
        struct SymbolTableEntry* symbolTableEntry = DEREF_IF_NOT_NULL(LUTsearchInLutS(varDecls, name));
        if (symbolTableEntry) {
            return symbolTableEntry;
        } else {
            currentScope = currentScope->parent;
        }
    }
    return NULL;
}

node *CAglobaldec(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAglobaldec");

    registerNewVarDecl(arg_node, arg_info, ID_NAME(GLOBALDEC_ID(arg_node)));

    DBUG_RETURN(arg_node);
}

node *CAglobaldef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAglobaldef");

    TRAVopt(GLOBALDEF_EXPR(arg_node), arg_info);
    registerNewVarDecl(arg_node, arg_info, ID_NAME(GLOBALDEF_ID(arg_node)));

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

    DBUG_RETURN(arg_node);
}

node *CAtypecast(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAtypecast");

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

    DBUG_RETURN(arg_node);
}

node *CAwhile(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAwhile");

    DBUG_RETURN(arg_node);
}

node *CAdo(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAdo");

    DBUG_RETURN(arg_node);
}

node *CAfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAfor");

    DBUG_RETURN(arg_node);
}

node *CAreturn(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAreturn");

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

   DBUG_RETURN(arg_node);
}

node *CArelop(node *arg_node, info *arg_info) {
    DBUG_ENTER("CArelop");

    DBUG_RETURN(arg_node);
}

node *CAlogicop(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAlogicop");

    DBUG_RETURN(arg_node);
}

node *CAunop(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAunop");

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

