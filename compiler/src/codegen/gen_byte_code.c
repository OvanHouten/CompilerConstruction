#include "gen_byte_code.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "ctinfo.h"

#include "type_utils.h"

// Used for grouping the pseudo codes at the top of the assembly program
typedef enum {PP_const, PP_global, PP_vardef, PP_fundef} pseudo_phase;

/*
 * INFO structure
 */
struct INFO {
  pseudo_phase pseudoType;
};

/*
 * INFO macros
 */
#define INFO_PSEUDOTYPE(n) ((n)->pseudoType)

/*
 * INFO functions
 */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));
  INFO_PSEUDOTYPE(result) = STE_const;

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

char *encodeReturnType(type returnType) {
    char *returnChar = "";
    switch (returnType) {
        case TY_int:
            returnChar = "i";
            break;
        case TY_float:
            returnChar = "f";
            break;
        case TY_bool:
            returnChar = "b";
            break;
        case TY_void:
            break;
        case TY_unknown :
            CTIerror("Type check failed earlier, a function is missing a return type, can't generate byte code.");
    }
    return returnChar;
}

node *GBCprogram(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCprogram");

    printf("; Constants\n");
    INFO_PSEUDOTYPE(arg_info) = PP_const;
    TRAVopt(PROGRAM_SYMBOLTABLE(arg_node), arg_info);
    printf("\n; Global variables\n");
    INFO_PSEUDOTYPE(arg_info) = PP_global;
    TRAVopt(PROGRAM_SYMBOLTABLE(arg_node), arg_info);
    printf("\n; Import/export variables\n");
    INFO_PSEUDOTYPE(arg_info) = PP_vardef;
    TRAVopt(PROGRAM_SYMBOLTABLE(arg_node), arg_info);
    printf("\n; Import/export funcation\n");
    INFO_PSEUDOTYPE(arg_info) = PP_fundef;
    TRAVopt(PROGRAM_SYMBOLTABLE(arg_node), arg_info);

    printf("\n; Functions\n");

    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *GBCsymboltableentry(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCsymboltableentry");

    TRAVopt(SYMBOLTABLEENTRY_NEXT(arg_node), arg_info);

    node *declaration = SYMBOLTABLEENTRY_DECL(arg_node);
    switch (NODE_TYPE(declaration)) {
        case N_vardef :
            switch (INFO_PSEUDOTYPE(arg_info)) {
                case PP_const :
                    break;
                case PP_global :
                    if (!VARDEF_EXTERN(declaration)) {
                        printf(".global %s\n", typeToString(VARDEF_TYPE(declaration)));
                    }
                    break;
                case PP_vardef :
                    if (VARDEF_EXTERN(declaration)) {
                        printf(".importvar \"%s\" %s\n", VARDEF_NAME(declaration), typeToString(VARDEF_TYPE(declaration)));
                    } else if (VARDEF_EXPORT(declaration)) {
                        printf(".exportvar \"%s\" %d\n", VARDEF_NAME(declaration), SYMBOLTABLEENTRY_OFFSET(VARDEF_DECL(declaration)));
                    }
                    break;
                default :
                    // Just to get the compiler happy
                    break;
            }
            break;
        case N_fundef :
            if (INFO_PSEUDOTYPE(arg_info) == PP_fundef) {
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
            break;
        default :
            // Just to get the compiler happy
            break;
    }

    DBUG_RETURN(arg_node);
}

node *GBCdeclarations(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCdeclarations");

    TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);
    TRAVdo(DECLARATIONS_DECLARATION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *GBCfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCfundef");

    if (!FUNDEF_EXTERN(arg_node)) {
        printf("\n%s:\n", FUNHEADER_NAME(FUNDEF_FUNHEADER(arg_node)));
        int localVarCount = 0;
        if (FUNDEF_SYMBOLTABLE(arg_node) && SYMBOLTABLE_VARIABLES(FUNDEF_SYMBOLTABLE(arg_node)) > 0) {
            localVarCount = SYMBOLTABLE_VARIABLES(FUNDEF_SYMBOLTABLE(arg_node));
            printf("    esr %d\n", localVarCount);
        }

        TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

        if (FUNHEADER_RETURNTYPE(FUNDEF_FUNHEADER(arg_node)) == TY_void) {
            printf("    return\n");
        }
    }

    DBUG_RETURN(arg_node);
}

node *GBCreturn(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCreturn");

    printf("    %sreturn\n", encodeReturnType(determineType(RETURN_EXPR(arg_node))));

    DBUG_RETURN(arg_node);
}

/*
 * Traversal start function
 */

node *GBCdoGenByteCode( node *syntaxtree)
{
  DBUG_ENTER("GBCdoGenByteCode");

  printf("; Starting the assembler generation...\n\n");

  info *arg_info = MakeInfo();

  TRAVpush(TR_gbc);

  TRAVdo(syntaxtree, arg_info);

  TRAVpop();

  arg_info = FreeInfo(arg_info);

  printf("\n\n; Assembler generation done.\n");

  DBUG_RETURN( syntaxtree);
}
