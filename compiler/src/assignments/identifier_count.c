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

void cleanList(struct varname_node *node) {
	if (node) {
		cleanList(node->next);
		node->varName = MEMfree(node->varName);
		MEMfree(node);
	}
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  INFO_LOOKUP_TABLE(info) = LUTremoveLut(INFO_LOOKUP_TABLE(info));

  cleanList(INFO_VARNAMES(info));

  INFO_VARNAMES(info) = NULL;

  info = MEMfree( info);

  DBUG_RETURN( info);
}

/*
 * Adds a new struct varname_node element to the head of the list and
 * returns the new head.
 */
struct varname_node *addNewVarName(struct varname_node *list, char* varName) {
	struct varname_node *newNode = MEMmalloc(sizeof(struct varname_node));
	newNode->varName = STRcpy(varName);
	newNode->next = list;
	return newNode;
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
		int* varUsage = MEMmalloc(sizeof(int));
		*varUsage = 1;
		LUTinsertIntoLutS(lut, varName, varUsage);
		// And add the name to our linked list. (iterator or visitor pattern would make this list superfluous)
		INFO_VARNAMES(arg_info) = addNewVarName(INFO_VARNAMES(arg_info), varName);
		return 1;
	} else {
		// Known variable, just increase the usage count
		int *varUsage = *lutInfo;
		*varUsage += 1;
		return 0;
	}
}

void printVarNameInfo(info* arg_info) {
	lut_t* lut = INFO_LOOKUP_TABLE(arg_info);
	struct varname_node* varNameInfo = INFO_VARNAMES(arg_info);

	// A iterator or visitor pattern would have been nice, now we need to keep
	// track of all the used names. The LUTfold methods only seem to provide the associated value
	// and not the key. So that no use :-(
	while (varNameInfo) {
		int* varUsage = *LUTsearchInLutS(lut, varNameInfo->varName);
		printf("Variable %s is referenced %d times.\n", varNameInfo->varName, *varUsage);
		varNameInfo = varNameInfo->next;
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

	  printVarNameInfo(arg_info);

	  arg_info = FreeInfo( arg_info);

	  DBUG_RETURN( syntaxtree);
}
