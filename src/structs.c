#include "common.h"

Node *new_node(){
    // TODO: Check all initializations, also on tests
    Node *node = malloc(sizeof(Node));
    node->stdins_count  = 0;
    node->stdins        = NULL;
    node->stdout        = NULL;
    return node;
}

Stream * wrap_pipe_into_stream (PipeStream *pipe_stream, int direction) {
    Stream *stream = malloc(sizeof(Stream));

    stream->type 			                = PipeStream_T;
    stream->file_descriptor                 = pipe_stream->descriptors[direction];
    stream->options.pipe.descriptors[0] 	= pipe_stream->descriptors[0];
    stream->options.pipe.descriptors[1] 	= pipe_stream->descriptors[1];

    return stream;
}

Stream ** wrap_stream_into_array (Stream *stream) {
    Stream **streams = malloc(sizeof(stream));
    streams[0] = stream;
    return streams;
}

