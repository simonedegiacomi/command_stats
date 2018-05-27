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

void destroy_appender (Appender *appender) {
    int i;
    for (i = 0; i < appender->from_count; i++) {
        destroy_stream(appender->from[i]);
    }

    destroy_stream(appender->to);
    free(appender);
}

void destroy_stream (Stream *stream) {
    if (stream->type == FileStream_T) {

        FileStream *file = &stream->options.file;
        free((void *) file->name);
    } else if (stream->type == PipeStream_T) {

        // TODO: hehe, questo non è facile...
    } else {
        //printf("fd stream %d\n", stream);
    }
    free(stream);
    printf("morto");
}
