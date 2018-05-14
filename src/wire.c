#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "wire.h"
#include "common.h"


void wire_pipe_nodes(OperandsNode *operands, Stream *in, Stream *out);

void wire_operand_nodes(OperandsNode *operands, Stream *in, Stream *out);

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
            wire_operand_nodes(&tree->value.operands, in, out);
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
    // create all the pipes
    int pipe_count      = operands->count - 1;
    PipeStream **pipes  = malloc(pipe_count * sizeof(PipeStream*));
    int i;
    for (i = 0; i < pipe_count; i++) {
        pipes[i] = create_pipe();
    }

    // wire first node
    Node *first                 = operands->nodes[0];
    PipeStream *first_to_second = pipes[0];
    wire_r(first, in, wrap_pipe_into_stream(first_to_second, WRITE_INTO_PIPE));

    // wire middle nodes
    for (i = 1; i < pipe_count - 1; i++) {
        // i - 1 is the left node, i the center and i + 1 the right node
        Node *center                = operands->nodes[i];

        PipeStream *left_to_center  = pipes[i - 1];
        PipeStream *center_to_right = pipes[i];
        
        wire_r(center,
               wrap_pipe_into_stream(left_to_center, READ_FROM_PIPE),
               wrap_pipe_into_stream(center_to_right, WRITE_INTO_PIPE));
    }

    // wire last node
    int last_node_index = pipe_count;
    int last_pipe_index = pipe_count - 1;
    Node *last          = operands->nodes[last_node_index];
    PipeStream *to_last = pipes[last_pipe_index];
    wire_r(last, wrap_pipe_into_stream(to_last, READ_FROM_PIPE), out);

    // we don't need the array with all the pipes anymore
    free(pipes);
}


void wire_operand_nodes(OperandsNode *operands, Stream *in, Stream *out) {
    operands->nodes[0]->std_in = in;

    Stream *end = malloc(sizeof(Stream));
    end->type = ConcatenatedStream_T;
    end->options.concat.from_count = operands->count;
    end->options.concat.from = malloc(operands->count * sizeof(Stream*));

    int i;
    for (i = 0; i < operands->count; i++) {
        Node *node = operands->nodes[i];
        PipeStream *node_to_concat = create_pipe();

        Stream *node_in = node->std_in;
        if (node_in == NULL) {
            node_in = create_std_stream(STDIN_FILENO);
        }

        wire_r(node, node_in, wrap_pipe_into_stream(node_to_concat, WRITE_INTO_PIPE));

        end->options.concat.from[i] = wrap_pipe_into_stream(node_to_concat, READ_FROM_PIPE);
    }

    end->options.concat.to = out;
    operands->concatenator = end;
}



