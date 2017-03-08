/*
 * context_checks.h
 *
 *  Created on: 3 Mar 2017
 *      Author: nico
 */

#ifndef SRC_CONTEXT_ANALYSIS_CONTEXT_ANALYSIS_H_
#define SRC_CONTEXT_ANALYSIS_CONTEXT_ANALYSIS_H_

#include "types.h"

extern node *CCprogram(node *arg_node, info *arg_info);
extern node *CCdeclarations(node *arg_node, info *arg_info);
extern node *CCstatements(node *arg_node, info *arg_info);
extern node *CCfundec(node *arg_node, info *arg_info);
extern node *CCfundef(node *arg_node, info *arg_info);
extern node *CCfunheader(node *arg_node, info *arg_info);
extern node *CCglobaldec(node *arg_node, info *arg_info);
extern node *CCglobaldef(node *arg_node, info *arg_info);
extern node *CCint(node *arg_node, info *arg_info);
extern node *CCfloat(node *arg_node, info *arg_info);
extern node *CCbool(node *arg_node, info *arg_info);
extern node *CCparams(node *arg_node, info *arg_info);
extern node *CCparam(node *arg_node, info *arg_info);
extern node *CCfunbody(node *arg_node, info *arg_info);
extern node *CCvardecs(node *arg_node, info *arg_info);
extern node *CCvardec(node *arg_node, info *arg_info);
extern node *CCfuncall(node *arg_node, info *arg_info);
extern node *CCtypecast(node *arg_node, info *arg_info);
extern node *CCassign(node *arg_node, info *arg_info);
extern node *CCif(node *arg_node, info *arg_info);
extern node *CCwhile(node *arg_node, info *arg_info);
extern node *CCdo(node *arg_node, info *arg_info);
extern node *CCfor(node *arg_node, info *arg_info);
extern node *CCreturn(node *arg_node, info *arg_info);
extern node *CCexprs(node *arg_node, info *arg_info);
extern node *CCarithop(node *arg_node, info *arg_info);
extern node *CCrelop(node *arg_node, info *arg_info);
extern node *CClogicop(node *arg_node, info *arg_info);
extern node *CCunop(node *arg_node, info *arg_info);
extern node *CCid(node *arg_node, info *arg_info);
extern node *CCvoid(node *arg_node, info *arg_info);
extern node *CCintconst(node *arg_node, info *arg_info);
extern node *CCfloatconst(node *arg_node, info *arg_info);
extern node *CCboolconst(node *arg_node, info *arg_info);
extern node *CCsymboltableentry(node *arg_node, info *arg_info);
extern node *CCerror(node *arg_node, info *arg_info);
extern node *CClocalfundef(node *arg_node, info *arg_info);
extern node *CClocalfundefs(node *arg_node, info *arg_info);
extern node *CCarrayassign(node *arg_node, info *arg_info);
extern node *CCarray(node *arg_node, info *arg_info);
extern node *CCids(node *arg_node, info *arg_info);
extern node *CCarrexprs(node *arg_node, info *arg_info);

extern node *CCstatements(node *arg_node, info *arg_info);
extern node *CCvardec(node *arg_node, info *arg_info);
extern node *CCid(node * arg_node, info * arg_info);

extern node *CCdoScopeAnalysis( node *syntaxtree);


#endif /* SRC_CONTEXT_CHECKS_CONTEXT_CHECK_H_ */
