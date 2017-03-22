/*
 * context_analysis.h
 *
 *  Created on: 3 Mar 2017
 *      Author: nico
 */

#ifndef SRC_CONTEXT_ANALYSIS_CONTEXT_ANALYSIS_H_
#define SRC_CONTEXT_ANALYSIS_CONTEXT_ANALYSIS_H_

#include "types.h"

extern node *SAprogram(node *arg_node, info *arg_info);
extern node *SAsymboltable(node *arg_node, info *info_arg);
extern node *SAdeclarations(node *arg_node, info *arg_info);
extern node *SAstatements(node *arg_node, info *arg_info);
extern node *SAfundef(node *arg_node, info *arg_info);
extern node *SAfunheader(node *arg_node, info *arg_info);
extern node *SAvardef(node *arg_node, info *arg_info);
extern node *SAparams(node *arg_node, info *arg_info);
extern node *SAfunbody(node *arg_node, info *arg_info);
extern node *SAvardecs(node *arg_node, info *arg_info);
extern node *SAfuncall(node *arg_node, info *arg_info);
extern node *SAtypecast(node *arg_node, info *arg_info);
extern node *SAassign(node *arg_node, info *arg_info);
extern node *SAif(node *arg_node, info *arg_info);
extern node *SAwhile(node *arg_node, info *arg_info);
extern node *SAdo(node *arg_node, info *arg_info);
extern node *SAfor(node *arg_node, info *arg_info);
extern node *SAreturn(node *arg_node, info *arg_info);
extern node *SAexprs(node *arg_node, info *arg_info);
extern node *SAbinop(node *arg_node, info *arg_info);
extern node *SAunop(node *arg_node, info *arg_info);
extern node *SAid(node *arg_node, info *arg_info);
extern node *SAintconst(node *arg_node, info *arg_info);
extern node *SAfloatconst(node *arg_node, info *arg_info);
extern node *SAboolconst(node *arg_node, info *arg_info);
extern node *SAsymboltableentry(node *arg_node, info *arg_info);
extern node *SAerror(node *arg_node, info *arg_info);
extern node *SAlocalfundefs(node *arg_node, info *arg_info);
extern node *SAarrayassign(node *arg_node, info *arg_info);
extern node *SAarray(node *arg_node, info *arg_info);
extern node *SAids(node *arg_node, info *arg_info);
extern node *SAarrexprs(node *arg_node, info *arg_info);

extern node *SAstatements(node *arg_node, info *arg_info);
extern node *SAid(node * arg_node, info * arg_info);

extern node *SAdoScopeAnalysis( node *syntaxtree);


#endif /* SRC_CONTEXT_CHECKS_CONTEXT_CHECK_H_ */
