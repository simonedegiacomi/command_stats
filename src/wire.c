#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "wire.h"


void            wire_pipe_nodes         (OperandsNode *operands, Stream **ins, Stream *out);
Stream *        create_std_stream       (int direction);
PipeStream *    create_pipe             ();
void            wire_r                  (Node *tree, Stream **ins, Stream *out);

void wire (Node *tree) {
	Stream *stdout = create_std_stream(STDOUT_FILENO);
	Stream *std_in = create_std_stream(STDIN_FILENO);
	wire_r(tree, wrap_stream_into_array(std_in), stdout);
}

void wire_r (Node *tree, Stream **ins, Stream *out) {
	tree->stdins = ins;
	tree->stdout = out;


	switch (tree->type) {
		case ExecutableNode_T:
			// nothing to do
			break;
		case PipeNode_T:
			wire_pipe_nodes(&tree->value.operands, ins, out);
			break;
        default:
			fprintf(stderr, "[WIRE] unexpected node type\n");
            //exit(-1);
			break;
	}
}

Stream * create_std_stream (int direction) {
	Stream *stream = malloc(sizeof(Stream));
	stream->type = FileDescriptorStream_T;
	stream->file_descriptor = direction;
	return stream;
}

PipeStream * create_pipe () {
	PipeStream *stream = malloc(sizeof(PipeStream));
	pipe(stream->descriptors);
	return stream;
}

void wire_pipe_nodes (OperandsNode *operands, Stream **ins, Stream *out) {
	operands->nodes[0]->stdins = ins;

	int pipe_count = operands->count;
	int i;
	for (i = 0; i < pipe_count; i++) {

		Node *left 	= operands->nodes[i];
		Node *right = operands->nodes[i + 1];
		PipeStream *left_to_right = create_pipe();

		left->stdout    = wrap_pipe_into_stream(left_to_right, WRITE_INTO_PIPE);
		right->stdins   = wrap_stream_into_array(wrap_pipe_into_stream(left_to_right, READ_FROM_PIPE));
	}

	operands->nodes[operands->count]->stdout = out;
}
