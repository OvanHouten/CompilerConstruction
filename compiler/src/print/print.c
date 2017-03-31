
/**
 * @file print.c
 *
 * Functions needed by print traversal.
 *
 */

#include "print.h"
#include "traverse.h"
#include "tree_basic.h"
#include "dbug.h"
#include "memory.h"
#include "globals.h"
#include "myglobals.h"

#include "type_utils.h"

/*
 * INFO structure
 */
struct INFO {
  bool firsterror;
  int indent;
  bool isNewLine;
};

#define INFO_FIRSTERROR(n) ((n)->firsterror)
#define INFO_INDENT(n) ((n)->indent)
#define INFO_IS_NEW_LINE(n) ((n)->isNewLine)

// Default indent size
#define INDENT_SIZE 4
// Macro to increase the indentation, as a side effect the first call to INDENT() will start on a new line
#define INCREASE_INDENTATION(arg_info) INFO_INDENT(arg_info) += INDENT_SIZE; INDENT_AT_NEWLINE(arg_info)
// Macro to decrease the indentation, as a side effect the first call to INDENT() will start on a new line
#define DECREASE_INDENTATION(arg_info) INFO_INDENT(arg_info) -= INDENT_SIZE; INDENT_AT_NEWLINE(arg_info)
// Will print the indentation if, and only if it is needed, as a side effect
#define INDENT(arg_info) printf("%*s", INFO_IS_NEW_LINE(arg_info) ? INFO_INDENT(arg_info) : 0, ""); INFO_IS_NEW_LINE(arg_info) = FALSE;
#define INDENT_AT_NEWLINE(arg_info) INFO_IS_NEW_LINE(arg_info) = TRUE;

static info *MakeInfo()
{
  info *result;
  
  result = MEMmalloc(sizeof(info));

  INFO_FIRSTERROR(result) = FALSE;
  INFO_INDENT(result) = 0;
  INFO_IS_NEW_LINE(result) = FALSE;

  return result;
}


static info *FreeInfo( info *info)
{
  info = MEMfree( info);

  return info;
}

/*
 * Traversal code starts here
 */

node *PRTprogram(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTprogram");
	
	TRAVopt(PROGRAM_SYMBOLTABLE(arg_node), arg_info);
	TRAVdo(PROGRAM_DECLARATIONS(arg_node), arg_info);

	DBUG_RETURN(arg_node);
}

node* PRTsymboltable(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTsymboltable");

	if (myglobal.print_st || myglobal.full_st_print) {
        INDENT(arg_info);
        printf("/*\n");
        INDENT_AT_NEWLINE(arg_info);

        INDENT(arg_info);
        printf(" * Symbol Table\n");
        INDENT_AT_NEWLINE(arg_info);

        INDENT(arg_info);
        printf(" * D  O Type    Name\n");
        INDENT_AT_NEWLINE(arg_info);

        INDENT(arg_info);
        printf(" * -------------------------------\n");
        INDENT_AT_NEWLINE(arg_info);

        TRAVopt(SYMBOLTABLE_SYMBOLTABLEENTRY(arg_node), arg_info);

        INDENT(arg_info);
        printf(" */\n");
        INDENT_AT_NEWLINE(arg_info);
	}

	DBUG_RETURN(arg_node);
}

