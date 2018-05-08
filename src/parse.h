#ifndef PARSE_H
#define PARSE_H

#include "common.h"

typedef enum NodeType {
	PipeNode_T,
	ExecutableNode_T,
	AndNode_T,
	OrNode_T,
	SemicolonNode_T
} NodeType;

typedef struct Node Node;

typedef struct PipeNode {
	Node *from;
	Node *to;
} PipeNode;

typedef struct ExecutableNode {
	char *path;

	char **argv;
	int argc;
} ExecutableNode;

typedef struct OperandsNode {
	int operands;
	Node **nodes;
} OperandsNode;


struct Node {
	NodeType type;
	union {
		PipeNode pipe;
		ExecutableNode executable;
		OperandsNode operands;
	} value;

	int stdIn;
	int stdOut;
};

Node * create_tree_from_string (const char *string);


#endif
