#include "gen_byte_code.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "ctinfo.h"
#include "globals.h"

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
  int doCount;
};

/*
 * INFO macros
 */
#define INFO_PSEUDOPHASE(n) ((n)->pseudoPhase)
#define INFO_CONSTANTS(n) ((n)->constants)
#define INFO_IFCOUNT(n) ((n)->ifCount)
#define INFO_WHILECOUNT(n) ((n)->whileCount)
#define INFO_DOCOUNT(n) ((n)->doCount)

static FILE *outfile = NULL;

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
  INFO_DOCOUNT(result) = 0;

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
    if (params) {
        printParamTypes(PARAMS_NEXT(params));
        fprintf(outfile, " %s", typeToString(VARDEF_TYPE(PARAMS_PARAM(params))));
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
             fprintf(outfile, "; Unknown operator used on line [%d].", lineNr);
    }
    return operator;
}

void prepareExpressions(node *exprs, info *info, int *expressionCount) {

    if (exprs) {
        (*expressionCount)++;
        prepareExpressions(EXPRS_NEXT(exprs), info, expressionCount);
        TRAVdo(EXPRS_EXPR(exprs), info);
    }
}

void printConstants(constantPool *constant) {
    if (constant) {
        printConstants(constant->next);
        if (constant->type == TY_int) {
            fprintf(outfile, ".const int %d\n", constant->intVal);
        } else {
            fprintf(outfile, ".const float %f\n", constant->floatVal);
        }
    }
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

    fprintf(outfile, "; %s\n", global.infile);

    fprintf(outfile, "\n; Functions\n");
    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);

    fprintf(outfile, "\n; Constants\n");
    printConstants(INFO_CONSTANTS(arg_info));

    fprintf(outfile, "\n; Global variables\n");
    INFO_PSEUDOPHASE(arg_info) = PP_global;
    TRAVopt(PROGRAM_SYMBOLTABLE(arg_node), arg_info);

    fprintf(outfile, "\n; Import/export variables\n");
    INFO_PSEUDOPHASE(arg_info) = PP_vardef;
    TRAVopt(PROGRAM_SYMBOLTABLE(arg_node), arg_info);

    fprintf(outfile, "\n; Import/export funcation\n");
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
                        fprintf(outfile, ".global %s\n", typeToString(VARDEF_TYPE(declaration)));
                    }
                    break;
                case PP_vardef :
                    if (VARDEF_EXTERN(declaration)) {
                        fprintf(outfile, ".importvar \"%s\" %s\n", VARDEF_NAME(declaration), typeToString(VARDEF_TYPE(declaration)));
                    } else if (VARDEF_EXPORT(declaration)) {
                        fprintf(outfile, ".exportvar \"%s\" %d\n", VARDEF_NAME(declaration), SYMBOLTABLEENTRY_OFFSET(VARDEF_DECL(declaration)));
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
                    fprintf(outfile, ".importfun \"%s\" %s", FUNHEADER_NAME(FUNDEF_FUNHEADER(declaration)), typeToString(FUNHEADER_RETURNTYPE(FUNDEF_FUNHEADER(declaration))));
                    printParamTypes(FUNHEADER_PARAMS(FUNDEF_FUNHEADER(declaration)));
                    fprintf(outfile, "\n");
                } else if (FUNDEF_EXPORT(declaration)) {
                    fprintf(outfile, ".exportfun \"%s\" %s", FUNHEADER_NAME(FUNDEF_FUNHEADER(declaration)), typeToString(FUNHEADER_RETURNTYPE(FUNDEF_FUNHEADER(declaration))));
                    printParamTypes(FUNHEADER_PARAMS(FUNDEF_FUNHEADER(declaration)));
                    fprintf(outfile, " %s\n", FUNHEADER_NAME(FUNDEF_FUNHEADER(declaration)));
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

    // No need to try the generate code for external functions
    if (!FUNDEF_EXTERN(arg_node)) {

        TRAVopt(FUNBODY_LOCALFUNDEFS(FUNDEF_FUNBODY(arg_node)), arg_info);

        if (FUNDEF_DECL(arg_node)) {
            fprintf(outfile, "\n%s:\n", SYMBOLTABLEENTRY_NAME(FUNDEF_DECL(arg_node)));
        } else {
            CTIerror("We're very sorry to let you know that you found a bug in the compiler. Please contact the developers.\nThe instruction on line %d of your program revealed the bug (missing STE for function).", NODE_LINE(arg_node));
        }
        if (FUNDEF_SYMBOLTABLE(arg_node) && SYMBOLTABLE_VARIABLES(FUNDEF_SYMBOLTABLE(arg_node)) > 0) {
            int localVarCount = SYMBOLTABLE_VARIABLES(FUNDEF_SYMBOLTABLE(arg_node));
            int paramCount = 0;
            node *params = FUNHEADER_PARAMS(FUNDEF_FUNHEADER(arg_node));
            while (params) {
                paramCount++;
                params = PARAMS_NEXT(params);
            }
            if (localVarCount - paramCount > 0) {
                fprintf(outfile, "    esr %d\n", localVarCount - paramCount);
            }
        }

        TRAVopt(FUNBODY_STATEMENTS(FUNDEF_FUNBODY(arg_node)), arg_info);

        if (FUNHEADER_RETURNTYPE(FUNDEF_FUNHEADER(arg_node)) == TY_void) {
            fprintf(outfile, "    return\n");
        }
    }

    DBUG_RETURN(arg_node);
}

