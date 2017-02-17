
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
#define INDENT_SIZE 4
#define INDENT(arg_info) INFO_INDENT(arg_info) += INDENT_SIZE; INFO_IS_NEW_LINE(arg_info) = TRUE;
#define UNINDENT(arg_info) INFO_INDENT(arg_info) -= INDENT_SIZE; INFO_IS_NEW_LINE(arg_info) = TRUE;
#define PRINT_INDENT(arg_info) printf("%*s", INFO_IS_NEW_LINE(arg_info) ? INFO_INDENT(arg_info) : 0, ""); INFO_IS_NEW_LINE(arg_info) = FALSE;

static info *MakeInfo()
{
  info *result;
  
  result = MEMmalloc(sizeof(info));

  INFO_FIRSTERROR(result) = FALSE;
  INFO_INDENT(result) = 0;
  

  return result;
}


static info *FreeInfo( info *info)
{
  info = MEMfree( info);

  return info;
}

node *PRTdeclaration(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTdeclaration");

	DBUG_RETURN(arg_node);
}

node *PRTfundec(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTfunDec");

	DBUG_RETURN(arg_node);
}
node *PRTfundef(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTfunDef");

	DBUG_RETURN(arg_node);
}
node *PRTglobaldec(node * arg_node, info * arg_info) {
	DBUG_ENTER("PRTglobalDec");

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

  STMTS_STMT( arg_node) = TRAVdo( STMTS_STMT( arg_node), arg_info);
  
  STMTS_NEXT( arg_node) = TRAVopt( STMTS_NEXT( arg_node), arg_info);
  
  DBUG_RETURN (arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn PRTassign
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
PRTassign (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTassign");

  if (ASSIGN_LET( arg_node) != NULL) {
    ASSIGN_LET( arg_node) = TRAVdo( ASSIGN_LET( arg_node), arg_info);
    printf( " = ");
  }
  
  ASSIGN_EXPR( arg_node) = TRAVdo( ASSIGN_EXPR( arg_node), arg_info);
  
  printf( ";\n");
  INFO_IS_NEW_LINE(arg_info) = TRUE;
  
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

  PRINT_INDENT(arg_info);
  printf("if (");
  IF_CONDITION( arg_node) = TRAVdo( IF_CONDITION( arg_node), arg_info);
  printf(") {\n");
  INDENT(arg_info);

  IF_IFBLOCK( arg_node) = TRAVdo( IF_IFBLOCK( arg_node), arg_info);

  if (IF_ELSEBLOCK( arg_node) != NULL) {
	  UNINDENT(arg_info);
	  PRINT_INDENT(arg_info);
	  printf("} else {\n");
	  INDENT(arg_info);
	  IF_ELSEBLOCK( arg_node) = TRAVdo(IF_ELSEBLOCK( arg_node), arg_info);
  }

  UNINDENT(arg_info);
  PRINT_INDENT(arg_info);
  printf( "}\n");
  INFO_IS_NEW_LINE(arg_info) = TRUE;

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

  PRINT_INDENT(arg_info);
  printf("while (");
  WHILE_CONDITION( arg_node) = TRAVdo( WHILE_CONDITION( arg_node), arg_info);
  printf(") {\n");
  INDENT(arg_info);

  WHILE_BLOCK( arg_node) = TRAVdo( WHILE_BLOCK( arg_node), arg_info);

  UNINDENT(arg_info);
  PRINT_INDENT(arg_info);
  printf( "}\n");

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

  PRINT_INDENT(arg_info);
  printf("do {\n");
  INDENT(arg_info);

  WHILE_BLOCK( arg_node) = TRAVdo( WHILE_BLOCK( arg_node), arg_info);

  UNINDENT(arg_info);
  PRINT_INDENT(arg_info);
  printf("} while (");

  WHILE_CONDITION( arg_node) = TRAVdo( WHILE_CONDITION( arg_node), arg_info);

  printf( ");\n");
  INFO_IS_NEW_LINE(arg_info) = TRUE;

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
  char *tmp;

  DBUG_ENTER ("PRTbinop");

  printf( "( ");

  BINOP_LEFT( arg_node) = TRAVdo( BINOP_LEFT( arg_node), arg_info);

  switch (BINOP_OP( arg_node)) {
    case BO_add:
      tmp = "+";
      break;
    case BO_sub:
      tmp = "-";
      break;
    case BO_mul:
      tmp = "*";
      break;
    case BO_div:
      tmp = "/";
      break;
    case BO_mod:
      tmp = "%";
      break;
    case BO_lt:
      tmp = "<";
      break;
    case BO_le:
      tmp = "<=";
      break;
    case BO_gt:
      tmp = ">";
      break;
    case BO_ge:
      tmp = ">=";
      break;
    case BO_eq:
      tmp = "==";
      break;
    case BO_ne:
      tmp = "!=";
      break;
    case BO_or:
      tmp = "||";
      break;
    case BO_and:
      tmp = "&&";
      break;
    case BO_unknown:
      DBUG_ASSERT( 0, "unknown binop detected!");
  }

  printf( " %s ", tmp);

  BINOP_RIGHT( arg_node) = TRAVdo( BINOP_RIGHT( arg_node), arg_info);

  printf( ")");

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

  printf( "(%s(", tmp);

  UNOP_RIGHT( arg_node) = TRAVdo( UNOP_RIGHT( arg_node), arg_info);

  printf( "))");

  DBUG_RETURN (arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn PRTfloat
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node Float node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTfloat (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTfloat");

  printf( "%f", FLOAT_VALUE( arg_node));

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

  printf( "%i", NUM_VALUE( arg_node));

  DBUG_RETURN (arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn PRTboolean
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node Boolean node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTbool (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTbool");

  if (BOOL_VALUE( arg_node)) {
    printf( "true");
  }
  else {
    printf( "false");
  }
  
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

  PRINT_INDENT(arg_info);
  printf( "%s", VAR_NAME( arg_node));

  DBUG_RETURN (arg_node);
}


/** <!--******************************************************************-->
 *
 * @fn PRTvarlet
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
PRTvarlet (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PRTvarlet");

  PRINT_INDENT(arg_info);
  printf( "%s", VARLET_NAME( arg_node));

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

/**
 * @}
 */
