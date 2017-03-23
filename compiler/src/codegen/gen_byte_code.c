#include "gen_byte_code.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"


/*
 * Traversal start function
 */

node *GBCdoGenByteCode( node *syntaxtree)
{
  DBUG_ENTER("GBCdoGenByteCode");

  printf("Starting the assembler generation...\n\n");

  printf("; The assembler code should appear here...\n");

  printf("\n\nAssembler generation done.\n");

  DBUG_RETURN( syntaxtree);
}
