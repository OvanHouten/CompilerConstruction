/*
 * scope_utils.h
 *
 *  Created on: 26 Mar 2017
 *      Author: nico
 */

#ifndef SRC_UTILS_SCOPE_UTILS_H_
#define SRC_UTILS_SCOPE_UTILS_H_

extern node* findWithinScope(node *symbolTable, char* name, ste_type type);
extern node* findInAnyScope(node* symbolTable, char *name, int* distance, ste_type type);
extern bool isUniqueInSymbolTable(node *symbolTable, char *name, ste_type type);
extern char *createUniqueNameForSymbolTable(node *symbolTable, char *name, ste_type type);
extern node *registerWithinCurrentScope(node* symbolTable, node* arg_node, char* name, ste_type entryType, type returnType);

#endif /* SRC_UTILS_SCOPE_UTILS_H_ */
