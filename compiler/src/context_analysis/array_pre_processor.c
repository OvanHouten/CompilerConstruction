#include "str.h"
#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "ctinfo.h"
#include "type_utils.h"
#include "globals.h"
#include "scope_utils.h"
#include "string.h"

#include "array_pre_processor.h"

/*
 * INFO structure
 */
struct INFO {
	bool dimsids;
	node* cur_scope;
	node* dimDecls;
};

/*
 * INFO macros
 */
#define INFO_DIMSIDS(n) ((n)->dimsids)
#define INFO_CURSCOPE(n) ((n)->cur_scope)
#define INFO_DIMDECLS(n) ((n)->dimDecls)

/*
 * INFO functions
 */
static info *MakeInfo(void) {
	info *result;

	DBUG_ENTER( "MakeInfo");

	result = (info *)MEMmalloc(sizeof(info));
	INFO_DIMSIDS(result) = FALSE;
	INFO_CURSCOPE(result) = NULL;
	INFO_DIMDECLS(result) = NULL;
	
	DBUG_RETURN( result);
}

static info *FreeInfo( info *info) {
	DBUG_ENTER ("FreeInfo");

	info = MEMfree( info);

	DBUG_RETURN( info);
}

// =============================================
// Traversal code starts here
// =============================================

node *PPAprogram(node *arg_node, info *arg_info) {
    DBUG_ENTER("PPAprogram");

	// Start new scope: change curscope
	INFO_CURSCOPE(arg_info) = PROGRAM_DECLARATIONS(arg_node);
	
    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);
    
    DBUG_RETURN(arg_node);
}

node *PPAvardef(node *arg_node, info *arg_info) {
    DBUG_ENTER("PPAvardef");
	
	// Varedef is an external array, split dims variables in separate external declaration
	if(VARDEF_EXTERN(arg_node) && VARDEF_SIZEIDS(arg_node)) {
		DBUG_PRINT("PPA", ("Extern %s", VARDEF_NAME(arg_node)));
		
		INFO_DIMSIDS(arg_info) = TRUE;
		INFO_DIMDECLS(arg_info) = NULL;
		
		// Create Declarations nodes for the dimension variables of the array
		TRAVdo(VARDEF_SIZEIDS(arg_node), arg_info);
		
		node* temp = INFO_DIMDECLS(arg_info);
		if(temp) {
			DBUG_PRINT("PPA", ("DIMSLIST:"));
			while(DECLARATIONS_NEXT(temp)) {
				printf("%s - ", VARDEF_NAME(DECLARATIONS_DECLARATION(temp)));
				temp = DECLARATIONS_NEXT(temp);
			}
			printf("%s\n", VARDEF_NAME(DECLARATIONS_DECLARATION(temp)));
		}
		INFO_DIMSIDS(arg_info) = FALSE;
	}
	
    DBUG_RETURN(arg_node);
}

node* PPAid(node* arg_node, info* arg_info) {
	DBUG_ENTER("PPAid");
	
	if(INFO_DIMSIDS(arg_info)) {
		DBUG_PRINT("PPA", ("id: %s", ID_NAME(arg_node)));
		node* new_node = TBmakeVardef(TRUE, FALSE, STRcpy(ID_NAME(arg_node)), TY_int, NULL, NULL, NULL, NULL);
		INFO_DIMDECLS(arg_info) = TBmakeDeclarations(new_node, INFO_DIMDECLS(arg_info));
	}
	
	DBUG_RETURN(arg_node);
}

node *PPAdoPreProcessorArray(node *syntaxtree) {
    DBUG_ENTER("PPAdoScopeAnalysis");

    info *arg_info = MakeInfo();

    TRAVpush(TR_ppa);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}

