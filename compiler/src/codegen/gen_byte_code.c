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
  pseudo_phase pseudoPhase;
  int ifCount;
  int whileCount;
};

/*
 * INFO macros
 */
#define INFO_PSEUDOPHASE(n) ((n)->pseudoPhase)
#define INFO_IFCOUNT(n) ((n)->ifCount)
#define INFO_WHILECOUNT(n) ((n)->whileCount)

/*
 * INFO functions
 */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));
  INFO_PSEUDOPHASE(result) = STE_const;
  INFO_IFCOUNT(result) = 0;
  INFO_WHILECOUNT(result) = 0;

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
    INFO_PSEUDOPHASE(arg_info) = PP_const;
    TRAVopt(PROGRAM_SYMBOLTABLE(arg_node), arg_info);
    printf("\n; Global variables\n");
    INFO_PSEUDOPHASE(arg_info) = PP_global;
    TRAVopt(PROGRAM_SYMBOLTABLE(arg_node), arg_info);
    printf("\n; Import/export variables\n");
    INFO_PSEUDOPHASE(arg_info) = PP_vardef;
    TRAVopt(PROGRAM_SYMBOLTABLE(arg_node), arg_info);
    printf("\n; Import/export funcation\n");
    INFO_PSEUDOPHASE(arg_info) = PP_fundef;
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
            switch (INFO_PSEUDOPHASE(arg_info)) {
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
            if (INFO_PSEUDOPHASE(arg_info) == PP_fundef) {
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

    if (RETURN_EXPR(arg_node)) {
        TRAVdo(RETURN_EXPR(arg_node), arg_info);
        printf("    %sreturn\n", encodeReturnType(determineType(RETURN_EXPR(arg_node))));
    } else {
        printf("    return\n");
    }

    DBUG_RETURN(arg_node);
}

node *GBCif(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCif");

    printf("; Line %d\n", NODE_LINE(arg_node));
    int ifCount = INFO_IFCOUNT(arg_info)++;
    // TODO Optimize for empty if-block
    TRAVdo(IF_CONDITION(arg_node), arg_info);
    if (IF_ELSEBLOCK(arg_node)) {
        printf("    branch_f _else_%d\n", ifCount);
        TRAVopt(IF_IFBLOCK(arg_node), arg_info);
        printf("    jump _if_end_%d\n", ifCount);
        printf("_else_%d:\n", ifCount);
        TRAVdo(IF_ELSEBLOCK(arg_node), arg_info);
    } else {
        printf("    branch_f _if_end_%d\n", ifCount);
        TRAVopt(IF_IFBLOCK(arg_node), arg_info);
    }
    printf("_if_end_%d:\n", ifCount);

    DBUG_RETURN(arg_node);
}

node *GBCwhile(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCwhile");

    printf("; Line %d\n", NODE_LINE(arg_node));
    if (WHILE_BLOCK(arg_node)) {
        int whileCount = INFO_WHILECOUNT(arg_info)++;
        printf("_while_start_%d\n", whileCount);
        TRAVdo(WHILE_CONDITION(arg_node), arg_info);
        printf("    branch_f _while_end_%d\n", whileCount);

        TRAVopt(WHILE_BLOCK(arg_node), arg_info);

        printf("    jump _while_start_%d\n", whileCount);
        printf("_while_end_%d:\n", whileCount);
    } else {
        printf("; Empty while block suppressed");
    }
    DBUG_RETURN(arg_node);
}

node *GBCassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCassign");

    TRAVdo(ASSIGN_EXPR(arg_node), arg_info);

    if (SYMBOLTABLEENTRY_DISTANCE(ID_DECL(ASSIGN_LET(arg_node))) == 0) {
        printf("    %sstore %d\n", encodeReturnType(SYMBOLTABLEENTRY_TYPE(ID_DECL(ASSIGN_LET(arg_node)))), SYMBOLTABLEENTRY_OFFSET(ID_DECL(ASSIGN_LET(arg_node))));
    } else {
        printf("; Assigning to non-local variables is not yet supported.\n");
    }

    DBUG_RETURN(arg_node);
}

node *GBCid(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCid");

    if (SYMBOLTABLEENTRY_DISTANCE(ID_DECL(arg_node)) == 0) {
        int offset = SYMBOLTABLEENTRY_OFFSET(ID_DECL(arg_node));
        if (offset <= 3) {
            printf("    %sload_%d\n", encodeReturnType(SYMBOLTABLEENTRY_TYPE(ID_DECL(arg_node))), offset);
        } else {
            printf("    %sload %d\n", encodeReturnType(SYMBOLTABLEENTRY_TYPE(ID_DECL(arg_node))), offset);
        }
    } else {
        printf("; Using non-local variables is not yet supported.\n");
    }

    DBUG_RETURN(arg_node);
}

node *GBCintconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCintconst");

    switch (INTCONST_VALUE(arg_node)) {
        case -1:
            printf("    iloadc_m1\n");
            break;
        case 0:
        case 1:
            printf("    iloadc_%d\n", INTCONST_VALUE(arg_node));
            break;
        default:
            // FIXME
            printf("; Integer constants for return statements other then -1, 0 and 1 are not yet supported!\n");
    }

    DBUG_RETURN(arg_node);
}

node *GBCfloatconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCfloatconst");

    if (FLOATCONST_VALUE(arg_node) == 0.0) {
        printf("    floadc_0\n");
    } else if (FLOATCONST_VALUE(arg_node) == 1.0) {
        printf("    floadc_1\n");
    } else {
        // FIXME
            printf("; Float constants for return statements other then 0 and 1 are not yet supported!\n");
    }

    DBUG_RETURN(arg_node);
}

node *GBCboolconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCboolconst");

    printf("    bloadc_%s\n", BOOLCONST_VALUE(arg_node) ? "t" : "f");

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