/*************************************************************************
 *
 * @fn PRTsymboltableentry
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *PRTsymboltableentry (node * arg_node, info * arg_info) {
	DBUG_ENTER ("PRTsymboltableentry");
	
	TRAVopt(SYMBOLTABLEENTRY_NEXT(arg_node), arg_info);

	INDENT(arg_info);
	if (SYMBOLTABLEENTRY_ENTRYTYPE(arg_node) == STE_vardef || myglobal.full_st_print) {
        if (SYMBOLTABLEENTRY_ENTRYTYPE(arg_node) == STE_fundef) {
            printf(" * %d  - %-7s %s()\n", SYMBOLTABLEENTRY_DISTANCE(arg_node), typeToString(SYMBOLTABLEENTRY_TYPE(arg_node)), SYMBOLTABLEENTRY_NAME(arg_node));
        } else {
            if (!VARDEF_EXTERN(SYMBOLTABLEENTRY_DECL(arg_node))) {
                printf(" * %d %2d %-7s %s\n", SYMBOLTABLEENTRY_DISTANCE(arg_node), SYMBOLTABLEENTRY_OFFSET(arg_node), typeToString(SYMBOLTABLEENTRY_TYPE(arg_node)), SYMBOLTABLEENTRY_NAME(arg_node));
            }
        }
        INDENT_AT_NEWLINE(arg_info);
	}
	
	DBUG_RETURN (arg_node);
}

node *PRTdeclarations(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTdeclarations");

	TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);
	TRAVopt(DECLARATIONS_DECLARATION(arg_node), arg_info);
	if (NODE_TYPE(DECLARATIONS_DECLARATION(arg_node)) == N_vardef) {
	    printf(";\n");
	    INDENT_AT_NEWLINE(arg_info);
	}
	
	TRAVopt(NODE_ERROR(arg_node), arg_info);
	
	DBUG_RETURN(arg_node);
}

node *PRTfunheader(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTfunheader");
	printf("%s %s", typeToString(FUNHEADER_RETURNTYPE(arg_node)), FUNHEADER_NAME(arg_node));
	printf("(");
	if (FUNHEADER_PARAMS(arg_node)) {
	    TRAVopt(FUNHEADER_PARAMS(arg_node), arg_info);
	}
	printf(")");

	DBUG_RETURN(arg_node);
}

node *PRTfundef(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTfunDef");
	
    if (FUNDEF_EXTERN(arg_node)) {
        printf("extern ");
    }
	if (FUNDEF_EXPORT(arg_node)) {
		printf("export ");
	}
	TRAVdo(FUNDEF_FUNHEADER(arg_node), arg_info);
	if (FUNDEF_FUNBODY(arg_node)) {
        printf(" {\n");
        INCREASE_INDENTATION(arg_info);
		
		TRAVopt(FUNDEF_SYMBOLTABLE(arg_node), arg_info);
        TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

        DECREASE_INDENTATION(arg_info);
        printf("}\n");
        INDENT_AT_NEWLINE(arg_info);
	} else {
	    printf(";\n");
	}

	DBUG_RETURN(arg_node);
}

node *PRTfunbody(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTfunbody");

	TRAVopt(FUNBODY_VARDECS(arg_node), arg_info);
	TRAVopt(FUNBODY_STATEMENTS(arg_node), arg_info);

	DBUG_RETURN(arg_node);
}

node *PRTvardecs(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTvardecs");

	TRAVopt(VARDECS_NEXT(arg_node), arg_info);

	TRAVdo(VARDECS_VARDEC(arg_node), arg_info);
	printf(";\n");
	INDENT_AT_NEWLINE(arg_info);

	DBUG_RETURN(arg_node);
}

node *PRTvardef(node * arg_node, info * arg_info) {
    DBUG_ENTER("PRTvarDef");

    INDENT(arg_info);
    if (VARDEF_EXTERN(arg_node)) {
        printf("extern ");
    }
    if (VARDEF_EXPORT(arg_node)) {
        printf("export ");
    }
    if (VARDEF_DECL(arg_node)) {
        printf("%s %s", typeToString(VARDEF_TYPE(arg_node)), SYMBOLTABLEENTRY_NAME(VARDEF_DECL(arg_node)));
    } else {
        // Parameters for external functions don't have an entry in the SymbolTable!
        printf("%s %s", typeToString(VARDEF_TYPE(arg_node)), VARDEF_NAME(arg_node));
    }
    if (VARDEF_EXPR(arg_node)) {
        printf(" = ");
        TRAVdo(VARDEF_EXPR(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node *PRTstatements(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTstatements");

	TRAVopt(STATEMENTS_NEXT(arg_node), arg_info);

	TRAVdo(STATEMENTS_STATEMENT(arg_node), arg_info);
	switch (NODE_TYPE(STATEMENTS_STATEMENT(arg_node))) {
	case N_assign:
	case N_funcall:
	case N_shortcut:
	    printf(";\n");
	    INDENT_AT_NEWLINE(arg_info);
	    break;
	default:;
		// No need to print a semicolon and force printing at the correct indentation level.
	}

	DBUG_RETURN(arg_node);
}

node *PRTassign (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTassign");

  INDENT(arg_info);
  TRAVdo( ASSIGN_LET( arg_node), arg_info);
  printf( " = ");
  TRAVdo( ASSIGN_EXPR( arg_node), arg_info);

  DBUG_RETURN (arg_node);
}

node *PRTshortcut(node *arg_node, info *arg_info) {
    DBUG_ENTER("PRTshortcut");

    INDENT(arg_info);
    TRAVdo( SHORTCUT_ID( arg_node), arg_info);
    if (SHORTCUT_OP(arg_node) == SO_inc) {
        printf( " += ");
    } else if (SHORTCUT_OP(arg_node) == SO_dec) {
        printf( " += ");
    }
    TRAVdo( SHORTCUT_CONST( arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *PRTtypecast(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTtypecast");

	printf("(%s)", typeToString(TYPECAST_TYPE(arg_node)));
	TRAVdo(TYPECAST_EXPR(arg_node), arg_info);

	DBUG_RETURN(arg_node);
}


node *PRTfuncall(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTfuncall");

    INDENT(arg_info);
	printf("%s", FUNCALL_NAME(arg_node));
	printf("(");
	TRAVopt(FUNCALL_EXPRS(arg_node), arg_info);
	printf(")");

	DBUG_RETURN(arg_node);
}

node *PRTreturn (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTreturn");

  INDENT(arg_info);
  printf("return");
  if (RETURN_EXPR(arg_node)) {
	  printf(" ");
	  TRAVdo(RETURN_EXPR(arg_node), arg_info);
  }
  printf(";\n");
  INDENT_AT_NEWLINE(arg_info);

  DBUG_RETURN (arg_node);
}

node *PRTif (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTif");

  INDENT(arg_info);
  printf("if (");
  TRAVdo( IF_CONDITION( arg_node), arg_info);
  printf(") {\n");
  INCREASE_INDENTATION(arg_info);
  
  TRAVdo( IF_IFBLOCK( arg_node), arg_info);

  if (IF_ELSEBLOCK( arg_node) != NULL) {
	  DECREASE_INDENTATION(arg_info);
	  INDENT(arg_info);
	  printf("} else {\n");
	  INCREASE_INDENTATION(arg_info);
	  TRAVdo(IF_ELSEBLOCK( arg_node), arg_info);
  }

  DECREASE_INDENTATION(arg_info);
  INDENT(arg_info);
  printf( "}\n");
  INDENT_AT_NEWLINE(arg_info);

  DBUG_RETURN (arg_node);
}

node *PRTwhile (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTwhile");

  INDENT(arg_info);
  printf("while (");
  TRAVdo( WHILE_CONDITION( arg_node), arg_info);
  printf(") {\n");
  INCREASE_INDENTATION(arg_info);

  TRAVdo( WHILE_BLOCK( arg_node), arg_info);

  DECREASE_INDENTATION(arg_info);
  INDENT(arg_info);
  printf( "}\n");
  INDENT_AT_NEWLINE(arg_info);

  DBUG_RETURN (arg_node);
}

node *PRTdo (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTdo");

  INDENT(arg_info);
  printf("do {\n");
  INCREASE_INDENTATION(arg_info);

  TRAVdo( DO_BLOCK( arg_node), arg_info);

  DECREASE_INDENTATION(arg_info);
  INDENT(arg_info);
  printf("} while (");

  TRAVdo( DO_CONDITION( arg_node), arg_info);

  printf( ");\n");
  INDENT_AT_NEWLINE(arg_info);

  DBUG_RETURN (arg_node);
}

node *PRTfor (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTfor");

  INDENT(arg_info);
  printf("for ( ");
  if (FOR_VARDEF(arg_node)) {
      if (VARDEF_TYPE(FOR_VARDEF(arg_node))) {
          printf("int ");
      }
      if (!myglobal.print_var_details) {
          printf("%s", SYMBOLTABLEENTRY_NAME(VARDEF_DECL(FOR_VARDEF(arg_node))));
      } else {
          printf("%s /* %d %d */", SYMBOLTABLEENTRY_NAME(VARDEF_DECL(FOR_VARDEF(arg_node))), SYMBOLTABLEENTRY_DISTANCE(VARDEF_DECL(FOR_VARDEF(arg_node))), SYMBOLTABLEENTRY_OFFSET(VARDEF_DECL(FOR_VARDEF(arg_node))));
      }
      if (VARDEF_EXPR(FOR_VARDEF(arg_node))) {
          printf(" = ");
          TRAVdo(VARDEF_EXPR(FOR_VARDEF(arg_node)), arg_info);
      }
      printf(", ");
      TRAVdo( FOR_FINISH(arg_node), arg_info);
      if (FOR_STEP(arg_node)) {
          printf(", ");
          TRAVdo(FOR_STEP(arg_node), arg_info);
      }
  }
  printf(" ) {\n");
  INCREASE_INDENTATION(arg_info);

  TRAVopt(FOR_BLOCK( arg_node), arg_info);

  DECREASE_INDENTATION(arg_info);
  INDENT(arg_info);
  printf("}\n");
  INDENT_AT_NEWLINE(arg_info);

  DBUG_RETURN (arg_node);
}

