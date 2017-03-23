/*
 * op_wrapper.h
 *
 *  Created on: 22 Mar 2017
 *      Author: nico
 */

#ifndef SRC_OPTIMISATIONS_OP_WRAPPER_H_
#define SRC_OPTIMISATIONS_OP_WRAPPER_H_

#include "types.h"

extern node *OPunop(node *arg_node, info *arg_info);

extern node *OPdoOptimisations(node *syntaxtree);


#endif /* SRC_OPTIMISATIONS_OP_WRAPPER_H_ */