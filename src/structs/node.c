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
    node->result        = NULL;
    return node;
}

Node *create_executable_node(const char *path) {
    Node *node = create_node();
    node->type = ExecutableNode_T;

    ExecutableNode *executable = &node->value.executable;
    executable->path = strdup(path);
    executable->cd = NULL;
    executable->cd_count = 0;

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

Node *find_father_of_node_in_tree(Node *child, Node *tree) {
    if (is_operand_node(tree)) {
        int i;
        const OperandsNode *operands = &tree->value.operands;
        for (i = 0; i < operands->count; i++) {
            if (operands->nodes[i] == child) {
                return tree;
            } else {
                Node *res = find_father_of_node_in_tree(child, operands->nodes[i]);
                if (res != NULL) {
                    return res;
                }
            }
        }

    }
    return NULL;
}

// TODO: Ma è davvero node e non eseguibile?
Node * find_next_node_in_operands(Node *father, Node *executed_child) {
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



void remove_node_from_operands(Node *operands_node, Node *to_remove) {
    OperandsNode *operands = &operands_node->value.operands;

    int i;
    BOOL removed = FALSE;
    for (i = 0; i < operands->count; i++) {
        if (operands->nodes[i] == to_remove) {
            destroy_node(operands->nodes[i]);

            removed = TRUE;
        } else if (removed) {
            operands->nodes[i - 1] = operands->nodes[i];
        }
    }
    operands->count--;
}



struct timespec get_total_clock_time(Node *node) {
    struct timespec start = node->result->start_time;
    struct timespec end = node->result->end_time;

    return (struct timespec) {
        .tv_sec     = end.tv_sec - start.tv_sec,
        .tv_nsec    = end.tv_nsec - start.tv_nsec
    };
}


void destroy_node (Node *node) {
    int i = 0;
    if (is_operand_node(node)) {
        OperandsNode *operands = &node->value.operands;

        for (i = 0; i < operands->count; i++) {
            destroy_node(operands->nodes[i]);
        }
        free(operands->nodes);

        if (operands->appender != NULL) {
            destroy_appender(operands->appender);
        }

        printf("-------");
    } else {
        ExecutableNode *executable = &node->value.executable;
        free(executable->path);

        for (i = 0; i < executable->argc; i++) { // NOTE: Last in null
            free(executable->argv[i]);
        }
        free(executable->argv);


        if (executable->cd != NULL) {
            for (i = 0; i < executable->cd_count; i++) {
                free(executable->cd[i]);
            }
            free(executable->cd);
        }
    }

    if (node->std_in != NULL) {
        destroy_stream(node->std_in);
    }

    if (node->std_out != NULL) {
        destroy_stream(node->std_out);
    }

    if (node->result != NULL) {
        destroy_result(node->result);
    }

    free(node);
}

void destroy_result (ExecutionResult *result) {
    free(result);
}
