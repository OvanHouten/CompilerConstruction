/*
 * split_vardefs.h
 *
 *  Created on: 14 Mar 2017
 *      Author: nico
 */

#ifndef SRC_TRANSFORMATIONS_SPLIT_VARDEFS_H_
#define SRC_TRANSFORMATIONS_SPLIT_VARDEFS_H_


#include "types.h"

extern node *SVfundef(node *arg_node, info *arg_info);
extern node *SVlocalfundef(node *arg_node, info *arg_info);
extern node *SVvardef(node *arg_node, info *arg_info);

extern node *SVdoSplitVarDefs(node *syntaxtree);

#endif /* SRC_TRANSFORMATIONS_SPLIT_VARDEFS_H_ */