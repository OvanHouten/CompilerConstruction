
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

void printSymbolTable(node* STE_node) {	
	while(STE_node) {
		printf("//%s %s %d %d\n", SYMBOLTABLEENTRY_NAME(STE_node), SYMBOLTABLEENTRY_TYPE(STE_node), SYMBOLTABLEENTRY_DISTANCE(STE_node), SYMBOLTABLEENTRY_OFFSET(STE_node));
		STE_node = SYMBOLTABLEENTRY_NEXT(STE_node);
	}
}

void printSymbolTableEntry(node* arg_node) {
	char* type;
	
	switch(ID_TYPE(arg_node)) {
		case TY_bool:
			type = "bool";
			break;
		case TY_int:
			type = "int";
			break;
		case TY_float:
			type = "float";
			break;
		case TY_unknown:
			type = "unknown";
		default:
			break;
	}
	
	printf("/* %s %s %d %d */", ID_NAME(arg_node), type, ID_DISTANCE(arg_node), ID_OFFSET(arg_node));
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
	
	TRAVopt(SYMBOLTABLE_SYMBOLTABLEENTRY(arg_node), arg_info);
	
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
	
	printf("//%s %s %d %d\n", SYMBOLTABLEENTRY_NAME(arg_node), SYMBOLTABLEENTRY_TYPE(arg_node), SYMBOLTABLEENTRY_DISTANCE(arg_node), SYMBOLTABLEENTRY_OFFSET(arg_node));
	
	DBUG_RETURN (arg_node);
}

node *PRTdeclarations(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTdeclarations");

	TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);
	TRAVopt(DECLARATIONS_DECLARATION(arg_node), arg_info);

	DBUG_RETURN(arg_node);
}