node *PRTexprs(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTexpressions");

	TRAVopt(EXPRS_NEXT(arg_node), arg_info);
	if (EXPRS_NEXT(arg_node)) {
		printf(", ");
	}
    TRAVdo(EXPRS_EXPR(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *PRTternop (node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTternop");
	
	printf("(");
	TRAVdo(TERNOP_CONDITION(arg_node), arg_info);
	printf("?");
	TRAVdo(TERNOP_THEN(arg_node), arg_info);
	printf(":");
	TRAVdo(TERNOP_ELSE(arg_node), arg_info);
	printf(")");
	
    DBUG_RETURN(arg_node);
}

node *PRTbinop (node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTbinop");

	printf("(");
	TRAVdo(BINOP_LEFT(arg_node), arg_info);

	char* op;
	switch(BINOP_OP(arg_node)) {
	case BO_lt:
		op = "<";
		break;
	case BO_le:
		op = "<=";
		break;
	case BO_eq:
		op = "==";
		break;
	case BO_ne:
		op = "!=";
		break;
	case BO_ge:
		op = ">=";
		break;
	case BO_gt:
		op = ">";
		break;
	case BO_mul:
		op = "*";
		break;
	case BO_div:
		op = "/";
		break;
	case BO_add:
		op = "+";
		break;
	case BO_sub:
		op = "-";
		break;
	case BO_mod:
		op = "%";
		break;
	case BO_and:
		op = "&&";
		break;
	case BO_or:
		op = "||";
		break;
	default:
		op = "<<UNKNOWN>>";
	}
	printf(" %s ", op);

	TRAVdo(BINOP_RIGHT(arg_node), arg_info);
	printf(")");

	DBUG_RETURN(arg_node);
}

node * PRTunop (node * arg_node, info * arg_info)
{
  char *tmp;

  DBUG_ENTER ("PRTunop");

  switch (UNOP_OP( arg_node)) {
    case UO_neg:
      tmp = "-";
      break;
    case UO_not:
      tmp = "!";
      break;
    case UO_unknown:
      DBUG_ASSERT( 0, "unknown unop detected!");
  }

  printf( "%s(", tmp);
  UNOP_EXPR( arg_node) = TRAVdo( UNOP_EXPR( arg_node), arg_info);
  printf( ")");

  DBUG_RETURN (arg_node);
}


node *PRTid(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTid");

	INDENT(arg_info);
	if (!myglobal.print_var_details) {
	    printf("%s", SYMBOLTABLEENTRY_NAME(ID_DECL(arg_node)));
	} else {
	    printf("%s /* %d %d */", SYMBOLTABLEENTRY_NAME(ID_DECL(arg_node)), SYMBOLTABLEENTRY_DISTANCE(ID_DECL(arg_node)), SYMBOLTABLEENTRY_OFFSET(ID_DECL(arg_node)));
	}

	DBUG_RETURN(arg_node);
}

// PARAMS

node *PRTparams(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTparams");

	TRAVopt(PARAMS_NEXT(arg_node), arg_info);
	if (PARAMS_NEXT(arg_node) != NULL) {
		printf(", ");
	}
	TRAVdo(PARAMS_PARAM(arg_node), arg_info);

	DBUG_RETURN(arg_node);
}

// TYPES

node *PRTintconst(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTintconst");

	printf("%d", INTCONST_VALUE(arg_node));

	DBUG_RETURN(arg_node);
}

node *PRTfloatconst(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTfloatconst");

	printf("%f", FLOATCONST_VALUE(arg_node));

	DBUG_RETURN(arg_node);
}

node *PRTboolconst(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTboolconst");

	printf("%s", BOOLCONST_VALUE(arg_node) ? "true" : "false");

	DBUG_RETURN(arg_node);
}


/**************************************************************************
 *
 * @fn PRTerror
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *PRTerror(node* arg_node, info* arg_info) {
	bool first_error;

	DBUG_ENTER ("PRTerror");

	if(NODE_ERROR(arg_node) != NULL) {
		NODE_ERROR(arg_node) = TRAVdo(NODE_ERROR(arg_node), arg_info);
	}

	first_error = INFO_FIRSTERROR(arg_info);

	if((global.outfile != NULL) && (ERROR_ANYPHASE( arg_node) == global.compiler_anyphase)) {
		if(first_error) {
			printf("\n/******* BEGIN TREE CORRUPTION ********\n");
			INFO_FIRSTERROR( arg_info) = FALSE;
		}

		printf("%s\n", ERROR_MESSAGE( arg_node));

		if(ERROR_NEXT(arg_node) != NULL) {
			TRAVopt(ERROR_NEXT(arg_node), arg_info);
		}

		if(first_error) {
			printf("********  END TREE CORRUPTION  *******/\n");
			INFO_FIRSTERROR(arg_info) = TRUE;
		}
	}

	DBUG_RETURN(arg_node);
}

