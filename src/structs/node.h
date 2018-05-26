#ifndef STRUCTS_H
#define STRUCTS_H

#include <sys/time.h>
#include "../structs/stream.h"
#include "../common/common.h"

typedef enum NodeType {
    PipeNode_T,
    ExecutableNode_T,
    AndNode_T,
    OrNode_T,
    SemicolonNode_T
} NodeType;

typedef struct Node Node;



typedef struct ExecutableNode {
    char 	*path;

    char 	**argv;
    int 	argc;

    char    **cd;
    int     cd_count;
} ExecutableNode;

typedef struct OperandsNode {
    int 	  count;
    Node 	  **nodes;
    Appender  *appender; // Used only in and, or, and semicolon
} OperandsNode;



typedef struct ExecutionResult {
    // TODO: Choose between REALTIME and MONOTONIC
    BOOL                execution_failed;
    struct timespec     start_time;
    struct timespec     end_time;
    int                 exit_code;
    struct timeval      user_cpu_time_used;
    struct timeval      system_cpu_time_used;
    long                maximum_resident_set_size;
} ExecutionResult;

struct Node {
    NodeType type;
    union {
        ExecutableNode 	executable;
        OperandsNode 	operands; // ||, &&, ; or |
    } value;

    // Array of in and outs.
    // TODO: To implement redirect for any file descriptor we need to change these
    // two fields into a sort of map
    Stream *std_in;
    Stream *std_out;

    ExecutionResult *result;
    int pid;
};




Node *create_node();
Node *create_executable_node(const char *path);
Node *create_executable_node_single_arg(const char *path);
ExecutionResult *create_execution_result();


int count_executables_in_tree (Node *node);


BOOL find_node_in_tree_with_pid (Node *tree, int pid, Node **result, Node **result_father);
Node * find_next_executable_in_operands(Node *father, Node *executed_child);
BOOL is_operand_node (Node *node);
int count_max_appender_file_descriptors(Node *node);

struct timespec get_total_clock_time(Node *node);

void remove_node_from_operands(Node *operands_node, Node *to_remove);
void destroy_node (Node *node);
void destroy_result (ExecutionResult *result);




#endif
