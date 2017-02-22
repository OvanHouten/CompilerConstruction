
/**
 * @file print.c
 *
 * Functions needed by print traversal.
 *
 */

/**
 * @defgroup print Print Functions.
 *
 * Functions needed by print traversal.
 *
 * @{
 */


#include "print.h"
#include "traverse.h"
#include "tree_basic.h"
#include "dbug.h"
#include "memory.h"
#include "globals.h"


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

node *PRTprogram(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTprogram");

	printf("\n\n/* CiviC program by Nico Tromp & Olaf van Houten */\n\n");

	TRAVdo(PROGRAM_DECLARATION(arg_node), arg_info);

	printf("\n//That's all folks....\n\n");

	DBUG_RETURN(arg_node);
}

node *PRTdeclarations(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTdeclarations");

	TRAVopt(DECLARATIONS_DECLARATION(arg_node), arg_info);
	TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);

	DBUG_RETURN(arg_node);
}

node *PRTfundec(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTfunDec");

	printf("extern ");
	TRAVopt(FUNDEC_FUNHEADER(arg_node), arg_info);
	printf(";\n");

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

node *PRTglobaldec(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTglobalDec");

	printf("extern ");
	TRAVdo(GLOBALDEC_TYPE(arg_node), arg_info);
	TRAVdo(GLOBALDEC_ID(arg_node), arg_info);
	printf(";\n");

	DBUG_RETURN(arg_node);
}

node *PRTfundef(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTfunDef");

	if (FUNDEF_EXPORT(arg_node) == TRUE) {
		printf("export ");
	}
	TRAVdo(FUNDEF_FUNHEADER(arg_node), arg_info);
	printf(" {\n");
	INCREASE_INDENTATION(arg_info);

	TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

	DECREASE_INDENTATION(arg_info);
	printf("}\n");
	INDENT_AT_NEWLINE(arg_info);

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

	TRAVdo(VARDECS_VARDEC(arg_node), arg_info);
	printf(";\n");
	INDENT_AT_NEWLINE(arg_info);
	TRAVopt(VARDECS_NEXT(arg_node), arg_info);

	DBUG_RETURN(arg_node);
}

node *PRTvardec(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTvardec");

	INDENT(arg_info);
	TRAVdo(VARDEC_TYPE(arg_node), arg_info);
	TRAVdo(VARDEC_ID(arg_node), arg_info);

	DBUG_RETURN(arg_node);
}

node *PRTstatements(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTstatements");

	TRAVdo(STATEMENTS_STATEMENT(arg_node), arg_info);
    printf(";\n");
    INDENT_AT_NEWLINE(arg_info);
	TRAVopt(STATEMENTS_NEXT(arg_node), arg_info);

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

node *PRTid(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTid");

	INDENT(arg_info);
	printf(ID_NAME(arg_node));

	DBUG_RETURN(arg_node);
}

// PARAMS

node *PRTparams(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTparams");

	TRAVopt(PARAMS_PARAM(arg_node), arg_info);
	if (PARAMS_NEXT(arg_node) != NULL) {
		printf(", ");
	}
	TRAVopt(PARAMS_NEXT(arg_node), arg_info);

	DBUG_RETURN(arg_node);
}

node *PRTparam(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTparam");

	TRAVdo(PARAM_TYPE(arg_node), arg_info);
	TRAVdo(PARAM_ID(arg_node), arg_info);

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

	printf("%s", INTCONST_VALUE(arg_node) ? "true" : "false");

	DBUG_RETURN(arg_node);
}


// Obsolete ???

node *PRTexpr(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTexpr");

	printf("expr");

	DBUG_RETURN(arg_node);
}


node *PRTstatement(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTstatement");

	DBUG_RETURN(arg_node);
}



node *PRTdeclaration(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTdeclaration");
printf("=============================\n");
	DBUG_RETURN(arg_node);
}


node *PRTrettype(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTrettype");

	printf("return type");

	DBUG_RETURN(arg_node);
}


node *PRTtype(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTtype");

	DBUG_RETURN(arg_node);
}


node *PRTbasictype(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTbasictype");

	DBUG_RETURN(arg_node);
}






node *PRTfuncall(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTfuncall");

	DBUG_RETURN(arg_node);
}


node *PRTtypecast(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTtypecast");

	DBUG_RETURN(arg_node);
}


node *PRTconst(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTconst");

	DBUG_RETURN(arg_node);
}


node *PRTblock(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTblock");

	DBUG_RETURN(arg_node);
}




node *PRTexpressions(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTexpressions");

	DBUG_RETURN(arg_node);
}


node *PRTexprs(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTexpressions");

	DBUG_RETURN(arg_node);
}


node *PRTglobalvardef(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTglobalvardef");

	DBUG_RETURN(arg_node);
}


node *PRTglobalarrdef(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTglobalarrdef");

	DBUG_RETURN(arg_node);
}


node *PRTarrexpr(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTarrexpr");

	DBUG_RETURN(arg_node);
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





node *PRTarithop (node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTarithop");

	DBUG_RETURN(arg_node);
}





node *PRTrelop (node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTrelop");

	DBUG_RETURN(arg_node);
}


node *PRTlogicop (node * arg_node, info * arg_info)
{
	DBUG_ENTER("PRTlogicop");

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






node *PRTglobaldef(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTglobalDef");

	DBUG_RETURN(arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn PRTstmts
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node BinOp node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTstmts (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTstmts");

//  STMTS_STMT( arg_node) = TRAVdo( STMTS_STMT( arg_node), arg_info);
//
//  STMTS_NEXT( arg_node) = TRAVopt( STMTS_NEXT( arg_node), arg_info);
  
  DBUG_RETURN (arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn PRTvardeclares
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node vardeclares node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTvardeclares (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTvardeclares");

//  VARDECLARES_VARDECLARE( arg_node) = TRAVdo( VARDECLARES_VARDECLARE( arg_node), arg_info);
//
//  if (VARDECLARES_NEXT( arg_node) != NULL) {
//	  printf(", ");
//  }
//  VARDECLARES_NEXT( arg_node) = TRAVopt( VARDECLARES_NEXT( arg_node), arg_info);
//
  DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTvardeclare
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node vardeclare node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTvardeclare (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTvardeclare");

//  INDENT(arg_info);
//  char *typeName;
//  switch (VARDECLARE_TYPE(arg_node)) {
//  case TY_int:
//	  typeName = "int ";
//	  break;
//  case TY_float:
//	  typeName = "float ";
//	  break;
//  case TY_bool:
//	  typeName = "bool ";
//	  break;
//  default:
//	  typeName = "FAILURE "; // Need to handle this properly!
//  }
//  printf( "%s%s", typeName, VARDECLARE_NAME( arg_node));

  DBUG_RETURN (arg_node);
}



/** <!--******************************************************************-->
 *
 * @fn PRTif
 *
 * @brief Prints the condition and its sons/attributes
 *
 * @param arg_node If node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTif (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTif");

//  INDENT(arg_info);
//  printf("if (");
//  IF_CONDITION( arg_node) = TRAVdo( IF_CONDITION( arg_node), arg_info);
//  printf(") {\n");
//  INCREASE_INDENTATION(arg_info);
//
//  IF_IFBLOCK( arg_node) = TRAVdo( IF_IFBLOCK( arg_node), arg_info);
//
//  if (IF_ELSEBLOCK( arg_node) != NULL) {
//	  DECREASE_INDENTATION(arg_info);
//	  INDENT(arg_info);
//	  printf("} else {\n");
//	  INCREASE_INDENTATION(arg_info);
//	  IF_ELSEBLOCK( arg_node) = TRAVdo(IF_ELSEBLOCK( arg_node), arg_info);
//  }
//
//  DECREASE_INDENTATION(arg_info);
//  INDENT(arg_info);
//  printf( "}\n");
//  INDENT_AT_NEWLINE(arg_info);

  DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTwhile
 *
 * @brief Prints the while and its sons/attributes
 *
 * @param arg_node while node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTwhile (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTwhile");

//  INDENT(arg_info);
//  printf("while (");
//  WHILE_CONDITION( arg_node) = TRAVdo( WHILE_CONDITION( arg_node), arg_info);
//  printf(") {\n");
//  INCREASE_INDENTATION(arg_info);
//
//  WHILE_BLOCK( arg_node) = TRAVdo( WHILE_BLOCK( arg_node), arg_info);
//
//  DECREASE_INDENTATION(arg_info);
//  INDENT(arg_info);
//  printf( "}\n");

  DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTdowhile
 *
 * @brief Prints the do-while and its sons/attributes
 *
 * @param arg_node while node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTdo (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTdo");

//  INDENT(arg_info);
//  printf("do {\n");
//  INCREASE_INDENTATION(arg_info);
//
//  DO_BLOCK( arg_node) = TRAVdo( DO_BLOCK( arg_node), arg_info);
//
//  DECREASE_INDENTATION(arg_info);
//  INDENT(arg_info);
//  printf("} while (");
//
//  DO_CONDITION( arg_node) = TRAVdo( DO_CONDITION( arg_node), arg_info);
//
//  printf( ");\n");
//  INDENT_AT_NEWLINE(arg_info);
//
  DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfor
 *
 * @brief Prints the for-loop and its sons/attributes
 *
 * @param arg_node for node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTfor (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTfor");

//  INDENT(arg_info);
//  printf("for ( int ");
//  FOR_VAR(arg_node) = TRAVdo( FOR_VAR(arg_node), arg_info);
//  printf(" = ");
//  FOR_START(arg_node) = TRAVdo( FOR_START(arg_node), arg_info);
//  printf(", ");
//  FOR_FINISH(arg_node) = TRAVdo( FOR_FINISH(arg_node), arg_info);
//  printf(" ) {\n");
//  INCREASE_INDENTATION(arg_info);
//
//  FOR_BLOCK( arg_node) = TRAVdo( FOR_BLOCK( arg_node), arg_info);
//
//  DECREASE_INDENTATION(arg_info);
//  INDENT(arg_info);
//  printf("}\n");
//  INDENT_AT_NEWLINE(arg_info);

  DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTbinop
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node BinOp node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node * PRTbinop (node * arg_node, info * arg_info)
{
//  char *tmp;

  DBUG_ENTER ("PRTbinop");

//  printf( "( ");
//
//  BINOP_LEFT( arg_node) = TRAVdo( BINOP_LEFT( arg_node), arg_info);
//
//  switch (BINOP_OP( arg_node)) {
//    case BO_add:
//      tmp = "+";
//      break;
//    case BO_sub:
//      tmp = "-";
//      break;
//    case BO_mul:
//      tmp = "*";
//      break;
//    case BO_div:
//      tmp = "/";
//      break;
//    case BO_mod:
//      tmp = "%";
//      break;
//    case BO_lt:
//      tmp = "<";
//      break;
//    case BO_le:
//      tmp = "<=";
//      break;
//    case BO_gt:
//      tmp = ">";
//      break;
//    case BO_ge:
//      tmp = ">=";
//      break;
//    case BO_eq:
//      tmp = "==";
//      break;
//    case BO_ne:
//      tmp = "!=";
//      break;
//    case BO_or:
//      tmp = "||";
//      break;
//    case BO_and:
//      tmp = "&&";
//      break;
//    case BO_unknown:
//      DBUG_ASSERT( 0, "unknown binop detected!");
//  }
//
//  printf( " %s ", tmp);
//
//  BINOP_RIGHT( arg_node) = TRAVdo( BINOP_RIGHT( arg_node), arg_info);
//
//  printf( ")");

  DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTunop
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node UnOp node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node * PRTunop (node * arg_node, info * arg_info)
{
//  char *tmp;

  DBUG_ENTER ("PRTunop");

//  switch (UNOP_OP( arg_node)) {
//    case UO_neg:
//      tmp = "-";
//      break;
//    case UO_not:
//      tmp = "!";
//      break;
//    case UO_unknown:
//      DBUG_ASSERT( 0, "unknown unop detected!");
//  }
//
//  printf( "(%s(", tmp);
//
//  UNOP_RIGHT( arg_node) = TRAVdo( UNOP_RIGHT( arg_node), arg_info);
//
//  printf( "))");

  DBUG_RETURN (arg_node);
}





/** <!--******************************************************************-->
 *
 * @fn PRTnum
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node Num node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTnum (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTnum");

//  printf( "%i", NUM_VALUE( arg_node));

  DBUG_RETURN (arg_node);
}





/** <!--******************************************************************-->
 *
 * @fn PRTvar
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTvar (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTvar");

//  INDENT(arg_info);
//  printf( "%s", VAR_NAME( arg_node));

  DBUG_RETURN (arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn PRTreturn
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node return node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *PRTreturn (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTreturn");

  DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
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

node *PRTsymboltableentry (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTsymboltableentry");

  DBUG_RETURN (arg_node);
}


/** <!--******************************************************************-->
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

node *
PRTerror (node * arg_node, info * arg_info)
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



/** <!-- ****************************************************************** -->
 * @brief Prints the given syntaxtree
 * 
 * @param syntaxtree a node structure
 * 
 * @return the unchanged nodestructure
 ******************************************************************************/

node 
*PRTdoPrint( node *syntaxtree)
{
  info *info;
  
  DBUG_ENTER("PRTdoPrint");

  DBUG_ASSERT( (syntaxtree!= NULL), "PRTdoPrint called with empty syntaxtree");

//  printf( "\n\n------------------------------\n\n");

  info = MakeInfo();
  
  TRAVpush( TR_prt);

  syntaxtree = TRAVdo( syntaxtree, info);

  TRAVpop();

  info = FreeInfo(info);

//  printf( "\n------------------------------\n\n");

  DBUG_RETURN( syntaxtree);
}