node *PRTfunheader(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTfunheader");

	TRAVdo(FUNHEADER_RETTYPE(arg_node), arg_info);
	TRAVdo(FUNHEADER_ID(arg_node), arg_info);
	printf("(");
	TRAVopt(FUNHEADER_PARAMS(arg_node), arg_info);
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

    if (VARDEF_EXTERN(arg_node)) {
        printf("extern ");
    }
    if (VARDEF_EXPORT(arg_node)) {
        printf("export ");
    }
    TRAVdo(VARDEF_TYPE(arg_node), arg_info);
    TRAVdo(VARDEF_ID(arg_node), arg_info);
    if (VARDEF_EXPR(arg_node)) {
        printf(" = ");
        TRAVdo(VARDEF_EXPR(arg_node), arg_info);
    }
    if(VARDEF_ISDECLARATION(arg_node)) {
        printf(";\n");
        INDENT_AT_NEWLINE(arg_info);
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

node *PRTtypecast(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTtypecast");

	printf("( ");
	TRAVdo(TYPECAST_TYPE(arg_node), arg_info);
	printf(") ");
	TRAVdo(TYPECAST_EXPR(arg_node), arg_info);

	DBUG_RETURN(arg_node);
}


node *PRTfuncall(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTfuncall");

	TRAVdo(FUNCALL_ID(arg_node), arg_info);
	printf("(");
	TRAVopt(FUNCALL_PARAMS(arg_node), arg_info);
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
      TRAVdo(VARDEF_ID(FOR_VARDEF(arg_node)), arg_info);
      if (VARDEF_EXPR(FOR_VARDEF(arg_node))) {
          printf(" = ");
          TRAVdo(VARDEF_EXPR(FOR_VARDEF(arg_node)), arg_info);
      }
  }
  printf(", ");
  TRAVdo( FOR_FINISH(arg_node), arg_info);
  if (FOR_STEP(arg_node)) {
      printf(", ");
      TRAVdo(FOR_STEP(arg_node), arg_info);
  }
  printf(" ) {\n");
  INCREASE_INDENTATION(arg_info);

  TRAVdo(FOR_BLOCK( arg_node), arg_info);

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

node *PRTrelop (node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTrelop");

	printf("(");
	TRAVdo(RELOP_LEFT(arg_node), arg_info);

	char* op;
	switch(RELOP_OP(arg_node)) {
	case RO_lt:
		op = "<";
		break;
	case RO_le:
		op = "<=";
		break;
	case RO_eq:
		op = "==";
		break;
	case RO_ne:
		op = "!=";
		break;
	case RO_ge:
		op = ">=";
		break;
	case RO_gt:
		op = ">";
		break;
	default:
		op = "<<UNKNOWN>>";
	}
	printf(" %s ", op);

	TRAVdo(RELOP_RIGHT(arg_node), arg_info);
	printf(")");

	DBUG_RETURN(arg_node);
}

node *PRTarithop (node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTarithop");

	printf("(");
	TRAVdo(ARITHOP_LEFT(arg_node), arg_info);

	char* op;
	switch(ARITHOP_OP(arg_node)) {
	case AO_mul:
		op = "*";
		break;
	case AO_div:
		op = "/";
		break;
	case AO_add:
		op = "+";
		break;
	case AO_sub:
		op = "-";
		break;
	case AO_mod:
		op = "%";
		break;
	default:
		op = "<<UNKNOWN>>";
	}
	printf(" %s ", op);

	TRAVdo(ARITHOP_RIGHT(arg_node), arg_info);
	printf(")");

	DBUG_RETURN(arg_node);
}

node *PRTlogicop (node * arg_node, info * arg_info)
{
	DBUG_ENTER("PRTlogicop");

	printf("(");
	TRAVdo(LOGICOP_LEFT(arg_node), arg_info);

	char* op;
	switch(LOGICOP_OP(arg_node)) {
	case LO_and:
		op = "&&";
		break;
	case LO_or:
		op = "||";
		break;
	default:
		op = "<<UNKNOWN>>";
	}
	printf(" %s ", op);

	TRAVdo(LOGICOP_RIGHT(arg_node), arg_info);
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
  UNOP_RIGHT( arg_node) = TRAVdo( UNOP_RIGHT( arg_node), arg_info);
  printf( ")");

  DBUG_RETURN (arg_node);
}


node *PRTid(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTid");

	INDENT(arg_info);
	// TODO Ugly if-statement, is caused by the fact that ID's are used for functioncalls and variables.
	if (ID_DECL(arg_node) && NODE_TYPE(ID_DECL(arg_node)) == N_vardef) {
        printf("%s", ID_NAME(VARDEF_ID(ID_DECL(arg_node))));
	} else {
	    printf("%s", ID_NAME(arg_node));
	}
	if (myglobal.pst) {
	    // Just handy at the moment to have this possibility while debugging the context checks.
	    printSymbolTableEntry(arg_node);
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

node *PRTint(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTint");

	INDENT(arg_info);
	printf("int ");

	DBUG_RETURN(arg_node);
}

node *PRTfloat (node * arg_node, info * arg_info) {
  DBUG_ENTER ("PRTfloat");

  INDENT(arg_info);
  printf( "float ");

  DBUG_RETURN (arg_node);
}

node *PRTbool (node * arg_node, info * arg_info) {
  DBUG_ENTER ("PRTbool");

  INDENT(arg_info);
  printf("bool ");

  DBUG_RETURN (arg_node);
}

node *PRTvoid(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTvoid");

	INDENT(arg_info);
	printf("void ");

	DBUG_RETURN(arg_node);
}

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

node *PRTerror (node * arg_node, info * arg_info)
{
  bool first_error;

  DBUG_ENTER ("PRTerror");

  if (NODE_ERROR (arg_node) != NULL) {
    NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
  }

  first_error = INFO_FIRSTERROR( arg_info);

  if( (global.outfile != NULL)
      && (ERROR_ANYPHASE( arg_node) == global.compiler_anyphase)) {

    if ( first_error) {
      printf ( "\n/******* BEGIN TREE CORRUPTION ********\n");
      INFO_FIRSTERROR( arg_info) = FALSE;
    }

    printf ( "%s\n", ERROR_MESSAGE( arg_node));

    if (ERROR_NEXT (arg_node) != NULL) {
      TRAVopt (ERROR_NEXT (arg_node), arg_info);
    }

    if ( first_error) {
      printf ( "********  END TREE CORRUPTION  *******/\n");
      INFO_FIRSTERROR( arg_info) = TRUE;
    }
  }

  DBUG_RETURN (arg_node);
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

  printf("\n\n/* CiviC compiler output by Nico Tromp & Olaf van Houten */\n\n");

  info = MakeInfo();

  TRAVpush( TR_prt);

  syntaxtree = TRAVdo( syntaxtree, info);

  TRAVpop();

  info = FreeInfo(info);

  printf("\n//That's all folks....\n\n");

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

node *PRTlocalfundef(node * arg_node, info * arg_info)
{
	DBUG_ENTER("PRTlocalfundef");

	DBUG_RETURN(arg_node);
}

node *PRTlocalfundefs(node * arg_node, info * arg_info)
{
	DBUG_ENTER("PRTlocalfundefs");

	DBUG_RETURN(arg_node);
}
