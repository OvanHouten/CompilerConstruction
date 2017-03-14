/*
 * forloo.h
 *
 *  Created on: 13 Mar 2017
 *      Author: nico
 */

#ifndef SRC_TRANSFORMATIONS_FORLOOP_H_
#define SRC_TRANSFORMATIONS_FORLOOP_H_

#include "types.h"

extern node *FLfor(node *arg_node, info *arg_info);
extern node *FLid(node *arg_node, info *arg_info);
extern node *FLfundef(node *arg_node, info *arg_info);
extern node *FLlocalfundef(node *arg_node, info *arg_info);

extern node *FLdoForLoop(node *syntaxtree);

#endif /* SRC_TRANSFORMATIONS_FORLOOP_H_ */
