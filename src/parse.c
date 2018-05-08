#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parse.h"


typedef struct SplitResult {
	int count;
	const char **values;
} SplitResult;
SplitResult* split_string_for_operator (const char *string, const char *operator);


typedef struct PriorityMapEntry {
	char *separator;
	Node *(*handler) (SplitResult*);
} PriorityMapEntry;

Node * create_pipe_from_strings			(SplitResult *pieces);
Node * create_and_from_strings			(SplitResult *pieces);
Node * create_or_from_strings			(SplitResult *pieces);
Node * create_executable_from_string	(const char *string);


PriorityMapEntry priority_map[] = {
	{ "&&", create_and_from_strings },
	{ "||", create_or_from_strings },
	// TODO: Parla di questo problema, mettendo la pipe sotto
	{ "|", create_pipe_from_strings }
};
const int operators_count = sizeof(priority_map) / sizeof(PriorityMapEntry);

Node * create_tree_from_string (const char *string) {
	int i = 0;
	for (i = 0; i < operators_count; i++) {
		PriorityMapEntry *entry = &priority_map[i];
		SplitResult *pieces = split_string_for_operator(string, entry->separator);

		if (pieces != NULL) {
			return entry->handler(pieces);
		}
	}

	return create_executable_from_string(string);
}

BOOL string_begins_with (const char *string, const char *sub_string) {
	char *start = strstr(string, sub_string);

	// Controllo tutta la stringa
	int sub_string_length = strlen(sub_string);
	if (strncmp(string, sub_string, sub_string_length) != 0) {
		return FALSE;
	}

	// Controllo .... che l'ultimo non sia lo stesso???
	if (string[sub_string_length] != '\0' &&
		string[sub_string_length] == sub_string[sub_string_length - 1]) {
		return FALSE;
	}

	return (start - string == 0) ? TRUE : FALSE;
}

int count_occurrences_in_first_level_string (const char *string, const char *to_find) {
	int i, count = 0, parentesis = 0;
	int to_find_length = strlen(to_find);
	int to_increment_after_found = to_find_length - 1;
	for (i = 0; string[i] != '\0'; i++) {
		char character = string[i];

		if (character == '(') {
			parentesis++;
		} else if (character == ')') {
			parentesis--;
		} else if (parentesis == 0 && string_begins_with(&string[i], to_find)) {
			count++;
			i += to_increment_after_found;
		}
	}
	return count;
}

SplitResult *split_string_for_operator (const char *string, const char *operator) {
	int count = count_occurrences_in_first_level_string(string, operator);
	if (count <= 0) {
		return NULL;
	}

	count++;

	char **pieces = malloc(count * sizeof(char*));

	int operatorLength = strlen(operator);

	int i, j, parentesis, start;
	j = start = parentesis = 0;
	for (i = 0; string[i] != '\0'; i++) {
		char character = string[i];
		if (character == '(') {
			parentesis++;
		} else if (character == ')') {
			parentesis--;
		} else if (parentesis == 0 && string_begins_with(&string[i], operator)) {
			int characters_to_copy = i - start;
			pieces[j++] = strndup(&string[start], characters_to_copy);
			i += operatorLength;
			start = i;
		}
	}
	pieces[j++] = strdup(&string[start]);

	SplitResult *res = malloc(sizeof(SplitResult));
	res->count = count;
	res->values = (const char**) pieces;
	return res;
}

// Potrei fare che ci sia una funzione pe rogni operatore che accetti
// la stringa contentente l'operatore (es: ls | wc), ma questo farebbe copiare
// molte volte l'operazione di divisione dei due sottocomandi.
// Però se è tutto in una funzione forse è più flessibile?
// Perchè l'altra soluzione è quella di avere dei costruttori che vengono chiamati
// in ciclo da un solo metodo. Il costruttore corretto e la priorità degli
// operatori viene così decisa dalla priority map.


Node * create_executable_from_string (const char *string) {
	Node *node = malloc(sizeof(Node));
	node->type = ExecutableNode_T;
	ExecutableNode *executable = &node->value.executable;

	// Skip space
	while (string[0] == ' ') {
		string++;
	}

	char *copyToFree = strdup(string);
	char *string_copy = copyToFree;

	// Parse path
	char *path = strsep(&string_copy, " ");
	executable->path = strdup(path);

	// Count arguments
	int i;
	int arguments = 0;
	BOOL lastSpace = FALSE;
	for (i = 0; string[i] != '\0'; i++) {
		if (string[i] == ' ') {
			lastSpace = TRUE;
			arguments++;
			// Skip space
			while (string[i] == ' ') {
				i++;
			}
		} else {
			lastSpace = FALSE;
		}
	}
	if (lastSpace == FALSE) {
		arguments++;
	}
	executable->argc = arguments;

	// Parse arguments
	executable->argv = malloc(arguments * sizeof(char*));
	executable->argv[0] = strdup(executable->path);
	if (arguments == 1) {
		return node;
	}

	for (i = 1; i < arguments; i++) {
		char *arg = strsep(&string_copy, " ");
		executable->argv[i] = strdup(arg);
	}
	
	free(copyToFree);
	return node;
}


Node * create_pipe_from_strings (SplitResult *pieces) {
	Node *node = malloc(sizeof(Node));
	node->type = PipeNode_T;
	PipeNode *pipe = &node->value.pipe;

	pipe->from = create_tree_from_string(pieces->values[0]);
	if (pieces->count == 2) {
		pipe->to = create_tree_from_string(pieces->values[1]);
	} else {
		SplitResult subSplit;
		subSplit.count = pieces->count - 1;
		subSplit.values = malloc(subSplit.count * sizeof(char*));
		int i;
		for (i = 0; i < subSplit.count; i++) {
			subSplit.values[i] = pieces->values[i + 1];
		}
		pipe->to = create_pipe_from_strings(&subSplit);
	}

	return node;
}

Node * create_and_from_strings (SplitResult *pieces) {
	Node *node = malloc(sizeof(Node));
	node->type = AndNode_T;
	OperandsNode *andNode = &node->value.operands;
	andNode->operands = pieces->count;
	andNode->nodes = malloc(pieces->count * sizeof(Node*));

	int i = 0;
	for (i = 0; i < andNode->operands; i++) {
		andNode->nodes[i] = create_tree_from_string(pieces->values[i]);
	}
	
	return node;
}


Node * create_or_from_strings (SplitResult *pieces) {
	Node *node = malloc(sizeof(Node));
	node->type = OrNode_T;
	OperandsNode *orNode = &node->value.operands;
	orNode->operands = pieces->count;
	orNode->nodes = malloc(pieces->count * sizeof(Node*));

	int i = 0;
	for (i = 0; i < orNode->operands; i++) {
		orNode->nodes[i] = create_tree_from_string(pieces->values[i]);
	}
	
	return node;
}
