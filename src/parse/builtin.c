#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include "builtin.h"
#include "../structs/node.h"


/** Private functions declaration */
void apply_cd_builtin (Node *root);
void apply_cd_builtin_r (Node *father, Node *node);
void add_cd (ExecutableNode *executable, char *dir);


void wire_redirects (Node *root);
void wire_redirects_r(Node *node, Stream *in, Stream *out);

void wire_pipe_nodes(OperandsNode *operands, Stream *in, Stream *out);
void wire_operand_nodes(OperandsNode *operands, Stream *in, Stream *out);

Stream *create_std_stream(int direction);
/** End of private function declaration */


void apply_builtin(Node *tree) {
    apply_cd_builtin(tree);
    wire_redirects(tree);
}

void apply_cd_builtin (Node *root) {
    apply_cd_builtin_r(NULL, root);
}

void apply_cd_builtin_r (Node *father, Node *node) {
    if (is_operand_node(node)) {
        int i;
        OperandsNode *operands = &node->value.operands;
        for (i = 0; i < operands->count; i++) {
            int count = operands->count;

            apply_cd_builtin_r(node, operands->nodes[i]);


            if (operands->count == (count - 1)) {
                i--;
            }
        }
    } else if (node->type == ExecutableNode_T) {
        ExecutableNode *executable = &node->value.executable;
        if (strcmp(executable->path, "cd") == 0 && executable->argc == 2) {


            if (father != NULL && father->type != PipeNode_T) {
                Node *brother = node;
                do {
                    brother = find_next_executable_in_operands(father, brother);
                    if (brother != NULL) {
                        add_cd(&brother->value.executable, executable->argv[1]);
                    }
                } while (brother != NULL);
                remove_node_from_operands(father, node);
            } else {
                add_cd(executable, executable->argv[1]);
            }
        }
    }
}

void add_cd (ExecutableNode *executable, char *dir) {
    int i = executable->cd_count;
    executable->cd_count++;
    executable->cd = realloc(executable->cd, executable->cd_count * sizeof(char*));
    executable->cd[i] = strdup(dir);
}

void wire_redirects (Node *root) {
    Stream *std_out = root->std_out == NULL ? create_std_stream(STDOUT_FILENO) : root->std_out;
    Stream *std_in = root->std_in == NULL ? create_std_stream(STDIN_FILENO) : root->std_in;
    wire_redirects_r(root, std_in, std_out);
}

void wire_redirects_r(Node *node, Stream *in, Stream *out) {
    if (node->std_in == NULL) {
        node->std_in = in;
    }
    if (node->std_out == NULL) {
        node->std_out = out;
    }


    switch (node->type) {
        case ExecutableNode_T:
            // nothing to do
            break;
        case PipeNode_T:
            wire_pipe_nodes(&node->value.operands, in, out);
            break;

        case AndNode_T:
        case OrNode_T:
        case SemicolonNode_T:
            wire_operand_nodes(&node->value.operands, in, out);
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
    printf("all str %d\n", stream);
    return stream;
}

void wire_pipe_nodes(OperandsNode *operands, Stream *in, Stream *out) {
    // create all the pipes
    int nodes_count     = operands->count;
    int pipe_count      = operands->count - 1;
    PipeStream **pipes  = malloc(pipe_count * sizeof(PipeStream*));
    int i;
    for (i = 0; i < pipe_count; i++) {
        pipes[i] = create_pipe();
    }

    // wire first node
    Node *first                 = operands->nodes[0];
    PipeStream *first_to_second = pipes[0];
    wire_redirects_r(first, in,
                     wrap_pipe_into_stream(first_to_second, WRITE_INTO_PIPE));

    // wire middle nodes
    for (i = 1; i < nodes_count - 1; i++) {

        // i - 1 is the left node, i the center and i + 1 the right node
        Node *center                = operands->nodes[i];

        PipeStream *left_to_center  = pipes[i - 1];
        PipeStream *center_to_right = pipes[i];

        wire_redirects_r(center,
                         wrap_pipe_into_stream(left_to_center, READ_FROM_PIPE),
                         wrap_pipe_into_stream(center_to_right,
                                               WRITE_INTO_PIPE));
    }

    // wire last node
    int last_node_index = pipe_count;
    int last_pipe_index = pipe_count - 1;
    Node *last          = operands->nodes[last_node_index];
    PipeStream *to_last = pipes[last_pipe_index];
    wire_redirects_r(last, wrap_pipe_into_stream(to_last, READ_FROM_PIPE), out);

    // we don't need the array with all the pipes anymore
    free(pipes);
}


void wire_operand_nodes(OperandsNode *operands, Stream *in, Stream *out) {
    operands->nodes[0]->std_in = in;

    Appender *appender = operands->appender = malloc(sizeof(Appender));
    appender->from_count = operands->count;
    appender->from = malloc(operands->count * sizeof(Stream*));
    appender->to = out;

    int i;
    for (i = 0; i < operands->count; i++) {
        Node *node = operands->nodes[i];
        PipeStream *node_to_concat = create_pipe();

        Stream *node_in = node->std_in;
        if (node_in == NULL) {
            node_in = create_std_stream(STDIN_FILENO);
        }

        wire_redirects_r(node, node_in,
                         wrap_pipe_into_stream(node_to_concat, WRITE_INTO_PIPE));

        appender->from[i] = wrap_pipe_into_stream(node_to_concat, READ_FROM_PIPE);
    }
}



