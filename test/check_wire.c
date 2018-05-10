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

	Stream *out = root.stdin[0];
	my_assert(out->type == FileDescriptorStream_T, "wrong stream type");
	my_assert(out->file_descriptor == STDOUT_FILENO, "not stdout file descriptor");

	Stream *in = root.stdin[1];
	my_assert(in->type == FileDescriptorStream_T, "wrong stream type");
	my_assert(in->file_descriptor == STDIN_FILENO, "not stdout file descriptor");
}

/*
void should_wire_a_pipe () {
	Node from = {
		.type = ExecutableNode_T,
		.value = {
			.executable = {
				.path = "ls"
			}
		}
	};
	Node to = {
		.type = ExecutableNode_T,
		.value = {
			.executable = {
				.path = "wc"
			}
		}
	};
	Node pipe = {
		.type = PipeNode_T,
		.value = {
			.pipe = {
				.from = &from,
				.to = &to
			}
		}
	};

	wire(&pipe);

	my_assert(pipe.stdin == STDIN_FILENO, "not bound to stdin");
	my_assert(from.stdin == STDIN_FILENO, "not bound to stdin (%d)", from.stdin);
	my_assert(from.stdout == 4, "not bound to pipe");
	my_assert(to.stdin == 3, "not bound to pipe");
	my_assert(to.stdout == STDOUT_FILENO, "not bound to stdout");
	my_assert(pipe.stdout == STDOUT_FILENO, "not bound to stdout");
}
*/
void run_wire_tests () {
	printf("[WIRE TEST] Start tests\n");
	should_wire_a_single_node_tree_to_std();
	//should_wire_a_pipe();
    printf("[WIRE TEST] All test passed\n");
}

#ifndef MAIN_TESTS
int main (int argc, char *argv[]) {
	run_wire_tests();
	return 0;
}
#endif