node *GBCreturn(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCreturn");

    if (RETURN_EXPR(arg_node)) {
        TRAVdo(RETURN_EXPR(arg_node), arg_info);
        fprintf(outfile, "    %sreturn\n", encodeType(determineType(RETURN_EXPR(arg_node)), NODE_LINE(arg_node)));
    } else {
        fprintf(outfile, "    return\n");
    }

    DBUG_RETURN(arg_node);
}

node *GBCfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCfuncall");

    if (SYMBOLTABLEENTRY_GLOBALFUN(FUNCALL_DECL(arg_node))) {
        fprintf(outfile, "    isrg\t\t\t; %s\n", SYMBOLTABLEENTRY_NAME(FUNCALL_DECL(arg_node)));
    } else if (SYMBOLTABLEENTRY_DISTANCE(FUNCALL_DECL(arg_node)) == 0) {
        fprintf(outfile, "    isrl\t\t\t; %s\n", SYMBOLTABLEENTRY_NAME(FUNCALL_DECL(arg_node)));
    } else if (SYMBOLTABLEENTRY_DISTANCE(FUNCALL_DECL(arg_node)) > 1) {
        fprintf(outfile, "    isrn %d\t\t\t; %s\n", SYMBOLTABLEENTRY_DISTANCE(FUNCALL_DECL(arg_node)) - 1, SYMBOLTABLEENTRY_NAME(FUNCALL_DECL(arg_node)));
    } else {
        fprintf(outfile, "    isr\t\t\t\t; %s\n", SYMBOLTABLEENTRY_NAME(FUNCALL_DECL(arg_node)));
    }

    int expressionCount = 0;
    prepareExpressions(FUNCALL_EXPRS(arg_node), arg_info, &expressionCount);

    if (FUNDEF_EXTERN(SYMBOLTABLEENTRY_DECL(FUNCALL_DECL(arg_node)))) {
        fprintf(outfile, "    jsre %d\t\t\t; %s()\n", SYMBOLTABLEENTRY_OFFSET(FUNCALL_DECL(arg_node)), SYMBOLTABLEENTRY_NAME(FUNCALL_DECL(arg_node)));
    } else {
        fprintf(outfile, "    jsr %d %s\n", expressionCount, SYMBOLTABLEENTRY_NAME(FUNDEF_DECL(SYMBOLTABLEENTRY_DECL(FUNCALL_DECL(arg_node)))));
    }
    if (FUNCALL_PROCEDURECALL(arg_node) && FUNHEADER_RETURNTYPE(FUNDEF_FUNHEADER(SYMBOLTABLEENTRY_DECL(FUNCALL_DECL(arg_node)))) != TY_void) {
        fprintf(outfile, "    %spop\n", encodeType(FUNHEADER_RETURNTYPE(FUNDEF_FUNHEADER(SYMBOLTABLEENTRY_DECL(FUNCALL_DECL(arg_node)))), NODE_LINE(arg_node)));
    }

    DBUG_RETURN(arg_node);
}

