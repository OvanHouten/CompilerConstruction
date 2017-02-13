/*
 * indentifier_count.c
 *
 *  Created on: 11 Feb 2017
 *      Author: Nico Tromp
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

/*
 * Node for a linkedlist holding the known variable names.
 */
struct varname_usage {
	char *varName; // By keeping a copy of the variable name we can make use of the LUTfoldLut method later on.
	int usageCount;
};

/*
 * INFO structure
 */
struct INFO {
  lut_t *lookupTable;
};


/*
 * INFO macros
 */

#define INFO_LOOKUP_TABLE(n)  ((n)->lookupTable)

/*
 * INFO functions
 */

static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));

  INFO_LOOKUP_TABLE( result) = LUTgenerateLut();

  DBUG_RETURN( result);
}

void* cleanVarUsageStruct(void *init, void *item) {
	struct varname_usage *varUsage = item;
	varUsage->varName = MEMfree(varUsage->varName);
	return NULL;
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  // We don't fold anything but use it for its iterating capacities
  LUTfoldLutS(INFO_LOOKUP_TABLE(info), NULL, &cleanVarUsageStruct);

  INFO_LOOKUP_TABLE(info) = LUTremoveLut(INFO_LOOKUP_TABLE(info));

  info = MEMfree( info);

  DBUG_RETURN( info);
}

/*
 * Registers the usage of a variable. If the variable is
 * used for the first time 1 is returned otherwise 0
 * is returned.
 */
int registerVarUsage(info *arg_info, char* varName) {
	lut_t *lut = INFO_LOOKUP_TABLE(arg_info);
	void** lutInfo;
	if ((lutInfo = LUTsearchInLutS(lut, varName)) == NULL) {
		// New variable, add to the lookup table
		struct varname_usage *varUsage = MEMmalloc(sizeof(struct varname_usage));
		varUsage->varName = STRcpy(varName);
		varUsage->usageCount = 1;
		LUTinsertIntoLutS(lut, varName, varUsage);
		return 1;
	} else {
		// Known variable, just increase the usage count
		struct varname_usage *varUsage = *lutInfo;
		varUsage->usageCount += 1;
		return 0;
	}
}

void *printVarUsage(void * init, void *item) {
	struct varname_usage *varUsage = item;
	printf("Variable [%s] is referenced %d times.\n", varUsage->varName, varUsage->usageCount);
	return NULL;
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

	  // We don't fold anything but use it for its iterating capacities
	  LUTfoldLutS(INFO_LOOKUP_TABLE(arg_info), NULL, &printVarUsage);

	  arg_info = FreeInfo( arg_info);

	  DBUG_RETURN( syntaxtree);
}
