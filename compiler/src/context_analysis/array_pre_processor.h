#ifndef SRC_CONTEXT_ANALYSIS_ARRAY_PRE_PROCESSOR_H_
#define SRC_CONTEXT_ANALYSIS_ARRAY_PRE_PROCESSOR_H_

#include "types.h"


extern node *PPAvardef(node *arg_node, info *arg_info);
extern node* PPAid(node* arg_node, info* arg_info);

extern node *PPAdoPreProcessorArray(node *syntaxtree);


#endif /* SRC_CONTEXT_ANALYSIS_ARRAY_PREPROCESSOR_H_ */