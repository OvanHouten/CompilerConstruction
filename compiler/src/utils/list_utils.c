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
