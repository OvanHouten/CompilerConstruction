
/**
 * @file print.h
 *
 * Functions to print node structures
 * 
 */

#ifndef _SAC_PRT_NODE_H_
#define _SAC_PRT_NODE_H_

#include "types.h"

extern node *PRTprogram(node * arg_node, info * arg_info);
extern node *PRTnop(node *arg_node, info *arg_info);
extern node *PRTsymboltable(node *arg_node, info *info_arg);

extern node *PRTdeclarations(node * arg_node, info * arg_info);
extern node *PRTfunheader(node * arg_node, info * arg_info);

extern node *PRTfundef(node * arg_node, info * arg_info);
extern node *PRTfunbody(node * arg_node, info * arg_info);

extern node *PRTvardecs(node * arg_node, info * arg_info);

extern node *PRTvardef(node * arg_node, info * arg_info);

extern node *PRTstatements(node * arg_node, info * arg_info);
extern node *PRTassign(node * arg_node, info * arg_info);
extern node *PRTcompop(node * arg_node, info * arg_info);
extern node *PRTtypecast(node * arg_node, info * arg_info);
extern node *PRTfuncall(node * arg_node, info * arg_info);
extern node *PRTreturn(node * arg_node, info * arg_info);

extern node *PRTif(node * arg_node, info * arg_info);
extern node *PRTwhile(node * arg_node, info * arg_info);
extern node *PRTdo(node * arg_node, info * arg_info);
extern node *PRTfor(node * arg_node, info * arg_info);

extern node *PRTexprs(node * arg_node, info * arg_info);

extern node *PRTternop (node * arg_node, info * arg_info);
extern node *PRTbinop (node * arg_node, info * arg_info);
extern node *PRTunop (node * arg_node, info * arg_info);

extern node *PRTid(node * arg_node, info * arg_info);

extern node *PRTparams(node * arg_node, info * arg_info);

extern node *PRTintconst(node * arg_node, info * arg_info);
extern node *PRTfloatconst(node * arg_node, info * arg_info);
extern node *PRTboolconst(node * arg_node, info * arg_info);

extern node *PRTerror (node * arg_node, info * arg_info);

extern node *PRTdoPrint( node *syntaxtree);

extern node *PRTarrayassign(node * arg_node, info * arg_info);
extern node *PRTarrdata(node * arg_node, info * arg_info);
extern node *PRTarray(node * arg_node, info * arg_info);
extern node *PRTids(node * arg_node, info * arg_info);
extern node *PRTarrexprs(node * arg_node, info * arg_info);

extern node *PRTlocalfundefs(node * arg_node, info * arg_info);

extern node *PRTsymboltableentry (node * arg_node, info * arg_info);
#endif /* _SAC_PRT_NODE_H_ */
