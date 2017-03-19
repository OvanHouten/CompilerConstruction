/*
 * list_utils.c
 *
 *  Created on: 15 Mar 2017
 *      Author: nico
 */

#include "dbug.h"
#include "tree_basic.h"
#include "node_basic.h"

#include "list_utils.h"

/**
 * Appends a list of new statements to the end of the existing list of statements in a functionbody.
 */
node *appendToStatements(node *funBody, node* newStatements) {
    DBUG_ENTER("appendToStatements");

    if (FUNBODY_STATEMENTS(funBody)) {
        DBUG_PRINT("UTIL", ("Appending to the end of the existing list."));
        node *statements = FUNBODY_STATEMENTS(funBody);
        while (STATEMENTS_NEXT(statements)) {
            statements = STATEMENTS_NEXT(statements);
        }
        STATEMENTS_NEXT(statements) = newStatements;
    } else {
        DBUG_PRINT("UTIL", ("Setting the statements."));
        FUNBODY_STATEMENTS(funBody) = newStatements;
    }

    DBUG_RETURN(funBody);
}

node *appendToSymbolTableEntries(node *symbolTable, node *symbolTableEntry) {
    DBUG_ENTER("appendToSymbolTableEntries");

    if (SYMBOLTABLE_SYMBOLTABLEENTRY(symbolTable)) {
        DBUG_PRINT("UTIL", ("Appending to the end of the existing list."));
        node *entries = SYMBOLTABLE_SYMBOLTABLEENTRY(symbolTable);
        while (SYMBOLTABLEENTRY_NEXT(entries)) {
            entries = SYMBOLTABLEENTRY_NEXT(entries);
        }
        SYMBOLTABLEENTRY_NEXT(entries) = symbolTableEntry;
    } else {
        DBUG_PRINT("UTIL", ("Setting the entries."));
        SYMBOLTABLE_SYMBOLTABLEENTRY(symbolTable) = symbolTableEntry;
    }

    DBUG_RETURN(symbolTable);
}

