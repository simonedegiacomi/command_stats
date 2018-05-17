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


typedef struct ExecutableNode {
    char 	*path;

    char 	**argv;
    int 	argc;
} ExecutableNode;

typedef struct OperandsNode {
    int 	count;
    Node 	**nodes;
    Stream  *concatenator;
} OperandsNode;

typedef enum StreamType {
    FileDescriptorStream_T,
    FileStream_T,
    PipeStream_T,
    ConcatenatedStream_T
} StreamType;

typedef struct FileStream {
    const char *name;
    int open_flag;
} FileStream;

typedef struct PipeStream {
    BOOL initialized;
    int  descriptors[2];
} PipeStream;


typedef struct ConcatenatedStream {

    Stream *to;
    int from_count;
    Stream **from;

} ConcatenatedStream;

typedef struct Stream {
    StreamType type;
    int file_descriptor;
    union {
        FileStream 	        file;
        PipeStream          *pipe;
        ConcatenatedStream  concat;

    } options;
} Stream;


typedef struct ExecutionResult {
    int         exit_code;
    struct      timeval user_cpu_time_used; 
    struct      timeval system_cpu_time_used;
    long        maximum_resident_set_size;
    long        block_imput_operation;
    long        block_output_operation;       
    long        voluntary_context_switches;       
    long        involuntary_context_switches;

} ExecutionResult;

struct Node {
    NodeType type;
    union {
        ExecutableNode 	executable;
        OperandsNode 	operands; // ||, &&, ; or |
    } value;

    // Array of in and outs.
    // TODO: To implement redirect for any file scriptor we need to change these
    // two fields into a sort of map
    Stream *std_in;
    Stream *std_out;

    ExecutionResult *result;
    int pid;
};




Node *create_node();
Node *create_executable_node(const char *path);
Node *create_executable_node_single_arg(const char *path);
Stream *        wrap_pipe_into_stream   (PipeStream *pipe_stream, int direction);
Stream **       wrap_stream_into_array  (Stream *stream);

#endif