/*****************************************************************************
 * @brief Prints the given syntaxtree
 *
 * @param syntaxtree a node structure
 *
 * @return the unchanged nodestructure
 ******************************************************************************/

node *PRTdoPrint( node *syntaxtree)
{
  info *info;

  DBUG_ENTER("PRTdoPrint");

  DBUG_ASSERT( (syntaxtree!= NULL), "PRTdoPrint called with empty syntaxtree");

  if (myglobal.print_ast) {
      printf("\n\n/* CiviC compiler output by Nico Tromp & Olaf van Houten */\n\n");

      info = MakeInfo();

      TRAVpush( TR_prt);

      syntaxtree = TRAVdo( syntaxtree, info);

      TRAVpop();

      info = FreeInfo(info);

      printf("\n//That's all folks....\n\n");
  }

  DBUG_RETURN( syntaxtree);
}

node *PRTarrayassign(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTarrayassign");

	DBUG_RETURN(arg_node);
}

node *PRTarrdata(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTarrdata");

	DBUG_RETURN(arg_node);
}


node *PRTarray(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTarray");

	DBUG_RETURN(arg_node);
}

node *PRTids(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTids");

	DBUG_RETURN(arg_node);
}

node *PRTarrexprs(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTarrexprs");

	DBUG_RETURN(arg_node);
}

node *PRTlocalfundefs(node * arg_node, info * arg_info)
{
	DBUG_ENTER("PRTlocalfundefs");

	DBUG_RETURN(arg_node);
}
