#include <stdio.h>
#include <unistd.h>
#include "my_assert.h"
#include "../src/wire.h"


int pipe (int fds[]) {
	static int count = 3;

	fds[0] = count++;
	fds[1] = count++;

	return 0;
}

void should_wire_a_single_node_tree_to_std () {
	Node root = {
		.type = ExecutableNode_T,
		.value = {
			.executable = {
				.path = "ls"
			}
		}
	};

	wire(&root);

	Stream *out = root.stdout;
	my_assert(out->type == FileDescriptorStream_T, "wrong stream type");
	my_assert(out->file_descriptor == STDOUT_FILENO, "not stdout file descriptor");

	Stream *in = root.stdins[0];
	my_assert(in->type == FileDescriptorStream_T, "wrong stream type");
	my_assert(in->file_descriptor == STDIN_FILENO, "not stdout file descriptor");
}

void run_wire_tests () {
	printf("[WIRE TEST] Start tests\n");
	should_wire_a_single_node_tree_to_std();
    printf("[WIRE TEST] All test passed\n");
}

#ifndef MAIN_TESTS
int main (int argc, char *argv[]) {
	run_wire_tests();
	return 0;
}
#endif