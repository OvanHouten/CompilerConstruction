/*****************************************************************************
 *
 * Module: rename_identifiers
 *
 * Prefix: RI
 *
 * Description:
 *
 * This module implements a demo traversal of the abstract syntax tree that 
 * prefixes any variable found by two underscores.
 *
 *****************************************************************************/


#include "rename_identifiers.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"


/*
 * Traversal functions
 */

node *RIvardef( node *arg_node, info *arg_info)
{
  char *name;

  DBUG_ENTER("RIvardec");

  DBUG_PRINT( "RI", ("Renaming variable: %s", VARDEF_NAME( arg_node)));

  name = VARDEF_NAME( arg_node);
  VARDEF_NAME( arg_node) = STRcat( "__", name);
  MEMfree(name);

  DBUG_RETURN( arg_node);
}

node *RIid( node *arg_node, info *arg_info)
{
  char *name;

  DBUG_ENTER("RIid");

  DBUG_PRINT( "RI", ("Renaming variable: %s", ID_NAME( arg_node)));

  name = ID_NAME( arg_node);
  ID_NAME( arg_node) = STRcat( "__", name);
  MEMfree(name);

  DBUG_RETURN( arg_node);
}


/*
 * Traversal start function
 */

node *RIdoRenameIdentifiers( node *syntaxtree)
{
  DBUG_ENTER("RIdoRenameIdentifiers");

  TRAVpush( TR_ri);   // Push traversal "ri" as defined in ast.xml

  syntaxtree = TRAVdo( syntaxtree, NULL);   // Initiate ast traversal

  TRAVpop();          // Pop current traversal

  DBUG_RETURN( syntaxtree);
}
