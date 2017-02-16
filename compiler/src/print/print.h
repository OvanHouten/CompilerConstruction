
/**
 * @file print.h
 *
 * Functions to print node structures
 * 
 */

#ifndef _SAC_PRT_NODE_H_
#define _SAC_PRT_NODE_H_

#include "types.h"

extern node *PRTdeclaration(node * arg_node, info * arg_info);
extern node *PRTfundec(node * arg_node, info * arg_info);
extern node *PRTfundef(node * arg_node, info * arg_info);
extern node *PRTglobaldec(node * arg_node, info * arg_info);
extern node *PRTglobaldef(node * arg_node, info * arg_info);
extern node *PRTstmts (node * arg_node, info * arg_info);
extern node *PRTassign (node * arg_node, info * arg_info);
extern node *PRTif (node * arg_node, info * arg_info);
extern node *PRTwhile (node * arg_node, info * arg_info);
extern node *PRTdo (node * arg_node, info * arg_info);
extern node *PRTvar (node * arg_node, info * arg_info);
extern node *PRTvarlet (node * arg_node, info * arg_info);
extern node *PRTbinop (node * arg_node, info * arg_info);
extern node *PRTunop (node * arg_node, info * arg_info);
extern node *PRTfloat (node * arg_node, info * arg_info);
extern node *PRTnum (node * arg_node, info * arg_info);
extern node *PRTbool (node * arg_node, info * arg_info);
extern node *PRTsymboltableentry (node * arg_node, info * arg_info);
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
