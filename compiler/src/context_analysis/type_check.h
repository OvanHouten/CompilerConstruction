/*
 * type_check.h
 *
 *  Created on: 15 Mar 2017
 *      Author: nico
 */

#ifndef SRC_CONTEXT_ANALYSIS_TYPE_CHECK_H_
#define SRC_CONTEXT_ANALYSIS_TYPE_CHECK_H_

#include "types.h"

extern node *TCassign(node *arg_node, info *arg_info);
extern node *TCvardef(node *arg_node, info *arg_info);
extern node *TCtypecast(node *arg_node, info *arg_info);
extern node *TCunop(node *arg_node, info *arg_info);
extern node *TCbinop(node *arg_node, info *arg_info);
extern node *TCfunbody(node *arg_node, info *arg_info);
extern node *TCfundef(node *arg_node, info *arg_info);
extern node *TCfuncall(node *arg_node, info *arg_info);
extern node *TCreturn(node *arg_node, info *arg_info);
extern node *TCdo(node *arg_node, info *arg_info);
extern node *TCwhile(node *arg_node, info *arg_info);
extern node *TCfor(node *arg_node, info *arg_info);

extern node *TCdoTypeCheck(node *syntaxtree);

#endif /* SRC_CONTEXT_ANALYSIS_TYPE_CHECK_H_ */
