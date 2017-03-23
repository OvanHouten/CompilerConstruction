#include "gen_byte_code.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"

#include "type_utils.h"

/*
 * INFO structure
 */
struct INFO {
  int varExportCount;
};

/*
 * INFO macros
 */
#define INFO_VAREXPORTCOUNT(n)  ((n)->varExportCount)

/*
 * INFO functions
 */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));
  INFO_VAREXPORTCOUNT(result) = 0;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  info = MEMfree( info);

  DBUG_RETURN( info);
}

void printParamTypes(node *params) {
    while (params) {
        printf(" %s", typeToString(VARDEF_TYPE(PARAMS_PARAM(params))));
        params = PARAMS_NEXT(params);
    }
}

node *GBCprogram(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCprogram");

    TRAVopt(PROGRAM_SYMBOLTABLE(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *GBCsymboltableentry(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCsymboltableentry");

    TRAVopt(SYMBOLTABLEENTRY_NEXT(arg_node), arg_info);
    node *declaration = SYMBOLTABLEENTRY_DECL(arg_node);
    if (NODE_TYPE(declaration) == N_vardef) {
        if (VARDEF_EXTERN(declaration)) {
            printf(".importvar \"%s\" %s\n", VARDEF_NAME(declaration), typeToString(VARDEF_TYPE(declaration)));
        } else if (VARDEF_EXPORT(declaration)) {
            printf(".exportvar \"%s\" %s %d\n", VARDEF_NAME(declaration), typeToString(VARDEF_TYPE(declaration)), INFO_VAREXPORTCOUNT(arg_info)++);
        } else {
            INFO_VAREXPORTCOUNT(arg_info)++;
        }
    } else if (NODE_TYPE(declaration) == N_fundef) {
        if (FUNDEF_EXTERN(declaration)) {
            printf(".importfun \"%s\" %s", FUNHEADER_NAME(FUNDEF_FUNHEADER(declaration)), typeToString(FUNHEADER_RETURNTYPE(FUNDEF_FUNHEADER(declaration))));
            printParamTypes(FUNHEADER_PARAMS(FUNDEF_FUNHEADER(declaration)));
            printf("\n");
        } else if (FUNDEF_EXPORT(declaration)) {
            printf(".exportfun \"%s\" %s", FUNHEADER_NAME(FUNDEF_FUNHEADER(declaration)), typeToString(FUNHEADER_RETURNTYPE(FUNDEF_FUNHEADER(declaration))));
            printParamTypes(FUNHEADER_PARAMS(FUNDEF_FUNHEADER(declaration)));
            printf(" %s\n", FUNHEADER_NAME(FUNDEF_FUNHEADER(declaration)));
        }
    }

    DBUG_RETURN(arg_node);
}

/*
 * Traversal start function
 */

node *GBCdoGenByteCode( node *syntaxtree)
{
  DBUG_ENTER("GBCdoGenByteCode");

  printf("Starting the assembler generation...\n\n");

  info *arg_info = MakeInfo();

  TRAVpush(TR_gbc);

  TRAVdo(syntaxtree, arg_info);

  TRAVpop();

  arg_info = FreeInfo(arg_info);

  printf("\n\nAssembler generation done.\n");

  DBUG_RETURN( syntaxtree);
}
