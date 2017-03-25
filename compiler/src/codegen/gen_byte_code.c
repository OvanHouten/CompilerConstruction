#include "gen_byte_code.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "ctinfo.h"

#include "type_utils.h"

// Used for grouping the pseudo codes at the bottom of the assembly program
typedef enum { PP_global, PP_vardef, PP_fundef, PP_none} pseudo_phase;

#define STR(n) ((n) == NULL ? "" : n)

// DO NOT CHANGE THIS NAME TO 'CONSTANTS' OR 'CONSTANT'!!
// Apparently the framework already as a struct with that name. Using one of those
// names leads to very strange behavior and memory (de)allocation errors!
typedef struct CONSTANT_POOL {
    type type;
    int offset;
    struct CONSTANT_POOL *next;
    union {
        float floatVal;
        int intVal;
    };
} constantPool;

/*
 * INFO structure
 */
struct INFO {
  pseudo_phase pseudoPhase;
  constantPool *constants;
  int ifCount;
  int whileCount;
};

/*
 * INFO macros
 */
#define INFO_PSEUDOPHASE(n) ((n)->pseudoPhase)
#define INFO_CONSTANTS(n) ((n)->constants)
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
  INFO_PSEUDOPHASE(result) = PP_none;
  INFO_CONSTANTS(result) = NULL;
  INFO_IFCOUNT(result) = 0;
  INFO_WHILECOUNT(result) = 0;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  constantPool *constants = INFO_CONSTANTS(info);
  while (constants) {
      constantPool *next = constants->next;
      MEMfree(constants);
      constants = next;
  }
  INFO_CONSTANTS(info) = NULL;

  info = MEMfree( info);

  DBUG_RETURN( info);
}

void printParamTypes(node *params) {
    while (params) {
        printf(" %s", typeToString(VARDEF_TYPE(PARAMS_PARAM(params))));
        params = PARAMS_NEXT(params);
    }
}

char *encodeType(type ypeToEncode, int lineNr) {
    char *typeId = "<BOGUS-T>";
    switch (ypeToEncode) {
        case TY_int:
            typeId = "i";
            break;
        case TY_float:
            typeId = "f";
            break;
        case TY_bool:
            typeId = "b";
            break;
        case TY_void:
            break;
        case TY_unknown :
            CTIerror("Type check failed earlier, type information is missing for an instruction on line [%d], can't generate byte code.", lineNr);
    }
    return typeId;
}

char *encodeOperator(binop op, int lineNr) {
    char *operator = "<BOGUS-O>";
    switch (op) {
        case BO_add :
            operator = "add";
            break;
        case BO_sub :
            operator = "sub";
            break;
         case BO_mul :
             operator = "mul";
             break;
         case BO_div :
             operator = "div";
             break;
         case BO_mod :
             operator = "rem";
             break;
         case BO_lt :
             operator = "lt";
             break;
         case BO_le :
             operator = "le";
             break;
         case BO_gt :
             operator = "gt";
             break;
         case BO_ge :
             operator = "ge";
             break;
         case BO_eq :
             operator = "eq";
             break;
         case BO_ne :
             operator = "ne";
             break;
         case BO_and :
             operator = "mul";
             break;
         case BO_or :
             operator = "add";
             break;
         default:
             printf("; Unknown operator used on line [%d].", lineNr);
    }
    return operator;
}

constantPool *registerNewConstant(info *arg_info, type type) {
    DBUG_ENTER("registerNewConstant");

    constantPool *constant = MEMmalloc(sizeof(constantPool));
    constant->next = INFO_CONSTANTS(arg_info);
    INFO_CONSTANTS(arg_info) = constant;

    constant->type = type;
    if (constant->next != NULL) {
        constant->offset = ((constant->next)->offset) + 1;
    } else {
        constant->offset = 0;
    }

    DBUG_RETURN(constant);
}

