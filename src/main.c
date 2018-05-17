#include <stdio.h>
#include "parse.h"
#include "execute.h"
#include "wire.h"


int main (int argc, char *argv[]) {
	// TODO: Parse arguments

	printf("[RUN] Running\n");

	const char *input = argv[1];
	Node *command_tree = create_tree_from_string(input);
    wire(command_tree);
    execute(command_tree);

	// TODO: Send results to deamon

	printf("[RUN] Done!\n");

	return 0;
}

