#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/parse.h"
#include "my_assert.h"
#include "utils.h"
#include <fcntl.h>



void should_parse_ls_with_argument () {
	char *input = "ls -lah";
	Node *parsed = create_tree_from_string(input);

	Node expected;
	expected.type = ExecutableNode_T;
	ExecutableNode *executable = &expected.value.executable;
	executable->path = "ls";
	executable->argc = 2;
	executable->argv = malloc(2 * sizeof(char*));
	executable->argv[0] = "ls";
	executable->argv[1] = "-lah";

	check_tree_equals(&expected, parsed);
}

void should_parse_ls_with_two_arguments () {
	char *input = "ls -l -a";
	Node *parsed = create_tree_from_string(input);

	Node expected;
	expected.type = ExecutableNode_T;
	ExecutableNode *executable = &expected.value.executable;
	executable->path = "ls";
	executable->argc = 3;
	executable->argv = malloc(4 * sizeof(char*));
	executable->argv[0] = "ls";
	executable->argv[1] = "-l";
	executable->argv[2] = "-a";

	check_tree_equals(&expected, parsed);
}

void should_parse_ls_pipe_wc_pipe_wc () {
	char *input = "ls -lah | wc | wc";
	Node *parsed = create_tree_from_string(input);

	char** lsArgs = malloc(2 * sizeof(char*));
	lsArgs[0] = "ls";
	lsArgs[1] = "-lah";
	Node ls = {
		.type = ExecutableNode_T,
		.value = {
			.executable = {
				.path = "ls",
				.argc = 2,
				.argv = lsArgs
			}
		}
	};

	char** wcArgs = malloc(sizeof(char*));
	wcArgs[0] = "wc";
	Node wc = {
		.type = ExecutableNode_T,
		.value = {
			.executable = {
				.path = "wc",
				.argc = 1,
				.argv = wcArgs
			}
		}
	};

	Node *nodes[] = { &ls, &wc, &wc };
	Node expected = {
		.type = PipeNode_T,
		.value = {
			.operands = {
				.operands = 3,
				.nodes = nodes
			}
		}
	};

	check_tree_equals(&expected, parsed);
}

void should_parse_and () {
	char *input = "true && true";
	Node *parsed = create_tree_from_string(input);

	char *trueNodeArgs[] = { "true" };
	Node trueNode = {
		.type = ExecutableNode_T,
		.value = {
			.executable = {
				.path = "true",
				.argc = 1,
				.argv = trueNodeArgs
			}
		}
	};
	Node *nodes[] = { &trueNode, &trueNode };

	Node expected = {
		.type = AndNode_T,
		.value = {
			.operands = {
				.operands = 2,
				.nodes = nodes
			}
		}
	};

	check_tree_equals(&expected, parsed);
}

void should_parse_or () {
	char *input = "true || true";
	Node *parsed = create_tree_from_string(input);

	char *trueNodeArgs[] = { "true" };
	Node trueNode = {
		.type = ExecutableNode_T,
		.value = {
			.executable = {
				.path = "true",
				.argc = 1,
				.argv = trueNodeArgs
			}
		}
	};
	Node *nodes[] = { &trueNode, &trueNode };

	Node expected = {
		.type = OrNode_T,
		.value = {
			.operands = {
				.operands = 2,
				.nodes = nodes
			}
		}
	};

	check_tree_equals(&expected, parsed);
}

void should_parse_parentesis () {
	char *command = "(true && true) || true";
	Node *parsed = create_tree_from_string(command);

	char *trueNodeArgs[] = { "true" };
	Node true_node = {
		.type = ExecutableNode_T,
		.value = {
			.executable = {
				.path = "true",
				.argc = 1,
				.argv = trueNodeArgs
			}
		}
	};
	Node *and_operands[] = { &true_node , &true_node };
	Node and_node = {
		.type = AndNode_T,
		.value = {
			.operands = {
				.operands = 2,
				.nodes = and_operands
			}
		}
	};

	Node *or_operands[] = { &and_node, &true_node };
	Node or_node = {
		.type = OrNode_T,
		.value = {
			.operands = {
				.operands = 2,
				.nodes = or_operands
			}
		}
	};

	check_tree_equals(&or_node, parsed);
}

void should_parse_redirect () {
	char *command = "ls > file.txt";
	Node *parsed = create_tree_from_string(command);

	char *lsNodeArgs[] = { "ls" };
	Stream file_out = {
		.type = FileStream_T,
		.options = {
			.file = {
				.name = "file.txt",
				.open_flag = O_WRONLY
			}
		}
	};
	Node expected = {
		.type = ExecutableNode_T,
		.value = {
			.executable = {
				.path = "ls",
				.argc = 1,
				.argv = lsNodeArgs
			}
		},
		.stdout = &file_out
	};

	check_tree_equals(&expected, parsed);
}

void should_parse_and_inside_parentesis_with_pipe () {
	char *command = "(ls && ls) > file.txt";
	Node *parsed = create_tree_from_string(command);

	char *lsNodeArgs[] = { "ls" };
	Node ls = {
		.type = ExecutableNode_T,
		.value = {
			.executable = {
				.path = "ls",
				.argc = 1,
				.argv = lsNodeArgs
			}
		}
	};
	Node *nodes[] = { &ls, &ls };
	Stream file_out = {
		.type = FileStream_T,
		.options = {
			.file = {
				.name = "file.txt",
				.open_flag = O_WRONLY
			}
		}
	};
	Node expected = {
		.type = AndNode_T,
		.value = {
			.operands = {
				.operands = 2,
				.nodes = nodes
			}
		},
		.stdout = &file_out
	};

	check_tree_equals(&expected, parsed);
}

void should_parse_escaped_parameters () {
	char *command = "echo \"a b\"\"c\" \"d\""; // echo "a b""c" "d"
}

void run_parser_test () {
	printf("[PARSER TEST] Start tests\n");
	should_parse_ls_with_argument();
   	should_parse_ls_with_two_arguments();
    should_parse_ls_pipe_wc_pipe_wc();

    should_parse_and();
    should_parse_or();

    should_parse_parentesis();
    // NOTE: The following tests are commented only because redirect is not supported yet
    //should_parse_redirect();
    //should_parse_and_inside_parentesis_with_pipe();
    //should_parse_escaped_parameters();
	
    printf("[PARSER TEST] All test passed\n");
}


#ifndef MAIN_TESTS
int main(int argc, char **argv) {
	run_parser_test();
 
    return 0;
}
#endif
