/*
 * rename_nested_functions.h
 *
 *  Created on: 28 Mar 2017
 *      Author: nico
 */

#ifndef SRC_TRANSFORMATIONS_RENAME_NESTED_FUNCTIONS_H_
#define SRC_TRANSFORMATIONS_RENAME_NESTED_FUNCTIONS_H_

#include "types.h"

extern node *RNFfundef(node *arg_node, info *arg_info);
extern node *RNFdoRenameNestedFunctions(node *syntaxtree);

#endif /* SRC_TRANSFORMATIONS_RENAME_NESTED_FUNCTIONS_H_ */
