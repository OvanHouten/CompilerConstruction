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

node *CCprogram(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCprogram");

    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}


node *CCdeclarations(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCdeclarations");

    TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);
    TRAVdo(DECLARATIONS_DECLARATION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}


node *CCfundec(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfundec");

    printf("Fundec\n");

    DBUG_RETURN(arg_node);
}


node *CCfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfundef");

    printf("Fundef\n");
    TRAVopt(FUNDEF_FUNHEADER(arg_node), arg_info);
    TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}


node *CCfunheader(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfunheader");

    printf("Funheader\n");

    DBUG_RETURN(arg_node);
}

node *CCfunbody(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfunbody");

    printf("Funbody\n");

    DBUG_RETURN(arg_node);
}

node *CCglobaldec(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCglobaldec");

    printf("Globaldec\n");

    DBUG_RETURN(arg_node);
}

node *CCglobaldef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCglobaldef");

    printf("Globaldef\n");

    DBUG_RETURN(arg_node);
}

node *CCint(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCint");

    DBUG_RETURN(arg_node);
}

node *CCfloat(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfloat");

    DBUG_RETURN(arg_node);
}

node *CCbool(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCbool");

    DBUG_RETURN(arg_node);
}

node *CCparams(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCparams");

    DBUG_RETURN(arg_node);
}

node *CCparam(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCparam");

    DBUG_RETURN(arg_node);
}

node *CCvardecs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCvardecs");

    DBUG_RETURN(arg_node);
}

node *CCfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfuncall");

    DBUG_RETURN(arg_node);
}

node *CCtypecast(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCtypecast");

    DBUG_RETURN(arg_node);
}

node *CCassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCassign");

    DBUG_RETURN(arg_node);
}

node *CCif(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCif");

    DBUG_RETURN(arg_node);
}

node *CCwhile(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCwhile");

    DBUG_RETURN(arg_node);
}

node *CCdo(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCdo");

    DBUG_RETURN(arg_node);
}

node *CCfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfor");

    DBUG_RETURN(arg_node);
}

node *CCreturn(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCreturn");

    DBUG_RETURN(arg_node);
}

node *CCexprs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCexprs");

    DBUG_RETURN(arg_node);
}

node *CCarithop(node *arg_node, info *arg_info) {
   DBUG_ENTER("CCarithop");

   DBUG_RETURN(arg_node);
}

node *CCrelop(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCrelop");

    DBUG_RETURN(arg_node);
}

node *CClogicop(node *arg_node, info *arg_info) {
    DBUG_ENTER("CClogicop");

    DBUG_RETURN(arg_node);
}

node *CCunop(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCunop");

    DBUG_RETURN(arg_node);
}

node *CCvoid(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCvoid");

    DBUG_RETURN(arg_node);
}

node *CCintconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCintconst");

    DBUG_RETURN(arg_node);
}

node *CCfloatconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCfloatconst");

    DBUG_RETURN(arg_node);
}

node *CCboolconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCboolconst");

    DBUG_RETURN(arg_node);
}

node *CCsymboltableentry(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCsymboltableentry");

    DBUG_RETURN(arg_node);
}

node *CCerror(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCerror");

    DBUG_RETURN(arg_node);
}

node *CClocalfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("CClocalfundef");

    DBUG_RETURN(arg_node);
}

node *CClocalfundefs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CClocalfundefs");

    DBUG_RETURN(arg_node);
}

node *CCarrayassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCarrayassign");

    DBUG_RETURN(arg_node);
}

node *CCarray(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCarray");

    DBUG_RETURN(arg_node);
}

node *CCids(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCids");

    DBUG_RETURN(arg_node);
}

node *CCarrexprs(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCarrexprs");

    DBUG_RETURN(arg_node);
}


node *CCstatements(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCstatements");

    printf("Statements\n");

    TRAVopt(STATEMENTS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CCvardec(node *arg_node, info *arg_info) {
    DBUG_ENTER("CCvardec");

    printf("Var Declaration [%s]\n", ID_NAME(VARDEC_ID(arg_node)));

    DBUG_RETURN(arg_node);
}

node *CCid(node * arg_node, info * arg_info) {
    DBUG_ENTER("CCid");

    printf("Var usage [%s]\n", ID_NAME(arg_node));

    DBUG_RETURN(arg_node);
}

node *CCdoScopeAnalysis( node *syntaxtree) {
    DBUG_ENTER("CCdoScopeAnslysis");

    info *arg_info = MakeInfo();

    TRAVpush(TR_cc);

    TRAVdo(syntaxtree, NULL);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}

