/*
 * indentifier_count.h
 *
 *  Created on: 11 Feb 2017
 *      Author: Nico Tromp
 */

#ifndef SRC_ASSIGNMENTS_IDENTIFIER_COUNT_H_
#define SRC_ASSIGNMENTS_IDENTIFIER_COUNT_H_

#include "types.h"

extern node *ICvar(node *arg_node, info *arg_info);
extern node *ICvarlet(node *arg_node, info *arg_info);
extern node *ICdoIdentifierCount( node *syntaxtree);

#endif /* SRC_ASSIGNMENTS_IDENTIFIER_COUNT_H_ */
