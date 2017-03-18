/*
 * type_utils.h
 *
 *  Created on: 16 Mar 2017
 *      Author: nico
 */

#ifndef SRC_UTILS_TYPE_UTILS_H_
#define SRC_UTILS_TYPE_UTILS_H_

#include "types_nodetype.h"
#include "mytypes.h"

extern char *nodeTypeToString(nodetype typeInfo);
extern char *typeToString(type typeInfo);
extern type nodeTypeToType(nodetype typeInfo);

#endif /* SRC_UTILS_TYPE_UTILS_H_ */
