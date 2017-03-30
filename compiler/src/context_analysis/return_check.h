/*
 * return_check.h
 *
 *  Created on: 30 Mar 2017
 *      Author: nico
 */

#ifndef SRC_CONTEXT_ANALYSIS_RETURN_CHECK_H_
#define SRC_CONTEXT_ANALYSIS_RETURN_CHECK_H_

#include "types.h"

extern node *RCfundef(node *arg_node, info *arg_info);

extern node *RCdoReturnCheck(node *syntaxtree);

#endif /* SRC_CONTEXT_ANALYSIS_RETURN_CHECK_H_ */
