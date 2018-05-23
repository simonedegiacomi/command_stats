#include <stdio.h>
#include <unistd.h>
#include "my_assert.h"
#include "../src/parse/wire.h"
#include "../src/common/common.h"
#include "../src/structs/node.h"


int pipe(int fds[]) {
    static int count = 3;

    fds[0] = count++;
    fds[1] = count++;

    return 0;
}

void assert_std_stream(Stream *to_check, int direction) {
    my_assert(to_check->type == FileDescriptorStream_T, "wrong stream type");
    my_assert(to_check->file_descriptor == direction, "not std_out[0] file descriptor");
}

void assert_pipe_connected(Stream *from, Stream *to) {
    my_assert(from->type == PipeStream_T &&
              to->type == PipeStream_T, "stream not a pipe");

    PipeStream *from_pipe = from->options.pipe;
    PipeStream *to_pipe = to->options.pipe;
    my_assert(from_pipe == to_pipe, "wrong pipe descriptors");
}

void should_wire_a_single_node_tree_to_std() {
    Node root = {
            .type = ExecutableNode_T,
            .value = {
                    .executable = {
                            .path = "ls"
                    }
            }
    };

    wire(&root);

    assert_std_stream(root.std_out, STDOUT_FILENO);
    assert_std_stream(root.std_in, STDIN_FILENO);
}

void should_wire_a_pipe() {
    Node *ls = create_executable_node("ls");
    Node *wc = create_executable_node("wc");
    Node *nodes[2] = {ls, wc};
    Node pipe = {
            .type = PipeNode_T,
            .value = {
                    .operands = {
                            .count = 2,
                            .nodes = nodes
                    }
            }
    };

    wire(&pipe);

    assert_std_stream(pipe.std_out, STDOUT_FILENO);
    assert_std_stream(pipe.std_in, STDIN_FILENO);

    assert_std_stream(ls->std_in, STDIN_FILENO);
    assert_pipe_connected(ls->std_out, wc->std_in);
    assert_std_stream(wc->std_out, STDOUT_FILENO);
}

void should_wire_two_pipes () {
    Node *ls = create_executable_node("ls");
    Node *wc1 = create_executable_node("wc");
    Node *wc2 = create_executable_node("wc");
    Node *nodes[3] = {ls, wc1, wc2};
    Node pipe = {
            .type = PipeNode_T,
            .value = {
                    .operands = {
                            .count = 3,
                            .nodes = nodes
                    }
            }
    };

    wire(&pipe);

    assert_std_stream(pipe.std_out, STDOUT_FILENO);
    assert_std_stream(pipe.std_in, STDIN_FILENO);

    assert_std_stream(ls->std_in, STDIN_FILENO);
    assert_pipe_connected(ls->std_out, wc1->std_in);
    assert_pipe_connected(wc1->std_out, wc2->std_in);
    assert_std_stream(wc2->std_out, STDOUT_FILENO);
}

void should_wire_operands () {
    Node *true = create_executable_node("true");
    Node *false = create_executable_node("false");
    Node *nodes[2] = {true, false};
    Node and = {
            .type = AndNode_T,
            .value = {
                    .operands = {
                            .count = 2,
                            .nodes = nodes
                    }
            }
    };

    wire(&and);

    assert_std_stream(and.std_in, STDIN_FILENO);
    assert_std_stream(true->std_in, STDIN_FILENO);
    assert_std_stream(false->std_in, STDIN_FILENO);
    assert_std_stream(and.std_out, STDOUT_FILENO);

    Appender *concat = and.value.operands.appender;
    my_assert(concat->from_count == and.value.operands.count, "%d != %d", concat->from_count, and.value.operands.count);
    assert_pipe_connected(true->std_out, concat->from[0]);
    assert_pipe_connected(false->std_out, concat->from[1]);
}

void run_wire_tests() {
    printf("[WIRE TEST] Start tests\n");
    should_wire_a_single_node_tree_to_std();
    should_wire_a_pipe();
    should_wire_two_pipes();
    should_wire_operands();
    printf("[WIRE TEST] All test passed\n");
}

#ifndef MAIN_TESTS

int main(int argc, char *argv[]) {
    run_wire_tests();
    return 0;
}

#endif