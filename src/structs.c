#include "structs.h"
#include "common.h"

Node *new_node(){
    // TODO: Check all initializations, also on tests
    Node *node = malloc(sizeof(Node));
    node->std_in        = NULL;
    node->std_out       = NULL;
    return node;
}

Node *new_executable_node(const char* path) {
    Node *node = new_node();
    node->type = ExecutableNode_T;
    node->value.executable.path = strdup(path);
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

