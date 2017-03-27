/*
 * scope_utils.c
 *
 *  Created on: 26 Mar 2017
 *      Author: nico
 */

#include "str.h"
#include "types.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "ctinfo.h"
#include "math.h"
#include "type_utils.h"

node* findWithinScope(node *symbolTable, char* name, ste_type type) {
    DBUG_ENTER("findDefWithinScope");
    node* varDefSTE = SYMBOLTABLE_SYMBOLTABLEENTRY(symbolTable);
    while (varDefSTE) {
        if (SYMBOLTABLEENTRY_ENTRYTYPE(varDefSTE) == type && STReq(name, SYMBOLTABLEENTRY_NAME(varDefSTE))) {
            break;
        }
        varDefSTE = SYMBOLTABLEENTRY_NEXT(varDefSTE);
    }
    DBUG_RETURN(varDefSTE);
}

node* findInAnyScope(node* symbolTable, char *name, int* distance, ste_type type) {
    DBUG_ENTER("findInAnyScope");

    DBUG_PRINT("SA", ("Trying to locate variable [%s] in the symbol table.", name));
    // Used for traversing to outer ST/scopes
    node* varDefSTE = SYMBOLTABLE_SYMBOLTABLEENTRY(symbolTable);
    while (varDefSTE || symbolTable) {
        if (varDefSTE) {
            if (SYMBOLTABLEENTRY_ENTRYTYPE(varDefSTE) == type && STReq(name, SYMBOLTABLEENTRY_NAME(varDefSTE))) {
                DBUG_PRINT("SA", ("Found [%s] at distance [%d] offset [%d].", name, *distance, SYMBOLTABLEENTRY_OFFSET(varDefSTE)));
                break;
            }
            DBUG_PRINT("SA", ("Trying next entry."));
            varDefSTE = SYMBOLTABLEENTRY_NEXT(varDefSTE);
        } else {
            DBUG_PRINT("SA", ("Trying outer scope for [%s].", name));
            (*distance)++;
            symbolTable = SYMBOLTABLE_PARENT(symbolTable);
            if (symbolTable) {
                varDefSTE = SYMBOLTABLE_SYMBOLTABLEENTRY(symbolTable);
            }
        }
    }
    DBUG_RETURN(varDefSTE);
}

bool isUniqueInSymbolTable(node *symbolTable, char *name, ste_type type) {
    node *symbolTableEntry = SYMBOLTABLE_SYMBOLTABLEENTRY(symbolTable);
    while (symbolTableEntry) {
        if (SYMBOLTABLEENTRY_ENTRYTYPE(symbolTableEntry) == type && STReq(name, SYMBOLTABLEENTRY_NAME(symbolTableEntry))) {
            return FALSE;
        }
        symbolTableEntry = SYMBOLTABLEENTRY_NEXT(symbolTableEntry);
    }
    return TRUE;
}

char *createUniqueNameForSymbolTable(node *symbolTable, char *name, ste_type type) {
    DBUG_ENTER("createUniqueNameForSymbolTable");
    int duplicates = 0;
    char *newName = NULL;
    do {
        int numberOfDigits = duplicates == 0 ? 1 : 1 + log10(duplicates);
        // 3 is the number of character in our pattern + room for the trailing zero
        newName = MEMmalloc(3 + STRlen(name) + numberOfDigits);
        sprintf(newName, "_%s_%d", name, duplicates);
        if (!isUniqueInSymbolTable(symbolTable, newName, type)) {
            duplicates++;
            newName = MEMfree(newName);
        }
    } while (newName == NULL);
    DBUG_RETURN(newName);
}

node *registerWithinCurrentScope(node* symbolTable, node* arg_node, char* name, ste_type entryType, type dataType) {
    DBUG_ENTER("registerWithinCurrentScope");
    DBUG_PRINT("SA", ("Creating a new Symbol Table Entry"));
    // Add the def to the ST
    node* defSTE = TBmakeSymboltableentry(SYMBOLTABLE_SYMBOLTABLEENTRY(symbolTable));
    SYMBOLTABLEENTRY_ENTRYTYPE(defSTE) = entryType;
    SYMBOLTABLEENTRY_NAME(defSTE) = STRcpy(name);
    SYMBOLTABLEENTRY_TYPE(defSTE) = dataType;
    NODE_LINE(defSTE) = NODE_LINE(arg_node);
    NODE_COL(defSTE) = NODE_COL(arg_node);

    SYMBOLTABLE_SYMBOLTABLEENTRY(symbolTable) = defSTE;

    DBUG_PRINT("SA", ("Registered."));

    DBUG_RETURN(defSTE);
}

