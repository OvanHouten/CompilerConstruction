#ifndef SRC_TRANSFORMATIONS_SHORT_CIRCUIT_H_
#define SRC_TRANSFORMATIONS_SHORT_CIRCUIT_H_


#include "types.h"

extern node *SCBEbinop(node *arg_node, info *arg_info);
extern node *SCBEunop(node *arg_node, info *arg_info);

extern node *SCBEdoShortCircuit(node *syntaxtree);

#endif /* SRC_TRANSFORMATIONS_SHORT_CIRCUIT_H_ */
