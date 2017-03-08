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
    DBUG_ENTER("CCprogram");

    INFO_CURRENTSCOPE(arg_info) = makeNewSymbolTable();
    PROGRAM_SYMBOLTABLE(arg_node) = (node *)INFO_CURRENTSCOPE(arg_info);

    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);

    INFO_CURRENTSCOPE(arg_info) = INFO_CURRENTSCOPE(arg_info)->parent;

    DBUG_RETURN(arg_node);
}


node *CAdeclarations(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCdeclarations");

    TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);
    TRAVdo(DECLARATIONS_DECLARATION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

void registerNewFunDecl(node* arg_node, info* arg_info, char* name) {
    lut_t* varDecls = INFO_CURRENTSCOPE(arg_info)->fundecls;
    struct SymbolTableEntry* symbolTableEntry = DEREF_IF_NOT_NULL(LUTsearchInLutS(varDecls, name));
    if (symbolTableEntry) {
        CTIerror(
                "Function [%s] has already been declared at line %d, column %d.",
                name, symbolTableEntry->decl->lineno,
                symbolTableEntry->decl->colno);
    } else {
        symbolTableEntry = MEMmalloc(sizeof(struct SymbolTableEntry));
        symbolTableEntry->decl = arg_node;
        LUTinsertIntoLutS(varDecls, name, symbolTableEntry);
    }
}


node *CAfundec(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfundec");

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
    DBUG_ENTER("CCfundef");

    registerNewFunDecl(arg_node, arg_info, ID_NAME(FUNHEADER_ID(FUNDEF_FUNHEADER(arg_node))));
    startNewScope(arg_info);

    TRAVopt(FUNDEF_FUNHEADER(arg_node), arg_info);
    TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

    closeScope(arg_info);

    DBUG_RETURN(arg_node);
}


node *CAfunheader(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfunheader");

    TRAVopt(FUNHEADER_PARAMS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAfunbody(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfunbody");

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
                "Variable [%s] has already been declared at line %d, column %d.",
                name, symbolTableEntry->decl->lineno,
                symbolTableEntry->decl->colno);
    } else {
        symbolTableEntry = MEMmalloc(sizeof(struct SymbolTableEntry));
        symbolTableEntry->decl = arg_node;
        LUTinsertIntoLutS(varDecls, name, symbolTableEntry);
    }
}

node *CAglobaldec(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCglobaldec");

    registerNewVarDecl(arg_node, arg_info, ID_NAME(GLOBALDEC_ID(arg_node)));

    DBUG_RETURN(arg_node);
}

node *CAglobaldef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCglobaldef");

    registerNewVarDecl(arg_node, arg_info, ID_NAME(GLOBALDEF_ID(arg_node)));

    DBUG_RETURN(arg_node);
}

node *CAint(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCint");

    DBUG_RETURN(arg_node);
}

node *CAfloat(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfloat");

    DBUG_RETURN(arg_node);
}

node *CAbool(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCbool");

    DBUG_RETURN(arg_node);
}

node *CAparams(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCparams");

    TRAVopt(PARAMS_NEXT(arg_node), arg_info);
    TRAVdo(PARAMS_PARAM(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAparam(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCparam");

    registerNewVarDecl(arg_node, arg_info, ID_NAME(PARAM_ID(arg_node)));

    DBUG_RETURN(arg_node);
}

node *CAvardecs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCvardecs");

    TRAVopt(VARDECS_NEXT(arg_node), arg_info);
    TRAVdo(VARDECS_VARDEC(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAvardec(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCvardec");

    registerNewVarDecl(arg_node, arg_info, ID_NAME(VARDEC_ID(arg_node)));

    DBUG_RETURN(arg_node);
}

node *CAfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfuncall");

    DBUG_RETURN(arg_node);
}

node *CAtypecast(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCtypecast");

    DBUG_RETURN(arg_node);
}

node *CAassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCassign");

    DBUG_RETURN(arg_node);
}

node *CAif(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCif");

    DBUG_RETURN(arg_node);
}

node *CAwhile(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCwhile");

    DBUG_RETURN(arg_node);
}

node *CAdo(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCdo");

    DBUG_RETURN(arg_node);
}

node *CAfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfor");

    DBUG_RETURN(arg_node);
}

node *CAreturn(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCreturn");

    DBUG_RETURN(arg_node);
}

node *CAexprs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCexprs");

    DBUG_RETURN(arg_node);
}

node *CAarithop(node *arg_node, info *arg_info) {
   DBUG_ENTER("CCarithop");

   DBUG_RETURN(arg_node);
}

node *CArelop(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCrelop");

    DBUG_RETURN(arg_node);
}

node *CAlogicop(node *arg_node, info *arg_info) {
    DBUG_ENTER("CClogicop");

    DBUG_RETURN(arg_node);
}

node *CAunop(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCunop");

    DBUG_RETURN(arg_node);
}

node *CAvoid(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCvoid");

    DBUG_RETURN(arg_node);
}

node *CAintconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCintconst");

    DBUG_RETURN(arg_node);
}

node *CAfloatconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfloatconst");

    DBUG_RETURN(arg_node);
}

node *CAboolconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCboolconst");

    DBUG_RETURN(arg_node);
}

node *CAsymboltableentry(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCsymboltableentry");

    DBUG_RETURN(arg_node);
}

node *CAerror(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCerror");

    DBUG_RETURN(arg_node);
}

node *CAlocalfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CClocalfundef");

    DBUG_RETURN(arg_node);
}

node *CAlocalfundefs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CClocalfundefs");

    DBUG_RETURN(arg_node);
}

node *CAarrayassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCarrayassign");

    DBUG_RETURN(arg_node);
}

node *CAarray(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCarray");

    DBUG_RETURN(arg_node);
}

node *CAids(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCids");

    DBUG_RETURN(arg_node);
}

node *CAarrexprs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCarrexprs");

    DBUG_RETURN(arg_node);
}


node *CAstatements(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCstatements");

    printf("Statements\n");

    TRAVopt(STATEMENTS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}


node *CAid(node * arg_node, info * arg_info) {
    DBUG_ENTER("CCid");

    printf("Var usage [%s]\n", ID_NAME(arg_node));

    DBUG_RETURN(arg_node);
}

node *CAdoScopeAnalysis( node *syntaxtree) {
    DBUG_ENTER("CCdoScopeAnslysis");

    info *arg_info = MakeInfo();

    TRAVpush(TR_ca);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}

