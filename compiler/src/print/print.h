
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
extern node *PRTdeclarations(node * arg_node, info * arg_info);
extern node *PRTfunheader(node * arg_node, info * arg_info);
extern node *PRTrettype(node * arg_node, info * arg_info);
extern node *PRTtype(node * arg_node, info * arg_info);
extern node *PRTbasictype(node * arg_node, info * arg_info);
extern node *PRTparam(node * arg_node, info * arg_info);
extern node *PRTfunbody(node * arg_node, info * arg_info);
extern node *PRTvardec(node * arg_node, info * arg_info);
extern node *PRTstatement(node * arg_node, info * arg_info);
extern node *PRTfuncall(node * arg_node, info * arg_info);
extern node *PRTtypecast(node * arg_node, info * arg_info);
extern node *PRTconst(node * arg_node, info * arg_info);
extern node *PRTblock(node * arg_node, info * arg_info);
extern node *PRTexpr(node * arg_node, info * arg_info);
extern node *PRTid(node * arg_node, info * arg_info);
extern node *PRTvoid(node * arg_node, info * arg_info);
extern node *PRTint(node * arg_node, info * arg_info);

extern node *PRTstatements(node * arg_node, info * arg_info);
extern node *PRTparams(node * arg_node, info * arg_info);
extern node *PRTvardecs(node * arg_node, info * arg_info);
extern node *PRTexpressions(node * arg_node, info * arg_info);

extern node *PRTdeclaration(node * arg_node, info * arg_info);
extern node *PRTfundec(node * arg_node, info * arg_info);
extern node *PRTfundef(node * arg_node, info * arg_info);
extern node *PRTglobaldec(node * arg_node, info * arg_info);
extern node *PRTglobaldef(node * arg_node, info * arg_info);
extern node *PRTstmts (node * arg_node, info * arg_info);
extern node *PRTvardeclares (node * arg_node, info * arg_info);
extern node *PRTvardeclare (node * arg_node, info * arg_info);
extern node *PRTassign (node * arg_node, info * arg_info);
extern node *PRTif (node * arg_node, info * arg_info);
extern node *PRTwhile (node * arg_node, info * arg_info);
extern node *PRTdo (node * arg_node, info * arg_info);
extern node *PRTfor (node * arg_node, info * arg_info);
extern node *PRTvar (node * arg_node, info * arg_info);
extern node *PRTbinop (node * arg_node, info * arg_info);
extern node *PRTunop (node * arg_node, info * arg_info);
extern node *PRTfloat (node * arg_node, info * arg_info);
extern node *PRTnum (node * arg_node, info * arg_info);
extern node *PRTbool (node * arg_node, info * arg_info);
extern node *PRTreturn (node * arg_node, info * arg_info);
extern node *PRTsymboltableentry (node * arg_node, info * arg_info);
extern node *PRTmodule (node * arg_node, info * arg_info);
extern node *PRTerror (node * arg_node, info * arg_info);

/*
 * Begin functioncounters
 */
/*
 * End functioncounters
 */
/*
 * Begin array
 */
/*
 * End arrays
 */
/*
 * Begin multi arrays
 */
/*
 * End multi arrays
 */

extern node *PRTdoPrint( node *syntaxtree);

#endif /* _SAC_PRT_NODE_H_ */
