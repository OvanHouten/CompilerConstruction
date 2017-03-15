/*
 * return_check.h
 *
 *  Created on: 15 Mar 2017
 *      Author: nico
 */

#ifndef SRC_CONTEXT_ANALYSIS_RETURN_CHECK_H_
#define SRC_CONTEXT_ANALYSIS_RETURN_CHECK_H_

extern node *RCfundef(node *arg_node, info *arg_info);
extern node *RClocalfundef(node *arg_node, info *arg_info);

extern node *RCdoReturnCheck(node *syntaxtree);

#endif /* SRC_CONTEXT_ANALYSIS_RETURN_CHECK_H_ */
