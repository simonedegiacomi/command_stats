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

typedef struct PipeNode {
	Node 	*from;
	Node 	*to;
} PipeNode;

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
	FileStream_T
} StreamType;

typedef struct FileStream {
	char *name;
	BOOL append;
} FileStream;

typedef struct Stream {
	StreamType type;
	int file_descriptor;
	union {
		FileStream 	file;
	} options;
} Stream;

struct Node {
	NodeType type;
	union {
		PipeNode 		pipe;
		ExecutableNode 	executable;
		OperandsNode 	operands;
	} value;

	Stream **stdin;
	Stream **stdout;
};

Node * create_tree_from_string (const char *string);


#endif
