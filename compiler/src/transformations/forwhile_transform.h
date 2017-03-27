#ifndef SRC_TRANSFORMATIONS_FORWHILE_TRANSFORM_H_
#define SRC_TRANSFORMATIONS_FORWHILE_TRANSFORM_H_


#include "types.h"

extern node *FWTfundef(node *arg_node, info *arg_info);
extern node *FWTstatements(node *arg_node, info *arg_info);
extern node *FWTfor(node* arg_node, info* arg_info);

extern node *FWTdoForWhileTransform(node* syntaxtree);


#endif /* SRC_TRANSFORMATIONS_FORWHILE_TRANSFORM_H_ */
