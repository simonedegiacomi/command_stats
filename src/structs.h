#ifndef STRUCTS_H
#define STRUCTS_H

#include "common.h"

typedef enum NodeType {
    PipeNode_T,
    ExecutableNode_T,
    AndNode_T,
    OrNode_T,
    SemicolonNode_T
} NodeType;

typedef struct Node Node;
typedef struct Stream Stream;
typedef struct Appender Appender;


typedef struct ExecutableNode {
    char 	*path;

    char 	**argv;
    int 	argc;
} ExecutableNode;

typedef struct OperandsNode {
    int 	  count;
    Node 	  **nodes;
    Appender  *appender; // Used only in and, or, and semicolon
} OperandsNode;

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


typedef struct Appender {
    int from_count;
    Stream **from;

    Stream *to;
} Appender;

typedef struct Stream {
    StreamType type;
    int file_descriptor;
    union {
        FileStream 	        file;
        PipeStream          *pipe;
    } options;
} Stream;


typedef struct ExecutionResult {
    // TODO: Choose between REALTIME and MONOTONIC
    long                start_time;
    long                end_time;
    int                 exit_code;
    struct timeval      user_cpu_time_used;
    struct timeval      system_cpu_time_used;
    long                clock_time;
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
Stream *        wrap_pipe_into_stream   (PipeStream *pipe_stream, int direction);
Stream **       wrap_stream_into_array  (Stream *stream);

int count_executables_in_tree (Node *node);
int count_max_appender_file_descriptors(Node *node);

#endif