node *GBCif(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCif");

    fprintf(outfile, "; Line %d\n", NODE_LINE(arg_node));
    int ifCount = INFO_IFCOUNT(arg_info)++;
    // TODO Optimize for empty if-block
    TRAVdo(IF_CONDITION(arg_node), arg_info);
    if (IF_ELSEBLOCK(arg_node)) {
        fprintf(outfile, "    branch_f _if_else_%d\n", ifCount);
        TRAVopt(IF_IFBLOCK(arg_node), arg_info);
        fprintf(outfile, "    jump _if_end_%d\n", ifCount);
        fprintf(outfile, "_if_else_%d:\n", ifCount);
        TRAVdo(IF_ELSEBLOCK(arg_node), arg_info);
    } else {
        fprintf(outfile, "    branch_f _if_end_%d\n", ifCount);
        TRAVopt(IF_IFBLOCK(arg_node), arg_info);
    }
    fprintf(outfile, "_if_end_%d:\n", ifCount);

    DBUG_RETURN(arg_node);
}

node *GBCternop(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCternop");

    fprintf(outfile, "; Line %d\n", NODE_LINE(arg_node));
    int ifCount = INFO_IFCOUNT(arg_info)++;
    TRAVdo(TERNOP_CONDITION(arg_node), arg_info);
    fprintf(outfile, "    branch_f _ternop_else_%d\n", ifCount);
    TRAVdo(TERNOP_THEN(arg_node), arg_info);
    fprintf(outfile, "    jump _ternop_end_%d\n", ifCount);
    fprintf(outfile, "_ternop_else_%d:\n", ifCount);
    TRAVdo(TERNOP_ELSE(arg_node), arg_info);
    fprintf(outfile, "_ternop_end_%d:\n", ifCount);

    DBUG_RETURN(arg_node);
}

node *GBCwhile(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCwhile");

    fprintf(outfile, "; Line %d\n", NODE_LINE(arg_node));
    if (WHILE_BLOCK(arg_node)) {
        int whileCount = INFO_WHILECOUNT(arg_info)++;
        fprintf(outfile, "_while_start_%d:\n", whileCount);
        TRAVdo(WHILE_CONDITION(arg_node), arg_info);
        fprintf(outfile, "    branch_f _while_end_%d\n", whileCount);

        TRAVopt(WHILE_BLOCK(arg_node), arg_info);

        fprintf(outfile, "    jump _while_start_%d\n", whileCount);
        fprintf(outfile, "_while_end_%d:\n", whileCount);
    } else {
        fprintf(outfile, "; Empty while block suppressed");
    }
    DBUG_RETURN(arg_node);
}

node *GBCdo(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCwhile");

    fprintf(outfile, "; Line %d\n", NODE_LINE(arg_node));
    if (DO_BLOCK(arg_node)) {
        int doCount = INFO_DOCOUNT(arg_info)++;
        fprintf(outfile, "_do_start_%d:\n", doCount);

        TRAVopt(DO_BLOCK(arg_node), arg_info);

        TRAVdo(DO_CONDITION(arg_node), arg_info);
        fprintf(outfile, "    branch_t _do_start_%d\n", doCount);
    } else {
        fprintf(outfile, "; Empty do block suppressed");
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
        fprintf(outfile, "    %sstore%s %d\n", dataType, STR(SYMBOLTABLEENTRY_ASSEMBLERPOSTFIX(VARDEF_DECL(varDef))), offset);
    } else {
        fprintf(outfile, "    %sstoren %d %d\n", dataType, distance, offset);
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
            fprintf(outfile, "    %sload_%d\n", dataType, offset);
        } else {
            fprintf(outfile, "    %sload %d\n", dataType, offset);
        }
    } else if (SYMBOLTABLEENTRY_ASSEMBLERPOSTFIX(VARDEF_DECL(varDef)) != NULL) {
        fprintf(outfile, "    %sload%s %d\n", dataType, STR(SYMBOLTABLEENTRY_ASSEMBLERPOSTFIX(VARDEF_DECL(varDef))), offset);
    } else {
        fprintf(outfile, "    %sloadn %d %d\n", dataType, distance, offset);
    }

    DBUG_RETURN(arg_node);
}

