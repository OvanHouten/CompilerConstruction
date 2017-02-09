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
		  INFO_ADD( arg_info)++;
	  	  break;
	  case BO_sub :
		  INFO_SUB( arg_info)++;
		  break;
	  case BO_mul :
		  INFO_MUL( arg_info)++;
		  break;
	  case BO_div :
		  INFO_DIV(arg_info)++;
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

	  // There must be another way to accomplish this!!!!
	  syntaxtree->attribs.N_binopinfo = TBmakeBinopinfo(0, 0 ,0, 0);
	  // Copy the information into the syntaxtree
	  BINOPINFO_ADD(syntaxtree) = INFO_ADD(arg_info);
	  BINOPINFO_SUB(syntaxtree) = INFO_SUB(arg_info);
	  BINOPINFO_MUL(syntaxtree) = INFO_MUL(arg_info);
	  BINOPINFO_DIV(syntaxtree) = INFO_DIV(arg_info);

	  arg_info = FreeInfo( arg_info);

	  DBUG_RETURN( syntaxtree);
}

