#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
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
		case PipeNode_T:
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
	my_assert(expected->argc == actual->argc, "different argc %d != %d", expected->argc, actual->argc);

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