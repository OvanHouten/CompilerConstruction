/*
 * context_checks.h
 *
 *  Created on: 3 Mar 2017
 *      Author: nico
 */

#ifndef SRC_CONTEXT_ANALYSIS_CONTEXT_ANALYSIS_H_
#define SRC_CONTEXT_ANALYSIS_CONTEXT_ANALYSIS_H_

#include "types.h"

extern node *CAprogram(node *arg_node, info *arg_info);
extern node *CAdeclarations(node *arg_node, info *arg_info);
extern node *CAstatements(node *arg_node, info *arg_info);
extern node *CAfundec(node *arg_node, info *arg_info);
extern node *CAfundef(node *arg_node, info *arg_info);
extern node *CAfunheader(node *arg_node, info *arg_info);
extern node *CAglobaldec(node *arg_node, info *arg_info);
extern node *CAglobaldef(node *arg_node, info *arg_info);
extern node *CAint(node *arg_node, info *arg_info);
extern node *CAfloat(node *arg_node, info *arg_info);
extern node *CAbool(node *arg_node, info *arg_info);
extern node *CAparams(node *arg_node, info *arg_info);
extern node *CAparam(node *arg_node, info *arg_info);
extern node *CAfunbody(node *arg_node, info *arg_info);
extern node *CAvardecs(node *arg_node, info *arg_info);
extern node *CAvardec(node *arg_node, info *arg_info);
extern node *CAfuncall(node *arg_node, info *arg_info);
extern node *CAtypecast(node *arg_node, info *arg_info);
extern node *CAassign(node *arg_node, info *arg_info);
extern node *CAif(node *arg_node, info *arg_info);
extern node *CAwhile(node *arg_node, info *arg_info);
extern node *CAdo(node *arg_node, info *arg_info);
extern node *CAfor(node *arg_node, info *arg_info);
extern node *CAreturn(node *arg_node, info *arg_info);
extern node *CAexprs(node *arg_node, info *arg_info);
extern node *CAarithop(node *arg_node, info *arg_info);
extern node *CArelop(node *arg_node, info *arg_info);
extern node *CAlogicop(node *arg_node, info *arg_info);
extern node *CAunop(node *arg_node, info *arg_info);
extern node *CAid(node *arg_node, info *arg_info);
extern node *CAvoid(node *arg_node, info *arg_info);
extern node *CAintconst(node *arg_node, info *arg_info);
extern node *CAfloatconst(node *arg_node, info *arg_info);
extern node *CAboolconst(node *arg_node, info *arg_info);
extern node *CAsymboltableentry(node *arg_node, info *arg_info);
extern node *CAerror(node *arg_node, info *arg_info);
extern node *CAlocalfundef(node *arg_node, info *arg_info);
extern node *CAlocalfundefs(node *arg_node, info *arg_info);
extern node *CAarrayassign(node *arg_node, info *arg_info);
extern node *CAarray(node *arg_node, info *arg_info);
extern node *CAids(node *arg_node, info *arg_info);
extern node *CAarrexprs(node *arg_node, info *arg_info);

extern node *CAstatements(node *arg_node, info *arg_info);
extern node *CAvardec(node *arg_node, info *arg_info);
extern node *CAid(node * arg_node, info * arg_info);

extern node *CAdoScopeAnalysis( node *syntaxtree);


#endif /* SRC_CONTEXT_CHECKS_CONTEXT_CHECK_H_ */
