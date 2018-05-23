#include <stdio.h>
#include "parse.h"
#include "execute.h"
#include "wire.h"
#include "collect_results.h"



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

	/*const char **options = parse_options("exit_code,usner_cpu_time");
	if (options != NULL) {
		for (int i = 0; i < 2; i++) {
			printf("%s\n", options[i]);
		}
	} else {
		printf("NO options\n");
	}*/
	

	return 0;
}

