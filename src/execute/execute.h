#ifndef EXECUTE_H
#define EXECUTE_H

#include "../common/common.h"
#include "../structs/node.h"

// Executes the node filling the inner structures with the execution result.
// This function block the thread until the execution of the tree is completed.
void execute(Node *node);


#endif