/*
 * type_check.h
 *
 *  Created on: 15 Mar 2017
 *      Author: nico
 */

#ifndef SRC_CONTEXT_ANALYSIS_TYPE_CHECK_H_
#define SRC_CONTEXT_ANALYSIS_TYPE_CHECK_H_

#include "types.h"

extern node *TCfundef(node *arg_node, info *arg_info);

extern node *TCdoTypeCheck(node *syntaxtree);

#endif /* SRC_CONTEXT_ANALYSIS_TYPE_CHECK_H_ */
