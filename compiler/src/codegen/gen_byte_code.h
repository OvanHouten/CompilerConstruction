#ifndef _GEN_BYTE_CODE_H_
#define _GEN_BYTE_CODE_H_
#include "types.h"

extern node *GBCprogram(node *arg_node, info *arg_info);
extern node *GBCsymboltableentry(node *arg_node, info *arg_info);
extern node *GBCdeclarations(node *arg_node, info *arg_info);
extern node *GBClocalfundefs(node *arg_node, info *arg_info);
extern node *GBCstatements(node *arg_node, info *arg_info);
extern node *GBCfundef(node *arg_node, info *arg_info);
extern node *GBCreturn(node *arg_node, info *arg_info);
extern node *GBCfuncall(node *arg_node, info *arg_info);
extern node *GBCif(node *arg_node, info *arg_info);
extern node *GBCternop(node *arg_node, info *arg_info);
extern node *GBCwhile(node *arg_node, info *arg_info);
extern node *GBCdo(node *arg_node, info *arg_info);
extern node *GBCassign(node *arg_node, info *arg_info);
extern node *GBCcompop(node *arg_node, info *arg_info);
extern node *GBCid(node *arg_node, info *arg_info);
extern node *GBCunop(node *arg_node, info *arg_info);
extern node *GBCtypecast(node *arg_node, info *arg_info);
extern node *GBCbinop(node *arg_node, info *arg_info);
extern node *GBCintconst(node *arg_node, info *arg_info);
extern node *GBCfloatconst(node *arg_node, info *arg_info);
extern node *GBCboolconst(node *arg_node, info *arg_info);
extern node *GBCdoGenByteCode( node *syntaxtree);

#endif
