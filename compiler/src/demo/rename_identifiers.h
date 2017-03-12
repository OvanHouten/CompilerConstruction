#ifndef _RENAME_IDENTIFIERS_H_
#define _RENAME_IDENTIFIERS_H_
#include "types.h"

extern node *RIvardef(node *arg_node, info *arg_info);
extern node *RIid(node * arg_node, info * arg_info);
extern node *RIdoRenameIdentifiers( node *syntaxtree);

#endif
