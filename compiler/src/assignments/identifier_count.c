/*
 * indentifier_count.c
 *
 *  Created on: 11 Feb 2017
 *      Author: nico
 */

#include "identifier_count.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"
#include "lookup_table.h"
#include "ctinfo.h"

struct varname_node {
	char *varName;
	struct varname_node *next;
};

/*
 * INFO structure
 */
struct INFO {
  lut_t *lookupTable;
  struct varname_node *varNames;
};


/*
 * INFO macros
 */

#define INFO_LOOKUP_TABLE(n)  ((n)->lookupTable)
#define INFO_VARNAMES(n) ((n)->varNames)

/*
 * INFO functions
 */

static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));

  // Would not be needed if MEMcalloc would exist ;-)
  INFO_LOOKUP_TABLE( result) = LUTgenerateLut();
  INFO_VARNAMES( result) = NULL;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  INFO_LOOKUP_TABLE(info) = LUTremoveLut(INFO_LOOKUP_TABLE(info));
  // Free memeory of linked list with var names

  info = MEMfree( info);

  DBUG_RETURN( info);
}

struct varname_node *addNewVarName(struct varname_node *list, char* varName) {
	struct varname_node *newNode = MEMmalloc(sizeof(struct varname_node));
	newNode->next = list;
	newNode->varName = STRcpy(varName);
	return newNode;
}

/*
 * Registers the usage of a variable. If the variable is
 * used for the first time 1 is returned otherwise 0
 * is returned. The calling code can
 */
int registerVarUsage(info *arg_info, char* varName) {
	lut_t *lut = INFO_LOOKUP_TABLE(arg_info);
	void** lutInfo;
	if ((lutInfo = LUTsearchInLutS(lut, varName)) == NULL) {
		int* varUsage = MEMmalloc(sizeof(int));
		*varUsage = 1;
		LUTinsertIntoLutS(lut, varName, varUsage);
		INFO_VARNAMES(arg_info) = addNewVarName(INFO_VARNAMES(arg_info), varName);
		return 1;
	} else {
		int *varUsage = *lutInfo;
		*varUsage += 1;
		return 0;
	}
}

node *ICvar(node *arg_node, info *arg_info) {
	  DBUG_ENTER( "ICvar");

	  registerVarUsage(arg_info, VAR_NAME(arg_node));

	  DBUG_RETURN( arg_node);
}

node *ICvarlet(node *arg_node, info *arg_info) {
	  DBUG_ENTER( "ICvarlet");

	  registerVarUsage(arg_info, VARLET_NAME(arg_node));

	  DBUG_RETURN( arg_node);
}

node *ICdoIdentifierCount( node *syntaxtree) {
	  info *arg_info;

	  DBUG_ENTER("ICdoIdentifierCount");

	  arg_info = MakeInfo();

	  TRAVpush( TR_ic);
	  syntaxtree = TRAVdo( syntaxtree, arg_info);
	  TRAVpop();

	  // Print info from LUT
	  lut_t *lut = INFO_LOOKUP_TABLE(arg_info);
	  struct varname_node *varNameInfo = INFO_VARNAMES(arg_info);
	  while (varNameInfo) {
		  int* varUsage = *LUTsearchInLutS(lut, varNameInfo->varName);
		  printf("Variable %s is referenced %d times.\n", varNameInfo->varName, *varUsage);
		  varNameInfo = varNameInfo->next;
	  }

	  arg_info = FreeInfo( arg_info);

	  DBUG_RETURN( syntaxtree);
}
