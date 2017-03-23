#ifndef _GEN_BYTE_CODE_H_
#define _GEN_BYTE_CODE_H_
#include "types.h"

extern node *GBCprogram(node *arg_node, info *arg_info);
extern node *GBCsymboltableentry(node *arg_node, info *arg_info);
extern node *GBCdoGenByteCode( node *syntaxtree);

#endif
