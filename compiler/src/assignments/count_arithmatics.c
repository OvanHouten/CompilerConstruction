/*
 * count_arithmatics.c
 *
 *  Created on: 9 Feb 2017
 *      Author: Nico Tromp
 */

#include "count_arithmatics.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "ctinfo.h"

/*
 * INFO structure
 */

struct INFO {
  int add;
  int sub;
  int mul;
  int div;
};


/*
 * INFO macros
 */

#define INFO_ADD(n)  ((n)->add)
#define INFO_SUB(n)  ((n)->sub)
#define INFO_MUL(n)  ((n)->mul)
#define INFO_DIV(n)  ((n)->div)

/*
 * INFO functions
 */

static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));

  // Would not be needed if MEMcalloc would exist ;-)
  INFO_ADD( result) = 0;
  INFO_SUB( result) = 0;
  INFO_MUL( result) = 0;
  INFO_DIV( result) = 0;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  info = MEMfree( info);

  DBUG_RETURN( info);
}

/*
 * Node traversal method.
 */
node *CAbinop(node *arg_node, info *arg_info) {
	  DBUG_ENTER( "CAbinop");

	  /*
	   * Extremely important:
	   *  we must continue to traverse the abstract syntax tree !!
	   */
	  BINOP_LEFT( arg_node) = TRAVdo( BINOP_LEFT( arg_node), arg_info);
	  BINOP_RIGHT( arg_node) = TRAVdo( BINOP_RIGHT( arg_node), arg_info);

	  switch (BINOP_OP( arg_node)) {
	  case BO_add :
		  INFO_ADD( arg_info) = INFO_ADD( arg_info) + 1;
	  	  break;
	  case BO_sub :
		  INFO_SUB( arg_info) = INFO_SUB( arg_info) + 1;
		  break;
	  case BO_mul :
		  INFO_MUL( arg_info) = INFO_MUL( arg_info) + 1;
		  break;
	  case BO_div :
		  INFO_DIV(arg_info) = INFO_DIV( arg_info) + 1;
		  break;
	  default:
		  // Ignore others :-)
		  break;
	  }

	  DBUG_RETURN( arg_node);
}

/*
 * Traveral start method.
 */
node *CAdoCountArithmatics( node *syntaxtree) {
	  info *arg_info;

	  DBUG_ENTER("CAdoCountArithmatics");

	  arg_info = MakeInfo();

	  TRAVpush( TR_ca);
	  syntaxtree = TRAVdo( syntaxtree, arg_info);
	  TRAVpop();

	  CTInote( "Number of add operations: %d", INFO_ADD( arg_info));
	  CTInote( "Number of sub operations: %d", INFO_SUB( arg_info));
	  CTInote( "Number of mul operations: %d", INFO_MUL( arg_info));
	  CTInote( "Number of div operations: %d", INFO_DIV( arg_info));

	  arg_info = FreeInfo( arg_info);

	  DBUG_RETURN( syntaxtree);
}

