#include "structs.h"
#include "common.h"

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

Stream * wrap_pipe_into_stream (PipeStream *pipe_stream, int direction) {
    Stream *stream = malloc(sizeof(Stream));

    stream->type 			                = PipeStream_T;
    stream->file_descriptor                 = pipe_stream->descriptors[direction];
    stream->options.pipe                    = pipe_stream;

    return stream;
}

Stream ** wrap_stream_into_array (Stream *stream) {
    Stream **streams = malloc(sizeof(stream));
    streams[0] = stream;
    return streams;
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