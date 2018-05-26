#ifndef STREAM_H
#define STREAM_H

#include "../common/common.h"

typedef struct Stream Stream;
typedef struct Appender Appender;


typedef enum StreamType {
    FileDescriptorStream_T,
    FileStream_T,
    PipeStream_T
} StreamType;

typedef struct FileStream {
    const char *name;
    int open_flag;
} FileStream;

typedef struct PipeStream {
    BOOL initialized;
    int  descriptors[2];
} PipeStream;

typedef struct Stream {
    StreamType type;
    int file_descriptor;
    union {
        FileStream 	        file;
        PipeStream          *pipe;
    } options;
} Stream;

typedef struct Appender {
    int from_count;
    Stream **from;

    Stream *to;
} Appender;



Stream *        wrap_pipe_into_stream   (PipeStream *pipe_stream, int direction);
Stream **       wrap_stream_into_array  (Stream *stream);
PipeStream      *create_pipe();

void destroy_appender (Appender *appender);
void destroy_stream (Stream *stream);

#endif
