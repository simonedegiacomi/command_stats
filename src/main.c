#include <stdio.h>
#include "parse.h"
#include "execute.h"
#include "wire.h"





int main (int argc, char *argv[]) {

	const char *input = argv[1];
	Node *command_tree = create_tree_from_string(input);
    wire(command_tree);
    execute(command_tree);
	return 0;
}

