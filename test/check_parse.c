#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/parse.h"
#include "my_assert.h"


void check_executable_equals (ExecutableNode *expected, ExecutableNode *actual);
void check_operands_equals (OperandsNode *expected, OperandsNode *actual);

void check_tree_equals (Node *expected, Node *actual) {
	my_assert(expected != NULL && actual != NULL, "null node");
	my_assert(expected->type == actual->type, "different type");

	switch (expected->type) {
		case ExecutableNode_T: {
			return check_executable_equals(
				&expected->value.executable,
				&actual->value.executable);
			break;
		}
		case PipeNode_T: {
			check_tree_equals(
				expected->value.pipe.from,
				actual->value.pipe.from
			);
			check_tree_equals(
				expected->value.pipe.to,
				actual->value.pipe.to
			);
			break;
		}
		case AndNode_T:
		case OrNode_T: {
			return check_operands_equals(
				&expected->value.operands,
				&actual->value.operands);
		}
		default: {
			my_assert(FALSE, "unknown node type");
		}
	}
}

void check_executable_equals (ExecutableNode *expected, ExecutableNode *actual) {
	my_assert(expected != NULL && actual != NULL, "null executable");
	my_assert(strcmp(expected->path, actual->path) == 0, "different path");
	my_assert(expected->argc == actual->argc, "different argc");

	for (int i = 0; i < expected->argc; i++) {
		my_assert(strcmp(expected->argv[i], actual->argv[i]) == 0, "different arg");
	}
}

void check_operands_equals (OperandsNode *expected, OperandsNode *actual) {
	my_assert(expected != NULL && actual != NULL, "null executable");
	my_assert(expected->operands == actual->operands, "differen operands count");

	int i;
	for (i = 0; i < expected->operands; i++) {
		check_tree_equals(expected->nodes[i], actual->nodes[i]);
	}
}


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

void should_parse_ls_pipe_wc () {
	char *input = "ls -lah | wc";
	Node *parsed = create_tree_from_string(input);

	Node expected;
	expected.type = PipeNode_T;
	PipeNode *pipe = &expected.value.pipe;

	Node *from = pipe->from = malloc(sizeof(Node));
	from->type = ExecutableNode_T;
	ExecutableNode *ls = &from->value.executable;
	ls->path = "ls";
	ls->argc = 2;
	ls->argv = malloc(2 * sizeof(char*));
	ls->argv[0] = "ls";
	ls->argv[1] = "-lah";


	Node *to = pipe->to = malloc(sizeof(Node));
	to->type = ExecutableNode_T;
	ExecutableNode *wc = &to->value.executable;
	wc->path = "wc";
	wc->argc = 1;
	wc->argv = malloc(1 * sizeof(char*));
	wc->argv[0] = "wc";

	check_tree_equals(&expected, parsed);
}

void should_parse_ls_pipe_wc_pipe_wc () {
	char *input = "ls -lah | wc | wc   ";
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

	Node pipe2 = {
		.type = PipeNode_T,
		.value = {
			.pipe = {
				.from = &wc,
				.to = &wc
			}
		}
	};

	Node expected = {
		.type = PipeNode_T,
		.value = {
			.pipe = {
				.from = &ls,
				.to = &pipe2
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


int main(int argc, char **argv) {
    should_parse_ls_with_argument();
   	should_parse_ls_with_two_arguments();

    should_parse_ls_pipe_wc();
    should_parse_ls_pipe_wc_pipe_wc();

    should_parse_and();
    should_parse_or();

    printf("ALL TEST PASSED\n");
 
    return 0;
}
