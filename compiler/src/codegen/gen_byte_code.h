#ifndef _GEN_BYTE_CODE_H_
#define _GEN_BYTE_CODE_H_
#include "types.h"

extern node *GBCprogram(node *arg_node, info *arg_info);
extern node *GBCsymboltableentry(node *arg_node, info *arg_info);
extern node *GBCdeclarations(node *arg_node, info *arg_info);
extern node *GBCfundef(node *arg_node, info *arg_info);
extern node *GBCreturn(node *arg_node, info *arg_info);
extern node *GBCif(node *arg_node, info *arg_info);
extern node *GBCwhile(node *arg_node, info *arg_info);
extern node *GBCintconst(node *arg_node, info *arg_info);
extern node *GBCfloatconst(node *arg_node, info *arg_info);
extern node *GBCboolconst(node *arg_node, info *arg_info);
extern node *GBCdoGenByteCode( node *syntaxtree);

#endif
