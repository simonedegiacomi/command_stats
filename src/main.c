#include <stdio.h>
#include "parse.h"
#include "execute.h"


int main (int argc, char *argv[]) {

	const char *input = argv[1];
	Node *command_tree = create_tree_from_string(input);
    execute(command_tree);

	return 0;
}

