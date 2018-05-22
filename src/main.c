#include <stdio.h>
#include "parse.h"
#include "execute.h"
#include "wire.h"


int main (int argc, char *argv[]) {
	// TODO: Parse arguments

	printf("[RUN] Started\n");

	const char *input = argv[1];
    initialize_parser();
	Node *command_tree = create_tree_from_string(input);
	printf("[RUN] Parsed!\n");

    wire(command_tree);
	printf("[RUN] Wired!\n");

    execute(command_tree);
	printf("[RUN] Executed!\n");

	// TODO: Send results to deamon

	printf("[RUN] Done!\n");

	return 0;
}

