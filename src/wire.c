#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "wire.h"
#include "common.h"


void wire_pipe_nodes(OperandsNode *operands, Stream *in, Stream *out);

Stream * wire_operand_nodes(OperandsNode *operands, Stream *in, Stream *out);

Stream *create_std_stream(int direction);

PipeStream *create_pipe();

void wire_r(Node *tree, Stream *in, Stream *out);

void wire(Node *tree) {
    Stream *std_out = create_std_stream(STDOUT_FILENO);
    Stream *std_in = create_std_stream(STDIN_FILENO);
    wire_r(tree, std_in, std_out);
}

void wire_r(Node *tree, Stream *in, Stream *out) {
    tree->std_in = in;
    tree->std_out = out;


    switch (tree->type) {
        case ExecutableNode_T:
            // nothing to do
            break;
        case PipeNode_T:
            wire_pipe_nodes(&tree->value.operands, in, out);
            break;

        case AndNode_T:
        case OrNode_T:
            tree->std_out = wire_operand_nodes(&tree->value.operands, in, out);
            break;
        default:
            fprintf(stderr, "[WIRE] unexpected node type\n");
            //exit(-1);
            break;
    }
}

Stream *create_std_stream(int direction) {
    Stream *stream = malloc(sizeof(Stream));
    stream->type = FileDescriptorStream_T;
    stream->file_descriptor = direction;
    return stream;
}

PipeStream *create_pipe() {
    PipeStream *stream = malloc(sizeof(PipeStream));
    pipe(stream->descriptors);
    return stream;
}

void wire_pipe_nodes(OperandsNode *operands, Stream *in, Stream *out) {
    operands->nodes[0]->std_in = in;

    int pipe_count = operands->count;
    int i;
    for (i = 0; i < pipe_count; i++) {

        Node *left = operands->nodes[i];
        Node *right = operands->nodes[i + 1];
        PipeStream *left_to_right = create_pipe();

        left->std_out = wrap_pipe_into_stream(left_to_right, WRITE_INTO_PIPE);
        right->std_in = wrap_pipe_into_stream(left_to_right, READ_FROM_PIPE);
    }

    int last_node = operands->count - 1;
    operands->nodes[last_node]->std_out = out;
}


Stream *wire_operand_nodes(OperandsNode *operands, Stream *in, Stream *out) {
    operands->nodes[0]->std_in = in;

    Stream *end = malloc(sizeof(Stream));
    end->type = ConcatenatedStream_T;
    end->options.concat.from_count = operands->count;
    end->options.concat.from = malloc(operands->count * sizeof(Stream*));

    int i;
    for (i = 0; i < operands->count; i++) {
        Node *node = operands->nodes[i];

        if (node->std_in == NULL) {
            node->std_in = create_std_stream(STDIN_FILENO);
        }

        PipeStream *node_to_concat = create_pipe();
        node->std_out = wrap_pipe_into_stream(node_to_concat, WRITE_INTO_PIPE);
        end->options.concat.from[i] = wrap_pipe_into_stream(node_to_concat, READ_FROM_PIPE);
    }

    end->options.concat.to = out;
    return end;
}



