#ifndef PARSE_H
#define PARSE_H

#include "common.h"

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

typedef struct OperandsNode {
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
	BOOL append;
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
	Stream **stdin;
	Stream **stdout;
};

Node * create_tree_from_string (const char *string);


#endif
