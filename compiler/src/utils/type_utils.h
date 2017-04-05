/*
 * type_utils.h
 *
 *  Created on: 16 Mar 2017
 *      Author: nico
 */

#ifndef SRC_UTILS_TYPE_UTILS_H_
#define SRC_UTILS_TYPE_UTILS_H_

#include "types_nodetype.h"
#include "node_basic.h"
#include "mytypes.h"

extern char *typeToString(type typeInfo);
extern type determineType(node *expr);
extern char *binopToString(binop op);
extern char *unopToString(unop op);

#endif /* SRC_UTILS_TYPE_UTILS_H_ */
