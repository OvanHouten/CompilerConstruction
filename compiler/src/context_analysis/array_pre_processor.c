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
	node* externScope;
	node* funheaderParams;
};

/*
 * INFO macros
 */
#define INFO_DIMSIDS(n) ((n)->dimsids)
#define INFO_CURSCOPE(n) ((n)->cur_scope)
#define INFO_DIMDECLS(n) ((n)->dimDecls)
#define INFO_EXTERNSCOPE(n) ((n)->externScope)
#define INFO_FUNHEADERPARAMS(n) ((n)->funheaderParams)

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
	INFO_EXTERNSCOPE(result) = NULL;
	INFO_FUNHEADERPARAMS(result) = NULL;
	
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

node* PPAprogram(node *arg_node, info *arg_info) {
    DBUG_ENTER("PPAprogram");

	// Start new scope: change curscope
	INFO_CURSCOPE(arg_info) = PROGRAM_DECLARATIONS(arg_node);
	INFO_EXTERNSCOPE(arg_info) = PROGRAM_DECLARATIONS(arg_node);
	
    TRAVopt(PROGRAM_DECLARATIONS(arg_node), arg_info);
    
    DBUG_RETURN(arg_node);
}

node* PPAdeclarations(node* arg_node, info* arg_info) {
	DBUG_ENTER("PPAdeclarations");
	
	// First declarations have to be processed first, default is other way around it seems
	TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);
	TRAVdo(DECLARATIONS_DECLARATION(arg_node), arg_info);
	
	DBUG_RETURN(arg_node);
}

node* PPAfunheader(node* arg_node, info* arg_info) {
	DBUG_ENTER("PPAfunheader");
	
	INFO_FUNHEADERPARAMS(arg_info) = FUNHEADER_PARAMS(arg_node);
	TRAVopt(FUNHEADER_PARAMS(arg_node), arg_info);
	INFO_FUNHEADERPARAMS(arg_info) = NULL;
	
	DBUG_RETURN(arg_node);
}

node* PPAvardef(node* arg_node, info* arg_info) {
    DBUG_ENTER("PPAvardef");
	
	// Vardef is an external array, split dims variables in separate external declaration
	if(VARDEF_EXTERN(arg_node) && VARDEF_SIZEIDS(arg_node)) {		
		INFO_DIMSIDS(arg_info) = TRUE;
		INFO_DIMDECLS(arg_info) = NULL;
		
		// Create Declarations nodes for the dimension variables of the array
		TRAVdo(VARDEF_SIZEIDS(arg_node), arg_info);
		
		// If a list of declarations has been created, put it before the array declaration
		if(INFO_DIMDECLS(arg_info)) {
			node* temp = INFO_EXTERNSCOPE(arg_info);
			
			// Find the current vardef node
			while(temp) {
				if(NODE_TYPE(DECLARATIONS_DECLARATION(temp)) == N_vardef) {
					if(DECLARATIONS_DECLARATION(temp) == arg_node) {
						node* tail = DECLARATIONS_NEXT(temp);
						DECLARATIONS_NEXT(temp) = INFO_DIMDECLS(arg_info);
						
						// put the tail behind the new list of decls
						node* end_decls = INFO_DIMDECLS(arg_info);
						while(DECLARATIONS_NEXT(end_decls)) {
							end_decls = DECLARATIONS_NEXT(end_decls);
						}
						DECLARATIONS_NEXT(end_decls) = tail;
					}
				}
				
				temp = DECLARATIONS_NEXT(temp);
			}
		}
		
		INFO_DIMSIDS(arg_info) = FALSE;
	}
	// Vardef is a parameter in a funheader.
	else if(INFO_FUNHEADERPARAMS(arg_info) && VARDEF_SIZEIDS(arg_node)) {
		DBUG_PRINT("PPA", ("PARAM %s", VARDEF_NAME(arg_node)));
		
		INFO_DIMDECLS(arg_info) = NULL;
		
		// Create Declarations nodes for the dimension variables of the array
		TRAVdo(VARDEF_SIZEIDS(arg_node), arg_info);
		
		
	}
	
    DBUG_RETURN(arg_node);
}

node* PPAid(node* arg_node, info* arg_info) {
	DBUG_ENTER("PPAid");
	
	if(INFO_DIMSIDS(arg_info)) {
		// We dont want any duplicate declarations eg arr[n] and arr2[n, m]
		node* temp = INFO_EXTERNSCOPE(arg_info);
		while(temp) {
			if(NODE_TYPE(DECLARATIONS_DECLARATION(temp)) == N_vardef) {
				if(strcmp(VARDEF_NAME(DECLARATIONS_DECLARATION(temp)), ID_NAME(arg_node)) == 0) {
					DBUG_RETURN(arg_node);
				}
			}
			
			temp = DECLARATIONS_NEXT(temp);
		}
		
		node* new_node = TBmakeVardef(TRUE, FALSE, STRcpy(ID_NAME(arg_node)), TY_int, NULL, NULL, NULL, NULL);
		INFO_DIMDECLS(arg_info) = TBmakeDeclarations(new_node, INFO_DIMDECLS(arg_info));
	}
	else if(INFO_FUNHEADERPARAMS(arg_info)) {
		DBUG_PRINT("PPA", ("PARAMTEST!"));
		
		node* new_node = TBmakeVardef(FALSE, FALSE, STRcpy(ID_NAME(arg_node)), TY_int, NULL, NULL, NULL, NULL);
		INFO_DIMDECLS(arg_info) = TBmakeIds(new_node, INFO_DIMDECLS(arg_info));
	}
	
	DBUG_RETURN(arg_node);
}

node* PPAdoPreProcessorArray(node *syntaxtree) {
    DBUG_ENTER("PPAdoScopeAnalysis");

    info *arg_info = MakeInfo();

    TRAVpush(TR_ppa);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}

