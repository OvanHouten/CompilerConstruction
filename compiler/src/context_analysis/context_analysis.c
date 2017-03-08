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

/*
 * INFO structure
 */
struct INFO {
  int scopelevel;
};

/*
 * INFO macros
 */
#define INFO_SCOPELEVEL(n)  ((n)->scopelevel)

/*
 * INFO functions
 */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));

  INFO_SCOPELEVEL(result) = 0;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  info = MEMfree( info);

  DBUG_RETURN( info);
}

node *CAprogram(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCprogram");

    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}


node *CAdeclarations(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCdeclarations");

    TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);
    TRAVdo(DECLARATIONS_DECLARATION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}


node *CAfundec(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfundec");

    printf("Fundec\n");

    DBUG_RETURN(arg_node);
}


node *CAfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfundef");

    printf("Fundef\n");
    TRAVopt(FUNDEF_FUNHEADER(arg_node), arg_info);
    TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}


node *CAfunheader(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfunheader");

    printf("Funheader\n");

    DBUG_RETURN(arg_node);
}

node *CAfunbody(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfunbody");

    printf("Funbody\n");

    DBUG_RETURN(arg_node);
}

node *CAglobaldec(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCglobaldec");

    printf("Globaldec\n");

    DBUG_RETURN(arg_node);
}

node *CAglobaldef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCglobaldef");

    printf("Globaldef\n");

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

    DBUG_RETURN(arg_node);
}

node *CAparam(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCparam");

    DBUG_RETURN(arg_node);
}

node *CAvardecs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCvardecs");

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

node *CAvardec(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCvardec");

    printf("Var Declaration [%s]\n", ID_NAME(VARDEC_ID(arg_node)));

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

    TRAVdo(syntaxtree, NULL);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}