node *GBCunop(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCunop");

    // TODO check if the ternary operator replaces boolean typecasts.
    TRAVdo(UNOP_EXPR(arg_node), arg_info);
    fprintf(outfile, "    %s%s\n", encodeType(determineType(arg_node), NODE_LINE(arg_node)), UNOP_OP(arg_node) == UO_not ? "not" : "neg");

    DBUG_RETURN(arg_node);
}

node *GBCtypecast(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCtypecast");

    TRAVdo(TYPECAST_EXPR(arg_node), arg_info);
    if (determineType(TYPECAST_EXPR(arg_node)) == TY_bool || TYPECAST_TYPE(arg_node) == TY_bool) {
        fprintf(outfile, "; Typecast with boolean is not yet supported.\n");
    } else {
        fprintf(outfile, "    %s2%s\n", encodeType(determineType(TYPECAST_EXPR(arg_node)), NODE_LINE(arg_node)), encodeType(TYPECAST_TYPE(arg_node), NODE_LINE(arg_node)));
    }

    DBUG_RETURN(arg_node);
}

node *GBCbinop(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCbinop");

    TRAVdo(BINOP_LEFT(arg_node), arg_info);
    TRAVdo(BINOP_RIGHT(arg_node), arg_info);
    fprintf(outfile, "    %s%s\n", encodeType(determineType(BINOP_LEFT(arg_node)), NODE_LINE(arg_node)), encodeOperator(BINOP_OP(arg_node), NODE_LINE(arg_node)));

    DBUG_RETURN(arg_node);
}

node *GBCintconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCintconst");

    switch (INTCONST_VALUE(arg_node)) {
        case -1:
            fprintf(outfile, "    iloadc_m1\n");
            break;
        case 0:
        case 1:
            fprintf(outfile, "    iloadc_%d\n", INTCONST_VALUE(arg_node));
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

            fprintf(outfile, "    iloadc %d\n", constant->offset);
        }
    }

    DBUG_RETURN(arg_node);
}

node *GBCfloatconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCfloatconst");

    if (FLOATCONST_VALUE(arg_node) == 0.0) {
        fprintf(outfile, "    floadc_0\n");
    } else if (FLOATCONST_VALUE(arg_node) == 1.0) {
        fprintf(outfile, "    floadc_1\n");
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

        fprintf(outfile, "    floadc %d\n", constant->offset);
    }

    DBUG_RETURN(arg_node);
}

node *GBCboolconst(node *arg_node, info *arg_info) {
    DBUG_ENTER("GBCboolconst");

    fprintf(outfile, "    bloadc_%s\n", BOOLCONST_VALUE(arg_node) ? "t" : "f");

    DBUG_RETURN(arg_node);
}

/*
 * Traversal start function
 */

node *GBCdoGenByteCode( node *syntaxtree) {
    DBUG_ENTER("GBCdoGenByteCode");

    if (global.outfile) {
        outfile = fopen(global.outfile, "w+");
    } else {
        outfile = stdout;
    }

    info *arg_info = MakeInfo();

    TRAVpush(TR_gbc);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    if (global.outfile) {
        fclose(outfile);
    }
    outfile = NULL;

    DBUG_RETURN( syntaxtree);
}
