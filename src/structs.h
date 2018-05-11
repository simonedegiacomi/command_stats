#ifndef STRUCTS_H
#define STRUCTS_H

typedef enum NodeType {
    PipeNode_T,
    ExecutableNode_T,
    AndNode_T,
    OrNode_T
} NodeType;

typedef struct Node Node;


typedef struct ExecutableNode {
    char 	*path;

    char 	**argv;
    int 	argc;
} ExecutableNode;

// TODO: Used for ||, &&, ; but also |, can we find a better name?
typedef struct OperandsNode {
    //TODO: rename everywhere with operands_count
    int 	operands;
    Node 	**nodes;
} OperandsNode;

typedef enum StreamType {
    FileDescriptorStream_T,
    FileStream_T,
    PipeStream_T
} StreamType;

typedef struct FileStream {
    char *name;
    int open_flag;
} FileStream;

typedef struct PipeStream {
    int descriptors[2];
} PipeStream;

typedef struct Stream {
    StreamType type;
    int file_descriptor;
    union {
        FileStream 	file;
        PipeStream  pipe;
    } options;
} Stream;

struct Node {
    NodeType type;
    union {
        ExecutableNode 	executable;
        OperandsNode 	operands; // ||, &&, ; or |
    } value;

    // Array of in and outs.
    // TODO: To implement redirect for any file scriptor we need to change these
    // two fields into a sort of map
    int stdins_count;
    Stream **stdins;
    Stream *stdout;
};




Node *new_node();
Stream *        wrap_pipe_into_stream   (PipeStream *pipe_stream, int direction);
Stream **       wrap_stream_into_array  (Stream *stream);

#endif