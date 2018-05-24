#include <stdlib.h>
#include "stream.h"


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



PipeStream *create_pipe() {
    PipeStream *stream = malloc(sizeof(PipeStream));
    stream->initialized = FALSE;
    return stream;
}
