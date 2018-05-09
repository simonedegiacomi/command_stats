#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "wire.h"

void wire_r (Node *tree, int stdin, int stdout);

void wire (Node *tree) {
	wire_r(tree, STDIN_FILENO, STDOUT_FILENO);
}


void wire_r (Node *tree, int in, int out) {
	tree->stdin = in;
	tree->stdout = out;

	switch (tree->type) {
		case PipeNode_T: {
			Node *from = tree->value.pipe.from;
			Node *to = tree->value.pipe.to;

			int pipe_fds[2];
			pipe(pipe_fds);

			wire_r(from, in, pipe_fds[1]);
			wire_r(to, pipe_fds[0], out);
			break;
		}
		default: {
			// No need to do anything
		}
	}
}