node *GBCprogram(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCprogram");

    printf("\n; Functions\n");
    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);

    printf("\n; Constants\n");
    constantPool *constant = INFO_CONSTANTS(arg_info);
    while (constant) {
        if (constant->type == TY_int) {
            printf(".const int %d\n", constant->intVal);
        } else {
            printf(".const float %f\n", constant->floatVal);
        }
        constant = constant->next;
    }

    printf("\n; Global variables\n");
    INFO_PSEUDOPHASE(arg_info) = PP_global;
    TRAVopt(PROGRAM_SYMBOLTABLE(arg_node), arg_info);

    printf("\n; Import/export variables\n");
    INFO_PSEUDOPHASE(arg_info) = PP_vardef;
    TRAVopt(PROGRAM_SYMBOLTABLE(arg_node), arg_info);

    printf("\n; Import/export funcation\n");
    INFO_PSEUDOPHASE(arg_info) = PP_fundef;
    TRAVopt(PROGRAM_SYMBOLTABLE(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *GBCsymboltableentry(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCsymboltableentry");

    TRAVopt(SYMBOLTABLEENTRY_NEXT(arg_node), arg_info);

    node *declaration = SYMBOLTABLEENTRY_DECL(arg_node);
    switch (NODE_TYPE(declaration)) {
        case N_vardef :
            switch (INFO_PSEUDOPHASE(arg_info)) {
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

node *GBCstatements(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCstatements");

    TRAVopt(STATEMENTS_NEXT(arg_node), arg_info);
    TRAVdo(STATEMENTS_STATEMENT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *GBCfundef(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCfundef");

    if (!FUNDEF_EXTERN(arg_node)) {
        printf("\n%s:\n", FUNHEADER_NAME(FUNDEF_FUNHEADER(arg_node)));
        if (FUNDEF_SYMBOLTABLE(arg_node) && SYMBOLTABLE_VARIABLES(FUNDEF_SYMBOLTABLE(arg_node)) > 0) {
            int localVarCount = SYMBOLTABLE_VARIABLES(FUNDEF_SYMBOLTABLE(arg_node));
            int paramCount = 0;
            node *params = FUNHEADER_PARAMS(FUNDEF_FUNHEADER(arg_node));
            while (params) {
                paramCount++;
                params = PARAMS_NEXT(params);
            }
            if (localVarCount - paramCount > 0) {
                printf("    esr %d\n", localVarCount - paramCount);
            }
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
        printf("    %sreturn\n", encodeType(determineType(RETURN_EXPR(arg_node)), NODE_LINE(arg_node)));
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
        printf("_while_start_%d:\n", whileCount);
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

    node *symbolTableEntry = ID_DECL(ASSIGN_LET(arg_node));
    node *varDef = SYMBOLTABLEENTRY_DECL(symbolTableEntry);

    char* dataType = encodeType(SYMBOLTABLEENTRY_TYPE(symbolTableEntry), NODE_LINE(arg_node));
    int distance = SYMBOLTABLEENTRY_DISTANCE(symbolTableEntry);
    int offset = SYMBOLTABLEENTRY_OFFSET(symbolTableEntry);

    if (distance == 0 || SYMBOLTABLEENTRY_ASSEMBLERPOSTFIX(VARDEF_DECL(varDef)) != NULL) {
        printf("    %sstore%s %d\n", dataType, STR(SYMBOLTABLEENTRY_ASSEMBLERPOSTFIX(VARDEF_DECL(varDef))), offset);
    } else {
        printf("; Assigning to relative free variables is not yet supported.\n");
    }

    DBUG_RETURN(arg_node);
}

node *GBCid(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCid");

    node *symbolTableEntry = ID_DECL(arg_node);
    node *varDef = SYMBOLTABLEENTRY_DECL(symbolTableEntry);

    char* dataType = encodeType(SYMBOLTABLEENTRY_TYPE(symbolTableEntry), NODE_LINE(arg_node));
    int distance = SYMBOLTABLEENTRY_DISTANCE(symbolTableEntry);
    int offset = SYMBOLTABLEENTRY_OFFSET(symbolTableEntry);

    if (distance == 0) {
        int offset = SYMBOLTABLEENTRY_OFFSET(symbolTableEntry);
        if (offset <= 3) {
            printf("    %sload_%d\n", dataType, offset);
        } else {
            printf("    %sload %d\n", dataType, offset);
        }
    } else if (SYMBOLTABLEENTRY_ASSEMBLERPOSTFIX(VARDEF_DECL(varDef)) != NULL) {
        printf("    %sload%s %d\n", dataType, STR(SYMBOLTABLEENTRY_ASSEMBLERPOSTFIX(VARDEF_DECL(varDef))), offset);
    } else {
        printf("; Using relative free variables is not yet supported.\n");
    }

    DBUG_RETURN(arg_node);
}

node *GBCunop(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCunop");

    // TODO check if the ternary operator replaces boolean typecasts.
    TRAVdo(UNOP_EXPR(arg_node), arg_info);
    printf("    %s%s\n", encodeType(determineType(arg_node), NODE_LINE(arg_node)), UNOP_OP(arg_node) == UO_not ? "not" : "neg");

    DBUG_RETURN(arg_node);
}

node *GBCtypecast(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCtypecast");

    TRAVdo(TYPECAST_EXPR(arg_node), arg_info);
    if (determineType(TYPECAST_EXPR(arg_node)) == TY_bool || TYPECAST_TYPE(arg_node) == TY_bool) {
        printf("; Typecast with boolean is not yet supported.\n");
    } else {
        printf("    %s2%s\n", encodeType(determineType(TYPECAST_EXPR(arg_node)), NODE_LINE(arg_node)), encodeType(TYPECAST_TYPE(arg_node), NODE_LINE(arg_node)));
    }

    DBUG_RETURN(arg_node);
}

node *GBCbinop(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCbinop");

    TRAVdo(BINOP_RIGHT(arg_node), arg_info);
    TRAVdo(BINOP_LEFT(arg_node), arg_info);
    printf("    %s%s\n", encodeType(determineType(arg_node), NODE_LINE(arg_node)), encodeOperator(BINOP_OP(arg_node), NODE_LINE(arg_node)));

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
        default: {
            constantPool *constant = INFO_CONSTANTS(arg_info);
            while (constant != NULL) {
                if (constant->type == TY_int && constant->intVal == INTCONST_VALUE(arg_node)) {
                    break;
                }
                constant = constant->next;
            }
            if (constant == NULL) {
                constant = registerNewConstant(arg_info, TY_int);
                constant->intVal = INTCONST_VALUE(arg_node);
            }

            printf("    iloadc %d\n", constant->offset);
        }
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
        constantPool *constant = INFO_CONSTANTS(arg_info);
        while (constant != NULL) {
            if (constant->type == TY_float && constant->floatVal == FLOATCONST_VALUE(arg_node)) {
                break;
            }
            constant = constant->next;
        }
        if (constant == NULL) {
            constant = registerNewConstant(arg_info, TY_float);
            constant->floatVal = FLOATCONST_VALUE(arg_node);
        }

        printf("    floadc %d\n", constant->offset);
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
