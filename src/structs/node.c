#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"


BOOL find_node_in_tree_with_pid_r (Node *tree, int pid, Node **result, Node **result_father);


Node *create_node(){
    // TODO: Check all initializations, also on tests
    Node *node = malloc(sizeof(Node));
    node->std_in        = NULL;
    node->std_out       = NULL;
    return node;
}

Node *create_executable_node(const char *path) {
    Node *node = create_node();
    node->type = ExecutableNode_T;
    node->value.executable.path = strdup(path);
    return node;
}

Node *create_executable_node_single_arg(const char *path) {
    Node *node = create_executable_node(path);
    ExecutableNode *value = &node->value.executable;
    value->argc = 1;
    value->argv = malloc(2 * sizeof(char*));
    value->argv[0] = strdup(path);
    value->argv[1] = NULL;
    return node;
}


int count_executables_in_tree (Node *node) {
    if (node->type == ExecutableNode_T) {
        return 1;
    }

    int sum = 0;
    int i;
    OperandsNode *operands = &node->value.operands;
    for (i = 0; i < operands->count; i++) {
        sum += count_executables_in_tree(operands->nodes[i]);
    }
    return sum;
}

ExecutionResult *create_execution_result() {
    ExecutionResult *res = malloc(sizeof(ExecutionResult));

    res->start_time = get_current_time();

    return res;
}

int count_max_appender_file_descriptors(Node *node) {
    if (!is_operand_node(node)) {
        return 0;
    }

    OperandsNode *operands = &node->value.operands;
    int sum = 0;
    if (node->type != PipeNode_T) {
        sum += 2 * operands->count + 2;
    }
    int i;
    for (i = 0; i < operands->count; i++) {
        sum += count_max_appender_file_descriptors(operands->nodes[i]);
    }
    
    return sum;
}

BOOL find_node_in_tree_with_pid (Node *tree, int pid, Node **result, Node **result_father){
    *result = NULL;
    if (result_father != NULL) {
        *result_father = NULL;
    }
    return find_node_in_tree_with_pid_r(tree, pid, result, result_father);
}

BOOL find_node_in_tree_with_pid_r (Node *tree, int pid, Node **result, Node **result_father) {
    if (tree->type == ExecutableNode_T) {
        if (tree->pid == pid) {
            *result = tree;
            return TRUE;
        } else {
            return FALSE;
        }
    }

    int i;
    BOOL found = FALSE;
    OperandsNode operands = tree->value.operands;

    for (i = 0; i < operands.count && !found; i++) {
        found = find_node_in_tree_with_pid(operands.nodes[i], pid, result, result_father);
        if (found && result_father != NULL && *result_father == NULL) {
            *result_father = tree;
        }
    }

    return found;
}



Node * find_next_executable_in_operands(Node *father, Node *executed_child) {
    int i;
    int next_index = -1;
    OperandsNode *operands = &father->value.operands;
    int last_child = operands->count - 1;
    for (i = 0; i < last_child && next_index < 0; i++) {
        if (operands->nodes[i] == executed_child) {
            next_index = i + 1;
        }
    }

    return next_index < 0 ? NULL : operands->nodes[next_index];
}

BOOL is_operand_node (Node *node) {
    return node->type == AndNode_T
            || node->type == OrNode_T
            || node->type == SemicolonNode_T
            || node->type == PipeNode_T;
}
