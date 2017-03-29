#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"
#include "ctinfo.h"
#include "myglobals.h"
#include "mytypes.h"
#include "type_utils.h"
#include "free_node.h"
#include "copy.h"

#include "short_circuit.h"

/*
 * INFO structure
 */
struct INFO {
	int non_empty;
};

/*
 * INFO macros
 */
#define INFO_NON_EMPTY(n) ((n)->non_empty)

/*
 * INFO functions
 */
static info *MakeInfo(void) {
	info* result;

	DBUG_ENTER("MakeInfo");

	result = (info*)MEMmalloc(sizeof(info));

	INFO_NON_EMPTY(result) = 0;

	DBUG_RETURN(result);
}

static info* FreeInfo(info* info) {
	DBUG_ENTER("FreeInfo");

	info = MEMfree(info);

	DBUG_RETURN(info);
}

node* SCBEbinop(node* arg_node, info* arg_info) {
    DBUG_ENTER("SCBEbinop");

    // First try to optimize the operands, who knows...
    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);
	
	if(BINOP_OP(arg_node) == BO_and) {
		// Create new Ternop node
		node* new_node = TBmakeTernop(BINOP_LEFT(arg_node), BINOP_RIGHT(arg_node), TBmakeBoolconst(TY_bool, FALSE));
		TERNOP_TYPE(new_node) = BINOP_TYPE(arg_node);
		
		// Free old binop node
		BINOP_LEFT(arg_node) = NULL;
		BINOP_RIGHT(arg_node) = NULL;
		arg_node = FREEbinop(arg_node, arg_info);
		
		arg_node = new_node;
	}
	else if(BINOP_OP(arg_node) == BO_or) {
		// Create new Ternop node
		node* new_node = TBmakeTernop(BINOP_LEFT(arg_node), TBmakeBoolconst(TY_bool, TRUE), BINOP_RIGHT(arg_node));
        TERNOP_TYPE(new_node) = BINOP_TYPE(arg_node);
		
		// Free old binop node
		BINOP_LEFT(arg_node) = NULL;
		BINOP_RIGHT(arg_node) = NULL;
		arg_node = FREEbinop(arg_node, arg_info);

		arg_node = new_node;
	}
	
    DBUG_RETURN(arg_node);
}

node* SCBEtypecast(node* arg_node, info* arg_info) {
	DBUG_ENTER("SCBEtypecast");

	// First try to optimize the expression, who knows...
	TYPECAST_EXPR(arg_node) = TRAVdo(TYPECAST_EXPR(arg_node), arg_info);
	
	if (TYPECAST_TYPE(arg_node) != determineType(TYPECAST_EXPR(arg_node))) {
        DBUG_PRINT("SCBE", ("Typecast found for %p.", arg_node));
        if (TYPECAST_TYPE(arg_node) == TY_bool) {
            DBUG_PRINT("SCBE", ("Potential cast from int/float to bool for node %p.", arg_node));
            // A numeric value is casted to a boolean value
            // Get the numeric false value according to the actual datatype
            node* numericFalseValue = NULL;
            if(determineType(TYPECAST_EXPR(arg_node)) == TY_int) {
                numericFalseValue = TBmakeIntconst(TY_int, 0);
            } else if (determineType(TYPECAST_EXPR(arg_node)) == TY_float) {
                numericFalseValue = TBmakeFloatconst(TY_float, 0.0);
            }

            // A possible typecast
            if(numericFalseValue) {
                DBUG_PRINT("SCBE", ("Actual cast from int/float to bool"));
                // Create new ternary operator that evaluates to either true or false
                node* comparison = TBmakeBinop(BO_ne, TYPECAST_EXPR(arg_node), numericFalseValue);
                BINOP_TYPE(comparison) = TY_bool;
                node* castOperation = TBmakeTernop(comparison, TBmakeBoolconst(TY_bool, TRUE), TBmakeBoolconst(TY_bool, FALSE));
                TERNOP_TYPE(castOperation) = TY_bool;

                // Free old typecast node
                TYPECAST_EXPR(arg_node) = NULL;
                arg_node = FREEtypecast(arg_node, arg_info);

                arg_node = castOperation;
                DBUG_PRINT("SCBE", ("Cast has been converted"));
            } else {
                DBUG_PRINT("SCBE", ("No actual cast to bool"));
            }
        } else if (determineType(TYPECAST_EXPR(arg_node)) == TY_bool) {
            DBUG_PRINT("SCBE", ("Potential cast from bool to int/float for node %p.", arg_node));
            // If the cast is supported create a ternary operation for the target type
            node *castOperation = NULL;
            if (TYPECAST_TYPE(arg_node) == TY_int) {
                castOperation = TBmakeTernop(TYPECAST_EXPR(arg_node), TBmakeIntconst(TY_int, 1), TBmakeIntconst(TY_int, 0));
                TERNOP_TYPE(castOperation) = TY_int;
            } else if (TYPECAST_TYPE(arg_node) == TY_float) {
                castOperation = TBmakeTernop(TYPECAST_EXPR(arg_node), TBmakeFloatconst(TY_float, 1.0), TBmakeFloatconst(TY_float, 0.0));
                TERNOP_TYPE(castOperation) = TY_float;
            }

            // A possible typecast
            if (castOperation) {
                DBUG_PRINT("SCBE", ("Actual cast from bool to int/float"));
                // Free old typecast node
                TYPECAST_EXPR(arg_node) = NULL;
                arg_node = FREEtypecast(arg_node, arg_info);

                arg_node = castOperation;
                DBUG_PRINT("SCBE", ("Cast has been converted"));
            } else {
                DBUG_PRINT("SCBE", ("No actual cast to int/float"));
            }
        }
	} else {
	    DBUG_PRINT("SCBE", ("Found a unneeded cast, leave it for the optimizations to handle it."));
	}
	
	DBUG_RETURN(arg_node);
}

node* SCBEdoShortCircuit(node* syntaxtree) {
    DBUG_ENTER("SCBEdoShortCircuit");

    info *arg_info = MakeInfo();

    TRAVpush(TR_scbe);

    TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
